#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === ĐƠN HÀNG ===

std::vector<OrderItemViewModel> AppController::getAllOrders() {
    return MockDataStore::getInstance().orders;
}

std::vector<OrderItemViewModel> AppController::filterOrders(const std::string& keyword, int statusFilter, int paymentFilter, int typeFilter) {
    auto& store = MockDataStore::getInstance();
    std::vector<OrderItemViewModel> result;

    for (const auto& ord : store.orders) {
        // Keyword filter: check id, customerName, receiverPhone
        if (!keyword.empty()) {
            bool match = Utf8Helper::containsCaseInsensitive(ord.id, keyword) ||
                         Utf8Helper::containsCaseInsensitive(ord.customerName, keyword) ||
                         Utf8Helper::containsCaseInsensitive(ord.receiverPhone, keyword);
            if (!match) continue;
        }

        // Status filter:
        if (statusFilter >= 0) {
            if (static_cast<int>(ord.status) != statusFilter) continue;
        }

        // Payment filter:
        if (paymentFilter >= 0) {
            if (static_cast<int>(ord.paymentStatus) != paymentFilter) continue;
        }

        // Type filter: 0 = In store (Tại quầy), 1 = Online
        if (typeFilter >= 0) {
            if (static_cast<int>(ord.deliveryMethod) != typeFilter) continue;
        }

        result.push_back(ord);
    }
    return result;
}

std::vector<OrderItemViewModel> AppController::getCustomerOrders(const std::string& customerId) {
    auto& store = MockDataStore::getInstance();
    std::vector<OrderItemViewModel> result;
    for (const auto& ord : store.orders) {
        if (ord.customerId == customerId || customerId.empty()) {
            result.push_back(ord);
        }
    }
    return result;
}

OrderItemViewModel AppController::getOrderById(const std::string& orderId) {
    auto& store = MockDataStore::getInstance();
    for (const auto& ord : store.orders) {
        if (ord.id == orderId) return ord;
    }
    return OrderItemViewModel();
}

OperationResult AppController::updateOrderStatus(const std::string& orderId, OrderStatus newStatus) {
    if (!IdGenerator::isValidId(orderId, "ORD", true)) {
        return OperationResult::Fail("Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (auto& ord : store.orders) {
        if (ord.id == orderId) {
            // Không cho phép sửa nếu đơn đã Hoàn thành hoặc Hủy
            if (ord.status == OrderStatus::COMPLETED) {
                return OperationResult::Fail("Đơn hàng đã hoàn thành, không thể thay đổi trạng thái!");
            }
            if (ord.status == OrderStatus::CANCELLED) {
                return OperationResult::Fail("Đơn hàng đã bị hủy, không thể thay đổi trạng thái!");
            }

            if (newStatus == OrderStatus::COMPLETED) {
                bool hasInvoice = false;
                for (const auto& invoice : store.invoices) {
                    if (invoice.orderId == ord.id) {
                        hasInvoice = true;
                        break;
                    }
                }
                if (!hasInvoice) {
                    OperationResult invoiceResult =
                        createInvoiceForOrder(ord.id, "EMP-000002", ord.paymentMethod);
                    if (!invoiceResult.success) return invoiceResult;
                }
            }

            // Lưu trạng thái đơn hàng xuống file qua OrderService
            if (orderService != nullptr) {
                std::string serviceErr;
                if (!orderService->updateOrderStatus(orderId, newStatus, serviceErr)) {
                    return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu trạng thái đơn hàng vào file!");
                }
            }

            OrderStatus oldStatus = ord.status;
            ord.status = newStatus;
            ord.statusText = orderStatusToString(newStatus);

            // Xử lý trừ kho hoặc giải phóng giữ chỗ theo quy tắc nghiệp vụ:
            if (newStatus == OrderStatus::COMPLETED) {
                // Trừ tồn kho thực tế, giải phóng giữ chỗ
                for (const auto& d : ord.details) {
                    for (auto& p : store.products) {
                        if (p.id == d.phoneId) {
                            p.actualStock -= d.quantity;
                            p.reservedStock -= d.quantity;
                            if (p.actualStock < 0) p.actualStock = 0;
                            if (p.reservedStock < 0) p.reservedStock = 0;
                            p.calculateStockStatus();
                            break;
                        }
                    }
                }
                ord.paymentStatus = PaymentStatus::PAID;
                ord.paymentStatusText = paymentStatusToString(ord.paymentStatus);

                // Cộng điểm tích lũy cho khách hàng (100.000 đ = 1 điểm)
                int pointsToAdd = static_cast<int>(ord.total / 100000.0);
                for (auto& c : store.customers) {
                    if (c.id == ord.customerId) {
                        c.points += pointsToAdd;
                        c.updateTier();
                        break;
                    }
                }
            } else if (newStatus == OrderStatus::CANCELLED) {
                // Giải phóng giữ chỗ
                for (const auto& d : ord.details) {
                    for (auto& p : store.products) {
                        if (p.id == d.phoneId) {
                            p.reservedStock -= d.quantity;
                            if (p.reservedStock < 0) p.reservedStock = 0;
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

OperationResult AppController::cancelOrder(const std::string& orderId) {
    if (!IdGenerator::isValidId(orderId, "ORD", true)) {
        return OperationResult::Fail("Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (const auto& ord : store.orders) {
        if (ord.id == orderId) {
            if (!ord.canCancel()) {
                return OperationResult::Fail("Chỉ có thể hủy đơn hàng đang ở trạng thái 'Chờ xử lý' (PENDING)!");
            }
            if (orderService != nullptr) {
                std::string serviceErr;
                if (!orderService->cancelOrder(orderId, serviceErr)) {
                    return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi hủy đơn hàng trong file!");
                }
            }
            return updateOrderStatus(orderId, OrderStatus::CANCELLED);
        }
    }
    return OperationResult::Fail("Không tìm thấy đơn hàng!");
}
