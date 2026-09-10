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
    Invoice() 
        : id(""), orderId(""), customerId(""), employeeId(""),
          paymentMethod(PaymentMethod::CASH), createTime(DateTime::now()),
          discountAmount(0.0) {}

    Invoice(std::string id, std::string orderId, std::string custId, 
            std::string empId, PaymentMethod pay, const Vector<OrderDetail>& dets, 
            double discount = 0.0)
        : id(id), orderId(orderId), customerId(custId), employeeId(empId),
          paymentMethod(pay), details(dets), createTime(DateTime::now()),
          discountAmount(discount) {}

    std::string getID() const { return id; }
    std::string getOrderId() const { return orderId; }
    std::string getCustomerId() const { return customerId; }
    std::string getEmployeeId() const { return employeeId; }
    PaymentMethod getPaymentMethod() const { return paymentMethod; }
    const Vector<OrderDetail>& getDetails() const { return details; }
    DateTime getCreateTime() const { return createTime; }
    double getDiscountAmount() const { return discountAmount; }

    void addDetail(const OrderDetail& d) {
        details.push_back(d);
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
};

#endif // INVOICE_H
