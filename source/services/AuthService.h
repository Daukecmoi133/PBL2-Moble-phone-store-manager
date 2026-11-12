// AUTH SERVICE - Tầng xác thực và phân quyền đăng nhập
// AuthService điều phối EmployeeService và CustomerService để xác thực;
// KHÔNG copy lại logic credential – chỉ gọi các service đã có.
#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <string>
#include "../models/Employee.h"
#include "../models/Customer.h"
#include "EmployeeService.h"
#include "CustomerService.h"

// Kết quả đăng nhập – chứa đủ thông tin để UI phân luồng sau login
struct LoginResult {
    bool success;           // Đăng nhập thành công hay không
    RoleType role;          // Chức vụ của người dùng
    std::string userId;     // ID thực thể (Employee ID hoặc Customer ID)
    std::string fullName;   // Tên hiển thị
    std::string errorMessage; // Thông báo lỗi nếu thất bại

    LoginResult() : success(false), role(RoleType::CUSTOMER) {}
};

// AuthService: Điều phối xác thực từ EmployeeService + CustomerService
// Ưu tiên kiểm tra Employee trước (hệ thống quản lý ưu tiên nhân viên)
class AuthService {
private:
    EmployeeService& empService;
    CustomerService& custService;

public:
    AuthService(EmployeeService& es, CustomerService& cs);

    // Đăng nhập: tìm trong Employee trước, rồi Customer
    // Trả về LoginResult với đầy đủ thông tin về tài khoản
    LoginResult login(const std::string& username, const std::string& password);

    // Kiểm tra username đã tồn tại trong hệ thống chưa (cả 2 nhóm)
    bool usernameExists(const std::string& username) const;

    // Lấy Employee đang đăng nhập (nullptr nếu không phải nhân viên)
    Employee* getEmployee(const std::string& username);

    // Lấy Customer đang đăng nhập (nullptr nếu không phải khách hàng)
    Customer* getCustomer(const std::string& username);
};

// ================= IMPLEMENTATION =================

inline AuthService::AuthService(EmployeeService& es, CustomerService& cs)
    : empService(es), custService(cs) {}

// Đăng nhập: kiểm tra nhân viên trước, sau đó khách hàng
// Phân biệt rõ lý do thất bại (tài khoản không tồn tại, bị vô hiệu hóa, sai mật khẩu, lỗi I/O)
inline LoginResult AuthService::login(const std::string& username, const std::string& password) {
    LoginResult result;

    // Validate input cơ bản
    std::string uname = username;
    // trim đơn giản (tránh phụ thuộc StringHelper ở đây)
    while (!uname.empty() && (uname.front() == ' ' || uname.front() == '\t')) uname.erase(uname.begin());
    while (!uname.empty() && (uname.back() == ' ' || uname.back() == '\t')) uname.pop_back();

    if (uname.empty() || password.empty()) {
        result.errorMessage = "Tên đăng nhập và mật khẩu không được để trống!";
        return result;
    }

    if (empService.hasLoadError() || custService.hasLoadError()) {
        result.errorMessage = "Lỗi đọc dữ liệu tài khoản từ tệp tin!";
        return result;
    }

    // 1. Tìm trong nhân viên trước
    Employee* emp = empService.getEmployeeByUsername(uname);
    if (emp != nullptr) {
        if (!emp->getAccount().isActive()) {
            result.errorMessage = "Tài khoản đã bị vô hiệu hóa!";
            return result;
        }
        if (!emp->getAccount().verifyPassword(password)) {
            result.errorMessage = "Mật khẩu không chính xác!";
            return result;
        }
        result.success = true;
        result.role = emp->getRoleType();
        result.userId = emp->getID();
        result.fullName = emp->getName().getFullName();
        result.errorMessage = "Đăng nhập thành công!";
        return result;
    }

    // 2. Tìm trong khách hàng
    Customer* cust = custService.getCustomerByUsername(uname);
    if (cust != nullptr) {
        if (!cust->getAccount().isActive()) {
            result.errorMessage = "Tài khoản đã bị vô hiệu hóa!";
            return result;
        }
        if (!cust->getAccount().verifyPassword(password)) {
            result.errorMessage = "Mật khẩu không chính xác!";
            return result;
        }
        result.success = true;
        result.role = RoleType::CUSTOMER;
        result.userId = cust->getID();
        result.fullName = cust->getName().getFullName();
        result.errorMessage = "Đăng nhập thành công!";
        return result;
    }

    // Không tìm thấy ở đâu → thất bại
    result.errorMessage = "Tài khoản không tồn tại hoặc đã bị khóa!";
    return result;
}

// Kiểm tra username đã tồn tại trong hệ thống chưa
inline bool AuthService::usernameExists(const std::string& username) const {
    return empService.getEmployeeByUsername(username) != nullptr ||
           custService.getCustomerByUsername(username) != nullptr;
}

// Lấy Employee theo username (nullptr nếu không phải nhân viên)
inline Employee* AuthService::getEmployee(const std::string& username) {
    return empService.getEmployeeByUsername(username);
}

// Lấy Customer theo username (nullptr nếu không phải khách hàng)
inline Customer* AuthService::getCustomer(const std::string& username) {
    return custService.getCustomerByUsername(username);
}

#endif // AUTHSERVICE_H