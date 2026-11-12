#include "OrderService.h"
#include "../common/Config.h"
#include "../common/Permission.h"
#include "../repository/UnitOfWork.h"
#include <sstream>
#include <iomanip>

OrderService::OrderService(OrderRepository& orders, PhoneService& phones, CustomerService& customers,
                           DiscountService& discounts)
    : orderRepo(orders), phoneService(phones), customerService(customers), discountService(discounts) {}

int OrderService::cancelExpiredPending() {
    int cancelledCount = 0;
    Vector<Order> all = orderRepo.getAll();
    DateTime now = DateTime::now();

    for (int i = 0; i < all.getSize(); ++i) {
        Order o = all[i];
        if (o.getStatus() != OrderStatus::PENDING) continue;

        // Nếu thiếu thông tin thời gian tạo thì coi như không hết hạn (tương thích ngược)
        if (o.getCreatedAt().year <= 0) continue;

        DateTime expireTime = o.getCreatedAt().addHours(Config::PENDING_ORDER_TTL_HOURS);
        if (expireTime < now) {
            UnitOfWork uow;
            uow.enlist(orderRepo);

            // Mỗi lượng hàng chỉ nhả đúng một lần theo inventoryState
            if (o.getInventoryState() == InventoryState::RESERVED) {
                const Vector<OrderDetail>& details = o.getDetails();
                for (int d = 0; d < details.getSize(); ++d) {
                    phoneService.releaseReserved(uow, details[d].getPhoneId(), details[d].getQuantity());
                }
                o.setInventoryState(InventoryState::RELEASED);
            }

            if (!o.getDiscountCode().empty()) {
                discountService.releaseUsage(uow, o.getDiscountCode());
            }

            o.setStatus(OrderStatus::CANCELLED);
            orderRepo.update(o);
            auto commitRes = uow.commit();
            if (commitRes.isOk()) {
                ++cancelledCount;
            }
        }
    }
    return cancelledCount;
}

Result<std::string> OrderService::createOrder(const Actor& actor, const OrderDraft& draft) {
    if (!Permission::isAllowed(actor.role, Action::CreateOrder)) {
        return Result<std::string>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có quyền tạo đơn hàng");
    }

    if (draft.lines.getSize() == 0) {
        return Result<std::string>::fail(ErrorCode::InvalidInput, "Đơn hàng phải có ít nhất một sản phẩm");
    }

    // Dọn các đơn hàng PENDING quá hạn trước khi kiểm tra tồn kho để nhả hàng
    cancelExpiredPending();

    // Kiểm tra số lượng từng dòng
    for (int i = 0; i < draft.lines.getSize(); ++i) {
        const OrderDraftLine& line = draft.lines[i];
        if (line.quantity <= 0) {
            return Result<std::string>::fail(ErrorCode::InvalidInput, "Số lượng sản phẩm phải lớn hơn 0 ở dòng " + std::to_string(i + 1));
        }
        if (line.quantity > Config::MAX_QUANTITY_PER_LINE) {
            return Result<std::string>::fail(ErrorCode::InvalidInput, "Số lượng vượt quá giới hạn tối đa " +
                std::to_string(Config::MAX_QUANTITY_PER_LINE) + " sản phẩm cho mỗi dòng");
        }
    }

    // Đếm số đơn PENDING hiện có của khách
    auto custOrdersRes = orderRepo.findByCustomer(actor.userId);
    int pendingCount = 0;
    if (custOrdersRes.isOk()) {
        const Vector<Order>& custOrders = custOrdersRes.value();
        for (int i = 0; i < custOrders.getSize(); ++i) {
            if (custOrders[i].getStatus() == OrderStatus::PENDING) {
                ++pendingCount;
            }
        }
    }
    if (pendingCount >= Config::MAX_PENDING_ORDERS_PER_CUSTOMER) {
        return Result<std::string>::fail(ErrorCode::InvalidState, "Khách hàng đã đạt số lượng tối đa " +
            std::to_string(Config::MAX_PENDING_ORDERS_PER_CUSTOMER) + " đơn hàng đang chờ xử lý");
    }

    // Sinh mã đơn hàng ORD-DDMMYYYY-xxxxxx
    DateTime now = DateTime::now();
    std::ostringstream prefixOss;
    prefixOss << "ORD-" << std::setfill('0') << std::setw(2) << now.day
              << std::setw(2) << now.month << now.year << "-";
    std::string prefix = prefixOss.str();

    int maxNum = 0;
    Vector<Order> allOrders = orderRepo.getAll();
    for (int i = 0; i < allOrders.getSize(); ++i) {
        std::string id = allOrders[i].getID();
        if (id.rfind(prefix, 0) == 0) {
            try {
                int n = std::stoi(id.substr(prefix.length()));
                if (n > maxNum) maxNum = n;
            } catch (...) {}
        }
    }
    std::ostringstream idOss;
    idOss << prefix << std::setfill('0') << std::setw(6) << (maxNum + 1);
    std::string orderId = idOss.str();

    // Giữ chỗ nguyên tử trong một UnitOfWork
    UnitOfWork uow;
    uow.enlist(orderRepo);

    Order order;
    order.setID(orderId);
    order.setCustomerId(actor.userId);
    order.setStatus(OrderStatus::PENDING);
    order.setOrderTime(now);
    order.setInventoryState(InventoryState::RESERVED);
    order.setCustomerConfirmed(false);
    order.setDeliveryMethod(draft.deliveryType == DeliveryType::PICKUP ? DeliveryMethod::PICKUP : DeliveryMethod::DELIVERY);
    order.setPaymentMethod(draft.paymentMethod);

    ShippingInfo ship(draft.receiverName, draft.receiverPhone, draft.address);
    order.setShippingInfo(ship);

    for (int i = 0; i < draft.lines.getSize(); ++i) {
        const OrderDraftLine& line = draft.lines[i];

        // Kiểm tra tồn kho khả dụng
        auto availRes = phoneService.checkAvailable(line.phoneId, line.quantity);
        if (!availRes.isOk()) {
            auto phoneRes = phoneService.getPhoneById(line.phoneId);
            std::string pName = phoneRes.isOk() ? phoneRes.value().getName() : line.phoneId;
            int remain = phoneRes.isOk() ? phoneRes.value().availableQuantity() : 0;
            return Result<std::string>::fail(ErrorCode::OutOfStock,
                "Sản phẩm '" + pName + "' không đủ hàng (chỉ còn " + std::to_string(remain) + ")");
        }

        // Giữ chỗ trong RAM
        auto reserveRes = phoneService.reserve(uow, line.phoneId, line.quantity);
        if (!reserveRes.isOk()) {
            return Result<std::string>::fail(reserveRes.code(), reserveRes.message());
        }

        auto phoneRes = phoneService.getPhoneById(line.phoneId);
        OrderDetail od(line.phoneId, phoneRes.value().getName(), line.quantity, phoneRes.value().getSellingPrice());
        order.addDetail(od);
    }

    // Áp dụng mã giảm giá nếu có
    if (!draft.discountCode.empty()) {
        Money subtotal = static_cast<Money>(order.calculateSubtotal());
        auto discRes = discountService.validateAndCalculate(draft.discountCode, subtotal);
        if (!discRes.isOk()) {
            return Result<std::string>::fail(discRes.code(), discRes.message());
        }
        auto consumeRes = discountService.consumeUsage(uow, draft.discountCode);
        if (!consumeRes.isOk()) {
            return Result<std::string>::fail(consumeRes.code(), consumeRes.message());
        }
        order.setDiscountAmount(static_cast<double>(discRes.value()));
        order.setDiscountCode(draft.discountCode);
    }

    auto addRes = orderRepo.add(order);
    if (!addRes.isOk()) {
        return Result<std::string>::fail(addRes.code(), addRes.message());
    }

    auto commitRes = uow.commit();
    if (!commitRes.isOk()) {
        return Result<std::string>::fail(commitRes.code(), "Lỗi ghi dữ liệu đơn hàng: " + commitRes.message());
    }

    return Result<std::string>::ok(orderId);
}

Result<void> OrderService::confirmReceived(const Actor& actor, const std::string& orderId) {
    auto oRes = orderRepo.findById(orderId);
    if (!oRes.isOk()) return Result<void>::fail(oRes.code(), oRes.message());
    Order o = oRes.value();

    if (actor.role != Role::CUSTOMER || o.getCustomerId() != actor.userId) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng đặt đơn mới được xác nhận đã nhận hàng");
    }

    if (o.getStatus() != OrderStatus::SHIPPING && o.getStatus() != OrderStatus::READY_FOR_PICKUP) {
        return Result<void>::fail(ErrorCode::InvalidState, "Đơn hàng chỉ có thể xác nhận nhận hàng khi đang giao hàng hoặc sẵn sàng nhận");
    }

    o.setCustomerConfirmed(true);
    auto updateRes = orderRepo.update(o);
    if (!updateRes.isOk()) return updateRes;
    return orderRepo.save();
}

Result<void> OrderService::completeOrder(const Actor& actor, const std::string& orderId) {
    if (actor.role == Role::CUSTOMER) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Khách hàng không thể hoàn tất đơn hàng");
    }
    auto oRes = orderRepo.findById(orderId);
    if (!oRes.isOk()) return Result<void>::fail(oRes.code(), oRes.message());
    Order o = oRes.value();

    if (o.getStatus() != OrderStatus::SHIPPING && o.getStatus() != OrderStatus::READY_FOR_PICKUP) {
        return Result<void>::fail(ErrorCode::InvalidState, "Đơn hàng không ở trạng thái có thể hoàn tất");
    }

    if (!o.isCustomerConfirmed()) {
        return Result<void>::fail(ErrorCode::InvalidState, "Khách hàng chưa xác nhận đã nhận được hàng");
    }

    o.setStatus(OrderStatus::COMPLETED);
    auto updateRes = orderRepo.update(o);
    if (!updateRes.isOk()) return updateRes;
    return orderRepo.save();
}

Result<void> OrderService::confirmOrder(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine PENDING->CONFIRMED; commitReserved; inventoryState = DEDUCTED; commit.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::confirmOrder");
}

Result<void> OrderService::startPreparing(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine CONFIRMED->PREPARING.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::startPreparing");
}

Result<void> OrderService::markReadyForPickup(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine PREPARING->READY_FOR_PICKUP (chỉ luồng PICKUP).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::markReadyForPickup");
}

Result<void> OrderService::markShipping(const Actor&, const std::string&) {
    // TODO(logic): OrderStateMachine PREPARING->SHIPPING (chỉ luồng DELIVERY).
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::markShipping");
}

Result<void> OrderService::takePayment(const Actor&, const std::string&, PaymentMethod) {
    // TODO(logic): quyền TakePayment; chuyển paymentStatus UNPAID -> PAID.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::takePayment");
}

Result<void> OrderService::cancelOrder(const Actor&, const std::string&) {
    // TODO(logic): PENDING: releaseReserved + releaseUsage, RELEASED; CONFIRMED: restock, RESTOCKED;
    // từ PREPARING trở đi không cho hủy; mỗi lượng hàng chỉ giải phóng/hoàn đúng một lần theo inventoryState.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: OrderService::cancelOrder");
}

Result<std::string> OrderService::createCounterSale(const Actor&, const OrderDraft&, PaymentMethod) {
    // TODO(logic): quyền CreateCounterSale; tạo đơn + thu tiền + lập hóa đơn trong một UnitOfWork
    // (mặc định tạm: trừ kho thẳng, không giữ chỗ — xem docs/OPEN_QUESTIONS.md).
    return Result<std::string>::fail(ErrorCode::NotImplemented, "TODO: OrderService::createCounterSale");
}

Result<Vector<OrderSummary>> OrderService::listForCustomer(const Actor& actor) const {
    if (actor.role != Role::CUSTOMER) {
        return Result<Vector<OrderSummary>>::fail(ErrorCode::PermissionDenied, "Chỉ khách hàng mới có thể tra cứu đơn hàng của mình");
    }
    auto ordersRes = orderRepo.findByCustomer(actor.userId);
    if (!ordersRes.isOk()) return Result<Vector<OrderSummary>>::fail(ordersRes.code(), ordersRes.message());

    Vector<Order> orders = ordersRes.value();
    Vector<OrderSummary> summaries;
    for (int i = 0; i < orders.getSize(); ++i) {
        const Order& o = orders[i];
        OrderSummary s;
        s.orderId = o.getID();
        s.customerId = o.getCustomerId();
        s.status = o.getStatus();
        s.deliveryType = (o.getDeliveryMethod() == DeliveryMethod::PICKUP ? DeliveryType::PICKUP : DeliveryType::DELIVERY);
        s.totalAmount = static_cast<Money>(o.calculateTotal());
        s.createdTime = o.getOrderTime().toString();
        int count = 0;
        for (int d = 0; d < o.getDetails().getSize(); ++d) count += o.getDetails()[d].getQuantity();
        s.itemCount = count;
        summaries.push_back(s);
    }
    return Result<Vector<OrderSummary>>::ok(summaries);
}

Result<Vector<OrderSummary>> OrderService::listByStatus(const Actor& actor, OrderStatus status) const {
    const_cast<OrderService*>(this)->cancelExpiredPending();

    auto ordersRes = orderRepo.findByStatus(status);
    if (!ordersRes.isOk()) return Result<Vector<OrderSummary>>::fail(ordersRes.code(), ordersRes.message());

    Vector<Order> orders = ordersRes.value();
    Vector<OrderSummary> summaries;
    for (int i = 0; i < orders.getSize(); ++i) {
        const Order& o = orders[i];
        if (actor.role == Role::CUSTOMER && o.getCustomerId() != actor.userId) {
            continue;
        }
        OrderSummary s;
        s.orderId = o.getID();
        s.customerId = o.getCustomerId();
        s.status = o.getStatus();
        s.deliveryType = (o.getDeliveryMethod() == DeliveryMethod::PICKUP ? DeliveryType::PICKUP : DeliveryType::DELIVERY);
        s.totalAmount = static_cast<Money>(o.calculateTotal());
        s.createdTime = o.getOrderTime().toString();
        int count = 0;
        for (int d = 0; d < o.getDetails().getSize(); ++d) count += o.getDetails()[d].getQuantity();
        s.itemCount = count;
        summaries.push_back(s);
    }
    return Result<Vector<OrderSummary>>::ok(summaries);
}

Result<OrderSummary> OrderService::getOrder(const Actor& actor, const std::string& orderId) const {
    auto oRes = orderRepo.findById(orderId);
    if (!oRes.isOk()) return Result<OrderSummary>::fail(oRes.code(), oRes.message());
    const Order& o = oRes.value();

    if (actor.role == Role::CUSTOMER && o.getCustomerId() != actor.userId) {
        return Result<OrderSummary>::fail(ErrorCode::PermissionDenied, "Không có quyền xem thông tin đơn hàng này");
    }

    OrderSummary s;
    s.orderId = o.getID();
    s.customerId = o.getCustomerId();
    s.status = o.getStatus();
    s.deliveryType = (o.getDeliveryMethod() == DeliveryMethod::PICKUP ? DeliveryType::PICKUP : DeliveryType::DELIVERY);
    s.totalAmount = static_cast<Money>(o.calculateTotal());
    s.createdTime = o.getOrderTime().toString();
    int count = 0;
    for (int d = 0; d < o.getDetails().getSize(); ++d) count += o.getDetails()[d].getQuantity();
    s.itemCount = count;
    return Result<OrderSummary>::ok(s);
}
