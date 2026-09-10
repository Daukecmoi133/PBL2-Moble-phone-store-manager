#ifndef DISCOUNTREPOSITORY_H
#define DISCOUNTREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/DiscountCode.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../utils/StringHelper.h"

class DiscountRepository {
private:
    std::string filePath;
    Vector<DiscountCode> discounts;
    HashTable<std::string, DiscountCode> codeTable;

    void rebuildIndices() {
        codeTable.clear();
        for (int i = 0; i < discounts.getSize(); ++i) {
            codeTable.insert(discounts[i].getCode(), discounts[i]);
        }
    }

public:
    DiscountRepository(std::string path = "data/PAY/Discount.txt") : filePath(path) {}

    bool loadFromFile() {
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

            // Format: code|type(0:%, 1:fixed)|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount
            Vector<std::string> parts = StringHelper::split(line, '|');
            if (parts.getSize() >= 9) {
                std::string code = StringHelper::trim(parts[0]);
                DiscountType type = (StringHelper::trim(parts[1]) == "0") ? DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT;
                double value = StringHelper::parsePrice(parts[2]);
                DateTime start = DateTime::fromDateString(StringHelper::trim(parts[3]));
                DateTime end = DateTime::fromDateString(StringHelper::trim(parts[4]));
                double minOrder = StringHelper::parsePrice(parts[5]);
                double maxDisc = StringHelper::parsePrice(parts[6]);
                int maxUsage = std::stoi(StringHelper::trim(parts[7]));
                int usedCount = std::stoi(StringHelper::trim(parts[8]));

                DiscountCode dc(code, type, value, start, end, minOrder, maxDisc, maxUsage, usedCount);
                discounts.push_back(dc);
            }
        }
        file.close();

        rebuildIndices();
        return true;
    }

    bool saveToFile() const {
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

    const Vector<DiscountCode>& getAll() const { return discounts; }

    DiscountCode* findByCode(const std::string& code) {
        return codeTable.find(code);
    }

    bool addDiscount(const DiscountCode& dc) {
        if (codeTable.contains(dc.getCode())) {
            return false;
        }
        discounts.push_back(dc);
        codeTable.insert(dc.getCode(), dc);
        saveToFile();
        return true;
    }
};

#endif // DISCOUNTREPOSITORY_H
