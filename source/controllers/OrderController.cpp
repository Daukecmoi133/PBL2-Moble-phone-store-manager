#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === ĐƠN HÀNG ===

void AppController::autoCompleteDeliveredOrders() {
  if (orderService == nullptr) {
    return;
  }
  std::time_t now = RealTimeManager::getInstance().getSimulationTime();
  std::vector<std::string> toComplete;

  const Vector<Order> &orders = orderService->getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() == OrderStatus::SHIPPING) {
      long long ordEpoch = RealTimeManager::dateTimeToEpochSeconds(
          orders[i].getOrderTime().toString());
      if (ordEpoch > 0 && (now - ordEpoch) >= (2 * 24 * 3600)) {
        toComplete.push_back(orders[i].getID());
      }
    }
  }

  if (!toComplete.empty()) {
    UserSessionViewModel prevSession = currentSession;
    currentSession.isAuthenticated = true;
    currentSession.role = RoleType::ADMIN;
    for (const auto &id : toComplete) {
      updateOrderStatus(id, OrderStatus::COMPLETED);
    }
    currentSession = prevSession;
  }
}

std::vector<OrderItemViewModel> AppController::getAllOrders() {
  if (!currentSession.isAuthenticated || orderService == nullptr) {
    return {};
  }
  autoCompleteDeliveredOrders();
  if (currentSession.isCustomer()) {
    return getCustomerOrders(currentSession.userId);
  }
  const Vector<Order> &orders = orderService->getAllOrders();
  std::vector<OrderItemViewModel> result;
  result.reserve(static_cast<size_t>(orders.getSize()));
  for (int i = orders.getSize() - 1; i >= 0; --i) {
    result.push_back(orderToViewModel(orders[i], customerService));
  }
  return result;
}

std::vector<OrderItemViewModel>
AppController::filterOrders(const std::string &keyword, int statusFilter,
                            int paymentFilter, int typeFilter) {
  if (!currentSession.isAuthenticated || orderService == nullptr) {
    return {};
  }
  autoCompleteDeliveredOrders();
  const Vector<Order> &orders = orderService->getAllOrders();
  std::vector<OrderItemViewModel> result;

  for (int i = orders.getSize() - 1; i >= 0; --i) {
    OrderItemViewModel ord = orderToViewModel(orders[i], customerService);

    // Customer session ownership check
    if (currentSession.isCustomer() &&
        ord.customerId != currentSession.userId) {
      continue;
    }

    // Keyword filter: check id, customerName, receiverPhone
    if (!keyword.empty()) {
      bool match =
          Utf8Helper::containsCaseInsensitive(ord.id, keyword) ||
          Utf8Helper::containsCaseInsensitive(ord.customerName, keyword) ||
          Utf8Helper::containsCaseInsensitive(ord.receiverPhone, keyword);
      if (!match)
        continue;
    }

    // Status filter:
    if (statusFilter >= 0) {
      if (static_cast<int>(ord.status) != statusFilter)
        continue;
    }

    // Payment filter:
    if (paymentFilter >= 0) {
      if (static_cast<int>(ord.paymentStatus) != paymentFilter)
        continue;
    }

    // Type filter: 0 = In store (Tại quầy), 1 = Online
    if (typeFilter >= 0) {
      if (static_cast<int>(ord.deliveryMethod) != typeFilter)
        continue;
    }

    result.push_back(ord);
  }
  return result;
}

std::vector<OrderItemViewModel>
AppController::getCustomerOrders(const std::string &customerId) {
  if (!currentSession.isAuthenticated || currentSession.isWarehouse() ||
      orderService == nullptr) {
    return {};
  }
  autoCompleteDeliveredOrders();
  const Vector<Order> &orders = orderService->getAllOrders();
  std::vector<OrderItemViewModel> result;
  std::string targetId = customerId;
  if (currentSession.isCustomer()) {
    targetId = currentSession.userId;
  }

  for (int i = orders.getSize() - 1; i >= 0; --i) {
    if (!targetId.empty()) {
      if (orders[i].getCustomerId() == targetId) {
        result.push_back(orderToViewModel(orders[i], customerService));
      }
    } else if (currentSession.isCustomer()) {
      // Customer with empty ID cannot view all orders
      continue;
    } else {
      // Admin or Sales can view all orders
      result.push_back(orderToViewModel(orders[i], customerService));
    }
  }
  return result;
}

OrderItemViewModel AppController::getOrderById(const std::string &orderId) {
  if (!currentSession.isAuthenticated || orderService == nullptr) {
    return OrderItemViewModel();
  }
  Order *orderPtr = orderService->getOrderById(orderId);
  if (orderPtr == nullptr) {
    return OrderItemViewModel();
  }
  if (currentSession.isCustomer() &&
      orderPtr->getCustomerId() != currentSession.userId) {
    return OrderItemViewModel();
  }
  return orderToViewModel(*orderPtr, customerService);
}

OperationResult AppController::updateOrderStatus(const std::string &orderId,
                                                 OrderStatus newStatus) {
  if (!currentSession.isAuthenticated) {
    return OperationResult::Fail("Từ chối truy cập: Vui lòng đăng nhập!");
  }
  if (!IdGenerator::isValidId(orderId, "ORD", true)) {
    return OperationResult::Fail(
        "Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
  }
  if (orderService == nullptr) {
    return OperationResult::Fail("Dịch vụ đơn hàng chưa sẵn sàng!");
  }

  Order *orderPtr = orderService->getOrderById(orderId);
  if (orderPtr == nullptr) {
    return OperationResult::Fail("Không tìm thấy đơn hàng " + orderId);
  }
  const OrderItemViewModel ord = orderToViewModel(*orderPtr, customerService);

  // Ràng buộc phân quyền (ANTY.md mục 4)
  if (currentSession.isCustomer()) {
    if (newStatus == OrderStatus::CANCELLED) {
      if (!ord.canCancel()) {
        return OperationResult::Fail(
            "Đơn hàng đang xử lý hoặc vận chuyển, không thể hủy!");
      }
    } else if (newStatus == OrderStatus::COMPLETED) {
      if (ord.status != OrderStatus::SHIPPING &&
          ord.status != OrderStatus::READY_FOR_PICKUP) {
        return OperationResult::Fail(
            "Chỉ có thể xác nhận đã nhận hàng khi đơn hàng đang giao hoặc "
            "sẵn sàng nhận!");
      }
    } else {
      return OperationResult::Fail(
          "Từ chối truy cập: Khách hàng chỉ có thể hủy đơn hoặc xác nhận "
          "đã nhận hàng!");
    }
    if (ord.customerId != currentSession.userId) {
      return OperationResult::Fail("Từ chối truy cập: Bạn không có quyền "
                                   "thao tác trên đơn hàng này!");
    }
  } else if (currentSession.isWarehouse()) {
    // Nhân viên kho: Chuẩn bị hàng, sẵn sàng nhận, bàn giao ship
    if (newStatus != OrderStatus::PREPARING &&
        newStatus != OrderStatus::READY_FOR_PICKUP &&
        newStatus != OrderStatus::SHIPPING) {
      return OperationResult::Fail("Nhân viên kho chỉ có quyền cập nhật "
                                   "trạng thái chuẩn bị hàng và xuất kho!");
    }
  } else if (!currentSession.isAdmin() && !currentSession.isSales()) {
    return OperationResult::Fail(
        "Từ chối truy cập: Thao tác này chỉ dành cho Nhân viên Bán hàng, "
        "Kho hoặc Quản trị viên!");
  }

  // Không cho phép sửa nếu đơn đã Hoàn thành hoặc Hủy
  if (ord.status == OrderStatus::COMPLETED) {
    return OperationResult::Fail(
        "Đơn hàng đã hoàn thành, không thể thay đổi trạng thái!");
  }
  if (ord.status == OrderStatus::CANCELLED) {
    return OperationResult::Fail(
        "Đơn hàng đã bị hủy, không thể thay đổi trạng thái!");
  }
  if (!OrderService::isValidTransition(ord.status, newStatus)) {
    return OperationResult::Fail("Chuyển trạng thái không hợp lệ từ '" +
                                 orderStatusToString(ord.status) + "' sang '" +
                                 orderStatusToString(newStatus) + "'!");
  }

  const bool needDeduct =
      (!OrderService::hasStockDeducted(ord.status) &&
       OrderService::hasStockDeducted(newStatus));
  const bool needRestore =
      (OrderService::hasStockDeducted(ord.status) &&
       newStatus == OrderStatus::CANCELLED);
  const bool needReleaseVoucher =
      (newStatus == OrderStatus::CANCELLED &&
       !StringHelper::trim(orderPtr->getDiscountCode()).empty() &&
       discountService != nullptr);
  const bool needComplete = (newStatus == OrderStatus::COMPLETED);

  // Khởi tạo FileTransaction để đồng bộ lưu trữ bền vững (D4: trừ kho ở CONFIRMED, hoàn kho & hoàn voucher khi hủy)
  FileTransaction tx;
  tx.addParticipant(orderService->getRepository().getFilePath());
  if ((needDeduct || needRestore) && phoneService != nullptr) {
    tx.addParticipant(phoneService->getRepository().getFilePath());
  }
  if (needReleaseVoucher) {
    tx.addParticipant(discountService->getRepository().getFilePath());
  }
  if (needComplete) {
    if (customerService != nullptr && !ord.customerId.empty()) {
      tx.addParticipant(customerService->getRepository().getFilePath());
    }
    if (invoiceService != nullptr) {
      tx.addParticipant(invoiceService->getRepository().getFilePath());
    }
  }
  tx.onRollback([this, needDeduct, needRestore, needReleaseVoucher, needComplete]() {
    if (this->orderService)
      this->orderService->reload();
    if ((needDeduct || needRestore) && this->phoneService)
      this->phoneService->reload();
    if (needReleaseVoucher && this->discountService)
      this->discountService->reload();
    if (needComplete) {
      if (this->customerService)
        this->customerService->reload();
      if (this->invoiceService)
        this->invoiceService->reload();
    }
  });

  if (!tx.begin()) {
    return OperationResult::Fail(
        "Không thể khởi tạo giao dịch lưu trữ trạng thái đơn hàng!");
  }

  // 1. Chuyển trạng thái cốt lõi (tự kiểm tra tồn kho, trừ kho ở CONFIRMED, hoàn kho & hoàn voucher khi CANCELLED)
  std::string serviceErr;
  if (!orderService->transitionStatusCore(orderId, newStatus, serviceErr)) {
    tx.rollback();
    return OperationResult::Fail(
        !serviceErr.empty()
            ? serviceErr
            : "Lỗi khi lưu trạng thái đơn hàng vào file!");
  }

  // 2. Nếu chuyển sang COMPLETED: Xuất hóa đơn (tồn kho ĐÃ trừ lúc CONFIRMED) và cộng điểm khách hàng
  if (needComplete) {
    Order *updatedOrderPtr = orderService->getOrderById(orderId);
    const double netRevenue =
        updatedOrderPtr ? updatedOrderPtr->calculateNetRevenue() : ord.netRevenue;
    const double discAmt =
        updatedOrderPtr ? updatedOrderPtr->getDiscountAmount() : ord.discountAmount;

    if (invoiceService != nullptr &&
        invoiceService->getByOrderId(ord.id) == nullptr) {
      std::string empId = currentSession.isEmployee()
                              ? currentSession.userId
                              : "EMP-000002";
      std::string invErr;
      if (!invoiceService->createInvoiceFromOrder(
              ord.id, empId, ord.paymentMethod, discAmt, invErr)) {
        tx.rollback();
        return OperationResult::Fail(
            "Lỗi xuất hóa đơn khi hoàn thành đơn: " + invErr);
      }
    }

    // Tích điểm cho khách hàng dựa trên doanh thu thuần (trước VAT)
    const int pointsToAdd = static_cast<int>(netRevenue / 100000.0);
    if (pointsToAdd > 0 && !ord.customerId.empty() &&
        customerService != nullptr) {
      std::string pointErr;
      if (!customerService->addPoints(ord.customerId, pointsToAdd, pointErr)) {
        tx.rollback();
        return OperationResult::Fail(
            "Lỗi khi cập nhật điểm tích lũy cho khách hàng: " + pointErr);
      }
    }
  }

  if (!tx.commit()) {
    tx.rollback();
    return OperationResult::Fail(
        "Lỗi khi xác nhận giao dịch ghi file trạng thái đơn hàng!");
  }

  return OperationResult::Ok("Chuyển trạng thái đơn " + orderId + " từ '" +
                             orderStatusToString(ord.status) + "' thành '" +
                             orderStatusToString(newStatus) + "'!");
}

OperationResult AppController::cancelOrder(const std::string &orderId) {
  if (!currentSession.isAuthenticated) {
    return OperationResult::Fail("Từ chối truy cập: Vui lòng đăng nhập!");
  }
  if (!IdGenerator::isValidId(orderId, "ORD", true)) {
    return OperationResult::Fail(
        "Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
  }
  if (orderService == nullptr) {
    return OperationResult::Fail("Dịch vụ đơn hàng chưa sẵn sàng!");
  }

  Order *orderPtr = orderService->getOrderById(orderId);
  if (orderPtr == nullptr) {
    return OperationResult::Fail("Không tìm thấy đơn hàng!");
  }
  const OrderItemViewModel ord = orderToViewModel(*orderPtr, customerService);

  if (currentSession.isCustomer()) {
    if (ord.customerId != currentSession.userId) {
      return OperationResult::Fail(
          "Bạn không có quyền hủy đơn hàng của khách hàng khác!");
    }
    if (!ord.canCancel()) {
      return OperationResult::Fail(
          "Chỉ có thể hủy đơn hàng đang ở trạng thái 'Chờ xử lý' (PENDING)!");
    }
  } else if (currentSession.isAdmin() || currentSession.isSales()) {
    if (ord.status == OrderStatus::COMPLETED) {
      return OperationResult::Fail(
          "Không thể hủy đơn hàng đã hoàn tất giao dịch/giao hàng thành công!");
    }
    if (ord.status == OrderStatus::CANCELLED) {
      return OperationResult::Fail("Đơn hàng đã được hủy trước đó!");
    }
  } else {
    return OperationResult::Fail(
        "Từ chối truy cập: Bạn không có quyền hủy đơn hàng!");
  }
  return updateOrderStatus(orderId, OrderStatus::CANCELLED);
}
