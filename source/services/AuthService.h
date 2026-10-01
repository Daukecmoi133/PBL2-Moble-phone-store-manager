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
// Không tiết lộ lý do cụ thể (sai username hay sai password) để bảo mật
inline LoginResult AuthService::login(const std::string& username, const std::string& password) {
    LoginResult result;

    // Validate input cơ bản
    std::string uname = username;
    // trim đơn giản (tránh phụ thuộc StringHelper ở đây)
    while (!uname.empty() && (uname.front() == ' ' || uname.front() == '\t')) uname.erase(uname.begin());
    while (!uname.empty() && (uname.back() == ' ' || uname.back() == '\t')) uname.pop_back();

    if (uname.empty() || password.empty()) {
        result.errorMessage = "Ten dang nhap va mat khau khong duoc de trong!";
        return result;
    }

    // 1. Tìm trong nhân viên trước (gọi EmployeeService::authenticate)
    Employee* emp = empService.authenticate(uname, password);
    if (emp != nullptr) {
        result.success = true;
        result.role = emp->getRoleType();
        result.userId = emp->getID();
        result.fullName = emp->getName().getFullName();
        result.errorMessage = "Dang nhap thanh cong!";
        return result;
    }

    // 2. Tìm trong khách hàng (gọi CustomerService::authenticate)
    Customer* cust = custService.authenticate(uname, password);
    if (cust != nullptr) {
        result.success = true;
        result.role = RoleType::CUSTOMER;
        result.userId = cust->getID();
        result.fullName = cust->getName().getFullName();
        result.errorMessage = "Dang nhap thanh cong!";
        return result;
    }

    // Không tìm thấy ở đâu → thất bại (không nói rõ lý do)
    result.errorMessage = "Ten dang nhap hoac mat khau khong chinh xac!";
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