#ifndef PRODUCT_DESCRIPTION_REPOSITORY_H
#define PRODUCT_DESCRIPTION_REPOSITORY_H

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
        return TextCodec::escapeField(productId) + "|" +
               TextCodec::escapeField(shortHighlights) + "|" +
               TextCodec::escapeField(description);
    }

    static bool fromFileLine(const std::string& line, ProductDescriptionRecord& out) {
        if (line.empty() || line[0] == '#') return false;
        auto tokens = TextCodec::splitFields(line, '|');
        if (tokens.getSize() < 3) return false;

        out.productId = StringHelper::trim(TextCodec::unescape(tokens[0]));
        if (out.productId == "productId" || out.productId.empty()) return false;

        out.shortHighlights = TextCodec::unescape(tokens[1]);
        out.description = TextCodec::unescape(tokens[2]);
        return true;
    }
};

class ProductDescriptionRepository {
private:
    std::string filePath;
    std::vector<ProductDescriptionRecord> records;
    std::unordered_map<std::string, size_t> indexByProductId;

    void rebuildIndex() {
        indexByProductId.clear();
        for (size_t i = 0; i < records.size(); ++i) {
            indexByProductId[records[i].productId] = i;
        }
    }

public:
    ProductDescriptionRepository(std::string path = "data/Device/ProductDescriptions.txt")
        : filePath(path) {}

    const std::string& getFilePath() const { return filePath; }
    std::string getResolvedFilePath() const { return PathHelper::resolveDataPath(filePath); }
    const std::vector<ProductDescriptionRecord>& getAll() const { return records; }

    bool loadFromFile() {
        records.clear();
        indexByProductId.clear();
        std::string resolved = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolved);
        if (!file.is_open()) return false;

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
                    SchemaHelper::PRODUCT_DESCRIPTIONS_HEADER,
                    {},
                    resolved,
                    [](const std::string& l) {
                        return SchemaHelper::looksLikePrefixedRecord(l, "PHN");
                    });
                if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                    continue;
                }
            }

            ProductDescriptionRecord rec;
            if (ProductDescriptionRecord::fromFileLine(line, rec)) {
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
            out << SchemaHelper::PRODUCT_DESCRIPTIONS_HEADER << "\n";
            for (const auto& r : records) {
                out << r.toFileLine() << "\n";
            }
            return out.good();
        });
    }

    const ProductDescriptionRecord* findByProductId(const std::string& productId) const {
        auto it = indexByProductId.find(productId);
        if (it != indexByProductId.end() && it->second < records.size()) {
            return &records[it->second];
        }
        return nullptr;
    }

    ProductDescriptionRecord getByProductId(const std::string& productId) const {
        auto it = indexByProductId.find(productId);
        if (it != indexByProductId.end() && it->second < records.size()) {
            return records[it->second];
        }
        ProductDescriptionRecord def;
        def.productId = productId;
        return def;
    }

    void setByProductId(const ProductDescriptionRecord& record) {
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

#endif // PRODUCT_DESCRIPTION_REPOSITORY_H
