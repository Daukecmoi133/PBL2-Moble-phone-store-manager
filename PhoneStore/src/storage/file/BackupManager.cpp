#include "BackupManager.h"

#include <ctime>

BackupManager::BackupManager(const FileManager& fm) : fileManager(fm) {}

// Tên backup không được chứa dấu phân cách đường dẫn hoặc ".." (tránh ghi/đọc ra ngoài data/backup)
bool BackupManager::isSafeName(const std::string& name) {
    if (name.empty()) return false;
    if (name.find("..") != std::string::npos) return false;
    if (name.find('/') != std::string::npos) return false;
    if (name.find('\\') != std::string::npos) return false;
    return true;
}

Result<std::string> BackupManager::createBackup() const {
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    char buf[32] = {0};
    if (lt == nullptr || std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", lt) == 0) {
        return Result<std::string>::fail(ErrorCode::StorageError, "Không lấy được thời gian hệ thống");
    }

    // Nếu trùng giây với backup trước, thêm hậu tố _2, _3...
    std::string name = buf;
    std::string root = backupRoot();
    for (int n = 2; fileManager.exists(root + "/" + name); ++n) {
        name = std::string(buf) + "_" + std::to_string(n);
    }

    Result<void> mk = fileManager.createDirectory(root + "/" + name);
    if (!mk.isOk()) return Result<std::string>::fail(mk.code(), mk.message());

    Vector<std::string> files = fileManager.listTextFiles();
    for (int i = 0; i < files.getSize(); ++i) {
        Result<Vector<std::string>> r = fileManager.readAllLines(files[i]);
        if (!r.isOk()) return Result<std::string>::fail(r.code(), r.message());
        Result<void> w = fileManager.writeAllLinesAtomic(root + "/" + name + "/" + files[i], r.value());
        if (!w.isOk()) return Result<std::string>::fail(w.code(), w.message());
    }
    return Result<std::string>::ok(name);
}

Result<Vector<std::string>> BackupManager::listBackups() const {
    return Result<Vector<std::string>>::ok(fileManager.listDirectories(backupRoot()));
}

Result<void> BackupManager::restore(const std::string& backupName) const {
    if (!isSafeName(backupName)) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Tên bản sao lưu không hợp lệ");
    }
    std::string dir = std::string(backupRoot()) + "/" + backupName;
    if (!fileManager.exists(dir)) {
        return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy bản sao lưu: " + backupName);
    }
    Vector<std::string> files = fileManager.listTextFiles(dir);
    if (files.isEmpty()) {
        return Result<void>::fail(ErrorCode::InvalidState, "Bản sao lưu rỗng: " + backupName);
    }

    // Bước 1: đọc và kiểm tra TOÀN BỘ file trước khi ghi đè bất kỳ file nào
    Vector<Vector<std::string>> contents;
    for (int i = 0; i < files.getSize(); ++i) {
        Result<Vector<std::string>> r = fileManager.readAllLines(dir + "/" + files[i]);
        if (!r.isOk()) return Result<void>::fail(r.code(), r.message());
        contents.push_back(r.value());
    }

    // Bước 2: ghi đè từng file (best effort, không nguyên tử giữa các file)
    for (int i = 0; i < files.getSize(); ++i) {
        Result<void> w = fileManager.writeAllLinesAtomic(files[i], contents[i]);
        if (!w.isOk()) return w;
    }
    return Result<void>::ok();
}
