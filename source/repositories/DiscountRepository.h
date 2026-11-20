// DISCOUNT REPOSITORY - Tầng truy xuất dữ liệu mã giảm giá
#ifndef DISCOUNTREPOSITORY_H
#define DISCOUNTREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/DiscountCode.h"
#include "../core/Vector.h"
#include "../utils/AppClock.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"
#include "../utils/SchemaHelper.h"

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
    const std::string& getFilePath() const { return filePath; }

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
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        SchemaHelper::DISCOUNTS_HEADER
    );
}

inline DiscountRepository::DiscountRepository(std::string path)
    : filePath(PathHelper::resolveDataPath(path)) {}

// Nạp dữ liệu từ Discount.txt vào RAM
// - Phân biệt rõ: File chưa tồn tại (seed 1 lần), File rỗng (sửa header), File có dữ liệu hợp lệ
// - Không tự động seed lại dữ liệu mặc định khi người dùng đã chủ động xóa toàn bộ bản ghi
inline bool DiscountRepository::loadFromFile() {
    std::error_code ec;
    bool fileExisted = std::filesystem::exists(filePath, ec);

    if (!fileExisted) {
        // File chưa từng tồn tại: Tạo file và nạp seed data chuẩn ban đầu
        discounts.clear();
        discounts.push_back(DiscountCode("SALE10", DiscountType::PERCENTAGE, 10.0,
            DateTime(2026, 1, 1, 0, 0, 0), DateTime(2026, 12, 31, 23, 59, 59),
            5000000.0, 500000.0, 100, 12));
        discounts.push_back(DiscountCode("GIAM500K", DiscountType::FIXED_AMOUNT, 500000.0,
            DateTime(2026, 1, 1, 0, 0, 0), DateTime(2026, 11, 30, 23, 59, 59),
            10000000.0, 500000.0, 200, 35));
        discounts.push_back(DiscountCode("VIP2026", DiscountType::PERCENTAGE, 15.0,
            DateTime(2026, 1, 1, 0, 0, 0), DateTime(2026, 12, 31, 23, 59, 59),
            20000000.0, 1500000.0, 50, 5));
        discounts.push_back(DiscountCode("EXPIRED2025", DiscountType::FIXED_AMOUNT, 200000.0,
            DateTime(2024, 1, 1, 0, 0, 0), DateTime(2025, 1, 1, 23, 59, 59),
            2000000.0, 200000.0, 50, 50));
        return saveToFile();
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath << std::endl;
        return false;
    }

    discounts.clear();
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
                SchemaHelper::DISCOUNTS_HEADER,
                {},
                filePath,
                [](const std::string& l) {
                    if (l.find('|') != std::string::npos) {
                        Vector<std::string> p = TextCodec::splitFields(l, '|');
                        if (p.getSize() >= 9) {
                            std::string t = StringHelper::trim(p[1]);
                            return (t == "0" || t == "1");
                        }
                        return false;
                    }
                    std::istringstream ss(l);
                    std::string c;
                    int tp = -1;
                    double v = 0;
                    return static_cast<bool>(ss >> c >> tp >> v);
                });
            if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                continue;
            }
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

    // Nếu file tồn tại nhưng rỗng hoàn toàn (0 dòng), tạo lại header định dạng chuẩn
    if (lineNum == 0) {
        createEmptyFile();
    }
    // Nếu discounts.getSize() == 0, người dùng đã xóa hết mã: giữ nguyên 0 mã, KHÔNG seed lại!

    return true;
}

// Ghi toàn bộ danh sách discounts xuống file (qua Atomic File Write)
inline bool DiscountRepository::saveToFile() const {
    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        file << SchemaHelper::DISCOUNTS_HEADER << "\n";
        for (int i = 0; i < discounts.getSize(); ++i) {
            file << discounts[i].toFileString() << "\n";
        }
        return file.good();
    });
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
    DateTime now = AppClock::now();
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].isValid(now)) {
            result.push_back(discounts[i]);
        }
    }
    return result;
}

// Thêm mã giảm giá mới (rollback RAM nếu ghi file thất bại)
inline bool DiscountRepository::addDiscount(const DiscountCode& dc) {
    if (existsByCode(dc.getCode())) {
        std::cerr << "[Canh bao] Ma giam gia da ton tai: " << dc.getCode() << std::endl;
        return false;
    }
    Vector<DiscountCode> backup = discounts;
    discounts.push_back(dc);
    if (!saveToFile()) {
        discounts = backup; // Rollback RAM
        return false;
    }
    return true;
}

// Cập nhật mã giảm giá theo code (rollback RAM nếu ghi file thất bại)
inline bool DiscountRepository::updateDiscount(const DiscountCode& updatedDc) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == updatedDc.getCode()) {
            Vector<DiscountCode> backup = discounts;
            discounts[i] = updatedDc;
            if (!saveToFile()) {
                discounts = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ma de cap nhat: " << updatedDc.getCode() << std::endl;
    return false;
}

// Xóa mã giảm giá theo code (rollback RAM nếu ghi file thất bại)
inline bool DiscountRepository::deleteDiscount(const std::string& code) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == code) {
            Vector<DiscountCode> backup = discounts;
            discounts.removeAt(i);
            if (!saveToFile()) {
                discounts = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ma de xoa: " << code << std::endl;
    return false;
}

inline bool DiscountRepository::existsByCode(const std::string& code) const {
    return findByCode(code) != nullptr;
}

#endif // DISCOUNTREPOSITORY_H
