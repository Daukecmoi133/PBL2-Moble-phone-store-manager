// SUPPLIER REPOSITORY - Tầng truy xuất dữ liệu nhà cung cấp
#ifndef SUPPLIERREPOSITORY_H
#define SUPPLIERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Supplier.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu nhà cung cấp từ/vào tệp tin.
// Lưu trữ danh sách Supplier trên RAM bằng Vector<Supplier>.
class SupplierRepository {
private:
    std::string filePath;       // Đường dẫn đến file Suppliers.txt
    Vector<Supplier> suppliers; // Danh sách nhà cung cấp trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng
    SupplierRepository(std::string path = "data/people/Suppliers.txt");

    // Nạp dữ liệu từ file vào RAM
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file
    bool saveToFile() const;
    const std::string& getFilePath() const { return filePath; }

    // Trả về toàn bộ danh sách
    const Vector<Supplier>& getAll() const;
    Vector<Supplier>& getAll();

    // Tìm theo ID nhà cung cấp
    Supplier* findById(const std::string& id);
    const Supplier* findById(const std::string& id) const;

    // Tìm kiếm theo tên công ty (không phân biệt hoa thường)
    Vector<Supplier> searchByCompanyName(const std::string& keyword) const;

    // CRUD: Thêm, cập nhật, xóa
    bool addSupplier(const Supplier& sup);
    bool updateSupplier(const Supplier& updatedSup);
    bool deleteSupplier(const std::string& id);

    // Kiểm tra ID có tồn tại không
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

// Tạo file Suppliers.txt mới với header chuẩn
inline bool SupplierRepository::createEmptyFile() const {
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        "id|companyName|taxCode|contactPerson|phoneNumber|email|address"
    );
}

inline SupplierRepository::SupplierRepository(std::string path)
    : filePath(PathHelper::resolveDataPath(path)) {}

// Nạp dữ liệu từ Suppliers.txt vào RAM
// - Gọi Supplier::fromFileLine() để phân tích mỗi dòng
// - Nếu file không tồn tại: tự tạo file mới
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool SupplierRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    suppliers.clear();
    std::string line;
    bool isHeader = true;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        line = StringHelper::trim(line);
        if (line.empty()) continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        try {
            Supplier sup = Supplier::fromFileLine(line);
            if (!sup.getID().empty()) {
                suppliers.push_back(sup);
            }
        } catch (const std::exception& e) {
            std::cerr << "[Canh bao] Loi doc dong " << lineNum
                      << " trong " << filePath << ": " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[Canh bao] Loi khong xac dinh tai dong " << lineNum
                      << " trong " << filePath << std::endl;
        }
    }
    file.close();
    return true;
}

// Ghi toàn bộ danh sách suppliers xuống file (qua Atomic File Write)
inline bool SupplierRepository::saveToFile() const {
    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        file << "id|companyName|taxCode|contactPerson|phoneNumber|email|address\n";
        for (int i = 0; i < suppliers.getSize(); ++i) {
            file << suppliers[i].toFileString() << "\n";
        }
        return file.good();
    });
}

inline const Vector<Supplier>& SupplierRepository::getAll() const { return suppliers; }
inline Vector<Supplier>& SupplierRepository::getAll() { return suppliers; }

// Tìm Supplier theo ID
inline Supplier* SupplierRepository::findById(const std::string& id) {
    for (int i = 0; i < suppliers.getSize(); ++i) {
        if (suppliers[i].getID() == id) return &suppliers[i];
    }
    return nullptr;
}

inline const Supplier* SupplierRepository::findById(const std::string& id) const {
    for (int i = 0; i < suppliers.getSize(); ++i) {
        if (suppliers[i].getID() == id) return &suppliers[i];
    }
    return nullptr;
}

// Tìm kiếm theo tên công ty (không phân biệt hoa thường)
inline Vector<Supplier> SupplierRepository::searchByCompanyName(const std::string& keyword) const {
    Vector<Supplier> result;
    for (int i = 0; i < suppliers.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(suppliers[i].getCompanyName(), keyword)) {
            result.push_back(suppliers[i]);
        }
    }
    return result;
}

// Thêm nhà cung cấp mới (rollback RAM nếu ghi file thất bại)
inline bool SupplierRepository::addSupplier(const Supplier& sup) {
    if (existsById(sup.getID())) {
        std::cerr << "[Canh bao] ID nha cung cap da ton tai: " << sup.getID() << std::endl;
        return false;
    }
    Vector<Supplier> backup = suppliers;
    suppliers.push_back(sup);
    if (!saveToFile()) {
        suppliers = backup; // Rollback RAM
        return false;
    }
    return true;
}

// Cập nhật nhà cung cấp theo ID (rollback RAM nếu ghi file thất bại)
inline bool SupplierRepository::updateSupplier(const Supplier& updatedSup) {
    for (int i = 0; i < suppliers.getSize(); ++i) {
        if (suppliers[i].getID() == updatedSup.getID()) {
            Vector<Supplier> backup = suppliers;
            suppliers[i] = updatedSup;
            if (!saveToFile()) {
                suppliers = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay NCC ID: " << updatedSup.getID() << std::endl;
    return false;
}

// Xóa nhà cung cấp theo ID (rollback RAM nếu ghi file thất bại)
inline bool SupplierRepository::deleteSupplier(const std::string& id) {
    for (int i = 0; i < suppliers.getSize(); ++i) {
        if (suppliers[i].getID() == id) {
            Vector<Supplier> backup = suppliers;
            suppliers.removeAt(i);
            if (!saveToFile()) {
                suppliers = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay NCC ID de xoa: " << id << std::endl;
    return false;
}

inline bool SupplierRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // SUPPLIERREPOSITORY_H
