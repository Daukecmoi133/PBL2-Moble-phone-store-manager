#include "InvoiceRepository.h"

InvoiceRepository::InvoiceRepository(FileManager& fm) : Repository<Invoice>(fm, "invoices.txt") {}

std::string InvoiceRepository::getId(const Invoice& i) const { return i.getID(); }

std::string InvoiceRepository::serialize(const Invoice& i) const { return i.toFileString(); }

Result<Invoice> InvoiceRepository::deserialize(const std::string& line) const {
    try {
        Invoice inv = Invoice::fromFileLine(line);
        if (inv.getID().empty()) return Result<Invoice>::fail(ErrorCode::ParseError, "Dòng hóa đơn không hợp lệ");
        return Result<Invoice>::ok(inv);
    } catch (...) {
        return Result<Invoice>::fail(ErrorCode::ParseError, "Không phân tích được dòng hóa đơn");
    }
}

void InvoiceRepository::rebuildIndexes() {
    // TODO(logic): dựng index orderId -> vị trí trong items.
}

Result<Invoice> InvoiceRepository::findByOrderId(const std::string&) const {
    // TODO(logic): tra index theo orderId.
    return Result<Invoice>::fail(ErrorCode::NotImplemented, "TODO: InvoiceRepository::findByOrderId");
}
