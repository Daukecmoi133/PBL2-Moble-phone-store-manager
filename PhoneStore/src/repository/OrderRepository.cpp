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
    rebuildPendingQueue();
}

Result<Vector<Order>> OrderRepository::findByCustomer(const std::string& customerId) const {
    Vector<Order> result;
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getCustomerId() == customerId) {
            result.push_back(items[i]);
        }
    }
    return Result<Vector<Order>>::ok(result);
}

Result<Vector<Order>> OrderRepository::findByStatus(OrderStatus status) const {
    Vector<Order> result;
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getStatus() == status) {
            result.push_back(items[i]);
        }
    }
    return Result<Vector<Order>>::ok(result);
}

Result<Order> OrderRepository::peekNextPending() const {
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getStatus() == OrderStatus::PENDING) {
            return Result<Order>::ok(items[i]);
        }
    }
    return Result<Order>::fail(ErrorCode::NotFound, "Không có đơn hàng PENDING nào");
}

void OrderRepository::rebuildPendingQueue() {
    pendingQueue.clear();
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getStatus() == OrderStatus::PENDING) {
            pendingQueue.push(items[i].getID());
        }
    }
}
