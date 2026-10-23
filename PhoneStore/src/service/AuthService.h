#pragma once
// Đăng nhập: gọi CustomerService rồi EmployeeService (Auth chỉ gọi Customer và Employee). Stub.

#include <string>

#include "CustomerService.h"
#include "EmployeeService.h"
#include "../common/Actor.h"
#include "../common/Result.h"

class AuthService {
public:
    AuthService(CustomerService& customers, EmployeeService& employees);

    Result<Actor> login(const std::string& username, const std::string& password) const;

private:
    CustomerService& customerService;
    EmployeeService& employeeService;
};
