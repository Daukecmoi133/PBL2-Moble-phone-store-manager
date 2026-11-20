#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === HÓA ĐƠN ===

std::vector<InvoiceItemViewModel> AppController::getAllInvoices() {
    if (!currentSession.isAuthenticated || (!currentSession.isAdmin() && !currentSession.isSales()) ||
        invoiceService == nullptr) {
        return {};
    }
    const Vector<Invoice>& invoices = invoiceService->getAll();
    std::vector<InvoiceItemViewModel> result;
    result.reserve(static_cast<size_t>(invoices.getSize()));
    for (int i = invoices.getSize() - 1; i >= 0; --i) {
        result.push_back(invoiceToViewModel(invoices[i], customerService, employeeService));
    }
    return result;
}

InvoiceItemViewModel AppController::getInvoiceById(const std::string& invoiceId) {
    if (!currentSession.isAuthenticated || currentSession.isWarehouse() ||
        invoiceService == nullptr) {
        return InvoiceItemViewModel();
    }
    Invoice* inv = invoiceService->getById(invoiceId);
    if (inv == nullptr) {
        return InvoiceItemViewModel();
    }
    if (currentSession.isCustomer() && inv->getCustomerId() != currentSession.userId) {
        return InvoiceItemViewModel();
    }
    return invoiceToViewModel(*inv, customerService, employeeService);
}

OperationResult AppController::createInvoiceForOrder(const std::string& orderId, const std::string& employeeId, PaymentMethod method) {
    REQUIRE_ADMIN_OR_SALES_ROLE();
    if (!IdGenerator::isValidId(orderId, "ORD", true) ||
        !IdGenerator::isValidId(employeeId, "EMP")) {
        return OperationResult::Fail("Mã đơn hàng hoặc nhân viên không đúng định dạng ID!");
    }
    if (invoiceService == nullptr || orderService == nullptr) {
        return OperationResult::Fail("Dịch vụ hóa đơn hoặc đơn hàng chưa sẵn sàng!");
    }
    if (employeeService == nullptr || employeeService->getEmployeeById(employeeId) == nullptr) {
        return OperationResult::Fail("Không tìm thấy nhân viên được tham chiếu!");
    }

    // Kiểm tra xem đơn hàng đã có hóa đơn chưa (mỗi đơn chỉ 1 hóa đơn)
    if (Invoice* existingInv = invoiceService->getByOrderId(orderId)) {
        return OperationResult::Fail("Đơn hàng '" + orderId + "' đã được xuất hóa đơn (" + existingInv->getID() + ")!");
    }

    Order* orderPtr = orderService->getOrderById(orderId);
    if (orderPtr == nullptr) {
        return OperationResult::Fail("Không tìm thấy đơn hàng!");
    }
    const std::string ordCustomerId = orderPtr->getCustomerId();
    if (!ordCustomerId.empty() &&
        (!IdGenerator::isValidId(ordCustomerId, "CUS") ||
         customerService == nullptr ||
         customerService->getCustomerById(ordCustomerId) == nullptr)) {
        return OperationResult::Fail("Khách hàng của đơn hàng không hợp lệ hoặc không tồn tại!");
    }

    std::string invId;
    std::string serviceErr;
    if (!invoiceService->createInvoiceFromOrder(orderId, employeeId, method, orderPtr->getDiscountAmount(), serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu hóa đơn vào file dữ liệu!");
    }
    Invoice* created = invoiceService->getByOrderId(orderId);
    if (created != nullptr) {
        invId = created->getID();
    }

    return OperationResult::Ok("Đã xuất hóa đơn thành công! Mã hóa đơn: " + invId);
}

OperationResult AppController::createPosInvoice(const std::string& customerId, const std::string& employeeId,
                                                const std::vector<CartItemViewModel>& items,
                                                const std::string& discountCode, PaymentMethod payMethod,
                                                double cashGiven, double& outChange, std::string& outInvoiceId) {
    REQUIRE_ADMIN_OR_SALES_ROLE();
    outChange = 0.0;
    outInvoiceId.clear();
    if (items.empty()) {
        return OperationResult::Fail("Vui lòng chọn ít nhất 1 sản phẩm vào đơn bán tại quầy!");
    }
    if (phoneService == nullptr || orderService == nullptr || invoiceService == nullptr) {
        return OperationResult::Fail("Dịch vụ bán hàng tại quầy chưa sẵn sàng!");
    }

    if (!IdGenerator::isValidId(employeeId, "EMP") ||
        employeeService == nullptr ||
        employeeService->getEmployeeById(employeeId) == nullptr) {
        return OperationResult::Fail("Nhân viên lập hóa đơn không hợp lệ hoặc không tồn tại!");
    }
    Customer* memberCustomer = nullptr;
    if (!customerId.empty() && customerId != "WALK-IN") {
        if (!IdGenerator::isValidId(customerId, "CUS") || customerService == nullptr) {
            return OperationResult::Fail("Khách hàng được tham chiếu không hợp lệ hoặc không tồn tại!");
        }
        memberCustomer = customerService->getCustomerById(customerId);
        if (memberCustomer == nullptr) {
            return OperationResult::Fail("Khách hàng được tham chiếu không hợp lệ hoặc không tồn tại!");
        }
    }

    // Tính tổng tiền & kiểm tra tồn kho khả dụng (gồm cả giữ chỗ đơn online)
    double subtotal = 0.0;
    for (const auto& it : items) {
        if (!IdGenerator::isValidId(it.phoneId, "PHN") || it.quantity <= 0) {
            return OperationResult::Fail("Chi tiết bán hàng chứa mã sản phẩm hoặc số lượng không hợp lệ!");
        }
        subtotal += it.subtotal;
        ProductItemViewModel p = getProductById(it.phoneId);
        if (p.id.empty() || p.availableStock < it.quantity) {
            return OperationResult::Fail("Sản phẩm '" + it.phoneName + "' không đủ tồn kho khả dụng!");
        }
    }

    // Mã giảm giá (kiểm tra giới hạn 1 lần/khách & đồng hồ thống nhất) + VAT qua PricingCalculator
    const std::string trimmedCoupon = StringHelper::trim(discountCode);
    const std::string effectiveCustId = memberCustomer ? customerId : "";
    const double vatRate = getVatRate();
    double rawDiscount = 0.0;
    if (!trimmedCoupon.empty()) {
        double validatedDisc = 0.0;
        OperationResult dRes = validateDiscountForCustomer(trimmedCoupon, subtotal, effectiveCustId, validatedDisc);
        if (!dRes.success) {
            return dRes;
        }
        const DiscountCode* dc = discountService ? discountService->getByCode(trimmedCoupon) : nullptr;
        rawDiscount = (dc != nullptr)
                          ? PricingCalculator::computeRawDiscount(subtotal, dc->getType(), dc->getValue(), dc->getMaxDiscountValue())
                          : validatedDisc;
    }

    const PricingBreakdown pb = PricingCalculator::compute(subtotal, rawDiscount, vatRate);

    double change = 0.0;
    if (payMethod == PaymentMethod::CASH) {
        if (cashGiven < pb.grandTotal) {
            return OperationResult::Fail("Số tiền khách đưa không đủ! Còn thiếu " +
                                         std::to_string(static_cast<long long>(pb.grandTotal - cashGiven)) + " đ");
        }
        change = cashGiven - pb.grandTotal;
    }

    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    std::string orderId;
    std::vector<std::string> orderIds = collectModelIds(orderService->getAllOrders());
    if (idDate.empty() ||
        !IdGenerator::nextId("ORD", idDate, orderIds, orderId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã đơn hàng." : idError);
    }

    // Transaction cho Multi-file (Products, Orders, Invoices, Discount, Customers)
    FileTransaction tx;
    tx.addParticipant(phoneService->getRepository().getFilePath());
    tx.addParticipant(orderService->getRepository().getFilePath());
    tx.addParticipant(invoiceService->getRepository().getFilePath());
    if (!trimmedCoupon.empty() && discountService != nullptr) {
        tx.addParticipant(discountService->getRepository().getFilePath());
    }
    if (memberCustomer != nullptr && customerService != nullptr) {
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

    // 1. Chụp snapshot giá vốn trước khi trừ kho qua PhoneService (AUD-03)
    std::vector<double> unitCosts;
    unitCosts.reserve(items.size());
    for (const auto& it : items) {
        const Phone* ph = phoneService->getPhoneById(it.phoneId);
        unitCosts.push_back(ph ? ph->getImportPrice() : 0.0);
    }

    // 2. Trừ kho qua PhoneService
    for (const auto& it : items) {
        std::string serviceErr;
        if (!phoneService->updateStock(it.phoneId, -it.quantity, serviceErr)) {
            tx.rollback();
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật kho cho đơn bán tại quầy!");
        }
    }

    std::string custName = "Khách vãng lai";
    if (memberCustomer != nullptr) {
        const std::string fullName = memberCustomer->getName().getFullName();
        if (!fullName.empty()) custName = fullName;
    }

    // 3. Lưu Order hoàn tất qua OrderService (snapshot unitCost + VAT cấu hình)
    {
        ShippingInfo ship(custName, "-", "Tại quầy thu ngân");
        Order ordModel(orderId, customerId == "WALK-IN" ? "" : customerId, DeliveryMethod::PICKUP, payMethod, ship);
        ordModel.setOrderTime(AppClock::now());
        ordModel.setStatus(OrderStatus::COMPLETED);
        ordModel.setDiscountAmount(pb.discountAmount);
        ordModel.setVat(pb.vatRate, pb.vatAmount, true);
        if (!trimmedCoupon.empty()) {
            ordModel.setDiscountCode(trimmedCoupon);
        }
        for (size_t idx = 0; idx < items.size(); ++idx) {
            const auto& it = items[idx];
            ordModel.addDetail(OrderDetail(it.phoneId, it.phoneName, it.quantity, it.unitPrice, unitCosts[idx]));
        }
        std::string serviceErr;
        if (!orderService->addCompletedOrder(ordModel, serviceErr)) {
            tx.rollback();
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu đơn hàng POS vào file!");
        }
    }

    // 4. Xuất hóa đơn và lưu qua InvoiceService (kế thừa unitCost & VAT từ Order, sinh mã INV 1 lần)
    std::string invoiceId;
    {
        std::string serviceErr;
        if (!invoiceService->createInvoiceFromOrder(orderId, employeeId, payMethod, pb.discountAmount, serviceErr)) {
            tx.rollback();
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu hóa đơn POS vào file!");
        }
        Invoice* created = invoiceService->getByOrderId(orderId);
        if (created != nullptr) {
            invoiceId = created->getID();
        }
    }

    // 5. Tiêu thụ voucher nếu có
    if (!trimmedCoupon.empty() && discountService != nullptr) {
        std::string discErr;
        if (!discountService->consumeUsage(trimmedCoupon, discErr)) {
            tx.rollback();
            return OperationResult::Fail(!discErr.empty() ? discErr : "Lỗi khi áp dụng và cập nhật lượt dùng mã giảm giá!");
        }
    }

    // 6. Tích điểm thành viên nếu có (trên doanh thu thuần trước VAT)
    if (memberCustomer != nullptr && customerService != nullptr) {
        const int points = static_cast<int>(pb.taxableAmount / 100000.0);
        if (points > 0) {
            if (!customerService->addPoints(customerId, points)) {
                tx.rollback();
                return OperationResult::Fail("Lỗi khi cập nhật điểm tích lũy cho khách hàng!");
            }
        }
    }

    if (!tx.commit()) {
        tx.rollback();
        return OperationResult::Fail("Lỗi khi xác nhận giao dịch ghi file cho đơn bán tại quầy!");
    }

    outInvoiceId = invoiceId;
    outChange = change;
    return OperationResult::Ok("Giao dịch thanh toán tại quầy thành công! Mã HĐ: " + outInvoiceId);
}
