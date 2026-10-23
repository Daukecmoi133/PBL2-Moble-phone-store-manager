#pragma once
// Giao diện cung cấp Actor đang đăng nhập cho Controller.
// Lý do tồn tại: đặc tả bắt Controller "lấy Actor từ Session" nhưng Session nằm ở app/
// (Controller không được include app). Session (app/) cài đặt giao diện này.

#include "Actor.h"

class ISessionProvider {
public:
    virtual ~ISessionProvider() = default;
    virtual const Actor& currentActor() const = 0; // Actor hiện tại (chưa đăng nhập: userId rỗng)
    virtual void setActor(const Actor& actor) = 0; // Gọi sau khi đăng nhập thành công
    virtual void clear() = 0;                      // Đăng xuất
};
