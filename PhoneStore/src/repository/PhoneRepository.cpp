#include "PhoneRepository.h"

PhoneRepository::PhoneRepository(FileManager& fm) : Repository<Phone>(fm, "phones.txt") {}

std::string PhoneRepository::getId(const Phone& p) const { return p.getID(); }

std::string PhoneRepository::serialize(const Phone& p) const { return p.toFileString(); }

Result<Phone> PhoneRepository::deserialize(const std::string& line) const {
    try {
        Phone p = Phone::fromFileLine(line);
        if (p.getID().empty()) return Result<Phone>::fail(ErrorCode::ParseError, "Dòng điện thoại không hợp lệ");
        return Result<Phone>::ok(p);
    } catch (...) {
        return Result<Phone>::fail(ErrorCode::ParseError, "Không phân tích được dòng điện thoại");
    }
}

void PhoneRepository::rebuildIndexes() {
    // TODO(logic): dựng lại priceIndex (giá bán -> vị trí) sau mỗi thay đổi dữ liệu.
}

Result<Vector<Phone>> PhoneRepository::findByName(const std::string&) const {
    // TODO(logic): duyệt items, so khớp tên không phân biệt hoa thường.
    return Result<Vector<Phone>>::fail(ErrorCode::NotImplemented, "TODO: PhoneRepository::findByName");
}

Result<Vector<Phone>> PhoneRepository::findByBrand(const std::string&) const {
    // TODO(logic): lọc theo hãng, không phân biệt hoa thường.
    return Result<Vector<Phone>>::fail(ErrorCode::NotImplemented, "TODO: PhoneRepository::findByBrand");
}

Result<Vector<Phone>> PhoneRepository::findByPriceRange(Money, Money) const {
    // TODO(logic): dùng priceIndex.findRange(min, max) rồi tra lại items theo vị trí.
    return Result<Vector<Phone>>::fail(ErrorCode::NotImplemented, "TODO: PhoneRepository::findByPriceRange");
}
