#pragma once
// Repository nhà cung cấp: CRUD cơ bản (kế thừa từ Repository<Supplier>).

#include <string>

#include "Repository.h"
#include "../model/supply/Supplier.h"

class SupplierRepository : public Repository<Supplier> {
public:
    explicit SupplierRepository(FileManager& fm);

protected:
    std::string getId(const Supplier& s) const override;
    std::string serialize(const Supplier& s) const override;
    Result<Supplier> deserialize(const std::string& line) const override;
    void rebuildIndexes() override;
};
