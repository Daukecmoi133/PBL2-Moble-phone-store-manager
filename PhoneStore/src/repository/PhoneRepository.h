#pragma once
// Repository điện thoại: giữ dữ liệu chính + index (ID: HashTable ở lớp cơ sở; giá: BST).
// Mọi hàm tra cứu mở rộng hiện là stub; serialize/deserialize bọc code cũ (Phone::toFileString/fromFileLine).

#include <string>

#include "Repository.h"
#include "../model/catalog/Phone.h"
#include "../common/Money.h"
#include "../storage/ds/BST.h"

class PhoneRepository : public Repository<Phone> {
public:
    explicit PhoneRepository(FileManager& fm);

    Result<Vector<Phone>> findByName(const std::string& keyword) const;
    Result<Vector<Phone>> findByBrand(const std::string& keyword) const;
    Result<Vector<Phone>> findByPriceRange(Money minPrice, Money maxPrice) const; // dùng BST theo giá

protected:
    std::string getId(const Phone& p) const override;
    std::string serialize(const Phone& p) const override;
    Result<Phone> deserialize(const std::string& line) const override;
    void rebuildIndexes() override;

private:
    // Index theo giá: chỉ lưu vị trí trong items (không lưu bản sao Phone).
    // TODO(logic): BST::insert ghi đè khi trùng khóa nên cần cách xử lý các điện thoại cùng giá (xem OPEN_QUESTIONS).
    BST<long long, int> priceIndex;
};
