#ifndef PHONEREPOSITORY_H
#define PHONEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Phone.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../core/BST.h"
#include "../utils/StringHelper.h"

class PhoneRepository {
private:
    std::string filePath;
    Vector<Phone> phones;
    HashTable<std::string, Phone> idTable; // Bảng băm để tìm kiếm theo ID với O(1)
    BST<double, Phone> priceTree;          // Cây BST để tìm kiếm theo khoảng giá O(log N)

    void rebuildIndices() {
        idTable.clear();
        priceTree.clear();
        for (int i = 0; i < phones.getSize(); ++i) {
            idTable.insert(phones[i].getID(), phones[i]);
            priceTree.insert(phones[i].getSellingPrice(), phones[i]);
        }
    }

public:
    PhoneRepository(std::string path = "data/Device/Products.txt") : filePath(path) {}

    bool loadFromFile() {
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
                isHeader = false; // Bỏ qua dòng tiêu đề: id|phoneName|...
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

        rebuildIndices();
        return true;
    }

    bool saveToFile() const {
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

    const Vector<Phone>& getAll() const {
        return phones;
    }

    Vector<Phone>& getAll() {
        return phones;
    }

    Phone* findById(const std::string& id) {
        return idTable.find(id);
    }

    Vector<Phone> findByName(const std::string& keyword) const {
        Vector<Phone> results;
        for (int i = 0; i < phones.getSize(); ++i) {
            if (StringHelper::containsIgnoreCase(phones[i].getName(), keyword)) {
                results.push_back(phones[i]);
            }
        }
        return results;
    }

    Vector<Phone> findByBrand(const std::string& brand) const {
        Vector<Phone> results;
        for (int i = 0; i < phones.getSize(); ++i) {
            if (StringHelper::containsIgnoreCase(phones[i].getBrand(), brand)) {
                results.push_back(phones[i]);
            }
        }
        return results;
    }

    Vector<Phone> findByPriceRange(double minPrice, double maxPrice) const {
        return priceTree.findRange(minPrice, maxPrice);
    }

    bool addPhone(const Phone& phone) {
        if (idTable.contains(phone.getID())) {
            return false; // ID đã tồn tại
        }
        phones.push_back(phone);
        idTable.insert(phone.getID(), phone);
        priceTree.insert(phone.getSellingPrice(), phone);
        saveToFile();
        return true;
    }

    bool updatePhone(const Phone& updatedPhone) {
        for (int i = 0; i < phones.getSize(); ++i) {
            if (phones[i].getID() == updatedPhone.getID()) {
                phones[i] = updatedPhone;
                rebuildIndices();
                saveToFile();
                return true;
            }
        }
        return false;
    }

    bool deletePhone(const std::string& id) {
        for (int i = 0; i < phones.getSize(); ++i) {
            if (phones[i].getID() == id) {
                phones.removeAt(i);
                rebuildIndices();
                saveToFile();
                return true;
            }
        }
        return false;
    }
};

#endif // PHONEREPOSITORY_H
