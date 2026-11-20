#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "Common.h"
#include "Phone.h"
#include "../utils/AppClock.h"
#include "../utils/DateTime.h"
#include "../utils/PricingCalculator.h"
#include "../utils/TextCodec.h"
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
    double unitCost; // Giá vốn snapshot tại thời điểm bán (-1 nếu chưa ghi nhận ở bản ghi cũ)

public:
    OrderDetail();
    OrderDetail(std::string id, std::string name, int qty, double price, double cost = -1.0);

    std::string getPhoneId() const;
    std::string getPhoneName() const;
    int getQuantity() const;
    double getUnitPrice() const;
    double getUnitCost() const;
    bool hasUnitCost() const;

    void setQuantity(int q);
    void setUnitPrice(double p);
    void setUnitCost(double c);

    double getSubtotal() const;
    double getTotalCost() const;
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
    std::string discountCode;
    double vatRate;
    double vatAmount;
    bool vatApplied;

public:
    Order();
    Order(std::string id, std::string customerId, DeliveryMethod deliv, 
          PaymentMethod pay, ShippingInfo ship, std::string discCode = "");

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
    std::string getDiscountCode() const;
    double getVatRate() const;
    double getVatAmount() const;
    bool isVatApplied() const;
    bool isLegacyNoVat() const;

    void setID(const std::string& orderId);
    void setCustomerId(const std::string& custId);
    void setStatus(OrderStatus s);
    void setDiscountAmount(double d);
    void setDiscountCode(const std::string& code);
    void setOrderTime(const DateTime& dt);
    void setVat(double rate, double amount, bool applied = true);
    void setDeliveryMethod(DeliveryMethod m);
    void setPaymentMethod(PaymentMethod p);
    void setShippingInfo(const ShippingInfo& s);

    void addDetail(const OrderDetail& detail);
    double calculateSubtotal() const;
    double calculateNetRevenue() const;
    double calculateTotal() const;
    PricingBreakdown getPricingBreakdown() const;
    void confirm();
    void cancel();

    // Format v3: id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details|discountCode|vatRate|vatAmount
    std::string toFileString() const;
    static Order fromFileLine(const std::string& line);
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

inline OrderDetail::OrderDetail()
    : phoneId(""), phoneName(""), quantity(0), unitPrice(0.0), unitCost(-1.0) {}

inline OrderDetail::OrderDetail(std::string id, std::string name, int qty, double price, double cost)
    : phoneId(id), phoneName(name), quantity(qty), unitPrice(price), unitCost(cost) {}

inline std::string OrderDetail::getPhoneId() const { return phoneId; }
inline std::string OrderDetail::getPhoneName() const { return phoneName; }
inline int OrderDetail::getQuantity() const { return quantity; }
inline double OrderDetail::getUnitPrice() const { return unitPrice; }
inline double OrderDetail::getUnitCost() const { return unitCost; }
inline bool OrderDetail::hasUnitCost() const { return unitCost >= 0.0; }

inline void OrderDetail::setQuantity(int q) { quantity = q; }
inline void OrderDetail::setUnitPrice(double p) { unitPrice = p; }
inline void OrderDetail::setUnitCost(double c) { unitCost = c; }

inline double OrderDetail::getSubtotal() const {
    return unitPrice * quantity;
}

inline double OrderDetail::getTotalCost() const {
    return (unitCost >= 0.0) ? (unitCost * quantity) : 0.0;
}

// ================= IMPLEMENTATION ORDER =================

inline Order::Order() 
    : id(""), customerId(""), deliveryMethod(DeliveryMethod::PICKUP),
      paymentMethod(PaymentMethod::CASH), status(OrderStatus::PENDING),
      orderTime(AppClock::now()), discountAmount(0.0), discountCode(""),
      vatRate(0.0), vatAmount(0.0), vatApplied(false) {}

inline Order::Order(std::string id, std::string customerId, DeliveryMethod deliv, 
                    PaymentMethod pay, ShippingInfo ship, std::string discCode)
    : id(id), customerId(customerId), deliveryMethod(deliv),
      paymentMethod(pay), shippingInfo(ship),
      status(OrderStatus::PENDING), orderTime(AppClock::now()),
      discountAmount(0.0), discountCode(discCode),
      vatRate(0.0), vatAmount(0.0), vatApplied(false) {}

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
inline std::string Order::getDiscountCode() const { return discountCode; }
inline double Order::getVatRate() const { return vatRate; }
inline double Order::getVatAmount() const { return vatAmount; }
inline bool Order::isVatApplied() const { return vatApplied; }
inline bool Order::isLegacyNoVat() const { return !vatApplied; }

inline void Order::setID(const std::string& orderId) { id = orderId; }
inline void Order::setCustomerId(const std::string& custId) { customerId = custId; }
inline void Order::setStatus(OrderStatus s) { status = s; }
inline void Order::setDiscountAmount(double d) { discountAmount = d; }
inline void Order::setDiscountCode(const std::string& code) { discountCode = code; }
inline void Order::setOrderTime(const DateTime& dt) { orderTime = dt; }
inline void Order::setVat(double rate, double amount, bool applied) {
    vatApplied = applied;
    if (applied) {
        vatRate = PricingCalculator::sanitizeVatRate(rate);
        vatAmount = (amount >= 0.0) ? amount : 0.0;
    } else {
        vatRate = 0.0;
        vatAmount = 0.0;
    }
}
inline void Order::setDeliveryMethod(DeliveryMethod m) { deliveryMethod = m; }
inline void Order::setPaymentMethod(PaymentMethod p) { paymentMethod = p; }
inline void Order::setShippingInfo(const ShippingInfo& s) { shippingInfo = s; }

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

inline double Order::calculateNetRevenue() const {
    return PricingCalculator::computeNetRevenue(calculateSubtotal(), discountAmount);
}

inline PricingBreakdown Order::getPricingBreakdown() const {
    return PricingCalculator::fromStored(
        calculateSubtotal(), discountAmount, vatRate, vatAmount, vatApplied);
}

inline double Order::calculateTotal() const {
    return getPricingBreakdown().grandTotal;
}

inline void Order::confirm() {
    status = OrderStatus::CONFIRMED;
}

inline void Order::cancel() {
    status = OrderStatus::CANCELLED;
}

// Format v3: id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details|discountCode|vatRate|vatAmount
// details: phoneId,phoneName,quantity,unitPrice,unitCost;...
inline std::string Order::toFileString() const {
    std::string line = TextCodec::escapeField(id) + "|" +
        TextCodec::escapeField(customerId) + "|" +
        std::to_string(static_cast<int>(deliveryMethod)) + "|" +
        std::to_string(static_cast<int>(paymentMethod)) + "|" +
        std::to_string(static_cast<int>(status)) + "|" +
        orderTime.toString() + "|" +
        StringHelper::formatMoneyDisk(discountAmount) + "|" +
        TextCodec::escapeField(shippingInfo.getReceiverName()) + "|" +
        TextCodec::escapeField(shippingInfo.getPhoneNumber()) + "|" +
        TextCodec::escapeField(shippingInfo.getAddress()) + "|";
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

// Phân tích dòng dữ liệu từ Orders.txt thành đối tượng Order (tương thích ngược v1/v2/v3)
inline Order Order::fromFileLine(const std::string& line) {
    Vector<std::string> parts = TextCodec::splitFields(line, '|');
    if (parts.getSize() < 11) return Order(); // Dòng không hợp lệ

    Order o;
    o.setID(StringHelper::trim(TextCodec::unescape(parts[0])));
    o.setCustomerId(StringHelper::trim(TextCodec::unescape(parts[1])));
    
    try {
        int deliv = std::stoi(StringHelper::trim(parts[2]));
        int pay   = std::stoi(StringHelper::trim(parts[3]));
        int stat  = std::stoi(StringHelper::trim(parts[4]));
        o.setDeliveryMethod(static_cast<DeliveryMethod>(deliv));
        o.setPaymentMethod(static_cast<PaymentMethod>(pay));
        o.setStatus(static_cast<OrderStatus>(stat));
        o.setDiscountAmount(StringHelper::parsePrice(parts[6]));
    } catch (...) {
        return Order(); // Dữ liệu số không hợp lệ
    }

    o.setOrderTime(DateTime::fromString(StringHelper::trim(parts[5])));

    ShippingInfo ship(
        TextCodec::unescape(parts[7]),
        StringHelper::trim(TextCodec::unescape(parts[8])),
        TextCodec::unescape(parts[9])
    );
    o.setShippingInfo(ship);

    // Cột 12 (index 11): discountCode nếu có
    if (parts.getSize() >= 12) {
        o.setDiscountCode(StringHelper::trim(TextCodec::unescape(parts[11])));
    }

    // Cột 13-14 (index 12-13): vatRate|vatAmount (v3). Bản ghi cũ giữ nguyên vatApplied = false
    if (parts.getSize() >= 14) {
        const std::string rawVatRate = StringHelper::trim(parts[12]);
        const std::string rawVatAmt = StringHelper::trim(parts[13]);
        if (rawVatRate.empty() || rawVatRate == "LEGACY" || rawVatRate == "-1") {
            o.setVat(0.0, 0.0, false);
        } else {
            double parsedRate = 0.0;
            if (PricingCalculator::tryParseVatRate(rawVatRate, parsedRate)) {
                double parsedAmt = rawVatAmt.empty() ? 0.0 : StringHelper::parsePrice(rawVatAmt);
                o.setVat(parsedRate, parsedAmt, true);
            } else {
                o.setVat(0.0, 0.0, false);
            }
        }
    } else {
        o.setVat(0.0, 0.0, false);
    }

    // Parse details: phoneId,phoneName,quantity,unitPrice[,unitCost];...
    if (!StringHelper::trim(parts[10]).empty()) {
        Vector<std::string> detailTokens = TextCodec::splitFields(parts[10], ';');
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
                    o.addDetail(od);
                } catch (...) {
                    // Bỏ qua chi tiết lỗi
                }
            }
        }
    }

    return o;
}

#endif // ORDER_H
