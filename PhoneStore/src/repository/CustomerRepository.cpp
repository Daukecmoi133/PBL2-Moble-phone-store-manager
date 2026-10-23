#include "CustomerRepository.h"

CustomerRepository::CustomerRepository(FileManager& fm) : Repository<Customer>(fm, "customers.txt") {}

std::string CustomerRepository::getId(const Customer& c) const { return c.getID(); }

std::string CustomerRepository::serialize(const Customer& c) const { return c.toFileString(); }

Result<Customer> CustomerRepository::deserialize(const std::string& line) const {
    try {
        Customer c = Customer::fromFileLine(line);
        if (c.getID().empty()) return Result<Customer>::fail(ErrorCode::ParseError, "Dòng khách hàng không hợp lệ");
        return Result<Customer>::ok(c);
    } catch (...) {
        return Result<Customer>::fail(ErrorCode::ParseError, "Không phân tích được dòng khách hàng");
    }
}

void CustomerRepository::rebuildIndexes() {
    // TODO(logic): dựng usernameIndex và phoneIndex (khóa -> vị trí trong items).
}

Result<Customer> CustomerRepository::findByUsername(const std::string&) const {
    // TODO(logic): tra usernameIndex.
    return Result<Customer>::fail(ErrorCode::NotImplemented, "TODO: CustomerRepository::findByUsername");
}

Result<Customer> CustomerRepository::findByPhone(const std::string&) const {
    // TODO(logic): tra phoneIndex.
    return Result<Customer>::fail(ErrorCode::NotImplemented, "TODO: CustomerRepository::findByPhone");
}
