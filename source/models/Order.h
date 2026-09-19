#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "Common.h"
#include "Phone.h"
#include "../utils/DateTime.h"
#include "../core/Vector.h"

class ShippingInfo {
private:
    std::string receiverName;
    std::string phoneNumber;
    std::string address;

public:
    ShippingInfo();
    ShippingInfo(std::string name, std::string phone, std::string addr);

    std::string getReceiverName() const;
    std::string getPhoneNumber() const;
    std::string getAddress() const;

    void setReceiverName(const std::string& name);
    void setPhoneNumber(const std::string& phone);
    void setAddress(const std::string& addr);
};

class OrderDetail {
private:
    std::string phoneId;
    std::string phoneName;
    int quantity;
    double unitPrice;

public:
    OrderDetail();
    OrderDetail(std::string id, std::string name, int qty, double price);

    std::string getPhoneId() const;
    std::string getPhoneName() const;
    int getQuantity() const;
    double getUnitPrice() const;

    void setQuantity(int q);
    void setUnitPrice(double p);

    double getSubtotal() const;
};

class Order {
private:
    std::string id;
    std::string customerId;
    DeliveryMethod deliveryMethod;
    PaymentMethod paymentMethod;
    Vector<OrderDetail> details;
    ShippingInfo shippingInfo;
    OrderStatus status;
    DateTime orderTime;
    double discountAmount;

public:
    Order();
    Order(std::string id, std::string customerId, DeliveryMethod deliv, 
          PaymentMethod pay, ShippingInfo ship);

    std::string getID() const;
    std::string getCustomerId() const;
    DeliveryMethod getDeliveryMethod() const;
    PaymentMethod getPaymentMethod() const;
    const Vector<OrderDetail>& getDetails() const;
    Vector<OrderDetail>& getDetails();
    ShippingInfo getShippingInfo() const;
    OrderStatus getStatus() const;
    DateTime getOrderTime() const;
    double getDiscountAmount() const;

    void setID(const std::string& orderId);
    void setCustomerId(const std::string& custId);
    void setStatus(OrderStatus s);
    void setDiscountAmount(double d);
    void setOrderTime(const DateTime& dt);

    void addDetail(const OrderDetail& detail);
    double calculateSubtotal() const;
    double calculateTotal() const;
    void confirm();
    void cancel();
};

// ================= IMPLEMENTATION SHIPPINGINFO =================

inline ShippingInfo::ShippingInfo() : receiverName(""), phoneNumber(""), address("") {}

inline ShippingInfo::ShippingInfo(std::string name, std::string phone, std::string addr)
    : receiverName(name), phoneNumber(phone), address(addr) {}

inline std::string ShippingInfo::getReceiverName() const { return receiverName; }
inline std::string ShippingInfo::getPhoneNumber() const { return phoneNumber; }
inline std::string ShippingInfo::getAddress() const { return address; }

inline void ShippingInfo::setReceiverName(const std::string& name) { receiverName = name; }
inline void ShippingInfo::setPhoneNumber(const std::string& phone) { phoneNumber = phone; }
inline void ShippingInfo::setAddress(const std::string& addr) { address = addr; }

// ================= IMPLEMENTATION ORDERDETAIL =================

inline OrderDetail::OrderDetail() : phoneId(""), phoneName(""), quantity(0), unitPrice(0.0) {}

inline OrderDetail::OrderDetail(std::string id, std::string name, int qty, double price)
    : phoneId(id), phoneName(name), quantity(qty), unitPrice(price) {}

inline std::string OrderDetail::getPhoneId() const { return phoneId; }
inline std::string OrderDetail::getPhoneName() const { return phoneName; }
inline int OrderDetail::getQuantity() const { return quantity; }
inline double OrderDetail::getUnitPrice() const { return unitPrice; }

inline void OrderDetail::setQuantity(int q) { quantity = q; }
inline void OrderDetail::setUnitPrice(double p) { unitPrice = p; }

inline double OrderDetail::getSubtotal() const {
    return unitPrice * quantity;
}

// ================= IMPLEMENTATION ORDER =================

inline Order::Order() 
    : id(""), customerId(""), deliveryMethod(DeliveryMethod::PICKUP),
      paymentMethod(PaymentMethod::CASH), status(OrderStatus::PENDING),
      orderTime(), discountAmount(0.0) {}

inline Order::Order(std::string id, std::string customerId, DeliveryMethod deliv, 
                    PaymentMethod pay, ShippingInfo ship)
    : id(id), customerId(customerId), deliveryMethod(deliv),
      paymentMethod(pay), shippingInfo(ship),
      status(OrderStatus::PENDING), orderTime(DateTime::now()),
      discountAmount(0.0) {}

inline std::string Order::getID() const { return id; }
inline std::string Order::getCustomerId() const { return customerId; }
inline DeliveryMethod Order::getDeliveryMethod() const { return deliveryMethod; }
inline PaymentMethod Order::getPaymentMethod() const { return paymentMethod; }
inline const Vector<OrderDetail>& Order::getDetails() const { return details; }
inline Vector<OrderDetail>& Order::getDetails() { return details; }
inline ShippingInfo Order::getShippingInfo() const { return shippingInfo; }
inline OrderStatus Order::getStatus() const { return status; }
inline DateTime Order::getOrderTime() const { return orderTime; }
inline double Order::getDiscountAmount() const { return discountAmount; }

inline void Order::setID(const std::string& orderId) { id = orderId; }
inline void Order::setCustomerId(const std::string& custId) { customerId = custId; }
inline void Order::setStatus(OrderStatus s) { status = s; }
inline void Order::setDiscountAmount(double d) { discountAmount = d; }
inline void Order::setOrderTime(const DateTime& dt) { orderTime = dt; }

inline void Order::addDetail(const OrderDetail& detail) {
    details.push_back(detail);
}

inline double Order::calculateSubtotal() const {
    double subtotal = 0.0;
    for (int i = 0; i < details.getSize(); ++i) {
        subtotal += details[i].getSubtotal();
    }
    return subtotal;
}

inline double Order::calculateTotal() const {
    double total = calculateSubtotal() - discountAmount;
    return total > 0.0 ? total : 0.0;
}

inline void Order::confirm() {
    status = OrderStatus::CONFIRMED;
}

inline void Order::cancel() {
    status = OrderStatus::CANCELLED;
}

#endif // ORDER_H
