#pragma once
// Nghiệp vụ mã giảm giá. Toàn bộ hàm hiện là stub.
// Điểm mở: tính lượt dùng khi tạo đơn hay khi xác nhận (mặc định tạm: khi tạo đơn, hoàn lượt khi hủy).

#include <string>

#include "../common/Actor.h"
#include "../common/Money.h"
#include "../common/Result.h"
#include "../model/sales/DiscountCode.h"
#include "../repository/DiscountRepository.h"
#include "../repository/UnitOfWork.h"
#include "../storage/ds/Vector.h"

class DiscountService {
public:
    explicit DiscountService(DiscountRepository& repo);

    Result<void> addDiscount(const Actor& actor, const DiscountCode& discount);
    Result<void> updateDiscount(const Actor& actor, const DiscountCode& discount);
    Result<void> removeDiscount(const Actor& actor, const std::string& code);
    Result<Vector<DiscountCode>> listAll(const Actor& actor) const;

    Result<Money> validateAndCalculate(const std::string& code, Money orderTotal) const;
    Result<void> consumeUsage(UnitOfWork& uow, const std::string& code);  // chỉ sửa RAM, người gọi commit
    Result<void> releaseUsage(UnitOfWork& uow, const std::string& code);  // hoàn lượt dùng

private:
    DiscountRepository& discountRepo;
};
