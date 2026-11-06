#ifndef ORDER_VIEWMODEL_H
#define ORDER_VIEWMODEL_H

#include <string>
#include <vector>
#include "../../source/models/Common.h"

/**
 * @file OrderViewModel.h
 * @brief ĐẶC TẢ DỮ LIỆU ĐƠN HÀNG & THANH TOÁN (ORDER & PAYMENT VIEWMODELS)
 * 
 * =========================================================================================
 * - OrderDetailItemViewModel: Thông tin chi tiết từng sản phẩm trong đơn.
 * - PaymentStatus: Trạng thái thanh toán (UNPAID / PAID) độc lập với trạng thái luân chuyển hàng.
 * - OrderItemViewModel: Mô hình đơn hàng đầy đủ phục vụ hiển thị trên bảng quản lý và theo dõi.
 * =========================================================================================
 */

// Chi tiết mặt hàng trong đơn
struct OrderDetailItemViewModel {
    std::string phoneId;
    std::string phoneName;
    int quantity;
    double unitPrice;
    double subtotal;

    OrderDetailItemViewModel() : quantity(0), unitPrice(0.0), subtotal(0.0) {}
};

// Trạng thái thanh toán (độc lập với trạng thái đơn hàng theo spec mục 7)
enum class PaymentStatus {
    UNPAID,
    PAID
};

inline std::string paymentStatusToString(PaymentStatus status) {
    switch (status) {
        case PaymentStatus::UNPAID: return "Chưa thanh toán";
        case PaymentStatus::PAID: return "Đã thanh toán";
        default: return "Không rõ";
    }
}

// ViewModel hiển thị một đơn hàng
struct OrderItemViewModel {
    std::string id;
    std::string customerId;
    std::string customerName;
    std::string orderTime;
    OrderStatus status;
    std::string statusText;
    DeliveryMethod deliveryMethod;
    std::string deliveryMethodText;
    PaymentMethod paymentMethod;
    std::string paymentMethodText;
    PaymentStatus paymentStatus;
    std::string paymentStatusText;
    double subtotal;
    double discountAmount;
    double total;
    std::string receiverName;
    std::string receiverPhone;
    std::string deliveryAddress;
    std::vector<OrderDetailItemViewModel> details;

    OrderItemViewModel()
        : status(OrderStatus::PENDING), deliveryMethod(DeliveryMethod::PICKUP),
          paymentMethod(PaymentMethod::CASH), paymentStatus(PaymentStatus::UNPAID),
          subtotal(0.0), discountAmount(0.0), total(0.0) {}

    bool canCancel() const {
        return status == OrderStatus::PENDING;
    }
};

#endif // ORDER_VIEWMODEL_H
