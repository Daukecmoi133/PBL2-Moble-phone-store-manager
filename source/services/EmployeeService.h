#ifndef EMPLOYEESERVICE_H
#define EMPLOYEESERVICE_H

#include <iostream>
#include <string>
#include <vector>
#include "../models/Employee.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../repositories/EmployeeRepository.h"
#include "../utils/IdGenerator.h"
#include "../utils/StringHelper.h"

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ nhân sự và xác thực tài khoản
class EmployeeService {
private:
    EmployeeRepository& repo;
    HashTable<std::string, Employee*> usernameTable; // Tra cứu O(1) theo tên đăng nhập
    HashTable<std::string, Employee*> idTable;       // Tra cứu O(1) theo mã nhân viên

public:
    EmployeeService(EmployeeRepository& repository);

    void rebuildIndices();
    const Vector<Employee*>& getAllEmployees() const;

    // Xác thực đăng nhập nhân viên với độ phức tạp O(1)
    Employee* authenticate(const std::string& username, const std::string& password);

    Employee* getEmployeeByUsername(const std::string& username);
    Employee* getEmployeeById(const std::string& id);

    // Nghiệp vụ thêm nhân viên mới: kiểm tra tính duy nhất của username & id
    bool addEmployee(Employee* emp, std::string& errorMessage);
    bool addEmployee(Employee* emp);

    bool deleteEmployee(const std::string& id, std::string& errorMessage);
    bool deleteEmployee(const std::string& id);
};

// ================= IMPLEMENTATION =================

inline EmployeeService::EmployeeService(EmployeeRepository& repository) : repo(repository) {
    rebuildIndices();
}

inline void EmployeeService::rebuildIndices() {
    usernameTable.clear();
    idTable.clear();
    const Vector<Employee*>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        if (list[i] != nullptr) {
            usernameTable.insert(list[i]->getAccount().getUsername(), list[i]);
            idTable.insert(list[i]->getID(), list[i]);
        }
    }
}

inline const Vector<Employee*>& EmployeeService::getAllEmployees() const {
    return repo.getAll();
}

inline Employee* EmployeeService::authenticate(const std::string& username, const std::string& password) {
    Employee** ptr = usernameTable.find(username);
    if (ptr != nullptr && *ptr != nullptr) {
        if ((*ptr)->getAccount().authenticate(username, password)) {
            return *ptr;
        }
    }
    return nullptr;
}

inline Employee* EmployeeService::getEmployeeByUsername(const std::string& username) {
    Employee** ptr = usernameTable.find(username);
    return ptr ? *ptr : nullptr;
}

inline Employee* EmployeeService::getEmployeeById(const std::string& id) {
    Employee** ptr = idTable.find(id);
    return ptr ? *ptr : nullptr;
}

inline bool EmployeeService::addEmployee(Employee* emp, std::string& errorMessage) {
    if (emp == nullptr) {
        errorMessage = "Dữ liệu nhân viên không hợp lệ!";
        return false;
    }

    std::string uname = emp->getAccount().getUsername();
    if (uname.empty()) {
        errorMessage = "Tên đăng nhập không được để trống!";
        return false;
    }

    if (emp->getID().empty()) {
        std::vector<std::string> existingIds;
        const Vector<Employee*>& employees = repo.getAll();
        for (int i = 0; i < employees.getSize(); ++i) {
            if (employees[i] != nullptr) existingIds.push_back(employees[i]->getID());
        }
        std::string generatedId;
        if (!IdGenerator::nextId("EMP", "", existingIds, generatedId, errorMessage)) {
            return false;
        }
        emp->setID(generatedId);
    } else if (!IdGenerator::isValidId(emp->getID(), "EMP")) {
        errorMessage = "Mã nhân viên phải đúng định dạng EMP-000001.";
        return false;
    }

    if (emp->getAccount().getID().empty()) {
        std::vector<std::string> accountIds;
        const Vector<Employee*>& employees = repo.getAll();
        for (int i = 0; i < employees.getSize(); ++i) {
            if (employees[i] != nullptr) {
                accountIds.push_back(employees[i]->getAccount().getID());
            }
        }
        std::string generatedAccountId;
        if (!IdGenerator::nextId("ACC", "", accountIds,
                                 generatedAccountId, errorMessage)) {
            return false;
        }
        emp->getAccount().setID(generatedAccountId);
    } else if (!IdGenerator::isValidId(emp->getAccount().getID(), "ACC")) {
        errorMessage = "Mã tài khoản phải đúng định dạng ACC-000001.";
        return false;
    }

    if (usernameTable.contains(uname)) {
        errorMessage = "Tên đăng nhập [" + uname + "] đã được sử dụng!";
        return false;
    }

    if (idTable.contains(emp->getID())) {
        errorMessage = "Mã nhân viên [" + emp->getID() + "] đã tồn tại!";
        return false;
    }

    if (!repo.addEmployee(emp)) {
        errorMessage = "Lỗi khi lưu nhân viên vào cơ sở dữ liệu!";
        return false;
    }

    usernameTable.insert(uname, emp);
    idTable.insert(emp->getID(), emp);
    return true;
}

inline bool EmployeeService::addEmployee(Employee* emp) {
    std::string err;
    return addEmployee(emp, err);
}

inline bool EmployeeService::deleteEmployee(const std::string& id, std::string& errorMessage) {
    if (!idTable.contains(id)) {
        errorMessage = "Không tìm thấy nhân viên có mã [" + id + "]!";
        return false;
    }

    if (!repo.deleteEmployee(id)) {
        errorMessage = "Lỗi khi xóa nhân viên khỏi cơ sở dữ liệu!";
        return false;
    }

    rebuildIndices();
    return true;
}

inline bool EmployeeService::deleteEmployee(const std::string& id) {
    std::string err;
    return deleteEmployee(id, err);
}

#endif // EMPLOYEESERVICE_H
