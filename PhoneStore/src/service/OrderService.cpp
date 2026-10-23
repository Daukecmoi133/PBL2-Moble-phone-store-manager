#include "OrderService.h"

OrderService::OrderService(OrderRepository& orders, PhoneService& phones, CustomerService& customers,
                           DiscountService& discounts)
    : orderRepo(orders), phoneService(phones), customerService(customers), discountService(discounts) {}

Result<std::string> OrderService::createOrder(const Actor&, const OrderDraft&) {
    // TODO(logic): quyền CreateOrder; checkAvailable từng dòng; UnitOfWork: reserve + consumeUsage (nếu có mã),
    // lưu đơn PENDING với inventoryState = RESERVED; commit.
    return Result<std::string>::fail(ErrorCode::NotImplemented, "TODO: OrderService::createOrder");
}

Result<void> OrderService::confirmOrder(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine PENDING->CONFIRMED; commitReserved; inventoryState = DEDUCTED; commit.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::confirmOrder");
}

Result<void> OrderService::startPreparing(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine CONFIRMED->PREPARING.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::startPreparing");
}

Result<void> OrderService::markReadyForPickup(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine PREPARING->READY_FOR_PICKUP (chỉ luồng PICKUP).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::markReadyForPickup");
}

Result<void> OrderService::markShipping(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine PREPARING->SHIPPING (chỉ luồng DELIVERY).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::markShipping");
}

Result<void> OrderService::takePayment(const Actor&, const std::string&, PaymentMethod) {
    // TODO(logic): quyền TakePayment; chuyển paymentStatus UNPAID -> PAID.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::takePayment");
}

Result<void> OrderService::completeOrder(const Actor&, const std::string&) {
    // TODO(logic): yêu cầu PAID; OrderStateMachine READY_FOR_PICKUP/SHIPPING -> COMPLETED.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::completeOrder");
}

Result<void> OrderService::cancelOrder(const Actor&, const std::string&) {
    // TODO(logic): PENDING: releaseReserved + releaseUsage, RELEASED; CONFIRMED: restock, RESTOCKED;
    // từ PREPARING trở đi không cho hủy; mỗi lượng hàng chỉ giải phóng/hoàn đúng một lần theo inventoryState.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::cancelOrder");
}

Result<std::string> OrderService::createCounterSale(const Actor&, const OrderDraft&, PaymentMethod) {
    // TODO(logic): quyền CreateCounterSale; tạo đơn + thu tiền + lập hóa đơn trong một UnitOfWork
    // (mặc định tạm: trừ kho thẳng, không giữ chỗ — xem docs/OPEN_QUESTIONS.md).
    return Result<std::string>::fail(ErrorCode::NotImplemented, "TODO: OrderService::createCounterSale");
}

Result<Vector<OrderSummary>> OrderService::listForCustomer(const Actor&) const {
    // TODO(logic): chỉ trả đơn của chính khách (OrderRepository::findByCustomer).
    return Result<Vector<OrderSummary>>::fail(ErrorCode::NotImplemented, "TODO: OrderService::listForCustomer");
}

Result<Vector<OrderSummary>> OrderService::listByStatus(const Actor&, OrderStatus) const {
    // TODO(logic): kiểm tra quyền theo vai trò (điểm mở: kho xem tất cả đơn hay chỉ từ CONFIRMED trở đi).
    return Result<Vector<OrderSummary>>::fail(ErrorCode::NotImplemented, "TODO: OrderService::listByStatus");
}

Result<OrderSummary> OrderService::getOrder(const Actor&, const std::string&) const {
    // TODO(logic): kiểm tra quyền xem đơn (khách chỉ xem đơn của mình).
    return Result<OrderSummary>::fail(ErrorCode::NotImplemented, "TODO: OrderService::getOrder");
}
