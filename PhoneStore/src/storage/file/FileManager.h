#pragma once
// Đọc/ghi file văn bản trong thư mục dữ liệu. Chỉ làm việc với dòng chữ,
// không biết thực thể (Phone, Order...) hay quy tắc nghiệp vụ.

#include <string>

#include "../ds/Vector.h"
#include "../../common/Result.h"

class FileManager {
public:
    explicit FileManager(const std::string& dataDir);

    // Đọc toàn bộ dòng của file (đã bỏ ký tự '\r' cuối dòng)
    Result<Vector<std::string>> readAllLines(const std::string& fileName) const;

    // Ghi toàn bộ dòng: ghi ra file .tmp rồi đổi tên thành file đích (tự tạo thư mục cha nếu thiếu)
    Result<void> writeAllLinesAtomic(const std::string& fileName,
                                     const Vector<std::string>& lines) const;

    bool exists(const std::string& fileName) const;

    // ---- Tiện ích thư mục (dùng cho BackupManager) ----
    const std::string& dataDirectory() const;
    Vector<std::string> listTextFiles(const std::string& subDir = "") const;   // tên các file *.txt trực tiếp trong thư mục
    Vector<std::string> listDirectories(const std::string& subDir = "") const; // tên thư mục con, đã sắp xếp tăng dần
    Result<void> createDirectory(const std::string& subDir) const;

private:
    std::string dataDir;

    std::string fullPath(const std::string& fileName) const;
};
