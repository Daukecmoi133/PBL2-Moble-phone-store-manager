#include "ImportExport.h"

#include <fstream>

Result<ImportedLines> ImportExport::importLines(const std::string& path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        return Result<ImportedLines>::fail(ErrorCode::NotFound, "Không mở được file: " + path);
    }

    ImportedLines result;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue; // bỏ dòng trống và dòng chú thích/phiên bản
        result.lines.push_back(line);
        ++result.report.totalLines;
        ++result.report.loadedLines;
    }
    if (in.bad()) {
        return Result<ImportedLines>::fail(ErrorCode::StorageError, "Lỗi khi đọc file: " + path);
    }
    return Result<ImportedLines>::ok(std::move(result));
}

Result<void> ImportExport::exportLines(const std::string& path, const Vector<std::string>& lines) {
    std::ofstream out(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!out.is_open()) {
        return Result<void>::fail(ErrorCode::StorageError, "Không ghi được file: " + path);
    }
    for (int i = 0; i < lines.getSize(); ++i) out << lines[i] << '\n';
    out.flush();
    if (!out.good()) {
        return Result<void>::fail(ErrorCode::StorageError, "Lỗi khi ghi file: " + path);
    }
    return Result<void>::ok();
}
