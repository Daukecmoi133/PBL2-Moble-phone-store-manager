// ĐỐI TƯỢNG NHÀ CUNG CẤP (KẾ THỪA TỪ PEOPLE)
#ifndef SUPPLIER_H
#define SUPPLIER_H

#include "People.h"

class Supplier : public People {
private:
    std::string companyName; // Tên công ty / doanh nghiệp cung cấp
    std::string taxCode;     // Mã số thuế doanh nghiệp

public:
    Supplier(); // Hàm dựng mặc định
    Supplier(std::string id, std::string companyName, std::string taxCode, 
             Name contactPerson, std::string phone, std::string email, std::string address); // Hàm dựng đủ tham số

    std::string getCompanyName() const; // Hàm trả về tên công ty
    void setCompanyName(const std::string& name); // Hàm thiết lập tên công ty

    std::string getTaxCode() const; // Hàm trả về mã số thuế
    void setTaxCode(const std::string& code); // Hàm thiết lập mã số thuế
};

// ================= IMPLEMENTATION =================

// Hàm dựng mặc định
inline Supplier::Supplier() : People(), companyName(""), taxCode("") {}

// Hàm dựng đủ tham số
inline Supplier::Supplier(std::string id, std::string companyName, std::string taxCode, 
                          Name contactPerson, std::string phone, std::string email, std::string address)
    : People(id, contactPerson, phone, email, address), 
      companyName(companyName), taxCode(taxCode) {}

// Hàm getter / setter cho tên công ty
inline std::string Supplier::getCompanyName() const { return companyName; }
inline void Supplier::setCompanyName(const std::string& name) { companyName = name; }

// Hàm getter / setter cho mã số thuế
inline std::string Supplier::getTaxCode() const { return taxCode; }
inline void Supplier::setTaxCode(const std::string& code) { taxCode = code; }

#endif // SUPPLIER_H
