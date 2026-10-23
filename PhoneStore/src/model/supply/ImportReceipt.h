#ifndef IMPORTRECEIPT_H
#define IMPORTRECEIPT_H

#include <string>
// FIX LỖI: đổi include theo cây thư mục mới
#include "../sales/Order.h"
#include "../../common/DateTime.h"
#include "../../common/StringUtil.h"
#include "../../storage/ds/StringSplit.h"
#include "../../storage/ds/Vector.h"

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
    : id(""), supplierId(""), employeeId(""), importTime(DateTime::now()) {}

inline ImportReceipt::ImportReceipt(std::string id, std::string supId, std::string empId)
    : id(id), supplierId(supId), employeeId(empId), importTime(DateTime::now()) {}

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
    std::string line = id + "|" + supplierId + "|" + employeeId + "|" + importTime.toString() + "|";
    for (int i = 0; i < details.getSize(); ++i) {
        line += details[i].getPhoneId() + "," + details[i].getPhoneName() + "," +
                std::to_string(details[i].getQuantity()) + "," +
                std::to_string(details[i].getUnitPrice());
        if (i < details.getSize() - 1) line += ";";
    }
    return line;
}

// Phân tích dòng dữ liệu từ StockReceipts.txt thành đối tượng ImportReceipt
// Format: id|supplierId|employeeId|importTime|details
inline ImportReceipt ImportReceipt::fromFileLine(const std::string& line) {
    Vector<std::string> parts = StringHelper::split(line, '|');
    if (parts.getSize() < 5) return ImportReceipt(); // Dòng không hợp lệ

    ImportReceipt rec;
    rec.setID(StringHelper::trim(parts[0]));
    rec.setSupplierId(StringHelper::trim(parts[1]));
    rec.setEmployeeId(StringHelper::trim(parts[2]));
    rec.setImportTime(DateTime::fromString(StringHelper::trim(parts[3])));

    // Parse details: phoneId,phoneName,quantity,importPrice;...
    std::string detailStr = StringHelper::trim(parts[4]);
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
                rec.addDetail(od);
            }
        }
    }
    return rec;
}

#endif // IMPORTRECEIPT_H
