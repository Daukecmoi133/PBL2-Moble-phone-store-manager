#include "SupplierRepository.h"

SupplierRepository::SupplierRepository(FileManager& fm) : Repository<Supplier>(fm, "suppliers.txt") {}

std::string SupplierRepository::getId(const Supplier& s) const { return s.getID(); }

std::string SupplierRepository::serialize(const Supplier& s) const { return s.toFileString(); }

Result<Supplier> SupplierRepository::deserialize(const std::string& line) const {
    try {
        Supplier s = Supplier::fromFileLine(line);
        if (s.getID().empty()) return Result<Supplier>::fail(ErrorCode::ParseError, "Dòng nhà cung cấp không hợp lệ");
        return Result<Supplier>::ok(s);
    } catch (...) {
        return Result<Supplier>::fail(ErrorCode::ParseError, "Không phân tích được dòng nhà cung cấp");
    }
}

void SupplierRepository::rebuildIndexes() {
    // Chưa có index riêng (index ID do lớp cơ sở dựng).
}
