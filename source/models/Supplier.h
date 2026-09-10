#ifndef SUPPLIER_H
#define SUPPLIER_H

#include "People.h"

class Supplier : public People {
private:
    std::string companyName;
    std::string taxCode;

public:
    Supplier() : People(), companyName(""), taxCode("") {}

    Supplier(std::string id, std::string companyName, std::string taxCode, 
             Name contactPerson, std::string phone, std::string email, std::string address)
        : People(id, contactPerson, phone, email, address), 
          companyName(companyName), taxCode(taxCode) {}

    std::string getCompanyName() const { return companyName; }
    void setCompanyName(const std::string& name) { companyName = name; }

    std::string getTaxCode() const { return taxCode; }
    void setTaxCode(const std::string& code) { taxCode = code; }
};

#endif // SUPPLIER_H
