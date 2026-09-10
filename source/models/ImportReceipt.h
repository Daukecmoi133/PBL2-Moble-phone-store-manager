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
    ImportReceipt() 
        : id(""), supplierId(""), employeeId(""), importTime(DateTime::now()) {}

    ImportReceipt(std::string id, std::string supId, std::string empId)
        : id(id), supplierId(supId), employeeId(empId), importTime(DateTime::now()) {}

    std::string getID() const { return id; }
    std::string getSupplierId() const { return supplierId; }
    std::string getEmployeeId() const { return employeeId; }
    const Vector<OrderDetail>& getDetails() const { return details; }
    DateTime getImportTime() const { return importTime; }

    void addDetail(const OrderDetail& detail) {
        details.push_back(detail);
    }

    double calculateTotal() const {
        double total = 0.0;
        for (int i = 0; i < details.getSize(); ++i) {
            total += details[i].getSubtotal();
        }
        return total;
    }
};

#endif // IMPORTRECEIPT_H
