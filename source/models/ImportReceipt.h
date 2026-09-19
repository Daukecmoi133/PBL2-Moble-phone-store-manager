#ifndef IMPORTRECEIPT_H
#define IMPORTRECEIPT_H

#include <string>
#include "Order.h"
#include "../utils/DateTime.h"
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

    void addDetail(const OrderDetail& detail);
    double calculateTotal() const;
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

#endif // IMPORTRECEIPT_H
