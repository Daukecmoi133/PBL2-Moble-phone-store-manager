#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === HÓA ĐƠN ===

std::vector<InvoiceItemViewModel> AppController::getAllInvoices() {
    return MockDataStore::getInstance().invoices;
}

InvoiceItemViewModel AppController::getInvoiceById(const std::string& invoiceId) {
    auto& store = MockDataStore::getInstance();
    for (const auto& inv : store.invoices) {
        if (inv.id == invoiceId) return inv;
    }
    return InvoiceItemViewModel();
}

OperationResult AppController::createInvoiceForOrder(const std::string& orderId, const std::string& employeeId, PaymentMethod method) {
    if (!IdGenerator::isValidId(orderId, "ORD", true) ||
        !IdGenerator::isValidId(employeeId, "EMP")) {
        return OperationResult::Fail("Mã đơn hàng hoặc nhân viên không đúng định dạng ID!");
    }
    auto& store = MockDataStore::getInstance();
    if (getEmployeeById(employeeId).id.empty()) {
        return OperationResult::Fail("Không tìm thấy nhân viên được tham chiếu!");
    }

    // Kiểm tra xem đơn hàng đã có hóa đơn chưa (mỗi đơn chỉ 1 hóa đơn)
    for (const auto& inv : store.invoices) {
        if (inv.orderId == orderId) {
            return OperationResult::Fail("Đơn hàng '" + orderId + "' đã được xuất hóa đơn (" + inv.id + ")!");
        }
    }

    OrderItemViewModel ord = getOrderById(orderId);
    if (ord.id.empty()) {
        return OperationResult::Fail("Không tìm thấy đơn hàng!");
    }
    if (!ord.customerId.empty() &&
        (!IdGenerator::isValidId(ord.customerId, "CUS") ||
         getCustomerById(ord.customerId).id.empty())) {
        return OperationResult::Fail("Khách hàng của đơn hàng không hợp lệ hoặc không tồn tại!");
    }

    std::string invId;
    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("INV", idDate, collectIds(store.invoices), invId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã hóa đơn." : idError);
    }

    // Lưu hóa đơn vào file qua InvoiceService
    if (invoiceService != nullptr) {
        std::string serviceErr;
        if (!invoiceService->createInvoiceFromOrder(orderId, employeeId, method, ord.discountAmount, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu hóa đơn vào file dữ liệu!");
        }
        Invoice* created = invoiceService->getByOrderId(orderId);
        if (created != nullptr) {
            invId = created->getID();
        }
    }

    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    InvoiceItemViewModel newInv;
    newInv.id = invId;
    newInv.orderId = ord.id;
    newInv.customerId = ord.customerId;
    newInv.customerName = ord.customerName;
    newInv.employeeId = employeeId;
    newInv.employeeName = "Nhân viên thu ngân";
    newInv.paymentMethod = method;
    newInv.paymentMethodText = paymentMethodToString(method);
    newInv.createTime = timeBuf;
    newInv.subtotal = ord.subtotal;
    newInv.discountAmount = ord.discountAmount;
    newInv.total = ord.total;
    newInv.details = ord.details;

    store.invoices.insert(store.invoices.begin(), newInv);
    return OperationResult::Ok("Đã xuất hóa đơn thành công! Mã hóa đơn: " + newInv.id);
}

OperationResult AppController::createPosInvoice(const std::string& customerId, const std::string& employeeId,
                                                const std::vector<CartItemViewModel>& items,
                                                const std::string& discountCode, PaymentMethod payMethod,
                                                double cashGiven, double& outChange, std::string& outInvoiceId) {
    outChange = 0.0;
    outInvoiceId.clear();
    if (items.empty()) {
        return OperationResult::Fail("Vui lòng chọn ít nhất 1 sản phẩm vào đơn bán tại quầy!");
    }

    auto& store = MockDataStore::getInstance();
    if (!IdGenerator::isValidId(employeeId, "EMP") ||
        getEmployeeById(employeeId).id.empty()) {
        return OperationResult::Fail("Nhân viên lập hóa đơn không hợp lệ hoặc không tồn tại!");
    }
    if (!customerId.empty() && customerId != "WALK-IN" &&
        (!IdGenerator::isValidId(customerId, "CUS") ||
         getCustomerById(customerId).id.empty())) {
        return OperationResult::Fail("Khách hàng được tham chiếu không hợp lệ hoặc không tồn tại!");
    }

    // Tính tổng tiền
    double subtotal = 0.0;
    for (const auto& it : items) {
        if (!IdGenerator::isValidId(it.phoneId, "PHN") || it.quantity <= 0) {
            return OperationResult::Fail("Chi tiết bán hàng chứa mã sản phẩm hoặc số lượng không hợp lệ!");
        }
        subtotal += it.subtotal;
        ProductItemViewModel p = getProductById(it.phoneId);
        if (p.availableStock < it.quantity) {
            return OperationResult::Fail("Sản phẩm '" + it.phoneName + "' không đủ tồn kho khả dụng!");
        }
    }

    // Mã giảm giá
    double discount = 0.0;
    if (!discountCode.empty()) {
        OperationResult dRes = validateDiscount(discountCode, subtotal, discount);
        if (!dRes.success) {
            return dRes;
        }
    }

    double total = subtotal - discount;
    if (total < 0) total = 0;

    double change = 0.0;
    if (payMethod == PaymentMethod::CASH) {
        if (cashGiven < total) {
            return OperationResult::Fail("Số tiền khách đưa không đủ! Còn thiếu " +
                                         std::to_string(static_cast<long long>(total - cashGiven)) + " đ");
        }
        change = cashGiven - total;
    }

    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    std::string orderId;
    std::vector<std::string> invoiceIds = collectIds(store.invoices);
    std::vector<std::string> orderIds = collectIds(store.orders);
    if (idDate.empty() ||
        !IdGenerator::nextId("ORD", idDate, orderIds, orderId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã đơn hàng." : idError);
    }
    std::string invoiceId;
    if (!IdGenerator::nextId("INV", idDate, invoiceIds, invoiceId, idError)) {
        return OperationResult::Fail(idError);
    }

    // Transaction cho Multi-file (Products, Orders, Invoices, Discount, Customers)
    FileTransaction tx;
    if (phoneService != nullptr) tx.addParticipant(phoneService->getRepository().getFilePath());
    if (orderService != nullptr) tx.addParticipant(orderService->getRepository().getFilePath());
    if (invoiceService != nullptr) tx.addParticipant(invoiceService->getRepository().getFilePath());
    if (!discountCode.empty() && discountService != nullptr) {
        tx.addParticipant(discountService->getRepository().getFilePath());
    }
    if (!customerId.empty() && customerId != "WALK-IN" && customerService != nullptr) {
        tx.addParticipant(customerService->getRepository().getFilePath());
    }

    tx.onRollback([this]() {
        if (this->phoneService) this->phoneService->reload();
        if (this->orderService) this->orderService->reload();
        if (this->invoiceService) this->invoiceService->reload();
        if (this->discountService) this->discountService->reload();
        if (this->customerService) this->customerService->reload();
    });

    if (!tx.begin()) {
        return OperationResult::Fail("Không thể khởi tạo giao dịch lưu trữ hóa đơn POS!");
    }

    // 1. Trừ kho qua PhoneService
    if (phoneService != nullptr) {
        for (const auto& it : items) {
            std::string serviceErr;
            if (!phoneService->updateStock(it.phoneId, -it.quantity, serviceErr)) {
                tx.rollback();
                return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật kho cho đơn bán tại quầy!");
            }
        }
    }

    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    std::string custName = "Khách vãng lai";
    if (!customerId.empty() && customerId != "WALK-IN") {
        CustomerItemViewModel c = getCustomerById(customerId);
        if (!c.fullName.empty()) custName = c.fullName;
    }

    // 2. Lưu Order hoàn tất qua OrderService
    if (orderService != nullptr) {
        ShippingInfo ship(custName, "-", "Tại quầy thu ngân");
        Order ordModel(orderId, customerId == "WALK-IN" ? "" : customerId, DeliveryMethod::PICKUP, payMethod, ship);
        ordModel.setStatus(OrderStatus::COMPLETED);
        ordModel.setDiscountAmount(discount);
        for (const auto& it : items) {
            ordModel.addDetail(OrderDetail(it.phoneId, it.phoneName, it.quantity, it.unitPrice));
        }
        std::string serviceErr;
        if (!orderService->addCompletedOrder(ordModel, serviceErr)) {
            tx.rollback();
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu đơn hàng POS vào file!");
        }
    }

    // 3. Xuất hóa đơn và lưu qua InvoiceService
    if (invoiceService != nullptr) {
        std::string serviceErr;
        if (!invoiceService->createInvoiceFromOrder(orderId, employeeId, payMethod, discount, serviceErr)) {
            tx.rollback();
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu hóa đơn POS vào file!");
        }
        Invoice* created = invoiceService->getByOrderId(orderId);
        if (created != nullptr) {
            invoiceId = created->getID();
        }
    }

    // 4. Tiêu thụ voucher nếu có
    if (!discountCode.empty() && discountService != nullptr) {
        if (!discountService->consumeUsage(discountCode)) {
            tx.rollback();
            return OperationResult::Fail("Lỗi khi áp dụng và cập nhật lượt dùng mã giảm giá!");
        }
    }

    // 5. Tích điểm thành viên nếu có
    int points = 0;
    if (!customerId.empty() && customerId != "WALK-IN" && customerService != nullptr) {
        points = static_cast<int>(total / 100000.0);
        if (points > 0) {
            if (!customerService->addPoints(customerId, points)) {
                tx.rollback();
                return OperationResult::Fail("Lỗi khi cập nhật điểm tích lũy cho khách hàng!");
            }
        }
    }

    // COMMIT TẤT CẢ FILE TRƯỚC KHI CẬP NHẬT VIEW CACHE
    if (!tx.commit()) {
        return OperationResult::Fail("Lỗi khi xác nhận giao dịch ghi file cho đơn bán tại quầy!");
    }

    // Đã commit thành công 100% -> Cập nhật MockDataStore:
    outInvoiceId = invoiceId;
    outChange = change;

    // Trừ kho trên View cache
    for (const auto& it : items) {
        for (auto& p : store.products) {
            if (p.id == it.phoneId) {
                p.actualStock -= it.quantity;
                p.calculateStockStatus();
                break;
            }
        }
    }

    // Cập nhật voucher trên View cache
    if (!discountCode.empty()) {
        for (auto& d : store.discounts) {
            if (d.code == discountCode) {
                d.usedCount += 1;
                break;
            }
        }
    }

    InvoiceItemViewModel inv;
    inv.id = invoiceId;
    inv.orderId = orderId;
    inv.customerId = customerId == "WALK-IN" ? "" : customerId;
    inv.customerName = custName;
    inv.employeeId = employeeId;
    inv.employeeName = "Nhân viên Bán hàng";
    inv.paymentMethod = payMethod;
    inv.paymentMethodText = paymentMethodToString(payMethod);
    inv.createTime = timeBuf;
    inv.subtotal = subtotal;
    inv.discountAmount = discount;
    inv.total = total;

    for (const auto& it : items) {
        OrderDetailItemViewModel d;
        d.phoneId = it.phoneId;
        d.phoneName = it.phoneName;
        d.quantity = it.quantity;
        d.unitPrice = it.unitPrice;
        d.subtotal = it.subtotal;
        inv.details.push_back(d);
    }

    store.invoices.insert(store.invoices.begin(), inv);

    // Cũng tạo 1 bản ghi đơn hàng COMPLETED để đồng bộ thống kê
    OrderItemViewModel ord;
    ord.id = orderId;
    ord.customerId = inv.customerId;
    ord.customerName = inv.customerName;
    ord.orderTime = timeBuf;
    ord.status = OrderStatus::COMPLETED;
    ord.statusText = "Hoàn thành";
    ord.deliveryMethod = DeliveryMethod::PICKUP;
    ord.deliveryMethodText = "Nhận tại quầy";
    ord.paymentMethod = payMethod;
    ord.paymentMethodText = paymentMethodToString(payMethod);
    ord.paymentStatus = PaymentStatus::PAID;
    ord.paymentStatusText = "Đã thanh toán";
    ord.subtotal = subtotal;
    ord.discountAmount = discount;
    ord.total = total;
    ord.receiverName = custName;
    ord.receiverPhone = "-";
    ord.deliveryAddress = "Tại quầy thu ngân";
    ord.details = inv.details;
    store.orders.insert(store.orders.begin(), ord);

    // Tích điểm nếu có customer
    if (points > 0 && !customerId.empty() && customerId != "WALK-IN") {
        for (auto& c : store.customers) {
            if (c.id == customerId) {
                c.points += points;
                c.updateTier();
                break;
            }
        }
    }

    return OperationResult::Ok("Giao dịch thanh toán tại quầy thành công! Mã HĐ: " + outInvoiceId);
}
