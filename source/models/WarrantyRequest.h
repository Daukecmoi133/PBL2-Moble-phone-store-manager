#ifndef WARRANTYREQUEST_H
#define WARRANTYREQUEST_H

#include <string>
#include "Common.h"
#include "../utils/AppClock.h"
#include "../utils/DateTime.h"
#include "../utils/StringHelper.h"
#include "../utils/TextCodec.h"
#include "../utils/IdGenerator.h"
#include "../core/Vector.h"

/**
 * @class WarrantyRequest
 * @brief Mô hình yêu cầu bảo hành sản phẩm (Warranty Request Model)
 *
 * Lưu trữ thông tin một phiếu tiếp nhận bảo hành theo chuẩn 11 cột:
 * id|orderId|phoneId|phoneName|customerId|createdAt|status|issueDescription|technicianNote|resolution|updatedAt
 */
class WarrantyRequest {
private:
    std::string id;
    std::string orderId;
    std::string phoneId;
    std::string phoneName;
    std::string customerId;
    DateTime createdAt;
    WarrantyStatus status;
    std::string issueDescription;
    std::string technicianNote;
    std::string resolution;
    DateTime updatedAt;

public:
    WarrantyRequest();
    WarrantyRequest(const std::string& id,
                    const std::string& orderId,
                    const std::string& phoneId,
                    const std::string& phoneName,
                    const std::string& customerId,
                    const DateTime& createdAt,
                    WarrantyStatus status,
                    const std::string& issueDescription,
                    const std::string& technicianNote = "",
                    const std::string& resolution = "",
                    const DateTime& updatedAt = AppClock::now());

    // Getters
    std::string getId() const;
    std::string getID() const;
    std::string getOrderId() const;
    std::string getPhoneId() const;
    std::string getPhoneName() const;
    std::string getCustomerId() const;
    DateTime getCreatedAt() const;
    WarrantyStatus getStatus() const;
    std::string getIssueDescription() const;
    std::string getTechnicianNote() const;
    std::string getResolution() const;
    DateTime getUpdatedAt() const;

    // Setters
    void setId(const std::string& val);
    void setID(const std::string& val);
    void setOrderId(const std::string& val);
    void setPhoneId(const std::string& val);
    void setPhoneName(const std::string& val);
    void setCustomerId(const std::string& val);
    void setCreatedAt(const DateTime& val);
    void setStatus(WarrantyStatus val);
    void setIssueDescription(const std::string& val);
    void setTechnicianNote(const std::string& val);
    void setResolution(const std::string& val);
    void setUpdatedAt(const DateTime& val);

    // Xử lý chuỗi an toàn
    static std::string sanitizeText(const std::string& text, size_t maxLen = 500);

    // Serialization & Parsing
    std::string toFileString() const;
    static WarrantyRequest fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

inline WarrantyRequest::WarrantyRequest()
    : id(""), orderId(""), phoneId(""), phoneName(""), customerId(""),
      createdAt(AppClock::now()), status(WarrantyStatus::RECEIVED),
      issueDescription(""), technicianNote(""), resolution(""),
      updatedAt(AppClock::now()) {}

inline WarrantyRequest::WarrantyRequest(const std::string& id,
                                        const std::string& orderId,
                                        const std::string& phoneId,
                                        const std::string& phoneName,
                                        const std::string& customerId,
                                        const DateTime& createdAt,
                                        WarrantyStatus status,
                                        const std::string& issueDescription,
                                        const std::string& technicianNote,
                                        const std::string& resolution,
                                        const DateTime& updatedAt)
    : id(id), orderId(orderId), phoneId(phoneId), phoneName(phoneName),
      customerId(customerId), createdAt(createdAt), status(status),
      issueDescription(issueDescription), technicianNote(technicianNote),
      resolution(resolution), updatedAt(updatedAt) {}

inline std::string WarrantyRequest::getId() const { return id; }
inline std::string WarrantyRequest::getID() const { return id; }
inline std::string WarrantyRequest::getOrderId() const { return orderId; }
inline std::string WarrantyRequest::getPhoneId() const { return phoneId; }
inline std::string WarrantyRequest::getPhoneName() const { return phoneName; }
inline std::string WarrantyRequest::getCustomerId() const { return customerId; }
inline DateTime WarrantyRequest::getCreatedAt() const { return createdAt; }
inline WarrantyStatus WarrantyRequest::getStatus() const { return status; }
inline std::string WarrantyRequest::getIssueDescription() const { return issueDescription; }
inline std::string WarrantyRequest::getTechnicianNote() const { return technicianNote; }
inline std::string WarrantyRequest::getResolution() const { return resolution; }
inline DateTime WarrantyRequest::getUpdatedAt() const { return updatedAt; }

inline void WarrantyRequest::setId(const std::string& val) { id = val; }
inline void WarrantyRequest::setID(const std::string& val) { id = val; }
inline void WarrantyRequest::setOrderId(const std::string& val) { orderId = val; }
inline void WarrantyRequest::setPhoneId(const std::string& val) { phoneId = val; }
inline void WarrantyRequest::setPhoneName(const std::string& val) { phoneName = val; }
inline void WarrantyRequest::setCustomerId(const std::string& val) { customerId = val; }
inline void WarrantyRequest::setCreatedAt(const DateTime& val) { createdAt = val; }
inline void WarrantyRequest::setStatus(WarrantyStatus val) { status = val; }
inline void WarrantyRequest::setIssueDescription(const std::string& val) { issueDescription = val; }
inline void WarrantyRequest::setTechnicianNote(const std::string& val) { technicianNote = val; }
inline void WarrantyRequest::setResolution(const std::string& val) { resolution = val; }
inline void WarrantyRequest::setUpdatedAt(const DateTime& val) { updatedAt = val; }

inline std::string WarrantyRequest::sanitizeText(const std::string& text, size_t maxLen) {
    if (text.empty()) return "";
    std::string s = text;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '|') {
            s[i] = '/';
        } else if (s[i] == '\r' || s[i] == '\n' || s[i] == '\t') {
            s[i] = ' ';
        }
    }
    s = StringHelper::trim(s);
    if (s.empty()) return "";

    // Cắt theo ký tự UTF-8 (tối đa maxLen ký tự, không cắt giữa ký tự nhiều byte)
    size_t bytePos = 0;
    size_t charCount = 0;
    while (bytePos < s.size() && charCount < maxLen) {
        unsigned char c = static_cast<unsigned char>(s[bytePos]);
        size_t charBytes = 1;
        if ((c & 0x80) == 0x00) {
            charBytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            charBytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            charBytes = 3;
        } else if ((c & 0xF8) == 0xF0) {
            charBytes = 4;
        } else {
            charBytes = 1;
        }

        if (bytePos + charBytes > s.size()) {
            break; // Tránh trường hợp ký tự cuối cùng bị đứt gãy byte
        }
        bytePos += charBytes;
        charCount++;
    }

    return s.substr(0, bytePos);
}

inline std::string WarrantyRequest::toFileString() const {
    return TextCodec::escapeField(id) + "|" +
           TextCodec::escapeField(orderId) + "|" +
           TextCodec::escapeField(phoneId) + "|" +
           TextCodec::escapeField(phoneName) + "|" +
           TextCodec::escapeField(customerId) + "|" +
           createdAt.toString() + "|" +
           std::to_string(static_cast<int>(status)) + "|" +
           TextCodec::escapeField(issueDescription) + "|" +
           TextCodec::escapeField(technicianNote) + "|" +
           TextCodec::escapeField(resolution) + "|" +
           updatedAt.toString();
}

inline WarrantyRequest WarrantyRequest::fromFileLine(const std::string& line) {
    Vector<std::string> parts = TextCodec::splitFields(line, '|');
    if (parts.getSize() != 11) {
        return WarrantyRequest(); // Không đủ hoặc thừa cột
    }

    std::string reqId = StringHelper::trim(TextCodec::unescape(parts[0]));
    if (!IdGenerator::isValidId(reqId, "WREQ", true)) {
        return WarrantyRequest(); // ID không đúng chuẩn WREQ-ddmmyyyy-xxxxxx
    }

    std::string ordId = StringHelper::trim(TextCodec::unescape(parts[1]));
    std::string phnId = StringHelper::trim(TextCodec::unescape(parts[2]));
    std::string phnName = TextCodec::unescape(parts[3]);
    std::string custId = StringHelper::trim(TextCodec::unescape(parts[4]));

    std::string createdStr = StringHelper::trim(parts[5]);
    DateTime created = DateTime::fromString(createdStr);

    std::string statusStr = StringHelper::trim(parts[6]);
    int statusInt = 0;
    try {
        statusInt = std::stoi(statusStr);
    } catch (...) {
        return WarrantyRequest();
    }
    if (statusInt < 0 || statusInt > 3) {
        return WarrantyRequest(); // Trạng thái ngoài khoảng 0..3
    }
    WarrantyStatus stat = static_cast<WarrantyStatus>(statusInt);

    std::string issue = TextCodec::unescape(parts[7]);
    std::string techNote = TextCodec::unescape(parts[8]);
    std::string res = TextCodec::unescape(parts[9]);

    std::string updatedStr = StringHelper::trim(parts[10]);
    DateTime updated = DateTime::fromString(updatedStr);

    return WarrantyRequest(reqId, ordId, phnId, phnName, custId,
                           created, stat, issue, techNote, res, updated);
}

#endif // WARRANTYREQUEST_H
