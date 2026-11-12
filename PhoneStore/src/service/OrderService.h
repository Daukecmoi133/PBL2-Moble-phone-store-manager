#pragma once
// Nghiệp vụ đơn hàng (máy trạng thái, tồn kho, thanh toán). Toàn bộ hàm hiện là stub.
// OrderService không biết Cart. Chỉ OrderService được đổi Order::inventoryState.

#include <string>

#include "CustomerService.h"
#include "DiscountService.h"
#include "PhoneService.h"
#include "../common/Actor.h"
#include "../common/Result.h"
#include "../dto/OrderDraft.h"
#include "../dto/OrderSummary.h"
#include "../model/enums/OrderStatus.h"
#include "../model/enums/PaymentMethod.h"
#include "../repository/OrderRepository.h"
#include "../storage/ds/Vector.h"

class OrderService {
public:
    OrderService(OrderRepository& orders, PhoneService& phones, CustomerService& customers,
                 DiscountService& discounts);

    Result<std::string> createOrder(const Actor& actor, const OrderDraft& draft);  // kiểm tra available, reserve, lưu PENDING
    Result<void> confirmOrder(const Actor& actor, const std::string& orderId);     // PENDING -> CONFIRMED, commitReserved
    Result<void> startPreparing(const Actor& actor, const std::string& orderId);   // CONFIRMED -> PREPARING
    Result<void> markReadyForPickup(const Actor& actor, const std::string& orderId); // PREPARING -> READY_FOR_PICKUP
    Result<void> markShipping(const Actor& actor, const std::string& orderId);     // PREPARING -> SHIPPING
    Result<void> takePayment(const Actor& actor, const std::string& orderId, PaymentMethod method); // UNPAID -> PAID
    Result<void> completeOrder(const Actor& actor, const std::string& orderId);    // yêu cầu PAID
    Result<void> confirmReceived(const Actor& actor, const std::string& orderId); // khách hàng xác nhận đã nhận hàng
    Result<void> cancelOrder(const Actor& actor, const std::string& orderId);

    // Hủy các đơn hàng PENDING quá hạn TTL, nhả hàng giữ chỗ
    int cancelExpiredPending();

    // Bán tại quầy: tạo đơn, thu tiền và lập hóa đơn trong một UnitOfWork
    Result<std::string> createCounterSale(const Actor& actor, const OrderDraft& draft, PaymentMethod method);

    Result<Vector<OrderSummary>> listForCustomer(const Actor& actor) const;
    Result<Vector<OrderSummary>> listByStatus(const Actor& actor, OrderStatus status) const;
    Result<OrderSummary> getOrder(const Actor& actor, const std::string& orderId) const;

private:
    OrderRepository& orderRepo;
    PhoneService& phoneService;
    CustomerService& customerService;
    DiscountService& discountService;
};
