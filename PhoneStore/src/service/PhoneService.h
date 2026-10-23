#pragma once
// Nghiệp vụ điện thoại và tồn kho. Toàn bộ hàm hiện là stub (chưa có logic).
// Các hàm tồn kho (reserve/release/commit/restock/receive) chỉ sửa RAM và enlist vào UnitOfWork;
// NGƯỜI GỌI chịu trách nhiệm commit.

#include <string>

#include "../common/Actor.h"
#include "../common/Money.h"
#include "../common/Result.h"
#include "../dto/PhoneStaffView.h"
#include "../dto/PhoneView.h"
#include "../model/catalog/Phone.h"
#include "../repository/PhoneRepository.h"
#include "../repository/UnitOfWork.h"
#include "../storage/ds/Vector.h"

class PhoneService {
public:
    explicit PhoneService(PhoneRepository& repo);

    // Danh sách cho khách: KHÔNG có ID nội bộ. Nếu truyền outIds, Service ghi ID tương ứng theo đúng thứ tự
    // hiển thị để Controller dựng bảng ánh xạ "vị trí hiển thị -> ID" (xem docs/OPEN_QUESTIONS.md).
    Result<Vector<PhoneView>> listForCustomer(Vector<std::string>* outIds = nullptr) const;
    Result<Vector<PhoneStaffView>> listForStaff(const Actor& actor) const;
    Result<Vector<PhoneView>> search(const std::string& keyword, Vector<std::string>* outIds = nullptr) const;
    Result<Vector<PhoneView>> filterByPrice(Money minPrice, Money maxPrice, Vector<std::string>* outIds = nullptr) const;

    Result<void> addPhone(const Actor& actor, const Phone& phone);
    Result<void> updatePhone(const Actor& actor, const Phone& phone);
    Result<void> removePhone(const Actor& actor, const std::string& id);

    // available = quantity - reservedQuantity
    Result<void> checkAvailable(const std::string& id, int qty) const;
    Result<void> reserve(UnitOfWork& uow, const std::string& id, int qty);          // giữ chỗ
    Result<void> releaseReserved(UnitOfWork& uow, const std::string& id, int qty);  // giải phóng giữ chỗ
    Result<void> commitReserved(UnitOfWork& uow, const std::string& id, int qty);   // giữ chỗ -> xuất kho thật
    Result<void> restock(UnitOfWork& uow, const std::string& id, int qty);          // hoàn kho khi hủy đơn đã xác nhận
    Result<void> receiveStock(UnitOfWork& uow, const std::string& id, int qty);     // nhập kho

private:
    PhoneRepository& phoneRepo;
};
