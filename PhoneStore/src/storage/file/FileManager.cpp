#include "FileManager.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

FileManager::FileManager(const std::string& dir) : dataDir(dir) {}

std::string FileManager::fullPath(const std::string& fileName) const {
    if (fileName.empty()) return dataDir;
    if (dataDir.empty()) return fileName;
    return dataDir + "/" + fileName;
}

const std::string& FileManager::dataDirectory() const { return dataDir; }

bool FileManager::exists(const std::string& fileName) const {
    std::error_code ec;
    return fs::exists(fs::path(fullPath(fileName)), ec);
}

Result<Vector<std::string>> FileManager::readAllLines(const std::string& fileName) const {
    std::ifstream in(fullPath(fileName), std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        return Result<Vector<std::string>>::fail(
            exists(fileName) ? ErrorCode::StorageError : ErrorCode::NotFound,
            "Không mở được file: " + fileName);
    }
    Vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    if (in.bad()) {
        return Result<Vector<std::string>>::fail(ErrorCode::StorageError, "Lỗi khi đọc file: " + fileName);
    }
    return Result<Vector<std::string>>::ok(std::move(lines));
}

Result<void> FileManager::writeAllLinesAtomic(const std::string& fileName,
                                              const Vector<std::string>& lines) const {
    std::error_code ec;
    fs::path target(fullPath(fileName));
    if (target.has_parent_path()) {
        fs::create_directories(target.parent_path(), ec);
        if (ec) return Result<void>::fail(ErrorCode::StorageError, "Không tạo được thư mục cho: " + fileName);
    }

    fs::path tmp = target;
    tmp += ".tmp";
    {
        std::ofstream out(tmp.string(), std::ios::out | std::ios::trunc | std::ios::binary);
        if (!out.is_open()) {
            return Result<void>::fail(ErrorCode::StorageError, "Không ghi được file tạm: " + fileName);
        }
        for (int i = 0; i < lines.getSize(); ++i) out << lines[i] << '\n';
        out.flush();
        if (!out.good()) {
            out.close();
            fs::remove(tmp, ec);
            return Result<void>::fail(ErrorCode::StorageError, "Lỗi khi ghi file tạm: " + fileName);
        }
    }

    fs::rename(tmp, target, ec);
    if (ec) {
        // Một số hệ thống không cho rename đè lên file có sẵn: xóa file đích rồi đổi tên lại
        std::error_code ec2;
        fs::remove(target, ec2);
        ec.clear();
        fs::rename(tmp, target, ec);
        if (ec) {
            fs::remove(tmp, ec2);
            return Result<void>::fail(ErrorCode::StorageError, "Không đổi tên file tạm thành: " + fileName);
        }
    }
    return Result<void>::ok();
}

Vector<std::string> FileManager::listTextFiles(const std::string& subDir) const {
    Vector<std::string> names;
    std::error_code ec;
    fs::directory_iterator it(fs::path(fullPath(subDir)), ec);
    if (ec) return names;
    for (const auto& entry : it) {
        std::error_code ec2;
        if (entry.is_regular_file(ec2) && entry.path().extension() == ".txt") {
            names.push_back(entry.path().filename().string());
        }
    }
    return names;
}

Vector<std::string> FileManager::listDirectories(const std::string& subDir) const {
    Vector<std::string> names;
    std::error_code ec;
    fs::directory_iterator it(fs::path(fullPath(subDir)), ec);
    if (ec) return names;
    for (const auto& entry : it) {
        std::error_code ec2;
        if (entry.is_directory(ec2)) names.push_back(entry.path().filename().string());
    }
    // Sắp xếp chèn tăng dần (số lượng nhỏ) để tên thư mục theo thời gian đứng đúng thứ tự
    for (int i = 1; i < names.getSize(); ++i) {
        std::string key = names[i];
        int j = i - 1;
        while (j >= 0 && key < names[j]) {
            names[j + 1] = names[j];
            --j;
        }
        names[j + 1] = key;
    }
    return names;
}

Result<void> FileManager::createDirectory(const std::string& subDir) const {
    std::error_code ec;
    fs::create_directories(fs::path(fullPath(subDir)), ec);
    if (ec) return Result<void>::fail(ErrorCode::StorageError, "Không tạo được thư mục: " + subDir);
    return Result<void>::ok();
}
