#pragma once
// Thông tin khách nhập khi thanh toán giỏ hàng (CartService::checkout).

#include <string>

#include "../model/enums/DeliveryType.h"
#include "../model/enums/PaymentMethod.h"

struct CheckoutInfo {
    DeliveryType deliveryType = DeliveryType::PICKUP;
    PaymentMethod paymentMethod = PaymentMethod::CASH;
    std::string receiverName;
    std::string receiverPhone;
    std::string address;
    std::string discountCode;  // rỗng nếu không dùng
};
