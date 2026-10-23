#pragma once
// Nghiệp vụ nhà cung cấp. Toàn bộ hàm hiện là stub.

#include <string>

#include "../common/Actor.h"
#include "../common/Result.h"
#include "../model/supply/Supplier.h"
#include "../repository/SupplierRepository.h"
#include "../storage/ds/Vector.h"

class SupplierService {
public:
    explicit SupplierService(SupplierRepository& repo);

    Result<void> addSupplier(const Actor& actor, const Supplier& supplier);
    Result<void> updateSupplier(const Actor& actor, const Supplier& supplier);
    Result<void> removeSupplier(const Actor& actor, const std::string& supplierId);
    Result<Vector<Supplier>> listAll(const Actor& actor) const;
    Result<Vector<Supplier>> search(const Actor& actor, const std::string& keyword) const;
    Result<Supplier> getSupplierById(const Actor& actor, const std::string& supplierId) const;

private:
    SupplierRepository& supplierRepo;
};
