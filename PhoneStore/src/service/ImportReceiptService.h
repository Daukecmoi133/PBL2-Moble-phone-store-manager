#pragma once
// Nghiệp vụ phiếu nhập kho (gọi Supplier, Phone). Toàn bộ hàm hiện là stub.

#include <string>

#include "PhoneService.h"
#include "SupplierService.h"
#include "../common/Actor.h"
#include "../common/Result.h"
#include "../model/supply/ImportReceipt.h"
#include "../repository/ImportReceiptRepository.h"
#include "../storage/ds/Vector.h"

class ImportReceiptService {
public:
    ImportReceiptService(ImportReceiptRepository& receipts, SupplierService& suppliers, PhoneService& phones);

    Result<std::string> createReceipt(const Actor& actor, const ImportReceipt& receipt); // gọi PhoneService::receiveStock
    Result<ImportReceipt> getById(const Actor& actor, const std::string& receiptId) const;
    Result<Vector<ImportReceipt>> list(const Actor& actor) const;

private:
    ImportReceiptRepository& receiptRepo;
    SupplierService& supplierService;
    PhoneService& phoneService;
};
