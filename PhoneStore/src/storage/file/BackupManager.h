#pragma once
// Sao lưu / khôi phục toàn bộ file *.txt trong thư mục dữ liệu. Dùng FileManager.
// Bản sao lưu nằm tại <dataDir>/backup/<YYYYMMDD_HHMMSS>/.

#include <string>

#include "FileManager.h"
#include "../ds/Vector.h"
#include "../../common/Result.h"

class BackupManager {
public:
    explicit BackupManager(const FileManager& fm);

    Result<std::string> createBackup() const;                    // trả tên thư mục backup (theo thời gian)
    Result<Vector<std::string>> listBackups() const;
    Result<void> restore(const std::string& backupName) const;   // kiểm tra hợp lệ toàn bộ trước khi ghi đè

private:
    const FileManager& fileManager;

    static const char* backupRoot() { return "backup"; }
    static bool isSafeName(const std::string& name);
};
