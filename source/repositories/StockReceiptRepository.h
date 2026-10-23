// STOCK RECEIPT REPOSITORY - Tầng truy xuất dữ liệu phiếu nhập kho
// Đổi tên file từ stockReceiptRepo.h → StockReceiptRepository.h (nhất quán PascalCase)
#ifndef STOCKRECEIPTREPOSITORY_H
#define STOCKRECEIPTREPOSITORY_H

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "../models/ImportReceipt.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/FileStore.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu phiếu nhập kho từ/vào tệp tin.
// Lưu trữ danh sách ImportReceipt trên RAM bằng Vector<ImportReceipt>.
class StockReceiptRepository {
private:
    std::string filePath;           // Đường dẫn đến file StockReceipts.txt
    Vector<ImportReceipt> receipts; // Danh sách phiếu nhập kho trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng
    StockReceiptRepository(std::string path = "data/Device/StockReceipts.txt");

    // Nạp dữ liệu từ file vào RAM
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file
    bool saveToFile() const;

    // Trả về toàn bộ danh sách
    const Vector<ImportReceipt>& getAll() const;
    Vector<ImportReceipt>& getAll();

    // Tìm theo ID phiếu nhập kho
    ImportReceipt* findById(const std::string& receiptId);
    const ImportReceipt* findById(const std::string& receiptId) const;

    // Lọc tất cả phiếu nhập của một nhà cung cấp
    Vector<ImportReceipt> findBySupplierId(const std::string& supplierId) const;

    // Lọc tất cả phiếu nhập do một nhân viên kho lập
    Vector<ImportReceipt> findByEmployeeId(const std::string& employeeId) const;

    // Kiểm tra xem có phiếu nào chứa sản phẩm với ID tương ứng không
    bool containsPhone(const std::string& phoneId) const;

    // CRUD: Thêm, cập nhật, xóa
    bool addReceipt(const ImportReceipt& receipt);
    bool updateReceipt(const ImportReceipt& updatedReceipt);
    bool deleteReceipt(const std::string& id);

    // Kiểm tra ID có tồn tại không
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

// Tạo file StockReceipts.txt mới với header chuẩn bằng FileStore
inline bool StockReceiptRepository::createEmptyFile() const {
    std::vector<std::string> lines = {
        "id|supplierId|employeeId|importTime|details|status"
    };
    bool ok = FileStore::ghiFileAnToan(filePath, lines);
    if (ok) {
        std::cout << "[Info] Da tao file moi: " << filePath << std::endl;
    }
    return ok;
}

inline StockReceiptRepository::StockReceiptRepository(std::string path) : filePath(path) {}

// Nạp dữ liệu từ StockReceipts.txt vào RAM bằng FileStore
inline bool StockReceiptRepository::loadFromFile() {
    receipts.clear();
    auto rows = FileStore::docFile(filePath);
    if (rows.empty()) {
        std::error_code ec;
        if (!std::filesystem::exists(filePath, ec)) {
            createEmptyFile();
        }
        return true;
    }

    for (size_t i = 0; i < rows.size(); ++i) {
        if (i == 0 && !rows[i].empty() && rows[i][0] == "id") {
            continue; // Bỏ qua tiêu đề
        }
        std::string line = FileStore::ghepTruong(rows[i]);
        try {
            ImportReceipt rec = ImportReceipt::fromFileLine(line);
            if (!rec.getID().empty()) {
                receipts.push_back(rec);
            }
        } catch (const std::exception& e) {
            std::cerr << "[Canh bao] Loi doc dong " << (i + 1)
                      << " trong " << filePath << ": " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[Canh bao] Loi khong xac dinh tai dong " << (i + 1)
                      << " trong " << filePath << std::endl;
        }
    }
    return true;
}

// Ghi toàn bộ danh sách receipts xuống file bằng FileStore
inline bool StockReceiptRepository::saveToFile() const {
    std::vector<std::string> lines;
    lines.push_back("id|supplierId|employeeId|importTime|details|status");
    for (int i = 0; i < receipts.getSize(); ++i) {
        lines.push_back(receipts[i].toFileString());
    }
    return FileStore::ghiFileAnToan(filePath, lines);
}

inline const Vector<ImportReceipt>& StockReceiptRepository::getAll() const { return receipts; }
inline Vector<ImportReceipt>& StockReceiptRepository::getAll() { return receipts; }

// Tìm ImportReceipt theo ID
inline ImportReceipt* StockReceiptRepository::findById(const std::string& receiptId) {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == receiptId) return &receipts[i];
    }
    return nullptr;
}

inline const ImportReceipt* StockReceiptRepository::findById(const std::string& receiptId) const {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == receiptId) return &receipts[i];
    }
    return nullptr;
}

// Lọc tất cả phiếu nhập của một nhà cung cấp
inline Vector<ImportReceipt> StockReceiptRepository::findBySupplierId(const std::string& supplierId) const {
    Vector<ImportReceipt> result;
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getSupplierId() == supplierId) {
            result.push_back(receipts[i]);
        }
    }
    return result;
}

// Lọc tất cả phiếu nhập do một nhân viên kho lập
inline Vector<ImportReceipt> StockReceiptRepository::findByEmployeeId(const std::string& employeeId) const {
    Vector<ImportReceipt> result;
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getEmployeeId() == employeeId) {
            result.push_back(receipts[i]);
        }
    }
    return result;
}

// Kiểm tra xem có phiếu nào chứa sản phẩm với ID tương ứng không
inline bool StockReceiptRepository::containsPhone(const std::string& phoneId) const {
    for (int i = 0; i < receipts.getSize(); ++i) {
        const Vector<OrderDetail>& details = receipts[i].getDetails();
        for (int j = 0; j < details.getSize(); ++j) {
            if (details[j].getPhoneId() == phoneId) return true;
        }
    }
    return false;
}

// Thêm phiếu nhập kho mới
inline bool StockReceiptRepository::addReceipt(const ImportReceipt& receipt) {
    if (existsById(receipt.getID())) {
        std::cerr << "[Canh bao] ID phieu nhap da ton tai: " << receipt.getID() << std::endl;
        return false;
    }
    receipts.push_back(receipt);
    return saveToFile();
}

// Cập nhật phiếu nhập kho theo ID
inline bool StockReceiptRepository::updateReceipt(const ImportReceipt& updatedReceipt) {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == updatedReceipt.getID()) {
            receipts[i] = updatedReceipt;
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay phieu nhap ID: " << updatedReceipt.getID() << std::endl;
    return false;
}

// Xóa phiếu nhập kho theo ID
inline bool StockReceiptRepository::deleteReceipt(const std::string& id) {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == id) {
            receipts.removeAt(i);
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay phieu nhap ID de xoa: " << id << std::endl;
    return false;
}

inline bool StockReceiptRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // STOCKRECEIPTREPOSITORY_H
