#pragma once
// Hằng số cấu hình hệ thống dùng chung

namespace Config {
    // Thời gian giữ chỗ tối đa cho đơn hàng PENDING (giờ)
    constexpr int PENDING_ORDER_TTL_HOURS = 24;

    // Số đơn hàng PENDING tối đa mỗi khách hàng được phép có cùng lúc
    constexpr int MAX_PENDING_ORDERS_PER_CUSTOMER = 3;

    // Số lượng sản phẩm tối đa cho mỗi dòng hàng (trong giỏ và đơn đặt)
    constexpr int MAX_QUANTITY_PER_LINE = 5;
}

