#ifndef PHONESERVICE_H
#define PHONESERVICE_H

#include <iostream>
#include <string>
#include "../models/Phone.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../core/BST.h"
#include "../repositories/PhoneRepository.h"
#include "../utils/StringHelper.h"

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ xử lý dữ liệu sản phẩm,
// cài đặt và vận hành các cấu trúc dữ liệu giải thuật (HashTable O(1), BST O(log N)).
class PhoneService {
private:
    PhoneRepository& repo;
    HashTable<std::string, Phone> idTable; // Bảng băm để tìm kiếm theo ID với O(1)
    BST<double, Phone> priceTree;          // Cây BST để tìm kiếm/lọc theo khoảng giá O(log N)

public:
    PhoneService(PhoneRepository& repository);

    // Tái cấu trúc các chỉ mục DSA (HashTable, BST) từ dữ liệu kho
    void rebuildIndices();

    const Vector<Phone>& getAllPhones() const;

    // Tra cứu O(1) qua bảng băm
    Phone* getPhoneById(const std::string& id);

    // Lọc theo khoảng giá O(log N) qua cây nhị phân BST
    Vector<Phone> filterByPriceRange(double minPrice, double maxPrice) const;

    // Tìm kiếm theo tên sản phẩm (không phân biệt hoa thường)
    Vector<Phone> searchByName(const std::string& keyword) const;

    // Tìm kiếm theo thương hiệu
    Vector<Phone> searchByBrand(const std::string& brand) const;

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

inline PhoneService::PhoneService(PhoneRepository& repository) : repo(repository) {
    rebuildIndices();
}

inline void PhoneService::rebuildIndices() {
    idTable.clear();
    priceTree.clear();
    const Vector<Phone>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        idTable.insert(list[i].getID(), list[i]);
        priceTree.insert(list[i].getSellingPrice(), list[i]);
    }
}

inline const Vector<Phone>& PhoneService::getAllPhones() const {
    return repo.getAll();
}

inline Phone* PhoneService::getPhoneById(const std::string& id) {
    return idTable.find(id);
}

inline Vector<Phone> PhoneService::filterByPriceRange(double minPrice, double maxPrice) const {
    return priceTree.findRange(minPrice, maxPrice);
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
    if (phone.getID().empty()) {
        errorMessage = "Mã sản phẩm không được để trống!";
        return false;
    }
    if (idTable.contains(phone.getID())) {
        errorMessage = "Mã sản phẩm [" + phone.getID() + "] đã tồn tại trong hệ thống!";
        return false;
    }
    if (phone.getName().empty()) {
        errorMessage = "Tên sản phẩm không được để trống!";
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

    if (!repo.addPhone(phone)) {
        errorMessage = "Lỗi khi lưu sản phẩm vào cơ sở dữ liệu!";
        return false;
    }

    // Cập nhật cấu trúc dữ liệu bộ nhớ
    idTable.insert(phone.getID(), phone);
    priceTree.insert(phone.getSellingPrice(), phone);
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

    int newQty = p->getQuantity() + quantityDelta;
    if (newQty < 0) {
        errorMessage = "Số lượng tồn kho không đủ để xuất (hiện có: " 
                       + std::to_string(p->getQuantity()) + ")!";
        return false;
    }

    p->setQuantity(newQty);
    repo.updatePhone(*p);
    rebuildIndices();
    return true;
}

inline bool PhoneService::updateStock(const std::string& id, int quantityDelta) {
    std::string err;
    return updateStock(id, quantityDelta, err);
}

#endif // PHONESERVICE_H
