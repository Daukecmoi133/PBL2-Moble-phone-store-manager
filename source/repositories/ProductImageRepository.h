#ifndef PRODUCT_IMAGE_REPOSITORY_H
#define PRODUCT_IMAGE_REPOSITORY_H

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdio>
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"
#include "../utils/TextCodec.h"
#include "../utils/SchemaHelper.h"

/**
 * @file ProductImageRepository.h
 * @brief Quản lý nạp và lưu trữ ánh xạ ảnh sản phẩm theo chuẩn DATA_FORMAT_GUIDE.md
 * 
 * Schema: productId|imagePath|isPrimary|displayOrder
 * Hỗ trợ tương thích ngược với định dạng legacy (id link).
 */

struct ProductImageRecord {
    std::string productId;
    std::string imagePath; // Đường dẫn tương đối với data/, ví dụ "Device/productImages/oppo.png"
    int isPrimary = 1;
    int displayOrder = 1;

    std::string toFileLine() const {
        return TextCodec::escapeField(productId) + "|" +
               TextCodec::escapeField(imagePath) + "|" +
               std::to_string(isPrimary) + "|" +
               std::to_string(displayOrder);
    }

    static bool fromFileLine(const std::string& line, ProductImageRecord& out) {
        std::string trimmed = StringHelper::trim(line);
        if (trimmed.empty()) return false;

        // Bỏ qua dòng tiêu đề
        if (trimmed.rfind("productId|", 0) == 0 || trimmed == "productId" || trimmed == "id link") {
            return false;
        }

        // Định dạng chuẩn dấu pipe: productId|imagePath|isPrimary|displayOrder
        if (line.find('|') != std::string::npos) {
            Vector<std::string> parts = TextCodec::splitFields(line, '|');
            if (parts.getSize() >= 2) {
                try {
                    out.productId = StringHelper::trim(TextCodec::unescape(parts[0]));
                    out.imagePath = TextCodec::unescape(parts[1]);
                    out.isPrimary = (parts.getSize() >= 3 && !StringHelper::trim(parts[2]).empty())
                                        ? std::stoi(StringHelper::trim(parts[2]))
                                        : 1;
                    out.displayOrder = (parts.getSize() >= 4 && !StringHelper::trim(parts[3]).empty())
                                           ? std::stoi(StringHelper::trim(parts[3]))
                                           : 1;
                    return !out.productId.empty();
                } catch (...) {
                    return false;
                }
            }
        }

        // Định dạng legacy cách nhau khoảng trắng: id link (ví dụ: 101 oppo_reno_16_f_5g_bac.png)
        std::istringstream iss(trimmed);
        std::string id, path;
        if (iss >> id >> path) {
            // Bỏ qua dòng cấu hình thư mục tuyệt đối cũ (ví dụ: 100 C:\Users\...)
            if (id == "100" || path.find(':') != std::string::npos) return false;

            // Chuẩn hóa ID số sang mã chuẩn PHN-000101
            if (id.rfind("PHN-", 0) != 0 && !id.empty() && std::isdigit(static_cast<unsigned char>(id[0]))) {
                try {
                    int num = std::stoi(id);
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "PHN-%06d", num);
                    out.productId = buf;
                } catch (...) {
                    out.productId = id;
                }
            } else {
                out.productId = id;
            }

            // Nếu chỉ là tên file thì thêm tiền tố "Device/productImages/"
            if (path.find('/') == std::string::npos && path.find('\\') == std::string::npos) {
                out.imagePath = "Device/productImages/" + path;
            } else {
                out.imagePath = path;
            }
            out.isPrimary = 1;
            out.displayOrder = 1;
            return true;
        }

        return false;
    }
};

class ProductImageRepository {
private:
    std::string filePath;
    std::vector<ProductImageRecord> records;
    std::unordered_map<std::string, std::vector<size_t>> indexByProductId;

    void rebuildIndex() {
        indexByProductId.clear();
        for (size_t i = 0; i < records.size(); ++i) {
            indexByProductId[records[i].productId].push_back(i);
        }
    }

public:
    ProductImageRepository(std::string path = "data/Device/ProductImage.txt")
        : filePath(path) {}

    const std::string& getFilePath() const { return filePath; }
    std::string getResolvedFilePath() const { return PathHelper::resolveDataPath(filePath); }

    bool loadFromFile() {
        records.clear();
        indexByProductId.clear();
        std::string resolvedPath = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolvedPath);
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
                    SchemaHelper::PRODUCT_IMAGES_HEADER,
                    {SchemaHelper::PRODUCT_IMAGES_V1_HEADER},
                    resolvedPath,
                    [](const std::string& l) {
                        return SchemaHelper::looksLikePrefixedRecord(l, "PHN") ||
                               (!l.empty() && std::isdigit(static_cast<unsigned char>(l[0])));
                    });
                if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                    continue;
                }
            }

            ProductImageRecord rec;
            if (ProductImageRecord::fromFileLine(line, rec)) {
                auto& bucket = indexByProductId[rec.productId];
                bool exists = false;
                for (size_t idx : bucket) {
                    if (records[idx].imagePath == rec.imagePath) {
                        records[idx] = rec;
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    bucket.push_back(records.size());
                    records.push_back(rec);
                }
            }
        }
        return true;
    }

    bool saveToFile() const {
        std::string resolvedPath = PathHelper::resolveDataPath(filePath);
        return FileHelper::atomicWrite(resolvedPath, [&](std::ofstream& out) {
            out << SchemaHelper::PRODUCT_IMAGES_HEADER << "\n";
            for (const auto& r : records) {
                out << r.toFileLine() << "\n";
            }
            return out.good();
        });
    }

    std::vector<ProductImageRecord> getImagesForProduct(const std::string& productId) const {
        std::vector<ProductImageRecord> result;
        auto it = indexByProductId.find(productId);
        if (it != indexByProductId.end()) {
            result.reserve(it->second.size());
            for (size_t idx : it->second) {
                if (idx < records.size()) {
                    result.push_back(records[idx]);
                }
            }
        }
        std::sort(result.begin(), result.end(), [](const ProductImageRecord& a, const ProductImageRecord& b) {
            if (a.isPrimary != b.isPrimary) return a.isPrimary > b.isPrimary;
            return a.displayOrder < b.displayOrder;
        });
        return result;
    }

    void setImagesForProduct(const std::string& productId, const std::vector<ProductImageRecord>& imgs) {
        // Remove existing
        for (auto it = records.begin(); it != records.end();) {
            if (it->productId == productId) {
                it = records.erase(it);
            } else {
                ++it;
            }
        }
        for (const auto& img : imgs) {
            records.push_back(img);
        }
        rebuildIndex();
    }

    std::string getImagePath(const std::string& productId) const {
        auto lookupBucket = [&](const std::string& key) -> std::string {
            auto it = indexByProductId.find(key);
            if (it == indexByProductId.end() || it->second.empty()) return "";
            for (size_t idx : it->second) {
                if (idx < records.size() && records[idx].isPrimary == 1) {
                    return records[idx].imagePath;
                }
            }
            for (size_t idx : it->second) {
                if (idx < records.size()) {
                    return records[idx].imagePath;
                }
            }
            return "";
        };

        std::string found = lookupBucket(productId);
        if (!found.empty()) return found;

        // Hỗ trợ tra cứu tương đương giữa "PHN-000101" và "101"
        if (productId.rfind("PHN-", 0) == 0) {
            try {
                int num = std::stoi(productId.substr(4));
                return lookupBucket(std::to_string(num));
            } catch (...) {}
        }
        return "";
    }

    bool setImagePath(const std::string& productId, const std::string& relativePath) {
        auto it = indexByProductId.find(productId);
        if (it != indexByProductId.end() && !it->second.empty()) {
            size_t idx = it->second.front();
            if (idx < records.size()) {
                records[idx].imagePath = relativePath;
                return true;
            }
        }
        ProductImageRecord rec;
        rec.productId = productId;
        rec.imagePath = relativePath;
        rec.isPrimary = 1;
        rec.displayOrder = 1;
        indexByProductId[productId].push_back(records.size());
        records.push_back(rec);
        return true;
    }

    bool removeImagePath(const std::string& productId) {
        auto it = indexByProductId.find(productId);
        if (it == indexByProductId.end() || it->second.empty()) {
            return false;
        }
        bool removed = false;
        for (auto recIt = records.begin(); recIt != records.end();) {
            if (recIt->productId == productId) {
                recIt = records.erase(recIt);
                removed = true;
            } else {
                ++recIt;
            }
        }
        if (removed) {
            rebuildIndex();
        }
        return removed;
    }

    const std::vector<ProductImageRecord>& getAll() const { return records; }
};

#endif // PRODUCT_IMAGE_REPOSITORY_H

