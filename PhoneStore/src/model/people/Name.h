#pragma once
// Cấu trúc Name (chuyển nguyên từ models/Common.h cũ).

#include <string>

// Khởi tạo kiểu dữ liệu cho Tên con người
struct Name {
    std::string firstName;
    std::string middleName;
    std::string lastName;

    Name();
    Name(std::string first, std::string middle, std::string last);

    std::string getFullName() const;
};

// ================= IMPLEMENTATION NAME =================

// Hàm dựng
inline Name::Name() : firstName(""), middleName(""), lastName("") {}

// Hàm dựng đủ đối số
inline Name::Name(std::string first, std::string middle, std::string last)
    : firstName(first), middleName(middle), lastName(last) {}

// Hàm trả về chuỗi đầy đủ tên của người
inline std::string Name::getFullName() const {
    std::string full = "";
    if (!firstName.empty()) full += firstName;
    if (!middleName.empty()) {
        if (!full.empty()) full += " ";
        full += middleName;
    }
    if (!lastName.empty()) {
        if (!full.empty()) full += " ";
        full += lastName;
    }
    return full;
}
