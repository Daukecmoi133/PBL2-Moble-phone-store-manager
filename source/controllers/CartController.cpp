#include "AppControllerSupport.h"
#include <algorithm>
#include <set>
using namespace AppControllerHelpers;

namespace {
std::string resolveCustomerId(const UserSessionViewModel &session,
                              const std::string &requestedId) {
  if (session.isAuthenticated && session.isCustomer()) {
    return session.userId;
  }
  if (!requestedId.empty()) {
    return requestedId;
  }
  if (session.isAuthenticated) {
    return session.userId;
  }
  return "GUEST";
}

CartSummaryViewModel &getOrCreateUserCart(MockDataStore &store,
                                          const std::string &customerId) {
  std::string key = customerId.empty() ? "GUEST" : customerId;
  auto it = store.userCarts.find(key);
  if (it == store.userCarts.end()) {
    CartSummaryViewModel newCart;
    newCart.customerId = key;
    store.userCarts[key] = newCart;
  }
  return store.userCarts[key];
}
} // namespace

// === GIỎ HÀNG & CHECKOUT ===

CartSummaryViewModel AppController::getCart(const std::string &customerId) {
  auto &store = MockDataStore::getInstance();
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(store, effectiveId);
  cart.customerId = effectiveId;
  cart.recalculate();
  store.customerCart = cart;
  return cart;
}

OperationResult AppController::addToCart(const std::string &customerId,
                                         const std::string &phoneId,
                                         int quantity) {
  if (quantity <= 0) {
    return OperationResult::Fail("Số lượng thêm vào giỏ phải lớn hơn 0!");
  }

  ProductItemViewModel product = getProductById(phoneId);
  if (product.id.empty()) {
    return OperationResult::Fail("Không tìm thấy sản phẩm!");
  }

  auto &store = MockDataStore::getInstance();
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(store, effectiveId);
  cart.customerId = effectiveId;

  bool found = false;
  for (auto &item : cart.items) {
    if (item.phoneId == phoneId) {
      item.quantity += quantity;
      item.availableStock = product.availableStock;
      item.calculateSubtotal();
      found = true;
      break;
    }
  }

  if (!found) {
    CartItemViewModel newItem;
    newItem.phoneId = product.id;
    newItem.phoneName = product.name;
    newItem.brand = product.brand;
    newItem.unitPrice = product.sellingPrice;
    newItem.quantity = quantity;
    newItem.availableStock = product.availableStock;
    newItem.calculateSubtotal();
    cart.items.push_back(newItem);
  }

  cart.recalculate();
  store.customerCart = cart;
  return OperationResult::Ok("Đã thêm '" + product.name + "' vào giỏ hàng!");
}

OperationResult AppController::updateCartQuantity(const std::string &customerId,
                                                  const std::string &phoneId,
                                                  int newQuantity) {
  auto &store = MockDataStore::getInstance();
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  if (newQuantity <= 0) {
    return removeFromCart(effectiveId, phoneId);
  }

  ProductItemViewModel product = getProductById(phoneId);
  if (newQuantity > product.availableStock) {
    return OperationResult::Fail("Số lượng vượt quá tồn kho khả dụng (" +
                                 std::to_string(product.availableStock) + ")!");
  }

  auto &cart = getOrCreateUserCart(store, effectiveId);
  for (auto &item : cart.items) {
    if (item.phoneId == phoneId) {
      item.quantity = newQuantity;
      item.calculateSubtotal();
      cart.recalculate();
      store.customerCart = cart;
      return OperationResult::Ok("Đã cập nhật số lượng!");
    }
  }

  return OperationResult::Fail("Sản phẩm không có trong giỏ hàng!");
}

OperationResult AppController::removeFromCart(const std::string &customerId,
                                              const std::string &phoneId) {
  auto &store = MockDataStore::getInstance();
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(store, effectiveId);
  for (auto it = cart.items.begin(); it != cart.items.end(); ++it) {
    if (it->phoneId == phoneId) {
      std::string name = it->phoneName;
      cart.items.erase(it);
      cart.recalculate();
      store.customerCart = cart;
      return OperationResult::Ok("Đã xóa '" + name + "' khỏi giỏ hàng!");
    }
  }
  return OperationResult::Fail("Không tìm thấy sản phẩm trong giỏ hàng!");
}

OperationResult AppController::applyCoupon(const std::string &customerId,
                                           const std::string &couponCode) {
  auto &store = MockDataStore::getInstance();
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(store, effectiveId);
  cart.recalculate();

  if (couponCode.empty()) {
    cart.appliedDiscountCode = "";
    cart.discountAmount = 0.0;
    cart.recalculate();
    store.customerCart = cart;
    return OperationResult::Ok("Đã hủy áp dụng mã giảm giá.");
  }

  double discount = 0.0;
  OperationResult res = validateDiscount(couponCode, cart.subtotal, discount);
  if (!res.success) {
    return res;
  }

  cart.appliedDiscountCode = couponCode;
  cart.discountAmount = discount;
  cart.recalculate();
  store.customerCart = cart;

  return OperationResult::Ok("Áp dụng mã giảm giá '" + couponCode +
                             "' thành công!");
}

OperationResult AppController::checkout(const CheckoutRequest &request,
                                        std::string &outOrderId) {
  outOrderId.clear();
  auto &store = MockDataStore::getInstance();

  const std::string customerId =
      resolveCustomerId(currentSession, request.customerId);
  auto &cart = getOrCreateUserCart(store, customerId);

  if (cart.items.empty()) {
    return OperationResult::Fail(
        "Giỏ hàng của bạn đang trống! Hãy chọn sản phẩm trước.");
  }
  if (request.receiverName.empty() || request.receiverPhone.empty()) {
    return OperationResult::Fail(
        "Họ tên và số điện thoại người nhận không được để trống!");
  }
  if (request.deliveryMethod == 1 &&
      request.deliveryAddress.empty()) { // Delivery
    return OperationResult::Fail(
        "Vui lòng nhập địa chỉ nhận hàng khi chọn giao tận nơi!");
  }
  if (!IdGenerator::isValidId(customerId, "CUS") ||
      getCustomerById(customerId).id.empty()) {
    return OperationResult::Fail(
        "Khách hàng đặt đơn không hợp lệ hoặc không tồn tại!");
  }

  // Lọc danh sách sản phẩm được chọn để thanh toán (nếu có chỉ định)
  std::vector<CartItemViewModel> checkoutItems;
  if (!request.selectedPhoneIds.empty()) {
    std::set<std::string> selectedSet(request.selectedPhoneIds.begin(),
                                      request.selectedPhoneIds.end());
    for (const auto &item : cart.items) {
      if (selectedSet.count(item.phoneId) > 0) {
        checkoutItems.push_back(item);
      }
    }
    if (checkoutItems.empty()) {
      return OperationResult::Fail(
          "Vui lòng chọn ít nhất một sản phẩm trong giỏ để đặt hàng!");
    }
  } else {
    checkoutItems = cart.items;
  }

  // Kiểm tra tồn kho khả dụng lần cuối cho các sản phẩm đặt mua
  for (const auto &item : checkoutItems) {
    ProductItemViewModel p = getProductById(item.phoneId);
    if (p.availableStock < item.quantity) {
      return OperationResult::Fail("Sản phẩm '" + item.phoneName +
                                   "' không còn đủ số lượng khả dụng!");
    }
  }

  // Tính toán tạm tính & giảm giá riêng cho các sản phẩm được chọn
  double checkoutSubtotal = 0.0;
  for (const auto &item : checkoutItems) {
    checkoutSubtotal += item.subtotal;
  }

  double checkoutDiscount = 0.0;
  if (!cart.appliedDiscountCode.empty()) {
    OperationResult discRes = validateDiscount(
        cart.appliedDiscountCode, checkoutSubtotal, checkoutDiscount);
    if (!discRes.success) {
      return OperationResult::Fail(
          "Mã giảm giá '" + cart.appliedDiscountCode +
          "' không đủ điều kiện cho các sản phẩm đã chọn: " + discRes.message);
    }
  }
  double checkoutGrandTotal = checkoutSubtotal - checkoutDiscount;
  if (checkoutGrandTotal < 0.0)
    checkoutGrandTotal = 0.0;

  std::string idError;
  const std::string idDate = IdGenerator::currentDateKey();
  if (idDate.empty() ||
      !IdGenerator::nextId("ORD", idDate, collectIds(store.orders), outOrderId,
                           idError)) {
    return OperationResult::Fail(
        idError.empty() ? "Không thể xác định ngày để sinh mã đơn hàng."
                        : idError);
  }

  // Khởi tạo FileTransaction cho checkout
  FileTransaction tx;
  if (orderService != nullptr)
    tx.addParticipant(orderService->getRepository().getFilePath());
  if (!cart.appliedDiscountCode.empty() && discountService != nullptr) {
    tx.addParticipant(discountService->getRepository().getFilePath());
  }
  tx.onRollback([this]() {
    if (this->orderService)
      this->orderService->reload();
    if (this->discountService)
      this->discountService->reload();
  });

  if (!tx.begin()) {
    return OperationResult::Fail(
        "Không thể khởi tạo giao dịch lưu trữ đơn hàng!");
  }

  // Lấy thời gian hiện tại
  std::time_t now = std::time(nullptr);
  std::tm *tm_now = std::localtime(&now);
  char timeBuf[64];
  std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

  std::string deliveryAddr =
      (request.deliveryMethod == 0)
          ? "Cửa hàng PBL2 Store - 54 Nguyễn Lương Bằng, Đà Nẵng"
          : request.deliveryAddress;

  // Lưu đơn hàng vào file thông qua OrderService
  if (orderService != nullptr) {
    DeliveryMethod deliv = (request.deliveryMethod == 0)
                               ? DeliveryMethod::PICKUP
                               : DeliveryMethod::DELIVERY;
    PaymentMethod pay = (request.paymentMethod == 0)
                            ? PaymentMethod::CASH
                            : PaymentMethod::BANK_TRANSFER;
    ShippingInfo ship(request.receiverName, request.receiverPhone,
                      deliveryAddr);
    Order ord(outOrderId, customerId, deliv, pay, ship);
    ord.setDiscountAmount(checkoutDiscount);
    for (const auto &cItem : checkoutItems) {
      ord.addDetail(OrderDetail(cItem.phoneId, cItem.phoneName, cItem.quantity,
                                cItem.unitPrice));
    }
    std::string serviceErr;
    if (!orderService->createOrder(ord, serviceErr)) {
      tx.rollback();
      return OperationResult::Fail(
          !serviceErr.empty() ? serviceErr
                              : "Lỗi khi lưu đơn hàng vào file dữ liệu!");
    }
    if (!cart.appliedDiscountCode.empty() && discountService != nullptr) {
      if (!discountService->consumeUsage(cart.appliedDiscountCode)) {
        tx.rollback();
        return OperationResult::Fail("Lỗi khi áp dụng mã giảm giá!");
      }
    }
  }

  if (!tx.commit()) {
    return OperationResult::Fail("Lỗi khi hoàn tất lưu trữ đơn hàng!");
  }

  // Giao dịch file thành công -> cập nhật View cache:
  // 1. Giữ chỗ tồn kho (reservedStock += quantity)
  for (const auto &item : checkoutItems) {
    for (auto &p : store.products) {
      if (p.id == item.phoneId) {
        p.reservedStock += item.quantity;
        p.calculateStockStatus();
        break;
      }
    }
  }

  // 2. Cập nhật lượt dùng voucher trong View cache nếu có
  if (!cart.appliedDiscountCode.empty()) {
    for (auto &d : store.discounts) {
      if (d.code == cart.appliedDiscountCode) {
        d.usedCount += 1;
        break;
      }
    }
  }

  OrderItemViewModel newOrder;
  newOrder.id = outOrderId;
  newOrder.customerId = customerId;
  newOrder.customerName = request.receiverName;
  newOrder.orderTime = timeBuf;
  newOrder.status = OrderStatus::PENDING;
  newOrder.statusText = "Chờ xử lý";
  newOrder.deliveryMethod = (request.deliveryMethod == 0)
                                ? DeliveryMethod::PICKUP
                                : DeliveryMethod::DELIVERY;
  newOrder.deliveryMethodText = deliveryMethodToString(newOrder.deliveryMethod);
  newOrder.paymentMethod = (request.paymentMethod == 0)
                               ? PaymentMethod::CASH
                               : PaymentMethod::BANK_TRANSFER;
  newOrder.paymentMethodText = paymentMethodToString(newOrder.paymentMethod);
  newOrder.paymentStatus =
      (newOrder.paymentMethod == PaymentMethod::BANK_TRANSFER)
          ? PaymentStatus::PAID
          : PaymentStatus::UNPAID;
  newOrder.paymentStatusText = paymentStatusToString(newOrder.paymentStatus);
  newOrder.subtotal = checkoutSubtotal;
  newOrder.discountAmount = checkoutDiscount;
  newOrder.total = checkoutGrandTotal;
  newOrder.receiverName = request.receiverName;
  newOrder.receiverPhone = request.receiverPhone;
  newOrder.deliveryAddress = deliveryAddr;

  for (const auto &cItem : checkoutItems) {
    OrderDetailItemViewModel d;
    d.phoneId = cItem.phoneId;
    d.phoneName = cItem.phoneName;
    d.quantity = cItem.quantity;
    d.unitPrice = cItem.unitPrice;
    d.subtotal = cItem.subtotal;
    newOrder.details.push_back(d);
  }

  store.orders.insert(store.orders.begin(), newOrder);

  // Xóa các sản phẩm đã đặt khỏi giỏ hàng, giữ lại các sản phẩm chưa chọn
  std::set<std::string> checkedOutIds;
  for (const auto &ci : checkoutItems) {
    checkedOutIds.insert(ci.phoneId);
  }
  cart.items.erase(std::remove_if(cart.items.begin(), cart.items.end(),
                                  [&](const CartItemViewModel &it) {
                                    return checkedOutIds.count(it.phoneId) > 0;
                                  }),
                   cart.items.end());
  if (!cart.appliedDiscountCode.empty()) {
    cart.appliedDiscountCode = "";
    cart.discountAmount = 0.0;
  }
  cart.recalculate();
  store.customerCart = cart;

  return OperationResult::Ok("Đặt hàng thành công! Mã đơn: " + outOrderId);
}
