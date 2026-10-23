#pragma once
// Nhập/xuất dòng văn bản theo đường dẫn người dùng đưa vào. Không biết thực thể:
// việc kiểm tra từng dòng có phải Phone/Customer hợp lệ hay không thuộc về Repository/Service,
// họ ghi thêm lỗi vào LoadReport trả về từ đây.

#include <string>

#include "LoadReport.h"
#include "../ds/Vector.h"
#include "../../common/Result.h"

struct ImportedLines {
    Vector<std::string> lines; // các dòng dữ liệu (đã bỏ dòng trống và dòng chú thích '#')
    LoadReport report;         // totalLines/loadedLines và lỗi mức file (nếu có)
};

class ImportExport {
public:
    static Result<ImportedLines> importLines(const std::string& path);
    static Result<void> exportLines(const std::string& path, const Vector<std::string>& lines);
};
