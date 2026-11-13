#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === ĐƠN HÀNG ===

void AppController::autoCompleteDeliveredOrders() {
  auto &store = MockDataStore::getInstance();
  std::time_t now = std::time(nullptr);
  std::vector<std::string> toComplete;

  for (const auto &ord : store.orders) {
    if (ord.status == OrderStatus::SHIPPING) {
      long long ordEpoch =
          RealTimeManager::dateTimeToEpochSeconds(ord.orderTime);
      if (ordEpoch > 0 && (now - ordEpoch) >= (2 * 24 * 3600)) {
        toComplete.push_back(ord.id);
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
  autoCompleteDeliveredOrders();
  auto &store = MockDataStore::getInstance();
  if (currentSession.isAuthenticated && currentSession.isCustomer()) {
    return getCustomerOrders(currentSession.userId);
  }
  return store.orders;
}

std::vector<OrderItemViewModel>
AppController::filterOrders(const std::string &keyword, int statusFilter,
                            int paymentFilter, int typeFilter) {
  autoCompleteDeliveredOrders();
  auto &store = MockDataStore::getInstance();
  std::vector<OrderItemViewModel> result;

  for (const auto &ord : store.orders) {
    // Customer session ownership check
    if (currentSession.isAuthenticated && currentSession.isCustomer() &&
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
  autoCompleteDeliveredOrders();
  auto &store = MockDataStore::getInstance();
  std::vector<OrderItemViewModel> result;
  std::string targetId = customerId;
  if (currentSession.isAuthenticated && currentSession.isCustomer()) {
    targetId = currentSession.userId;
  }

  for (const auto &ord : store.orders) {
    if (!targetId.empty()) {
      if (ord.customerId == targetId) {
        result.push_back(ord);
      }
    } else if (!currentSession.isAuthenticated || currentSession.isCustomer()) {
      // Customer or unauthenticated with empty ID cannot view all orders
      continue;
    } else {
      // Admin or Sales can view all orders
      result.push_back(ord);
    }
  }
  return result;
}

OrderItemViewModel AppController::getOrderById(const std::string &orderId) {
  auto &store = MockDataStore::getInstance();
  for (const auto &ord : store.orders) {
    if (ord.id == orderId) {
      if (currentSession.isAuthenticated && currentSession.isCustomer() &&
          ord.customerId != currentSession.userId) {
        return OrderItemViewModel();
      }
      return ord;
    }
  }
  return OrderItemViewModel();
}

OperationResult AppController::updateOrderStatus(const std::string &orderId,
                                                 OrderStatus newStatus) {
  if (!IdGenerator::isValidId(orderId, "ORD", true)) {
    return OperationResult::Fail(
        "Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
  }
  auto &store = MockDataStore::getInstance();
  for (auto &ord : store.orders) {
    if (ord.id == orderId) {
      // Ràng buộc phân quyền (ANTY.md mục 4)
      if (currentSession.isAuthenticated && currentSession.isCustomer()) {
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
      } else if (currentSession.isAuthenticated &&
                 currentSession.isWarehouse()) {
        // Nhân viên kho: Chuẩn bị hàng, sẵn sàng nhận, bàn giao ship
        if (newStatus != OrderStatus::PREPARING &&
            newStatus != OrderStatus::READY_FOR_PICKUP &&
            newStatus != OrderStatus::SHIPPING) {
          return OperationResult::Fail("Nhân viên kho chỉ có quyền cập nhật "
                                       "trạng thái chuẩn bị hàng và xuất kho!");
        }
      } else if (currentSession.isAuthenticated && !currentSession.isAdmin() &&
                 !currentSession.isSales()) {
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

      // Khởi tạo FileTransaction để đồng bộ lưu trữ bền vững
      FileTransaction tx;
      if (orderService != nullptr)
        tx.addParticipant(orderService->getRepository().getFilePath());
      if (newStatus == OrderStatus::COMPLETED) {
        if (phoneService != nullptr)
          tx.addParticipant(phoneService->getRepository().getFilePath());
        if (customerService != nullptr && !ord.customerId.empty()) {
          tx.addParticipant(customerService->getRepository().getFilePath());
        }
        if (invoiceService != nullptr) {
          tx.addParticipant(invoiceService->getRepository().getFilePath());
        }
      }
      tx.onRollback([this]() {
        if (this->orderService)
          this->orderService->reload();
        if (this->phoneService)
          this->phoneService->reload();
        if (this->customerService)
          this->customerService->reload();
        if (this->invoiceService)
          this->invoiceService->reload();
      });

      if (!tx.begin()) {
        return OperationResult::Fail(
            "Không thể khởi tạo giao dịch lưu trữ trạng thái đơn hàng!");
      }

      // 1. Cập nhật trạng thái đơn hàng trong file qua OrderService trước
      if (orderService != nullptr) {
        std::string serviceErr;
        if (!orderService->updateOrderStatus(orderId, newStatus, serviceErr)) {
          tx.rollback();
          return OperationResult::Fail(
              !serviceErr.empty()
                  ? serviceErr
                  : "Lỗi khi lưu trạng thái đơn hàng vào file!");
        }
      }

      // 2. Nếu chuyển sang COMPLETED: Xuất hóa đơn, trừ kho qua PhoneService và
      // cộng điểm qua CustomerService
      int pointsToAdd = 0;
      InvoiceItemViewModel newInv;
      bool createdInvoice = false;

      if (newStatus == OrderStatus::COMPLETED) {
        // Xuất hóa đơn nếu đơn hàng chưa có hóa đơn
        bool hasInvoice = false;
        for (const auto &invoice : store.invoices) {
          if (invoice.orderId == ord.id) {
            hasInvoice = true;
            break;
          }
        }

        if (!hasInvoice && invoiceService != nullptr) {
          std::string empId = currentSession.isEmployee()
                                  ? currentSession.userId
                                  : "EMP-000002";
          std::string invErr;
          if (!invoiceService->createInvoiceFromOrder(
                  ord.id, empId, ord.paymentMethod, ord.discountAmount,
                  invErr)) {
            tx.rollback();
            return OperationResult::Fail(
                "Lỗi xuất hóa đơn khi hoàn thành đơn: " + invErr);
          }
          Invoice *invModel = invoiceService->getByOrderId(ord.id);
          if (invModel != nullptr) {
            std::time_t now = std::time(nullptr);
            std::tm *tm_now = std::localtime(&now);
            char timeBuf[64];
            std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

            newInv.id = invModel->getID();
            newInv.orderId = ord.id;
            newInv.customerId = ord.customerId;
            newInv.customerName = ord.customerName;
            newInv.employeeId = empId;
            newInv.employeeName = "Nhân viên xử lý đơn";
            newInv.paymentMethod = ord.paymentMethod;
            newInv.paymentMethodText = paymentMethodToString(ord.paymentMethod);
            newInv.createTime = timeBuf;
            newInv.subtotal = ord.subtotal;
            newInv.discountAmount = ord.discountAmount;
            newInv.total = ord.total;
            newInv.details = ord.details;
            createdInvoice = true;
          }
        }

        // Trừ kho vật lý
        if (phoneService != nullptr) {
          for (const auto &d : ord.details) {
            std::string stockErr;
            if (!phoneService->updateStock(d.phoneId, -d.quantity, stockErr)) {
              tx.rollback();
              return OperationResult::Fail("Lỗi trừ tồn kho cho sản phẩm '" +
                                           d.phoneName + "': " + stockErr);
            }
          }
        }

        // Tích điểm cho khách hàng
        pointsToAdd = static_cast<int>(ord.total / 100000.0);
        if (pointsToAdd > 0 && !ord.customerId.empty() &&
            customerService != nullptr) {
          std::string pointErr;
          if (!customerService->addPoints(ord.customerId, pointsToAdd,
                                          pointErr)) {
            tx.rollback();
            return OperationResult::Fail(
                "Lỗi khi cập nhật điểm tích lũy cho khách hàng: " + pointErr);
          }
        }
      }

      if (!tx.commit()) {
        return OperationResult::Fail(
            "Lỗi khi xác nhận giao dịch ghi file trạng thái đơn hàng!");
      }

      // Giao dịch đĩa thành công 100% -> Cập nhật RAM View cache
      OrderStatus oldStatus = ord.status;
      ord.status = newStatus;
      ord.statusText = orderStatusToString(newStatus);

      if (newStatus == OrderStatus::COMPLETED) {
        if (createdInvoice) {
          store.invoices.insert(store.invoices.begin(), newInv);
        }

        // Trừ tồn kho thực tế, giải phóng giữ chỗ trên view cache
        for (const auto &d : ord.details) {
          for (auto &p : store.products) {
            if (p.id == d.phoneId) {
              p.actualStock -= d.quantity;
              p.reservedStock -= d.quantity;
              if (p.actualStock < 0)
                p.actualStock = 0;
              if (p.reservedStock < 0)
                p.reservedStock = 0;
              p.calculateStockStatus();
              break;
            }
          }
        }
        ord.paymentStatus = PaymentStatus::PAID;
        ord.paymentStatusText = paymentStatusToString(ord.paymentStatus);

        // Cập nhật điểm tích lũy trên view cache
        if (pointsToAdd > 0) {
          for (auto &c : store.customers) {
            if (c.id == ord.customerId) {
              c.points += pointsToAdd;
              c.updateTier();
              break;
            }
          }
        }
      } else if (newStatus == OrderStatus::CANCELLED) {
        // Giải phóng giữ chỗ trên view cache
        for (const auto &d : ord.details) {
          for (auto &p : store.products) {
            if (p.id == d.phoneId) {
              p.reservedStock -= d.quantity;
              if (p.reservedStock < 0)
                p.reservedStock = 0;
              p.calculateStockStatus();
              break;
            }
          }
        }
      }

      return OperationResult::Ok("Chuyển trạng thái đơn " + orderId + " từ '" +
                                 orderStatusToString(oldStatus) + "' thành '" +
                                 orderStatusToString(newStatus) + "'!");
    }
  }
  return OperationResult::Fail("Không tìm thấy đơn hàng " + orderId);
}

OperationResult AppController::cancelOrder(const std::string &orderId) {
  if (!IdGenerator::isValidId(orderId, "ORD", true)) {
    return OperationResult::Fail(
        "Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
  }
  auto &store = MockDataStore::getInstance();
  for (const auto &ord : store.orders) {
    if (ord.id == orderId) {
      if (currentSession.isAuthenticated && currentSession.isCustomer() &&
          ord.customerId != currentSession.userId) {
        return OperationResult::Fail(
            "Bạn không có quyền hủy đơn hàng của khách hàng khác!");
      }
      if (!ord.canCancel()) {
        return OperationResult::Fail(
            "Chỉ có thể hủy đơn hàng đang ở trạng thái 'Chờ xử lý' (PENDING)!");
      }
      return updateOrderStatus(orderId, OrderStatus::CANCELLED);
    }
  }
  return OperationResult::Fail("Không tìm thấy đơn hàng!");
}
