#pragma once
// Kết quả nhập dữ liệu từ file ngoài: số dòng thành công và lỗi từng dòng.

#include <string>

#include "../storage/ds/Vector.h"

struct ImportReport {
    int totalLines = 0;
    int importedLines = 0;
    Vector<std::string> errors;  // "dòng N: lý do"
};
