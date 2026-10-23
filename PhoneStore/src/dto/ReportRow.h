#pragma once
// Một dòng báo cáo thống kê (doanh thu theo kỳ, số đơn theo trạng thái, sản phẩm bán chạy, tồn kho...).

#include <string>

#include "../common/Money.h"

struct ReportRow {
    std::string label;   // tên nhóm (tháng, trạng thái, tên sản phẩm...)
    long long count = 0; // số lượng (đơn, sản phẩm...)
    Money amount = 0;    // giá trị tiền (nếu có)
};
