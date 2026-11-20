// ACCOUNT REPOSITORY - Tầng truy xuất dữ liệu tài khoản (xác thực đăng nhập)
// AccountRepository không có file riêng - tài khoản được nhúng trong Customer và Employee.
// Lớp này hoạt động như một facade: tổng hợp tìm kiếm từ CustomerRepository và EmployeeRepository.
#ifndef ACCOUNTREPOSITORY_H
#define ACCOUNTREPOSITORY_H

#include <iostream>
#include <string>
#include "../models/Account.h"
#include "../models/Customer.h"
#include "../models/Employee.h"
#include "../core/Vector.h"

// AccountRepository: Không đọc/ghi file riêng.
// Nhận tham chiếu đến CustomerRepository và EmployeeRepository để tra cứu tài khoản.
// Dùng chủ yếu trong AuthService để xác thực đăng nhập.
class AccountRepository {
private:
    Vector<Customer>* customers;    // Tham chiếu đến danh sách khách hàng (không sở hữu)
    Vector<Employee*>* employees;   // Tham chiếu đến danh sách nhân viên (không sở hữu)

public:
    // Hàm dựng: nhận tham chiếu đến hai danh sách từ ngoài
    // Lưu ý: AccountRepository KHÔNG sở hữu các con trỏ này
    AccountRepository(Vector<Customer>* custList = nullptr, 
                      Vector<Employee*>* empList = nullptr);

    // Thiết lập lại nguồn dữ liệu
    void setCustomers(Vector<Customer>* custList);
    void setEmployees(Vector<Employee*>* empList);

    // Xác thực đăng nhập: tìm trong danh sách khách hàng và nhân viên theo Username hoặc Số điện thoại (khóa tài khoản)
    // Trả về RoleType của tài khoản nếu thành công, CUSTOMER là giá trị fail-safe
    // outId: ID của đối tượng tìm được (Customer ID hoặc Employee ID)
    bool authenticate(const std::string& usernameOrPhone, const std::string& password,
                      RoleType& outRole, std::string& outId) const;

    // Tìm tài khoản nhân viên theo username
    Account* findEmployeeAccount(const std::string& username);

    // Tìm tài khoản nhân viên theo số điện thoại (khóa)
    Account* findEmployeeAccountByPhone(const std::string& phone);

    // Tìm tài khoản khách hàng theo username
    Account* findCustomerAccount(const std::string& username);

    // Tìm tài khoản khách hàng theo số điện thoại (khóa)
    Account* findCustomerAccountByPhone(const std::string& phone);

    // Kiểm tra username đã tồn tại trong hệ thống chưa (cả Customer lẫn Employee)
    bool usernameExists(const std::string& username) const;

    // Kiểm tra số điện thoại đã tồn tại cho bất kỳ tài khoản nào chưa (duy nhất toàn hệ thống)
    bool phoneExists(const std::string& phone) const;
};

// ================= IMPLEMENTATION =================

inline AccountRepository::AccountRepository(Vector<Customer>* custList,
                                             Vector<Employee*>* empList)
    : customers(custList), employees(empList) {}

inline void AccountRepository::setCustomers(Vector<Customer>* custList) {
    customers = custList;
}

inline void AccountRepository::setEmployees(Vector<Employee*>* empList) {
    employees = empList;
}

// Xác thực đăng nhập: kiểm tra trong cả Employee và Customer theo username hoặc số điện thoại (khóa)
// Ưu tiên Employee trước (nhân viên đăng nhập hệ thống quản lý)
inline bool AccountRepository::authenticate(const std::string& usernameOrPhone,
                                             const std::string& password,
                                             RoleType& outRole,
                                             std::string& outId) const {
    // Tìm trong danh sách nhân viên trước (theo username hoặc số điện thoại)
    if (employees != nullptr) {
        for (int i = 0; i < employees->getSize(); ++i) {
            Employee* emp = (*employees)[i];
            if (emp) {
                bool matchesUser = (emp->getAccount().getUsername() == usernameOrPhone);
                bool matchesPhone = (!emp->getPhoneNumber().empty() && emp->getPhoneNumber() == usernameOrPhone);
                if ((matchesUser || matchesPhone) && emp->getAccount().isActive() && emp->getAccount().verifyPassword(password)) {
                    outRole = emp->getRoleType();
                    outId = emp->getID();
                    return true;
                }
            }
        }
    }
    // Nếu không tìm thấy trong nhân viên, tìm trong khách hàng (theo username hoặc số điện thoại)
    if (customers != nullptr) {
        for (int i = 0; i < customers->getSize(); ++i) {
            const Customer& cust = (*customers)[i];
            bool matchesUser = (cust.getAccount().getUsername() == usernameOrPhone);
            bool matchesPhone = (!cust.getPhoneNumber().empty() && cust.getPhoneNumber() == usernameOrPhone);
            if ((matchesUser || matchesPhone) && cust.getAccount().isActive() && cust.getAccount().verifyPassword(password)) {
                outRole = RoleType::CUSTOMER;
                outId = cust.getID();
                return true;
            }
        }
    }
    return false; // Không tìm thấy tài khoản khớp
}

// Tìm Account của nhân viên theo username (trả về con trỏ để có thể sửa)
inline Account* AccountRepository::findEmployeeAccount(const std::string& username) {
    if (employees == nullptr) return nullptr;
    for (int i = 0; i < employees->getSize(); ++i) {
        Employee* emp = (*employees)[i];
        if (emp && emp->getAccount().getUsername() == username) {
            return &emp->getAccount();
        }
    }
    return nullptr;
}

// Tìm Account của nhân viên theo số điện thoại (khóa)
inline Account* AccountRepository::findEmployeeAccountByPhone(const std::string& phone) {
    if (employees == nullptr || phone.empty()) return nullptr;
    for (int i = 0; i < employees->getSize(); ++i) {
        Employee* emp = (*employees)[i];
        if (emp && emp->getPhoneNumber() == phone) {
            return &emp->getAccount();
        }
    }
    return nullptr;
}

// Tìm Account của khách hàng theo username (trả về con trỏ để có thể sửa)
inline Account* AccountRepository::findCustomerAccount(const std::string& username) {
    if (customers == nullptr) return nullptr;
    for (int i = 0; i < customers->getSize(); ++i) {
        if ((*customers)[i].getAccount().getUsername() == username) {
            return &(*customers)[i].getAccount();
        }
    }
    return nullptr;
}

// Tìm Account của khách hàng theo số điện thoại (khóa)
inline Account* AccountRepository::findCustomerAccountByPhone(const std::string& phone) {
    if (customers == nullptr || phone.empty()) return nullptr;
    for (int i = 0; i < customers->getSize(); ++i) {
        if ((*customers)[i].getPhoneNumber() == phone) {
            return &(*customers)[i].getAccount();
        }
    }
    return nullptr;
}

// Kiểm tra username đã tồn tại trong hệ thống chưa
inline bool AccountRepository::usernameExists(const std::string& username) const {
    if (employees != nullptr) {
        for (int i = 0; i < employees->getSize(); ++i) {
            if ((*employees)[i] && (*employees)[i]->getAccount().getUsername() == username)
                return true;
        }
    }
    if (customers != nullptr) {
        for (int i = 0; i < customers->getSize(); ++i) {
            if ((*customers)[i].getAccount().getUsername() == username)
                return true;
        }
    }
    return false;
}

// Kiểm tra số điện thoại đã tồn tại cho bất kỳ tài khoản nào chưa
inline bool AccountRepository::phoneExists(const std::string& phone) const {
    if (phone.empty()) return false;
    if (employees != nullptr) {
        for (int i = 0; i < employees->getSize(); ++i) {
            if ((*employees)[i] && (*employees)[i]->getPhoneNumber() == phone)
                return true;
        }
    }
    if (customers != nullptr) {
        for (int i = 0; i < customers->getSize(); ++i) {
            if ((*customers)[i].getPhoneNumber() == phone)
                return true;
        }
    }
    return false;
}

#endif // ACCOUNTREPOSITORY_H