#pragma once
// Điều phối giao dịch nhiều Repository (best effort, KHÔNG nguyên tử giữa các file).
// Việc phục hồi dựa trên snapshot trong RAM. Đối tượng UnitOfWork chưa commit mà bị hủy
// sẽ tự rollback (trả RAM về snapshot, không ghi file).

#include "IRepositoryTx.h"
#include "../common/Result.h"
#include "../storage/ds/Vector.h"

class UnitOfWork {
public:
    UnitOfWork() = default;
    ~UnitOfWork();
    UnitOfWork(const UnitOfWork&) = delete;
    UnitOfWork& operator=(const UnitOfWork&) = delete;

    void enlist(IRepositoryTx& repo);   // snapshot ngay khi enlist (mỗi repo một lần)
    Result<void> commit();              // flush từng repo; lỗi thì restore tất cả và ghi lại từ snapshot
    void rollback();                    // restore, không ghi file

private:
    Vector<IRepositoryTx*> repos;
};
