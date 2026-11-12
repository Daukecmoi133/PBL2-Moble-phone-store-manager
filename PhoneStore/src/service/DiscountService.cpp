#include "DiscountService.h"
#include "../common/Permission.h"

DiscountService::DiscountService(DiscountRepository& repo) : discountRepo(repo) {}

Result<void> DiscountService::addDiscount(const Actor& actor, const DiscountCode& discount) {
    if (!Permission::isAllowed(actor.role, Action::ManageDiscount)) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Không có quyền thêm mã giảm giá");
    }
    auto addRes = discountRepo.add(discount);
    if (!addRes.isOk()) return addRes;
    return discountRepo.save();
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

Result<Money> DiscountService::validateAndCalculate(const std::string& code, Money orderTotal) const {
    auto res = discountRepo.findById(code);
    if (!res.isOk()) return Result<Money>::fail(ErrorCode::NotFound, "Mã giảm giá không tồn tại: " + code);
    const DiscountCode& dc = res.value();
    if (!dc.isValid()) return Result<Money>::fail(ErrorCode::InvalidState, "Mã giảm giá đã hết hạn hoặc hết lượt dùng");
    if (!dc.isApplicable(static_cast<double>(orderTotal))) {
        return Result<Money>::fail(ErrorCode::InvalidState, "Đơn hàng chưa đạt giá trị tối thiểu để áp dụng mã");
    }
    double discount = dc.calculateDiscount(static_cast<double>(orderTotal));
    return Result<Money>::ok(static_cast<Money>(discount));
}

Result<void> DiscountService::consumeUsage(UnitOfWork& uow, const std::string& code) {
    auto res = discountRepo.findById(code);
    if (!res.isOk()) return Result<void>::fail(ErrorCode::NotFound, "Mã giảm giá không tồn tại");
    DiscountCode dc = res.value();
    uow.enlist(discountRepo);
    dc.increaseUsage();
    discountRepo.update(dc);
    return Result<void>::ok();
}

Result<void> DiscountService::releaseUsage(UnitOfWork& uow, const std::string& code) {
    auto res = discountRepo.findById(code);
    if (!res.isOk()) return Result<void>::fail(ErrorCode::NotFound, "Mã giảm giá không tồn tại");
    DiscountCode dc = res.value();
    uow.enlist(discountRepo);
    if (dc.getUsedCount() > 0) {
        dc.setUsedCount(dc.getUsedCount() - 1);
        discountRepo.update(dc);
    }
    return Result<void>::ok();
}
