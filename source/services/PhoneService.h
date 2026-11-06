#ifndef PHONESERVICE_H
#define PHONESERVICE_H

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include "../models/Phone.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../core/BST.h"
#include "../repositories/PhoneRepository.h"
#include "../repositories/ProductImageRepository.h"
#include "../utils/IdGenerator.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"
#include "Check.h"

// FIX LỖI: Key tổ hợp (Composite Key) cho BST để hỗ trợ nhiều sản phẩm có cùng giá bán
struct PhonePriceKey {
    double price;
    std::string id;

    bool operator<(const PhonePriceKey& other) const {
        if (price != other.price) return price < other.price;
        return id < other.id;
    }
    bool operator>(const PhonePriceKey& other) const {
        if (price != other.price) return price > other.price;
        return id > other.id;
    }
    bool operator<=(const PhonePriceKey& other) const { return !(other < *this); }
    bool operator>=(const PhonePriceKey& other) const { return !(*this < other); }
    bool operator==(const PhonePriceKey& other) const {
        return price == other.price && id == other.id;
    }
    bool operator!=(const PhonePriceKey& other) const { return !(*this == other); }
};

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ xử lý dữ liệu sản phẩm,
// cài đặt và vận hành các cấu trúc dữ liệu giải thuật (HashTable O(1), BST O(log N)).
class PhoneService {
private:
    PhoneRepository& repo;
    HashTable<std::string, Phone> idTable;        // Bảng băm để tìm kiếm theo ID với O(1)
    BST<PhonePriceKey, Phone> priceTree;          // FIX LỖI: Cây BST dùng PhonePriceKey để không mất sản phẩm trùng giá
    ReferenceChecker* refChecker;                 // FIX LỖI: Ràng buộc toàn vẹn tham chiếu trước khi xóa
    ProductImageRepository* imgRepo;              // Quản lý ánh xạ ảnh sản phẩm (BUG-HIGH-05)

public:
    PhoneService(PhoneRepository& repository, ReferenceChecker* checker = nullptr, ProductImageRepository* imageRepository = nullptr);

    void setReferenceChecker(ReferenceChecker* checker);
    void setProductImageRepository(ProductImageRepository* imageRepository) { imgRepo = imageRepository; }
    ProductImageRepository* getProductImageRepository() { return imgRepo; }
    const ProductImageRepository* getProductImageRepository() const { return imgRepo; }

    // Tái cấu trúc các chỉ mục DSA (HashTable, BST) từ dữ liệu kho
    void rebuildIndices();
    void reload() { repo.loadFromFile(); rebuildIndices(); }
    PhoneRepository& getRepository() { return repo; }
    const PhoneRepository& getRepository() const { return repo; }

    const Vector<Phone>& getAllPhones() const;

    // Tra cứu O(1) qua bảng băm
    Phone* getPhoneById(const std::string& id);

    // Lọc theo khoảng giá O(log N) qua cây nhị phân BST
    Vector<Phone> filterByPriceRange(double minPrice, double maxPrice) const;

    // Tìm kiếm theo tên sản phẩm (không phân biệt hoa thường)
    Vector<Phone> searchByName(const std::string& keyword) const;

    // Tìm kiếm theo thương hiệu
    Vector<Phone> searchByBrand(const std::string& brand) const;

    // Quản lý ảnh sản phẩm (BUG-HIGH-05)
    std::string getProductImagePath(const std::string& productId) const;
    bool updateProductImage(const std::string& productId, const std::string& sourceFilePath, std::string& errorMessage);

    // Nghiệp vụ thêm sản phẩm: Kiểm tra trùng ID, kiểm tra giá > 0, số lượng >= 0
    bool addPhone(const Phone& phone, std::string& errorMessage);
    bool addPhone(const Phone& phone);

    // Nghiệp vụ cập nhật sản phẩm
    bool updatePhone(const Phone& phone, std::string& errorMessage);
    bool updatePhone(const Phone& phone);

    // Nghiệp vụ xóa sản phẩm
    bool deletePhone(const std::string& id, std::string& errorMessage);
    bool deletePhone(const std::string& id);

    // Nghiệp vụ kiểm tra tồn kho
    bool hasStock(const std::string& id, int requestedQuantity);

    // Nghiệp vụ thay đổi số lượng tồn kho (nhập thêm hoặc bán ra)
    bool updateStock(const std::string& id, int quantityDelta, std::string& errorMessage);
    bool updateStock(const std::string& id, int quantityDelta);
};

// ================= IMPLEMENTATION =================

inline PhoneService::PhoneService(PhoneRepository& repository, ReferenceChecker* checker, ProductImageRepository* imageRepository) 
    : repo(repository), refChecker(checker), imgRepo(imageRepository) {
    rebuildIndices();
}

inline void PhoneService::setReferenceChecker(ReferenceChecker* checker) {
    refChecker = checker;
}

inline std::string PhoneService::getProductImagePath(const std::string& productId) const {
    if (imgRepo) {
        return imgRepo->getImagePath(productId);
    }
    return "";
}

inline bool PhoneService::updateProductImage(const std::string& productId, const std::string& sourceFilePath, std::string& errorMessage) {
    if (imgRepo == nullptr) {
        errorMessage = "Hệ thống quản lý ảnh sản phẩm chưa sẵn sàng!";
        return false;
    }
    if (productId.empty()) {
        errorMessage = "Mã sản phẩm không được để trống!";
        return false;
    }
    if (sourceFilePath.empty()) {
        errorMessage = "Đường dẫn file ảnh nguồn không được để trống!";
        return false;
    }

    std::string trimmedSrc = StringHelper::trim(sourceFilePath);

    // 1. Nếu file đã là ảnh hiện tại của sản phẩm thì không cần làm lại
    std::string currentRel = imgRepo->getImagePath(productId);
    if (!currentRel.empty() && (trimmedSrc == currentRel || trimmedSrc == ("data/" + currentRel))) {
        return true;
    }

    // 2. Xác định đường dẫn file nguồn vật lý
    std::string actualSrcPath = trimmedSrc;
    std::error_code ec;
    std::filesystem::path srcP(trimmedSrc);
    if (!std::filesystem::exists(srcP, ec)) {
        std::string resolved = PathHelper::resolveDataPath(trimmedSrc);
        if (std::filesystem::exists(std::filesystem::path(resolved), ec)) {
            actualSrcPath = resolved;
            srcP = std::filesystem::path(resolved);
        } else {
            errorMessage = "File ảnh nguồn không tồn tại: " + trimmedSrc;
            return false;
        }
    }

    if (!std::filesystem::is_regular_file(srcP, ec)) {
        errorMessage = "Đường dẫn nguồn không phải là file hợp lệ: " + trimmedSrc;
        return false;
    }

    // 3. Kiểm tra định dạng phần mở rộng (.png, .jpg, .jpeg, .webp)
    std::string ext = srcP.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".webp") {
        errorMessage = "Định dạng file không được hỗ trợ (" + ext + ")! Chỉ chấp nhận .png, .jpg, .jpeg, .webp.";
        return false;
    }

    // 4. Kiểm tra file đọc được và kích thước > 0
    {
        std::ifstream in(actualSrcPath, std::ios::binary);
        if (!in.is_open()) {
            errorMessage = "Không thể mở file ảnh nguồn để đọc: " + actualSrcPath;
            return false;
        }
    }
    auto sz = std::filesystem::file_size(srcP, ec);
    if (ec || sz == 0) {
        errorMessage = "File ảnh rỗng hoặc không có dữ liệu!";
        return false;
    }

    // 5. Chuẩn bị thư mục đích data/Device/productImages/
    std::string targetDir = PathHelper::resolveDataPath("data/Device/productImages");
    std::filesystem::create_directories(targetDir, ec);
    if (ec) {
        errorMessage = "Không thể tạo thư mục lưu ảnh: " + targetDir + " (" + ec.message() + ")";
        return false;
    }

    // 6. Tạo tên file duy nhất tránh xung đột
    std::string cleanId = productId;
    for (char& ch : cleanId) {
        if (ch == '-' || ch == ' ' || ch == '/' || ch == '\\') ch = '_';
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string uniqueFileName = "prod_" + cleanId + "_" + std::to_string(ms) + ext;

    std::filesystem::path destPhysical = std::filesystem::path(targetDir) / uniqueFileName;
    std::string newRelativePath = "Device/productImages/" + uniqueFileName;

    // 7. Thực hiện sao chép file vật lý
    std::filesystem::copy_file(srcP, destPhysical, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        errorMessage = "Không thể sao chép file ảnh vào hệ thống: " + ec.message();
        return false;
    }
    FileHelper::flushFile(destPhysical.string());

    // 8. Cập nhật mapping trong ProductImageRepository (Transaction & Rollback)
    std::string oldRelative = imgRepo->getImagePath(productId);
    imgRepo->setImagePath(productId, newRelativePath);

    if (!imgRepo->saveToFile()) {
        // ROLLBACK:
        if (oldRelative.empty()) {
            imgRepo->removeImagePath(productId);
        } else {
            imgRepo->setImagePath(productId, oldRelative);
        }
        std::filesystem::remove(destPhysical, ec);
        errorMessage = "Lỗi khi lưu ánh xạ ảnh vào ProductImage.txt!";
        return false;
    }

    return true;
}

inline void PhoneService::rebuildIndices() {
    idTable.clear();
    priceTree.clear();
    const Vector<Phone>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        idTable.insert(list[i].getID(), list[i]);
        // FIX LỖI: Chèn vào BST với Composite Key (price, id)
        priceTree.insert(PhonePriceKey{list[i].getSellingPrice(), list[i].getID()}, list[i]);
    }
}

inline const Vector<Phone>& PhoneService::getAllPhones() const {
    return repo.getAll();
}

inline Phone* PhoneService::getPhoneById(const std::string& id) {
    return idTable.find(id);
}

inline Vector<Phone> PhoneService::filterByPriceRange(double minPrice, double maxPrice) const {
    if (minPrice > maxPrice) return Vector<Phone>();
    // FIX LỖI: Biên min lấy từ id rỗng "", biên max lấy từ chuỗi ký tự kết thúc
    PhonePriceKey minKey{minPrice, ""};
    PhonePriceKey maxKey{maxPrice, "\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F"};
    return priceTree.findRange(minKey, maxKey);
}

inline Vector<Phone> PhoneService::searchByName(const std::string& keyword) const {
    Vector<Phone> results;
    const Vector<Phone>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(list[i].getName(), keyword)) {
            results.push_back(list[i]);
        }
    }
    return results;
}

inline Vector<Phone> PhoneService::searchByBrand(const std::string& brand) const {
    Vector<Phone> results;
    const Vector<Phone>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(list[i].getBrand(), brand)) {
            results.push_back(list[i]);
        }
    }
    return results;
}

inline bool PhoneService::addPhone(const Phone& phone, std::string& errorMessage) {
    Phone newPhone = phone;
    if (newPhone.getID().empty()) {
        std::vector<std::string> existingIds;
        const Vector<Phone>& phones = repo.getAll();
        for (int i = 0; i < phones.getSize(); ++i) {
            existingIds.push_back(phones[i].getID());
        }
        std::string generatedId;
        if (!IdGenerator::nextId("PHN", "", existingIds, generatedId, errorMessage)) {
            return false;
        }
        newPhone.setID(generatedId);
    } else if (!IdGenerator::isValidId(newPhone.getID(), "PHN")) {
        errorMessage = "Mã sản phẩm phải đúng định dạng PHN-000001.";
        return false;
    }
    if (idTable.contains(newPhone.getID())) {
        errorMessage = "Mã sản phẩm [" + newPhone.getID() + "] đã tồn tại trong hệ thống!";
        return false;
    }
    if (newPhone.getName().empty()) {
        errorMessage = "Tên sản phẩm không được để trống!";
        return false;
    }
    if (newPhone.getSellingPrice() <= 0) {
        errorMessage = "Giá bán phải lớn hơn 0!";
        return false;
    }
    if (newPhone.getQuantity() < 0) {
        errorMessage = "Số lượng tồn kho không được âm!";
        return false;
    }

    if (!repo.addPhone(newPhone)) {
        errorMessage = "Lỗi khi lưu sản phẩm vào cơ sở dữ liệu!";
        return false;
    }

    // Cập nhật cấu trúc dữ liệu bộ nhớ
    idTable.insert(newPhone.getID(), newPhone);
    priceTree.insert(PhonePriceKey{newPhone.getSellingPrice(), newPhone.getID()}, newPhone);
    return true;
}

inline bool PhoneService::addPhone(const Phone& phone) {
    std::string err;
    return addPhone(phone, err);
}

inline bool PhoneService::updatePhone(const Phone& phone, std::string& errorMessage) {
    if (!idTable.contains(phone.getID())) {
        errorMessage = "Không tìm thấy sản phẩm có mã [" + phone.getID() + "] để cập nhật!";
        return false;
    }
    if (phone.getSellingPrice() <= 0) {
        errorMessage = "Giá bán phải lớn hơn 0!";
        return false;
    }
    if (phone.getQuantity() < 0) {
        errorMessage = "Số lượng tồn kho không được âm!";
        return false;
    }

    if (!repo.updatePhone(phone)) {
        errorMessage = "Lỗi khi cập nhật sản phẩm vào cơ sở dữ liệu!";
        return false;
    }

    rebuildIndices();
    return true;
}

inline bool PhoneService::updatePhone(const Phone& phone) {
    std::string err;
    return updatePhone(phone, err);
}

inline bool PhoneService::deletePhone(const std::string& id, std::string& errorMessage) {
    if (!idTable.contains(id)) {
        errorMessage = "Không tìm thấy sản phẩm có mã [" + id + "] để xóa!";
        return false;
    }

    // FIX LỖI: Kiểm tra toàn vẹn tham chiếu, ngăn xóa sản phẩm đã có đơn hàng/hóa đơn/phiếu nhập
    if (refChecker != nullptr && refChecker->isPhoneReferenced(id)) {
        errorMessage = "Không thể xóa sản phẩm [" + id + "] vì đã phát sinh giao dịch trong Đơn hàng, Hóa đơn hoặc Phiếu nhập!";
        return false;
    }

    if (!repo.deletePhone(id)) {
        errorMessage = "Lỗi khi xóa sản phẩm khỏi cơ sở dữ liệu!";
        return false;
    }

    rebuildIndices();
    return true;
}

inline bool PhoneService::deletePhone(const std::string& id) {
    std::string err;
    return deletePhone(id, err);
}

inline bool PhoneService::hasStock(const std::string& id, int requestedQuantity) {
    Phone* p = getPhoneById(id);
    if (p == nullptr) return false;
    return p->getQuantity() >= requestedQuantity;
}

inline bool PhoneService::updateStock(const std::string& id, int quantityDelta, std::string& errorMessage) {
    Phone* p = getPhoneById(id);
    if (p == nullptr) {
        errorMessage = "Không tìm thấy sản phẩm có mã: " + id;
        return false;
    }

    int oldQty = p->getQuantity();
    int newQty = oldQty + quantityDelta;
    if (newQty < 0) {
        errorMessage = "Số lượng tồn kho không đủ để xuất (hiện có: " 
                       + std::to_string(p->getQuantity()) + ")!";
        return false;
    }

    p->setQuantity(newQty);
    // FIX LỖI: Kiểm tra kết quả ghi đĩa của repository, rollback nếu lưu thất bại
    if (!repo.updatePhone(*p)) {
        p->setQuantity(oldQty); // Rollback RAM
        errorMessage = "Lỗi khi cập nhật số lượng tồn kho vào cơ sở dữ liệu!";
        return false;
    }

    rebuildIndices();
    return true;
}

inline bool PhoneService::updateStock(const std::string& id, int quantityDelta) {
    std::string err;
    return updateStock(id, quantityDelta, err);
}

#endif // PHONESERVICE_H
