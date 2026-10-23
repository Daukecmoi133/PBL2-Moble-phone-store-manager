#include "OrderStateMachine.h"

namespace {

// Một dòng của bảng chuyển trạng thái (đặc tả mục 7)
struct Transition {
    OrderStatus from;
    OrderStatus to;
    bool pickup;       // áp dụng cho luồng PICKUP
    bool delivery;     // áp dụng cho luồng DELIVERY
    bool customer;     // vai trò được phép
    bool sales;
    bool warehouse;
    bool admin;
};

//                                   from                         to                            pickup delivery cust   sales  wh     admin
const Transition TRANSITIONS[] = {
    { OrderStatus::PENDING,          OrderStatus::CONFIRMED,        true,  true,  false, true,  false, true },
    { OrderStatus::PENDING,          OrderStatus::CANCELLED,        true,  true,  true,  true,  false, true },
    { OrderStatus::CONFIRMED,        OrderStatus::PREPARING,        true,  true,  false, false, true,  true },
    { OrderStatus::CONFIRMED,        OrderStatus::CANCELLED,        true,  true,  false, true,  false, true },
    { OrderStatus::PREPARING,        OrderStatus::READY_FOR_PICKUP, true,  false, false, false, true,  true },
    { OrderStatus::PREPARING,        OrderStatus::SHIPPING,         false, true,  false, false, true,  true },
    { OrderStatus::READY_FOR_PICKUP, OrderStatus::COMPLETED,        true,  false, false, true,  false, true },
    { OrderStatus::SHIPPING,         OrderStatus::COMPLETED,        false, true,  false, true,  false, true },
};

bool roleAllowed(const Transition& t, Role role) {
    switch (role) {
        case Role::CUSTOMER: return t.customer;
        case Role::SALE_EMPLOYEE: return t.sales;
        case Role::WAREHOUSE_EMPLOYEE: return t.warehouse;
        case Role::ADMIN: return t.admin;
    }
    return false;
}

} // namespace

// Từ PREPARING trở đi không cho hủy (V1): bảng không có dòng nào đi tới CANCELLED từ PREPARING/READY/SHIPPING.
bool OrderStateMachine::canTransition(DeliveryType delivery, OrderStatus from, OrderStatus to, Role actor) {
    for (const Transition& t : TRANSITIONS) {
        if (t.from != from || t.to != to) continue;
        bool flowOk = (delivery == DeliveryType::PICKUP) ? t.pickup : t.delivery;
        return flowOk && roleAllowed(t, actor);
    }
    return false;
}
