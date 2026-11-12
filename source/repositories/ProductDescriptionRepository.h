#ifndef PRODUCT_DESCRIPTION_REPOSITORY_H
#define PRODUCT_DESCRIPTION_REPOSITORY_H

#include "../utils/FileHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/StringHelper.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct ProductDescriptionRecord {
    std::string productId;
    std::string shortHighlights;
    std::string description;

    static std::string encodeNewlines(const std::string& input) {
        std::string out;
        for (char c : input) {
            if (c == '\n') out += "\\n";
            else if (c == '\r') continue;
            else out += c;
        }
        return out;
    }

    static std::string decodeNewlines(const std::string& input) {
        std::string out;
        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '\\' && i + 1 < input.size() && input[i + 1] == 'n') {
                out += '\n';
                ++i;
            } else {
                out += input[i];
            }
        }
        return out;
    }

    std::string toFileLine() const {
        return productId + "|" + encodeNewlines(shortHighlights) + "|" + encodeNewlines(description);
    }

    static bool fromFileLine(const std::string& line, ProductDescriptionRecord& out) {
        if (line.empty() || line[0] == '#') return false;
        auto tokens = StringHelper::split(line, '|');
        if (tokens.getSize() < 3) return false;

        out.productId = StringHelper::trim(tokens[0]);
        if (out.productId == "productId" || out.productId.empty()) return false;

        out.shortHighlights = decodeNewlines(tokens[1]);
        out.description = decodeNewlines(tokens[2]);
        return true;
    }
};

class ProductDescriptionRepository {
private:
    std::string filePath;
    std::vector<ProductDescriptionRecord> records;

public:
    ProductDescriptionRepository(std::string path = "data/Device/ProductDescriptions.txt")
        : filePath(path) {}

    const std::string& getFilePath() const { return filePath; }

    bool loadFromFile() {
        records.clear();
        std::string resolved = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolved);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            ProductDescriptionRecord rec;
            if (ProductDescriptionRecord::fromFileLine(line, rec)) {
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
            out << "productId|shortHighlights|description\n";
            for (const auto& r : records) {
                out << r.toFileLine() << "\n";
            }
            return true;
        });
    }

    ProductDescriptionRecord getByProductId(const std::string& productId) const {
        for (const auto& r : records) {
            if (r.productId == productId) return r;
        }
        ProductDescriptionRecord def;
        def.productId = productId;
        return def;
    }

    void setByProductId(const ProductDescriptionRecord& record) {
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

#endif // PRODUCT_DESCRIPTION_REPOSITORY_H
