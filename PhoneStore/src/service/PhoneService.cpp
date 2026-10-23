#include "PhoneService.h"

PhoneService::PhoneService(PhoneRepository& repo) : phoneRepo(repo) {}

Result<Vector<PhoneView>> PhoneService::listForCustomer(Vector<std::string>*) const {
    // TODO(logic): đổi Phone -> PhoneView (không ID); ghi ID vào outIds đúng thứ tự.
    return Result<Vector<PhoneView>>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::listForCustomer");
}

Result<Vector<PhoneStaffView>> PhoneService::listForStaff(const Actor&) const {
    // TODO(logic): kiểm tra quyền (Permission::isAllowed) rồi đổi Phone -> PhoneStaffView.
    return Result<Vector<PhoneStaffView>>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::listForStaff");
}

Result<Vector<PhoneView>> PhoneService::search(const std::string&, Vector<std::string>*) const {
    // TODO(logic): gọi PhoneRepository::findByName / findByBrand.
    return Result<Vector<PhoneView>>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::search");
}

Result<Vector<PhoneView>> PhoneService::filterByPrice(Money, Money, Vector<std::string>*) const {
    // TODO(logic): gọi PhoneRepository::findByPriceRange.
    return Result<Vector<PhoneView>>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::filterByPrice");
}

Result<void> PhoneService::addPhone(const Actor&, const Phone&) {
    // TODO(logic): kiểm tra quyền ManagePhone, hợp lệ dữ liệu, add + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::addPhone");
}

Result<void> PhoneService::updatePhone(const Actor&, const Phone&) {
    // TODO(logic): kiểm tra quyền ManagePhone, update + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::updatePhone");
}

Result<void> PhoneService::removePhone(const Actor&, const std::string&) {
    // TODO(logic): kiểm tra quyền và ràng buộc tham chiếu (đơn, hóa đơn, phiếu nhập) trước khi xóa.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::removePhone");
}

Result<void> PhoneService::checkAvailable(const std::string&, int) const {
    // TODO(logic): available = quantity - reservedQuantity >= qty, ngược lại OutOfStock.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::checkAvailable");
}

Result<void> PhoneService::reserve(UnitOfWork&, const std::string&, int) {
    // TODO(logic): enlist phoneRepo vào uow, reservedQuantity += qty (chỉ sửa RAM).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::reserve");
}

Result<void> PhoneService::releaseReserved(UnitOfWork&, const std::string&, int) {
    // TODO(logic): enlist, reservedQuantity -= qty.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::releaseReserved");
}

Result<void> PhoneService::commitReserved(UnitOfWork&, const std::string&, int) {
    // TODO(logic): enlist, reservedQuantity -= qty và quantity -= qty.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::commitReserved");
}

Result<void> PhoneService::restock(UnitOfWork&, const std::string&, int) {
    // TODO(logic): enlist, quantity += qty (hoàn kho khi hủy đơn CONFIRMED).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::restock");
}

Result<void> PhoneService::receiveStock(UnitOfWork&, const std::string&, int) {
    // TODO(logic): enlist, quantity += qty (nhập kho).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: PhoneService::receiveStock");
}
