#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "People.h"
#include "Account.h"

class Employee : public People {
protected:
    Account account;
    double salary;

public:
    Employee() : People(), account("", "", RoleType::SALE_EMPLOYEE), salary(0.0) {}

    Employee(std::string id, Name name, std::string phone, std::string email, 
             std::string address, std::string username, std::string password, 
             RoleType roleType, double salary = 0.0)
        : People(id, name, phone, email, address), 
          account(username, password, roleType), salary(salary) {}

    virtual ~Employee() = default;

    Account& getAccount() { return account; }
    const Account& getAccount() const { return account; }
    void setAccount(const Account& acc) { account = acc; }

    double getSalary() const { return salary; }
    void setSalary(double s) { salary = s; }

    // Phương thức thuần ảo biểu diễn tính Đa hình (Polymorphism)
    virtual std::string getRole() const = 0;
    virtual RoleType getRoleType() const = 0;

    virtual std::string toFileString() const {
        return id + "|" + name.firstName + "|" + name.middleName + "|" + name.lastName + "|" +
               phoneNumber + "|" + email + "|" + account.getUsername() + "|******|" +
               std::to_string(static_cast<int>(getRoleType())) + "|" + address;
    }
};

class AdminEmployee : public Employee {
public:
    AdminEmployee() : Employee() {
        account.setRole(Role(RoleType::ADMIN));
    }

    AdminEmployee(std::string id, Name name, std::string phone, std::string email, 
                  std::string address, std::string username, std::string password, double salary = 0.0)
        : Employee(id, name, phone, email, address, username, password, RoleType::ADMIN, salary) {}

    std::string getRole() const override {
        return "Quản trị viên (Admin)";
    }

    RoleType getRoleType() const override {
        return RoleType::ADMIN;
    }
};

class SalesEmployee : public Employee {
public:
    SalesEmployee() : Employee() {
        account.setRole(Role(RoleType::SALE_EMPLOYEE));
    }

    SalesEmployee(std::string id, Name name, std::string phone, std::string email, 
                  std::string address, std::string username, std::string password, double salary = 0.0)
        : Employee(id, name, phone, email, address, username, password, RoleType::SALE_EMPLOYEE, salary) {}

    std::string getRole() const override {
        return "Nhân viên Bán hàng";
    }

    RoleType getRoleType() const override {
        return RoleType::SALE_EMPLOYEE;
    }
};

class WarehouseEmployee : public Employee {
public:
    WarehouseEmployee() : Employee() {
        account.setRole(Role(RoleType::WAREHOUSE_EMPLOYEE));
    }

    WarehouseEmployee(std::string id, Name name, std::string phone, std::string email, 
                      std::string address, std::string username, std::string password, double salary = 0.0)
        : Employee(id, name, phone, email, address, username, password, RoleType::WAREHOUSE_EMPLOYEE, salary) {}

    std::string getRole() const override {
        return "Nhân viên Kho";
    }

    RoleType getRoleType() const override {
        return RoleType::WAREHOUSE_EMPLOYEE;
    }
};

#endif // EMPLOYEE_H
