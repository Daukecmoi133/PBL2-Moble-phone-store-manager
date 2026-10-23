#include "CartService.h"

CartService::CartService(OrderService& orders, PhoneService& phones, DiscountService& discounts)
    : orderService(orders), phoneService(phones), discountService(discounts) {}

Result<Vector<CartLineView>> CartService::getCart(const Actor&, Vector<std::string>*) const {
    // TODO(logic): tìm giỏ của khách, đổi sang CartLineView (không ID), ghi phoneId vào outIds.
    return Result<Vector<CartLineView>>::fail(ErrorCode::NotImplemented, "TODO: CartService::getCart");
}

Result<void> CartService::addItem(const Actor&, const std::string&, int) {
    // TODO(logic): phoneService.checkAvailable rồi cộng dồn vào giỏ.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: CartService::addItem");
}

Result<void> CartService::removeItem(const Actor&, const std::string&) {
    // TODO(logic): xóa dòng khỏi giỏ.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: CartService::removeItem");
}

Result<void> CartService::updateQuantity(const Actor&, const std::string&, int) {
    // TODO(logic): kiểm tra tồn kho khả dụng, cập nhật số lượng (<= 0 thì xóa dòng).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: CartService::updateQuantity");
}

Result<void> CartService::clear(const Actor&) {
    // TODO(logic): dọn sạch giỏ của khách.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: CartService::clear");
}

Result<Money> CartService::calculateTotal(const Actor&, const std::string&) const {
    // TODO(logic): tổng giỏ, trừ discountService.validateAndCalculate nếu có mã.
    return Result<Money>::fail(ErrorCode::NotImplemented, "TODO: CartService::calculateTotal");
}

Result<std::string> CartService::checkout(const Actor&, const CheckoutInfo&) {
    // TODO(logic): đổi giỏ + CheckoutInfo thành OrderDraft, gọi orderService.createOrder, xóa giỏ khi thành công.
    return Result<std::string>::fail(ErrorCode::NotImplemented, "TODO: CartService::checkout");
}
