#pragma once
// Tóm tắt một đơn hàng để hiển thị trong danh sách.

#include <string>

#include "../common/Money.h"
#include "../model/enums/DeliveryType.h"
#include "../model/enums/OrderStatus.h"
#include "../model/enums/PaymentStatus.h"

struct OrderSummary {
    std::string orderId;
    std::string customerId;
    OrderStatus status = OrderStatus::PENDING;
    PaymentStatus paymentStatus = PaymentStatus::UNPAID;
    DeliveryType deliveryType = DeliveryType::PICKUP;
    Money totalAmount = 0;
    std::string createdTime;   // chuỗi đã định dạng
    int itemCount = 0;         // tổng số lượng sản phẩm
};
