#include "AppControllerSupport.h"
#include <algorithm>
#include <set>
using namespace AppControllerHelpers;

namespace {
std::string resolveCustomerId(const UserSessionViewModel &session,
                              const std::string &requestedId) {
  if (!session.isAuthenticated) {
    return "GUEST";
  }
  if (session.isCustomer()) {
    return session.userId;
  }
  if (!requestedId.empty()) {
    return requestedId;
  }
  return session.userId;
}

CartSummaryViewModel &getOrCreateUserCart(
    std::map<std::string, CartSummaryViewModel> &userCarts,
    const std::string &customerId) {
  std::string key = customerId.empty() ? "GUEST" : customerId;
  auto it = userCarts.find(key);
  if (it == userCarts.end()) {
    CartSummaryViewModel newCart;
    newCart.customerId = key;
    userCarts[key] = newCart;
  }
  return userCarts[key];
}
} // namespace

// === GIỎ HÀNG & CHECKOUT ===

CartSummaryViewModel AppController::getCart(const std::string &customerId) {
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(userCarts, effectiveId);
  cart.customerId = effectiveId;
  cart.recalculate(getVatRate());
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

  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(userCarts, effectiveId);
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

  cart.recalculate(getVatRate());
  return OperationResult::Ok("Đã thêm '" + product.name + "' vào giỏ hàng!");
}

OperationResult AppController::updateCartQuantity(const std::string &customerId,
                                                  const std::string &phoneId,
                                                  int newQuantity) {
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  if (newQuantity <= 0) {
    return removeFromCart(effectiveId, phoneId);
  }

  ProductItemViewModel product = getProductById(phoneId);
  if (newQuantity > product.availableStock) {
    return OperationResult::Fail("Số lượng vượt quá tồn kho khả dụng (" +
                                 std::to_string(product.availableStock) + ")!");
  }

  auto &cart = getOrCreateUserCart(userCarts, effectiveId);
  for (auto &item : cart.items) {
    if (item.phoneId == phoneId) {
      item.quantity = newQuantity;
      item.availableStock = product.availableStock;
      item.calculateSubtotal();
      cart.recalculate(getVatRate());
      return OperationResult::Ok("Đã cập nhật số lượng!");
    }
  }

  return OperationResult::Fail("Sản phẩm không có trong giỏ hàng!");
}

OperationResult AppController::removeFromCart(const std::string &customerId,
                                              const std::string &phoneId) {
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(userCarts, effectiveId);
  for (auto it = cart.items.begin(); it != cart.items.end(); ++it) {
    if (it->phoneId == phoneId) {
      std::string name = it->phoneName;
      cart.items.erase(it);
      cart.recalculate(getVatRate());
      return OperationResult::Ok("Đã xóa '" + name + "' khỏi giỏ hàng!");
    }
  }
  return OperationResult::Fail("Không tìm thấy sản phẩm trong giỏ hàng!");
}

OperationResult AppController::applyCoupon(const std::string &customerId,
                                           const std::string &couponCode) {
  const std::string effectiveId = resolveCustomerId(currentSession, customerId);
  auto &cart = getOrCreateUserCart(userCarts, effectiveId);
  cart.recalculate(getVatRate());

  if (couponCode.empty()) {
    cart.appliedDiscountCode = "";
    cart.discountAmount = 0.0;
    cart.recalculate(getVatRate());
    return OperationResult::Ok("Đã hủy áp dụng mã giảm giá.");
  }

  double discount = 0.0;
  OperationResult res =
      validateDiscountForCustomer(couponCode, cart.subtotal, effectiveId, discount);
  if (!res.success) {
    return res;
  }

  cart.appliedDiscountCode = StringHelper::trim(couponCode);
  cart.discountAmount = discount;
  cart.recalculate(getVatRate());

  return OperationResult::Ok("Áp dụng mã giảm giá '" + cart.appliedDiscountCode +
                             "' thành công!");
}

OperationResult AppController::checkout(const CheckoutRequest &request,
                                        std::string &outOrderId) {
  outOrderId.clear();

  const std::string customerId =
      resolveCustomerId(currentSession, request.customerId);
  auto &cart = getOrCreateUserCart(userCarts, customerId);

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
      customerService == nullptr ||
      customerService->getCustomerById(customerId) == nullptr) {
    return OperationResult::Fail(
        "Khách hàng đặt đơn không hợp lệ hoặc không tồn tại!");
  }
  if (orderService == nullptr) {
    return OperationResult::Fail("Dịch vụ đơn hàng chưa sẵn sàng!");
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

  const std::string activeCoupon =
      !StringHelper::trim(cart.appliedDiscountCode).empty()
          ? StringHelper::trim(cart.appliedDiscountCode)
          : StringHelper::trim(request.discountCode);
  const double vatRate = getVatRate();
  double rawDiscount = 0.0;
  if (!activeCoupon.empty()) {
    double validatedDiscount = 0.0;
    OperationResult discRes = validateDiscountForCustomer(
        activeCoupon, checkoutSubtotal, customerId, validatedDiscount);
    if (!discRes.success) {
      return OperationResult::Fail(
          "Mã giảm giá '" + activeCoupon +
          "' không đủ điều kiện cho các sản phẩm đã chọn: " + discRes.message);
    }
    const DiscountCode *dc =
        discountService ? discountService->getByCode(activeCoupon) : nullptr;
    rawDiscount = (dc != nullptr)
                      ? PricingCalculator::computeRawDiscount(
                            checkoutSubtotal, dc->getType(), dc->getValue(),
                            dc->getMaxDiscountValue())
                      : validatedDiscount;
  }

  const PricingBreakdown pb =
      PricingCalculator::compute(checkoutSubtotal, rawDiscount, vatRate);

  std::string idError;
  const std::string idDate = IdGenerator::currentDateKey();
  if (idDate.empty() ||
      !IdGenerator::nextId("ORD", idDate,
                           collectModelIds(orderService->getAllOrders()),
                           outOrderId, idError)) {
    return OperationResult::Fail(
        idError.empty() ? "Không thể xác định ngày để sinh mã đơn hàng."
                        : idError);
  }

  // Khởi tạo FileTransaction cho checkout
  FileTransaction tx;
  tx.addParticipant(orderService->getRepository().getFilePath());
  if (!activeCoupon.empty() && discountService != nullptr) {
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

  std::string deliveryAddr =
      (request.deliveryMethod == 0)
          ? "Cửa hàng PBL2 Store - 54 Nguyễn Lương Bằng, Đà Nẵng"
          : request.deliveryAddress;

  // Lưu đơn hàng vào file thông qua OrderService (snapshot unitCost & VAT)
  DeliveryMethod deliv = (request.deliveryMethod == 0)
                             ? DeliveryMethod::PICKUP
                             : DeliveryMethod::DELIVERY;
  PaymentMethod pay = (request.paymentMethod == 0)
                          ? PaymentMethod::CASH
                          : PaymentMethod::BANK_TRANSFER;
  ShippingInfo ship(request.receiverName, request.receiverPhone,
                    deliveryAddr);
  Order ord(outOrderId, customerId, deliv, pay, ship);
  ord.setOrderTime(AppClock::now());
  ord.setDiscountAmount(pb.discountAmount);
  ord.setVat(pb.vatRate, pb.vatAmount, true);
  if (!activeCoupon.empty()) {
    ord.setDiscountCode(activeCoupon);
  }
  for (const auto &cItem : checkoutItems) {
    const Phone *ph =
        phoneService ? phoneService->getPhoneById(cItem.phoneId) : nullptr;
    const double snapCost = ph ? ph->getImportPrice() : 0.0;
    ord.addDetail(OrderDetail(cItem.phoneId, cItem.phoneName, cItem.quantity,
                              cItem.unitPrice, snapCost));
  }
  std::string serviceErr;
  if (!orderService->createOrder(ord, serviceErr)) {
    tx.rollback();
    return OperationResult::Fail(
        !serviceErr.empty() ? serviceErr
                            : "Lỗi khi lưu đơn hàng vào file dữ liệu!");
  }
  if (!activeCoupon.empty() && discountService != nullptr) {
    std::string discErr;
    if (!discountService->consumeUsage(activeCoupon, discErr)) {
      tx.rollback();
      return OperationResult::Fail(
          !discErr.empty() ? discErr : "Lỗi khi áp dụng mã giảm giá!");
    }
  }

  if (!tx.commit()) {
    tx.rollback();
    return OperationResult::Fail("Lỗi khi hoàn tất lưu trữ đơn hàng!");
  }

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
  cart.recalculate(getVatRate());

  return OperationResult::Ok("Đặt hàng thành công! Mã đơn: " + outOrderId);
}
