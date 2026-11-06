/**
 * @file MainShellWindow.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI CỬA SỔ CHÍNH VÀ ĐIỀU HƯỚNG PHÂN QUYỀN
 * 
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildHeader: Khởi tạo thanh tiêu đề, hiển thị thương hiệu, đồng hồ thời gian thực và badge người dùng.
 * - buildSidebar: Dựng menu điều hướng động theo vai trò (Customer / Sales / Warehouse / Admin).
 * - buildStack: Tích hợp các ScrolledWindow của từng phân hệ vào GtkStack.
 * - navigateTo: Điều hướng mượt mà giữa các tab và cập nhật kiểu dáng sidebar-btn-active.
 * - on_confirm_logout: Xác nhận đăng xuất có hộp thoại hỏi lại, kích hoạt callback chuyển về login.
 * =========================================================================================
 */

#include "MainShellWindow.h"
#include "CustomerScreens.h"
#include "SalesScreens.h"
#include "WarehouseScreens.h"
#include "AdminScreens.h"
#include "../components/UIComponents.h"
#include "../styles/AppCss.h"

struct NavButtonData {
    MainShellWindow* shell;
    std::string pageName;
};

MainShellWindow::MainShellWindow(GtkApplication* gapp, AppController* ctrl, LogoutCallback onLogout, gpointer userData)
    : controller(ctrl), app(gapp), logoutCallback(onLogout), callbackData(userData),
      customerScreens(nullptr), salesScreens(nullptr), warehouseScreens(nullptr), adminScreens(nullptr) {

    window = gtk_application_window_new(app);
    g_signal_connect(window, "close-request", G_CALLBACK(on_window_close_request), this);
    gtk_window_set_title(GTK_WINDOW(window), "PBL2 — Phone Store Management System");
    gtk_window_set_default_size(GTK_WINDOW(window), 1240, 680);
    gtk_window_maximize(GTK_WINDOW(window));

    // Root layout: Vertical Box (Header on top, HBox for Sidebar + Stack below)
    GtkWidget* rootBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    buildHeader();
    gtk_box_append(GTK_BOX(rootBox), headerBar);

    GtkWidget* bodyBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_vexpand(bodyBox, TRUE);

    buildSidebar();
    gtk_box_append(GTK_BOX(bodyBox), sidebarBox);

    buildStack();
    gtk_box_append(GTK_BOX(bodyBox), stack);

    gtk_box_append(GTK_BOX(rootBox), bodyBox);

    toastOverlay = gtk_overlay_new();
    gtk_overlay_set_child(GTK_OVERLAY(toastOverlay), rootBox);
    gtk_window_set_child(GTK_WINDOW(window), toastOverlay);

    UIComponents::registerShellWindow(this);
    refreshCurrentRoleUI();
}

MainShellWindow::~MainShellWindow() {
    UIComponents::registerShellWindow(nullptr);
    delete customerScreens;
    delete salesScreens;
    delete warehouseScreens;
    delete adminScreens;
    if (window) {
        GtkWidget* win = window;
        window = nullptr;
        if (GTK_IS_WINDOW(win)) {
            gtk_window_destroy(GTK_WINDOW(win));
        }
    }
}

void MainShellWindow::show() {
    gtk_window_present(GTK_WINDOW(window));
}

void MainShellWindow::showToast(const std::string& message, const std::string& type) {
    if (!toastOverlay) return;

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(box, "toast-box");
    if (type == "success") {
        gtk_widget_add_css_class(box, "toast-success");
    } else if (type == "error") {
        gtk_widget_add_css_class(box, "toast-error");
    } else {
        gtk_widget_add_css_class(box, "toast-info");
    }

    gtk_widget_set_halign(box, GTK_ALIGN_END);
    gtk_widget_set_valign(box, GTK_ALIGN_START);
    gtk_widget_set_margin_top(box, 75);
    gtk_widget_set_margin_end(box, 24);

    GtkWidget* iconLbl = gtk_label_new(type == "success" ? "✅" : (type == "error" ? "❌" : "ℹ️"));
    gtk_box_append(GTK_BOX(box), iconLbl);

    GtkWidget* msgLbl = gtk_label_new(message.c_str());
    gtk_widget_add_css_class(msgLbl, "toast-text");
    gtk_label_set_wrap(GTK_LABEL(msgLbl), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(msgLbl), 40);
    gtk_box_append(GTK_BOX(box), msgLbl);

    GtkWidget* btnClose = gtk_button_new_with_label("✕");
    gtk_widget_add_css_class(btnClose, "toast-close-btn");
    UIComponents::setButtonCursor(btnClose);
    gtk_box_append(GTK_BOX(box), btnClose);

    struct ToastData {
        GtkWidget* overlay;
        GtkWidget* toast;
    };
    ToastData* td = new ToastData{toastOverlay, box};

    g_signal_connect_data(btnClose, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
        ToastData* data = static_cast<ToastData*>(d);
        if (data->overlay && data->toast && gtk_widget_get_parent(data->toast) == data->overlay) {
            gtk_overlay_remove_overlay(GTK_OVERLAY(data->overlay), data->toast);
        }
    }), td, [](gpointer d, GClosure*) { delete static_cast<ToastData*>(d); }, (GConnectFlags)0);

    gtk_overlay_add_overlay(GTK_OVERLAY(toastOverlay), box);

    // Auto dismiss after 2500ms
    g_timeout_add(2500, +[](gpointer d) -> gboolean {
        GtkWidget* toast = GTK_WIDGET(d);
        GtkWidget* parent = gtk_widget_get_parent(toast);
        if (parent && GTK_IS_OVERLAY(parent)) {
            gtk_overlay_remove_overlay(GTK_OVERLAY(parent), toast);
        }
        return G_SOURCE_REMOVE;
    }, box);
}

void MainShellWindow::buildHeader() {
    headerBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(headerBar, "main-header");

    // Brand Logo
    GtkWidget* lblBrand = gtk_label_new("📱 PHONE STORE MANAGER");
    gtk_widget_add_css_class(lblBrand, "app-brand-title");
    gtk_box_append(GTK_BOX(headerBar), lblBrand);

    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start(sep, 8);
    gtk_widget_set_margin_end(sep, 8);
    gtk_box_append(GTK_BOX(headerBar), sep);

    // Current page breadcrumb
    lblHeaderTitle = gtk_label_new("Tổng quan");
    gtk_widget_add_css_class(lblHeaderTitle, "header-screen-title");
    gtk_widget_set_hexpand(lblHeaderTitle, TRUE);
    gtk_label_set_xalign(GTK_LABEL(lblHeaderTitle), 0.0f);
    gtk_box_append(GTK_BOX(headerBar), lblHeaderTitle);

    // Simulated Time Badge
    lblSimulatedTimeBadge = gtk_label_new(("⏱️ " + controller->getSimulatedDate()).c_str());
    gtk_widget_add_css_class(lblSimulatedTimeBadge, "realtime-header-badge");
    gtk_box_append(GTK_BOX(headerBar), lblSimulatedTimeBadge);

    // User profile pill
    lblUserBadge = gtk_label_new("");
    gtk_widget_add_css_class(lblUserBadge, "user-profile-badge");
    gtk_box_append(GTK_BOX(headerBar), lblUserBadge);

    // Change Password Button
    GtkWidget* btnChangePw = gtk_button_new_with_label("🔑 Đổi mật khẩu");
    gtk_widget_add_css_class(btnChangePw, "btn-secondary");
    gtk_widget_set_name(btnChangePw, "change_password_button");
    UIComponents::setButtonCursor(btnChangePw);
    g_signal_connect(btnChangePw, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
        MainShellWindow* self = static_cast<MainShellWindow*>(d);
        UIComponents::showChangePasswordDialog(GTK_WINDOW(self->getWindow()), self->controller);
    }), this);
    gtk_box_append(GTK_BOX(headerBar), btnChangePw);

    // Logout Button
    GtkWidget* btnLogout = gtk_button_new_with_label("🚪 Đăng xuất");
    gtk_widget_add_css_class(btnLogout, "btn-danger");
    gtk_widget_set_name(btnLogout, "logout_button");
    UIComponents::setButtonCursor(btnLogout);
    g_signal_connect(btnLogout, "clicked", G_CALLBACK(on_logout_btn_clicked), this);
    gtk_box_append(GTK_BOX(headerBar), btnLogout);
}

void MainShellWindow::buildSidebar() {
    sidebarBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
    gtk_widget_add_css_class(sidebarBox, "sidebar-panel");
    gtk_widget_set_size_request(sidebarBox, DesignTokens::SIDEBAR_WIDTH, -1);
}

void MainShellWindow::buildStack() {
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 150);
    gtk_widget_set_hexpand(stack, TRUE);
    gtk_widget_set_vexpand(stack, TRUE);

    // Instantiate all role screen managers
    customerScreens = new CustomerScreens(controller, this);
    salesScreens = new SalesScreens(controller, this);
    warehouseScreens = new WarehouseScreens(controller, this);
    adminScreens = new AdminScreens(controller, this);

    // Add Customer views to Stack
    gtk_stack_add_named(GTK_STACK(stack), customerScreens->getDashboardView(), "customer_dashboard");
    gtk_stack_add_named(GTK_STACK(stack), customerScreens->getProductsView(), "customer_products");
    gtk_stack_add_named(GTK_STACK(stack), customerScreens->getCartView(), "customer_cart");
    gtk_stack_add_named(GTK_STACK(stack), customerScreens->getOrdersView(), "customer_orders");
    gtk_stack_add_named(GTK_STACK(stack), customerScreens->getProfileView(), "customer_profile");
    gtk_stack_add_named(GTK_STACK(stack), customerScreens->getWarrantyView(), "customer_warranty");

    // Add Sales views to Stack
    gtk_stack_add_named(GTK_STACK(stack), salesScreens->getDashboardView(), "sales_dashboard");
    gtk_stack_add_named(GTK_STACK(stack), salesScreens->getPosView(), "sales_pos");
    gtk_stack_add_named(GTK_STACK(stack), salesScreens->getProductLookupView(), "sales_lookup");
    gtk_stack_add_named(GTK_STACK(stack), salesScreens->getCustomerMgmtView(), "sales_customers");
    gtk_stack_add_named(GTK_STACK(stack), salesScreens->getInvoicesView(), "sales_invoices");
    gtk_stack_add_named(GTK_STACK(stack), salesScreens->getOnlineOrdersView(), "sales_orders");
    gtk_stack_add_named(GTK_STACK(stack), salesScreens->getWarrantyIntakeView(), "sales_warranty");

    // Add Warehouse views to Stack
    gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getDashboardView(), "warehouse_dashboard");
    gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getInventoryView(), "warehouse_inventory");
    gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getLowStockView(), "warehouse_low_stock");
    gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getReceiptsView(), "warehouse_receipts");
    gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getCreateReceiptView(), "warehouse_create_receipt");
    gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getFulfillmentView(), "warehouse_fulfillment");

    // Add Admin views to Stack
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getDashboardView(), "admin_dashboard");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getProductMgmtView(), "admin_products");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getEmployeeMgmtView(), "admin_employees");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getCustomerMgmtView(), "admin_customers");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getSupplierMgmtView(), "admin_suppliers");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getDiscountMgmtView(), "admin_discounts");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getAccountMgmtView(), "admin_accounts");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getAllOrdersView(), "admin_orders");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getAllInvoicesView(), "admin_invoices");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getWarrantyMonitorView(), "admin_warranty");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getReportsView(), "admin_reports");
    gtk_stack_add_named(GTK_STACK(stack), adminScreens->getRealTimeView(), "admin_realtime");
}

void MainShellWindow::addSidebarButton(const std::string& pageName, const std::string& label, const std::string& icon) {
    GtkWidget* btn = gtk_button_new();
    gtk_widget_add_css_class(btn, "sidebar-btn");
    gtk_widget_set_name(btn, ("nav_" + pageName).c_str());
    UIComponents::setButtonCursor(btn);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* iconLbl = gtk_label_new(icon.c_str());
    gtk_widget_add_css_class(iconLbl, "sidebar-icon-slot");
    GtkWidget* textLbl = gtk_label_new(label.c_str());
    gtk_widget_set_hexpand(textLbl, TRUE);
    gtk_label_set_xalign(GTK_LABEL(textLbl), 0.0f);

    gtk_box_append(GTK_BOX(box), iconLbl);
    gtk_box_append(GTK_BOX(box), textLbl);

    if (pageName == "customer_cart") {
        lblCartCountBadge = gtk_label_new("0");
        gtk_widget_add_css_class(lblCartCountBadge, "cart-count-badge");
        gtk_box_append(GTK_BOX(box), lblCartCountBadge);
    }

    gtk_button_set_child(GTK_BUTTON(btn), box);

    NavButtonData* data = new NavButtonData{this, pageName};
    g_signal_connect_data(btn, "clicked", G_CALLBACK(on_nav_btn_clicked),
                          data, [](gpointer d, GClosure*) { delete static_cast<NavButtonData*>(d); },
                          (GConnectFlags)0);

    gtk_box_append(GTK_BOX(sidebarBox), btn);
    sidebarButtons[pageName] = btn;
}

void MainShellWindow::refreshCurrentRoleUI() {
    const UserSessionViewModel& user = controller->getCurrentUser();

    // Update Header Pill
    std::string badgeText = "👤 " + user.fullName + "  •  " + user.roleName;
    gtk_label_set_text(GTK_LABEL(lblUserBadge), badgeText.c_str());

    // Clear current sidebar items
    GtkWidget* child = gtk_widget_get_first_child(sidebarBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(sidebarBox), child);
        child = next;
    }
    sidebarButtons.clear();

    auto addCategory = [this](const char* cat) {
        GtkWidget* l = gtk_label_new(cat);
        gtk_widget_add_css_class(l, "sidebar-category-label");
        gtk_label_set_xalign(GTK_LABEL(l), 0.0f);
        gtk_box_append(GTK_BOX(this->sidebarBox), l);
    };

    std::string initialPage = "";

    if (user.isCustomer()) {
        addCategory("Khách hàng");
        addSidebarButton("customer_dashboard", "Tổng quan", "📊");
        addSidebarButton("customer_products", "Mua sắm điện thoại", "🛍️");
        addSidebarButton("customer_cart", "Giỏ hàng", "🛒");
        addSidebarButton("customer_orders", "Đơn hàng của tôi", "📦");
        addSidebarButton("customer_warranty", "Trung tâm bảo hành", "🛡️");
        addSidebarButton("customer_profile", "Hồ sơ cá nhân", "👤");

        initialPage = "customer_dashboard";
        CartSummaryViewModel cart = controller->getCart(user.userId);
        updateCartBadge(cart.totalQuantity);
        customerScreens->refreshAll();

    } else if (user.isSales()) {
        addCategory("Nghiệp vụ Bán hàng");
        addSidebarButton("sales_dashboard", "Bàn làm việc", "📊");
        addSidebarButton("sales_pos", "Bán hàng tại quầy (POS)", "🖥️");
        addSidebarButton("sales_lookup", "Tra cứu sản phẩm", "🔍");
        addSidebarButton("sales_customers", "Quản lý khách hàng", "👥");
        addSidebarButton("sales_invoices", "Hóa đơn bán hàng", "📑");
        addSidebarButton("sales_orders", "Đơn hàng online", "📦");
        addSidebarButton("sales_warranty", "Tiếp nhận bảo hành", "🛡️");

        initialPage = "sales_dashboard";
        salesScreens->refreshAll();

    } else if (user.isWarehouse()) {
        addCategory("Nghiệp vụ Kho bãi");
        addSidebarButton("warehouse_dashboard", "Tổng quan kho", "📊");
        addSidebarButton("warehouse_inventory", "Quản lý tồn kho", "📦");
        addSidebarButton("warehouse_low_stock", "Cảnh báo sắp hết", "⚠️");
        addSidebarButton("warehouse_receipts", "Phiếu nhập kho", "📥");
        addSidebarButton("warehouse_create_receipt", "Lập phiếu nhập", "➕");
        addSidebarButton("warehouse_fulfillment", "Chuẩn bị đơn hàng", "🚚");

        initialPage = "warehouse_dashboard";
        warehouseScreens->refreshAll();

    } else if (user.isAdmin()) {
        auto addDivider = [this]() {
            GtkWidget* div = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_add_css_class(div, "sidebar-divider");
            gtk_box_append(GTK_BOX(this->sidebarBox), div);
        };

        addCategory("Tổng quan & Báo cáo");
        addSidebarButton("admin_dashboard", "Báo cáo quản trị", "📊");
        addSidebarButton("admin_reports", "Thống kê & Lợi nhuận", "📈");

        addDivider();
        addCategory("Danh mục & Nguồn hàng");
        addSidebarButton("admin_products", "Quản lý sản phẩm", "📱");
        addSidebarButton("admin_suppliers", "Nhà cung cấp", "🏢");
        addSidebarButton("admin_discounts", "Mã giảm giá", "🎟️");

        addDivider();
        addCategory("Giao dịch & Bảo hành");
        addSidebarButton("admin_orders", "Tất cả đơn hàng", "📦");
        addSidebarButton("admin_invoices", "Tất cả hóa đơn", "📑");
        addSidebarButton("admin_warranty", "Giám sát bảo hành", "🛡️");

        addDivider();
        addCategory("Nhân sự & Hệ thống");
        addSidebarButton("admin_employees", "Quản lý nhân sự", "👥");
        addSidebarButton("admin_customers", "Quản lý khách hàng", "👤");
        addSidebarButton("admin_accounts", "Quản trị tài khoản", "🔐");
        addSidebarButton("admin_realtime", "Kiểm soát thời gian thực", "⏱️");

        initialPage = "admin_dashboard";
        adminScreens->refreshAll();
    }

    updateSimulatedTimeBadge();
    navigateTo(initialPage);
}

void MainShellWindow::navigateTo(const std::string& pageName) {
    if (pageName.empty()) return;
    currentPage = pageName;

    // Switch stack view
    gtk_stack_set_visible_child_name(GTK_STACK(stack), pageName.c_str());

    // Update buttons active class
    for (auto& pair : sidebarButtons) {
        if (pair.first == pageName) {
            gtk_widget_add_css_class(pair.second, "sidebar-btn-active");
        } else {
            gtk_widget_remove_css_class(pair.second, "sidebar-btn-active");
        }
    }

    // Update Header Breadcrumb Title
    std::string pageTitle = "Tổng quan";
    if (pageName == "customer_products" || pageName == "sales_lookup" || pageName == "admin_products") pageTitle = "Sản phẩm điện thoại";
    else if (pageName == "customer_cart") pageTitle = "Giỏ hàng của bạn";
    else if (pageName == "customer_orders" || pageName == "sales_orders" || pageName == "admin_orders") pageTitle = "Quản lý Đơn hàng";
    else if (pageName == "customer_profile") pageTitle = "Hồ sơ cá nhân & Điểm thưởng";
    else if (pageName == "customer_warranty" || pageName == "sales_warranty" || pageName == "admin_warranty") pageTitle = "Bảo hành thiết bị";
    else if (pageName == "sales_pos") pageTitle = "Bán hàng tại quầy (POS)";
    else if (pageName == "sales_customers" || pageName == "admin_customers") pageTitle = "Quản lý Khách hàng";
    else if (pageName == "sales_invoices" || pageName == "admin_invoices") pageTitle = "Quản lý Hóa đơn";
    else if (pageName == "warehouse_inventory") pageTitle = "Tồn kho thiết bị";
    else if (pageName == "warehouse_low_stock") pageTitle = "Cảnh báo sắp hết hàng";
    else if (pageName == "warehouse_receipts") pageTitle = "Phiếu nhập kho";
    else if (pageName == "warehouse_create_receipt") pageTitle = "Lập phiếu nhập kho";
    else if (pageName == "warehouse_fulfillment") pageTitle = "Chuẩn bị đơn hàng";
    else if (pageName == "admin_employees") pageTitle = "Quản lý Nhân sự";
    else if (pageName == "admin_suppliers") pageTitle = "Quản lý Nhà cung cấp";
    else if (pageName == "admin_discounts") pageTitle = "Quản lý Mã giảm giá";
    else if (pageName == "admin_accounts") pageTitle = "Tài khoản & Phân quyền";
    else if (pageName == "admin_reports") pageTitle = "Báo cáo Doanh thu & Lợi nhuận";
    else if (pageName == "admin_realtime") pageTitle = "Kiểm soát Thời gian thực Hệ thống";

    gtk_label_set_text(GTK_LABEL(lblHeaderTitle), pageTitle.c_str());

    if (pageName == "sales_pos" && salesScreens) {
        salesScreens->focusPosSearch();
    }
}

void MainShellWindow::updateCartBadge(int count) {
    if (lblCartCountBadge) {
        gtk_label_set_text(GTK_LABEL(lblCartCountBadge), std::to_string(count).c_str());
        gtk_widget_set_visible(lblCartCountBadge, count > 0);
    }
}

void MainShellWindow::updateSimulatedTimeBadge() {
    if (lblSimulatedTimeBadge) {
        std::string badge = "⏱️ " + controller->getSimulatedDate();
        gtk_label_set_text(GTK_LABEL(lblSimulatedTimeBadge), badge.c_str());
    }
}

void MainShellWindow::on_nav_btn_clicked(GtkButton* btn, gpointer data) {
    (void)btn;
    NavButtonData* nd = static_cast<NavButtonData*>(data);
    nd->shell->navigateTo(nd->pageName);
}

void MainShellWindow::on_logout_btn_clicked(GtkButton* btn, gpointer data) {
    (void)btn;
    MainShellWindow* self = static_cast<MainShellWindow*>(data);
    UIComponents::showConfirmDialog(
        self->getWindow(),
        "Xác nhận đăng xuất",
        "Bạn có chắc chắn muốn đăng xuất khỏi phiên làm việc hiện tại không?",
        on_confirm_logout,
        self
    );
}

void MainShellWindow::on_confirm_logout(gpointer data) {
    MainShellWindow* self = static_cast<MainShellWindow*>(data);
    self->controller->logout();
    if (self->logoutCallback) {
        self->logoutCallback(self->controller, self->callbackData);
    }
}

gboolean MainShellWindow::on_window_close_request(GtkWindow* closedWindow, gpointer data) {
    MainShellWindow* self = static_cast<MainShellWindow*>(data);
    if (self->window == GTK_WIDGET(closedWindow)) {
        self->window = nullptr;
    }
    return FALSE;
}
