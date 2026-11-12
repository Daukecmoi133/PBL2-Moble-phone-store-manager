#ifndef ORDER_H
#define ORDER_H

#include <string>
// FIX LỖI: đổi include theo cây thư mục mới
#include "../enums/DeliveryType.h"
#include "../enums/PaymentMethod.h"
#include "../enums/OrderStatus.h"
#include "../enums/InventoryState.h"
#include "../catalog/Phone.h"
#include "../../common/DateTime.h"
#include "../../storage/ds/Vector.h"
#include "../../common/StringUtil.h"
#include "../../storage/ds/StringSplit.h"

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
    InventoryState inventoryState; // // FIX LỖI: trạng thái tồn kho của đơn (đặc tả mục 7); CHỈ OrderService được đổi
    bool customerConfirmed;        // Khách hàng xác nhận đã nhận hàng
    std::string discountCode;      // Mã giảm giá đã áp dụng

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
    DateTime getCreatedAt() const;
    double getDiscountAmount() const;
    InventoryState getInventoryState() const; // // FIX LỖI: đặc tả mục 7
    bool isCustomerConfirmed() const;
    std::string getDiscountCode() const;

    void setID(const std::string& orderId);
    void setCustomerId(const std::string& custId);
    void setDeliveryMethod(DeliveryMethod m);
    void setPaymentMethod(PaymentMethod p);
    void setShippingInfo(const ShippingInfo& s);
    void setStatus(OrderStatus s);
    void setDiscountAmount(double d);
    void setOrderTime(const DateTime& dt);
    void setCreatedAt(const DateTime& dt);
    void setInventoryState(InventoryState s); // // FIX LỖI: đặc tả mục 7
    void setCustomerConfirmed(bool c);
    void setDiscountCode(const std::string& code);

    void addDetail(const OrderDetail& detail);
    double calculateSubtotal() const;
    double calculateTotal() const;
    void confirm();
    void cancel();

    // Format: id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details
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
      orderTime(), discountAmount(0.0), inventoryState(InventoryState::NONE),
      customerConfirmed(false), discountCode("") {}

inline Order::Order(std::string id, std::string customerId, DeliveryMethod deliv, 
                    PaymentMethod pay, ShippingInfo ship)
    : id(id), customerId(customerId), deliveryMethod(deliv),
      paymentMethod(pay), shippingInfo(ship),
      status(OrderStatus::PENDING), orderTime(DateTime::now()),
      discountAmount(0.0), inventoryState(InventoryState::NONE),
      customerConfirmed(false), discountCode("") {}

inline std::string Order::getID() const { return id; }
inline std::string Order::getCustomerId() const { return customerId; }
inline DeliveryMethod Order::getDeliveryMethod() const { return deliveryMethod; }
inline PaymentMethod Order::getPaymentMethod() const { return paymentMethod; }
inline const Vector<OrderDetail>& Order::getDetails() const { return details; }
inline Vector<OrderDetail>& Order::getDetails() { return details; }
inline ShippingInfo Order::getShippingInfo() const { return shippingInfo; }
inline OrderStatus Order::getStatus() const { return status; }
inline DateTime Order::getOrderTime() const { return orderTime; }
inline DateTime Order::getCreatedAt() const { return orderTime; }
inline double Order::getDiscountAmount() const { return discountAmount; }
inline InventoryState Order::getInventoryState() const { return inventoryState; }
inline bool Order::isCustomerConfirmed() const { return customerConfirmed; }
inline std::string Order::getDiscountCode() const { return discountCode; }

inline void Order::setID(const std::string& orderId) { id = orderId; }
inline void Order::setCustomerId(const std::string& custId) { customerId = custId; }
inline void Order::setDeliveryMethod(DeliveryMethod m) { deliveryMethod = m; }
inline void Order::setPaymentMethod(PaymentMethod p) { paymentMethod = p; }
inline void Order::setShippingInfo(const ShippingInfo& s) { shippingInfo = s; }
inline void Order::setStatus(OrderStatus s) { status = s; }
inline void Order::setDiscountAmount(double d) { discountAmount = d; }
inline void Order::setOrderTime(const DateTime& dt) { orderTime = dt; }
inline void Order::setCreatedAt(const DateTime& dt) { orderTime = dt; }
inline void Order::setInventoryState(InventoryState s) { inventoryState = s; }
inline void Order::setCustomerConfirmed(bool c) { customerConfirmed = c; }
inline void Order::setDiscountCode(const std::string& code) { discountCode = code; }

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

// Format: id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details|inventoryState|customerConfirmed|discountCode
// details: phoneId,phoneName,quantity,unitPrice;phoneId,phoneName,...
inline std::string Order::toFileString() const {
    std::string line = id + "|" + customerId + "|" +
        std::to_string(static_cast<int>(deliveryMethod)) + "|" +
        std::to_string(static_cast<int>(paymentMethod)) + "|" +
        std::to_string(static_cast<int>(status)) + "|" +
        orderTime.toString() + "|" +
        std::to_string(static_cast<long long>(discountAmount)) + "|" +
        shippingInfo.getReceiverName() + "|" +
        shippingInfo.getPhoneNumber() + "|" +
        shippingInfo.getAddress() + "|";
    for (int i = 0; i < details.getSize(); ++i) {
        line += details[i].getPhoneId() + "," + details[i].getPhoneName() + "," +
                std::to_string(details[i].getQuantity()) + "," +
                std::to_string(static_cast<long long>(details[i].getUnitPrice()));
        if (i < details.getSize() - 1) line += ";";
    }
    line += "|" + std::to_string(static_cast<int>(inventoryState)) + "|" +
            (customerConfirmed ? "1" : "0") + "|" +
            discountCode;
    return line;
}

// Phân tích dòng dữ liệu từ Orders.txt thành đối tượng Order
inline Order Order::fromFileLine(const std::string& line) {
    Vector<std::string> parts = StringHelper::split(line, '|');
    if (parts.getSize() < 11) return Order(); // Dòng không hợp lệ

    Order o;
    o.setID(StringHelper::trim(parts[0]));
    o.setCustomerId(StringHelper::trim(parts[1]));
    // deliveryMethod: 0=PICKUP, 1=DELIVERY
    int deliv = std::stoi(StringHelper::trim(parts[2]));
    // paymentMethod: 0=CASH, 1=BANK_TRANSFER, 2=CREDIT_CARD
    int pay   = std::stoi(StringHelper::trim(parts[3]));
    int stat  = std::stoi(StringHelper::trim(parts[4]));
    o.setDeliveryMethod(static_cast<DeliveryMethod>(deliv));
    o.setPaymentMethod(static_cast<PaymentMethod>(pay));
    o.setOrderTime(DateTime::fromString(StringHelper::trim(parts[5])));
    o.setDiscountAmount(std::stod(StringHelper::trim(parts[6])));

    ShippingInfo ship(
        StringHelper::trim(parts[7]),
        StringHelper::trim(parts[8]),
        StringHelper::trim(parts[9])
    );
    o.setShippingInfo(ship);

    // Parse details: phoneId,phoneName,quantity,unitPrice;...
    if (!StringHelper::trim(parts[10]).empty()) {
        Vector<std::string> detailTokens = StringHelper::split(parts[10], ';');
        for (int i = 0; i < detailTokens.getSize(); ++i) {
            Vector<std::string> d = StringHelper::split(detailTokens[i], ',');
            if (d.getSize() >= 4) {
                OrderDetail od(
                    StringHelper::trim(d[0]),
                    StringHelper::trim(d[1]),
                    std::stoi(StringHelper::trim(d[2])),
                    std::stod(StringHelper::trim(d[3]))
                );
                o.addDetail(od);
            }
        }
    }

    o.setStatus(static_cast<OrderStatus>(stat));

    if (parts.getSize() >= 12) {
        try {
            o.setInventoryState(static_cast<InventoryState>(std::stoi(StringHelper::trim(parts[11]))));
        } catch (...) {
            o.setInventoryState(InventoryState::NONE);
        }
    }
    if (parts.getSize() >= 13) {
        std::string confStr = StringHelper::trim(parts[12]);
        o.setCustomerConfirmed(confStr == "1" || StringHelper::toLower(confStr) == "true");
    } else {
        o.setCustomerConfirmed(static_cast<OrderStatus>(stat) == OrderStatus::COMPLETED);
    }
    if (parts.getSize() >= 14) {
        o.setDiscountCode(StringHelper::trim(parts[13]));
    }

    return o;
}

#endif // ORDER_H
