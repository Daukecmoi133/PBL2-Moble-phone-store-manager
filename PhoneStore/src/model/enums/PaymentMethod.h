#pragma once
// Phương thức thanh toán theo đặc tả: CASH, TRANSFER.
// Lưu ý: bản cũ có BANK_TRANSFER và CREDIT_CARD (giá trị số 1, 2 trong file dữ liệu cũ).
// Điểm này đã ghi vào docs/OPEN_QUESTIONS.md, chưa tự quyết cách ánh xạ.

#include <string>

enum class PaymentMethod {
    CASH,     // Tiền mặt
    TRANSFER  // Chuyển khoản
};

// Chuyển về phương thức thanh toán với dịch nghĩa tiếng Việt
inline std::string paymentMethodToString(PaymentMethod method) {
    switch (method) {
        case PaymentMethod::CASH: return "Tiền mặt";
        case PaymentMethod::TRANSFER: return "Chuyển khoản";
        default: return "Không rõ";
    }
}
