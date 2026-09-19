#ifndef EMPLOYEEREPOSITORY_H
#define EMPLOYEEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Employee.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu nhân viên từ/vào tệp tin
class EmployeeRepository {
private:
    std::string filePath;
    Vector<Employee*> employees;

    void clearMemory();

public:
    EmployeeRepository(std::string path = "data/people/Employee.txt");
    ~EmployeeRepository();

    // Không cho copy nông tránh double free
    EmployeeRepository(const EmployeeRepository&) = delete;
    EmployeeRepository& operator=(const EmployeeRepository&) = delete;

    bool loadFromFile();
    bool saveToFile() const;

    const Vector<Employee*>& getAll() const;
    Vector<Employee*>& getAll();

    Employee* findById(const std::string& id);
    const Employee* findById(const std::string& id) const;

    bool addEmployee(Employee* emp);
    bool updateEmployee(Employee* updatedEmp);
    bool deleteEmployee(const std::string& id);
};

// ================= IMPLEMENTATION =================

inline void EmployeeRepository::clearMemory() {
    for (int i = 0; i < employees.getSize(); ++i) {
        delete employees[i];
    }
    employees.clear();
}

inline EmployeeRepository::EmployeeRepository(std::string path) : filePath(path) {}

inline EmployeeRepository::~EmployeeRepository() {
    clearMemory();
}

inline bool EmployeeRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Cảnh báo] Không thể mở file: " << filePath << std::endl;
        return false;
    }

    clearMemory();
    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        line = StringHelper::trim(line);
        if (line.empty() || line == "...") continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        Vector<std::string> parts = StringHelper::split(line, '|');
        if (parts.getSize() >= 10) {
            std::string id = StringHelper::trim(parts[0]);
            Name name(StringHelper::trim(parts[1]), 
                      StringHelper::trim(parts[2]), 
                      StringHelper::trim(parts[3]));
            std::string phone = StringHelper::trim(parts[4]);
            std::string email = StringHelper::trim(parts[5]);
            std::string username = StringHelper::trim(parts[6]);
            std::string password = StringHelper::trim(parts[7]);
            std::string roleStr = StringHelper::trim(parts[8]);
            std::string address = StringHelper::trim(parts[9]);

            int roleId = 1;
            try {
                roleId = std::stoi(roleStr);
            } catch (...) {
                roleId = 1;
            }

            Employee* emp = nullptr;
            if (roleId == 1) {
                emp = new AdminEmployee(id, name, phone, email, address, username, password);
            } else if (roleId == 2) {
                emp = new SalesEmployee(id, name, phone, email, address, username, password);
            } else if (roleId == 3) {
                emp = new WarehouseEmployee(id, name, phone, email, address, username, password);
            } else {
                emp = new SalesEmployee(id, name, phone, email, address, username, password);
            }

            employees.push_back(emp);
        }
    }
    file.close();
    return true;
}

inline bool EmployeeRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address\n";
    for (int i = 0; i < employees.getSize(); ++i) {
        file << employees[i]->toFileString() << "\n";
    }
    file.close();
    return true;
}

inline const Vector<Employee*>& EmployeeRepository::getAll() const {
    return employees;
}

inline Vector<Employee*>& EmployeeRepository::getAll() {
    return employees;
}

inline Employee* EmployeeRepository::findById(const std::string& id) {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i]->getID() == id) {
            return employees[i];
        }
    }
    return nullptr;
}

inline const Employee* EmployeeRepository::findById(const std::string& id) const {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i]->getID() == id) {
            return employees[i];
        }
    }
    return nullptr;
}

inline bool EmployeeRepository::addEmployee(Employee* emp) {
    if (emp == nullptr) return false;
    employees.push_back(emp);
    return saveToFile();
}

inline bool EmployeeRepository::updateEmployee(Employee* updatedEmp) {
    if (updatedEmp == nullptr) return false;
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i]->getID() == updatedEmp->getID()) {
            delete employees[i];
            employees[i] = updatedEmp;
            return saveToFile();
        }
    }
    return false;
}

inline bool EmployeeRepository::deleteEmployee(const std::string& id) {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i]->getID() == id) {
            delete employees[i];
            employees.removeAt(i);
            return saveToFile();
        }
    }
    return false;
}

#endif // EMPLOYEEREPOSITORY_H
