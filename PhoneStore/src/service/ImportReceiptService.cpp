#include "ImportReceiptService.h"

ImportReceiptService::ImportReceiptService(ImportReceiptRepository& receipts, SupplierService& suppliers,
                                           PhoneService& phones)
    : receiptRepo(receipts), supplierService(suppliers), phoneService(phones) {}

Result<std::string> ImportReceiptService::createReceipt(const Actor&, const ImportReceipt&) {
    // TODO(logic): quyền CreateImportReceipt; xác minh nhà cung cấp; UnitOfWork: receiveStock từng dòng + add phiếu; commit.
    return Result<std::string>::fail(ErrorCode::NotImplemented, "TODO: ImportReceiptService::createReceipt");
}

Result<ImportReceipt> ImportReceiptService::getById(const Actor&, const std::string&) const {
    // TODO(logic): kiểm tra quyền rồi receiptRepo.findById.
    return Result<ImportReceipt>::fail(ErrorCode::NotImplemented, "TODO: ImportReceiptService::getById");
}

Result<Vector<ImportReceipt>> ImportReceiptService::list(const Actor&) const {
    // TODO(logic): kiểm tra quyền rồi trả danh sách phiếu nhập.
    return Result<Vector<ImportReceipt>>::fail(ErrorCode::NotImplemented, "TODO: ImportReceiptService::list");
}
