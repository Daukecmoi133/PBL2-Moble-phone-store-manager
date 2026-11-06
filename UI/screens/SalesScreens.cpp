/**
 * @file SalesScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ NHÂN VIÊN BÁN HÀNG (SALES REPRESENTATIVE)
 * 
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildDashboardView & refreshDashboard: Bàn làm việc Saler, thống kê doanh số và đơn cần xử lý.
 * - buildPosView & refreshPos: Bán hàng tại quầy (POS), tính toán chiết khấu voucher, tiền khách đưa và tiền thối.
 * - buildProductLookupView: Tra cứu cấu hình, giá niêm yết và tồn kho thực tế.
 * - buildCustomerMgmtView: Thêm khách hàng mới, tra cứu thông tin khách hàng và lịch sử.
 * - buildInvoicesView: Danh sách hóa đơn bán hàng đã phát hành.
 * - buildOnlineOrdersView: Tra cứu chi tiết đơn hàng online (sản phẩm, số lượng, địa chỉ).
 * - buildWarrantyIntakeView: Tiếp nhận thiết bị bảo hành từ khách hàng.
 * =========================================================================================
 */

#include "SalesScreens.h"
#include "MainShellWindow.h"
#include "../components/UIComponents.h"

SalesScreens::SalesScreens(AppController* ctrl, MainShellWindow* parentShell)
    : controller(ctrl), shell(parentShell), posSelectedCustomerId("WALK-IN"), posAppliedCoupon("") {

    dashboardView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    posView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(posView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    productLookupView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productLookupView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    customerMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(customerMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    invoicesView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(invoicesView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    onlineOrdersView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(onlineOrdersView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    warrantyIntakeView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyIntakeView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    buildDashboardView();
    buildPosView();
    buildProductLookupView();
    buildCustomerMgmtView();
    buildInvoicesView();
    buildOnlineOrdersView();
    buildWarrantyIntakeView();
}

void SalesScreens::refreshAll() {
    refreshDashboard();
    refreshPos();
    refreshProductLookup();
    refreshCustomers();
    refreshInvoices();
    refreshOnlineOrders();
    refreshWarrantyIntake();
}

// ================= 1. SALES DASHBOARD =================

void SalesScreens::buildDashboardView() {

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_add_css_class(content, "page-container");

    const UserSessionViewModel& user = controller->getCurrentUser();
    std::string title = "Bàn làm việc Nhân viên Bán hàng — " + user.fullName;
    GtkWidget* header = UIComponents::createSectionHeader(title, "Theo dõi các giao dịch bán hàng, đơn hàng online và hỗ trợ khách hàng.");
    gtk_box_append(GTK_BOX(content), header);

    // Metric Cards
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    std::vector<OrderItemViewModel> orders = controller->getAllOrders();
    std::vector<InvoiceItemViewModel> invoices = controller->getAllInvoices();
    std::vector<CustomerItemViewModel> custs = controller->getCustomers();

    int pendingOrders = 0;
    for (const auto& o : orders) {
        if (o.status == OrderStatus::PENDING) pendingOrders++;
    }

    double todayRev = 0.0;
    for (const auto& inv : invoices) {
        todayRev += inv.total;
    }

    GtkWidget* c1 = UIComponents::createStatCard("Doanh thu bán hàng", UIComponents::formatVND(todayRev), "Tổng hóa đơn đã xuất", "💰");
    GtkWidget* c2 = UIComponents::createStatCard("Đơn online chờ duyệt", std::to_string(pendingOrders) + " đơn", "Cần xác nhận", "⏳");
    GtkWidget* c3 = UIComponents::createStatCard("Tổng số hóa đơn", std::to_string(invoices.size()) + " HĐ", "Hóa đơn hợp lệ", "📑");
    GtkWidget* c4 = UIComponents::createStatCard("Khách hàng hệ thống", std::to_string(custs.size()) + " người", "Đã đăng ký tài khoản", "👥");

    gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c4, 3, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), grid);

    // Quick POS Action Card
    GtkWidget* quickCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(quickCard, "card");

    GtkWidget* lblQuick = gtk_label_new("⚡ Thao tác nhanh");
    gtk_widget_add_css_class(lblQuick, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblQuick), 0.0f);
    gtk_box_append(GTK_BOX(quickCard), lblQuick);

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    GtkWidget* bPos = gtk_button_new_with_label("🖥️ Mở máy bán hàng tại quầy (POS)");
    gtk_widget_add_css_class(bPos, "btn-primary");
    UIComponents::setButtonCursor(bPos);
    g_signal_connect_swapped(bPos, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("sales_pos");
    }), shell);
    gtk_box_append(GTK_BOX(btnRow), bPos);

    GtkWidget* bOrders = gtk_button_new_with_label("📦 Xử lý đơn online");
    gtk_widget_add_css_class(bOrders, "btn-secondary");
    UIComponents::setButtonCursor(bOrders);
    g_signal_connect_swapped(bOrders, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("sales_orders");
    }), shell);
    gtk_box_append(GTK_BOX(btnRow), bOrders);

    GtkWidget* bCust = gtk_button_new_with_label("👥 Thêm khách hàng mới");
    gtk_widget_add_css_class(bCust, "btn-outline");
    UIComponents::setButtonCursor(bCust);
    g_signal_connect_swapped(bCust, "clicked", G_CALLBACK(+[](SalesScreens* s) {
        s->showCustomerModal(nullptr);
    }), this);
    gtk_box_append(GTK_BOX(btnRow), bCust);

    gtk_box_append(GTK_BOX(quickCard), btnRow);
    gtk_box_append(GTK_BOX(content), quickCard);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(dashboardView), content);
}

void SalesScreens::refreshDashboard() {
    buildDashboardView();
}

// ================= 2. MÀN HÌNH BÁN HÀNG TẠI QUẦY (POS) =================

void SalesScreens::buildPosView() {
    posView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(posView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Bán hàng tại quầy (Point of Sale)", "Tạo đơn hàng, thêm sản phẩm và xuất hóa đơn trực tiếp cho khách.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    // Left Panel: Cart & Items selection
    GtkWidget* leftCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(leftCard, "card");
    gtk_widget_set_hexpand(leftCard, TRUE);

    GtkWidget* topBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    // Customer Dropdown
    std::vector<CustomerItemViewModel> custs = controller->getCustomers();
    std::vector<const char*> custNames;
    custNames.push_back("Khách vãng lai (Không tích điểm)");
    for (const auto& c : custs) {
        custNames.push_back(c.fullName.c_str());
    }
    custNames.push_back(nullptr);

    posComboCustomer = gtk_drop_down_new_from_strings(custNames.data());
    gtk_widget_set_hexpand(posComboCustomer, TRUE);
    gtk_box_append(GTK_BOX(topBar), posComboCustomer);

    GtkWidget* btnAddProd = gtk_button_new_with_label("Chọn sản phẩm (F2)");
    gtk_widget_add_css_class(btnAddProd, "btn-primary");
    UIComponents::setButtonCursor(btnAddProd);
    g_signal_connect_swapped(btnAddProd, "clicked", G_CALLBACK(+[](SalesScreens* s) {
        s->showPosProductPickerModal();
    }), this);
    gtk_box_append(GTK_BOX(topBar), btnAddProd);

    // Ô tìm kiếm / quét mã: luôn được focus, Enter = thêm sản phẩm khớp đầu tiên
    posEntrySearch = gtk_entry_new();
    gtk_widget_add_css_class(posEntrySearch, "pos-search");
    gtk_entry_set_placeholder_text(GTK_ENTRY(posEntrySearch), "Quét mã hoặc nhập tên / mã / hãng sản phẩm rồi nhấn Enter...");
    g_signal_connect_data(posEntrySearch, "activate", G_CALLBACK(+[](GtkEntry* e, gpointer d) {
        SalesScreens* self = static_cast<SalesScreens*>(d);
        std::string key = gtk_editable_get_text(GTK_EDITABLE(e));
        if (key.empty()) return;
        auto lower = [](std::string s) { for (auto& ch : s) ch = (char)tolower((unsigned char)ch); return s; };
        std::string lk = lower(key);
        std::vector<ProductItemViewModel> prods = self->controller->getProducts();
        for (const auto& p : prods) {
            if (p.availableStock <= 0) continue;
            if (lower(p.id) != lk && lower(p.name).find(lk) == std::string::npos && lower(p.brand).find(lk) == std::string::npos) continue;
            bool found = false;
            for (auto& item : self->posCartItems) {
                if (item.phoneId == p.id) { item.quantity += 1; item.calculateSubtotal(); found = true; break; }
            }
            if (!found) {
                CartItemViewModel ci;
                ci.phoneId = p.id;
                ci.phoneName = p.name;
                ci.brand = p.brand;
                ci.unitPrice = p.sellingPrice;
                ci.quantity = 1;
                ci.availableStock = p.availableStock;
                ci.calculateSubtotal();
                self->posCartItems.push_back(ci);
            }
            self->refreshPos();
            gtk_editable_set_text(GTK_EDITABLE(e), "");
            gtk_widget_grab_focus(GTK_WIDGET(e));
            return;
        }
        UIComponents::showWarningDialog(self->shell->getWindow(), "Không tìm thấy", "Không có sản phẩm còn hàng khớp với: " + key);
    }), this, nullptr, (GConnectFlags)0);
    g_signal_connect(posEntrySearch, "map", G_CALLBACK(+[](GtkWidget* w, gpointer) {
        gtk_widget_grab_focus(w);
    }), nullptr);

    gtk_box_append(GTK_BOX(leftCard), topBar);
    gtk_box_append(GTK_BOX(leftCard), posEntrySearch);

    std::vector<UIComponents::TableColumnSpec> posCols = {
        {"Sản phẩm", 0, 0.0f, true},
        {"Đơn giá", 120, 1.0f, false},
        {"SL", 48, 1.0f, false},
        {"Thành tiền", 130, 1.0f, false},
        {"Thao tác", 70, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(leftCard), UIComponents::createTableHeaderRow(posCols));

    // Items list container (cuộn độc lập)
    posCartListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget* cartScroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(cartScroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(cartScroll), 320);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(cartScroll), posCartListBox);
    gtk_widget_set_vexpand(cartScroll, TRUE);
    gtk_box_append(GTK_BOX(leftCard), cartScroll);

    GtkWidget* hintRow = gtk_label_new("F2: Chọn sản phẩm   |   F9: Thanh toán   |   Esc: Xóa giỏ hàng");
    gtk_widget_add_css_class(hintRow, "shortcut-hint");
    gtk_label_set_xalign(GTK_LABEL(hintRow), 0.0f);
    gtk_box_append(GTK_BOX(leftCard), hintRow);

    gtk_widget_set_vexpand(leftCard, TRUE);
    gtk_grid_attach(GTK_GRID(grid), leftCard, 0, 0, 1, 1);

    // Right Panel: Payment & Checkout
    GtkWidget* payCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(payCard, "card");
    gtk_widget_set_size_request(payCard, 384, -1);

    GtkWidget* lblPayTitle = gtk_label_new("Thanh toán hóa đơn");
    gtk_widget_add_css_class(lblPayTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblPayTitle), 0.0f);
    gtk_box_append(GTK_BOX(payCard), lblPayTitle);

    // Totals labels
    posLblSubtotal = gtk_label_new("Tạm tính: 0 ₫");
    gtk_widget_add_css_class(posLblSubtotal, "stat-label");
    gtk_label_set_xalign(GTK_LABEL(posLblSubtotal), 0.0f);
    gtk_box_append(GTK_BOX(payCard), posLblSubtotal);

    // Coupon
    GtkWidget* couponRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    posEntryCoupon = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(posEntryCoupon), "Mã voucher...");
    gtk_widget_set_hexpand(posEntryCoupon, TRUE);

    GtkWidget* btnAppCoupon = gtk_button_new_with_label("Áp dụng");
    gtk_widget_add_css_class(btnAppCoupon, "btn-secondary");
    UIComponents::setButtonCursor(btnAppCoupon);
    g_signal_connect_data(btnAppCoupon, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        SalesScreens* self = static_cast<SalesScreens*>(d);
        self->posAppliedCoupon = gtk_editable_get_text(GTK_EDITABLE(self->posEntryCoupon));
        self->refreshPos();
    }), this, nullptr, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(couponRow), posEntryCoupon);
    gtk_box_append(GTK_BOX(couponRow), btnAppCoupon);
    gtk_box_append(GTK_BOX(payCard), UIComponents::createFormField("Mã giảm giá", couponRow));

    posLblDiscount = gtk_label_new("Giảm giá: 0 ₫");
    gtk_widget_add_css_class(posLblDiscount, "stat-label");
    gtk_label_set_xalign(GTK_LABEL(posLblDiscount), 0.0f);
    gtk_box_append(GTK_BOX(payCard), posLblDiscount);

    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(payCard), sep);

    GtkWidget* totalBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class(totalBox, "pos-total-box");
    GtkWidget* totalCaption = gtk_label_new("TỔNG THANH TOÁN");
    gtk_widget_add_css_class(totalCaption, "pos-total-label");
    gtk_label_set_xalign(GTK_LABEL(totalCaption), 0.0f);
    gtk_box_append(GTK_BOX(totalBox), totalCaption);
    posLblGrandTotal = gtk_label_new("0 ₫");
    gtk_widget_add_css_class(posLblGrandTotal, "pos-total-value");
    gtk_label_set_xalign(GTK_LABEL(posLblGrandTotal), 1.0f);
    gtk_box_append(GTK_BOX(totalBox), posLblGrandTotal);
    gtk_box_append(GTK_BOX(payCard), totalBox);

    const char* payMethods[] = {"Tiền mặt", "Chuyển khoản", nullptr};
    posComboPayMethod = gtk_drop_down_new_from_strings(payMethods);
    gtk_box_append(GTK_BOX(payCard), UIComponents::createFormField("Phương thức thanh toán", posComboPayMethod));

    posEntryCash = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(posEntryCash), "Nhập số tiền khách đưa...");
    UIComponents::setupCurrencyInput(posEntryCash);
    gtk_box_append(GTK_BOX(payCard), UIComponents::createFormField("Tiền khách đưa (nếu tiền mặt)", posEntryCash));

    posLblChange = gtk_label_new("Tiền thừa trả khách: 0 ₫");
    gtk_widget_add_css_class(posLblChange, "stat-trend");
    gtk_label_set_xalign(GTK_LABEL(posLblChange), 0.0f);
    gtk_box_append(GTK_BOX(payCard), posLblChange);

    g_signal_connect_data(posEntryCash, "changed", G_CALLBACK(+[](GtkEditable* e, gpointer d) {
        SalesScreens* self = static_cast<SalesScreens*>(d);
        double cashGiven = UIComponents::parseCurrencyInput(GTK_WIDGET(e));
        double subtotal = 0.0;
        for (const auto& item : self->posCartItems) subtotal += item.subtotal;
        double discount = 0.0;
        if (!self->posAppliedCoupon.empty()) {
            self->controller->validateDiscount(self->posAppliedCoupon, subtotal, discount);
        }
        double grandTotal = subtotal - discount;
        if (grandTotal < 0) grandTotal = 0;
        double change = cashGiven - grandTotal;
        if (change < 0 || cashGiven == 0) change = 0.0;
        gtk_label_set_text(GTK_LABEL(self->posLblChange), ("Tiền thừa trả khách: " + UIComponents::formatVND(change)).c_str());
    }), this, nullptr, (GConnectFlags)0);

    // Confirm Payment Button
    GtkWidget* btnSubmitPos = gtk_button_new_with_label("Thanh toán & Xuất HĐ (F9)");
    gtk_widget_add_css_class(btnSubmitPos, "btn-success");
    gtk_widget_add_css_class(btnSubmitPos, "btn-pay");
    UIComponents::setButtonCursor(btnSubmitPos);
    gtk_widget_set_margin_top(btnSubmitPos, 10);
    gtk_widget_set_vexpand(btnSubmitPos, TRUE);
    gtk_widget_set_valign(btnSubmitPos, GTK_ALIGN_END);
    posBtnPay = btnSubmitPos;

    g_signal_connect_data(btnSubmitPos, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        SalesScreens* self = static_cast<SalesScreens*>(d);
        if (self->posCartItems.empty()) {
            UIComponents::showWarningDialog(self->shell->getWindow(), "Giỏ hàng rỗng", "Vui lòng thêm sản phẩm vào giao dịch!");
            return;
        }

        guint custSel = gtk_drop_down_get_selected(GTK_DROP_DOWN(self->posComboCustomer));
        std::string custId = "WALK-IN";
        std::vector<CustomerItemViewModel> custs = self->controller->getCustomers();
        if (custSel > 0 && custSel <= custs.size()) {
            custId = custs[custSel - 1].id;
        }

        guint paySel = gtk_drop_down_get_selected(GTK_DROP_DOWN(self->posComboPayMethod));
        PaymentMethod payMethod = (paySel == 0) ? PaymentMethod::CASH : PaymentMethod::BANK_TRANSFER;

        double cashGiven = UIComponents::parseCurrencyInput(self->posEntryCash);

        double change = 0.0;
        std::string invId;
        const UserSessionViewModel& curUser = self->controller->getCurrentUser();
        OperationResult res = self->controller->createPosInvoice(
            custId, curUser.userId, self->posCartItems, self->posAppliedCoupon,
            payMethod, cashGiven, change, invId
        );

        if (res.success) {
            self->posCartItems.clear();
            self->posAppliedCoupon = "";
            self->refreshPos();
            self->refreshInvoices();
            std::string msg = res.message + "\nTiền thừa trả khách: " + UIComponents::formatVND(change);
            UIComponents::showSuccessDialog(self->shell->getWindow(), "Hoàn tất thanh toán", msg);
        } else {
            UIComponents::showErrorDialog(self->shell->getWindow(), "Lỗi thanh toán", res.message);
        }
    }), this, nullptr, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(payCard), btnSubmitPos);

    gtk_grid_attach(GTK_GRID(grid), payCard, 1, 0, 1, 1);
    gtk_widget_set_vexpand(grid, TRUE);
    gtk_box_append(GTK_BOX(content), grid);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(posView), content);

    // Phím tắt (chỉ hoạt động khi focus nằm trong màn hình POS)
    GtkEventController* keys = gtk_event_controller_key_new();
    gtk_event_controller_set_propagation_phase(keys, GTK_PHASE_CAPTURE);
    g_signal_connect(keys, "key-pressed", G_CALLBACK(+[](GtkEventControllerKey*, guint keyval, guint, GdkModifierType, gpointer d) -> gboolean {
        SalesScreens* self = static_cast<SalesScreens*>(d);
        if (keyval == GDK_KEY_F2) { self->showPosProductPickerModal(); return TRUE; }
        if (keyval == GDK_KEY_F9) { g_signal_emit_by_name(self->posBtnPay, "clicked"); return TRUE; }
        if (keyval == GDK_KEY_Escape) {
            if (!self->posCartItems.empty()) {
                UIComponents::showConfirmDialog(self->shell->getWindow(), "Xác nhận xóa giỏ hàng",
                    "Bạn có chắc chắn muốn xóa toàn bộ sản phẩm trong giỏ hàng POS hiện tại?",
                    +[](gpointer data) {
                        SalesScreens* s = static_cast<SalesScreens*>(data);
                        s->posCartItems.clear();
                        s->posAppliedCoupon = "";
                        s->refreshPos();
                    }, self);
            }
            return TRUE;
        }
        return FALSE;
    }), this);
    gtk_widget_add_controller(posView, keys);
    refreshPos();
}

void SalesScreens::refreshPos() {
    GtkWidget* child = gtk_widget_get_first_child(posCartListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(posCartListBox), child);
        child = next;
    }

    if (posCartItems.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState("", "Chưa có sản phẩm nào",
            "Quét mã, gõ tên sản phẩm rồi nhấn Enter, hoặc bấm F2 để chọn.", "Chọn sản phẩm",
            G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                SalesScreens* self = static_cast<SalesScreens*>(d);
                self->showPosProductPickerModal();
            }), this);
        gtk_box_append(GTK_BOX(posCartListBox), empty);
        gtk_label_set_text(GTK_LABEL(posLblSubtotal), "Tạm tính: 0 ₫");
        gtk_label_set_text(GTK_LABEL(posLblDiscount), "Giảm giá: 0 ₫");
        gtk_label_set_text(GTK_LABEL(posLblGrandTotal), "0 ₫");
        return;
    }

    double subtotal = 0.0;
    for (size_t i = 0; i < posCartItems.size(); ++i) {
        const auto& item = posCartItems[i];
        subtotal += item.subtotal;

        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        GtkWidget* nameLbl = gtk_label_new(item.phoneName.c_str());
        gtk_widget_add_css_class(nameLbl, "product-name");
        gtk_widget_set_hexpand(nameLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(nameLbl), 0.0f);
        gtk_box_append(GTK_BOX(row), nameLbl);

        GtkWidget* priceLbl = gtk_label_new(UIComponents::formatVND(item.unitPrice).c_str());
        gtk_widget_add_css_class(priceLbl, "stat-label");
        gtk_label_set_xalign(GTK_LABEL(priceLbl), 1.0f);
        gtk_widget_set_size_request(priceLbl, 120, -1);
        gtk_box_append(GTK_BOX(row), priceLbl);

        GtkWidget* qtyLbl = gtk_label_new(("x" + std::to_string(item.quantity)).c_str());
        gtk_widget_add_css_class(qtyLbl, "stat-label");
        gtk_label_set_xalign(GTK_LABEL(qtyLbl), 1.0f);
        gtk_widget_set_size_request(qtyLbl, 48, -1);
        gtk_box_append(GTK_BOX(row), qtyLbl);

        GtkWidget* subLbl = gtk_label_new(UIComponents::formatVND(item.subtotal).c_str());
        gtk_widget_add_css_class(subLbl, "product-price");
        gtk_label_set_xalign(GTK_LABEL(subLbl), 1.0f);
        gtk_widget_set_size_request(subLbl, 130, -1);
        gtk_box_append(GTK_BOX(row), subLbl);

        // Remove item
        GtkWidget* btnDel = UIComponents::createTableActionButton("✕ Xóa", "Xóa sản phẩm khỏi giỏ hàng", "btn-action-delete");
        gtk_widget_set_size_request(btnDel, 70, -1);

        struct PosDelCtx { SalesScreens* screens; size_t index; };
        PosDelCtx* pCtx = new PosDelCtx{this, i};
        g_signal_connect_data(btnDel, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            PosDelCtx* pd = static_cast<PosDelCtx*>(d);
            if (pd->index < pd->screens->posCartItems.size()) {
                pd->screens->posCartItems.erase(pd->screens->posCartItems.begin() + pd->index);
                pd->screens->refreshPos();
            }
        }), pCtx, [](gpointer d, GClosure*) { delete static_cast<PosDelCtx*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), btnDel);
        gtk_box_append(GTK_BOX(posCartListBox), row);
    }

    double discount = 0.0;
    if (!posAppliedCoupon.empty()) {
        controller->validateDiscount(posAppliedCoupon, subtotal, discount);
    }

    double grandTotal = subtotal - discount;
    if (grandTotal < 0) grandTotal = 0;

    gtk_label_set_text(GTK_LABEL(posLblSubtotal), ("Tạm tính: " + UIComponents::formatVND(subtotal)).c_str());
    gtk_label_set_text(GTK_LABEL(posLblDiscount), ("Giảm giá: -" + UIComponents::formatVND(discount)).c_str());
    gtk_label_set_text(GTK_LABEL(posLblGrandTotal), UIComponents::formatVND(grandTotal).c_str());

    double cashGiven = UIComponents::parseCurrencyInput(posEntryCash);
    double change = cashGiven - grandTotal;
    if (change < 0 || cashGiven == 0) change = 0.0;
    gtk_label_set_text(GTK_LABEL(posLblChange), ("Tiền thừa trả khách: " + UIComponents::formatVND(change)).c_str());
}

void SalesScreens::showPosProductPickerModal() {
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), "Chọn sản phẩm vào hóa đơn");
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 560, 480);

    GtkWidget* scroll = gtk_scrolled_window_new();
    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    GtkWidget* lblTitle = gtk_label_new("Danh sách sản phẩm sẵn có");
    gtk_widget_add_css_class(lblTitle, "page-title");
    gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
    gtk_box_append(GTK_BOX(card), lblTitle);

    std::vector<ProductItemViewModel> prods = controller->getProducts();
    for (size_t i = 0; i < prods.size(); ++i) {
        const auto& p = prods[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        if (!p.imageUrl.empty() && g_file_test(p.imageUrl.c_str(), G_FILE_TEST_EXISTS)) {
            GtkWidget* thumb = gtk_image_new_from_file(p.imageUrl.c_str());
            gtk_image_set_pixel_size(GTK_IMAGE(thumb), 36);
            gtk_box_append(GTK_BOX(row), thumb);
        } else {
            GtkWidget* ico = gtk_label_new("📱");
            gtk_box_append(GTK_BOX(row), ico);
        }

        GtkWidget* n = gtk_label_new(p.name.c_str());
        gtk_widget_add_css_class(n, "product-name");
        gtk_widget_set_hexpand(n, TRUE);
        gtk_label_set_xalign(GTK_LABEL(n), 0.0f);
        gtk_box_append(GTK_BOX(row), n);

        GtkWidget* pr = gtk_label_new(UIComponents::formatVND(p.sellingPrice).c_str());
        gtk_widget_add_css_class(pr, "product-price");
        gtk_label_set_xalign(GTK_LABEL(pr), 1.0f);
        gtk_box_append(GTK_BOX(row), pr);

        GtkWidget* st = gtk_label_new(("Còn: " + std::to_string(p.availableStock)).c_str());
        gtk_widget_add_css_class(st, "stat-label");
        gtk_box_append(GTK_BOX(row), st);

        GtkWidget* btnAdd = gtk_button_new_with_label("+ Thêm");
        if (p.availableStock <= 0) {
            gtk_widget_set_sensitive(btnAdd, FALSE);
        } else {
            gtk_widget_add_css_class(btnAdd, "btn-primary");
            UIComponents::setButtonCursor(btnAdd);
        }

        struct PickCtx { SalesScreens* screens; ProductItemViewModel prod; GtkWidget* dialog; };
        PickCtx* pk = new PickCtx{this, p, win};
        g_signal_connect_data(btnAdd, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            PickCtx* pdata = static_cast<PickCtx*>(d);

            bool found = false;
            for (auto& item : pdata->screens->posCartItems) {
                if (item.phoneId == pdata->prod.id) {
                    item.quantity += 1;
                    item.calculateSubtotal();
                    found = true;
                    break;
                }
            }
            if (!found) {
                CartItemViewModel ci;
                ci.phoneId = pdata->prod.id;
                ci.phoneName = pdata->prod.name;
                ci.brand = pdata->prod.brand;
                ci.unitPrice = pdata->prod.sellingPrice;
                ci.quantity = 1;
                ci.availableStock = pdata->prod.availableStock;
                ci.calculateSubtotal();
                pdata->screens->posCartItems.push_back(ci);
            }

            pdata->screens->refreshPos();
            gtk_window_destroy(GTK_WINDOW(pdata->dialog));
        }), pk, [](gpointer d, GClosure*) { delete static_cast<PickCtx*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), btnAdd);
        gtk_box_append(GTK_BOX(card), row);
    }

    GtkWidget* btnClose = gtk_button_new_with_label("Đóng");
    gtk_widget_add_css_class(btnClose, "btn-secondary");
    UIComponents::setButtonCursor(btnClose);
    gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
    g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(card), btnClose);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), card);
    gtk_window_set_child(GTK_WINDOW(win), scroll);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 3. TRA CỨU SẢN PHẨM =================

void SalesScreens::buildProductLookupView() {
    productLookupView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productLookupView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Tra cứu sản phẩm & Giá bán", "Tra cứu nhanh thông số kỹ thuật, giá niêm yết và tồn kho khả dụng.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* searchBar = UIComponents::createSearchBar("Nhập tên, thương hiệu hoặc mã điện thoại...", G_CALLBACK(+[](GtkEditable* e, gpointer d) {
        SalesScreens* self = static_cast<SalesScreens*>(d);
        self->refreshProductLookup(gtk_editable_get_text(e));
    }), this);
    gtk_box_append(GTK_BOX(content), searchBar);

    std::vector<UIComponents::TableColumnSpec> prodCols = {
        {"Tên thiết bị & Cấu hình", 0, 0.0f, true},
        {"Trạng thái", 130, 0.5f, false},
        {"Tồn khả dụng", 120, 1.0f, false},
        {"Giá niêm yết", 140, 1.0f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(prodCols));

    productLookupListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), productLookupListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(productLookupView), content);
    refreshProductLookup();
}

void SalesScreens::refreshProductLookup(const std::string& keyword) {
    GtkWidget* child = gtk_widget_get_first_child(productLookupListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(productLookupListBox), child);
        child = next;
    }

    ProductFilterRequest req;
    req.keyword = keyword;
    std::vector<ProductItemViewModel> prods = controller->getProducts(req);

    if (prods.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState("🔍", "Không tìm thấy sản phẩm", "Thử tìm kiếm với từ khóa khác.");
        gtk_box_append(GTK_BOX(productLookupListBox), empty);
        return;
    }

    for (size_t i = 0; i < prods.size(); ++i) {
        const auto& p = prods[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        GtkWidget* info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_hexpand(info, TRUE);
        gtk_widget_set_valign(info, GTK_ALIGN_CENTER);

        GtkWidget* nameLbl = gtk_label_new(p.name.c_str());
        gtk_widget_add_css_class(nameLbl, "product-name");
        gtk_label_set_xalign(GTK_LABEL(nameLbl), 0.0f);

        GtkWidget* specLbl = gtk_label_new(p.specs.c_str());
        gtk_widget_add_css_class(specLbl, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(specLbl), 0.0f);

        gtk_box_append(GTK_BOX(info), nameLbl);
        gtk_box_append(GTK_BOX(info), specLbl);
        gtk_box_append(GTK_BOX(row), info);

        // Trạng thái badge
        GtkWidget* badge = UIComponents::createStockStatusBadge(p.status);
        GtkWidget* badgeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(badgeBox, 130, -1);
        gtk_widget_set_valign(badgeBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(badgeBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(badgeBox), badge);
        gtk_box_append(GTK_BOX(row), badgeBox);

        // Tồn khả dụng (căn phải)
        GtkWidget* stLbl = gtk_label_new((std::to_string(p.availableStock) + " máy").c_str());
        gtk_widget_add_css_class(stLbl, "stat-label");
        gtk_widget_set_size_request(stLbl, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(stLbl), 1.0f);
        gtk_widget_set_valign(stLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), stLbl);

        // Giá niêm yết (căn phải)
        GtkWidget* prLbl = gtk_label_new(UIComponents::formatVND(p.sellingPrice).c_str());
        gtk_widget_add_css_class(prLbl, "product-price");
        gtk_widget_set_size_request(prLbl, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(prLbl), 1.0f);
        gtk_widget_set_valign(prLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), prLbl);

        gtk_box_append(GTK_BOX(productLookupListBox), row);
    }
}

// ================= 4. QUẢN LÝ KHÁCH HÀNG =================

void SalesScreens::buildCustomerMgmtView() {
    customerMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(customerMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* btnNew = gtk_button_new_with_label("➕ Thêm khách hàng mới");
    gtk_widget_add_css_class(btnNew, "btn-primary");
    UIComponents::setButtonCursor(btnNew);
    g_signal_connect_swapped(btnNew, "clicked", G_CALLBACK(+[](SalesScreens* s) {
        s->showCustomerModal(nullptr);
    }), this);

    GtkWidget* header = UIComponents::createSectionHeader("Quản lý thông tin khách hàng", "Tìm kiếm, thêm mới và theo dõi lịch sử mua hàng, điểm tích lũy.", btnNew);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* searchBar = UIComponents::createSearchBar("Tìm theo tên, SĐT hoặc mã khách hàng...", G_CALLBACK(+[](GtkEditable* e, gpointer d) {
        SalesScreens* self = static_cast<SalesScreens*>(d);
        self->refreshCustomers(gtk_editable_get_text(e));
    }), this);
    gtk_box_append(GTK_BOX(content), searchBar);

    std::vector<UIComponents::TableColumnSpec> custCols = {
        {"Mã KH", 110, 0.0f, false},
        {"Họ và tên", 180, 0.0f, false},
        {"Thông tin liên hệ (SĐT / Email / Địa chỉ)", 0, 0.0f, true},
        {"Hạng TV", 130, 0.5f, false},
        {"Điểm", 100, 1.0f, false},
        {"Thao tác", 90, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(custCols));

    customerListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), customerListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(customerMgmtView), content);
    refreshCustomers();
}

void SalesScreens::refreshCustomers(const std::string& keyword) {
    GtkWidget* child = gtk_widget_get_first_child(customerListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(customerListBox), child);
        child = next;
    }

    std::vector<CustomerItemViewModel> custs = controller->getCustomers(keyword);

    if (custs.empty()) {
        std::string emptyMsg = keyword.empty() ? "Chưa có thông tin khách hàng nào trong hệ thống." : "Không tìm thấy khách hàng nào khớp với từ khóa tìm kiếm.";
        GtkWidget* empty = UIComponents::createEmptyState(
            "👥", "Không tìm thấy khách hàng", emptyMsg,
            "➕ Đăng ký khách hàng mới",
            G_CALLBACK(+[](GtkButton*, gpointer d) {
                SalesScreens* s = static_cast<SalesScreens*>(d);
                s->showCustomerModal(nullptr);
            }), this);
        gtk_box_append(GTK_BOX(customerListBox), empty);
        return;
    }

    for (size_t i = 0; i < custs.size(); ++i) {
        const auto& c = custs[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Mã KH
        GtkWidget* idLbl = gtk_label_new(c.id.c_str());
        gtk_widget_add_css_class(idLbl, "product-name");
        gtk_widget_set_size_request(idLbl, 110, -1);
        gtk_label_set_xalign(GTK_LABEL(idLbl), 0.0f);
        gtk_widget_set_valign(idLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), idLbl);

        // Họ và tên
        GtkWidget* nameLbl = gtk_label_new(c.fullName.c_str());
        gtk_widget_add_css_class(nameLbl, "product-name");
        gtk_widget_set_size_request(nameLbl, 180, -1);
        gtk_label_set_xalign(GTK_LABEL(nameLbl), 0.0f);
        gtk_widget_set_valign(nameLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), nameLbl);

        // Liên hệ
        GtkWidget* contactBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_hexpand(contactBox, TRUE);
        gtk_widget_set_valign(contactBox, GTK_ALIGN_CENTER);

        GtkWidget* phoneLbl = gtk_label_new(("SĐT: " + c.phone + (c.email.empty() ? "" : (" | " + c.email))).c_str());
        gtk_label_set_xalign(GTK_LABEL(phoneLbl), 0.0f);
        gtk_box_append(GTK_BOX(contactBox), phoneLbl);

        if (!c.address.empty()) {
            GtkWidget* addrLbl = gtk_label_new(("ĐC: " + c.address).c_str());
            gtk_widget_add_css_class(addrLbl, "page-subtitle");
            gtk_label_set_xalign(GTK_LABEL(addrLbl), 0.0f);
            gtk_box_append(GTK_BOX(contactBox), addrLbl);
        }
        gtk_box_append(GTK_BOX(row), contactBox);

        // Hạng TV badge
        GtkWidget* badgeTier = UIComponents::createRankBadge(c.membershipTier);
        GtkWidget* tierBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(tierBox, 130, -1);
        gtk_widget_set_valign(tierBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(tierBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(tierBox), badgeTier);
        gtk_box_append(GTK_BOX(row), tierBox);

        // Điểm tích lũy (căn phải)
        GtkWidget* ptLbl = gtk_label_new((std::to_string(c.points) + " điểm").c_str());
        gtk_widget_add_css_class(ptLbl, "stat-label");
        gtk_widget_set_size_request(ptLbl, 100, -1);
        gtk_label_set_xalign(GTK_LABEL(ptLbl), 1.0f);
        gtk_widget_set_valign(ptLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), ptLbl);

        // Nút Sửa
        GtkWidget* btnEdit = UIComponents::createTableActionButton("✏️ Sửa", "Chỉnh sửa thông tin khách hàng", "btn-action-edit");
        gtk_widget_set_size_request(btnEdit, 90, -1);

        struct CustCtx { SalesScreens* screens; CustomerItemViewModel item; };
        CustCtx* eCtx = new CustCtx{this, c};
        g_signal_connect_data(btnEdit, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            CustCtx* cx = static_cast<CustCtx*>(d);
            cx->screens->showCustomerModal(&cx->item);
        }), eCtx, [](gpointer d, GClosure*) { delete static_cast<CustCtx*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), btnEdit);
        gtk_box_append(GTK_BOX(customerListBox), row);
    }
}

void SalesScreens::showCustomerModal(const CustomerItemViewModel* existingCust) {
    bool isEdit = (existingCust != nullptr);
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), isEdit ? "Chỉnh sửa khách hàng" : "Thêm khách hàng mới");
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 440, 480);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    GtkWidget* title = gtk_label_new(isEdit ? "Cập nhật thông tin khách hàng" : "Đăng ký khách hàng mới");
    gtk_widget_add_css_class(title, "page-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(card), title);

    GtkWidget* eFirst = gtk_entry_new();
    GtkWidget* eMid = gtk_entry_new();
    GtkWidget* eLast = gtk_entry_new();
    GtkWidget* ePhone = gtk_entry_new();
    GtkWidget* eEmail = gtk_entry_new();
    GtkWidget* eAddr = gtk_entry_new();

    if (isEdit && existingCust) {
        gtk_editable_set_text(GTK_EDITABLE(eFirst), existingCust->firstName.c_str());
        gtk_editable_set_text(GTK_EDITABLE(eMid), existingCust->middleName.c_str());
        gtk_editable_set_text(GTK_EDITABLE(eLast), existingCust->lastName.c_str());
        gtk_editable_set_text(GTK_EDITABLE(ePhone), existingCust->phone.c_str());
        gtk_editable_set_text(GTK_EDITABLE(eEmail), existingCust->email.c_str());
        gtk_editable_set_text(GTK_EDITABLE(eAddr), existingCust->address.c_str());
    }

    UIComponents::setupPhoneInput(ePhone);

    GtkWidget* errFirst = nullptr;
    GtkWidget* errLast = nullptr;
    GtkWidget* errPhone = nullptr;

    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Tên *", eFirst, &errFirst));
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Tên đệm", eMid));
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Họ *", eLast, &errLast));
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Số điện thoại *", ePhone, &errPhone));
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Email", eEmail));
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Địa chỉ", eAddr));

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
    gtk_widget_set_margin_top(btnRow, 10);

    GtkWidget* btnCancel = gtk_button_new_with_label("Hủy");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(btnRow), btnCancel);

    struct CustModalCtx {
        SalesScreens* screens;
        GtkWidget* win;
        bool isEdit;
        std::string custId;
        GtkWidget* eFirst;
        GtkWidget* eMid;
        GtkWidget* eLast;
        GtkWidget* ePhone;
        GtkWidget* eEmail;
        GtkWidget* eAddr;
        GtkWidget* errFirst;
        GtkWidget* errLast;
        GtkWidget* errPhone;
    };
    CustModalCtx* cm = new CustModalCtx{
        this, win, isEdit, isEdit ? existingCust->id : "",
        eFirst, eMid, eLast, ePhone, eEmail, eAddr,
        errFirst, errLast, errPhone
    };

    GtkWidget* btnSave = gtk_button_new_with_label(isEdit ? "Lưu thay đổi" : "Tạo khách hàng");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    g_signal_connect_data(btnSave, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        CustModalCtx* c = static_cast<CustModalCtx*>(d);
        UIComponents::clearFieldError(c->errFirst, c->eFirst);
        UIComponents::clearFieldError(c->errLast, c->eLast);
        UIComponents::clearFieldError(c->errPhone, c->ePhone);

        CustomerFormRequest req;
        req.id = c->custId;
        req.firstName = gtk_editable_get_text(GTK_EDITABLE(c->eFirst));
        req.middleName = gtk_editable_get_text(GTK_EDITABLE(c->eMid));
        req.lastName = gtk_editable_get_text(GTK_EDITABLE(c->eLast));
        req.phone = gtk_editable_get_text(GTK_EDITABLE(c->ePhone));
        req.email = gtk_editable_get_text(GTK_EDITABLE(c->eEmail));
        req.address = gtk_editable_get_text(GTK_EDITABLE(c->eAddr));

        bool hasError = false;
        GtkWidget* firstErr = nullptr;
        if (req.firstName.empty()) {
            UIComponents::setFieldError(c->errFirst, c->eFirst, "Tên không được để trống");
            hasError = true;
            if (!firstErr) firstErr = c->eFirst;
        }
        if (req.lastName.empty()) {
            UIComponents::setFieldError(c->errLast, c->eLast, "Họ không được để trống");
            hasError = true;
            if (!firstErr) firstErr = c->eLast;
        }
        std::string phoneErr;
        if (!UIComponents::validatePhoneNumber(req.phone, phoneErr)) {
            UIComponents::setFieldError(c->errPhone, c->ePhone, phoneErr);
            hasError = true;
            if (!firstErr) firstErr = c->ePhone;
        }

        if (hasError) {
            if (firstErr) gtk_widget_grab_focus(firstErr);
            return;
        }

        OperationResult res = c->isEdit ? c->screens->controller->updateCustomer(req)
                                        : c->screens->controller->addCustomer(req);
        if (res.success) {
            c->screens->refreshCustomers();
            UIComponents::showSuccessDialog(c->screens->shell->getWindow(), "Thành công", res.message);
            gtk_window_destroy(GTK_WINDOW(c->win));
        } else {
            if (res.message.find("điện thoại") != std::string::npos) {
                UIComponents::setFieldError(c->errPhone, c->ePhone, res.message);
                gtk_widget_grab_focus(c->ePhone);
            } else {
                UIComponents::showErrorDialog(c->screens->shell->getWindow(), "Lỗi", res.message);
            }
        }
    }), cm, [](gpointer d, GClosure*) { delete static_cast<CustModalCtx*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(btnRow), btnSave);
    gtk_box_append(GTK_BOX(card), btnRow);

    gtk_window_set_child(GTK_WINDOW(win), card);
    UIComponents::setupDialogShortcuts(win, btnSave, btnCancel);
    gtk_widget_grab_focus(eFirst);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 5. DANH SÁCH HÓA ĐƠN =================

void SalesScreens::buildInvoicesView() {
    invoicesView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(invoicesView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Danh sách hóa đơn thanh toán", "Tra cứu hóa đơn bán hàng tại quầy và đơn online đã thanh toán.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* searchBar = UIComponents::createSearchBar("Tìm theo mã hóa đơn, mã đơn hoặc khách hàng...", G_CALLBACK(+[](GtkEditable* e, gpointer d) {
        SalesScreens* self = static_cast<SalesScreens*>(d);
        self->refreshInvoices(gtk_editable_get_text(e));
    }), this);
    gtk_box_append(GTK_BOX(content), searchBar);

    std::vector<UIComponents::TableColumnSpec> invCols = {
        {"Mã HĐ", 120, 0.0f, false},
        {"Mã đơn", 120, 0.0f, false},
        {"Khách hàng", 170, 0.0f, false},
        {"Hình thức TT", 140, 0.5f, false},
        {"Thời gian", 0, 0.0f, true},
        {"Tổng tiền", 140, 1.0f, false},
        {"Thao tác", 90, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(invCols));

    invoiceListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), invoiceListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(invoicesView), content);
    refreshInvoices();
}

void SalesScreens::refreshInvoices(const std::string& keyword) {
    GtkWidget* child = gtk_widget_get_first_child(invoiceListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(invoiceListBox), child);
        child = next;
    }

    std::vector<InvoiceItemViewModel> invs = controller->getAllInvoices();
    std::vector<InvoiceItemViewModel> filtered;
    for (const auto& inv : invs) {
        if (!keyword.empty()) {
            if (inv.id.find(keyword) == std::string::npos &&
                inv.orderId.find(keyword) == std::string::npos &&
                inv.customerName.find(keyword) == std::string::npos) {
                continue;
            }
        }
        filtered.push_back(inv);
    }

    if (filtered.empty()) {
        gtk_box_append(GTK_BOX(invoiceListBox), UIComponents::createEmptyState("🧾", "Chưa có dữ liệu", "Không có hóa đơn nào khớp với từ khóa tìm kiếm."));
        return;
    }

    for (size_t i = 0; i < filtered.size(); ++i) {
        const auto& inv = filtered[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Cột Mã HĐ
        GtkWidget* idLbl = gtk_label_new(inv.id.c_str());
        gtk_widget_add_css_class(idLbl, "product-name");
        gtk_widget_set_size_request(idLbl, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(idLbl), 0.0f);
        gtk_widget_set_valign(idLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), idLbl);

        // Cột Mã đơn
        GtkWidget* ordLbl = gtk_label_new(inv.orderId.c_str());
        gtk_widget_set_size_request(ordLbl, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(ordLbl), 0.0f);
        gtk_widget_set_valign(ordLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), ordLbl);

        // Cột Khách hàng
        GtkWidget* custLbl = gtk_label_new(inv.customerName.c_str());
        gtk_widget_set_size_request(custLbl, 170, -1);
        gtk_label_set_xalign(GTK_LABEL(custLbl), 0.0f);
        gtk_widget_set_valign(custLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), custLbl);

        // Cột Phương thức (Badge)
        GtkWidget* payBadge = UIComponents::createStatusBadge(inv.paymentMethodText, "badge-confirmed");
        GtkWidget* payBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(payBox, 140, -1);
        gtk_widget_set_valign(payBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(payBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(payBox), payBadge);
        gtk_box_append(GTK_BOX(row), payBox);

        // Cột Thời gian
        GtkWidget* timeLbl = gtk_label_new(inv.createTime.c_str());
        gtk_widget_set_hexpand(timeLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(timeLbl), 0.0f);
        gtk_widget_set_valign(timeLbl, GTK_ALIGN_CENTER);
        gtk_widget_add_css_class(timeLbl, "page-subtitle");
        gtk_box_append(GTK_BOX(row), timeLbl);

        // Cột Tổng tiền (căn phải)
        GtkWidget* totalLbl = gtk_label_new(UIComponents::formatVND(inv.total).c_str());
        gtk_widget_add_css_class(totalLbl, "product-price");
        gtk_widget_set_size_request(totalLbl, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(totalLbl), 1.0f);
        gtk_widget_set_valign(totalLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), totalLbl);

        // Cột Thao tác
        GtkWidget* btnDetail = UIComponents::createTableActionButton("🔍 Xem", "Xem biên nhận chi tiết hóa đơn", "btn-action-edit");
        gtk_widget_set_size_request(btnDetail, 90, -1);

        struct InvCtx { SalesScreens* screens; InvoiceItemViewModel item; };
        InvCtx* ivCtx = new InvCtx{this, inv};
        g_signal_connect_data(btnDetail, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            InvCtx* ic = static_cast<InvCtx*>(d);
            ic->screens->showInvoiceDetailModal(ic->item);
        }), ivCtx, [](gpointer d, GClosure*) { delete static_cast<InvCtx*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), btnDetail);
        gtk_box_append(GTK_BOX(invoiceListBox), row);
    }
}

void SalesScreens::showInvoiceDetailModal(const InvoiceItemViewModel& inv) {
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), ("Chi tiết hóa đơn — " + inv.id).c_str());
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 480, 420);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    GtkWidget* title = gtk_label_new(("HÓA ĐƠN BÁN HÀNG " + inv.id).c_str());
    gtk_widget_add_css_class(title, "page-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(card), title);

    auto addRow = [card](const char* l, const std::string& v) {
        GtkWidget* r = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* lbl = gtk_label_new(l);
        gtk_widget_add_css_class(lbl, "stat-label");
        gtk_widget_set_size_request(lbl, 130, -1);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);

        GtkWidget* val = gtk_label_new(v.c_str());
        gtk_label_set_xalign(GTK_LABEL(val), 0.0f);
        gtk_box_append(GTK_BOX(r), lbl);
        gtk_box_append(GTK_BOX(r), val);
        gtk_box_append(GTK_BOX(card), r);
    };

    addRow("Mã đơn gốc:", inv.orderId);
    addRow("Khách hàng:", inv.customerName);
    addRow("Thu ngân:", inv.employeeName);
    addRow("Thời gian:", inv.createTime);
    addRow("Phương thức:", inv.paymentMethodText);

    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(card), sep);

    for (const auto& d : inv.details) {
        GtkWidget* r = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* n = gtk_label_new(d.phoneName.c_str());
        gtk_widget_set_hexpand(n, TRUE);
        gtk_label_set_xalign(GTK_LABEL(n), 0.0f);

        GtkWidget* q = gtk_label_new(("x" + std::to_string(d.quantity)).c_str());
        GtkWidget* p = gtk_label_new(UIComponents::formatVND(d.subtotal).c_str());
        gtk_widget_add_css_class(p, "stat-label");

        gtk_box_append(GTK_BOX(r), n);
        gtk_box_append(GTK_BOX(r), q);
        gtk_box_append(GTK_BOX(r), p);
        gtk_box_append(GTK_BOX(card), r);
    }

    if (inv.discountAmount > 0) {
        addRow("Giảm giá:", "-" + UIComponents::formatVND(inv.discountAmount));
    }
    addRow("Tổng tiền:", UIComponents::formatVND(inv.total));

    GtkWidget* btnClose = gtk_button_new_with_label("Đóng");
    gtk_widget_add_css_class(btnClose, "btn-secondary");
    UIComponents::setButtonCursor(btnClose);
    gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
    g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(card), btnClose);

    gtk_window_set_child(GTK_WINDOW(win), card);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 6. QUẢN LÝ ĐƠN HÀNG ONLINE =================

void SalesScreens::buildOnlineOrdersView() {
    onlineOrdersView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(onlineOrdersView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Quản lý đơn hàng Online", "Tiếp nhận đơn, xác nhận đơn hàng và chuyển trạng thái xử lý.");
    gtk_box_append(GTK_BOX(content), header);

    std::vector<UIComponents::TableColumnSpec> ordCols = {
        {"Mã đơn", 120, 0.0f, false},
        {"Khách hàng / SĐT", 180, 0.0f, false},
        {"Giao nhận", 130, 0.0f, false},
        {"Thời gian", 140, 0.0f, false},
        {"Trạng thái", 130, 0.5f, false},
        {"Thanh toán", 130, 0.5f, false},
        {"Tổng tiền", 140, 1.0f, false},
        {"Xử lý đơn", 0, 0.5f, true}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(ordCols));

    ordersListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), ordersListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(onlineOrdersView), content);
    refreshOnlineOrders();
}

void SalesScreens::refreshOnlineOrders() {
    GtkWidget* child = gtk_widget_get_first_child(ordersListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(ordersListBox), child);
        child = next;
    }

    std::vector<OrderItemViewModel> orders = controller->getAllOrders();
    if (orders.empty()) {
        gtk_box_append(GTK_BOX(ordersListBox), UIComponents::createEmptyState("📦", "Chưa có dữ liệu", "Không có đơn hàng online nào cần xử lý."));
        return;
    }

    for (size_t i = 0; i < orders.size(); ++i) {
        const auto& ord = orders[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Mã đơn
        GtkWidget* idLbl = gtk_label_new(ord.id.c_str());
        gtk_widget_add_css_class(idLbl, "product-name");
        gtk_widget_set_size_request(idLbl, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(idLbl), 0.0f);
        gtk_widget_set_valign(idLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), idLbl);

        // Khách hàng / SĐT
        GtkWidget* custBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_size_request(custBox, 180, -1);
        gtk_widget_set_valign(custBox, GTK_ALIGN_CENTER);
        GtkWidget* cName = gtk_label_new(ord.customerName.c_str());
        gtk_label_set_xalign(GTK_LABEL(cName), 0.0f);
        GtkWidget* cPhone = gtk_label_new(("SĐT: " + ord.receiverPhone).c_str());
        gtk_widget_add_css_class(cPhone, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(cPhone), 0.0f);
        gtk_box_append(GTK_BOX(custBox), cName);
        gtk_box_append(GTK_BOX(custBox), cPhone);
        gtk_box_append(GTK_BOX(row), custBox);

        // Giao nhận
        GtkWidget* delivLbl = gtk_label_new(ord.deliveryMethodText.c_str());
        gtk_widget_set_size_request(delivLbl, 130, -1);
        gtk_label_set_xalign(GTK_LABEL(delivLbl), 0.0f);
        gtk_widget_set_valign(delivLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), delivLbl);

        // Thời gian
        GtkWidget* timeLbl = gtk_label_new(ord.orderTime.c_str());
        gtk_widget_set_size_request(timeLbl, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(timeLbl), 0.0f);
        gtk_widget_set_valign(timeLbl, GTK_ALIGN_CENTER);
        gtk_widget_add_css_class(timeLbl, "page-subtitle");
        gtk_box_append(GTK_BOX(row), timeLbl);

        // Trạng thái đơn (Badge)
        GtkWidget* stBadge = UIComponents::createOrderStatusBadge(ord.status);
        GtkWidget* stBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(stBox, 130, -1);
        gtk_widget_set_valign(stBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(stBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(stBox), stBadge);
        gtk_box_append(GTK_BOX(row), stBox);

        // Thanh toán (Badge)
        GtkWidget* payBadge = UIComponents::createPaymentStatusBadge(ord.paymentStatus);
        GtkWidget* payBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(payBox, 130, -1);
        gtk_widget_set_valign(payBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(payBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(payBox), payBadge);
        gtk_box_append(GTK_BOX(row), payBox);

        // Tổng tiền (căn phải)
        GtkWidget* totalLbl = gtk_label_new(UIComponents::formatVND(ord.total).c_str());
        gtk_widget_add_css_class(totalLbl, "product-price");
        gtk_widget_set_size_request(totalLbl, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(totalLbl), 1.0f);
        gtk_widget_set_valign(totalLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), totalLbl);

        // Action buttons based on status
        GtkWidget* actionsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_widget_set_hexpand(actionsBox, TRUE);
        gtk_widget_set_valign(actionsBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(actionsBox, GTK_ALIGN_CENTER);

        struct OrderActionCtx { SalesScreens* screens; std::string orderId; OrderStatus targetStatus; };

        if (ord.status == OrderStatus::PENDING) {
            GtkWidget* btnConfirm = UIComponents::createTableActionButton("✓ Xác nhận", "Xác nhận tiếp nhận và xử lý đơn hàng", "btn-primary");
            OrderActionCtx* act = new OrderActionCtx{this, ord.id, OrderStatus::CONFIRMED};
            g_signal_connect_data(btnConfirm, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                OrderActionCtx* a = static_cast<OrderActionCtx*>(d);
                OperationResult res = a->screens->controller->updateOrderStatus(a->orderId, a->targetStatus);
                if (res.success) {
                    a->screens->refreshOnlineOrders();
                    UIComponents::showSuccessDialog(a->screens->shell->getWindow(), "Đã cập nhật", res.message);
                } else {
                    UIComponents::showErrorDialog(a->screens->shell->getWindow(), "Lỗi", res.message);
                }
            }), act, [](gpointer d, GClosure*) { delete static_cast<OrderActionCtx*>(d); }, (GConnectFlags)0);
            gtk_box_append(GTK_BOX(actionsBox), btnConfirm);

            GtkWidget* btnCancel = UIComponents::createTableActionButton("✕ Hủy", "Hủy bỏ đơn hàng online này", "btn-action-delete");
            OrderActionCtx* actC = new OrderActionCtx{this, ord.id, OrderStatus::CANCELLED};
            g_signal_connect_data(btnCancel, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                OrderActionCtx* a = static_cast<OrderActionCtx*>(d);
                UIComponents::showConfirmDialog(a->screens->shell->getWindow(), "Hủy đơn hàng online",
                    "Bạn có chắc chắn muốn hủy đơn hàng " + a->orderId + " không?",
                    +[](gpointer data) {
                        OrderActionCtx* act = static_cast<OrderActionCtx*>(data);
                        act->screens->controller->cancelOrder(act->orderId);
                        act->screens->refreshOnlineOrders();
                    }, a);
            }), actC, [](gpointer d, GClosure*) { delete static_cast<OrderActionCtx*>(d); }, (GConnectFlags)0);
            gtk_box_append(GTK_BOX(actionsBox), btnCancel);
        } else if (ord.status == OrderStatus::SHIPPING || ord.status == OrderStatus::READY_FOR_PICKUP) {
            GtkWidget* btnComplete = UIComponents::createTableActionButton("✓ Hoàn tất giao", "Đánh dấu đơn hàng đã được giao thành công", "btn-primary");
            OrderActionCtx* act = new OrderActionCtx{this, ord.id, OrderStatus::COMPLETED};
            g_signal_connect_data(btnComplete, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                OrderActionCtx* a = static_cast<OrderActionCtx*>(d);
                OperationResult res = a->screens->controller->updateOrderStatus(a->orderId, a->targetStatus);
                if (res.success) {
                    a->screens->refreshOnlineOrders();
                    UIComponents::showSuccessDialog(a->screens->shell->getWindow(), "Đã hoàn thành", res.message);
                }
            }), act, [](gpointer d, GClosure*) { delete static_cast<OrderActionCtx*>(d); }, (GConnectFlags)0);
            gtk_box_append(GTK_BOX(actionsBox), btnComplete);
        } else {
            GtkWidget* noActionLbl = gtk_label_new("—");
            gtk_widget_add_css_class(noActionLbl, "page-subtitle");
            gtk_box_append(GTK_BOX(actionsBox), noActionLbl);
        }

        gtk_box_append(GTK_BOX(row), actionsBox);
        gtk_box_append(GTK_BOX(ordersListBox), row);
    }
}

// ================= 7. TIẾP NHẬN YÊU CẦU BẢO HÀNH =================

void SalesScreens::buildWarrantyIntakeView() {
    warrantyIntakeView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyIntakeView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Tiếp nhận yêu cầu bảo hành", "Ghi nhận thiết bị khách hàng mang tới cửa hàng bảo hành.");
    gtk_box_append(GTK_BOX(content), header);

    std::vector<UIComponents::TableColumnSpec> warCols = {
        {"Mã phiếu", 120, 0.0f, false},
        {"Thiết bị", 180, 0.0f, false},
        {"Khách hàng", 170, 0.0f, false},
        {"Ngày nhận", 130, 0.0f, false},
        {"Trạng thái", 140, 0.5f, false},
        {"Mô tả sự cố", 0, 0.0f, true}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(warCols));

    warrantyListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), warrantyListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(warrantyIntakeView), content);
    refreshWarrantyIntake();
}

void SalesScreens::refreshWarrantyIntake() {
    if (!warrantyListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(warrantyListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(warrantyListBox), child);
        child = next;
    }

    std::vector<WarrantyItemViewModel> list = controller->getWarranties();
    if (list.empty()) {
        gtk_box_append(GTK_BOX(warrantyListBox), UIComponents::createEmptyState("🛡️", "Chưa có dữ liệu", "Chưa có yêu cầu hoặc biên nhận bảo hành nào."));
        return;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& w = list[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Mã phiếu
        GtkWidget* lblId = gtk_label_new(w.id.c_str());
        gtk_widget_add_css_class(lblId, "product-name");
        gtk_widget_set_size_request(lblId, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(lblId), 0.0f);
        gtk_widget_set_valign(lblId, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), lblId);

        // Thiết bị
        GtkWidget* phoneLbl = gtk_label_new(w.phoneName.c_str());
        gtk_widget_set_size_request(phoneLbl, 180, -1);
        gtk_label_set_xalign(GTK_LABEL(phoneLbl), 0.0f);
        gtk_widget_set_valign(phoneLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), phoneLbl);

        // Khách hàng
        GtkWidget* custBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_size_request(custBox, 170, -1);
        gtk_widget_set_valign(custBox, GTK_ALIGN_CENTER);
        GtkWidget* cName = gtk_label_new(w.customerName.c_str());
        gtk_label_set_xalign(GTK_LABEL(cName), 0.0f);
        GtkWidget* cPhone = gtk_label_new(w.customerPhone.c_str());
        gtk_widget_add_css_class(cPhone, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(cPhone), 0.0f);
        gtk_box_append(GTK_BOX(custBox), cName);
        gtk_box_append(GTK_BOX(custBox), cPhone);
        gtk_box_append(GTK_BOX(row), custBox);

        // Ngày nhận
        GtkWidget* timeLbl = gtk_label_new(w.requestDate.c_str());
        gtk_widget_set_size_request(timeLbl, 130, -1);
        gtk_label_set_xalign(GTK_LABEL(timeLbl), 0.0f);
        gtk_widget_set_valign(timeLbl, GTK_ALIGN_CENTER);
        gtk_widget_add_css_class(timeLbl, "page-subtitle");
        gtk_box_append(GTK_BOX(row), timeLbl);

        // Trạng thái badge
        GtkWidget* badge = UIComponents::createWarrantyStatusBadge(w.status);
        GtkWidget* badgeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(badgeBox, 140, -1);
        gtk_widget_set_valign(badgeBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(badgeBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(badgeBox), badge);
        gtk_box_append(GTK_BOX(row), badgeBox);

        // Mô tả sự cố
        GtkWidget* descLbl = gtk_label_new(w.issueDescription.c_str());
        gtk_widget_set_hexpand(descLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(descLbl), 0.0f);
        gtk_widget_set_valign(descLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), descLbl);

        gtk_box_append(GTK_BOX(warrantyListBox), row);
    }
}

void SalesScreens::focusPosSearch() {
    if (posEntrySearch && GTK_IS_WIDGET(posEntrySearch)) {
        gtk_widget_grab_focus(posEntrySearch);
    }
}
