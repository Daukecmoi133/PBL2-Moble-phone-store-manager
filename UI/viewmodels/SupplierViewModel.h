#ifndef SUPPLIER_VIEWMODEL_H
#define SUPPLIER_VIEWMODEL_H

#include <string>

// ViewModel hiển thị một nhà cung cấp
struct SupplierItemViewModel {
    std::string id;
    std::string companyName;
    std::string taxCode;
    std::string contactPerson;
    std::string phone;
    std::string email;
    std::string address;
};

// Form tạo / sửa nhà cung cấp
struct SupplierFormRequest {
    std::string id;
    std::string companyName;
    std::string taxCode;
    std::string contactPerson;
    std::string phone;
    std::string email;
    std::string address;
};

#endif // SUPPLIER_VIEWMODEL_H
