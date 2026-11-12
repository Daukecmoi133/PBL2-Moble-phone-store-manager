#ifndef PRODUCT_MARKETING_REPOSITORY_H
#define PRODUCT_MARKETING_REPOSITORY_H

#include "../utils/FileHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/StringHelper.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct ProductMarketingRecord {
    std::string productId;
    double listPrice = 0.0;
    std::string saleStart;
    std::string saleEnd;
    bool installmentEnabled = false;
    std::string installmentNote;
    std::vector<std::string> colors;
    int warrantyMonths = 12;
    int returnDays = 30;

    std::string toFileLine() const {
        std::ostringstream ss;
        // Escape colors as comma-separated
        std::string colorStr;
        for (size_t i = 0; i < colors.size(); ++i) {
            if (i > 0) colorStr += ",";
            colorStr += colors[i];
        }
        ss << productId << "|"
           << static_cast<long long>(listPrice) << "|"
           << saleStart << "|"
           << saleEnd << "|"
           << (installmentEnabled ? "1" : "0") << "|"
           << installmentNote << "|"
           << colorStr << "|"
           << warrantyMonths << "|"
           << returnDays;
        return ss.str();
    }

    static bool fromFileLine(const std::string& line, ProductMarketingRecord& out) {
        if (line.empty() || line[0] == '#') return false;
        auto tokens = StringHelper::split(line, '|');
        if (tokens.getSize() < 9) return false;

        out.productId = StringHelper::trim(tokens[0]);
        if (out.productId == "productId" || out.productId.empty()) return false;

        try {
            out.listPrice = std::stod(StringHelper::trim(tokens[1]));
        } catch (...) {
            out.listPrice = 0.0;
        }

        out.saleStart = StringHelper::trim(tokens[2]);
        out.saleEnd = StringHelper::trim(tokens[3]);
        out.installmentEnabled = (StringHelper::trim(tokens[4]) == "1");
        out.installmentNote = StringHelper::trim(tokens[5]);

        out.colors.clear();
        std::string rawColors = StringHelper::trim(tokens[6]);
        if (!rawColors.empty()) {
            auto cTokens = StringHelper::split(rawColors, ',');
            for (int i = 0; i < cTokens.getSize(); ++i) {
                std::string tc = StringHelper::trim(cTokens[i]);
                if (!tc.empty()) out.colors.push_back(tc);
            }
        }

        try {
            out.warrantyMonths = std::stoi(StringHelper::trim(tokens[7]));
        } catch (...) {
            out.warrantyMonths = 12;
        }

        try {
            out.returnDays = std::stoi(StringHelper::trim(tokens[8]));
        } catch (...) {
            out.returnDays = 30;
        }

        return true;
    }
};

class ProductMarketingRepository {
private:
    std::string filePath;
    std::vector<ProductMarketingRecord> records;

public:
    ProductMarketingRepository(std::string path = "data/Device/ProductMarketing.txt")
        : filePath(path) {}

    const std::string& getFilePath() const { return filePath; }

    bool loadFromFile() {
        records.clear();
        std::string resolved = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolved);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            ProductMarketingRecord rec;
            if (ProductMarketingRecord::fromFileLine(line, rec)) {
                bool exists = false;
                for (auto& r : records) {
                    if (r.productId == rec.productId) {
                        r = rec;
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    records.push_back(rec);
                }
            }
        }
        return true;
    }

    bool saveToFile() const {
        std::string resolved = PathHelper::resolveDataPath(filePath);
        return FileHelper::atomicWrite(resolved, [&](std::ofstream& out) {
            out << "productId|listPrice|saleStart|saleEnd|installmentEnabled|installmentNote|colors|warrantyMonths|returnDays\n";
            for (const auto& r : records) {
                out << r.toFileLine() << "\n";
            }
            return true;
        });
    }

    ProductMarketingRecord getByProductId(const std::string& productId) const {
        for (const auto& r : records) {
            if (r.productId == productId) return r;
        }
        ProductMarketingRecord def;
        def.productId = productId;
        return def;
    }

    void setByProductId(const ProductMarketingRecord& record) {
        for (auto& r : records) {
            if (r.productId == record.productId) {
                r = record;
                return;
            }
        }
        records.push_back(record);
    }

    void removeByProductId(const std::string& productId) {
        for (auto it = records.begin(); it != records.end(); ++it) {
            if (it->productId == productId) {
                records.erase(it);
                return;
            }
        }
    }
};

#endif // PRODUCT_MARKETING_REPOSITORY_H
