#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === MÃ GIẢM GIÁ ===

std::vector<DiscountItemViewModel> AppController::getDiscounts() {
    auto& store = MockDataStore::getInstance();
    for (auto& d : store.discounts) {
        d.isValid = !RealTimeManager::getInstance().isDateExpired(d.expiryDate);
    }
    return store.discounts;
}

OperationResult AppController::updateDiscount(const DiscountFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    std::string codeTrim = StringHelper::trim(req.code);
    if (codeTrim.empty() || req.value <= 0) {
        return OperationResult::Fail("Mã voucher và giá trị giảm phải lớn hơn 0!");
    }
    std::string expTrim = StringHelper::trim(req.expiryDate);
    if (expTrim.empty()) {
        return OperationResult::Fail("Ngày hết hạn của voucher không được để trống!");
    }
    if (!DateTime::isValidDateString(expTrim)) {
        return OperationResult::Fail("Ngày hết hạn không hợp lệ theo lịch (định dạng chuẩn: DD/MM/YYYY)!");
    }
    if (req.minOrderValue < 0) {
        return OperationResult::Fail("Giá trị đơn hàng tối thiểu không được âm!");
    }
    double maxDisc = (req.type == 0) ? req.maxDiscountAmount : req.value;
    if (maxDisc < 0) {
        return OperationResult::Fail("Mức giảm tối đa không được âm!");
    }

    std::string oldCode = StringHelper::trim(req.id);
    if (oldCode.empty()) oldCode = codeTrim;

    // Cập nhật xuống file qua DiscountService
    if (discountService != nullptr) {
        DiscountCode* existing = discountService->getByCode(oldCode);
        if (existing == nullptr) {
            existing = discountService->getByCode(codeTrim);
        }
        DiscountType dtype = (req.type == 0) ? DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT;
        DateTime startDate = existing ? existing->getStartDate() : DateTime::now();
        DateTime endDate = DateTime::fromDateString(expTrim);
        if (endDate < startDate) {
            startDate = endDate;
        }
        int used = existing ? existing->getUsedCount() : 0;
        DiscountCode updated(codeTrim, dtype, req.value, startDate, endDate, req.minOrderValue, maxDisc, req.maxUsage, used);
        std::string serviceErr;
        if (codeTrim != oldCode && discountService->getByCode(oldCode) != nullptr) {
            if (discountService->getByCode(codeTrim) != nullptr) {
                return OperationResult::Fail("Mã voucher mới '" + codeTrim + "' đã tồn tại!");
            }
            discountService->deleteDiscount(oldCode);
            if (!discountService->addDiscount(updated, serviceErr)) {
                return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật mã giảm giá vào file dữ liệu!");
            }
        } else {
            if (!discountService->updateDiscount(updated, serviceErr)) {
                return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật mã giảm giá vào file dữ liệu!");
            }
        }
    }

    auto& store = MockDataStore::getInstance();
    bool ok = store.updateDiscount(req);
    if (!ok) {
        return OperationResult::Fail("Không tìm thấy mã giảm giá cần cập nhật!");
    }
    return OperationResult::Ok("Cập nhật thời hạn hiệu lực và thông tin mã giảm giá thành công!");
}

OperationResult AppController::addDiscount(const DiscountFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    std::string codeTrim = StringHelper::trim(req.code);
    if (codeTrim.empty() || req.value <= 0) {
        return OperationResult::Fail("Mã voucher và giá trị giảm phải lớn hơn 0!");
    }
    std::string expTrim = StringHelper::trim(req.expiryDate);
    if (expTrim.empty()) {
        return OperationResult::Fail("Ngày hết hạn của voucher không được để trống!");
    }
    if (!DateTime::isValidDateString(expTrim)) {
        return OperationResult::Fail("Ngày hết hạn không hợp lệ theo lịch (định dạng chuẩn: DD/MM/YYYY)!");
    }
    if (req.minOrderValue < 0) {
        return OperationResult::Fail("Giá trị đơn hàng tối thiểu không được âm!");
    }
    if (req.maxDiscountAmount < 0) {
        return OperationResult::Fail("Mức giảm tối đa không được âm!");
    }

    auto& store = MockDataStore::getInstance();
    for (const auto& d : store.discounts) {
        if (d.code == codeTrim) {
            return OperationResult::Fail("Mã giảm giá '" + codeTrim + "' đã tồn tại!");
        }
    }

    // Lưu xuống file qua DiscountService
    if (discountService != nullptr) {
        DiscountType dtype = (req.type == 0) ? DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT;
        DateTime startDate = DateTime::now();
        DateTime endDate = DateTime::fromDateString(expTrim);
        DiscountCode dc(codeTrim, dtype, req.value, startDate, endDate, req.minOrderValue, req.maxDiscountAmount, req.maxUsage, 0);
        std::string serviceErr;
        if (!discountService->addDiscount(dc, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu mã giảm giá vào file dữ liệu!");
        }
    }

    DiscountItemViewModel d;
    d.id = codeTrim;
    d.code = codeTrim;
    d.typeName = (req.type == 0) ? "Giảm phần trăm (%)" : "Số tiền cố định";
    d.value = req.value;
    d.formattedValue = (req.type == 0) ? (std::to_string(static_cast<int>(req.value)) + "%")
                                        : StringHelper::formatCurrency(req.value);
    d.minOrderValue = req.minOrderValue;
    d.maxDiscountAmount = req.maxDiscountAmount;
    d.expiryDate = expTrim;
    d.usedCount = 0;
    d.maxUsage = req.maxUsage;
    d.isValid = true;
    d.description = req.description;

    store.discounts.push_back(d);
    return OperationResult::Ok("Thêm mã giảm giá '" + codeTrim + "' thành công!");
}

OperationResult AppController::deleteDiscount(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    auto& store = MockDataStore::getInstance();
    for (auto it = store.discounts.begin(); it != store.discounts.end(); ++it) {
        if (it->id == id) {
            if (it->usedCount > 0) {
                return OperationResult::Fail("Không thể xóa mã giảm giá đã có lịch sử sử dụng!");
            }

            // Xóa khỏi file qua DiscountService
            if (discountService != nullptr) {
                std::string serviceErr;
                if (!discountService->deleteDiscount(id, serviceErr)) {
                    return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi xóa mã giảm giá khỏi file dữ liệu!");
                }
            }

            std::string code = it->code;
            store.discounts.erase(it);
            return OperationResult::Ok("Đã xóa mã giảm giá '" + code + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy mã giảm giá!");
}

OperationResult AppController::validateDiscount(const std::string& code, double orderTotal, double& outDiscount) {
    outDiscount = 0.0;
    auto& store = MockDataStore::getInstance();

    for (const auto& d : store.discounts) {
        if (d.code == code) {
            if (!d.isValid) {
                return OperationResult::Fail("Mã giảm giá '" + code + "' đã hết hạn sử dụng!");
            }
            if (d.usedCount >= d.maxUsage) {
                return OperationResult::Fail("Mã giảm giá '" + code + "' đã hết lượt sử dụng!");
            }
            if (orderTotal < d.minOrderValue) {
                return OperationResult::Fail("Đơn hàng chưa đạt giá trị tối thiểu " +
                                             std::to_string(static_cast<long long>(d.minOrderValue)) + " đ để áp dụng mã này!");
            }

            if (d.typeName.find("%") != std::string::npos) {
                outDiscount = (orderTotal * d.value) / 100.0;
                if (d.maxDiscountAmount > 0 && outDiscount > d.maxDiscountAmount) {
                    outDiscount = d.maxDiscountAmount;
                }
            } else {
                outDiscount = d.value;
                if (outDiscount > orderTotal) outDiscount = orderTotal;
            }
            return OperationResult::Ok("Mã giảm giá hợp lệ!");
        }
    }

    return OperationResult::Fail("Mã giảm giá '" + code + "' không tồn tại!");
}
