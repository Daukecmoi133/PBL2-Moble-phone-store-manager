#pragma once
// Repository mã giảm giá: tra cứu theo mã voucher.

#include <string>

#include "Repository.h"
#include "../model/sales/DiscountCode.h"

class DiscountRepository : public Repository<DiscountCode> {
public:
    explicit DiscountRepository(FileManager& fm);

    Result<DiscountCode> findByCode(const std::string& code) const;

protected:
    std::string getId(const DiscountCode& d) const override;
    std::string serialize(const DiscountCode& d) const override;
    Result<DiscountCode> deserialize(const std::string& line) const override;
    void rebuildIndexes() override;
};
