#include "EmployeeService.h"

EmployeeService::EmployeeService(EmployeeRepository& repo) : employeeRepo(repo) {}

Result<void> EmployeeService::addEmployee(const Actor&, const Employee&) {
    // TODO(logic): kiểm tra quyền ManageEmployee, hợp lệ dữ liệu, trùng username, add + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: EmployeeService::addEmployee");
}

Result<void> EmployeeService::updateEmployee(const Actor&, const Employee&) {
    // TODO(logic): kiểm tra quyền ManageEmployee, update + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: EmployeeService::updateEmployee");
}

Result<void> EmployeeService::removeEmployee(const Actor&, const std::string&) {
    // TODO(logic): kiểm tra quyền ManageEmployee và ràng buộc tham chiếu trước khi xóa.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: EmployeeService::removeEmployee");
}

Result<Vector<std::unique_ptr<Employee>>> EmployeeService::listAll(const Actor&) const {
    // TODO(logic): kiểm tra quyền ManageEmployee rồi trả danh sách bản sao.
    return Result<Vector<std::unique_ptr<Employee>>>::fail(ErrorCode::NotImplemented,
                                                           "TODO: EmployeeService::listAll");
}

Result<std::unique_ptr<Employee>> EmployeeService::getEmployeeById(const Actor&, const std::string&) const {
    // TODO(logic): kiểm tra quyền rồi findById.
    return Result<std::unique_ptr<Employee>>::fail(ErrorCode::NotImplemented,
                                                   "TODO: EmployeeService::getEmployeeById");
}

Result<Actor> EmployeeService::authenticate(const std::string&, const std::string&) const {
    // TODO(logic): findByUsername, đối chiếu mật khẩu, kiểm tra active, trả Actor{id, role}.
    return Result<Actor>::fail(ErrorCode::NotImplemented, "TODO: EmployeeService::authenticate");
}

Result<void> EmployeeService::assignRole(const Actor&, const std::string&, Role) {
    // TODO(logic): kiểm tra quyền AssignRole, đổi vai trò, save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: EmployeeService::assignRole");
}

Result<void> EmployeeService::setActive(const Actor&, const std::string&, bool) {
    // TODO(logic): kiểm tra quyền ManageEmployee, khóa/mở tài khoản, save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: EmployeeService::setActive");
}
