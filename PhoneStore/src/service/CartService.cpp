#include "CartService.h"
#include "../common/Config.h"
#include "../common/Permission.h"
#include <algorithm>

CartService::CartService(OrderService& orders, PhoneService& phones, DiscountService& discounts)
    : orderService(orders), phoneService(phones), discountService(discounts) {}

Cart* CartService::findCart(const std::string& customerId) {
    for (int i = 0; i < carts.getSize(); ++i) {
        if (carts[i].getCustomerId() == customerId) {
            return &carts[i];
        }
    }
    return nullptr;
}

const Cart* CartService::findCart(const std::string& customerId) const {
    for (int i = 0; i < carts.getSize(); ++i) {
        if (carts[i].getCustomerId() == customerId) {
            return &carts[i];
        }
    }
    return nullptr;
}

Cart& CartService::getOrCreateCart(const std::string& customerId) {
    Cart* c = findCart(customerId);
    if (c != nullptr) return *c;
    carts.push_back(Cart(customerId));
    return carts[carts.getSize() - 1];
}

Result<Vector<CartLineView>> CartService::getCart(const Actor& actor, Vector<std::string>* outIds) const {
    if (actor.role != Role::CUSTOMER) {
        return Result<Vector<CartLineView>>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }
    if (outIds) outIds->clear();

    const Cart* cart = findCart(actor.userId);
    Vector<CartLineView> views;
    if (!cart) return Result<Vector<CartLineView>>::ok(views);

    const Vector<CartItem>& items = cart->getItems();
    for (int i = 0; i < items.getSize(); ++i) {
        const CartItem& item = items[i];
        std::string phoneId = item.getPhone().getID();

        CartLineView lv;
        lv.selected = item.isSelected();
        lv.quantity = item.getQuantity();

        auto phoneRes = phoneService.getPhoneById(phoneId);
        if (phoneRes.isOk()) {
            const Phone& p = phoneRes.value();
            lv.phoneName = p.getName();
            lv.unitPrice = static_cast<Money>(p.getSellingPrice());
            lv.subtotal = lv.unitPrice * lv.quantity;
            lv.available = p.isListed() && (p.availableQuantity() >= item.getQuantity());
        } else {
            lv.phoneName = item.getPhone().getName();
            lv.unitPrice = static_cast<Money>(item.getUnitPrice());
            lv.subtotal = lv.unitPrice * lv.quantity;
            lv.available = false;
        }

        views.push_back(lv);
        if (outIds) outIds->push_back(phoneId);
    }
    return Result<Vector<CartLineView>>::ok(views);
}

Result<void> CartService::addItem(const Actor& actor, const std::string& phoneId, int quantity) {
    if (actor.role != Role::CUSTOMER) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }
    if (quantity <= 0) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng thêm vào giỏ phải lớn hơn 0");
    }

    Cart& cart = getOrCreateCart(actor.userId);
    int currentInCart = 0;
    const Vector<CartItem>& items = cart.getItems();
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getPhone().getID() == phoneId) {
            currentInCart = items[i].getQuantity();
            break;
        }
    }

    int targetQty = currentInCart + quantity;
    if (targetQty > Config::MAX_QUANTITY_PER_LINE) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng vượt quá giới hạn tối đa " +
            std::to_string(Config::MAX_QUANTITY_PER_LINE) + " sản phẩm cho mỗi dòng");
    }

    // Kiểm tra khả dụng nhưng KHÔNG giữ chỗ
    auto availRes = phoneService.checkAvailable(phoneId, targetQty);
    if (!availRes.isOk()) {
        return availRes;
    }

    auto phoneRes = phoneService.getPhoneById(phoneId);
    if (!phoneRes.isOk()) {
        return Result<void>::fail(phoneRes.code(), phoneRes.message());
    }

    cart.addItem(phoneRes.value(), quantity);
    return Result<void>::ok();
}

Result<void> CartService::updateQuantity(const Actor& actor, const std::string& phoneId, int newQuantity) {
    if (actor.role != Role::CUSTOMER) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }

    Cart& cart = getOrCreateCart(actor.userId);
    if (newQuantity <= 0) {
        cart.removeItem(phoneId);
        return Result<void>::ok();
    }

    if (newQuantity > Config::MAX_QUANTITY_PER_LINE) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng vượt quá giới hạn tối đa " +
            std::to_string(Config::MAX_QUANTITY_PER_LINE) + " sản phẩm cho mỗi dòng");
    }

    auto availRes = phoneService.checkAvailable(phoneId, newQuantity);
    if (!availRes.isOk()) {
        return availRes;
    }

    if (!cart.updateQuantity(phoneId, newQuantity)) {
        return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy sản phẩm trong giỏ hàng");
    }
    return Result<void>::ok();
}

Result<void> CartService::removeItem(const Actor& actor, const std::string& phoneId) {
    if (actor.role != Role::CUSTOMER) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }
    Cart* cart = findCart(actor.userId);
    if (cart) {
        cart->removeItem(phoneId);
    }
    return Result<void>::ok();
}

Result<void> CartService::setSelected(const Actor& actor, const std::string& phoneId, bool selected) {
    if (actor.role != Role::CUSTOMER) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }
    Cart* cart = findCart(actor.userId);
    if (!cart) {
        return Result<void>::fail(ErrorCode::NotFound, "Giỏ hàng trống");
    }

    Vector<CartItem>& items = cart->getItems();
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getPhone().getID() == phoneId) {
            items[i].setSelected(selected);
            return Result<void>::ok();
        }
    }
    return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy sản phẩm trong giỏ hàng");
}

Result<void> CartService::setAllSelected(const Actor& actor, bool selected) {
    if (actor.role != Role::CUSTOMER) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }
    Cart* cart = findCart(actor.userId);
    if (!cart) return Result<void>::ok();

    Vector<CartItem>& items = cart->getItems();
    for (int i = 0; i < items.getSize(); ++i) {
        items[i].setSelected(selected);
    }
    return Result<void>::ok();
}

Result<void> CartService::clear(const Actor& actor) {
    if (actor.role != Role::CUSTOMER) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }
    Cart* cart = findCart(actor.userId);
    if (cart) {
        cart->clear();
    }
    return Result<void>::ok();
}

Result<Money> CartService::calculateTotal(const Actor& actor, const std::string& discountCode) const {
    if (actor.role != Role::CUSTOMER) {
        return Result<Money>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể sử dụng giỏ hàng");
    }
    const Cart* cart = findCart(actor.userId);
    if (!cart) {
        return Result<Money>::fail(ErrorCode::InvalidInput, "Chưa chọn sản phẩm nào");
    }

    Vector<CartItem> selected = cart->getSelectedItems();
    if (selected.getSize() == 0) {
        return Result<Money>::fail(ErrorCode::InvalidInput, "Chưa chọn sản phẩm nào");
    }

    Money total = static_cast<Money>(cart->calculateSelectedTotal());
    if (!discountCode.empty()) {
        auto discRes = discountService.validateAndCalculate(discountCode, total);
        if (!discRes.isOk()) {
            return Result<Money>::fail(discRes.code(), discRes.message());
        }
        total = std::max(0LL, total - discRes.value());
    }
    return Result<Money>::ok(total);
}

Result<std::string> CartService::checkout(const Actor& actor, const CheckoutInfo& info) {
    if (actor.role != Role::CUSTOMER) {
        return Result<std::string>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể thanh toán giỏ hàng");
    }
    Cart* cart = findCart(actor.userId);
    if (!cart) {
        return Result<std::string>::fail(ErrorCode::InvalidInput, "Chưa chọn sản phẩm nào để thanh toán");
    }

    Vector<CartItem> selected = cart->getSelectedItems();
    if (selected.getSize() == 0) {
        return Result<std::string>::fail(ErrorCode::InvalidInput, "Chưa chọn sản phẩm nào để thanh toán");
    }

    OrderDraft draft;
    draft.deliveryType = info.deliveryType;
    draft.paymentMethod = info.paymentMethod;
    draft.receiverName = info.receiverName;
    draft.receiverPhone = info.receiverPhone;
    draft.address = info.address;
    draft.discountCode = info.discountCode;

    for (int i = 0; i < selected.getSize(); ++i) {
        OrderDraftLine line;
        line.phoneId = selected[i].getPhone().getID();
        line.quantity = selected[i].getQuantity();
        draft.lines.push_back(line);
    }

    auto createRes = orderService.createOrder(actor, draft);
    if (!createRes.isOk()) {
        // Thất bại: giữ nguyên giỏ hàng, trả lỗi gốc của createOrder
        return createRes;
    }

    // Chỉ khi thành công mới xóa các dòng đã chọn khỏi giỏ hàng
    cart->removeSelected();
    return createRes;
}
