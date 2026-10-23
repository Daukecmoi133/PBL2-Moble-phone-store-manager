#ifndef IMPORTRECEIPT_H
#define IMPORTRECEIPT_H

#include <string>
#include <cmath>
#include "Order.h"
#include "../utils/DateTime.h"
#include "../utils/StringHelper.h"
#include "../core/Vector.h"

class ImportReceipt {
private:
    std::string id;
    std::string supplierId;
    std::string employeeId;
    Vector<OrderDetail> details;
    DateTime importTime;
    std::string status; // "HoatDong" hoặc "DaHuy"

public:
    ImportReceipt();
    ImportReceipt(std::string id, std::string supId, std::string empId, std::string status = "HoatDong");

    std::string getID() const;
    std::string getSupplierId() const;
    std::string getEmployeeId() const;
    const Vector<OrderDetail>& getDetails() const;
    DateTime getImportTime() const;
    std::string getStatus() const;

    void setID(const std::string& receiptId);
    void setSupplierId(const std::string& supId);
    void setEmployeeId(const std::string& empId);
    void setImportTime(const DateTime& dt);
    void setStatus(const std::string& st);

    void addDetail(const OrderDetail& detail);
    double calculateTotal() const;
    std::string toFileString() const;

    // Đọc và phân tích một dòng từ StockReceipts.txt
    static ImportReceipt fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

inline ImportReceipt::ImportReceipt() 
    : id(""), supplierId(""), employeeId(""), importTime(DateTime::now()), status("HoatDong") {}

inline ImportReceipt::ImportReceipt(std::string id, std::string supId, std::string empId, std::string status)
    : id(id), supplierId(supId), employeeId(empId), importTime(DateTime::now()), status(status) {}

inline std::string ImportReceipt::getID() const { return id; }
inline std::string ImportReceipt::getSupplierId() const { return supplierId; }
inline std::string ImportReceipt::getEmployeeId() const { return employeeId; }
inline const Vector<OrderDetail>& ImportReceipt::getDetails() const { return details; }
inline DateTime ImportReceipt::getImportTime() const { return importTime; }
inline std::string ImportReceipt::getStatus() const { return status; }

inline void ImportReceipt::setID(const std::string& receiptId) { id = receiptId; }
inline void ImportReceipt::setSupplierId(const std::string& supId) { supplierId = supId; }
inline void ImportReceipt::setEmployeeId(const std::string& empId) { employeeId = empId; }
inline void ImportReceipt::setImportTime(const DateTime& dt) { importTime = dt; }
inline void ImportReceipt::setStatus(const std::string& st) { status = st; }

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

// Định dạng lưu trữ mới: id|supplierId|employeeId|importTime|details|status
// Trong đó details: productId,soLuong,donGia (bỏ tên sản phẩm)
inline std::string ImportReceipt::toFileString() const {
    std::string line = id + "|" + supplierId + "|" + employeeId + "|" + importTime.toString() + "|";
    for (int i = 0; i < details.getSize(); ++i) {
        long long prLL = static_cast<long long>(std::round(details[i].getUnitPrice()));
        line += details[i].getPhoneId() + "," +
                std::to_string(details[i].getQuantity()) + "," +
                std::to_string(prLL);
        if (i < details.getSize() - 1) line += ";";
    }
    line += "|" + (status.empty() ? "HoatDong" : status);
    return line;
}

// Phân tích dòng dữ liệu từ StockReceipts.txt thành đối tượng ImportReceipt
// Format mới: id|supplierId|employeeId|importTime|details|status
inline ImportReceipt ImportReceipt::fromFileLine(const std::string& line) {
    Vector<std::string> parts = StringHelper::split(line, '|');
    if (parts.getSize() < 5) return ImportReceipt(); // Dòng không hợp lệ

    ImportReceipt rec;
    rec.setID(StringHelper::trim(parts[0]));
    rec.setSupplierId(StringHelper::trim(parts[1]));
    rec.setEmployeeId(StringHelper::trim(parts[2]));
    rec.setImportTime(DateTime::fromString(StringHelper::trim(parts[3])));

    // Parse details: productId,quantity,importPrice (hoặc định dạng cũ có phoneName)
    std::string detailStr = StringHelper::trim(parts[4]);
    if (!detailStr.empty()) {
        Vector<std::string> detailTokens = StringHelper::split(detailStr, ';');
        for (int i = 0; i < detailTokens.getSize(); ++i) {
            Vector<std::string> d = StringHelper::split(detailTokens[i], ',');
            if (d.getSize() >= 4) {
                // Định dạng cũ: phoneId,phoneName,quantity,importPrice
                OrderDetail od(
                    StringHelper::trim(d[0]),
                    StringHelper::trim(d[1]),
                    std::stoi(StringHelper::trim(d[2])),
                    StringHelper::parsePrice(d[3])
                );
                rec.addDetail(od);
            } else if (d.getSize() == 3) {
                // Định dạng mới: phoneId,quantity,importPrice
                OrderDetail od(
                    StringHelper::trim(d[0]),
                    "",
                    std::stoi(StringHelper::trim(d[1])),
                    StringHelper::parsePrice(d[2])
                );
                rec.addDetail(od);
            }
        }
    }

    if (parts.getSize() >= 6) {
        rec.setStatus(StringHelper::trim(parts[5]));
    } else {
        rec.setStatus("HoatDong");
    }

    return rec;
}

#endif // IMPORTRECEIPT_H
