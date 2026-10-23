// DISCOUNT REPOSITORY - Tầng truy xuất dữ liệu mã giảm giá
#ifndef DISCOUNTREPOSITORY_H
#define DISCOUNTREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/DiscountCode.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu mã giảm giá từ/vào tệp tin.
// Lưu trữ danh sách DiscountCode trên RAM bằng Vector<DiscountCode>.
class DiscountRepository {
private:
    std::string filePath;           // Đường dẫn đến file Discount.txt
    Vector<DiscountCode> discounts; // Danh sách mã giảm giá trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng
    DiscountRepository(std::string path = "data/PAY/Discount.txt");

    // Nạp dữ liệu từ file vào RAM
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file
    bool saveToFile() const;

    // Trả về toàn bộ danh sách
    const Vector<DiscountCode>& getAll() const;
    Vector<DiscountCode>& getAll();

    // Tìm theo mã code
    DiscountCode* findByCode(const std::string& code);
    const DiscountCode* findByCode(const std::string& code) const;

    // Lọc chỉ các mã còn hiệu lực tại thời điểm hiện tại
    Vector<DiscountCode> findValidCodes() const;

    // CRUD: Thêm, cập nhật, xóa
    bool addDiscount(const DiscountCode& dc);
    bool updateDiscount(const DiscountCode& updatedDc);
    bool deleteDiscount(const std::string& code);

    // Kiểm tra code có tồn tại không
    bool existsByCode(const std::string& code) const;
};

// ================= IMPLEMENTATION =================

// Tạo file Discount.txt mới với header chuẩn
inline bool DiscountRepository::createEmptyFile() const {
    std::ofstream newFile(filePath);
    if (!newFile.is_open()) {
        std::cerr << "[Loi] Khong the tao file moi: " << filePath << std::endl;
        return false;
    }
    newFile << "code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount\n";
    newFile.close();
    std::cout << "[Info] Da tao file moi: " << filePath << std::endl;
    return true;
}

inline DiscountRepository::DiscountRepository(std::string path) : filePath(path) {}

// Nạp dữ liệu từ Discount.txt vào RAM
// - Gọi DiscountCode::fromFileLine() để phân tích mỗi dòng
// - Nếu file không tồn tại: tự tạo file mới
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool DiscountRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    discounts.clear();
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
            DiscountCode dc = DiscountCode::fromFileLine(line);
            if (!dc.getCode().empty()) {
                discounts.push_back(dc);
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

// Ghi toàn bộ danh sách discounts xuống file
// Dùng DiscountCode::toFileString() (nếu có) hoặc tự build chuỗi
inline bool DiscountRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Loi] Khong the ghi file: " << filePath << std::endl;
        return false;
    }
    file << "code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount\n";
    for (int i = 0; i < discounts.getSize(); ++i) {
        const DiscountCode& dc = discounts[i];
        // Build chuỗi theo đúng format: code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount
        file << dc.getCode() << "|"
             << (dc.getType() == DiscountType::PERCENTAGE ? "0" : "1") << "|"
             << static_cast<long long>(dc.getValue()) << "|"
             << dc.getStartDate().toDateString() << "|"
             << dc.getEndDate().toDateString() << "|"
             << static_cast<long long>(dc.getMinOrderValue()) << "|"
             << static_cast<long long>(dc.getMaxDiscountValue()) << "|"
             << dc.getMaxUsage() << "|"
             << dc.getUsedCount() << "\n";
    }
    file.close();
    return true;
}

inline const Vector<DiscountCode>& DiscountRepository::getAll() const { return discounts; }
inline Vector<DiscountCode>& DiscountRepository::getAll() { return discounts; }

// Tìm DiscountCode theo mã code
inline DiscountCode* DiscountRepository::findByCode(const std::string& code) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == code) return &discounts[i];
    }
    return nullptr;
}

inline const DiscountCode* DiscountRepository::findByCode(const std::string& code) const {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == code) return &discounts[i];
    }
    return nullptr;
}

// Lọc chỉ các mã còn hiệu lực tại thời điểm hiện tại
inline Vector<DiscountCode> DiscountRepository::findValidCodes() const {
    Vector<DiscountCode> result;
    DateTime now = DateTime::now();
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].isValid(now)) {
            result.push_back(discounts[i]);
        }
    }
    return result;
}

// Thêm mã giảm giá mới
inline bool DiscountRepository::addDiscount(const DiscountCode& dc) {
    if (existsByCode(dc.getCode())) {
        std::cerr << "[Canh bao] Ma giam gia da ton tai: " << dc.getCode() << std::endl;
        return false;
    }
    discounts.push_back(dc);
    return saveToFile();
}

// Cập nhật mã giảm giá theo code
inline bool DiscountRepository::updateDiscount(const DiscountCode& updatedDc) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == updatedDc.getCode()) {
            discounts[i] = updatedDc;
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ma de cap nhat: " << updatedDc.getCode() << std::endl;
    return false;
}

// Xóa mã giảm giá theo code
inline bool DiscountRepository::deleteDiscount(const std::string& code) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == code) {
            discounts.removeAt(i);
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ma de xoa: " << code << std::endl;
    return false;
}

inline bool DiscountRepository::existsByCode(const std::string& code) const {
    return findByCode(code) != nullptr;
}

#endif // DISCOUNTREPOSITORY_H
