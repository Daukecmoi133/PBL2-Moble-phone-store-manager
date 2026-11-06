#ifndef PRODUCT_IMAGE_REPOSITORY_H
#define PRODUCT_IMAGE_REPOSITORY_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdio>
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"

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
        return productId + "|" + imagePath + "|" + std::to_string(isPrimary) + "|" + std::to_string(displayOrder);
    }

    static bool fromFileLine(const std::string& line, ProductImageRecord& out) {
        std::string trimmed = StringHelper::trim(line);
        if (trimmed.empty()) return false;

        // Bỏ qua dòng tiêu đề
        if (trimmed.rfind("productId", 0) == 0 || trimmed.rfind("id link", 0) == 0) {
            return false;
        }

        // Định dạng chuẩn dấu pipe: productId|imagePath|isPrimary|displayOrder
        if (trimmed.find('|') != std::string::npos) {
            Vector<std::string> parts = StringHelper::split(trimmed, '|');
            if (parts.getSize() >= 2) {
                out.productId = StringHelper::trim(parts[0]);
                out.imagePath = StringHelper::trim(parts[1]);
                out.isPrimary = (parts.getSize() >= 3) ? std::stoi(StringHelper::trim(parts[2])) : 1;
                out.displayOrder = (parts.getSize() >= 4) ? std::stoi(StringHelper::trim(parts[3])) : 1;
                return true;
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

public:
    ProductImageRepository(std::string path = "data/Device/ProductImage.txt")
        : filePath(path) {}

    const std::string& getFilePath() const { return filePath; }

    bool loadFromFile() {
        records.clear();
        std::string resolvedPath = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolvedPath);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            ProductImageRecord rec;
            if (ProductImageRecord::fromFileLine(line, rec)) {
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
        std::string resolvedPath = PathHelper::resolveDataPath(filePath);
        return FileHelper::atomicWrite(resolvedPath, [&](std::ofstream& out) {
            out << "productId|imagePath|isPrimary|displayOrder\n";
            for (const auto& r : records) {
                out << r.toFileLine() << "\n";
            }
            return true;
        });
    }

    std::string getImagePath(const std::string& productId) const {
        for (const auto& r : records) {
            if (r.productId == productId) return r.imagePath;
            // Hỗ trợ tra cứu tương đương giữa "PHN-000101" và "101"
            if (productId.rfind("PHN-", 0) == 0) {
                try {
                    int num = std::stoi(productId.substr(4));
                    if (r.productId == std::to_string(num)) return r.imagePath;
                } catch (...) {}
            }
        }
        return "";
    }

    bool setImagePath(const std::string& productId, const std::string& relativePath) {
        for (auto& r : records) {
            if (r.productId == productId) {
                r.imagePath = relativePath;
                return true;
            }
        }
        ProductImageRecord rec;
        rec.productId = productId;
        rec.imagePath = relativePath;
        rec.isPrimary = 1;
        rec.displayOrder = 1;
        records.push_back(rec);
        return true;
    }

    bool removeImagePath(const std::string& productId) {
        for (auto it = records.begin(); it != records.end(); ++it) {
            if (it->productId == productId) {
                records.erase(it);
                return true;
            }
        }
        return false;
    }

    const std::vector<ProductImageRecord>& getAll() const { return records; }
};

#endif // PRODUCT_IMAGE_REPOSITORY_H

