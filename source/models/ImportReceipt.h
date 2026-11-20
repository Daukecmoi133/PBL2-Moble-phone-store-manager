#ifndef IMPORTRECEIPT_H
#define IMPORTRECEIPT_H

#include <string>
#include "Order.h"
#include "../utils/AppClock.h"
#include "../utils/DateTime.h"
#include "../utils/StringHelper.h"
#include "../utils/TextCodec.h"
#include "../core/Vector.h"

class ImportReceipt {
private:
    std::string id;
    std::string supplierId;
    std::string employeeId;
    Vector<OrderDetail> details;
    DateTime importTime;

public:
    ImportReceipt();
    ImportReceipt(std::string id, std::string supId, std::string empId);

    std::string getID() const;
    std::string getSupplierId() const;
    std::string getEmployeeId() const;
    const Vector<OrderDetail>& getDetails() const;
    DateTime getImportTime() const;

    void setID(const std::string& receiptId);
    void setSupplierId(const std::string& supId);
    void setEmployeeId(const std::string& empId);
    void setImportTime(const DateTime& dt);

    void addDetail(const OrderDetail& detail);
    double calculateTotal() const;
    std::string toFileString() const;

    // Đọc và phân tích một dòng từ StockReceipts.txt
    static ImportReceipt fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

inline ImportReceipt::ImportReceipt() 
    : id(""), supplierId(""), employeeId(""), importTime(AppClock::now()) {}

inline ImportReceipt::ImportReceipt(std::string id, std::string supId, std::string empId)
    : id(id), supplierId(supId), employeeId(empId), importTime(AppClock::now()) {}

inline std::string ImportReceipt::getID() const { return id; }
inline std::string ImportReceipt::getSupplierId() const { return supplierId; }
inline std::string ImportReceipt::getEmployeeId() const { return employeeId; }
inline const Vector<OrderDetail>& ImportReceipt::getDetails() const { return details; }
inline DateTime ImportReceipt::getImportTime() const { return importTime; }

inline void ImportReceipt::setID(const std::string& receiptId) { id = receiptId; }
inline void ImportReceipt::setSupplierId(const std::string& supId) { supplierId = supId; }
inline void ImportReceipt::setEmployeeId(const std::string& empId) { employeeId = empId; }
inline void ImportReceipt::setImportTime(const DateTime& dt) { importTime = dt; }

inline void ImportReceipt::addDetail(const OrderDetail& detail) {
    details.push_back(detail);
}

inline double ImportReceipt::calculateTotal() const {
    double total = 0.0;
    for (int i = 0; i < details.getSize(); ++i) {
        total += details[i].getSubtotal();
    }
    return total;
}

inline std::string ImportReceipt::toFileString() const {
    std::string line = TextCodec::escapeField(id) + "|" +
                       TextCodec::escapeField(supplierId) + "|" +
                       TextCodec::escapeField(employeeId) + "|" +
                       importTime.toString() + "|";
    for (int i = 0; i < details.getSize(); ++i) {
        line += TextCodec::escapeSubField(details[i].getPhoneId()) + "," +
                TextCodec::escapeSubField(details[i].getPhoneName()) + "," +
                std::to_string(details[i].getQuantity()) + "," +
                StringHelper::formatMoneyDisk(details[i].getUnitPrice());
        if (i < details.getSize() - 1) line += ";";
    }
    return line;
}

// Phân tích dòng dữ liệu từ StockReceipts.txt thành đối tượng ImportReceipt
// Format: id|supplierId|employeeId|importTime|details
inline ImportReceipt ImportReceipt::fromFileLine(const std::string& line) {
    Vector<std::string> parts = TextCodec::splitFields(line, '|');
    if (parts.getSize() < 5) return ImportReceipt(); // Dòng không hợp lệ

    ImportReceipt rec;
    rec.setID(StringHelper::trim(TextCodec::unescape(parts[0])));
    rec.setSupplierId(StringHelper::trim(TextCodec::unescape(parts[1])));
    rec.setEmployeeId(StringHelper::trim(TextCodec::unescape(parts[2])));
    rec.setImportTime(DateTime::fromString(StringHelper::trim(parts[3])));

    // Parse details: phoneId,phoneName,quantity,importPrice;...
    std::string detailStr = parts[4];
    if (!StringHelper::trim(detailStr).empty()) {
        Vector<std::string> detailTokens = TextCodec::splitFields(detailStr, ';');
        for (int i = 0; i < detailTokens.getSize(); ++i) {
            if (StringHelper::trim(detailTokens[i]).empty()) continue;
            Vector<std::string> d = TextCodec::splitFields(detailTokens[i], ',');
            if (d.getSize() >= 4) {
                try {
                    OrderDetail od(
                        StringHelper::trim(TextCodec::unescape(d[0])),
                        TextCodec::unescape(d[1]),
                        std::stoi(StringHelper::trim(d[2])),
                        std::stod(StringHelper::trim(d[3]))
                    );
                    rec.addDetail(od);
                } catch (...) {
                    // Bỏ qua chi tiết lỗi
                }
            }
        }
    }
    return rec;
}

#endif // IMPORTRECEIPT_H
