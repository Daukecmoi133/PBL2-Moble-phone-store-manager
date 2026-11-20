// DISCOUNT SERVICE - Tầng nghiệp vụ quản lý mã giảm giá / voucher
#ifndef DISCOUNTSERVICE_H
#define DISCOUNTSERVICE_H

#include <iostream>
#include <string>
#include "../models/DiscountCode.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../repositories/DiscountRepository.h"
#include "../utils/AppClock.h"
#include "../utils/PricingCalculator.h"
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
    ReferenceChecker* refChecker;

    // Tái cấu trúc bảng băm từ repository
    void rebuildIndices();

    // Validate các trường khi thêm/cập nhật discount
    bool validateDiscount(const DiscountCode& dc, std::string& errorMessage) const;

public:
    DiscountService(DiscountRepository& repository, ReferenceChecker* checker = nullptr);
    void setReferenceChecker(ReferenceChecker* checker) { refChecker = checker; }

    void reload() { repo.loadFromFile(); rebuildIndices(); }
    DiscountRepository& getRepository() { return repo; }
    const DiscountRepository& getRepository() const { return repo; }

    const Vector<DiscountCode>& getAll() const;

    // Tra cứu theo mã code O(1)
    DiscountCode* getByCode(const std::string& code);
    const DiscountCode* getByCode(const std::string& code) const;

    // Lọc danh sách chỉ gồm mã còn hiệu lực tại thời điểm now
    Vector<DiscountCode> getValidCodes(const DateTime& now = AppClock::now()) const;

    // ==== NGHIỆP VỤ KIỂM TRA VOUCHER TRƯỚC KHI ÁP DỤNG ====

    // Kiểm tra voucher có thể áp dụng cho đơn hàng với giá trị orderTotal không
    // errorMessage ghi rõ lý do từ chối nếu false
    bool validateCode(const std::string& code, double orderTotal, std::string& errorMessage,
                      const DateTime& now = AppClock::now()) const;

    // Tính số tiền được giảm (không làm thay đổi usedCount)
    double calculateDiscountAmount(const std::string& code, double orderTotal,
                                   const DateTime& now = AppClock::now()) const;

    // ==== NGHIỆP VỤ ÁP DỤNG / HOÀN LẠI VOUCHER ====

    // Tăng usedCount khi giao dịch có áp dụng voucher được commit thành công
    bool consumeUsage(const std::string& code, std::string& errorMessage);
    bool consumeUsage(const std::string& code);

    // Hoàn trả 1 lượt sử dụng khi đơn hàng áp voucher bị hủy trước khi hoàn tất (D4)
    bool releaseUsage(const std::string& code, std::string& errorMessage);
    bool releaseUsage(const std::string& code);

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
    DateTime startDay(dc.getStartDate().year, dc.getStartDate().month, dc.getStartDate().day);
    DateTime endDay(dc.getEndDate().year, dc.getEndDate().month, dc.getEndDate().day);
    if (endDay < startDay) {
        errorMessage = "Ngay het han phai sau ngay bat dau hieu luc!";
        return false;
    }
    return true;
}

inline DiscountService::DiscountService(DiscountRepository& repository, ReferenceChecker* checker)
    : repo(repository), refChecker(checker) {
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

// Lọc chỉ các mã còn hiệu lực tại thời điểm now
inline Vector<DiscountCode> DiscountService::getValidCodes(const DateTime& now) const {
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
                                          std::string& errorMessage,
                                          const DateTime& now) const {
    const std::string trimmed = StringHelper::trim(code);
    if (orderTotal < 0) {
        errorMessage = "Gia tri don hang khong hop le!";
        return false;
    }

    const DiscountCode* dc = getByCode(trimmed);
    if (dc == nullptr) {
        errorMessage = "Ma voucher [" + trimmed + "] khong ton tai!";
        return false;
    }

    if (!dc->isValid(now)) {
        if (now < dc->getStartDate()) {
            errorMessage = "Voucher [" + trimmed + "] chua den ngay hieu luc!";
        } else if (now > dc->getEndDate()) {
            errorMessage = "Voucher [" + trimmed + "] da het han!";
        } else {
            errorMessage = "Voucher [" + trimmed + "] da het luot su dung!";
        }
        return false;
    }

    if (!dc->isApplicable(orderTotal, now)) {
        errorMessage = "Don hang chua dat gia tri toi thieu de ap dung voucher [" + trimmed + "]!";
        return false;
    }

    return true;
}

// Tính số tiền được giảm - KHÔNG thay đổi usedCount
inline double DiscountService::calculateDiscountAmount(const std::string& code, double orderTotal,
                                                       const DateTime& now) const {
    const DiscountCode* dc = getByCode(code);
    if (dc == nullptr) return 0.0;
    return dc->calculateDiscount(orderTotal, now);
}

// Tăng usedCount sau khi giao dịch commit thành công
// Chỉ gọi sau khi Order/Invoice đã lưu thành công (không gọi nếu giao dịch fail)
inline bool DiscountService::consumeUsage(const std::string& code, std::string& errorMessage) {
    const std::string trimmed = StringHelper::trim(code);
    DiscountCode* dc = repo.findByCode(trimmed);
    if (dc == nullptr) {
        errorMessage = "Khong tim thay voucher [" + trimmed + "] de cap nhat luot dung!";
        return false;
    }

    if (dc->getUsedCount() >= dc->getMaxUsage()) {
        errorMessage = "Voucher [" + trimmed + "] da het luot su dung!";
        return false;
    }

    dc->increaseUsage();
    if (!repo.saveToFile()) {
        repo.loadFromFile();
        rebuildIndices();
        errorMessage = "Loi khi luu trang thai voucher! Da hoan nguyen.";
        return false;
    }

    rebuildIndices();
    errorMessage = "Da ghi nhan luot su dung voucher [" + trimmed + "].";
    return true;
}

inline bool DiscountService::consumeUsage(const std::string& code) {
    std::string err;
    return consumeUsage(code, err);
}

inline bool DiscountService::releaseUsage(const std::string& code, std::string& errorMessage) {
    const std::string trimmed = StringHelper::trim(code);
    if (trimmed.empty()) {
        return true;
    }
    DiscountCode* dc = repo.findByCode(trimmed);
    if (dc == nullptr || dc->getUsedCount() <= 0) {
        return true;
    }

    dc->decreaseUsage();
    if (!repo.saveToFile()) {
        repo.loadFromFile();
        rebuildIndices();
        errorMessage = "Loi khi hoan tra luot su dung voucher [" + trimmed + "]!";
        return false;
    }

    rebuildIndices();
    errorMessage = "Da hoan tra 1 luot su dung voucher [" + trimmed + "].";
    return true;
}

inline bool DiscountService::releaseUsage(const std::string& code) {
    std::string err;
    return releaseUsage(code, err);
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
    const DiscountCode* existing = getByCode(code);
    if (existing == nullptr) {
        errorMessage = "Khong tim thay ma voucher [" + code + "] de xoa!";
        return false;
    }

    if (existing->getUsedCount() > 0 || (refChecker != nullptr && refChecker->isDiscountReferenced(code))) {
        errorMessage = "Khong the xoa ma giam gia [" + code + "] vi da co luot su dung hoac don hang/hoa don tham chieu!";
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
