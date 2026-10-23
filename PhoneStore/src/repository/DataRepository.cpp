#include "DataRepository.h"

DataRepository::DataRepository(FileManager& fm) : backupManager(fm) {}

Result<std::string> DataRepository::createBackup() const { return backupManager.createBackup(); }

Result<Vector<std::string>> DataRepository::listBackups() const { return backupManager.listBackups(); }

Result<void> DataRepository::restoreBackup(const std::string& backupName) const {
    // TODO(logic): sau khi restore file, các Repository phải load() lại (DataService điều phối).
    return backupManager.restore(backupName);
}

Result<ImportedLines> DataRepository::importLines(const std::string& path) const {
    return ImportExport::importLines(path);
}

Result<void> DataRepository::exportLines(const std::string& path, const Vector<std::string>& lines) const {
    return ImportExport::exportLines(path, lines);
}
