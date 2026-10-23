// SUPPLIER REPOSITORY - Tầng truy xuất dữ liệu nhà cung cấp
#ifndef SUPPLIERREPOSITORY_H
#define SUPPLIERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Supplier.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

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
    std::ofstream newFile(filePath);
    if (!newFile.is_open()) {
        std::cerr << "[Loi] Khong the tao file moi: " << filePath << std::endl;
        return false;
    }
    newFile << "id|companyName|taxCode|contactPerson|phoneNumber|email|address\n";
    newFile.close();
    std::cout << "[Info] Da tao file moi: " << filePath << std::endl;
    return true;
}

inline SupplierRepository::SupplierRepository(std::string path) : filePath(path) {}

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

// Ghi toàn bộ danh sách suppliers xuống file (dùng Supplier::toFileString())
inline bool SupplierRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Loi] Khong the ghi file: " << filePath << std::endl;
        return false;
    }
    file << "id|companyName|taxCode|contactPerson|phoneNumber|email|address\n";
    for (int i = 0; i < suppliers.getSize(); ++i) {
        file << suppliers[i].toFileString() << "\n";
    }
    file.close();
    return true;
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

// Thêm nhà cung cấp mới
inline bool SupplierRepository::addSupplier(const Supplier& sup) {
    if (existsById(sup.getID())) {
        std::cerr << "[Canh bao] ID nha cung cap da ton tai: " << sup.getID() << std::endl;
        return false;
    }
    suppliers.push_back(sup);
    return saveToFile();
}

// Cập nhật nhà cung cấp theo ID
inline bool SupplierRepository::updateSupplier(const Supplier& updatedSup) {
    for (int i = 0; i < suppliers.getSize(); ++i) {
        if (suppliers[i].getID() == updatedSup.getID()) {
            suppliers[i] = updatedSup;
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay NCC ID: " << updatedSup.getID() << std::endl;
    return false;
}

// Xóa nhà cung cấp theo ID
inline bool SupplierRepository::deleteSupplier(const std::string& id) {
    for (int i = 0; i < suppliers.getSize(); ++i) {
        if (suppliers[i].getID() == id) {
            suppliers.removeAt(i);
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay NCC ID de xoa: " << id << std::endl;
    return false;
}

inline bool SupplierRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // SUPPLIERREPOSITORY_H
