#pragma once
// Vai trò người dùng. Chuyển từ `RoleType` trong models/Common.h cũ (giữ nguyên giá trị).

#include <string>

enum class Role {
    ADMIN,              // Admin / Manager
    SALE_EMPLOYEE,      // Nhân viên bán hàng
    WAREHOUSE_EMPLOYEE, // Nhân viên kho
    CUSTOMER            // Khách hàng
};

// Tên cũ của enum, giữ lại để code cũ (Account/Employee/Customer) vẫn biên dịch
using RoleType = Role;

// Trả về tên Role dưới giải nghĩa tiếng Việt
inline std::string roleTypeToString(Role role) {
    switch (role) {
        case Role::ADMIN: return "Quản trị viên (Admin)";
        case Role::SALE_EMPLOYEE: return "Nhân viên Bán hàng";
        case Role::WAREHOUSE_EMPLOYEE: return "Nhân viên Kho";
        case Role::CUSTOMER: return "Khách hàng";
        default: return "Chưa xác định";
    }
}
