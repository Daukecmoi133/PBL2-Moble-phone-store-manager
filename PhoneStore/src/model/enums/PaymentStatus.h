#pragma once
// Trạng thái thanh toán của đơn. Thanh toán online chưa làm: nhân viên bán hàng đánh dấu PAID.

#include <string>

enum class PaymentStatus {
    UNPAID, // Chưa thanh toán
    PAID    // Đã thanh toán
};

inline std::string paymentStatusToString(PaymentStatus status) {
    switch (status) {
        case PaymentStatus::UNPAID: return "Chưa thanh toán";
        case PaymentStatus::PAID: return "Đã thanh toán";
        default: return "Không rõ";
    }
}
