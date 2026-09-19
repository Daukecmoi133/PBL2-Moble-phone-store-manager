#ifndef PHONEREPOSITORY_H
#define PHONEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Phone.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu sản phẩm từ/vào tệp tin
class PhoneRepository {
private:
    std::string filePath;
    Vector<Phone> phones;

public:
    PhoneRepository(std::string path = "data/Device/Products.txt");

    bool loadFromFile();
    bool saveToFile() const;

    const Vector<Phone>& getAll() const;
    Vector<Phone>& getAll();

    Phone* findById(const std::string& id);
    const Phone* findById(const std::string& id) const;

    bool addPhone(const Phone& phone);
    bool updatePhone(const Phone& updatedPhone);
    bool deletePhone(const std::string& id);
};

// ================= IMPLEMENTATION =================

inline PhoneRepository::PhoneRepository(std::string path) : filePath(path) {}

inline bool PhoneRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Cảnh báo] Không thể mở file: " << filePath << std::endl;
        return false;
    }

    phones.clear();
    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        line = StringHelper::trim(line);
        if (line.empty() || line == "...") continue;

        if (isHeader) {
            isHeader = false; // Bỏ qua dòng tiêu đề
            continue;
        }

        Vector<std::string> parts = StringHelper::split(line, '|');
        if (parts.getSize() >= 7) {
            std::string id = StringHelper::trim(parts[0]);
            std::string name = StringHelper::trim(parts[1]);
            std::string brand = StringHelper::trim(parts[2]);
            std::string os = StringHelper::trim(parts[3]);
            double importPrice = StringHelper::parsePrice(parts[4]);
            double sellingPrice = StringHelper::parsePrice(parts[5]);
            int quantity = 0;
            try {
                quantity = std::stoi(StringHelper::trim(parts[6]));
            } catch (...) {
                quantity = 0;
            }

            Phone p(id, name, brand, os, importPrice, sellingPrice, quantity);
            phones.push_back(p);
        }
    }
    file.close();
    return true;
}

inline bool PhoneRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Lỗi] Không thể ghi file: " << filePath << std::endl;
        return false;
    }

    // Ghi dòng tiêu đề
    file << "id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity\n";
    for (int i = 0; i < phones.getSize(); ++i) {
        file << phones[i].toFileString() << "\n";
    }
    file.close();
    return true;
}

inline const Vector<Phone>& PhoneRepository::getAll() const {
    return phones;
}

inline Vector<Phone>& PhoneRepository::getAll() {
    return phones;
}

inline Phone* PhoneRepository::findById(const std::string& id) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == id) {
            return &phones[i];
        }
    }
    return nullptr;
}

inline const Phone* PhoneRepository::findById(const std::string& id) const {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == id) {
            return &phones[i];
        }
    }
    return nullptr;
}

inline bool PhoneRepository::addPhone(const Phone& phone) {
    phones.push_back(phone);
    return saveToFile();
}

inline bool PhoneRepository::updatePhone(const Phone& updatedPhone) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == updatedPhone.getID()) {
            phones[i] = updatedPhone;
            return saveToFile();
        }
    }
    return false;
}

inline bool PhoneRepository::deletePhone(const std::string& id) {
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getID() == id) {
            phones.removeAt(i);
            return saveToFile();
        }
    }
    return false;
}

#endif // PHONEREPOSITORY_H
