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

Result<Vector<Phone>> PhoneRepository::findByName(const std::string& keyword) const {
    Vector<Phone> result;
    for (int i = 0; i < items.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(items[i].getName(), keyword)) {
            result.push_back(items[i]);
        }
    }
    return Result<Vector<Phone>>::ok(result);
}

Result<Vector<Phone>> PhoneRepository::findByBrand(const std::string& keyword) const {
    Vector<Phone> result;
    for (int i = 0; i < items.getSize(); ++i) {
        if (StringHelper::containsIgnoreCase(items[i].getBrand(), keyword)) {
            result.push_back(items[i]);
        }
    }
    return Result<Vector<Phone>>::ok(result);
}

Result<Vector<Phone>> PhoneRepository::findByPriceRange(Money minPrice, Money maxPrice) const {
    Vector<Phone> result;
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getSellingPrice() >= minPrice && items[i].getSellingPrice() <= maxPrice) {
            result.push_back(items[i]);
        }
    }
    return Result<Vector<Phone>>::ok(result);
}
