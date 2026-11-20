#ifndef INVOICE_H
#define INVOICE_H

#include <string>
#include "Common.h"
#include "Order.h"
#include "../utils/AppClock.h"
#include "../utils/DateTime.h"
#include "../utils/PricingCalculator.h"
#include "../utils/StringHelper.h"
#include "../utils/TextCodec.h"
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
    std::string discountCode;
    double vatRate;
    double vatAmount;
    bool vatApplied;

public:
    Invoice();
    Invoice(std::string id, std::string orderId, std::string custId, 
            std::string empId, PaymentMethod pay, const Vector<OrderDetail>& dets, 
            double discount = 0.0, std::string discCode = "");

    std::string getID() const;
    std::string getOrderId() const;
    std::string getCustomerId() const;
    std::string getEmployeeId() const;
    PaymentMethod getPaymentMethod() const;
    const Vector<OrderDetail>& getDetails() const;
    Vector<OrderDetail>& getDetails();
    DateTime getCreateTime() const;
    double getDiscountAmount() const;
    std::string getDiscountCode() const;
    double getVatRate() const;
    double getVatAmount() const;
    bool isVatApplied() const;
    bool isLegacyNoVat() const;

    void setID(const std::string& invoiceId);
    void setOrderId(const std::string& ordId);
    void setCustomerId(const std::string& custId);
    void setEmployeeId(const std::string& empId);
    void setPaymentMethod(PaymentMethod pay);
    void setCreateTime(const DateTime& dt);
    void setDiscountAmount(double d);
    void setDiscountCode(const std::string& code);
    void setVat(double rate, double amount, bool applied = true);

    void addDetail(const OrderDetail& d);
    double calculateSubtotal() const;
    double calculateNetRevenue() const;
    PricingBreakdown getPricingBreakdown() const;
    double calculateTotal() const;
    std::string toFileString() const;

    // Đọc và phân tích một dòng từ Invoices.txt (tương thích ngược v1/v2/v3)
    static Invoice fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

inline Invoice::Invoice() 
    : id(""), orderId(""), customerId(""), employeeId(""),
      paymentMethod(PaymentMethod::CASH), createTime(AppClock::now()),
      discountAmount(0.0), discountCode(""),
      vatRate(0.0), vatAmount(0.0), vatApplied(false) {}

inline Invoice::Invoice(std::string id, std::string orderId, std::string custId, 
                        std::string empId, PaymentMethod pay, const Vector<OrderDetail>& dets, 
                        double discount, std::string discCode)
    : id(id), orderId(orderId), customerId(custId), employeeId(empId),
      paymentMethod(pay), details(dets), createTime(AppClock::now()),
      discountAmount(discount), discountCode(discCode),
      vatRate(0.0), vatAmount(0.0), vatApplied(false) {}

inline std::string Invoice::getID() const { return id; }
inline std::string Invoice::getOrderId() const { return orderId; }
inline std::string Invoice::getCustomerId() const { return customerId; }
inline std::string Invoice::getEmployeeId() const { return employeeId; }
inline PaymentMethod Invoice::getPaymentMethod() const { return paymentMethod; }
inline const Vector<OrderDetail>& Invoice::getDetails() const { return details; }
inline Vector<OrderDetail>& Invoice::getDetails() { return details; }
inline DateTime Invoice::getCreateTime() const { return createTime; }
inline double Invoice::getDiscountAmount() const { return discountAmount; }
inline std::string Invoice::getDiscountCode() const { return discountCode; }
inline double Invoice::getVatRate() const { return vatRate; }
inline double Invoice::getVatAmount() const { return vatAmount; }
inline bool Invoice::isVatApplied() const { return vatApplied; }
inline bool Invoice::isLegacyNoVat() const { return !vatApplied; }

inline void Invoice::setID(const std::string& invoiceId) { id = invoiceId; }
inline void Invoice::setOrderId(const std::string& ordId) { orderId = ordId; }
inline void Invoice::setCustomerId(const std::string& custId) { customerId = custId; }
inline void Invoice::setEmployeeId(const std::string& empId) { employeeId = empId; }
inline void Invoice::setPaymentMethod(PaymentMethod pay) { paymentMethod = pay; }
inline void Invoice::setCreateTime(const DateTime& dt) { createTime = dt; }
inline void Invoice::setDiscountAmount(double d) { discountAmount = d; }
inline void Invoice::setDiscountCode(const std::string& code) { discountCode = code; }
inline void Invoice::setVat(double rate, double amount, bool applied) {
    vatApplied = applied;
    if (applied) {
        vatRate = PricingCalculator::sanitizeVatRate(rate);
        vatAmount = (amount >= 0.0) ? amount : 0.0;
    } else {
        vatRate = 0.0;
        vatAmount = 0.0;
    }
}

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

inline double Invoice::calculateNetRevenue() const {
    return PricingCalculator::computeNetRevenue(calculateSubtotal(), discountAmount);
}

inline PricingBreakdown Invoice::getPricingBreakdown() const {
    return PricingCalculator::fromStored(
        calculateSubtotal(), discountAmount, vatRate, vatAmount, vatApplied);
}

inline double Invoice::calculateTotal() const {
    return getPricingBreakdown().grandTotal;
}

inline std::string Invoice::toFileString() const {
    std::string line = TextCodec::escapeField(id) + "|" +
                       TextCodec::escapeField(orderId) + "|" +
                       TextCodec::escapeField(customerId) + "|" +
                       TextCodec::escapeField(employeeId) + "|" +
                       std::to_string(static_cast<int>(paymentMethod)) + "|" +
                       createTime.toString() + "|" +
                       StringHelper::formatMoneyDisk(discountAmount) + "|";
    for (int i = 0; i < details.getSize(); ++i) {
        std::string costStr = details[i].hasUnitCost()
            ? StringHelper::formatMoneyDisk(details[i].getUnitCost())
            : "-1";
        line += TextCodec::escapeSubField(details[i].getPhoneId()) + "," +
                TextCodec::escapeSubField(details[i].getPhoneName()) + "," +
                std::to_string(details[i].getQuantity()) + "," +
                StringHelper::formatMoneyDisk(details[i].getUnitPrice()) + "," +
                costStr;
        if (i < details.getSize() - 1) line += ";";
    }
    line += "|" + TextCodec::escapeField(discountCode);
    if (vatApplied) {
        line += "|" + PricingCalculator::formatVatRateDisk(vatRate) +
                "|" + StringHelper::formatMoneyDisk(vatAmount);
    } else {
        line += "|LEGACY|0";
    }
    return line;
}

// Phân tích dòng dữ liệu từ Invoices.txt thành đối tượng Invoice (v1/v2/v3)
// Format v3: id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details|discountCode|vatRate|vatAmount
inline Invoice Invoice::fromFileLine(const std::string& line) {
    Vector<std::string> parts = TextCodec::splitFields(line, '|');
    if (parts.getSize() < 8) return Invoice(); // Dòng không hợp lệ

    Invoice inv;
    inv.setID(StringHelper::trim(TextCodec::unescape(parts[0])));
    inv.setOrderId(StringHelper::trim(TextCodec::unescape(parts[1])));
    inv.setCustomerId(StringHelper::trim(TextCodec::unescape(parts[2])));
    inv.setEmployeeId(StringHelper::trim(TextCodec::unescape(parts[3])));
    try {
        inv.setPaymentMethod(static_cast<PaymentMethod>(std::stoi(StringHelper::trim(parts[4]))));
        inv.setDiscountAmount(StringHelper::parsePrice(parts[6]));
    } catch (...) {
        return Invoice();
    }
    inv.setCreateTime(DateTime::fromString(StringHelper::trim(parts[5])));

    // Cột 9 (index 8): discountCode nếu có
    if (parts.getSize() >= 9) {
        inv.setDiscountCode(StringHelper::trim(TextCodec::unescape(parts[8])));
    }

    // Cột 10-11 (index 9-10): vatRate|vatAmount (v3)
    if (parts.getSize() >= 11) {
        const std::string rawVatRate = StringHelper::trim(parts[9]);
        const std::string rawVatAmt = StringHelper::trim(parts[10]);
        if (rawVatRate.empty() || rawVatRate == "LEGACY" || rawVatRate == "-1") {
            inv.setVat(0.0, 0.0, false);
        } else {
            double parsedRate = 0.0;
            if (PricingCalculator::tryParseVatRate(rawVatRate, parsedRate)) {
                double parsedAmt = rawVatAmt.empty() ? 0.0 : StringHelper::parsePrice(rawVatAmt);
                inv.setVat(parsedRate, parsedAmt, true);
            } else {
                inv.setVat(0.0, 0.0, false);
            }
        }
    } else {
        inv.setVat(0.0, 0.0, false);
    }

    // Parse details: phoneId,phoneName,quantity,unitPrice[,unitCost];...
    std::string detailStr = parts[7];
    if (!StringHelper::trim(detailStr).empty()) {
        Vector<std::string> detailTokens = TextCodec::splitFields(detailStr, ';');
        for (int i = 0; i < detailTokens.getSize(); ++i) {
            if (StringHelper::trim(detailTokens[i]).empty()) continue;
            Vector<std::string> d = TextCodec::splitFields(detailTokens[i], ',');
            if (d.getSize() >= 4) {
                try {
                    double parsedCost = -1.0;
                    if (d.getSize() >= 5) {
                        std::string rawCost = StringHelper::trim(d[4]);
                        if (!rawCost.empty() && rawCost != "-1") {
                            parsedCost = StringHelper::parsePrice(rawCost);
                        }
                    }
                    OrderDetail od(
                        StringHelper::trim(TextCodec::unescape(d[0])),
                        TextCodec::unescape(d[1]),
                        std::stoi(StringHelper::trim(d[2])),
                        StringHelper::parsePrice(d[3]),
                        parsedCost
                    );
                    inv.addDetail(od);
                } catch (...) {
                    // Bỏ qua chi tiết lỗi
                }
            }
        }
    }
    return inv;
}

#endif // INVOICE_H
