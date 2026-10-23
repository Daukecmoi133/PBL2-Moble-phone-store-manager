#include "DiscountRepository.h"

DiscountRepository::DiscountRepository(FileManager& fm) : Repository<DiscountCode>(fm, "discounts.txt") {}

std::string DiscountRepository::getId(const DiscountCode& d) const { return d.getCode(); }

// Định dạng chuyển nguyên từ DiscountRepository::saveToFile() cũ (legacy/repositories/DiscountRepository.h):
// code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount
std::string DiscountRepository::serialize(const DiscountCode& dc) const {
    return dc.getCode() + "|" +
           (dc.getType() == DiscountType::PERCENTAGE ? "0" : "1") + "|" +
           std::to_string(static_cast<long long>(dc.getValue())) + "|" +
           dc.getStartDate().toDateString() + "|" +
           dc.getEndDate().toDateString() + "|" +
           std::to_string(static_cast<long long>(dc.getMinOrderValue())) + "|" +
           std::to_string(static_cast<long long>(dc.getMaxDiscountValue())) + "|" +
           std::to_string(dc.getMaxUsage()) + "|" +
           std::to_string(dc.getUsedCount());
}

Result<DiscountCode> DiscountRepository::deserialize(const std::string& line) const {
    try {
        DiscountCode d = DiscountCode::fromFileLine(line);
        if (d.getCode().empty()) return Result<DiscountCode>::fail(ErrorCode::ParseError, "Dòng mã giảm giá không hợp lệ");
        return Result<DiscountCode>::ok(d);
    } catch (...) {
        return Result<DiscountCode>::fail(ErrorCode::ParseError, "Không phân tích được dòng mã giảm giá");
    }
}

void DiscountRepository::rebuildIndexes() {
    // TODO(logic): dựng index theo mã voucher nếu cần (ID của repository này chính là mã voucher).
}

Result<DiscountCode> DiscountRepository::findByCode(const std::string&) const {
    // TODO(logic): tra theo mã voucher (không phân biệt hoa thường?).
    return Result<DiscountCode>::fail(ErrorCode::NotImplemented, "TODO: DiscountRepository::findByCode");
}
