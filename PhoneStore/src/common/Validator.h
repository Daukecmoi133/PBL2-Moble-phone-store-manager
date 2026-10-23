#pragma once
// Kiểm tra hợp lệ dữ liệu đầu vào cơ bản.
// isValidID(...) chuyển nguyên văn từ services/Check.h cũ (phần ReferenceChecker phụ thuộc
// Repository nên không đặt ở common, xem docs/MIGRATION.md).

#include <cctype>
#include <string>

// Kiểm tra tính hợp lệ của mã ID (chỉ chứa các ký tự chữ cái và số, không rỗng)
inline bool isValidID(const std::string& id, std::string& errorMessage) {
    if (id.empty()) {
        errorMessage = "Mã ID không được để trống!";
        return false;
    }
    for (char c : id) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_') {
            errorMessage = "Mã ID chỉ được chứa chữ cái, số và dấu gạch ngang!";
            return false;
        }
    }
    return true;
}

inline bool isValidID(const std::string& id) {
    std::string err;
    return isValidID(id, err);
}

// Lớp bọc các hàm kiểm tra để Service gọi theo kiểu hướng đối tượng
class Validator {
public:
    // Mã ID hợp lệ (dùng lại isValidID)
    static bool isValidId(const std::string& id);
    // Chuỗi không rỗng sau khi bỏ khoảng trắng hai đầu
    static bool isNotBlank(const std::string& text);
};
