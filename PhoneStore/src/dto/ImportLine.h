#pragma once
// DTO cho dòng phiếu nhập kho (hỗ trợ cả sản phẩm có sẵn lẫn sản phẩm mới)

#include <string>
#include "../common/Money.h"

struct NewPhoneInfo {
    std::string name;            // bắt buộc
    std::string brand;           // bắt buộc
    std::string operatingSystem; // tùy chọn
    std::string manufacturer;    // tùy chọn
    std::string specifications;  // tùy chọn
};

struct ImportLine {
    std::string phoneId;   // có giá trị: sản phẩm có sẵn. Rỗng: sản phẩm mới
    NewPhoneInfo newPhone; // chỉ dùng khi phoneId rỗng
    int quantity = 0;      // > 0
    Money unitPrice = 0;   // giá nhập, > 0
};

