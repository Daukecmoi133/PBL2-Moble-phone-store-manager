#include "SupplierService.h"

SupplierService::SupplierService(SupplierRepository& repo) : supplierRepo(repo) {}

Result<void> SupplierService::addSupplier(const Actor&, const Supplier&) {
    // TODO(logic): kiểm tra quyền ManageSupplier, hợp lệ dữ liệu, add + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: SupplierService::addSupplier");
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

Result<Supplier> SupplierService::getSupplierById(const Actor&, const std::string&) const {
    // TODO(logic): kiểm tra quyền rồi findById (ImportReceiptService dùng để xác minh nhà cung cấp).
    return Result<Supplier>::fail(ErrorCode::NotImplemented, "TODO: SupplierService::getSupplierById");
}
