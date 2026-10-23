#pragma once
// Repository hóa đơn: tra cứu theo ID đơn hàng.

#include <string>

#include "Repository.h"
#include "../model/sales/Invoice.h"

class InvoiceRepository : public Repository<Invoice> {
public:
    explicit InvoiceRepository(FileManager& fm);

    Result<Invoice> findByOrderId(const std::string& orderId) const;

protected:
    std::string getId(const Invoice& i) const override;
    std::string serialize(const Invoice& i) const override;
    Result<Invoice> deserialize(const std::string& line) const override;
    void rebuildIndexes() override;
};
