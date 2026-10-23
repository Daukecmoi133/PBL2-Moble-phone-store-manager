#pragma once
// Giỏ hàng (lưu trong RAM theo khách) và thanh toán qua OrderService::createOrder. Toàn bộ hàm hiện là stub.
// CartService gọi Order, Phone, Discount; OrderService không biết Cart.

#include <string>

#include "DiscountService.h"
#include "OrderService.h"
#include "PhoneService.h"
#include "../common/Actor.h"
#include "../common/Money.h"
#include "../common/Result.h"
#include "../dto/CartLineView.h"
#include "../dto/CheckoutInfo.h"
#include "../model/sales/Cart.h"
#include "../storage/ds/Vector.h"

class CartService {
public:
    CartService(OrderService& orders, PhoneService& phones, DiscountService& discounts);

    // Nếu truyền outIds, Service ghi phoneId theo đúng thứ tự dòng để Controller dựng bảng ánh xạ vị trí -> ID.
    Result<Vector<CartLineView>> getCart(const Actor& actor, Vector<std::string>* outIds = nullptr) const;
    Result<void> addItem(const Actor& actor, const std::string& phoneId, int quantity);
    Result<void> removeItem(const Actor& actor, const std::string& phoneId);
    Result<void> updateQuantity(const Actor& actor, const std::string& phoneId, int newQuantity);
    Result<void> clear(const Actor& actor);
    Result<Money> calculateTotal(const Actor& actor, const std::string& discountCode = "") const;
    Result<std::string> checkout(const Actor& actor, const CheckoutInfo& info); // gọi OrderService::createOrder

private:
    OrderService& orderService;
    PhoneService& phoneService;
    DiscountService& discountService;
    Vector<Cart> carts; // TODO(logic): giỏ hàng theo khách (chưa lưu file; xem docs/OPEN_QUESTIONS.md)
};
