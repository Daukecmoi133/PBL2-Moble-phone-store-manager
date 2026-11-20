// INVOICE REPOSITORY - Tầng truy xuất dữ liệu hóa đơn thanh toán
#ifndef INVOICEREPOSITORY_H
#define INVOICEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Invoice.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"
#include "../utils/SchemaHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu hóa đơn từ/vào tệp tin.
// Lưu trữ danh sách Invoice trên RAM bằng Vector<Invoice>.
class InvoiceRepository {
private:
    std::string filePath;       // Đường dẫn đến file Invoices.txt
    Vector<Invoice> invoices;   // Danh sách hóa đơn trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng
    InvoiceRepository(std::string path = "data/PAY/Invoices.txt");

    // Nạp dữ liệu từ file vào RAM
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file
    bool saveToFile() const;
    const std::string& getFilePath() const { return filePath; }

    // Trả về toàn bộ danh sách
    const Vector<Invoice>& getAll() const;
    Vector<Invoice>& getAll();

    // Tìm theo ID hóa đơn
    Invoice* findById(const std::string& invoiceId);
    const Invoice* findById(const std::string& invoiceId) const;

    // Tìm theo ID đơn hàng liên kết
    Invoice* findByOrderId(const std::string& orderId);
    const Invoice* findByOrderId(const std::string& orderId) const;

    // Lọc tất cả hóa đơn của một khách hàng
    Vector<Invoice> findByCustomerId(const std::string& customerId) const;

    // Lọc tất cả hóa đơn do một nhân viên lập
    Vector<Invoice> findByEmployeeId(const std::string& employeeId) const;

    // Kiểm tra xem có hóa đơn nào chứa sản phẩm với ID tương ứng không
    bool containsPhone(const std::string& phoneId) const;

    // CRUD: Thêm, cập nhật, xóa
    bool addInvoice(const Invoice& invoice);
    bool updateInvoice(const Invoice& updatedInvoice);
    bool deleteInvoice(const std::string& id);

    // Kiểm tra ID có tồn tại không
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

// Tạo file Invoices.txt mới với header chuẩn (9 cột bao gồm discountCode)
inline bool InvoiceRepository::createEmptyFile() const {
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        SchemaHelper::INVOICES_HEADER
    );
}

inline InvoiceRepository::InvoiceRepository(std::string path)
    : filePath(PathHelper::resolveDataPath(path)) {}

// Nạp dữ liệu từ Invoices.txt vào RAM
// - Gọi Invoice::fromFileLine() để phân tích mỗi dòng
// - Nếu file không tồn tại: tự tạo file mới
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool InvoiceRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    invoices.clear();
    std::string line;
    bool isFirstNonEmpty = true;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        if (lineNum == 1) StringHelper::stripUtf8Bom(line);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (StringHelper::trim(line).empty()) continue;

        if (isFirstNonEmpty) {
            isFirstNonEmpty = false;
            auto kind = SchemaHelper::inspectFirstLine(
                line,
                SchemaHelper::INVOICES_HEADER,
                {SchemaHelper::INVOICES_V2_HEADER, SchemaHelper::INVOICES_V1_HEADER},
                filePath,
                [](const std::string& l) {
                    return SchemaHelper::looksLikePrefixedRecord(l, "INV");
                });
            if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                continue;
            }
        }

        try {
            Invoice inv = Invoice::fromFileLine(line);
            if (!inv.getID().empty()) {
                invoices.push_back(inv);
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

// Ghi toàn bộ danh sách invoices xuống file (qua Atomic File Write)
inline bool InvoiceRepository::saveToFile() const {
    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        file << SchemaHelper::INVOICES_HEADER << "\n";
        for (int i = 0; i < invoices.getSize(); ++i) {
            file << invoices[i].toFileString() << "\n";
        }
        return file.good();
    });
}

inline const Vector<Invoice>& InvoiceRepository::getAll() const { return invoices; }
inline Vector<Invoice>& InvoiceRepository::getAll() { return invoices; }

// Tìm Invoice theo ID
inline Invoice* InvoiceRepository::findById(const std::string& invoiceId) {
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getID() == invoiceId) return &invoices[i];
    }
    return nullptr;
}

inline const Invoice* InvoiceRepository::findById(const std::string& invoiceId) const {
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getID() == invoiceId) return &invoices[i];
    }
    return nullptr;
}

// Tìm Invoice theo orderId
inline Invoice* InvoiceRepository::findByOrderId(const std::string& orderId) {
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getOrderId() == orderId) return &invoices[i];
    }
    return nullptr;
}

inline const Invoice* InvoiceRepository::findByOrderId(const std::string& orderId) const {
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getOrderId() == orderId) return &invoices[i];
    }
    return nullptr;
}

// Lọc tất cả hóa đơn của một khách hàng
inline Vector<Invoice> InvoiceRepository::findByCustomerId(const std::string& customerId) const {
    Vector<Invoice> result;
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getCustomerId() == customerId) {
            result.push_back(invoices[i]);
        }
    }
    return result;
}

// Lọc tất cả hóa đơn do một nhân viên lập
inline Vector<Invoice> InvoiceRepository::findByEmployeeId(const std::string& employeeId) const {
    Vector<Invoice> result;
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getEmployeeId() == employeeId) {
            result.push_back(invoices[i]);
        }
    }
    return result;
}

// Kiểm tra xem có hóa đơn nào chứa sản phẩm với ID tương ứng không
inline bool InvoiceRepository::containsPhone(const std::string& phoneId) const {
    for (int i = 0; i < invoices.getSize(); ++i) {
        const Vector<OrderDetail>& details = invoices[i].getDetails();
        for (int j = 0; j < details.getSize(); ++j) {
            if (details[j].getPhoneId() == phoneId) return true;
        }
    }
    return false;
}

// Thêm hóa đơn mới (rollback RAM nếu ghi file thất bại)
inline bool InvoiceRepository::addInvoice(const Invoice& invoice) {
    if (existsById(invoice.getID())) {
        std::cerr << "[Canh bao] ID hoa don da ton tai: " << invoice.getID() << std::endl;
        return false;
    }
    Vector<Invoice> backup = invoices;
    invoices.push_back(invoice);
    if (!saveToFile()) {
        invoices = backup; // Rollback RAM
        return false;
    }
    return true;
}

// Cập nhật hóa đơn theo ID (rollback RAM nếu ghi file thất bại)
inline bool InvoiceRepository::updateInvoice(const Invoice& updatedInvoice) {
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getID() == updatedInvoice.getID()) {
            Vector<Invoice> backup = invoices;
            invoices[i] = updatedInvoice;
            if (!saveToFile()) {
                invoices = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay hoa don ID: " << updatedInvoice.getID() << std::endl;
    return false;
}

// Xóa hóa đơn theo ID (rollback RAM nếu ghi file thất bại)
inline bool InvoiceRepository::deleteInvoice(const std::string& id) {
    for (int i = 0; i < invoices.getSize(); ++i) {
        if (invoices[i].getID() == id) {
            Vector<Invoice> backup = invoices;
            invoices.removeAt(i);
            if (!saveToFile()) {
                invoices = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay hoa don ID de xoa: " << id << std::endl;
    return false;
}

inline bool InvoiceRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // INVOICEREPOSITORY_H