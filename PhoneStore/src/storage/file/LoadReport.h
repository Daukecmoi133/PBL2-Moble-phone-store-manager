#pragma once
// Báo cáo nạp/nhập dữ liệu: dòng lỗi được bỏ qua và ghi vào đây, không làm chương trình dừng.
// Chỉ dùng chuỗi, không biết thực thể nào.

#include <string>

#include "../ds/Vector.h"

struct LoadReport {
    int totalLines = 0;          // Số dòng dữ liệu đã đọc (không tính dòng trống / dòng chú thích '#')
    int loadedLines = 0;         // Số dòng nạp thành công
    Vector<std::string> errors;  // Mô tả từng dòng lỗi: "dòng N: lý do"

    // Ghi nhận một dòng lỗi
    void addError(int lineNumber, const std::string& reason) {
        errors.push_back("dòng " + std::to_string(lineNumber) + ": " + reason);
    }

    bool hasErrors() const { return errors.getSize() > 0; }
};
