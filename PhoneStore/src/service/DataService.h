#pragma once
// Sao lưu/khôi phục và nhập/xuất dữ liệu. Chỉ gọi Repository (DataRepository bọc BackupManager/ImportExport).
// Toàn bộ hàm hiện là stub.

#include <string>

#include "../common/Actor.h"
#include "../common/Result.h"
#include "../dto/ImportReport.h"
#include "../repository/CustomerRepository.h"
#include "../repository/DataRepository.h"
#include "../repository/InvoiceRepository.h"
#include "../repository/PhoneRepository.h"
#include "../storage/ds/Vector.h"

class DataService {
public:
    DataService(DataRepository& data, PhoneRepository& phones, CustomerRepository& customers,
                InvoiceRepository& invoices);

    Result<std::string> backup(const Actor& actor);
    Result<Vector<std::string>> listBackups(const Actor& actor) const;
    Result<void> restore(const Actor& actor, const std::string& backupName);
    Result<ImportReport> importPhones(const Actor& actor, const std::string& path);
    Result<ImportReport> importCustomers(const Actor& actor, const std::string& path);
    Result<void> exportInvoice(const Actor& actor, const std::string& invoiceId, const std::string& path);

private:
    DataRepository& dataRepo;
    PhoneRepository& phoneRepo;
    CustomerRepository& customerRepo;
    InvoiceRepository& invoiceRepo;
};
