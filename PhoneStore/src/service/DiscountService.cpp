#include "DiscountService.h"

DiscountService::DiscountService(DiscountRepository& repo) : discountRepo(repo) {}

Result<void> DiscountService::addDiscount(const Actor&, const DiscountCode&) {
    // TODO(logic): kiểm tra quyền ManageDiscount, hợp lệ dữ liệu, trùng mã, add + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: DiscountService::addDiscount");
}

Result<void> DiscountService::updateDiscount(const Actor&, const DiscountCode&) {
    // TODO(logic): kiểm tra quyền ManageDiscount, update + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: DiscountService::updateDiscount");
}

Result<void> DiscountService::removeDiscount(const Actor&, const std::string&) {
    // TODO(logic): kiểm tra quyền ManageDiscount, remove + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: DiscountService::removeDiscount");
}

Result<Vector<DiscountCode>> DiscountService::listAll(const Actor&) const {
    // TODO(logic): kiểm tra quyền rồi trả danh sách mã giảm giá.
    return Result<Vector<DiscountCode>>::fail(ErrorCode::NotImplemented, "TODO: DiscountService::listAll");
}

Result<Money> DiscountService::validateAndCalculate(const std::string&, Money) const {
    // TODO(logic): findByCode, kiểm tra hạn/lượt dùng/giá trị tối thiểu rồi tính số tiền được giảm.
    return Result<Money>::fail(ErrorCode::NotImplemented, "TODO: DiscountService::validateAndCalculate");
}

Result<void> DiscountService::consumeUsage(UnitOfWork&, const std::string&) {
    // TODO(logic): enlist discountRepo vào uow, usedCount += 1.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: DiscountService::consumeUsage");
}

Result<void> DiscountService::releaseUsage(UnitOfWork&, const std::string&) {
    // TODO(logic): enlist discountRepo vào uow, usedCount -= 1 (không âm).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: DiscountService::releaseUsage");
}
