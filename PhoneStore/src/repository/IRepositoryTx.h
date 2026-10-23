#pragma once
// Giao diện để UnitOfWork điều phối nhiều Repository trong một giao dịch (best effort).

#include "../common/Result.h"

class IRepositoryTx {
public:
    virtual void snapshot() = 0;       // chụp dữ liệu trong RAM
    virtual Result<void> flush() = 0;  // ghi ra file
    virtual void restore() = 0;        // trả RAM về snapshot (và dựng lại index)
    virtual ~IRepositoryTx() = default;
};
