#pragma once
// Một dòng giỏ hàng để hiển thị cho khách (không có ID nội bộ).

#include <string>

#include "../common/Money.h"

struct CartLineView {
    std::string phoneName;
    int quantity = 0;
    Money unitPrice = 0;
    Money subtotal = 0;
    bool selected = true;
    bool available = true; // còn hàng khả dụng và đang được đăng bán
};
