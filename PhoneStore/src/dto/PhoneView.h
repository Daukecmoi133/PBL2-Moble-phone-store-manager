#pragma once
// Dữ liệu điện thoại hiển thị cho KHÁCH HÀNG. TUYỆT ĐỐI KHÔNG có ID nội bộ.
// Controller giữ bảng ánh xạ "vị trí hiển thị -> ID" cho danh sách đang hiển thị.

#include <string>

#include "../common/Money.h"

struct PhoneView {
    std::string name;
    std::string brand;
    std::string operatingSystem;
    std::string specifications;
    Money sellingPrice = 0;
    bool inStock = false;  // còn hàng khả dụng (available > 0)
};
