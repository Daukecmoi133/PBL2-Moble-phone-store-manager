#include "AuthService.h"

AuthService::AuthService(CustomerService& customers, EmployeeService& employees)
    : customerService(customers), employeeService(employees) {}

Result<Actor> AuthService::login(const std::string&, const std::string&) const {
    // TODO(logic): thử employeeService.authenticate rồi customerService.authenticate;
    // cả hai thất bại thì trả AuthFailed (không tiết lộ username có tồn tại hay không).
    return Result<Actor>::fail(ErrorCode::NotImplemented, "TODO: AuthService::login");
}
