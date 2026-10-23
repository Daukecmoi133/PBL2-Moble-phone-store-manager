#pragma once
// Repository khách hàng: tra cứu theo username / số điện thoại (HashTable index, chưa dựng).

#include <string>

#include "Repository.h"
#include "../model/people/Customer.h"

class CustomerRepository : public Repository<Customer> {
public:
    explicit CustomerRepository(FileManager& fm);

    Result<Customer> findByUsername(const std::string& username) const;
    Result<Customer> findByPhone(const std::string& phone) const;

protected:
    std::string getId(const Customer& c) const override;
    std::string serialize(const Customer& c) const override;
    Result<Customer> deserialize(const std::string& line) const override;
    void rebuildIndexes() override;

private:
    // Index chỉ lưu vị trí trong items. TODO(logic): dựng trong rebuildIndexes().
    HashTable<std::string, int> usernameIndex;
    HashTable<std::string, int> phoneIndex;
};
