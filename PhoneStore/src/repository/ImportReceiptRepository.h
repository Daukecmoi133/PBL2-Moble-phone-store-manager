#pragma once
// Repository phiếu nhập kho: CRUD cơ bản (kế thừa từ Repository<ImportReceipt>).

#include <string>

#include "Repository.h"
#include "../model/supply/ImportReceipt.h"

class ImportReceiptRepository : public Repository<ImportReceipt> {
public:
    explicit ImportReceiptRepository(FileManager& fm);

protected:
    std::string getId(const ImportReceipt& r) const override;
    std::string serialize(const ImportReceipt& r) const override;
    Result<ImportReceipt> deserialize(const std::string& line) const override;
    void rebuildIndexes() override;
};
