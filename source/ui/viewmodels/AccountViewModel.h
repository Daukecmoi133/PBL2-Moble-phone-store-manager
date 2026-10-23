#ifndef ACCOUNT_VIEWMODEL_H
#define ACCOUNT_VIEWMODEL_H

#include <string>
#include <vector>
#include "../../models/Common.h"

// ViewModel hiển thị một tài khoản người dùng
struct AccountItemViewModel {
    std::string id;
    std::string username;
    RoleType role;
    std::string roleName;
    bool isActive;
    std::string linkedEntityId; // Mã nhân viên hoặc mã khách hàng liên kết
    std::string linkedFullName;

    AccountItemViewModel() : role(RoleType::CUSTOMER), isActive(true) {}
};

#endif // ACCOUNT_VIEWMODEL_H
