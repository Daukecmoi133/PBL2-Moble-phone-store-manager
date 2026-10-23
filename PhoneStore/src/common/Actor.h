#pragma once
// Người thực hiện hành động (đi kèm mọi hàm ghi của Service).
// Lưu ý đặc tả: common chỉ được include thư viện chuẩn, nhưng Actor cần `Role`;
// Role là enum thuần nên tạm cho phép common include model/enums (xem docs/OPEN_QUESTIONS.md).

#include <string>
#include <utility>

#include "../model/enums/Role.h"

struct Actor {
    std::string userId;       // ID người dùng đang đăng nhập (rỗng nếu chưa đăng nhập)
    Role role = Role::CUSTOMER;

    Actor() = default;
    Actor(std::string id, Role r) : userId(std::move(id)), role(r) {}

    // Đã đăng nhập hay chưa
    bool isAuthenticated() const { return !userId.empty(); }
};
