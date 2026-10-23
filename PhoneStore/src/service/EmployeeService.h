#pragma once
// Nghiệp vụ nhân viên. Toàn bộ hàm hiện là stub.

#include <memory>
#include <string>

#include "../common/Actor.h"
#include "../common/Result.h"
#include "../model/enums/Role.h"
#include "../model/people/Employee.h"
#include "../repository/EmployeeRepository.h"
#include "../storage/ds/Vector.h"

class EmployeeService {
public:
    explicit EmployeeService(EmployeeRepository& repo);

    Result<void> addEmployee(const Actor& actor, const Employee& employee);
    Result<void> updateEmployee(const Actor& actor, const Employee& employee);
    Result<void> removeEmployee(const Actor& actor, const std::string& employeeId);
    Result<Vector<std::unique_ptr<Employee>>> listAll(const Actor& actor) const; // bản sao
    Result<std::unique_ptr<Employee>> getEmployeeById(const Actor& actor, const std::string& employeeId) const;

    Result<Actor> authenticate(const std::string& username, const std::string& password) const;
    Result<void> assignRole(const Actor& actor, const std::string& employeeId, Role role);
    Result<void> setActive(const Actor& actor, const std::string& employeeId, bool active);

private:
    EmployeeRepository& employeeRepo;
};
