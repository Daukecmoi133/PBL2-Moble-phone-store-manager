// DISCOUNT SERVICE - Tầng nghiệp vụ quản lý mã giảm giá / voucher
#ifndef DISCOUNTSERVICE_H
#define DISCOUNTSERVICE_H

#include <iostream>
#include <string>
#include "../models/DiscountCode.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../repositories/DiscountRepository.h"
#include "../utils/StringHelper.h"
#include "Check.h"

// Tầng Service: Toàn bộ nghiệp vụ quản lý và áp dụng mã giảm giá
// Ứng dụng cấu trúc dữ liệu:
// - HashTable<string, DiscountCode>: Tra cứu mã voucher O(1)
// Nguyên tắc: usedCount chỉ được tăng qua consumeUsage(); UI không được sửa trực tiếp.
class DiscountService {
private:
    DiscountRepository& repo;
    HashTable<std::string, DiscountCode> codeTable; // Tra cứu O(1) theo code

    // Tái cấu trúc bảng băm từ repository
    void rebuildIndices();

    // Validate các trường khi thêm/cập nhật discount
    bool validateDiscount(const DiscountCode& dc, std::string& errorMessage) const;

public:
    DiscountService(DiscountRepository& repository);

    void reload() { repo.loadFromFile(); rebuildIndices(); }
    DiscountRepository& getRepository() { return repo; }
    const DiscountRepository& getRepository() const { return repo; }

    const Vector<DiscountCode>& getAll() const;

    // Tra cứu theo mã code O(1)
    DiscountCode* getByCode(const std::string& code);
    const DiscountCode* getByCode(const std::string& code) const;

    // Lọc danh sách chỉ gồm mã còn hiệu lực
    Vector<DiscountCode> getValidCodes() const;

    // ==== NGHIỆP VỤ KIỂM TRA VOUCHER TRƯỚC KHI ÁP DỤNG ====

    // Kiểm tra voucher có thể áp dụng cho đơn hàng với giá trị orderTotal không
    // errorMessage ghi rõ lý do từ chối nếu false
    bool validateCode(const std::string& code, double orderTotal, std::string& errorMessage) const;

    // Tính số tiền được giảm (không làm thay đổi usedCount)
    double calculateDiscountAmount(const std::string& code, double orderTotal) const;

    // ==== NGHIỆP VỤ ÁP DỤNG VOUCHER (commit giao dịch) ====

    // Tăng usedCount khi giao dịch có áp dụng voucher được commit thành công
    // Chỉ gọi sau khi Order/Invoice đã lưu thành công
    bool consumeUsage(const std::string& code, std::string& errorMessage);
    bool consumeUsage(const std::string& code);

    // ==== CRUD QUẢN LÝ MÃ GIẢM GIÁ ====

    bool addDiscount(const DiscountCode& dc, std::string& errorMessage);
    bool addDiscount(const DiscountCode& dc);

    bool updateDiscount(const DiscountCode& updated, std::string& errorMessage);
    bool updateDiscount(const DiscountCode& updated);

    bool deleteDiscount(const std::string& code, std::string& errorMessage);
    bool deleteDiscount(const std::string& code);
};

// ================= IMPLEMENTATION =================

inline void DiscountService::rebuildIndices() {
    codeTable.clear();
    const Vector<DiscountCode>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        if (!list[i].getCode().empty()) {
            codeTable.insert(list[i].getCode(), list[i]);
        }
    }
}

// Validate các trường nghiệp vụ của DiscountCode (dùng chung cho add/update)
inline bool DiscountService::validateDiscount(const DiscountCode& dc, std::string& errorMessage) const {
    if (dc.getCode().empty()) {
        errorMessage = "Ma voucher khong duoc de trong!";
        return false;
    }
    if (dc.getValue() <= 0) {
        errorMessage = "Gia tri giam phai lon hon 0!";
        return false;
    }
    if (dc.getType() == DiscountType::PERCENTAGE && dc.getValue() > 100.0) {
        errorMessage = "Gia tri giam theo % khong the vuot qua 100!";
        return false;
    }
    if (dc.getMinOrderValue() < 0) {
        errorMessage = "Gia tri don toi thieu khong duoc am!";
        return false;
    }
    if (dc.getMaxDiscountValue() < 0) {
        errorMessage = "Muc giam toi da khong duoc am!";
        return false;
    }
    if (dc.getMaxUsage() <= 0) {
        errorMessage = "So luot su dung toi da phai lon hon 0!";
        return false;
    }
    if (!DateTime::isValidDate(dc.getStartDate().year, dc.getStartDate().month, dc.getStartDate().day)) {
        errorMessage = "Ngay bat dau hieu luc khong hop le theo lich!";
        return false;
    }
    if (!DateTime::isValidDate(dc.getEndDate().year, dc.getEndDate().month, dc.getEndDate().day)) {
        errorMessage = "Ngay het han khong hop le theo lich!";
        return false;
    }
    if (dc.getEndDate() < dc.getStartDate()) {
        errorMessage = "Ngay het han phai sau ngay bat dau hieu luc!";
        return false;
    }
    return true;
}

inline DiscountService::DiscountService(DiscountRepository& repository)
    : repo(repository) {
    rebuildIndices();
}

inline const Vector<DiscountCode>& DiscountService::getAll() const {
    return repo.getAll();
}

// Tra cứu O(1) theo code từ HashTable
inline DiscountCode* DiscountService::getByCode(const std::string& code) {
    // Trả về từ repo (nguồn sự thật) thông qua code tra trong HashTable
    DiscountCode* cached = codeTable.find(StringHelper::trim(code));
    if (cached == nullptr) return nullptr;
    return repo.findByCode(cached->getCode());
}

inline const DiscountCode* DiscountService::getByCode(const std::string& code) const {
    const DiscountCode* cached = codeTable.find(StringHelper::trim(code));
    if (cached == nullptr) return nullptr;
    return repo.findByCode(cached->getCode());
}

// Lọc chỉ các mã còn hiệu lực tại thời điểm hiện tại
inline Vector<DiscountCode> DiscountService::getValidCodes() const {
    DateTime now = DateTime::now();
    Vector<DiscountCode> result;
    const Vector<DiscountCode>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        if (list[i].isValid(now)) {
            result.push_back(list[i]);
        }
    }
    return result;
}

// Kiểm tra voucher có thể áp dụng cho đơn hàng giá trị orderTotal không
// Trả về false kèm lý do nếu không hợp lệ
inline bool DiscountService::validateCode(const std::string& code, double orderTotal,
                                          std::string& errorMessage) const {
    if (orderTotal < 0) {
        errorMessage = "Gia tri don hang khong hop le!";
        return false;
    }

    const DiscountCode* dc = getByCode(code);
    if (dc == nullptr) {
        errorMessage = "Ma voucher [" + code + "] khong ton tai!";
        return false;
    }

    DateTime now = DateTime::now();
    if (!dc->isValid(now)) {
        if (now < dc->getStartDate()) {
            errorMessage = "Voucher [" + code + "] chua den ngay hieu luc!";
        } else if (now > dc->getEndDate()) {
            errorMessage = "Voucher [" + code + "] da het han!";
        } else {
            errorMessage = "Voucher [" + code + "] da het luot su dung!";
        }
        return false;
    }

    if (!dc->isApplicable(orderTotal)) {
        errorMessage = "Don hang chua dat gia tri toi thieu de ap dung voucher [" + code + "]!";
        return false;
    }

    return true;
}

// Tính số tiền được giảm - KHÔNG thay đổi usedCount
inline double DiscountService::calculateDiscountAmount(const std::string& code, double orderTotal) const {
    const DiscountCode* dc = getByCode(code);
    if (dc == nullptr) return 0.0;
    return dc->calculateDiscount(orderTotal);
}

// Tăng usedCount sau khi giao dịch commit thành công
// Chỉ gọi sau khi Order/Invoice đã lưu thành công (không gọi nếu giao dịch fail)
inline bool DiscountService::consumeUsage(const std::string& code, std::string& errorMessage) {
    DiscountCode* dc = repo.findByCode(StringHelper::trim(code));
    if (dc == nullptr) {
        errorMessage = "Khong tim thay voucher [" + code + "] de cap nhat luot dung!";
        return false;
    }

    if (dc->getUsedCount() >= dc->getMaxUsage()) {
        errorMessage = "Voucher [" + code + "] da het luot su dung!";
        return false;
    }

    dc->increaseUsage();
    if (!repo.saveToFile()) {
        // Rollback trong RAM (giảm lại usedCount)
        // increaseUsage đã tăng, ta cần rollback
        // Tạo bản sao đã rollback
        DiscountCode rolled = *dc;
        // Không có decreaseUsage() → trực tiếp cập nhật object qua constructor
        std::string err;
        dc->increaseUsage(); // cố ý tăng thêm để ghi chú cần có method rollback
        // Thực tế: load lại từ file để rollback
        repo.loadFromFile();
        rebuildIndices();
        errorMessage = "Loi khi luu trang thai voucher! Da hoan nguyen.";
        return false;
    }

    rebuildIndices();
    errorMessage = "Da ghi nhan luot su dung voucher [" + code + "].";
    return true;
}

inline bool DiscountService::consumeUsage(const std::string& code) {
    std::string err;
    return consumeUsage(code, err);
}

// Thêm mã giảm giá mới
inline bool DiscountService::addDiscount(const DiscountCode& dc, std::string& errorMessage) {
    if (!validateDiscount(dc, errorMessage)) return false;

    if (codeTable.contains(dc.getCode())) {
        errorMessage = "Ma voucher [" + dc.getCode() + "] da ton tai!";
        return false;
    }

    if (!repo.addDiscount(dc)) {
        errorMessage = "Loi khi luu ma giam gia vao co so du lieu!";
        return false;
    }

    codeTable.insert(dc.getCode(), dc);
    errorMessage = "Them ma giam gia thanh cong!";
    return true;
}

inline bool DiscountService::addDiscount(const DiscountCode& dc) {
    std::string err;
    return addDiscount(dc, err);
}

// Cập nhật mã giảm giá (không cho đổi code, không cho giảm usedCount)
inline bool DiscountService::updateDiscount(const DiscountCode& updated, std::string& errorMessage) {
    if (!codeTable.contains(updated.getCode())) {
        errorMessage = "Khong tim thay ma voucher [" + updated.getCode() + "] de cap nhat!";
        return false;
    }

    const DiscountCode* existing = getByCode(updated.getCode());
    if (existing != nullptr && updated.getUsedCount() < existing->getUsedCount()) {
        errorMessage = "Khong duoc phep giam so luot da su dung (usedCount)!";
        return false;
    }

    if (!validateDiscount(updated, errorMessage)) return false;

    if (!repo.updateDiscount(updated)) {
        errorMessage = "Loi khi cap nhat ma giam gia!";
        return false;
    }

    rebuildIndices();
    errorMessage = "Cap nhat ma giam gia thanh cong!";
    return true;
}

inline bool DiscountService::updateDiscount(const DiscountCode& updated) {
    std::string err;
    return updateDiscount(updated, err);
}

// Xóa mã giảm giá
inline bool DiscountService::deleteDiscount(const std::string& code, std::string& errorMessage) {
    if (!codeTable.contains(code)) {
        errorMessage = "Khong tim thay ma voucher [" + code + "] de xoa!";
        return false;
    }

    if (!repo.deleteDiscount(code)) {
        errorMessage = "Loi khi xoa ma giam gia!";
        return false;
    }

    rebuildIndices();
    errorMessage = "Xoa ma giam gia thanh cong!";
    return true;
}

inline bool DiscountService::deleteDiscount(const std::string& code) {
    std::string err;
    return deleteDiscount(code, err);
}

#endif // DISCOUNTSERVICE_H
