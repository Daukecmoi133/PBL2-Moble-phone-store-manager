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

    void setID(const std::string& invoiceId);
    void setOrderId(const std::string& ordId);
    void setCustomerId(const std::string& custId);
    void setEmployeeId(const std::string& empId);
    void setPaymentMethod(PaymentMethod pay);
    void setCreateTime(const DateTime& dt);
    void setDiscountAmount(double d);

    void addDetail(const OrderDetail& d);
    double calculateSubtotal() const;
    double calculateTotal() const;
    std::string toFileString() const;

    // Đọc và phân tích một dòng từ Invoices.txt
    static Invoice fromFileLine(const std::string& line);
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

inline void Invoice::setID(const std::string& invoiceId) { id = invoiceId; }
inline void Invoice::setOrderId(const std::string& ordId) { orderId = ordId; }
inline void Invoice::setCustomerId(const std::string& custId) { customerId = custId; }
inline void Invoice::setEmployeeId(const std::string& empId) { employeeId = empId; }
inline void Invoice::setPaymentMethod(PaymentMethod pay) { paymentMethod = pay; }
inline void Invoice::setCreateTime(const DateTime& dt) { createTime = dt; }
inline void Invoice::setDiscountAmount(double d) { discountAmount = d; }

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

inline std::string Invoice::toFileString() const {
    std::string line = id + "|" + orderId + "|" + customerId + "|" + employeeId + "|" +
                       std::to_string(static_cast<int>(paymentMethod)) + "|" +
                       createTime.toString() + "|" +
                       std::to_string(discountAmount) + "|";
    for (int i = 0; i < details.getSize(); ++i) {
        line += details[i].getPhoneId() + "," + details[i].getPhoneName() + "," +
                std::to_string(details[i].getQuantity()) + "," +
                std::to_string(details[i].getUnitPrice());
        if (i < details.getSize() - 1) line += ";";
    }
    return line;
}

// Phân tích dòng dữ liệu từ Invoices.txt thành đối tượng Invoice
// Format: id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details
inline Invoice Invoice::fromFileLine(const std::string& line) {
    Vector<std::string> parts = StringHelper::split(line, '|');
    if (parts.getSize() < 8) return Invoice(); // Dòng không hợp lệ

    Invoice inv;
    inv.setID(StringHelper::trim(parts[0]));
    inv.setOrderId(StringHelper::trim(parts[1]));
    inv.setCustomerId(StringHelper::trim(parts[2]));
    inv.setEmployeeId(StringHelper::trim(parts[3]));
    inv.setPaymentMethod(static_cast<PaymentMethod>(std::stoi(StringHelper::trim(parts[4]))));
    inv.setCreateTime(DateTime::fromString(StringHelper::trim(parts[5])));
    inv.setDiscountAmount(std::stod(StringHelper::trim(parts[6])));

    // Parse details: phoneId,phoneName,quantity,unitPrice;...
    std::string detailStr = StringHelper::trim(parts[7]);
    if (!detailStr.empty()) {
        Vector<std::string> detailTokens = StringHelper::split(detailStr, ';');
        for (int i = 0; i < detailTokens.getSize(); ++i) {
            Vector<std::string> d = StringHelper::split(detailTokens[i], ',');
            if (d.getSize() >= 4) {
                OrderDetail od(
                    StringHelper::trim(d[0]),
                    StringHelper::trim(d[1]),
                    std::stoi(StringHelper::trim(d[2])),
                    std::stod(StringHelper::trim(d[3]))
                );
                inv.addDetail(od);
            }
        }
    }
    return inv;
}

#endif // INVOICE_H
