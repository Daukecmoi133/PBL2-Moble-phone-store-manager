#pragma once
// Bản nháp đơn hàng do Controller/CartService gửi xuống OrderService.

#include <string>

#include "../common/Money.h"
#include "../model/enums/DeliveryType.h"
#include "../model/enums/PaymentMethod.h"
#include "../storage/ds/Vector.h"

// Một dòng hàng trong đơn nháp (ID nội bộ chỉ tồn tại ở tầng Controller trở xuống)
struct OrderDraftLine {
    std::string phoneId;
    int quantity = 0;
};

struct OrderDraft {
    Vector<OrderDraftLine> lines;
    DeliveryType deliveryType = DeliveryType::PICKUP;
    PaymentMethod paymentMethod = PaymentMethod::CASH;
    std::string receiverName;
    std::string receiverPhone;
    std::string address;
    std::string discountCode;  // rỗng nếu không dùng
};
