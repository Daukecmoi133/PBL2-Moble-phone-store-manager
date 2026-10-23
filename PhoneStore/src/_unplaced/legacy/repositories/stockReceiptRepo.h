#ifndef STOCKRECEIPTREPOSITORY_H
#define STOCKRECEIPTREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/ImportReceipt.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: Chịu trách nhiệm nạp và lưu dữ liệu phiếu nhập kho từ/vào tệp tin
class StockReceiptRepository {
private:
    std::string filePath;
    Vector<ImportReceipt> receipts;

public:
    StockReceiptRepository(std::string path = "data/Device/StockReceipts.txt");

    bool loadFromFile();
    bool saveToFile() const;

    const Vector<ImportReceipt>& getAll() const;
    Vector<ImportReceipt>& getAll();

    ImportReceipt* findById(const std::string& receiptId);
    const ImportReceipt* findById(const std::string& receiptId) const;

    bool addReceipt(const ImportReceipt& receipt);
    bool updateReceipt(const ImportReceipt& updatedReceipt);
    bool deleteReceipt(const std::string& id);

    bool containsPhone(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

inline StockReceiptRepository::StockReceiptRepository(std::string path) : filePath(path) {}

inline bool StockReceiptRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    receipts.clear();
    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        line = StringHelper::trim(line);
        if (line.empty() || line == "...") continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        Vector<std::string> parts = StringHelper::split(line, '|');
        if (parts.getSize() >= 4) {
            std::string id = StringHelper::trim(parts[0]);
            std::string supplierId = StringHelper::trim(parts[1]);
            std::string employeeId = StringHelper::trim(parts[2]);
            DateTime importTime = DateTime::fromString(StringHelper::trim(parts[3]));

            ImportReceipt receipt(id, supplierId, employeeId);
            receipt.setImportTime(importTime);

            if (parts.getSize() >= 5) {
                std::string detailsStr = StringHelper::trim(parts[4]);
                Vector<std::string> itemTokens = StringHelper::split(detailsStr, ';');
                for (int i = 0; i < itemTokens.getSize(); ++i) {
                    Vector<std::string> d = StringHelper::split(itemTokens[i], ',');
                    if (d.getSize() >= 4) {
                        std::string pId = StringHelper::trim(d[0]);
                        std::string pName = StringHelper::trim(d[1]);
                        int qty = 1;
                        double price = 0.0;
                        try {
                            qty = std::stoi(StringHelper::trim(d[2]));
                            price = StringHelper::parsePrice(d[3]);
                        } catch (...) {}
                        receipt.addDetail(OrderDetail(pId, pName, qty, price));
                    }
                }
            }
            receipts.push_back(receipt);
        }
    }
    file.close();
    return true;
}

inline bool StockReceiptRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "id|supplierId|employeeId|importTime|details\n";
    for (int i = 0; i < receipts.getSize(); ++i) {
        file << receipts[i].toFileString() << "\n";
    }
    file.close();
    return true;
}

inline const Vector<ImportReceipt>& StockReceiptRepository::getAll() const {
    return receipts;
}

inline Vector<ImportReceipt>& StockReceiptRepository::getAll() {
    return receipts;
}

inline ImportReceipt* StockReceiptRepository::findById(const std::string& receiptId) {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == receiptId) {
            return &receipts[i];
        }
    }
    return nullptr;
}

inline const ImportReceipt* StockReceiptRepository::findById(const std::string& receiptId) const {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == receiptId) {
            return &receipts[i];
        }
    }
    return nullptr;
}

inline bool StockReceiptRepository::addReceipt(const ImportReceipt& receipt) {
    receipts.push_back(receipt);
    return saveToFile();
}

inline bool StockReceiptRepository::updateReceipt(const ImportReceipt& updatedReceipt) {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == updatedReceipt.getID()) {
            receipts[i] = updatedReceipt;
            return saveToFile();
        }
    }
    return false;
}

inline bool StockReceiptRepository::deleteReceipt(const std::string& id) {
    for (int i = 0; i < receipts.getSize(); ++i) {
        if (receipts[i].getID() == id) {
            receipts.removeAt(i);
            return saveToFile();
        }
    }
    return false;
}

inline bool StockReceiptRepository::containsPhone(const std::string& id) const {
    for (int i = 0; i < receipts.getSize(); ++i) {
        const Vector<OrderDetail>& details = receipts[i].getDetails();
        for (int j = 0; j < details.getSize(); ++j) {
            if (details[j].getPhoneId() == id) {
                return true;
            }
        }
    }
    return false;
}

#endif // STOCKRECEIPTREPOSITORY_H