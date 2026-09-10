#ifndef ORDERREPOSITORY_H
#define ORDERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Order.h"
#include "../core/Vector.h"
#include "../core/Queue.h"

class OrderRepository {
private:
    std::string directoryPath;
    Vector<Order> orders;
    Queue<Order> pendingQueue; // Hàng đợi FIFO xử lý đơn hàng chờ duyệt

public:
    OrderRepository(std::string dir = "data/PAY/Order/") : directoryPath(dir) {}

    void addOrder(const Order& order) {
        orders.push_back(order);
        if (order.getStatus() == OrderStatus::PENDING) {
            pendingQueue.push(order);
        }
    }

    const Vector<Order>& getAllOrders() const {
        return orders;
    }

    Queue<Order>& getPendingQueue() {
        return pendingQueue;
    }

    Vector<Order> getOrdersByCustomer(const std::string& customerId) const {
        Vector<Order> result;
        for (int i = 0; i < orders.getSize(); ++i) {
            if (orders[i].getCustomerId() == customerId) {
                result.push_back(orders[i]);
            }
        }
        return result;
    }

    Order* findById(const std::string& orderId) {
        for (int i = 0; i < orders.getSize(); ++i) {
            if (orders[i].getID() == orderId) {
                return &orders[i];
            }
        }
        return nullptr;
    }

    bool processNextPendingOrder(Order& processedOrder) {
        if (pendingQueue.isEmpty()) return false;
        processedOrder = pendingQueue.front();
        pendingQueue.pop();

        // Cập nhật trạng thái trong danh sách chính
        Order* ord = findById(processedOrder.getID());
        if (ord) {
            ord->setStatus(OrderStatus::CONFIRMED);
            processedOrder.setStatus(OrderStatus::CONFIRMED);
        }
        return true;
    }
};

#endif // ORDERREPOSITORY_H
