// PHONE REPOSITORY - Tầng truy xuất dữ liệu sản phẩm điện thoại
#ifndef PHONEREPOSITORY_H
#define PHONEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Phone.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu sản phẩm từ/vào tệp tin.
// Lưu trữ danh sách Phone trên RAM bằng Vector<Phone>.
class PhoneRepository {
private:
    std::string filePath;    // Đường dẫn đến file Products.txt
    Vector<Phone> phones;    // Danh sách sản phẩm trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng: nhận đường dẫn file (mặc định là path chuẩn)
    PhoneRepository(std::string path = "data/Device/Products.txt");

    // Nạp dữ liệu từ file vào RAM. Tự tạo file mới nếu file chưa tồn tại.
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file (ghi đè).
    bool saveToFile() const;

    // Trả về toàn bộ danh sách (chỉ đọc / cho phép sửa)
    const Vector<Phone>& getAll() const;
    Vector<Phone>& getAll();

    // Tìm theo ID - trả về con trỏ (nullptr nếu không tìm thấy)
    Phone* findById(const std::string& id);
    const Phone* findById(const std::string& id) const;

    // Tìm kiếm theo tên (không phân biệt hoa thường, tìm substring)
    Vector<Phone> searchByName(const std::string& keyword) const;

    // Tìm kiếm theo hãng sản xuất
    Vector<Phone> findByBrand(const std::string& brand) const;

    // CRUD: Thêm, cập nhật, xóa (đều lưu file ngay sau khi thay đổi)
    bool addPhone(const Phone& phone);
    bool updatePhone(const Phone& updatedPhone);
    bool deletePhone(const std::string& id);

    // Kiểm tra ID có tồn tại không (tiện cho validation)
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

// Tạo file mới với dòng header chuẩn
inline bool PhoneRepository::createEmptyFile() const {
    std::ofstream newFile(filePath);
    if (!newFile.is_open()) {
        std::cerr << "[Loi] Khong the tao file moi: " << filePath << std::endl;
        return false;
    }
    newFile << "id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications\n";
    newFile.close();
    std::cout << "[Info] Da tao file moi: " << filePath << std::endl;
    return true;
}

inline PhoneRepository::PhoneRepository(std::string path) : filePath(path) {}

// Nạp dữ liệu từ Products.txt vào RAM
// - Nếu file không tồn tại: tự tạo file mới và trả về true (danh sách rỗng)
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool PhoneRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    phones.clear();
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
            Phone p = Phone::fromFileLine(line);
            if (!p.getID().empty()) {
                phones.push_back(p);
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

// Ghi toàn bộ danh sách phones xuống file (ghi đè)
inline bool PhoneRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Loi] Khong the ghi file: " << filePath << std::endl;
        return false;
    }
    // Ghi header đúng với định dạng 9 cột
    file << "id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications\n";
    for (int i = 0; i < phones.getSize(); ++i) {
        file << phones[i].toFileString() << "\n";
    }
    file.close();
    return true;
}

inline const Vector<Phone>& PhoneRepository::getAll() const { return phones; }
inline Vector<Phone>& PhoneRepository::getAll() { return phones; }

// Tìm Phone theo ID (trả về con trỏ trực tiếp vào RAM để có thể sửa)
inline Phone* PhoneRepository::findById(const std::string& id) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == id) return &phones[i];
    }
    return nullptr;
}

inline const Phone* PhoneRepository::findById(const std::string& id) const {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == id) return &phones[i];
    }
    return nullptr;
}

// Tìm kiếm theo tên sản phẩm (không phân biệt hoa thường)
inline Vector<Phone> PhoneRepository::searchByName(const std::string& keyword) const {
    Vector<Phone> result;
    for (int i = 0; i < phones.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(phones[i].getName(), keyword)) {
            result.push_back(phones[i]);
        }
    }
    return result;
}

// Lọc theo hãng sản xuất
inline Vector<Phone> PhoneRepository::findByBrand(const std::string& brand) const {
    Vector<Phone> result;
    for (int i = 0; i < phones.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(phones[i].getBrand(), brand)) {
            result.push_back(phones[i]);
        }
    }
    return result;
}

// Thêm sản phẩm mới và lưu ngay
inline bool PhoneRepository::addPhone(const Phone& phone) {
    if (existsById(phone.getID())) {
        std::cerr << "[Canh bao] ID da ton tai: " << phone.getID() << std::endl;
        return false;
    }
    phones.push_back(phone);
    return saveToFile();
}

// Cập nhật thông tin sản phẩm và lưu ngay
inline bool PhoneRepository::updatePhone(const Phone& updatedPhone) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == updatedPhone.getID()) {
            phones[i] = updatedPhone;
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de cap nhat: " << updatedPhone.getID() << std::endl;
    return false;
}

// Xóa sản phẩm theo ID và lưu ngay
inline bool PhoneRepository::deletePhone(const std::string& id) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == id) {
            phones.removeAt(i);
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de xoa: " << id << std::endl;
    return false;
}

// Kiểm tra ID có tồn tại không
inline bool PhoneRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // PHONEREPOSITORY_H
