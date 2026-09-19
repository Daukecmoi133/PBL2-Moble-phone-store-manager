#ifndef ORDERREPOSITORY_H
#define ORDERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "../models/Order.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu đơn hàng từ/vào tệp tin
class OrderRepository {
private:
    std::string filePath;
    Vector<Order> orders;

public:
    OrderRepository(std::string path = "data/PAY/Order/Orders.txt");

    bool loadFromFile();
    bool saveToFile() const;

    const Vector<Order>& getAllOrders() const;
    Vector<Order>& getAllOrders();

    Order* findById(const std::string& orderId);
    const Order* findById(const std::string& orderId) const;

    bool addOrder(const Order& order);
    bool updateOrder(const Order& updatedOrder);
    bool deleteOrder(const std::string& id);
};

// ================= IMPLEMENTATION =================

inline OrderRepository::OrderRepository(std::string path) : filePath(path) {}

inline bool OrderRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    orders.clear();
    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        line = StringHelper::trim(line);
        if (line.empty() || line == "...") continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        Vector<std::string> parts = StringHelper::split(line, '|');
        if (parts.getSize() >= 10) {
            std::string id = StringHelper::trim(parts[0]);
            std::string customerId = StringHelper::trim(parts[1]);
            int delivInt = 0;
            int payInt = 0;
            int statusInt = 0;
            try {
                delivInt = std::stoi(StringHelper::trim(parts[2]));
                payInt = std::stoi(StringHelper::trim(parts[3]));
                statusInt = std::stoi(StringHelper::trim(parts[4]));
            } catch (...) {}

            DeliveryMethod deliv = static_cast<DeliveryMethod>(delivInt);
            PaymentMethod pay = static_cast<PaymentMethod>(payInt);
            OrderStatus status = static_cast<OrderStatus>(statusInt);

            DateTime orderTime = DateTime::fromString(StringHelper::trim(parts[5]));
            double discountAmount = StringHelper::parsePrice(parts[6]);
            ShippingInfo ship(StringHelper::trim(parts[7]), 
                              StringHelper::trim(parts[8]), 
                              StringHelper::trim(parts[9]));

            Order order(id, customerId, deliv, pay, ship);
            order.setStatus(status);
            order.setOrderTime(orderTime);
            order.setDiscountAmount(discountAmount);

            if (parts.getSize() >= 11) {
                std::string detailsStr = StringHelper::trim(parts[10]);
                Vector<std::string> itemTokens = StringHelper::split(detailsStr, ';');
                for (int i = 0; i < itemTokens.getSize(); ++i) {
                    Vector<std::string> d = StringHelper::split(itemTokens[i], ',');
                    if (d.getSize() >= 4) {
                        std::string pId = StringHelper::trim(d[0]);
                        std::string pName = StringHelper::trim(d[1]);
                        int qty = 1;
                        double price = 0.0;
                        try {
                            qty = std::stoi(StringHelper::trim(d[2]));
                            price = StringHelper::parsePrice(d[3]);
                        } catch (...) {}
                        order.addDetail(OrderDetail(pId, pName, qty, price));
                    }
                }
            }

            orders.push_back(order);
        }
    }
    file.close();
    return true;
}

inline bool OrderRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details\n";
    for (int i = 0; i < orders.getSize(); ++i) {
        const Order& o = orders[i];
        file << o.getID() << "|"
             << o.getCustomerId() << "|"
             << static_cast<int>(o.getDeliveryMethod()) << "|"
             << static_cast<int>(o.getPaymentMethod()) << "|"
             << static_cast<int>(o.getStatus()) << "|"
             << o.getOrderTime().toString() << "|"
             << o.getDiscountAmount() << "|"
             << o.getShippingInfo().getReceiverName() << "|"
             << o.getShippingInfo().getPhoneNumber() << "|"
             << o.getShippingInfo().getAddress() << "|";

        const Vector<OrderDetail>& d = o.getDetails();
        for (int j = 0; j < d.getSize(); ++j) {
            file << d[j].getPhoneId() << ","
                 << d[j].getPhoneName() << ","
                 << d[j].getQuantity() << ","
                 << d[j].getUnitPrice();
            if (j < d.getSize() - 1) {
                file << ";";
            }
        }
        file << "\n";
    }
    file.close();
    return true;
}

inline const Vector<Order>& OrderRepository::getAllOrders() const {
    return orders;
}

inline Vector<Order>& OrderRepository::getAllOrders() {
    return orders;
}

inline Order* OrderRepository::findById(const std::string& orderId) {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == orderId) {
            return &orders[i];
        }
    }
    return nullptr;
}

inline const Order* OrderRepository::findById(const std::string& orderId) const {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == orderId) {
            return &orders[i];
        }
    }
    return nullptr;
}

inline bool OrderRepository::addOrder(const Order& order) {
    orders.push_back(order);
    return saveToFile();
}

inline bool OrderRepository::updateOrder(const Order& updatedOrder) {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == updatedOrder.getID()) {
            orders[i] = updatedOrder;
            return saveToFile();
        }
    }
    return false;
}

inline bool OrderRepository::deleteOrder(const std::string& id) {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == id) {
            orders.removeAt(i);
            return saveToFile();
        }
    }
    return false;
}

#endif // ORDERREPOSITORY_H
