#include "UnitOfWork.h"

UnitOfWork::~UnitOfWork() {
    rollback(); // chưa commit thì trả RAM về như cũ; đã commit thì danh sách rỗng, không làm gì
}

void UnitOfWork::enlist(IRepositoryTx& repo) {
    for (int i = 0; i < repos.getSize(); ++i) {
        if (repos[i] == &repo) return; // mỗi repo chỉ snapshot một lần
    }
    repo.snapshot();
    repos.push_back(&repo);
}

Result<void> UnitOfWork::commit() {
    for (int i = 0; i < repos.getSize(); ++i) {
        Result<void> r = repos[i]->flush();
        if (!r.isOk()) {
            // Lỗi: trả tất cả về snapshot rồi ghi lại file từ snapshot (best effort)
            for (int j = 0; j < repos.getSize(); ++j) repos[j]->restore();
            for (int j = 0; j < repos.getSize(); ++j) repos[j]->flush();
            repos.clear();
            return r;
        }
    }
    repos.clear();
    return Result<void>::ok();
}

void UnitOfWork::rollback() {
    for (int i = 0; i < repos.getSize(); ++i) repos[i]->restore();
    repos.clear();
}
