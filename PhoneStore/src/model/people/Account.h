// ĐỐI TƯỢNG TÀI KHOẢN
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
// FIX LỖI: đổi include theo cây thư mục mới
#include "../enums/Role.h"
#include "../../storage/ds/Vector.h"

// FIX LỖI: enum Permission cũ trùng tên với struct Permission (common/Permission.h) -> đổi tên AccountPermission.
// Class Role cũ trùng tên với enum Role (model/enums/Role.h) -> đổi tên AccountRole.
// Giá trị enum giữ nguyên như models/Common.h cũ.
enum class AccountPermission {
    PRODUCT_VIEW,
    PRODUCT_CREATE,
    PRODUCT_UPDATE,
    PRODUCT_DELETE,

    ORDER_VIEW,
    ORDER_CREATE,

    INVOICE_CREATE,

    STOCK_VIEW,
    STOCK_UPDATE,

    EMPLOYEE_VIEW,
    EMPLOYEE_CREATE
};

// Lớp AccountRole: Quản lý vai trò người dùng và danh sách các quyền hạn được cấp
class AccountRole {
private:
    RoleType type;                  // Loại vai trò (Admin, Nhân viên, Khách hàng)
    Vector<AccountPermission> permissions; // Danh sách các quyền hạn thuộc vai trò

public:
    AccountRole(RoleType type = RoleType::CUSTOMER); // Hàm dựng gán vai trò và thiết lập quyền mặc định

    RoleType getType() const; // Hàm trả về vai trò hiện tại
    void setType(RoleType t); // Hàm gán lại vai trò
    bool hasPermission(AccountPermission permission) const; // Hàm kiểm tra vai trò có quyền cụ thể hay không
    void addPermission(AccountPermission permission); // Hàm cấp thêm quyền cho vai trò
    void delPermission(AccountPermission permission); // Hàm tước quyền khỏi vai trò
};

// Lớp Account: Quản lý thông tin đăng nhập và trạng thái tài khoản
class Account {
private:
    std::string username; // Tên đăng nhập
    std::string password; // Mật khẩu tài khoản
    AccountRole role;            // Đối tượng vai trò và quyền hạn của tài khoản
    bool active;          // Trạng thái kích hoạt (true: hoạt động, false: bị khóa)

public:
    Account(); // Hàm dựng mặc định
    Account(std::string username, std::string password, RoleType roleType = RoleType::CUSTOMER); // Hàm dựng đủ tham số

    std::string getUsername() const; // Hàm trả về tên đăng nhập
    std::string getPassword() const; // Hàm trả về mật khẩu
    AccountRole getRole() const; // Hàm trả về đối tượng vai trò AccountRole
    RoleType getRoleType() const; // Hàm trả về loại vai trò RoleType
    void setRole(const AccountRole& r); // Hàm thiết lập vai trò
    void setUsername(const std::string& u); // Hàm thiết lập tên đăng nhập
    void setPassword(const std::string& p); // Hàm thiết lập mật khẩu
    bool isActive() const; // Hàm kiểm tra tài khoản còn hoạt động không
    void setActive(bool a); // Hàm thiết lập trạng thái kích hoạt tài khoản
    bool authenticate(const std::string& user, const std::string& pass) const; // Hàm xác minh tên đăng nhập và mật khẩu
    bool hasPermission(AccountPermission permission) const; // Hàm kiểm tra tài khoản có quyền thực thi chức năng không
};

inline AccountRole :: AccountRole(RoleType type) : type(type) {
    // Khởi tạo các quyền mặc định theo vai trò
    if (type == RoleType::ADMIN) {
        permissions.push_back(AccountPermission::PRODUCT_VIEW);
        permissions.push_back(AccountPermission::PRODUCT_CREATE);
        permissions.push_back(AccountPermission::PRODUCT_UPDATE);
        permissions.push_back(AccountPermission::PRODUCT_DELETE);
        permissions.push_back(AccountPermission::ORDER_VIEW);
        permissions.push_back(AccountPermission::ORDER_CREATE);
        permissions.push_back(AccountPermission::INVOICE_CREATE);
        permissions.push_back(AccountPermission::STOCK_VIEW);
        permissions.push_back(AccountPermission::STOCK_UPDATE);
        permissions.push_back(AccountPermission::EMPLOYEE_VIEW);
        permissions.push_back(AccountPermission::EMPLOYEE_CREATE);
    } else if (type == RoleType::SALE_EMPLOYEE) {
        permissions.push_back(AccountPermission::PRODUCT_VIEW);
        permissions.push_back(AccountPermission::ORDER_VIEW);
        permissions.push_back(AccountPermission::ORDER_CREATE);
        permissions.push_back(AccountPermission::INVOICE_CREATE);
        permissions.push_back(AccountPermission::STOCK_VIEW);
    } else if (type == RoleType::WAREHOUSE_EMPLOYEE) {
        permissions.push_back(AccountPermission::PRODUCT_VIEW);
        permissions.push_back(AccountPermission::STOCK_VIEW);
        permissions.push_back(AccountPermission::STOCK_UPDATE);
    } else if (type == RoleType::CUSTOMER) {
        permissions.push_back(AccountPermission::PRODUCT_VIEW);
        permissions.push_back(AccountPermission::ORDER_CREATE);
    }
}

// ============= IMPLEMENTATION ROLE ===============
inline RoleType AccountRole :: getType() const { 
    // Hàm trả về AccountRole được gán
    return type; 
}
inline void AccountRole :: setType(RoleType t) { 
    // Hàm gán AccountRole 
    type = t; 
}

inline bool AccountRole :: hasPermission(AccountPermission permission) const {
    // Hàm có chức năng kiểm tra AccountRole có quyền hiện tại không
    for (int i = 0; i < permissions.getSize(); ++i) {
        if (permissions[i] == permission) {
            return true;
        }
    }
    return false;
}

inline void AccountRole :: addPermission(AccountPermission permission) {
    // Hàm có chức năng thêm quyền mới cho AccountRole
    if (!hasPermission(permission)) {
        permissions.push_back(permission);
    }
}

inline void AccountRole :: delPermission(AccountPermission permission) {
    // Hàm có chức năng tước quyền hiện tại cho AccountRole
    for (int i = 0; i < permissions.getSize(); ++i) {
        if (permissions[i] == permission) {
            permissions.removeAt(i);
            return;
        }
    }
}

// ============= IMPLEMENTATION ACCOUNT ===============

// Hàm dựng mặc định
inline Account :: Account() : username(""), password(""), role(RoleType::CUSTOMER), active(true) {}

// Hàm dựng có đủ tham số
inline Account :: Account(std::string username, std::string password, RoleType roleType)
    : username(username), password(password), role(roleType), active(true) {}

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

inline AccountRole Account :: getRole() const { 
    // Hàm trả về AccountRole hiện tại của tài khoản
    return role; 
}

inline RoleType Account :: getRoleType() const { 
    // Hàm trả về loại AccountRole được gán cho tài khoản
    return role.getType(); 
}

inline void Account :: setRole(const AccountRole& r) { 
    // Hàm gán AccountRole cho tài khoản
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

inline bool Account :: hasPermission(AccountPermission permission) const {
    // Hàm kiểm tra Tài khoản có quyền hiện tại không
    return role.hasPermission(permission);
}

#endif // ACCOUNT_H
