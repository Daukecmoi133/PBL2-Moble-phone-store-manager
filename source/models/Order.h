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
    ShippingInfo() : receiverName(""), phoneNumber(""), address("") {}

    ShippingInfo(std::string name, std::string phone, std::string addr)
        : receiverName(name), phoneNumber(phone), address(addr) {}

    std::string getReceiverName() const { return receiverName; }
    std::string getPhoneNumber() const { return phoneNumber; }
    std::string getAddress() const { return address; }

    void setReceiverName(const std::string& name) { receiverName = name; }
    void setPhoneNumber(const std::string& phone) { phoneNumber = phone; }
    void setAddress(const std::string& addr) { address = addr; }
};

class OrderDetail {
private:
    std::string phoneId;
    std::string phoneName;
    int quantity;
    double unitPrice;

public:
    OrderDetail() : phoneId(""), phoneName(""), quantity(0), unitPrice(0.0) {}

    OrderDetail(std::string id, std::string name, int qty, double price)
        : phoneId(id), phoneName(name), quantity(qty), unitPrice(price) {}

    std::string getPhoneId() const { return phoneId; }
    std::string getPhoneName() const { return phoneName; }
    int getQuantity() const { return quantity; }
    double getUnitPrice() const { return unitPrice; }

    void setQuantity(int q) { quantity = q; }
    void setUnitPrice(double p) { unitPrice = p; }

    double getSubtotal() const {
        return unitPrice * quantity;
    }
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
    Order() 
        : id(""), customerId(""), deliveryMethod(DeliveryMethod::PICKUP),
          paymentMethod(PaymentMethod::CASH), status(OrderStatus::PENDING),
          orderTime(), discountAmount(0.0) {}

    Order(std::string id, std::string customerId, DeliveryMethod deliv, 
          PaymentMethod pay, ShippingInfo ship)
        : id(id), customerId(customerId), deliveryMethod(deliv),
          paymentMethod(pay), shippingInfo(ship),
          status(OrderStatus::PENDING), orderTime(DateTime::now()),
          discountAmount(0.0) {}

    std::string getID() const { return id; }
    std::string getCustomerId() const { return customerId; }
    DeliveryMethod getDeliveryMethod() const { return deliveryMethod; }
    PaymentMethod getPaymentMethod() const { return paymentMethod; }
    const Vector<OrderDetail>& getDetails() const { return details; }
    Vector<OrderDetail>& getDetails() { return details; }
    ShippingInfo getShippingInfo() const { return shippingInfo; }
    OrderStatus getStatus() const { return status; }
    DateTime getOrderTime() const { return orderTime; }
    double getDiscountAmount() const { return discountAmount; }

    void setID(const std::string& orderId) { id = orderId; }
    void setCustomerId(const std::string& custId) { customerId = custId; }
    void setStatus(OrderStatus s) { status = s; }
    void setDiscountAmount(double d) { discountAmount = d; }
    void setOrderTime(const DateTime& dt) { orderTime = dt; }

    void addDetail(const OrderDetail& detail) {
        details.push_back(detail);
    }

    double calculateSubtotal() const {
        double subtotal = 0.0;
        for (int i = 0; i < details.getSize(); ++i) {
            subtotal += details[i].getSubtotal();
        }
        return subtotal;
    }

    double calculateTotal() const {
        double total = calculateSubtotal() - discountAmount;
        return total > 0.0 ? total : 0.0;
    }

    void confirm() {
        status = OrderStatus::CONFIRMED;
    }

    void cancel() {
        status = OrderStatus::CANCELLED;
    }
};

#endif // ORDER_H
