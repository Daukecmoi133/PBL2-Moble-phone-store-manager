#pragma once
// Mã lỗi dùng chung cho mọi tầng. Thay thế cho tham số `errorMessage` và
// ErrorMessage.h cũ (PhoneServiceError rỗng).

#include <string>

enum class ErrorCode {
    Ok,               // Thành công
    NotImplemented,   // Hàm còn là stub, chưa cài logic
    InvalidInput,     // Dữ liệu đầu vào sai
    NotFound,         // Không tìm thấy thực thể
    Duplicate,        // Trùng khóa (ID, username, mã voucher...)
    PermissionDenied, // Vai trò không được phép làm hành động này
    InvalidState,     // Sai trạng thái (ví dụ chuyển trạng thái đơn không hợp lệ)
    OutOfStock,       // Không đủ tồn kho khả dụng
    ReferenceExists,  // Còn dữ liệu khác tham chiếu tới, không được xóa
    AuthFailed,       // Đăng nhập thất bại
    StorageError,     // Lỗi đọc/ghi file
    ParseError        // Lỗi phân tích dòng văn bản
};

// Chuyển mã lỗi thành tên ngắn để ghi log / hiển thị gỡ lỗi
inline const char* errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::Ok: return "Ok";
        case ErrorCode::NotImplemented: return "NotImplemented";
        case ErrorCode::InvalidInput: return "InvalidInput";
        case ErrorCode::NotFound: return "NotFound";
        case ErrorCode::Duplicate: return "Duplicate";
        case ErrorCode::PermissionDenied: return "PermissionDenied";
        case ErrorCode::InvalidState: return "InvalidState";
        case ErrorCode::OutOfStock: return "OutOfStock";
        case ErrorCode::ReferenceExists: return "ReferenceExists";
        case ErrorCode::AuthFailed: return "AuthFailed";
        case ErrorCode::StorageError: return "StorageError";
        case ErrorCode::ParseError: return "ParseError";
    }
    return "Unknown";
}
