#include "ImportReceiptRepository.h"

ImportReceiptRepository::ImportReceiptRepository(FileManager& fm)
    : Repository<ImportReceipt>(fm, "import_receipts.txt") {}

std::string ImportReceiptRepository::getId(const ImportReceipt& r) const { return r.getID(); }

std::string ImportReceiptRepository::serialize(const ImportReceipt& r) const { return r.toFileString(); }

Result<ImportReceipt> ImportReceiptRepository::deserialize(const std::string& line) const {
    try {
        ImportReceipt r = ImportReceipt::fromFileLine(line);
        if (r.getID().empty()) return Result<ImportReceipt>::fail(ErrorCode::ParseError, "Dòng phiếu nhập không hợp lệ");
        return Result<ImportReceipt>::ok(r);
    } catch (...) {
        return Result<ImportReceipt>::fail(ErrorCode::ParseError, "Không phân tích được dòng phiếu nhập");
    }
}

void ImportReceiptRepository::rebuildIndexes() {
    // Chưa có index riêng (index ID do lớp cơ sở dựng).
}
