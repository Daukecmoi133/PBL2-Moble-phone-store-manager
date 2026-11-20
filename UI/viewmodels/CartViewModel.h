#ifndef CART_VIEWMODEL_H
#define CART_VIEWMODEL_H

#include <string>
#include <vector>
#include "../../source/utils/PricingCalculator.h"

// Chi tiết một mặt hàng trong giỏ hàng
struct CartItemViewModel {
    std::string phoneId;
    std::string phoneName;
    std::string brand;
    double unitPrice;
    int quantity;
    int availableStock; // Để UI validate không bấm quá tồn kho khả dụng
    double subtotal;

    CartItemViewModel() : unitPrice(0.0), quantity(0), availableStock(0), subtotal(0.0) {}

    void calculateSubtotal() {
        subtotal = unitPrice * quantity;
    }
};

// Tổng hợp giỏ hàng và thanh toán
struct CartSummaryViewModel {
    std::string customerId;
    std::vector<CartItemViewModel> items;
    int totalQuantity;
    double subtotal;
    std::string appliedDiscountCode;
    double discountAmount;
    double taxableAmount;
    double vatRate;
    double vatAmount;
    double grandTotal;

    CartSummaryViewModel()
        : totalQuantity(0), subtotal(0.0), discountAmount(0.0),
          taxableAmount(0.0), vatRate(PricingCalculator::DEFAULT_VAT_RATE),
          vatAmount(0.0), grandTotal(0.0) {}

    void recalculate(double overrideVatRate = -1.0) {
        if (overrideVatRate >= 0.0) {
            vatRate = PricingCalculator::sanitizeVatRate(overrideVatRate);
        }
        totalQuantity = 0;
        subtotal = 0.0;
        for (auto& item : items) {
            item.calculateSubtotal();
            totalQuantity += item.quantity;
            subtotal += item.subtotal;
        }
        const PricingBreakdown pb = PricingCalculator::computeFromDiscountAmount(
            subtotal, discountAmount, vatRate);
        subtotal = pb.subtotal;
        discountAmount = pb.discountAmount;
        taxableAmount = pb.taxableAmount;
        vatAmount = pb.vatAmount;
        grandTotal = pb.grandTotal;
    }
};

// Request thực hiện Checkout
struct CheckoutRequest {
    std::string customerId;
    std::string receiverName;
    std::string receiverPhone;
    std::string deliveryAddress;
    int deliveryMethod; // 0: Pickup (Nhận tại shop), 1: Delivery (Giao tận nơi)
    int paymentMethod;  // 0: Cash (Tiền mặt), 1: Bank Transfer (Chuyển khoản)
    std::string discountCode;
    std::vector<std::string> selectedPhoneIds; // Danh sách ID sản phẩm được tick chọn (rỗng = chọn tất cả)
};

#endif // CART_VIEWMODEL_H
