#ifndef INVOICE_H
#define INVOICE_H

#include <string>
#include "Common.h"
#include "Order.h"
#include "../utils/DateTime.h"
#include "../core/Vector.h"

class Invoice {
private:
    std::string id;
    std::string orderId;
    std::string customerId;
    std::string employeeId;
    PaymentMethod paymentMethod;
    Vector<OrderDetail> details;
    DateTime createTime;
    double discountAmount;

public:
    Invoice();
    Invoice(std::string id, std::string orderId, std::string custId, 
            std::string empId, PaymentMethod pay, const Vector<OrderDetail>& dets, 
            double discount = 0.0);

    std::string getID() const;
    std::string getOrderId() const;
    std::string getCustomerId() const;
    std::string getEmployeeId() const;
    PaymentMethod getPaymentMethod() const;
    const Vector<OrderDetail>& getDetails() const;
    DateTime getCreateTime() const;
    double getDiscountAmount() const;

    void addDetail(const OrderDetail& d);
    double calculateSubtotal() const;
    double calculateTotal() const;
};

// ================= IMPLEMENTATION =================

inline Invoice::Invoice() 
    : id(""), orderId(""), customerId(""), employeeId(""),
      paymentMethod(PaymentMethod::CASH), createTime(DateTime::now()),
      discountAmount(0.0) {}

inline Invoice::Invoice(std::string id, std::string orderId, std::string custId, 
                        std::string empId, PaymentMethod pay, const Vector<OrderDetail>& dets, 
                        double discount)
    : id(id), orderId(orderId), customerId(custId), employeeId(empId),
      paymentMethod(pay), details(dets), createTime(DateTime::now()),
      discountAmount(discount) {}

inline std::string Invoice::getID() const { return id; }
inline std::string Invoice::getOrderId() const { return orderId; }
inline std::string Invoice::getCustomerId() const { return customerId; }
inline std::string Invoice::getEmployeeId() const { return employeeId; }
inline PaymentMethod Invoice::getPaymentMethod() const { return paymentMethod; }
inline const Vector<OrderDetail>& Invoice::getDetails() const { return details; }
inline DateTime Invoice::getCreateTime() const { return createTime; }
inline double Invoice::getDiscountAmount() const { return discountAmount; }

inline void Invoice::addDetail(const OrderDetail& d) {
    details.push_back(d);
}

inline double Invoice::calculateSubtotal() const {
    double subtotal = 0.0;
    for (int i = 0; i < details.getSize(); ++i) {
        subtotal += details[i].getSubtotal();
    }
    return subtotal;
}

inline double Invoice::calculateTotal() const {
    double total = calculateSubtotal() - discountAmount;
    return total > 0.0 ? total : 0.0;
}

#endif // INVOICE_H
