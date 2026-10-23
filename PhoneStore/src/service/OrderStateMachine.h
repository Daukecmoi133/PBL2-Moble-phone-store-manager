#pragma once
// Máy trạng thái đơn hàng: thuần logic, KHÔNG phụ thuộc Repository.
// Bảng chuyển trạng thái (đặc tả mục 7) được mã hóa thành bảng hằng trong OrderStateMachine.cpp.
// Lưu ý: điều kiện "COMPLETED yêu cầu PAID" và "Customer chỉ hủy đơn của mình" thuộc OrderService, không nằm ở đây.

#include "../model/enums/DeliveryType.h"
#include "../model/enums/OrderStatus.h"
#include "../model/enums/Role.h"

class OrderStateMachine {
public:
    static bool canTransition(DeliveryType delivery, OrderStatus from, OrderStatus to, Role actor);
};
