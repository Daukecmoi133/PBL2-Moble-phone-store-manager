#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include "Common.h"
#include "../core/Vector.h"

class Role {
private:
    RoleType type;
    Vector<Permission> permissions;

public:
    Role(RoleType type = RoleType::CUSTOMER) : type(type) {
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

    RoleType getType() const { return type; }
    void setType(RoleType t) { type = t; }

    bool hasPermission(Permission permission) const {
        for (int i = 0; i < permissions.getSize(); ++i) {
            if (permissions[i] == permission) {
                return true;
            }
        }
        return false;
    }

    void addPermission(Permission permission) {
        if (!hasPermission(permission)) {
            permissions.push_back(permission);
        }
    }

    void delPermission(Permission permission) {
        for (int i = 0; i < permissions.getSize(); ++i) {
            if (permissions[i] == permission) {
                permissions.removeAt(i);
                return;
            }
        }
    }
};

class Account {
private:
    std::string username;
    std::string password;
    Role role;
    bool active;

public:
    Account() : username(""), password(""), role(RoleType::CUSTOMER), active(true) {}

    Account(std::string username, std::string password, RoleType roleType = RoleType::CUSTOMER)
        : username(username), password(password), role(roleType), active(true) {}

    std::string getUsername() const { return username; }
    void setUsername(const std::string& u) { username = u; }

    void setPassword(const std::string& p) { password = p; }

    Role getRole() const { return role; }
    RoleType getRoleType() const { return role.getType(); }
    void setRole(const Role& r) { role = r; }

    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }

    bool authenticate(const std::string& user, const std::string& pass) const {
        return active && (username == user) && (password == pass);
    }

    bool hasPermission(Permission permission) const {
        return role.hasPermission(permission);
    }
};

#endif // ACCOUNT_H
