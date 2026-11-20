// CUSTOMER REPOSITORY - Tầng truy xuất dữ liệu khách hàng
#ifndef CUSTOMERREPOSITORY_H
#define CUSTOMERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Customer.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"
#include "../utils/SchemaHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu khách hàng từ/vào tệp tin.
// Lưu trữ danh sách Customer trên RAM bằng Vector<Customer>.
class CustomerRepository {
private:
    std::string filePath;       // Đường dẫn đến file Customers.txt
    Vector<Customer> customers; // Danh sách khách hàng trên RAM
    bool loadAttempted = false; // Đã gọi loadFromFile() chưa
    bool loadSucceeded = false; // Lần nạp gần nhất có đọc được dữ liệu không

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng: nhận đường dẫn file (mặc định là path chuẩn)
    CustomerRepository(std::string path = "data/people/Customers.txt");

    // Nạp dữ liệu từ file vào RAM. Tự tạo file nếu chưa có.
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file (ghi đè).
    bool saveToFile() const;
    const std::string& getFilePath() const { return filePath; }

    // true nếu đã gọi loadFromFile() nhưng KHÔNG đọc/tạo được file dữ liệu (xác thực phải báo lỗi lưu trữ,
    // không được coi như "tài khoản không tồn tại")
    bool hasLoadError() const { return loadAttempted && !loadSucceeded; }

    // Trả về toàn bộ danh sách (chỉ đọc / cho phép sửa)
    const Vector<Customer>& getAll() const;
    Vector<Customer>& getAll();

    // Tìm theo ID khách hàng
    Customer* findById(const std::string& id);
    const Customer* findById(const std::string& id) const;

    // Tìm theo username tài khoản (dùng cho đăng nhập)
    Customer* findByUsername(const std::string& username);
    const Customer* findByUsername(const std::string& username) const;

    // Tìm theo số điện thoại (khóa định danh tài khoản)
    Customer* findByPhone(const std::string& phone);
    const Customer* findByPhone(const std::string& phone) const;

    // Tìm kiếm theo tên (không phân biệt hoa thường)
    Vector<Customer> searchByName(const std::string& keyword) const;

    // CRUD: Thêm, cập nhật, xóa
    bool addCustomer(const Customer& cust);
    bool updateCustomer(const Customer& updatedCust);
    bool deleteCustomer(const std::string& id);

    // Kiểm tra ID / username / số điện thoại có tồn tại không
    bool existsById(const std::string& id) const;
    bool existsByUsername(const std::string& username) const;
    bool existsByPhone(const std::string& phone) const;
};

// ================= IMPLEMENTATION =================

// Tạo file Customers.txt mới với header chuẩn (12 cột)
inline bool CustomerRepository::createEmptyFile() const {
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        SchemaHelper::CUSTOMERS_HEADER
    );
}

inline CustomerRepository::CustomerRepository(std::string path)
    : filePath(PathHelper::resolveDataPath(path)) {}

// Nạp dữ liệu từ Customers.txt vào RAM
// - Nếu file không tồn tại: tự tạo file mới và trả về true (danh sách rỗng)
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool CustomerRepository::loadFromFile() {
    loadAttempted = true;
    loadSucceeded = false;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        loadSucceeded = createEmptyFile();
        return loadSucceeded;
    }

    customers.clear();
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
                SchemaHelper::CUSTOMERS_HEADER,
                {SchemaHelper::CUSTOMERS_V1_HEADER, SchemaHelper::CUSTOMERS_V1_HEADER_LOWER},
                filePath,
                [](const std::string& l) {
                    return SchemaHelper::looksLikePrefixedRecord(l, "CUS");
                });
            if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                continue;
            }
        }

        try {
            Customer c = Customer::fromFileLine(line);
            if (!c.getID().empty()) {
                if (!c.getAccount().isPasswordHashed()) {
                    std::cerr << "[CANH BAO AN NINH STARTUP] Khach hang '" << c.getAccount().getUsername()
                              << "' (ID: " << c.getID() << ") trong " << filePath
                              << " dang luu mat khau chua bam! Yeu cau cap nhat mat khau ngay lap tuc.\n";
                }
                customers.push_back(c);
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
    loadSucceeded = true;
    return true;
}

// Ghi toàn bộ danh sách customers xuống file (ghi đè)
inline bool CustomerRepository::saveToFile() const {
    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        // Header khớp với định dạng Customer::toFileString() (12 cột)
        file << SchemaHelper::CUSTOMERS_HEADER << "\n";
        for (int i = 0; i < customers.getSize(); ++i) {
            file << customers[i].toFileString() << "\n";
        }
        return file.good();
    });
}

inline const Vector<Customer>& CustomerRepository::getAll() const { return customers; }
inline Vector<Customer>& CustomerRepository::getAll() { return customers; }

// Tìm Customer theo ID
inline Customer* CustomerRepository::findById(const std::string& id) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == id) return &customers[i];
    }
    return nullptr;
}

inline const Customer* CustomerRepository::findById(const std::string& id) const {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == id) return &customers[i];
    }
    return nullptr;
}

// Tìm Customer theo username (dùng để xác thực đăng nhập)
inline Customer* CustomerRepository::findByUsername(const std::string& username) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getAccount().getUsername() == username) return &customers[i];
    }
    return nullptr;
}

inline const Customer* CustomerRepository::findByUsername(const std::string& username) const {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getAccount().getUsername() == username) return &customers[i];
    }
    return nullptr;
}

// Tìm Customer theo số điện thoại (khóa định danh tài khoản)
inline Customer* CustomerRepository::findByPhone(const std::string& phone) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getPhoneNumber() == phone) return &customers[i];
    }
    return nullptr;
}

inline const Customer* CustomerRepository::findByPhone(const std::string& phone) const {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getPhoneNumber() == phone) return &customers[i];
    }
    return nullptr;
}

inline bool CustomerRepository::existsByPhone(const std::string& phone) const {
    return findByPhone(phone) != nullptr;
}

// Tìm kiếm theo tên khách hàng (không phân biệt hoa thường)
inline Vector<Customer> CustomerRepository::searchByName(const std::string& keyword) const {
    Vector<Customer> result;
    for (int i = 0; i < customers.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(customers[i].getFullName(), keyword)) {
            result.push_back(customers[i]);
        }
    }
    return result;
}

// Thêm khách hàng mới (kiểm tra ID trùng trước, rollback RAM nếu ghi file thất bại)
inline bool CustomerRepository::addCustomer(const Customer& cust) {
    if (existsById(cust.getID())) {
        std::cerr << "[Canh bao] ID khach hang da ton tai: " << cust.getID() << std::endl;
        return false;
    }
    Vector<Customer> backup = customers;
    customers.push_back(cust);
    if (!saveToFile()) {
        customers = backup; // Rollback RAM
        return false;
    }
    return true;
}

// Cập nhật khách hàng theo ID (rollback RAM nếu ghi file thất bại)
inline bool CustomerRepository::updateCustomer(const Customer& updatedCust) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == updatedCust.getID()) {
            Vector<Customer> backup = customers;
            customers[i] = updatedCust;
            if (!saveToFile()) {
                customers = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de cap nhat: " << updatedCust.getID() << std::endl;
    return false;
}

// Xóa khách hàng theo ID (rollback RAM nếu ghi file thất bại)
inline bool CustomerRepository::deleteCustomer(const std::string& id) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == id) {
            Vector<Customer> backup = customers;
            customers.removeAt(i);
            if (!saveToFile()) {
                customers = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de xoa: " << id << std::endl;
    return false;
}

inline bool CustomerRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

inline bool CustomerRepository::existsByUsername(const std::string& username) const {
    return findByUsername(username) != nullptr;
}

#endif // CUSTOMERREPOSITORY_H
