#ifndef WARRANTYREPOSITORY_H
#define WARRANTYREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Warranty.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"

// Tầng Repository: Quản lý nạp và lưu trữ hồ sơ bảo hành (Warranties.txt)
class WarrantyRepository {
private:
    std::string filePath;
    Vector<Warranty> warranties;

    bool createEmptyFile() const;

public:
    WarrantyRepository(std::string path = "data/warranty/Warranties.txt");

    bool loadFromFile();
    bool saveToFile() const;
    const std::string& getFilePath() const { return filePath; }

    const Vector<Warranty>& getAll() const;
    Vector<Warranty>& getAll();

    Warranty* findById(const std::string& id);
    const Warranty* findById(const std::string& id) const;

    Warranty* findByImei(const std::string& imei);
    const Warranty* findByImei(const std::string& imei) const;

    Vector<Warranty> findByCustomerId(const std::string& customerId) const;
    Vector<Warranty> findByInvoiceId(const std::string& invoiceId) const;

    bool existsById(const std::string& id) const;

    bool addWarranty(const Warranty& warranty);
    bool updateWarranty(const Warranty& updatedWarranty);
    bool deleteWarranty(const std::string& id);
};

// ================= IMPLEMENTATION =================

inline bool WarrantyRepository::createEmptyFile() const {
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        "id|phoneId|phoneName|imei|customerId|invoiceId|startDate|endDate|status"
    );
}

inline WarrantyRepository::WarrantyRepository(std::string path)
    : filePath(PathHelper::resolveDataPath(path)) {}

inline bool WarrantyRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    warranties.clear();
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
            Warranty w = Warranty::fromFileLine(line);
            if (!w.getID().empty()) {
                warranties.push_back(w);
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

inline bool WarrantyRepository::saveToFile() const {
    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        file << "id|phoneId|phoneName|imei|customerId|invoiceId|startDate|endDate|status\n";
        for (int i = 0; i < warranties.getSize(); ++i) {
            file << warranties[i].toFileString() << "\n";
        }
        return file.good();
    });
}

inline const Vector<Warranty>& WarrantyRepository::getAll() const { return warranties; }
inline Vector<Warranty>& WarrantyRepository::getAll() { return warranties; }

inline Warranty* WarrantyRepository::findById(const std::string& id) {
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getID() == id) return &warranties[i];
    }
    return nullptr;
}

inline const Warranty* WarrantyRepository::findById(const std::string& id) const {
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getID() == id) return &warranties[i];
    }
    return nullptr;
}

inline Warranty* WarrantyRepository::findByImei(const std::string& imei) {
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getImei() == imei) return &warranties[i];
    }
    return nullptr;
}

inline const Warranty* WarrantyRepository::findByImei(const std::string& imei) const {
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getImei() == imei) return &warranties[i];
    }
    return nullptr;
}

inline Vector<Warranty> WarrantyRepository::findByCustomerId(const std::string& customerId) const {
    Vector<Warranty> result;
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getCustomerId() == customerId) {
            result.push_back(warranties[i]);
        }
    }
    return result;
}

inline Vector<Warranty> WarrantyRepository::findByInvoiceId(const std::string& invoiceId) const {
    Vector<Warranty> result;
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getInvoiceId() == invoiceId) {
            result.push_back(warranties[i]);
        }
    }
    return result;
}

inline bool WarrantyRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

inline bool WarrantyRepository::addWarranty(const Warranty& warranty) {
    if (existsById(warranty.getID())) {
        std::cerr << "[Canh bao] ID ho so bao hanh da ton tai: " << warranty.getID() << std::endl;
        return false;
    }
    Vector<Warranty> backup = warranties;
    warranties.push_back(warranty);
    if (!saveToFile()) {
        warranties = backup; // Rollback RAM neu ghi file that bai
        return false;
    }
    return true;
}

inline bool WarrantyRepository::updateWarranty(const Warranty& updatedWarranty) {
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getID() == updatedWarranty.getID()) {
            Vector<Warranty> backup = warranties;
            warranties[i] = updatedWarranty;
            if (!saveToFile()) {
                warranties = backup; // Rollback RAM neu ghi file that bai
                return false;
            }
            return true;
        }
    }
    return false;
}

inline bool WarrantyRepository::deleteWarranty(const std::string& id) {
    for (int i = 0; i < warranties.getSize(); ++i) {
        if (warranties[i].getID() == id) {
            Vector<Warranty> backup = warranties;
            warranties.removeAt(i);
            if (!saveToFile()) {
                warranties = backup; // Rollback RAM neu ghi file that bai
                return false;
            }
            return true;
        }
    }
    return false;
}

#endif // WARRANTYREPOSITORY_H
