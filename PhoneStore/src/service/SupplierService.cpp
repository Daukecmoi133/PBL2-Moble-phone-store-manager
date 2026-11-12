#include "SupplierService.h"
#include "../common/Permission.h"

SupplierService::SupplierService(SupplierRepository& repo) : supplierRepo(repo) {}

Result<void> SupplierService::addSupplier(const Actor& actor, const Supplier& supplier) {
    if (!Permission::isAllowed(actor.role, Action::ManageSupplier)) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Không có quyền thêm nhà cung cấp");
    }
    auto addRes = supplierRepo.add(supplier);
    if (!addRes.isOk()) return addRes;
    return supplierRepo.save();
}

Result<void> SupplierService::updateSupplier(const Actor&, const Supplier&) {
    // TODO(logic): kiểm tra quyền ManageSupplier, update + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: SupplierService::updateSupplier");
}

Result<void> SupplierService::removeSupplier(const Actor&, const std::string&) {
    // TODO(logic): kiểm tra quyền và ràng buộc tham chiếu (phiếu nhập) trước khi xóa.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: SupplierService::removeSupplier");
}

Result<Vector<Supplier>> SupplierService::listAll(const Actor&) const {
    // TODO(logic): kiểm tra quyền rồi trả danh sách nhà cung cấp.
    return Result<Vector<Supplier>>::fail(ErrorCode::NotImplemented, "TODO: SupplierService::listAll");
}

Result<Vector<Supplier>> SupplierService::search(const Actor&, const std::string&) const {
    // TODO(logic): tìm theo tên công ty / mã số thuế / người liên hệ.
    return Result<Vector<Supplier>>::fail(ErrorCode::NotImplemented, "TODO: SupplierService::search");
}

Result<Supplier> SupplierService::getSupplierById(const Actor& actor, const std::string& supplierId) const {
    if (!Permission::isAllowed(actor.role, Action::CreateImportReceipt) &&
        !Permission::isAllowed(actor.role, Action::ManageSupplier)) {
        return Result<Supplier>::fail(ErrorCode::PermissionDenied, "Không có quyền xem thông tin nhà cung cấp");
    }
    return supplierRepo.findById(supplierId);
}
