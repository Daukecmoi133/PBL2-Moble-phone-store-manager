#ifndef DISCOUNTREPOSITORY_H
#define DISCOUNTREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/DiscountCode.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu mã giảm giá từ/vào tệp tin
class DiscountRepository {
private:
    std::string filePath;
    Vector<DiscountCode> discounts;

public:
    DiscountRepository(std::string path = "data/PAY/Discount.txt");

    bool loadFromFile();
    bool saveToFile() const;

    const Vector<DiscountCode>& getAll() const;
    Vector<DiscountCode>& getAll();

    DiscountCode* findByCode(const std::string& code);
    const DiscountCode* findByCode(const std::string& code) const;

    bool addDiscount(const DiscountCode& dc);
    bool updateDiscount(const DiscountCode& updatedDc);
    bool deleteDiscount(const std::string& code);
};

// ================= IMPLEMENTATION =================

inline DiscountRepository::DiscountRepository(std::string path) : filePath(path) {}

inline bool DiscountRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    discounts.clear();
    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        line = StringHelper::trim(line);
        if (line.empty() || line.find("SỐ LƯỢNG") != std::string::npos) continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        Vector<std::string> parts = StringHelper::split(line, '|');
        if (parts.getSize() >= 9) {
            std::string code = StringHelper::trim(parts[0]);
            DiscountType type = (StringHelper::trim(parts[1]) == "0") ? DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT;
            double value = StringHelper::parsePrice(parts[2]);
            DateTime start = DateTime::fromDateString(StringHelper::trim(parts[3]));
            DateTime end = DateTime::fromDateString(StringHelper::trim(parts[4]));
            double minOrder = StringHelper::parsePrice(parts[5]);
            double maxDisc = StringHelper::parsePrice(parts[6]);
            int maxUsage = 0;
            int usedCount = 0;
            try {
                maxUsage = std::stoi(StringHelper::trim(parts[7]));
                usedCount = std::stoi(StringHelper::trim(parts[8]));
            } catch (...) {}

            DiscountCode dc(code, type, value, start, end, minOrder, maxDisc, maxUsage, usedCount);
            discounts.push_back(dc);
        }
    }
    file.close();
    return true;
}

inline bool DiscountRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    file << "code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount\n";
    for (int i = 0; i < discounts.getSize(); ++i) {
        file << discounts[i].getCode() << "|"
             << (discounts[i].getType() == DiscountType::PERCENTAGE ? "0" : "1") << "|"
             << discounts[i].getValue() << "|"
             << discounts[i].getStartDate().toDateString() << "|"
             << discounts[i].getEndDate().toDateString() << "|"
             << discounts[i].getMinOrderValue() << "|"
             << discounts[i].getMaxDiscountValue() << "|"
             << discounts[i].getMaxUsage() << "|"
             << discounts[i].getUsedCount() << "\n";
    }
    file.close();
    return true;
}

inline const Vector<DiscountCode>& DiscountRepository::getAll() const { 
    return discounts; 
}

inline Vector<DiscountCode>& DiscountRepository::getAll() { 
    return discounts; 
}

inline DiscountCode* DiscountRepository::findByCode(const std::string& code) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == code) {
            return &discounts[i];
        }
    }
    return nullptr;
}

inline const DiscountCode* DiscountRepository::findByCode(const std::string& code) const {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == code) {
            return &discounts[i];
        }
    }
    return nullptr;
}

inline bool DiscountRepository::addDiscount(const DiscountCode& dc) {
    discounts.push_back(dc);
    return saveToFile();
}

inline bool DiscountRepository::updateDiscount(const DiscountCode& updatedDc) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == updatedDc.getCode()) {
            discounts[i] = updatedDc;
            return saveToFile();
        }
    }
    return false;
}

inline bool DiscountRepository::deleteDiscount(const std::string& code) {
    for (int i = 0; i < discounts.getSize(); ++i) {
        if (discounts[i].getCode() == code) {
            discounts.removeAt(i);
            return saveToFile();
        }
    }
    return false;
}

#endif // DISCOUNTREPOSITORY_H
