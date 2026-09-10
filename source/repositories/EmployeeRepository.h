#ifndef EMPLOYEEREPOSITORY_H
#define EMPLOYEEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Employee.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../utils/StringHelper.h"

class EmployeeRepository {
private:
    std::string filePath;
    Vector<Employee*> employees;
    HashTable<std::string, Employee*> usernameTable; // Băm theo username để đăng nhập O(1)

    void rebuildIndices() {
        usernameTable.clear();
        for (int i = 0; i < employees.getSize(); ++i) {
            usernameTable.insert(employees[i]->getAccount().getUsername(), employees[i]);
        }
    }

    void clearMemory() {
        for (int i = 0; i < employees.getSize(); ++i) {
            delete employees[i];
        }
        employees.clear();
        usernameTable.clear();
    }

public:
    EmployeeRepository(std::string path = "data/people/Employee.txt") : filePath(path) {}

    ~EmployeeRepository() {
        clearMemory();
    }

    // Không cho copy nông tránh double free
    EmployeeRepository(const EmployeeRepository&) = delete;
    EmployeeRepository& operator=(const EmployeeRepository&) = delete;

    bool loadFromFile() {
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
                    // Mặc định hoặc bán hàng
                    emp = new SalesEmployee(id, name, phone, email, address, username, password);
                }

                employees.push_back(emp);
            }
        }
        file.close();

        rebuildIndices();
        return true;
    }

    bool saveToFile() const {
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

    const Vector<Employee*>& getAll() const {
        return employees;
    }

    Employee* findByUsername(const std::string& username) {
        Employee** ptr = usernameTable.find(username);
        return ptr ? *ptr : nullptr;
    }

    Employee* authenticate(const std::string& username, const std::string& password) {
        Employee* emp = findByUsername(username);
        if (emp != nullptr && emp->getAccount().authenticate(username, password)) {
            return emp;
        }
        return nullptr;
    }

    bool addEmployee(Employee* emp) {
        if (emp == nullptr) return false;
        if (usernameTable.contains(emp->getAccount().getUsername())) {
            return false; // Trùng username
        }
        employees.push_back(emp);
        usernameTable.insert(emp->getAccount().getUsername(), emp);
        saveToFile();
        return true;
    }
};

#endif // EMPLOYEEREPOSITORY_H
