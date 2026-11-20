#ifndef AUTH_VIEWMODELS_H
#define AUTH_VIEWMODELS_H

#include <string>
#include "OperationResult.h"
#include "../../source/models/Common.h"

// Thông tin phiên đăng nhập người dùng hiện tại
struct UserSessionViewModel {
    std::string userId;
    std::string username;
    std::string fullName;
    RoleType role;
    std::string roleName;
    std::string phone;
    std::string email;
    std::string address;
    bool isAuthenticated;

    UserSessionViewModel()
        : role(RoleType::CUSTOMER), isAuthenticated(false) {}

    bool isAdmin() const { return role == RoleType::ADMIN; }
    bool isSales() const { return role == RoleType::SALE_EMPLOYEE; }
    bool isWarehouse() const { return role == RoleType::WAREHOUSE_EMPLOYEE; }
    bool isCustomer() const { return role == RoleType::CUSTOMER; }
    bool isEmployee() const { return role != RoleType::CUSTOMER; }
};

// Yêu cầu đăng nhập
struct LoginRequest {
    std::string username;
    std::string password;
};

// Kết quả đăng nhập trả về UI
struct LoginResponse {
    OperationResult result;
    UserSessionViewModel session;
};

#endif // AUTH_VIEWMODELS_H
