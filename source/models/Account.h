// ĐỐI TƯỢNG TÀI KHOẢN
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include "Common.h"
#include "../core/Vector.h"
#include "../utils/PasswordHasher.h"

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
    std::string password; // Thông tin xác thực đã lưu: chuỗi hash PBKDF2 (PasswordHasher) hoặc plaintext cũ chờ chuyển đổi
    Role role;            // Đối tượng vai trò và quyền hạn của tài khoản
    bool active;          // Trạng thái kích hoạt (true: hoạt động, false: bị khóa)

public:
    Account(); // Hàm dựng mặc định
    // passwordCredential là giá trị ĐÃ LƯU (hash PBKDF2 hoặc plaintext cũ đọc từ file).
    // Service sẽ tự băm lại bất kỳ giá trị chưa băm nào trước khi ghi xuống file (xem ensureCredentialHashed).
    Account(std::string username, std::string passwordCredential,
            RoleType roleType = RoleType::CUSTOMER, std::string id = ""); // Hàm dựng đủ tham số

    std::string getID() const;
    void setID(const std::string& id);
    std::string getUsername() const; // Hàm trả về tên đăng nhập
    std::string getPasswordHash() const; // Hàm trả về thông tin xác thực đã lưu (hash hoặc plaintext cũ), dùng để ghi file
    Role getRole() const; // Hàm trả về đối tượng vai trò Role
    RoleType getRoleType() const; // Hàm trả về loại vai trò RoleType
    void setRole(const Role& r); // Hàm thiết lập vai trò
    void setUsername(const std::string& u); // Hàm thiết lập tên đăng nhập
    bool setPassword(const std::string& plainPassword); // Băm mật khẩu rồi lưu; false nếu băm thất bại (giữ nguyên giá trị cũ)
    void setPasswordHash(const std::string& storedCredential); // Gán trực tiếp giá trị đã lưu (dùng khi nạp file/hoàn nguyên)
    bool isPasswordHashed() const; // Thông tin xác thực đã ở dạng hash chưa (false = plaintext cũ cần chuyển đổi)
    bool ensureCredentialHashed(); // Băm thông tin plaintext đang giữ (giữ nguyên mật khẩu); false nếu rỗng hoặc băm thất bại
    bool isActive() const; // Hàm kiểm tra tài khoản còn hoạt động không
    void setActive(bool a); // Hàm thiết lập trạng thái kích hoạt tài khoản
    bool verifyPassword(const std::string& pass) const; // Kiểm tra mật khẩu với thông tin đã lưu (không xét trạng thái active)
    bool authenticate(const std::string& user, const std::string& pass) const; // Hàm xác minh tên đăng nhập và mật khẩu (yêu cầu tài khoản đang hoạt động)
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
inline Account :: Account(std::string username, std::string passwordCredential,
                          RoleType roleType, std::string id)
    : id(id), username(username), password(passwordCredential), role(roleType), active(true) {}

inline std::string Account :: getID() const { return id; }
inline void Account :: setID(const std::string& accountId) { id = accountId; }

inline std::string Account :: getUsername() const { 
    // Hàm trả về Tên đăng nhập của tài khoản
    return username; 

}
inline std::string Account :: getPasswordHash() const {
    return password;
}

inline void Account :: setUsername(const std::string& u) { 
    // Hàm gán Tên người dùng cho tài khoản
    username = u; 
}

inline bool Account :: setPassword(const std::string& plainPassword) {
    // Hàm đặt mật khẩu mới cho tài khoản: luôn băm trước khi lưu, không bao giờ lưu dạng rõ
    std::string hashed = PasswordHasher::hash(plainPassword);
    if (hashed.empty()) return false;
    password = hashed;
    return true;
}

inline void Account :: setPasswordHash(const std::string& storedCredential) {
    // Gán nguyên giá trị đã lưu (nạp từ file hoặc hoàn nguyên khi ghi file thất bại)
    password = storedCredential;
}

inline bool Account :: isPasswordHashed() const {
    return PasswordHasher::isHashed(password);
}

inline bool Account :: ensureCredentialHashed() {
    // Chuyển plaintext (từ dữ liệu cũ hoặc do caller truyền vào) sang hash mà KHÔNG đổi mật khẩu
    if (password.empty()) return false;
    if (PasswordHasher::isHashed(password)) return true;
    std::string hashed = PasswordHasher::hash(password);
    if (hashed.empty()) return false;
    password = hashed;
    return true;
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

inline bool Account :: verifyPassword(const std::string& pass) const {
    if (!PasswordHasher::isHashed(password)) {
        std::cerr << "[CANH BAO AN NINH] Tu choi xac thuc: Mat khau cua tai khoan '"
                  << username << "' chua duoc bam bang PBKDF2 (dang o dang ro).\n";
        return false;
    }
    return PasswordHasher::verify(pass, password);
}

inline bool Account :: authenticate(const std::string& user, const std::string& pass) const {
    // Hàm xác minh đăng nhập cho tài khoản
    return active && (username == user) && verifyPassword(pass);
}

inline bool Account :: hasPermission(Permission permission) const {
    // Hàm kiểm tra Tài khoản có quyền hiện tại không
    return role.hasPermission(permission);
}

#endif // ACCOUNT_H
