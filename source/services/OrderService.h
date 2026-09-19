#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H

#include <iostream>
#include <string>
#include "../models/Order.h"
#include "../core/Vector.h"
#include "../core/Queue.h"
#include "../repositories/OrderRepository.h"
#include "PhoneService.h"

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ đơn hàng,
// hàng đợi duyệt đơn FIFO (Queue) và liên kết trừ tồn kho sản phẩm.
class OrderService {
private:
    OrderRepository& orderRepo;
    PhoneService* phoneService;
    Queue<Order> pendingQueue; // Hàng đợi FIFO xử lý đơn hàng chờ duyệt

    void loadPendingQueue();

public:
    OrderService(OrderRepository& oRepo, PhoneService* pService = nullptr);

    void setPhoneService(PhoneService* pService);
    Queue<Order>& getPendingQueue();
    const Vector<Order>& getAllOrders() const;

    Order* getOrderById(const std::string& orderId);
    Vector<Order> getOrdersByCustomer(const std::string& customerId) const;

    // Nghiệp vụ tạo đơn hàng: Kiểm tra tồn kho trước khi tạo
    bool createOrder(const Order& order, std::string& errorMessage);
    bool createOrder(const Order& order);

    // Nghiệp vụ duyệt đơn hàng tiếp theo từ hàng đợi FIFO
    bool processNextPendingOrder(Order& processedOrder, std::string& errorMessage);
    bool processNextPendingOrder(Order& processedOrder);

    // Nghiệp vụ hủy đơn hàng
    bool cancelOrder(const std::string& orderId, std::string& errorMessage);

    // Nghiệp vụ tính tổng doanh thu từ các đơn đã xác nhận/hoàn thành
    double calculateTotalRevenue() const;
};

// ================= IMPLEMENTATION =================

inline void OrderService::loadPendingQueue() {
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == OrderStatus::PENDING) {
            pendingQueue.push(orders[i]);
        }
    }
}

inline OrderService::OrderService(OrderRepository& oRepo, PhoneService* pService)
    : orderRepo(oRepo), phoneService(pService) {
    loadPendingQueue();
}

inline void OrderService::setPhoneService(PhoneService* pService) {
    phoneService = pService;
}

inline Queue<Order>& OrderService::getPendingQueue() {
    return pendingQueue;
}

inline const Vector<Order>& OrderService::getAllOrders() const {
    return orderRepo.getAllOrders();
}

inline Order* OrderService::getOrderById(const std::string& orderId) {
    return orderRepo.findById(orderId);
}

inline Vector<Order> OrderService::getOrdersByCustomer(const std::string& customerId) const {
    Vector<Order> result;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getCustomerId() == customerId) {
            result.push_back(orders[i]);
        }
    }
    return result;
}

inline bool OrderService::createOrder(const Order& order, std::string& errorMessage) {
    if (order.getID().empty()) {
        errorMessage = "Mã đơn hàng không hợp lệ!";
        return false;
    }

    if (order.getDetails().getSize() == 0) {
        errorMessage = "Đơn hàng phải có ít nhất một sản phẩm!";
        return false;
    }

    // Kiểm tra số lượng tồn kho từng sản phẩm nếu có liên kết PhoneService
    if (phoneService != nullptr) {
        const Vector<OrderDetail>& details = order.getDetails();
        for (int i = 0; i < details.getSize(); ++i) {
            if (!phoneService->hasStock(details[i].getPhoneId(), details[i].getQuantity())) {
                errorMessage = "Sản phẩm [" + details[i].getPhoneName() + "] không đủ số lượng tồn kho!";
                return false;
            }
        }
    }

    if (!orderRepo.addOrder(order)) {
        errorMessage = "Không thể lưu đơn hàng vào hệ thống!";
        return false;
    }

    if (order.getStatus() == OrderStatus::PENDING) {
        pendingQueue.push(order);
    }

    return true;
}

inline bool OrderService::createOrder(const Order& order) {
    std::string err;
    return createOrder(order, err);
}

inline bool OrderService::processNextPendingOrder(Order& processedOrder, std::string& errorMessage) {
    if (pendingQueue.isEmpty()) {
        errorMessage = "Hàng đợi rỗng! Không có đơn hàng nào đang chờ duyệt.";
        return false;
    }

    processedOrder = pendingQueue.front();
    pendingQueue.pop();

    // Cập nhật trạng thái trong kho dữ liệu
    Order* ord = orderRepo.findById(processedOrder.getID());
    if (ord != nullptr) {
        ord->setStatus(OrderStatus::CONFIRMED);
        processedOrder.setStatus(OrderStatus::CONFIRMED);

        // Tự động trừ số lượng tồn kho sản phẩm khi đơn hàng được duyệt
        if (phoneService != nullptr) {
            const Vector<OrderDetail>& details = ord->getDetails();
            for (int i = 0; i < details.getSize(); ++i) {
                phoneService->updateStock(details[i].getPhoneId(), -details[i].getQuantity());
            }
        }

        orderRepo.updateOrder(*ord);
        return true;
    }

    errorMessage = "Không tìm thấy đơn hàng trong cơ sở dữ liệu!";
    return false;
}

inline bool OrderService::processNextPendingOrder(Order& processedOrder) {
    std::string err;
    return processNextPendingOrder(processedOrder, err);
}

inline bool OrderService::cancelOrder(const std::string& orderId, std::string& errorMessage) {
    Order* ord = orderRepo.findById(orderId);
    if (ord == nullptr) {
        errorMessage = "Không tìm thấy đơn hàng [" + orderId + "]!";
        return false;
    }

    if (ord->getStatus() == OrderStatus::CANCELLED) {
        errorMessage = "Đơn hàng đã được hủy trước đó!";
        return false;
    }

    // Nếu đơn đã xác nhận (đã trừ kho), khi hủy cần hoàn lại số lượng tồn kho
    if (ord->getStatus() == OrderStatus::CONFIRMED && phoneService != nullptr) {
        const Vector<OrderDetail>& details = ord->getDetails();
        for (int i = 0; i < details.getSize(); ++i) {
            phoneService->updateStock(details[i].getPhoneId(), details[i].getQuantity());
        }
    }

    ord->setStatus(OrderStatus::CANCELLED);
    orderRepo.updateOrder(*ord);
    return true;
}

inline double OrderService::calculateTotalRevenue() const {
    double revenue = 0.0;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == OrderStatus::CONFIRMED || 
            orders[i].getStatus() == OrderStatus::COMPLETED) {
            revenue += orders[i].calculateTotal();
        }
    }
    return revenue;
}

#endif // ORDERSERVICE_H
