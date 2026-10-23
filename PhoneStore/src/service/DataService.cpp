#include "DataService.h"

DataService::DataService(DataRepository& data, PhoneRepository& phones, CustomerRepository& customers,
                         InvoiceRepository& invoices)
    : dataRepo(data), phoneRepo(phones), customerRepo(customers), invoiceRepo(invoices) {}

Result<std::string> DataService::backup(const Actor&) {
    // TODO(logic): quyền BackupRestore; save các Repository rồi dataRepo.createBackup().
    return Result<std::string>::fail(ErrorCode::NotImplemented, "TODO: DataService::backup");
}

Result<Vector<std::string>> DataService::listBackups(const Actor&) const {
    // TODO(logic): quyền BackupRestore; dataRepo.listBackups().
    return Result<Vector<std::string>>::fail(ErrorCode::NotImplemented, "TODO: DataService::listBackups");
}

Result<void> DataService::restore(const Actor&, const std::string&) {
    // TODO(logic): quyền BackupRestore; dataRepo.restoreBackup rồi load() lại mọi Repository.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: DataService::restore");
}

Result<ImportReport> DataService::importPhones(const Actor&, const std::string&) {
    // TODO(logic): quyền ImportExport; đọc dòng, kiểm tra từng dòng, add vào PhoneRepository, trả ImportReport.
    return Result<ImportReport>::fail(ErrorCode::NotImplemented, "TODO: DataService::importPhones");
}

Result<ImportReport> DataService::importCustomers(const Actor&, const std::string&) {
    // TODO(logic): quyền ImportExport; đọc dòng, kiểm tra từng dòng, add vào CustomerRepository, trả ImportReport.
    return Result<ImportReport>::fail(ErrorCode::NotImplemented, "TODO: DataService::importCustomers");
}

Result<void> DataService::exportInvoice(const Actor&, const std::string&, const std::string&) {
    // TODO(logic): quyền ImportExport; invoiceRepo.findById rồi dataRepo.exportLines.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: DataService::exportInvoice");
}
