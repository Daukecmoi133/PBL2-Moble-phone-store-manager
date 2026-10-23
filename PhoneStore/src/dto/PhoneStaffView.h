#pragma once
// Dữ liệu điện thoại hiển thị cho NHÂN VIÊN: có ID, tồn kho, giữ chỗ và giá nhập.

#include <string>

#include "../common/Money.h"

struct PhoneStaffView {
    std::string id;
    std::string name;
    std::string brand;
    std::string operatingSystem;
    Money importPrice = 0;
    Money sellingPrice = 0;
    int quantity = 0;          // tồn kho thực
    int reservedQuantity = 0;  // đang giữ chỗ cho đơn PENDING
};
