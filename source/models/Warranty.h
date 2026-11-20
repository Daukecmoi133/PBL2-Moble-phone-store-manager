#ifndef WARRANTY_H
#define WARRANTY_H

#include <string>
#include <vector>
#include <stdexcept>
#include "Common.h"
#include "../core/Vector.h"
#include "../utils/AppClock.h"
#include "../utils/DateTime.h"
#include "../utils/StringHelper.h"
#include "../utils/TextCodec.h"

// Mô hình đại diện cho một hồ sơ bảo hành sản phẩm (WAR-000001)
class Warranty {
private:
    std::string id;
    std::string phoneId;
    std::string phoneName;
    std::string imei;
    std::string customerId;
    std::string invoiceId;
    DateTime startDate;
    DateTime endDate;
    WarrantyPolicyStatus status;

public:
    Warranty();
    Warranty(std::string id, std::string phoneId, std::string phoneName,
             std::string imei, std::string customerId, std::string invoiceId,
             DateTime startDate, DateTime endDate, WarrantyPolicyStatus status);

    // Getters
    std::string getID() const { return id; }
    std::string getPhoneId() const { return phoneId; }
    std::string getPhoneName() const { return phoneName; }
    std::string getImei() const { return imei; }
    std::string getCustomerId() const { return customerId; }
    std::string getInvoiceId() const { return invoiceId; }
    DateTime getStartDate() const { return startDate; }
    DateTime getEndDate() const { return endDate; }
    WarrantyPolicyStatus getStatus() const { return status; }

    // Setters
    void setStatus(WarrantyPolicyStatus s) { status = s; }

    // Nghiệp vụ
    bool isExpired() const;

    // Làm sạch ký tự phân tách và xuống dòng trước khi lưu file
    static std::string sanitizeField(const std::string& input);

    // Chuyển đổi tệp tin (9 cột phân tách bởi '|')
    std::string toFileString() const;
    static Warranty fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

inline Warranty::Warranty()
    : id(""), phoneId(""), phoneName(""), imei(""), customerId(""),
      invoiceId(""), startDate(), endDate(), status(WarrantyPolicyStatus::ACTIVE) {}

inline Warranty::Warranty(std::string id, std::string phoneId, std::string phoneName,
                         std::string imei, std::string customerId, std::string invoiceId,
                         DateTime startDate, DateTime endDate, WarrantyPolicyStatus status)
    : id(id), phoneId(phoneId), phoneName(phoneName), imei(imei), customerId(customerId),
      invoiceId(invoiceId), startDate(startDate), endDate(endDate), status(status) {}

inline bool Warranty::isExpired() const {
    if (status == WarrantyPolicyStatus::EXPIRED || status == WarrantyPolicyStatus::VOIDED) return true;
    DateTime current = AppClock::now();
    return current > endDate;
}

inline std::string Warranty::sanitizeField(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        if (c == '|' || c == '\r' || c == '\n') continue;
        result.push_back(c);
    }
    return result;
}

inline std::string Warranty::toFileString() const {
    return TextCodec::escapeField(id) + "|" +
           TextCodec::escapeField(phoneId) + "|" +
           TextCodec::escapeField(phoneName) + "|" +
           TextCodec::escapeField(imei) + "|" +
           TextCodec::escapeField(customerId) + "|" +
           TextCodec::escapeField(invoiceId) + "|" +
           startDate.toString() + "|" +
           endDate.toString() + "|" +
           std::to_string(static_cast<int>(status));
}

inline Warranty Warranty::fromFileLine(const std::string& line) {
    Vector<std::string> tokens = TextCodec::splitFields(line, '|');
    if (tokens.getSize() < 9) {
        throw std::runtime_error("Du lieu ho so bao hanh khong du 9 cot!");
    }
    Warranty w;
    w.id = StringHelper::trim(TextCodec::unescape(tokens[0]));
    w.phoneId = StringHelper::trim(TextCodec::unescape(tokens[1]));
    w.phoneName = TextCodec::unescape(tokens[2]);
    w.imei = StringHelper::trim(TextCodec::unescape(tokens[3]));
    w.customerId = StringHelper::trim(TextCodec::unescape(tokens[4]));
    w.invoiceId = StringHelper::trim(TextCodec::unescape(tokens[5]));
    w.startDate = DateTime::fromString(StringHelper::trim(tokens[6]));
    w.endDate = DateTime::fromString(StringHelper::trim(tokens[7]));
    int stCode = std::stoi(StringHelper::trim(tokens[8]));
    w.status = static_cast<WarrantyPolicyStatus>(stCode);
    return w;
}

#endif // WARRANTY_H