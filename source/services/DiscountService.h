#ifndef DISCOUNTSERVICE_H
#define DISCOUNTSERVICE_H

#include <iostream>
#include <string>
#include "../models/DiscountCode.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../repositories/DiscountRepository.h"
#include "../utils/DateTime.h"

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ kiểm tra, tính toán và áp dụng mã giảm giá
class DiscountService {
private:
    DiscountRepository& repo;
    HashTable<std::string, DiscountCode> codeTable; // Bảng băm tra cứu mã O(1)

public:
    DiscountService(DiscountRepository& repository);

    void rebuildIndices();
    const Vector<DiscountCode>& getAllDiscounts() const;
    DiscountCode* getDiscountByCode(const std::string& code);

    // Nghiệp vụ xác thực mã giảm giá cho một giá trị đơn hàng cụ thể
    bool validateDiscount(const std::string& code, double orderTotal, std::string& errorMessage);

    // Tính số tiền được giảm
    double calculateDiscountAmount(const std::string& code, double orderTotal);

    // Nghiệp vụ áp dụng mã giảm giá: kiểm tra, tính tiền và tăng số lượt đã dùng
    bool applyDiscount(const std::string& code, double orderTotal, double& discountAmount, std::string& errorMessage);

    // Thêm mã giảm giá mới với kiểm tra nghiệp vụ trùng mã
    bool addDiscount(const DiscountCode& dc, std::string& errorMessage);
    bool addDiscount(const DiscountCode& dc);
};

// ================= IMPLEMENTATION =================

inline DiscountService::DiscountService(DiscountRepository& repository) : repo(repository) {
    rebuildIndices();
}

inline void DiscountService::rebuildIndices() {
    codeTable.clear();
    const Vector<DiscountCode>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        codeTable.insert(list[i].getCode(), list[i]);
    }
}

inline const Vector<DiscountCode>& DiscountService::getAllDiscounts() const {
    return repo.getAll();
}

inline DiscountCode* DiscountService::getDiscountByCode(const std::string& code) {
    return codeTable.find(code);
}

inline bool DiscountService::validateDiscount(const std::string& code, double orderTotal, std::string& errorMessage) {
    DiscountCode* dc = getDiscountByCode(code);
    if (dc == nullptr) {
        errorMessage = "Mã giảm giá [" + code + "] không tồn tại!";
        return false;
    }

    DateTime now = DateTime::now();
    if (dc->getUsedCount() >= dc->getMaxUsage()) {
        errorMessage = "Mã giảm giá [" + code + "] đã hết lượt sử dụng!";
        return false;
    }

    if (now < dc->getStartDate()) {
        errorMessage = "Mã giảm giá [" + code + "] chưa đến ngày áp dụng!";
        return false;
    }

    if (now > dc->getEndDate()) {
        errorMessage = "Mã giảm giá [" + code + "] đã hết hạn sử dụng!";
        return false;
    }

    if (orderTotal < dc->getMinOrderValue()) {
        errorMessage = "Đơn hàng chưa đạt giá trị tối thiểu (" 
                       + std::to_string(static_cast<long long>(dc->getMinOrderValue())) 
                       + " đ) để dùng mã này!";
        return false;
    }

    return true;
}

inline double DiscountService::calculateDiscountAmount(const std::string& code, double orderTotal) {
    DiscountCode* dc = getDiscountByCode(code);
    if (dc == nullptr) return 0.0;
    return dc->calculateDiscount(orderTotal);
}

inline bool DiscountService::applyDiscount(const std::string& code, double orderTotal, double& discountAmount, std::string& errorMessage) {
    if (!validateDiscount(code, orderTotal, errorMessage)) {
        discountAmount = 0.0;
        return false;
    }

    DiscountCode* dc = getDiscountByCode(code);
    discountAmount = dc->calculateDiscount(orderTotal);
    dc->increaseUsage();

    // Lưu trạng thái đã sử dụng vào kho dữ liệu
    repo.updateDiscount(*dc);
    rebuildIndices();
    return true;
}

inline bool DiscountService::addDiscount(const DiscountCode& dc, std::string& errorMessage) {
    if (dc.getCode().empty()) {
        errorMessage = "Mã giảm giá không được để trống!";
        return false;
    }

    if (codeTable.contains(dc.getCode())) {
        errorMessage = "Mã giảm giá [" + dc.getCode() + "] đã tồn tại!";
        return false;
    }

    if (!repo.addDiscount(dc)) {
        errorMessage = "Lỗi khi lưu mã giảm giá vào cơ sở dữ liệu!";
        return false;
    }

    codeTable.insert(dc.getCode(), dc);
    return true;
}

inline bool DiscountService::addDiscount(const DiscountCode& dc) {
    std::string err;
    return addDiscount(dc, err);
}

#endif // DISCOUNTSERVICE_H
