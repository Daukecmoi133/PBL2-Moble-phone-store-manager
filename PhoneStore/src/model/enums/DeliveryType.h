#pragma once
// Hình thức nhận hàng. Chuyển từ `DeliveryMethod` trong models/Common.h cũ.

#include <string>

enum class DeliveryType {
    PICKUP,   // Nhận tại cửa hàng
    DELIVERY  // Giao hàng tận nơi
};

// Tên cũ của enum, giữ lại để code cũ (Order) vẫn biên dịch
using DeliveryMethod = DeliveryType;

// Chuyển về phương thức nhận hàng với dịch nghĩa tiếng Việt
inline std::string deliveryMethodToString(DeliveryType method) {
    switch (method) {
        case DeliveryType::PICKUP: return "Nhận tại cửa hàng";
        case DeliveryType::DELIVERY: return "Giao tận nơi";
        default: return "Không rõ";
    }
}
