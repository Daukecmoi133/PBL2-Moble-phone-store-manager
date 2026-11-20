#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === MÃ GIẢM GIÁ ===

std::vector<DiscountItemViewModel> AppController::getDiscounts() {
    std::vector<DiscountItemViewModel> result;
    if (discountService == nullptr) return result;
    const Vector<DiscountCode>& list = discountService->getAll();
    result.reserve(static_cast<size_t>(list.getSize()));
    for (int i = 0; i < list.getSize(); ++i) {
        result.push_back(discountToViewModel(list[i]));
    }
    return result;
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
    if (discountService == nullptr) {
        return OperationResult::Fail("Dịch vụ mã giảm giá chưa sẵn sàng!");
    }

    std::string oldCode = StringHelper::trim(req.id);
    if (oldCode.empty()) oldCode = codeTrim;

    DiscountCode* existing = discountService->getByCode(oldCode);
    if (existing == nullptr) {
        existing = discountService->getByCode(codeTrim);
        if (existing == nullptr) {
            return OperationResult::Fail("Không tìm thấy mã giảm giá cần cập nhật!");
        }
        oldCode = existing->getCode();
    }

    DiscountType dtype = (req.type == 0) ? DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT;
    DateTime startDate = existing->getStartDate();
    DateTime endDate = DateTime::fromDateString(expTrim);
    endDate.hour = 23;
    endDate.minute = 59;
    endDate.second = 59;
    if (endDate < startDate) {
        startDate = endDate;
    }
    int used = existing->getUsedCount();
    DiscountCode updated(codeTrim, dtype, req.value, startDate, endDate, req.minOrderValue, maxDisc, req.maxUsage, used);
    std::string serviceErr;
    if (codeTrim != oldCode) {
        if (discountService->getByCode(codeTrim) != nullptr) {
            return OperationResult::Fail("Mã voucher mới '" + codeTrim + "' đã tồn tại!");
        }
        if (!discountService->deleteDiscount(oldCode, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Không thể đổi mã giảm giá đã có lịch sử tham chiếu!");
        }
        if (!discountService->addDiscount(updated, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật mã giảm giá vào file dữ liệu!");
        }
    } else {
        if (!discountService->updateDiscount(updated, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật mã giảm giá vào file dữ liệu!");
        }
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
    if (discountService == nullptr) {
        return OperationResult::Fail("Dịch vụ mã giảm giá chưa sẵn sàng!");
    }
    if (discountService->getByCode(codeTrim) != nullptr) {
        return OperationResult::Fail("Mã giảm giá '" + codeTrim + "' đã tồn tại!");
    }

    DiscountType dtype = (req.type == 0) ? DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT;
    DateTime startDate = AppClock::now();
    startDate.hour = 0;
    startDate.minute = 0;
    startDate.second = 0;
    DateTime endDate = DateTime::fromDateString(expTrim);
    endDate.hour = 23;
    endDate.minute = 59;
    endDate.second = 59;
    DiscountCode dc(codeTrim, dtype, req.value, startDate, endDate, req.minOrderValue, req.maxDiscountAmount, req.maxUsage, 0);
    std::string serviceErr;
    if (!discountService->addDiscount(dc, serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu mã giảm giá vào file dữ liệu!");
    }

    return OperationResult::Ok("Thêm mã giảm giá '" + codeTrim + "' thành công!");
}

OperationResult AppController::deleteDiscount(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    if (discountService == nullptr) {
        return OperationResult::Fail("Dịch vụ mã giảm giá chưa sẵn sàng!");
    }
    const DiscountCode* existing = discountService->getByCode(id);
    if (existing == nullptr) {
        return OperationResult::Fail("Không tìm thấy mã giảm giá!");
    }
    if (existing->getUsedCount() > 0) {
        return OperationResult::Fail("Không thể xóa mã giảm giá đã có lịch sử sử dụng!");
    }

    std::string code = existing->getCode();
    std::string serviceErr;
    if (!discountService->deleteDiscount(code, serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi xóa mã giảm giá khỏi file dữ liệu!");
    }

    return OperationResult::Ok("Đã xóa mã giảm giá '" + code + "'!");
}

double AppController::getVatRate() const {
    return storeSettingsRepo.getVatRate();
}

OperationResult AppController::setVatRate(double newRate) {
    REQUIRE_ADMIN_ROLE();
    if (std::isnan(newRate) || std::isinf(newRate) || newRate < 0.0 || newRate > 1.0) {
        return OperationResult::Fail("Thuế suất VAT phải nằm trong khoảng 0% đến 100% (0.0 - 1.0)!");
    }
    const double prevRate = storeSettingsRepo.getVatRate();
    if (!storeSettingsRepo.setVatRate(newRate)) {
        return OperationResult::Fail("Giá trị thuế suất VAT không hợp lệ!");
    }
    if (!storeSettingsRepo.saveToFile()) {
        storeSettingsRepo.setVatRate(prevRate);
        return OperationResult::Fail("Không thể lưu cấu hình thuế VAT xuống file StoreSettings.txt!");
    }
    return OperationResult::Ok("Đã cập nhật thuế suất VAT thành " + PricingCalculator::formatVatPercent(newRate) + "!");
}

OperationResult AppController::validateDiscount(const std::string& code, double orderTotal, double& outDiscount) {
    const std::string customerId = currentSession.isCustomer() ? currentSession.userId : "";
    return validateDiscountForCustomer(code, orderTotal, customerId, outDiscount);
}

OperationResult AppController::validateDiscountForCustomer(const std::string& code,
                                                           double orderTotal,
                                                           const std::string& customerId,
                                                           double& outDiscount) {
    outDiscount = 0.0;
    const std::string codeTrim = StringHelper::trim(code);
    if (discountService == nullptr) {
        return OperationResult::Fail("Dịch vụ mã giảm giá chưa sẵn sàng!");
    }

    std::string svcErr;
    const DateTime simNow = AppClock::now();
    if (!discountService->validateCode(codeTrim, orderTotal, svcErr, simNow)) {
        return OperationResult::Fail(!svcErr.empty() ? svcErr : ("Mã giảm giá '" + codeTrim + "' không hợp lệ!"));
    }

    if (orderService != nullptr &&
        orderService->hasCustomerUsedDiscountCode(customerId, codeTrim)) {
        return OperationResult::Fail("Khách hàng đã sử dụng mã giảm giá '" + codeTrim + "' trước đó (mỗi khách chỉ dùng tối đa 1 lần cho mỗi mã)!");
    }

    const DiscountCode* dc = discountService->getByCode(codeTrim);
    if (dc == nullptr) {
        return OperationResult::Fail("Mã giảm giá '" + codeTrim + "' không tồn tại!");
    }

    const PricingBreakdown pb = PricingCalculator::computeWithVoucher(
        orderTotal, dc->getType(), dc->getValue(), dc->getMaxDiscountValue(), getVatRate());
    outDiscount = pb.discountAmount;
    return OperationResult::Ok("Mã giảm giá hợp lệ!");
}
