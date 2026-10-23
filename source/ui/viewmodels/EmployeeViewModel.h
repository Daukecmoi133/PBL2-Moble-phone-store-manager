#ifndef EMPLOYEE_VIEWMODEL_H
#define EMPLOYEE_VIEWMODEL_H

#include <string>
#include "../../models/Common.h"

// ViewModel hiển thị một nhân viên
struct EmployeeItemViewModel {
    std::string id;
    std::string fullName;
    std::string firstName;
    std::string middleName;
    std::string lastName;
    std::string phone;
    std::string email;
    std::string username;
    RoleType role;
    std::string roleName;
    std::string address;
    double salary;
    bool isActive;

    EmployeeItemViewModel() : role(RoleType::SALE_EMPLOYEE), salary(0.0), isActive(true) {}
};

// Form tạo / sửa thông tin nhân viên
struct EmployeeFormRequest {
    std::string id;
    std::string firstName;
    std::string middleName;
    std::string lastName;
    std::string phone;
    std::string email;
    std::string address;
    std::string username;
    std::string password;
    RoleType role;
    double salary;

    EmployeeFormRequest() : role(RoleType::SALE_EMPLOYEE), salary(0.0) {}
};

#endif // EMPLOYEE_VIEWMODEL_H
