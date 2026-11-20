#ifndef PRODUCT_MARKETING_REPOSITORY_H
#define PRODUCT_MARKETING_REPOSITORY_H

#include "../utils/FileHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/StringHelper.h"
#include "../utils/TextCodec.h"
#include "../utils/SchemaHelper.h"
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
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
        std::string colorStr;
        for (size_t i = 0; i < colors.size(); ++i) {
            if (i > 0) colorStr += ",";
            colorStr += TextCodec::escapeSubField(colors[i]);
        }
        ss << TextCodec::escapeField(productId) << "|"
           << StringHelper::formatMoneyDisk(listPrice) << "|"
           << TextCodec::escapeField(saleStart) << "|"
           << TextCodec::escapeField(saleEnd) << "|"
           << (installmentEnabled ? "1" : "0") << "|"
           << TextCodec::escapeField(installmentNote) << "|"
           << colorStr << "|"
           << warrantyMonths << "|"
           << returnDays;
        return ss.str();
    }

    static bool fromFileLine(const std::string& line, ProductMarketingRecord& out) {
        if (line.empty() || line[0] == '#') return false;
        auto tokens = TextCodec::splitFields(line, '|');
        if (tokens.getSize() < 9) return false;

        out.productId = StringHelper::trim(TextCodec::unescape(tokens[0]));
        if (out.productId == "productId" || out.productId.empty()) return false;

        out.listPrice = StringHelper::parsePrice(tokens[1]);
        out.saleStart = StringHelper::trim(TextCodec::unescape(tokens[2]));
        out.saleEnd = StringHelper::trim(TextCodec::unescape(tokens[3]));
        out.installmentEnabled = (StringHelper::trim(tokens[4]) == "1");
        out.installmentNote = TextCodec::unescape(tokens[5]);

        out.colors.clear();
        const std::string& rawColors = tokens[6];
        if (!rawColors.empty()) {
            auto cTokens = TextCodec::splitFields(rawColors, ',');
            for (int i = 0; i < cTokens.getSize(); ++i) {
                std::string tc = TextCodec::unescape(cTokens[i]);
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
    std::unordered_map<std::string, size_t> indexByProductId;

    void rebuildIndex() {
        indexByProductId.clear();
        for (size_t i = 0; i < records.size(); ++i) {
            indexByProductId[records[i].productId] = i;
        }
    }

public:
    ProductMarketingRepository(std::string path = "data/Device/ProductMarketing.txt")
        : filePath(path) {}

    const std::string& getFilePath() const { return filePath; }
    std::string getResolvedFilePath() const { return PathHelper::resolveDataPath(filePath); }
    const std::vector<ProductMarketingRecord>& getAll() const { return records; }

    bool loadFromFile() {
        records.clear();
        indexByProductId.clear();
        std::string resolved = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolved);
        if (!file.is_open()) {
            return false;
        }

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
                    SchemaHelper::PRODUCT_MARKETING_HEADER,
                    {},
                    resolved,
                    [](const std::string& l) {
                        return SchemaHelper::looksLikePrefixedRecord(l, "PHN");
                    });
                if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                    continue;
                }
            }

            ProductMarketingRecord rec;
            if (ProductMarketingRecord::fromFileLine(line, rec)) {
                auto it = indexByProductId.find(rec.productId);
                if (it != indexByProductId.end()) {
                    records[it->second] = rec;
                } else {
                    indexByProductId[rec.productId] = records.size();
                    records.push_back(rec);
                }
            }
        }
        return true;
    }

    bool saveToFile() const {
        std::string resolved = PathHelper::resolveDataPath(filePath);
        return FileHelper::atomicWrite(resolved, [&](std::ofstream& out) {
            out << SchemaHelper::PRODUCT_MARKETING_HEADER << "\n";
            for (const auto& r : records) {
                out << r.toFileLine() << "\n";
            }
            return out.good();
        });
    }

    const ProductMarketingRecord* findByProductId(const std::string& productId) const {
        auto it = indexByProductId.find(productId);
        if (it != indexByProductId.end() && it->second < records.size()) {
            return &records[it->second];
        }
        return nullptr;
    }

    ProductMarketingRecord getByProductId(const std::string& productId) const {
        auto it = indexByProductId.find(productId);
        if (it != indexByProductId.end() && it->second < records.size()) {
            return records[it->second];
        }
        ProductMarketingRecord def;
        def.productId = productId;
        return def;
    }

    void setByProductId(const ProductMarketingRecord& record) {
        auto it = indexByProductId.find(record.productId);
        if (it != indexByProductId.end() && it->second < records.size()) {
            records[it->second] = record;
            return;
        }
        indexByProductId[record.productId] = records.size();
        records.push_back(record);
    }

    void removeByProductId(const std::string& productId) {
        auto it = indexByProductId.find(productId);
        if (it == indexByProductId.end() || it->second >= records.size()) {
            return;
        }
        records.erase(records.begin() + static_cast<std::ptrdiff_t>(it->second));
        rebuildIndex();
    }
};

#endif // PRODUCT_MARKETING_REPOSITORY_H
