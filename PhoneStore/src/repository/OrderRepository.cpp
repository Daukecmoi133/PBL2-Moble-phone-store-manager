#include "OrderRepository.h"

OrderRepository::OrderRepository(FileManager& fm) : Repository<Order>(fm, "orders.txt") {}

std::string OrderRepository::getId(const Order& o) const { return o.getID(); }

std::string OrderRepository::serialize(const Order& o) const { return o.toFileString(); }

Result<Order> OrderRepository::deserialize(const std::string& line) const {
    try {
        Order o = Order::fromFileLine(line);
        if (o.getID().empty()) return Result<Order>::fail(ErrorCode::ParseError, "Dòng đơn hàng không hợp lệ");
        return Result<Order>::ok(o);
    } catch (...) {
        return Result<Order>::fail(ErrorCode::ParseError, "Không phân tích được dòng đơn hàng");
    }
}

void OrderRepository::rebuildIndexes() {
    // TODO(logic): dựng index theo khách hàng và gọi rebuildPendingQueue().
}

Result<Vector<Order>> OrderRepository::findByCustomer(const std::string&) const {
    // TODO(logic): lọc các đơn của khách, trả bản sao.
    return Result<Vector<Order>>::fail(ErrorCode::NotImplemented, "TODO: OrderRepository::findByCustomer");
}

Result<Vector<Order>> OrderRepository::findByStatus(OrderStatus) const {
    // TODO(logic): lọc đơn theo trạng thái, trả bản sao.
    return Result<Vector<Order>>::fail(ErrorCode::NotImplemented, "TODO: OrderRepository::findByStatus");
}

Result<Order> OrderRepository::peekNextPending() const {
    // TODO(logic): lấy ID từ pendingQueue, đọc lại trạng thái, bỏ qua đơn không còn PENDING.
    return Result<Order>::fail(ErrorCode::NotImplemented, "TODO: OrderRepository::peekNextPending");
}

void OrderRepository::rebuildPendingQueue() {
    // TODO(logic): xóa pendingQueue rồi đẩy ID các đơn PENDING theo thứ tự thời gian đặt.
}
