// CUSTOMER REPOSITORY - Tầng truy xuất dữ liệu khách hàng
#ifndef CUSTOMERREPOSITORY_H
#define CUSTOMERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Customer.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu khách hàng từ/vào tệp tin.
// Lưu trữ danh sách Customer trên RAM bằng Vector<Customer>.
class CustomerRepository {
private:
    std::string filePath;       // Đường dẫn đến file Customers.txt
    Vector<Customer> customers; // Danh sách khách hàng trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng: nhận đường dẫn file (mặc định là path chuẩn)
    CustomerRepository(std::string path = "data/people/Customers.txt");

    // Nạp dữ liệu từ file vào RAM. Tự tạo file nếu chưa có.
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file (ghi đè).
    bool saveToFile() const;

    // Trả về toàn bộ danh sách (chỉ đọc / cho phép sửa)
    const Vector<Customer>& getAll() const;
    Vector<Customer>& getAll();

    // Tìm theo ID khách hàng
    Customer* findById(const std::string& id);
    const Customer* findById(const std::string& id) const;

    // Tìm theo username tài khoản (dùng cho đăng nhập)
    Customer* findByUsername(const std::string& username);
    const Customer* findByUsername(const std::string& username) const;

    // Tìm kiếm theo tên (không phân biệt hoa thường)
    Vector<Customer> searchByName(const std::string& keyword) const;

    // CRUD: Thêm, cập nhật, xóa
    bool addCustomer(const Customer& cust);
    bool updateCustomer(const Customer& updatedCust);
    bool deleteCustomer(const std::string& id);

    // Kiểm tra ID / username có tồn tại không
    bool existsById(const std::string& id) const;
    bool existsByUsername(const std::string& username) const;
};

// ================= IMPLEMENTATION =================

// Tạo file Customers.txt mới với header chuẩn
inline bool CustomerRepository::createEmptyFile() const {
    std::ofstream newFile(filePath);
    if (!newFile.is_open()) {
        std::cerr << "[Loi] Khong the tao file moi: " << filePath << std::endl;
        return false;
    }
    newFile << "id|firstName|middleName|lastName|phoneNumber|Email|username|password|points|address\n";
    newFile.close();
    std::cout << "[Info] Da tao file moi: " << filePath << std::endl;
    return true;
}

inline CustomerRepository::CustomerRepository(std::string path) : filePath(path) {}

// Nạp dữ liệu từ Customers.txt vào RAM
// - Nếu file không tồn tại: tự tạo file mới và trả về true (danh sách rỗng)
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool CustomerRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    customers.clear();
    std::string line;
    bool isHeader = true;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        line = StringHelper::trim(line);
        if (line.empty()) continue;

        if (isHeader) {
            isHeader = false; // Bỏ qua dòng tiêu đề
            continue;
        }

        try {
            Customer c = Customer::fromFileLine(line);
            if (!c.getID().empty()) {
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
    return true;
}

// Ghi toàn bộ danh sách customers xuống file (ghi đè)
inline bool CustomerRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Loi] Khong the ghi file: " << filePath << std::endl;
        return false;
    }
    // Header khớp với định dạng Customer::toFileString() (cột 9 là points)
    file << "id|firstName|middleName|lastName|phoneNumber|Email|username|password|points|address\n";
    for (int i = 0; i < customers.getSize(); ++i) {
        file << customers[i].toFileString() << "\n";
    }
    file.close();
    return true;
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

// Thêm khách hàng mới (kiểm tra ID trùng trước)
inline bool CustomerRepository::addCustomer(const Customer& cust) {
    if (existsById(cust.getID())) {
        std::cerr << "[Canh bao] ID khach hang da ton tai: " << cust.getID() << std::endl;
        return false;
    }
    customers.push_back(cust);
    return saveToFile();
}

// Cập nhật khách hàng theo ID
inline bool CustomerRepository::updateCustomer(const Customer& updatedCust) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == updatedCust.getID()) {
            customers[i] = updatedCust;
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de cap nhat: " << updatedCust.getID() << std::endl;
    return false;
}

// Xóa khách hàng theo ID
inline bool CustomerRepository::deleteCustomer(const std::string& id) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == id) {
            customers.removeAt(i);
            return saveToFile();
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
