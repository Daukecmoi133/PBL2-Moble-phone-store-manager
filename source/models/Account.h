// ĐỐI TƯỢNG TÀI KHOẢN
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include "Common.h"
#include "../core/Vector.h"

// Lớp Role: Quản lý vai trò người dùng và danh sách các quyền hạn được cấp
class Role {
private:
    RoleType type;                  // Loại vai trò (Admin, Nhân viên, Khách hàng)
    Vector<Permission> permissions; // Danh sách các quyền hạn thuộc vai trò

public:
    Role(RoleType type = RoleType::CUSTOMER); // Hàm dựng gán vai trò và thiết lập quyền mặc định

    RoleType getType() const; // Hàm trả về vai trò hiện tại
    void setType(RoleType t); // Hàm gán lại vai trò
    bool hasPermission(Permission permission) const; // Hàm kiểm tra vai trò có quyền cụ thể hay không
    void addPermission(Permission permission); // Hàm cấp thêm quyền cho vai trò
    void delPermission(Permission permission); // Hàm tước quyền khỏi vai trò
};

// Lớp Account: Quản lý thông tin đăng nhập và trạng thái tài khoản
class Account {
private:
    std::string id;
    std::string username; // Tên đăng nhập
    std::string password; // Mật khẩu tài khoản
    Role role;            // Đối tượng vai trò và quyền hạn của tài khoản
    bool active;          // Trạng thái kích hoạt (true: hoạt động, false: bị khóa)

public:
    Account(); // Hàm dựng mặc định
    Account(std::string username, std::string password,
            RoleType roleType = RoleType::CUSTOMER, std::string id = ""); // Hàm dựng đủ tham số

    std::string getID() const;
    void setID(const std::string& id);
    std::string getUsername() const; // Hàm trả về tên đăng nhập
    std::string getPassword() const; // Hàm trả về mật khẩu
    Role getRole() const; // Hàm trả về đối tượng vai trò Role
    RoleType getRoleType() const; // Hàm trả về loại vai trò RoleType
    void setRole(const Role& r); // Hàm thiết lập vai trò
    void setUsername(const std::string& u); // Hàm thiết lập tên đăng nhập
    void setPassword(const std::string& p); // Hàm thiết lập mật khẩu
    bool isActive() const; // Hàm kiểm tra tài khoản còn hoạt động không
    void setActive(bool a); // Hàm thiết lập trạng thái kích hoạt tài khoản
    bool authenticate(const std::string& user, const std::string& pass) const; // Hàm xác minh tên đăng nhập và mật khẩu
    bool hasPermission(Permission permission) const; // Hàm kiểm tra tài khoản có quyền thực thi chức năng không
};

inline Role :: Role(RoleType type) : type(type) {
    // Khởi tạo các quyền mặc định theo vai trò
    if (type == RoleType::ADMIN) {
        permissions.push_back(Permission::PRODUCT_VIEW);
        permissions.push_back(Permission::PRODUCT_CREATE);
        permissions.push_back(Permission::PRODUCT_UPDATE);
        permissions.push_back(Permission::PRODUCT_DELETE);
        permissions.push_back(Permission::ORDER_VIEW);
        permissions.push_back(Permission::ORDER_CREATE);
        permissions.push_back(Permission::INVOICE_CREATE);
        permissions.push_back(Permission::STOCK_VIEW);
        permissions.push_back(Permission::STOCK_UPDATE);
        permissions.push_back(Permission::EMPLOYEE_VIEW);
        permissions.push_back(Permission::EMPLOYEE_CREATE);
    } else if (type == RoleType::SALE_EMPLOYEE) {
        permissions.push_back(Permission::PRODUCT_VIEW);
        permissions.push_back(Permission::ORDER_VIEW);
        permissions.push_back(Permission::ORDER_CREATE);
        permissions.push_back(Permission::INVOICE_CREATE);
        permissions.push_back(Permission::STOCK_VIEW);
    } else if (type == RoleType::WAREHOUSE_EMPLOYEE) {
        permissions.push_back(Permission::PRODUCT_VIEW);
        permissions.push_back(Permission::STOCK_VIEW);
        permissions.push_back(Permission::STOCK_UPDATE);
    } else if (type == RoleType::CUSTOMER) {
        permissions.push_back(Permission::PRODUCT_VIEW);
        permissions.push_back(Permission::ORDER_CREATE);
    }
}

// ============= IMPLEMENTATION ROLE ===============
inline RoleType Role :: getType() const { 
    // Hàm trả về Role được gán
    return type; 
}
inline void Role :: setType(RoleType t) { 
    // Hàm gán Role 
    type = t; 
}

inline bool Role :: hasPermission(Permission permission) const {
    // Hàm có chức năng kiểm tra Role có quyền hiện tại không
    for (int i = 0; i < permissions.getSize(); ++i) {
        if (permissions[i] == permission) {
            return true;
        }
    }
    return false;
}

inline void Role :: addPermission(Permission permission) {
    // Hàm có chức năng thêm quyền mới cho Role
    if (!hasPermission(permission)) {
        permissions.push_back(permission);
    }
}

inline void Role :: delPermission(Permission permission) {
    // Hàm có chức năng tước quyền hiện tại cho Role
    for (int i = 0; i < permissions.getSize(); ++i) {
        if (permissions[i] == permission) {
            permissions.removeAt(i);
            return;
        }
    }
}

// ============= IMPLEMENTATION ACCOUNT ===============

// Hàm dựng mặc định
inline Account :: Account() : id(""), username(""), password(""), role(RoleType::CUSTOMER), active(true) {}

// Hàm dựng có đủ tham số
inline Account :: Account(std::string username, std::string password,
                          RoleType roleType, std::string id)
    : id(id), username(username), password(password), role(roleType), active(true) {}

inline std::string Account :: getID() const { return id; }
inline void Account :: setID(const std::string& accountId) { id = accountId; }

inline std::string Account :: getUsername() const { 
    // Hàm trả về Tên đăng nhập của tài khoản
    return username; 

}
inline std::string Account :: getPassword() const {
    return password;
}

inline void Account :: setUsername(const std::string& u) { 
    // Hàm gán Tên người dùng cho tài khoản
    username = u; 
}

inline void Account :: setPassword(const std::string& p) {
    // Hàm gán Mật khẩu cho tài khoản 
    password = p; 
}

inline Role Account :: getRole() const { 
    // Hàm trả về Role hiện tại của tài khoản
    return role; 
}

inline RoleType Account :: getRoleType() const { 
    // Hàm trả về loại Role được gán cho tài khoản
    return role.getType(); 
}

inline void Account :: setRole(const Role& r) { 
    // Hàm gán Role cho tài khoản
    role = r; 
}

inline bool Account :: isActive() const { 
    // Hàm trả về tình trạng của tài khoản
    return active; 
}

inline void Account :: setActive(bool a) { 
    // Hàm gán tình trạng của tài khoản
    active = a; 
}

inline bool Account :: authenticate(const std::string& user, const std::string& pass) const {
    // Hàm xác minh đăng nhập cho tài khoản
    return active && (username == user) && (password == pass);
}

inline bool Account :: hasPermission(Permission permission) const {
    // Hàm kiểm tra Tài khoản có quyền hiện tại không
    return role.hasPermission(permission);
}

#endif // ACCOUNT_H
