#pragma once
// Repository nhân viên. Ngoại lệ đã được chủ dự án chấp nhận: sở hữu các Employee* nội bộ (đa hình),
// nên KHÔNG kế thừa Repository<T>. Bên ngoài chỉ nhận BẢN SAO qua clone() bọc trong std::unique_ptr.

#include <memory>
#include <string>

#include "IRepositoryTx.h"
#include "../model/people/Employee.h"
#include "../storage/ds/HashTable.h"
#include "../storage/ds/Vector.h"
#include "../storage/file/FileManager.h"
#include "../storage/file/LoadReport.h"

class EmployeeRepository : public IRepositoryTx {
public:
    explicit EmployeeRepository(FileManager& fm);
    ~EmployeeRepository() override;
    EmployeeRepository(const EmployeeRepository&) = delete;
    EmployeeRepository& operator=(const EmployeeRepository&) = delete;

    Result<void> load();
    Result<void> save();

    Vector<std::unique_ptr<Employee>> getAll() const;                      // bản sao
    Result<std::unique_ptr<Employee>> findById(const std::string& id) const; // bản sao
    Result<std::unique_ptr<Employee>> findByUsername(const std::string& username) const;

    Result<void> add(const Employee& employee);     // lưu một bản clone; chỉ sửa RAM
    Result<void> update(const Employee& employee);  // thay bằng bản clone mới; chỉ sửa RAM
    Result<void> remove(const std::string& id);
    size_t count() const;
    const LoadReport& loadReport() const;

    // ---- IRepositoryTx ----
    void snapshot() override;
    Result<void> flush() override;
    void restore() override;

private:
    FileManager& fileManager;
    std::string fileName;
    LoadReport report;

    Vector<Employee*> items;          // sở hữu, giải phóng trong destructor / khi thay thế
    Vector<Employee*> snapshotItems;  // sở hữu: bản clone tại thời điểm snapshot
    bool hasSnapshot;
    HashTable<std::string, int> idIndex;        // ID -> vị trí
    HashTable<std::string, int> usernameIndex;  // TODO(logic): dựng index username -> vị trí

    static void destroyAll(Vector<Employee*>& list);
    static Vector<Employee*> cloneAll(const Vector<Employee*>& list);
    void rebuildIndexes();
};
