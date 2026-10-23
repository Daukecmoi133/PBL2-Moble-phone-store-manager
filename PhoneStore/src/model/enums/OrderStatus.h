#pragma once
// Trạng thái đơn hàng (chuyển nguyên từ models/Common.h cũ).
// TODO(logic): bảng chuyển trạng thái nằm ở service/OrderStateMachine.

#include <string>

enum class OrderStatus {
    PENDING,          // Đang chờ xử lý
    CONFIRMED,        // Đã xác nhận
    PREPARING,        // Đang chuẩn bị hàng
    READY_FOR_PICKUP, // Sẵn sàng nhận hàng
    SHIPPING,         // Đang giao hàng
    COMPLETED,        // Hoàn tất
    CANCELLED         // Đã hủy
};

// Trả về trạng thái đơn hàng với dịch nghĩa tiếng Việt
inline std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::PENDING: return "Chờ xử lý";
        case OrderStatus::CONFIRMED: return "Đã xác nhận";
        case OrderStatus::PREPARING: return "Đang đóng gói";
        case OrderStatus::READY_FOR_PICKUP: return "Sẵn sàng nhận hàng";
        case OrderStatus::SHIPPING: return "Đang giao hàng";
        case OrderStatus::COMPLETED: return "Hoàn thành";
        case OrderStatus::CANCELLED: return "Đã hủy";
        default: return "Không rõ";
    }
}
