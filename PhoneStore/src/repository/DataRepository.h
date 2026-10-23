#pragma once
// Bọc BackupManager và ImportExport cho DataService (Service không được include storage/file).

#include <string>

#include "../storage/file/BackupManager.h"
#include "../storage/file/FileManager.h"
#include "../storage/file/ImportExport.h"
#include "../storage/ds/Vector.h"
#include "../common/Result.h"

class DataRepository {
public:
    explicit DataRepository(FileManager& fm);

    Result<std::string> createBackup() const;
    Result<Vector<std::string>> listBackups() const;
    Result<void> restoreBackup(const std::string& backupName) const;

    Result<ImportedLines> importLines(const std::string& path) const;
    Result<void> exportLines(const std::string& path, const Vector<std::string>& lines) const;

private:
    BackupManager backupManager;
};
