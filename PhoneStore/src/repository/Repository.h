#pragma once
// Lớp cơ sở dạng template cho Repository: giữ dữ liệu chính (Vector<T>) và index ID.
// Quy tắc: đọc trả BẢN SAO (không trả con trỏ/reference sửa được); add/update/remove chỉ sửa RAM,
// KHÔNG tự save (người gọi dùng UnitOfWork::commit hoặc gọi save()).
//
// Lớp con phải cài: getId, serialize, deserialize, rebuildIndexes.
// Lớp con KHÔNG cần dựng index ID (base đã làm), chỉ dựng các index riêng (HashTable/BST...).
// Index chỉ lưu vị trí hoặc ID, không lưu bản sao thực thể.

#include <string>

#include "IRepositoryTx.h"
#include "../common/Result.h"
#include "../storage/ds/HashTable.h"
#include "../storage/ds/Vector.h"
#include "../storage/file/FileManager.h"
#include "../storage/file/LoadReport.h"

template <typename T>
class Repository : public IRepositoryTx {
public:
    Repository(FileManager& fm, const std::string& file)
        : fileManager(fm), fileName(file), hasSnapshot(false) {}

    // Nạp file; dòng lỗi được bỏ qua và ghi vào loadReport(). File chưa tồn tại được coi là rỗng.
    Result<void> load() {
        items.clear();
        idIndex.clear();
        report = LoadReport();

        Result<Vector<std::string>> read = fileManager.readAllLines(fileName);
        if (!read.isOk()) {
            if (read.code() == ErrorCode::NotFound) {
                rebuildAllIndexes();
                return Result<void>::ok();
            }
            return Result<void>::fail(read.code(), read.message());
        }

        const Vector<std::string>& lines = read.value();
        for (int i = 0; i < lines.getSize(); ++i) {
            const std::string& line = lines[i];
            if (line.empty() || line[0] == '#') continue; // dòng trống / dòng phiên bản "#v1"
            ++report.totalLines;

            Result<T> parsed = deserialize(line);
            if (!parsed.isOk()) {
                report.addError(i + 1, parsed.message());
                continue;
            }
            const std::string id = getId(parsed.value());
            if (id.empty() || idIndex.contains(id)) {
                report.addError(i + 1, id.empty() ? "ID rỗng" : "trùng ID: " + id);
                continue;
            }
            idIndex.insert(id, items.getSize());
            items.push_back(parsed.value());
            ++report.loadedLines;
        }
        rebuildAllIndexes();
        return Result<void>::ok();
    }

    Result<void> save() { return flush(); }

    Vector<T> getAll() const { return items; } // bản sao

    Result<T> findById(const std::string& id) const {
        const int* pos = idIndex.find(id);
        if (pos == nullptr) return Result<T>::fail(ErrorCode::NotFound, "Không tìm thấy ID: " + id);
        return Result<T>::ok(items[*pos]);
    }

    // Chỉ sửa RAM
    Result<void> add(const T& item) {
        const std::string id = getId(item);
        if (id.empty()) return Result<void>::fail(ErrorCode::InvalidInput, "ID không được để trống");
        if (idIndex.contains(id)) return Result<void>::fail(ErrorCode::Duplicate, "Trùng ID: " + id);
        items.push_back(item);
        rebuildAllIndexes();
        return Result<void>::ok();
    }

    Result<void> update(const T& item) {
        const std::string id = getId(item);
        const int* pos = idIndex.find(id);
        if (pos == nullptr) return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy ID: " + id);
        items[*pos] = item;
        rebuildAllIndexes();
        return Result<void>::ok();
    }

    Result<void> remove(const std::string& id) {
        const int* pos = idIndex.find(id);
        if (pos == nullptr) return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy ID: " + id);
        items.removeAt(*pos);
        rebuildAllIndexes();
        return Result<void>::ok();
    }

    size_t count() const { return static_cast<size_t>(items.getSize()); }

    const LoadReport& loadReport() const { return report; }

    // ---- IRepositoryTx ----
    void snapshot() override {
        snapshotItems = items;
        hasSnapshot = true;
    }

    Result<void> flush() override {
        Vector<std::string> lines;
        lines.push_back("#v1");
        for (int i = 0; i < items.getSize(); ++i) lines.push_back(serialize(items[i]));
        return fileManager.writeAllLinesAtomic(fileName, lines);
    }

    void restore() override {
        if (!hasSnapshot) return;
        items = snapshotItems;
        rebuildAllIndexes();
    }

protected:
    virtual std::string getId(const T&) const = 0;
    virtual std::string serialize(const T&) const = 0;
    virtual Result<T> deserialize(const std::string& line) const = 0;
    virtual void rebuildIndexes() = 0; // chỉ dựng index riêng của lớp con

    Vector<T> items;                     // dữ liệu chính
    HashTable<std::string, int> idIndex; // ID -> vị trí trong items

private:
    void rebuildAllIndexes() {
        idIndex.clear();
        for (int i = 0; i < items.getSize(); ++i) idIndex.insert(getId(items[i]), i);
        rebuildIndexes();
    }

    FileManager& fileManager;
    std::string fileName;
    LoadReport report;
    Vector<T> snapshotItems;
    bool hasSnapshot;
};
