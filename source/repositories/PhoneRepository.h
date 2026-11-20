// PHONE REPOSITORY - Tầng truy xuất dữ liệu sản phẩm điện thoại
#ifndef PHONEREPOSITORY_H
#define PHONEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Phone.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"
#include "../utils/SchemaHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu sản phẩm từ/vào tệp tin.
// Lưu trữ danh sách Phone trên RAM bằng Vector<Phone>.
class PhoneRepository {
private:
    std::string filePath;    // Đường dẫn đến file Products.txt
    std::string specificationsFilePath;
    Vector<Phone> phones;    // Danh sách sản phẩm trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;
    void loadDetailedSpecifications();

public:
    // Hàm dựng: nhận đường dẫn file (mặc định là path chuẩn)
    PhoneRepository(
        std::string path = "data/Device/Products.txt",
        std::string specsPath = "data/Device/Thongsokythuat.txt");

    // Nạp dữ liệu từ file vào RAM. Tự tạo file mới nếu file chưa tồn tại.
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file (ghi đè).
    bool saveToFile() const;
    const std::string& getFilePath() const { return filePath; }
    const std::string& getSpecificationsFilePath() const { return specificationsFilePath; }

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

    // Lưu thông số kỹ thuật chi tiết ra file Thongsokythuat.txt
    bool saveDetailedSpecifications() const;

    // Kiểm tra ID có tồn tại không (tiện cho validation)
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

// Tạo file mới với dòng header chuẩn
inline bool PhoneRepository::createEmptyFile() const {
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        SchemaHelper::PRODUCTS_HEADER
    );
}

inline PhoneRepository::PhoneRepository(std::string path, std::string specsPath)
    : filePath(PathHelper::resolveDataPath(path)),
      specificationsFilePath(PathHelper::resolveDataPath(specsPath)) {}

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
                SchemaHelper::PRODUCTS_HEADER,
                {},
                filePath,
                [](const std::string& l) {
                    return SchemaHelper::looksLikePrefixedRecord(l, "PHN");
                });
            if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                continue;
            }
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
    loadDetailedSpecifications();
    return true;
}

// Ghep moi khoi thong so voi san pham co ID tuong ung.
// Noi dung chi tiet duoc giu trong Phone rieng, khong ghi de cot specifications
// ngan trong Products.txt.
inline void PhoneRepository::loadDetailedSpecifications() {
    std::ifstream file(specificationsFilePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file thong so ky thuat: "
                  << specificationsFilePath << std::endl;
        return;
    }

    Phone* currentPhone = nullptr;
    std::string details;
    std::string line;
    int lineNum = 0;

    auto saveCurrentDetails = [&]() {
        if (currentPhone != nullptr) {
            while (!details.empty() &&
                   (details.back() == '\n' || details.back() == '\r')) {
                details.pop_back();
            }
            currentPhone->setDetailedSpecifications(details);
        }
        details.clear();
    };

    while (std::getline(file, line)) {
        ++lineNum;
        if (lineNum == 1) StringHelper::stripUtf8Bom(line);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const std::string trimmedLine = StringHelper::trim(line);
        if (trimmedLine.empty()) {
            if (currentPhone != nullptr) {
                details += '\n';
            }
            continue;
        }

        const std::size_t separator = trimmedLine.find('|');
        const std::string candidateId =
            StringHelper::trim(trimmedLine.substr(0, separator));
        Phone* matchedPhone = nullptr;

        if (separator != std::string::npos) {
            matchedPhone = findById(candidateId);
        } else if (separator == std::string::npos) {
            matchedPhone = findById(trimmedLine);
        }
        const bool hasIdMarker =
            separator != std::string::npos &&
            StringHelper::trim(trimmedLine.substr(separator + 1)).empty();

        if (hasIdMarker || matchedPhone != nullptr) {
            saveCurrentDetails();
            currentPhone = matchedPhone;
            if (currentPhone == nullptr) {
                std::cerr << "[Canh bao] Khong tim thay san pham ID "
                          << candidateId << " cho thong so tai dong "
                          << lineNum << " trong " << specificationsFilePath
                          << std::endl;
            } else if (separator != std::string::npos && !hasIdMarker) {
                details += trimmedLine.substr(separator + 1);
                details += '\n';
            }
            continue;
        }

        if (currentPhone != nullptr) {
            details += line;
            details += '\n';
        }
    }

    saveCurrentDetails();
}

// Ghi toàn bộ danh sách phones xuống file (ghi đè an toàn qua Atomic File Write)
inline bool PhoneRepository::saveToFile() const {
    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        // Ghi header đúng với định dạng 9 cột
        file << SchemaHelper::PRODUCTS_HEADER << "\n";
        for (int i = 0; i < phones.getSize(); ++i) {
            file << phones[i].toFileString() << "\n";
        }
        return file.good();
    });
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
    // FIX LỖI: Luu snapshot RAM de rollback neu ghi file that bai
    Vector<Phone> backup = phones;
    phones.push_back(phone);
    if (!saveToFile()) {
        phones = backup; // Rollback
        return false;
    }
    if (!phone.getDetailedSpecifications().empty() && !saveDetailedSpecifications()) {
        phones = backup;
        saveToFile();
        return false;
    }
    return true;
}

// Cập nhật thông tin sản phẩm và lưu ngay
inline bool PhoneRepository::updatePhone(const Phone& updatedPhone) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == updatedPhone.getID()) {
            // FIX LỖI: Luu snapshot RAM de rollback neu ghi file that bai
            Vector<Phone> backup = phones;
            Phone replacement = updatedPhone;
            if (replacement.getDetailedSpecifications().empty()) {
                replacement.setDetailedSpecifications(
                    phones[i].getDetailedSpecifications());
            }
            phones[i] = replacement;
            if (!saveToFile()) {
                phones = backup; // Rollback
                return false;
            }
            if (!saveDetailedSpecifications()) {
                phones = backup;
                saveToFile();
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de cap nhat: " << updatedPhone.getID() << std::endl;
    return false;
}

// Lưu thông số kỹ thuật chi tiết ra file Thongsokythuat.txt
inline bool PhoneRepository::saveDetailedSpecifications() const {
    if (specificationsFilePath.empty()) return true;
    return FileHelper::atomicWrite(specificationsFilePath, [&](std::ofstream& file) {
        for (int i = 0; i < phones.getSize(); ++i) {
            const std::string& specs = phones[i].getDetailedSpecifications();
            if (!specs.empty()) {
                file << phones[i].getID() << "|\n";
                file << specs;
                if (specs.back() != '\n') {
                    file << "\n";
                }
            }
        }
        return file.good();
    });
}

// Xóa sản phẩm theo ID và lưu ngay
inline bool PhoneRepository::deletePhone(const std::string& id) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == id) {
            // FIX LỖI: Luu snapshot RAM de rollback neu ghi file that bai
            Vector<Phone> backup = phones;
            bool hadDetails = !phones[i].getDetailedSpecifications().empty();
            phones.removeAt(i);
            if (!saveToFile()) {
                phones = backup; // Rollback
                return false;
            }
            if (hadDetails && !saveDetailedSpecifications()) {
                phones = backup;
                saveToFile();
                return false;
            }
            return true;
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
