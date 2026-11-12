/**
 * @file MainShellWindow.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI CỬA SỔ CHÍNH VÀ ĐIỀU HƯỚNG PHÂN QUYỀN
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildHeader: Khởi tạo thanh tiêu đề, hiển thị thương hiệu, đồng hồ thời
 * gian thực và badge người dùng.
 * - buildSidebar: Dựng menu điều hướng động theo vai trò (Customer / Sales /
 * Warehouse / Admin).
 * - buildStack: Tích hợp các ScrolledWindow của từng phân hệ vào GtkStack.
 * - navigateTo: Điều hướng mượt mà giữa các tab và cập nhật kiểu dáng
 * sidebar-btn-active.
 * - on_confirm_logout: Xác nhận đăng xuất có hộp thoại hỏi lại, kích hoạt
 * callback chuyển về login.
 * =========================================================================================
 */

#include "MainShellWindow.h"
#include "../components/UIComponents.h"
#include "../styles/AppCss.h"
#include "AdminScreens.h"
#include "CustomerScreens.h"
#include "SalesScreens.h"
#include "WarehouseScreens.h"
#include <algorithm>
#include <gdk/gdkkeysyms.h>

static MainShellWindow *s_activeShellInstance = nullptr;

MainShellWindow *MainShellWindow::getActiveInstance() {
  return s_activeShellInstance;
}

struct NavButtonData {
  MainShellWindow *shell;
  std::string pageName;
};

MainShellWindow::MainShellWindow(GtkWindow *parentWindow, AppController *ctrl,
                                 LogoutCallback onLogout, gpointer userData)
    : controller(ctrl), app(nullptr), window(GTK_WIDGET(parentWindow)),
      logoutCallback(onLogout), callbackData(userData),
      customerScreens(nullptr), salesScreens(nullptr),
      warehouseScreens(nullptr), adminScreens(nullptr), ownsWindow(false) {
  initShellContent();
}

MainShellWindow::MainShellWindow(GtkApplication *gapp, AppController *ctrl,
                                 LogoutCallback onLogout, gpointer userData)
    : controller(ctrl), app(gapp), logoutCallback(onLogout),
      callbackData(userData), customerScreens(nullptr), salesScreens(nullptr),
      warehouseScreens(nullptr), adminScreens(nullptr), ownsWindow(true) {

  window = gtk_application_window_new(app);
  g_signal_connect(window, "close-request", G_CALLBACK(on_window_close_request),
                   this);
  gtk_window_set_title(GTK_WINDOW(window),
                       "PBL2 — Phone Store Management System");
  gtk_widget_set_size_request(window, 1024, 600);
  gtk_window_set_default_size(GTK_WINDOW(window), 1240, 680);
  gtk_window_maximize(GTK_WINDOW(window));

  initShellContent();
  gtk_window_set_child(GTK_WINDOW(window), toastOverlay);
  activateSession();
}

void MainShellWindow::initShellContent() {
  // Phím Esc đóng Overlay Dialog hoặc Pop Page Stack
  if (window && GTK_IS_WIDGET(window)) {
    keyController = gtk_event_controller_key_new();
    g_signal_connect(keyController, "key-pressed", G_CALLBACK(on_key_pressed),
                     this);
    gtk_widget_add_controller(window, keyController);
  }

  // Root layout: Vertical Box (Header on top, HBox for Sidebar + Stack below)
  GtkWidget *rootBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(rootBox, TRUE);
  gtk_widget_set_vexpand(rootBox, TRUE);

  buildHeader();
  gtk_box_append(GTK_BOX(rootBox), headerBar);

  GtkWidget *bodyBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_vexpand(bodyBox, TRUE);

  buildSidebar();
  GtkWidget *sidebarScroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sidebarScroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_add_css_class(sidebarScroll, "sidebar-panel");
  gtk_widget_set_size_request(sidebarScroll, DesignTokens::SIDEBAR_WIDTH, -1);
  gtk_widget_set_hexpand(sidebarScroll, FALSE);
  gtk_widget_set_vexpand(sidebarScroll, TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sidebarScroll), sidebarBox);
  gtk_box_append(GTK_BOX(bodyBox), sidebarScroll);

  buildStack();

  // Container Page Stack thứ hai bọc ngoài stack các màn hình vai trò chính
  pageNavStack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(pageNavStack),
                                GTK_STACK_TRANSITION_TYPE_OVER_LEFT);
  gtk_stack_set_transition_duration(GTK_STACK(pageNavStack), 260);
  gtk_widget_set_hexpand(pageNavStack, TRUE);
  gtk_widget_set_vexpand(pageNavStack, TRUE);

  gtk_stack_add_named(GTK_STACK(pageNavStack), stack, "__main_views__");
  gtk_stack_set_visible_child_name(GTK_STACK(pageNavStack), "__main_views__");

  gtk_box_append(GTK_BOX(bodyBox), pageNavStack);

  gtk_box_append(GTK_BOX(rootBox), bodyBox);

  toastOverlay = gtk_overlay_new();
  gtk_widget_set_hexpand(toastOverlay, TRUE);
  gtk_widget_set_vexpand(toastOverlay, TRUE);
  gtk_overlay_set_child(GTK_OVERLAY(toastOverlay), rootBox);
}

void MainShellWindow::flushPendingCleanups() {
  for (auto &item : pendingCleanups) {
    if (item.timerId > 0) {
      g_source_remove(item.timerId);
    }
    if (item.page && GTK_IS_WIDGET(item.page)) {
      if (item.stack && GTK_IS_STACK(item.stack) &&
          gtk_widget_get_parent(item.page) == item.stack) {
        gtk_stack_remove(GTK_STACK(item.stack), item.page);
      }
      g_object_unref(item.page);
    }
  }
  pendingCleanups.clear();
}

void MainShellWindow::activateSession() {
  s_activeShellInstance = this;
  UIComponents::registerShellWindow(this);
  flushPendingCleanups();
  closeOverlayDialog();
  clearPageStack();
  refreshCurrentRoleUI();
  startClockTimer();
}

void MainShellWindow::deactivateSession() {
  stopClockTimer();
  flushPendingCleanups();
  closeOverlayDialog();
  clearPageStack();
  if (s_activeShellInstance == this) {
    s_activeShellInstance = nullptr;
  }
  UIComponents::registerShellWindow(nullptr);
}

MainShellWindow::~MainShellWindow() {
  deactivateSession();

  if (window && keyController && GTK_IS_WIDGET(window)) {
    gtk_widget_remove_controller(window, keyController);
    keyController = nullptr;
  }

  delete customerScreens;
  delete salesScreens;
  delete warehouseScreens;
  delete adminScreens;

  if (ownsWindow && window) {
    GtkWidget *win = window;
    window = nullptr;
    if (GTK_IS_WINDOW(win)) {
      gtk_window_destroy(GTK_WINDOW(win));
    }
  } else {
    window = nullptr;
  }
}

void MainShellWindow::show() {
  if (window && GTK_IS_WINDOW(window)) {
    gtk_window_present(GTK_WINDOW(window));
  }
}

void MainShellWindow::showToast(const std::string &message,
                                const std::string &type) {
  if (!toastOverlay)
    return;

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
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

  GtkWidget *iconLbl =
      gtk_label_new(type == "success" ? "✅" : (type == "error" ? "❌" : "ℹ️"));
  gtk_box_append(GTK_BOX(box), iconLbl);

  GtkWidget *msgLbl = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(msgLbl, "toast-text");
  gtk_label_set_wrap(GTK_LABEL(msgLbl), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(msgLbl), 40);
  gtk_box_append(GTK_BOX(box), msgLbl);

  GtkWidget *btnClose = gtk_button_new_with_label("✕");
  gtk_widget_add_css_class(btnClose, "toast-close-btn");
  UIComponents::setButtonCursor(btnClose);
  gtk_box_append(GTK_BOX(box), btnClose);

  struct ToastData {
    GtkWidget *overlay;
    GtkWidget *toast;
  };
  ToastData *td = new ToastData{toastOverlay, box};

  g_signal_connect_data(
      btnClose, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
        ToastData *data = static_cast<ToastData *>(d);
        if (data->overlay && data->toast &&
            gtk_widget_get_parent(data->toast) == data->overlay) {
          gtk_overlay_remove_overlay(GTK_OVERLAY(data->overlay), data->toast);
        }
      }),
      td, [](gpointer d, GClosure *) { delete static_cast<ToastData *>(d); },
      (GConnectFlags)0);

  gtk_overlay_add_overlay(GTK_OVERLAY(toastOverlay), box);

  // Auto dismiss after 2500ms
  g_timeout_add(
      2500,
      +[](gpointer d) -> gboolean {
        GtkWidget *toast = GTK_WIDGET(d);
        GtkWidget *parent = gtk_widget_get_parent(toast);
        if (parent && GTK_IS_OVERLAY(parent)) {
          gtk_overlay_remove_overlay(GTK_OVERLAY(parent), toast);
        }
        return G_SOURCE_REMOVE;
      },
      box);
}

void MainShellWindow::buildHeader() {
  headerBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
  gtk_widget_add_css_class(headerBar, "main-header");

  // Brand Logo
  GtkWidget *lblBrand = gtk_label_new("📱 PHONE STORE MANAGER");
  gtk_widget_add_css_class(lblBrand, "app-brand-title");
  gtk_box_append(GTK_BOX(headerBar), lblBrand);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_margin_start(sep, 8);
  gtk_widget_set_margin_end(sep, 8);
  gtk_box_append(GTK_BOX(headerBar), sep);

  // Current page breadcrumb
  lblHeaderTitle = gtk_label_new("Khách hàng / Tổng quan");
  gtk_widget_add_css_class(lblHeaderTitle, "header-screen-title");
  gtk_widget_set_hexpand(lblHeaderTitle, TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblHeaderTitle), 0.0f);
  gtk_box_append(GTK_BOX(headerBar), lblHeaderTitle);

  // Simulated Time Badge
  lblSimulatedTimeBadge =
      gtk_label_new(("⏱️ " + controller->getSimulatedTime()).c_str());
  gtk_widget_add_css_class(lblSimulatedTimeBadge, "realtime-header-badge");
  gtk_box_append(GTK_BOX(headerBar), lblSimulatedTimeBadge);

  // User profile pill
  lblUserBadge = gtk_label_new("");
  gtk_widget_add_css_class(lblUserBadge, "user-profile-badge");
  gtk_box_append(GTK_BOX(headerBar), lblUserBadge);

  // Change Password Button
  GtkWidget *btnChangePw = gtk_button_new_with_label("🔑 Đổi mật khẩu");
  gtk_widget_add_css_class(btnChangePw, "btn-secondary");
  gtk_widget_set_name(btnChangePw, "change_password_button");
  UIComponents::setButtonCursor(btnChangePw);
  g_signal_connect(btnChangePw, "clicked",
                   G_CALLBACK(+[](GtkButton *, gpointer d) {
                     MainShellWindow *self = static_cast<MainShellWindow *>(d);
                     UIComponents::showChangePasswordDialog(
                         GTK_WINDOW(self->getWindow()), self->controller);
                   }),
                   this);
  gtk_box_append(GTK_BOX(headerBar), btnChangePw);

  // Logout Button
  GtkWidget *btnLogout = gtk_button_new_with_label("🚪 Đăng xuất");
  gtk_widget_add_css_class(btnLogout, "btn-danger");
  gtk_widget_set_name(btnLogout, "logout_button");
  UIComponents::setButtonCursor(btnLogout);
  g_signal_connect(btnLogout, "clicked", G_CALLBACK(on_logout_btn_clicked),
                   this);
  gtk_box_append(GTK_BOX(headerBar), btnLogout);
}

void MainShellWindow::buildSidebar() {
  sidebarBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  gtk_widget_add_css_class(sidebarBox, "sidebar-panel");
  gtk_widget_set_size_request(sidebarBox, DesignTokens::SIDEBAR_WIDTH, -1);
  gtk_widget_set_hexpand(sidebarBox, FALSE);
  gtk_widget_set_vexpand(sidebarBox, TRUE);
}

void MainShellWindow::buildStack() {
  stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(stack),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  gtk_stack_set_transition_duration(GTK_STACK(stack), 200);
  gtk_widget_set_hexpand(stack, TRUE);
  gtk_widget_set_vexpand(stack, TRUE);

  // Instantiate all role screen managers
  customerScreens = new CustomerScreens(controller, this);
  salesScreens = new SalesScreens(controller, this);
  warehouseScreens = new WarehouseScreens(controller, this);
  adminScreens = new AdminScreens(controller, this);

  // Add Customer views to Stack
  gtk_stack_add_named(GTK_STACK(stack), customerScreens->getDashboardView(),
                      "customer_dashboard");
  gtk_stack_add_named(GTK_STACK(stack), customerScreens->getProductsView(),
                      "customer_products");
  gtk_stack_add_named(GTK_STACK(stack), customerScreens->getCartView(),
                      "customer_cart");
  gtk_stack_add_named(GTK_STACK(stack), customerScreens->getOrdersView(),
                      "customer_orders");
  gtk_stack_add_named(GTK_STACK(stack), customerScreens->getDiscountsView(),
                      "customer_discounts");
  gtk_stack_add_named(GTK_STACK(stack), customerScreens->getProfileView(),
                      "customer_profile");
  gtk_stack_add_named(GTK_STACK(stack), customerScreens->getWarrantyView(),
                      "customer_warranty");

  // Add Sales views to Stack
  gtk_stack_add_named(GTK_STACK(stack), salesScreens->getDashboardView(),
                      "sales_dashboard");
  gtk_stack_add_named(GTK_STACK(stack), salesScreens->getPosView(),
                      "sales_pos");
  gtk_stack_add_named(GTK_STACK(stack), salesScreens->getProductLookupView(),
                      "sales_lookup");
  gtk_stack_add_named(GTK_STACK(stack), salesScreens->getCustomerMgmtView(),
                      "sales_customers");
  gtk_stack_add_named(GTK_STACK(stack), salesScreens->getInvoicesView(),
                      "sales_invoices");
  gtk_stack_add_named(GTK_STACK(stack), salesScreens->getOnlineOrdersView(),
                      "sales_orders");
  gtk_stack_add_named(GTK_STACK(stack), salesScreens->getWarrantyIntakeView(),
                      "sales_warranty");

  // Add Warehouse views to Stack
  gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getDashboardView(),
                      "warehouse_dashboard");
  gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getInventoryView(),
                      "warehouse_inventory");
  gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getLowStockView(),
                      "warehouse_low_stock");
  gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getReceiptsView(),
                      "warehouse_receipts");
  gtk_stack_add_named(GTK_STACK(stack),
                      warehouseScreens->getCreateReceiptView(),
                      "warehouse_create_receipt");
  gtk_stack_add_named(GTK_STACK(stack), warehouseScreens->getFulfillmentView(),
                      "warehouse_fulfillment");

  // Add Admin views to Stack
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getDashboardView(),
                      "admin_dashboard");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getProductMgmtView(),
                      "admin_products");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getEmployeeMgmtView(),
                      "admin_employees");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getCustomerMgmtView(),
                      "admin_customers");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getSupplierMgmtView(),
                      "admin_suppliers");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getDiscountMgmtView(),
                      "admin_discounts");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getAccountMgmtView(),
                      "admin_accounts");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getAllOrdersView(),
                      "admin_orders");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getAllInvoicesView(),
                      "admin_invoices");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getWarrantyMonitorView(),
                      "admin_warranty");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getReportsView(),
                      "admin_reports");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getProfileView(),
                      "admin_profile");
  gtk_stack_add_named(GTK_STACK(stack), adminScreens->getRealTimeView(),
                      "admin_realtime");
}

void MainShellWindow::addSidebarButton(const std::string &pageName,
                                       const std::string &label,
                                       const std::string &icon) {
  GtkWidget *btn = gtk_button_new();
  gtk_widget_add_css_class(btn, "sidebar-btn");
  gtk_widget_set_name(btn, ("nav_" + pageName).c_str());
  gtk_widget_set_hexpand(btn, FALSE);
  UIComponents::setButtonCursor(btn);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  GtkWidget *iconLbl = gtk_label_new(icon.c_str());
  gtk_widget_add_css_class(iconLbl, "sidebar-icon-slot");
  GtkWidget *textLbl = gtk_label_new(label.c_str());
  gtk_widget_set_hexpand(textLbl, TRUE);
  gtk_label_set_xalign(GTK_LABEL(textLbl), 0.0f);
  gtk_label_set_ellipsize(GTK_LABEL(textLbl), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars(GTK_LABEL(textLbl), 1);

  gtk_box_append(GTK_BOX(box), iconLbl);
  gtk_box_append(GTK_BOX(box), textLbl);

  if (pageName == "customer_cart") {
    lblCartCountBadge = gtk_label_new("0");
    gtk_widget_add_css_class(lblCartCountBadge, "cart-count-badge");
    gtk_box_append(GTK_BOX(box), lblCartCountBadge);
  }

  gtk_button_set_child(GTK_BUTTON(btn), box);

  NavButtonData *data = new NavButtonData{this, pageName};
  g_signal_connect_data(
      btn, "clicked", G_CALLBACK(on_nav_btn_clicked), data,
      [](gpointer d, GClosure *) { delete static_cast<NavButtonData *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(sidebarBox), btn);
  sidebarButtons[pageName] = btn;
}

void MainShellWindow::refreshCurrentRoleUI() {
  const UserSessionViewModel &user = controller->getCurrentUser();

  // Update Header Pill
  std::string badgeText = "👤 " + user.fullName + "  •  " + user.roleName;
  gtk_label_set_text(GTK_LABEL(lblUserBadge), badgeText.c_str());

  // Clear current sidebar items
  GtkWidget *child = gtk_widget_get_first_child(sidebarBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(sidebarBox), child);
    child = next;
  }
  sidebarButtons.clear();

  auto addCategory = [this](const char *cat) {
    GtkWidget *l = gtk_label_new(cat);
    gtk_widget_add_css_class(l, "sidebar-category-label");
    gtk_label_set_xalign(GTK_LABEL(l), 0.0f);
    gtk_label_set_ellipsize(GTK_LABEL(l), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(l), 1);
    gtk_widget_set_hexpand(l, FALSE);
    gtk_box_append(GTK_BOX(this->sidebarBox), l);
  };

  std::string initialPage = "";

  if (user.isCustomer()) {
    addCategory("Khách hàng");
    addSidebarButton("customer_dashboard", "Tổng quan", "📊");
    addSidebarButton("customer_products", "Mua sắm điện thoại", "🛍️");
    addSidebarButton("customer_cart", "Giỏ hàng", "🛒");
    addSidebarButton("customer_orders", "Đơn hàng của tôi", "📦");
    addSidebarButton("customer_discounts", "Ví voucher & Ưu đãi", "🎟️");
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
      GtkWidget *div = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
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
    addSidebarButton("admin_profile", "Hồ sơ quản trị viên", "👤");
    addSidebarButton("admin_realtime", "Kiểm soát thời gian thực", "⏱️");

    initialPage = "admin_dashboard";
    adminScreens->refreshAll();
  }

  updateSimulatedTimeBadge();
  navigateTo(initialPage);
}

void MainShellWindow::startClockTimer() {
  stopClockTimer();
  clockTimerId = g_timeout_add(1000, on_clock_tick, this);
}

void MainShellWindow::stopClockTimer() {
  if (clockTimerId > 0) {
    g_source_remove(clockTimerId);
    clockTimerId = 0;
  }
}

gboolean MainShellWindow::on_clock_tick(gpointer data) {
  MainShellWindow *self = static_cast<MainShellWindow *>(data);
  if (!self || !self->window)
    return G_SOURCE_REMOVE;
  self->handleClockTick();
  return G_SOURCE_CONTINUE;
}

void MainShellWindow::handleClockTick() {
  if (!controller)
    return;
  controller->advanceSimulatedSeconds(1);
  updateSimulatedTimeBadge();
  if (currentPage == "admin_realtime" && adminScreens) {
    adminScreens->updateClockDisplay(controller->getSimulatedTime());
  }
}

void MainShellWindow::navigateTo(const std::string &pageName) {
  if (pageName.empty())
    return;

  // Bảo vệ phân quyền: Nhân viên kho (Warehouse) không có quyền xem các trang bảo hành
  const auto &currentUser = controller->getCurrentUser();
  if (currentUser.isWarehouse()) {
    if (pageName == "sales_warranty" || pageName == "admin_warranty" ||
        pageName == "customer_warranty") {
      return;
    }
  }

  // Đóng overlay dialog nếu đang mở khi chuyển trang
  closeOverlayDialog();

  // Tự động pop hết các trang đã push khi bấm mục sidebar khác
  clearPageStack();

  currentPage = pageName;

  // Switch stack view
  gtk_stack_set_visible_child_name(GTK_STACK(stack), pageName.c_str());

  // Update buttons active class
  for (auto &pair : sidebarButtons) {
    if (pair.first == pageName) {
      gtk_widget_add_css_class(pair.second, "sidebar-btn-active");
    } else {
      gtk_widget_remove_css_class(pair.second, "sidebar-btn-active");
    }
  }

  // Update Header Breadcrumb Title
  std::string pageTitle = "Tổng quan";
  if (pageName == "customer_dashboard")
    pageTitle = "Khách hàng / Tổng quan";
  else if (pageName == "customer_products")
    pageTitle = "Khách hàng / Mua sắm điện thoại";
  else if (pageName == "customer_cart")
    pageTitle = "Khách hàng / Giỏ hàng của bạn";
  else if (pageName == "customer_orders")
    pageTitle = "Khách hàng / Đơn hàng của tôi";
  else if (pageName == "customer_discounts")
    pageTitle = "Khách hàng / Ví Voucher & Ưu đãi";
  else if (pageName == "customer_profile")
    pageTitle = "Khách hàng / Hồ sơ cá nhân & Điểm thưởng";
  else if (pageName == "customer_warranty")
    pageTitle = "Khách hàng / Bảo hành thiết bị";
  else if (pageName == "sales_lookup" || pageName == "admin_products")
    pageTitle = "Sản phẩm điện thoại";
  else if (pageName == "customer_cart")
    pageTitle = "Giỏ hàng của bạn";
  else if (pageName == "customer_orders" || pageName == "sales_orders" ||
           pageName == "admin_orders")
    pageTitle = "Quản lý Đơn hàng";
  else if (pageName == "customer_discounts")
    pageTitle = "Ví Voucher & Mã giảm giá";
  else if (pageName == "customer_profile")
    pageTitle = "Hồ sơ cá nhân & Điểm thưởng";
  else if (pageName == "customer_warranty" || pageName == "sales_warranty" ||
           pageName == "admin_warranty")
    pageTitle = "Bảo hành thiết bị";
  else if (pageName == "sales_pos")
    pageTitle = "Bán hàng tại quầy (POS)";
  else if (pageName == "sales_customers" || pageName == "admin_customers")
    pageTitle = "Quản lý Khách hàng";
  else if (pageName == "sales_invoices" || pageName == "admin_invoices")
    pageTitle = "Quản lý Hóa đơn";
  else if (pageName == "warehouse_inventory")
    pageTitle = "Tồn kho thiết bị";
  else if (pageName == "warehouse_low_stock")
    pageTitle = "Cảnh báo sắp hết hàng";
  else if (pageName == "warehouse_receipts")
    pageTitle = "Phiếu nhập kho";
  else if (pageName == "warehouse_create_receipt")
    pageTitle = "Lập phiếu nhập kho";
  else if (pageName == "warehouse_fulfillment")
    pageTitle = "Chuẩn bị đơn hàng";
  else if (pageName == "admin_employees")
    pageTitle = "Quản lý Nhân sự";
  else if (pageName == "admin_suppliers")
    pageTitle = "Quản lý Nhà cung cấp";
  else if (pageName == "admin_discounts")
    pageTitle = "Quản lý Mã giảm giá";
  else if (pageName == "admin_accounts")
    pageTitle = "Tài khoản & Phân quyền";
  else if (pageName == "admin_profile")
    pageTitle = "Hồ sơ quản trị viên";
  else if (pageName == "admin_reports")
    pageTitle = "Báo cáo Doanh thu & Lợi nhuận";
  else if (pageName == "admin_realtime")
    pageTitle = "Kiểm soát Thời gian thực Hệ thống";

  gtk_label_set_text(GTK_LABEL(lblHeaderTitle), pageTitle.c_str());

  // Targeted Screen Refreshes
  if (salesScreens) {
    if (pageName == "sales_pos") {
      salesScreens->refreshPosCustomers();
      salesScreens->focusPosSearch();
    } else if (pageName == "sales_orders") {
      salesScreens->refreshOnlineOrders();
    } else if (pageName == "sales_invoices") {
      salesScreens->refreshInvoices();
    } else if (pageName == "sales_dashboard") {
      salesScreens->refreshDashboard();
    } else if (pageName == "sales_customers") {
      salesScreens->refreshCustomers();
    } else if (pageName == "sales_lookup") {
      salesScreens->refreshProductLookup();
    } else if (pageName == "sales_warranty") {
      salesScreens->refreshWarrantyIntake();
    }
  }

  if (adminScreens) {
    if (pageName == "admin_orders") {
      adminScreens->refreshOrders();
    } else if (pageName == "admin_invoices") {
      adminScreens->refreshInvoices();
    } else if (pageName == "admin_realtime") {
      adminScreens->refreshRealTime();
    } else if (pageName == "admin_discounts") {
      adminScreens->refreshDiscounts();
    } else if (pageName == "admin_reports") {
      adminScreens->refreshReports();
    } else if (pageName == "admin_dashboard") {
      adminScreens->refreshDashboard();
    } else if (pageName == "admin_products") {
      adminScreens->refreshProducts();
    } else if (pageName == "admin_employees") {
      adminScreens->refreshEmployees();
    } else if (pageName == "admin_customers") {
      adminScreens->refreshCustomers();
    } else if (pageName == "admin_suppliers") {
      adminScreens->refreshSuppliers();
    } else if (pageName == "admin_accounts") {
      adminScreens->refreshAccounts();
    } else if (pageName == "admin_warranty") {
      adminScreens->refreshWarrantyMonitor();
    }
  }

  if (customerScreens) {
    if (pageName == "customer_dashboard") {
      customerScreens->refreshDashboard();
    } else if (pageName == "customer_orders") {
      customerScreens->refreshOrders();
    } else if (pageName == "customer_discounts") {
      customerScreens->refreshDiscounts();
    } else if (pageName == "customer_products") {
      customerScreens->refreshProducts();
    } else if (pageName == "customer_cart") {
      customerScreens->refreshCart();
    } else if (pageName == "customer_profile") {
      customerScreens->refreshProfile();
    } else if (pageName == "customer_warranty") {
      customerScreens->refreshWarranty();
    }
  }

  if (warehouseScreens) {
    if (pageName == "warehouse_dashboard") {
      warehouseScreens->refreshDashboard();
    } else if (pageName == "warehouse_inventory") {
      warehouseScreens->refreshInventory();
    } else if (pageName == "warehouse_low_stock") {
      warehouseScreens->refreshLowStock();
    } else if (pageName == "warehouse_receipts") {
      warehouseScreens->refreshReceipts();
    } else if (pageName == "warehouse_fulfillment") {
      warehouseScreens->refreshFulfillmentQueue();
    }
  }
}

void MainShellWindow::updateCartBadge(int count) {
  if (lblCartCountBadge) {
    gtk_label_set_text(GTK_LABEL(lblCartCountBadge),
                       std::to_string(count).c_str());
    gtk_widget_set_visible(lblCartCountBadge, count > 0);
  }
}

void MainShellWindow::updateSimulatedTimeBadge() {
  if (lblSimulatedTimeBadge && controller) {
    std::string badge = controller->getSimulatedTime();
    gtk_label_set_text(GTK_LABEL(lblSimulatedTimeBadge), badge.c_str());
  }
}

void MainShellWindow::on_nav_btn_clicked(GtkButton *btn, gpointer data) {
  (void)btn;
  NavButtonData *nd = static_cast<NavButtonData *>(data);
  nd->shell->navigateTo(nd->pageName);
}

void MainShellWindow::on_logout_btn_clicked(GtkButton *btn, gpointer data) {
  (void)btn;
  MainShellWindow *self = static_cast<MainShellWindow *>(data);
  UIComponents::showConfirmDialog(
      self->getWindow(), "Xác nhận đăng xuất",
      "Bạn có chắc chắn muốn đăng xuất khỏi phiên làm việc hiện tại không?",
      on_confirm_logout, self);
}

void MainShellWindow::on_confirm_logout(gpointer data) {
  MainShellWindow *self = static_cast<MainShellWindow *>(data);
  self->deactivateSession();
  self->controller->logout();
  if (self->logoutCallback) {
    self->logoutCallback(self->controller, self->callbackData);
  }
}

gboolean MainShellWindow::on_window_close_request(GtkWindow *closedWindow,
                                                  gpointer data) {
  MainShellWindow *self = static_cast<MainShellWindow *>(data);
  self->stopClockTimer();
  if (self->window == GTK_WIDGET(closedWindow)) {
    self->window = nullptr;
  }
  return FALSE;
}

void MainShellWindow::pushPage(const std::string &id, GtkWidget *content,
                               const std::string &title, bool showTopBack) {
  if (!pageNavStack || !content)
    return;

  // Tạo container bao bọc trang con
  GtkWidget *pageBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(pageBox, "page-container-view");
  gtk_widget_set_hexpand(pageBox, TRUE);
  gtk_widget_set_vexpand(pageBox, TRUE);

  // Thanh tiêu đề trang (.page-header)
  GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(header, "page-header");

  // Nút Quay lại (.page-back-btn) chỉ hiển thị khi trang không tự bố trí nút điều hướng
  if (showTopBack) {
    GtkWidget *btnBack = gtk_button_new_with_label("← Quay lại");
    gtk_widget_add_css_class(btnBack, "page-back-btn");
    UIComponents::setButtonCursor(btnBack);
    g_signal_connect_swapped(btnBack, "clicked",
                             G_CALLBACK(+[](gpointer d) {
                               static_cast<MainShellWindow *>(d)->popPage();
                             }),
                             this);
    gtk_box_append(GTK_BOX(header), btnBack);
  }

  // Tiêu đề trang (.page-title)
  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "page-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_widget_set_hexpand(lblTitle, TRUE);
  gtk_box_append(GTK_BOX(header), lblTitle);

  gtk_box_append(GTK_BOX(pageBox), header);

  // Gắn nội dung trang vào bên dưới header
  gtk_widget_set_hexpand(content, TRUE);
  gtk_widget_set_vexpand(content, TRUE);
  gtk_box_append(GTK_BOX(pageBox), content);

  // Định danh cho child trong GtkStack
  std::string childName =
      "page_" + std::to_string(pageStack.size()) + "_" + id;
  gtk_stack_add_named(GTK_STACK(pageNavStack), pageBox, childName.c_str());

  // Transition OVER_LEFT (260ms — trang mới trượt đè êm ái từ phải sang trái)
  gtk_stack_set_transition_type(GTK_STACK(pageNavStack),
                                GTK_STACK_TRANSITION_TYPE_OVER_LEFT);
  gtk_stack_set_transition_duration(GTK_STACK(pageNavStack), 260);
  gtk_stack_set_visible_child(GTK_STACK(pageNavStack), pageBox);

  // Lưu vào ngăn xếp
  pageStack.push_back({id, title, childName, pageBox, content});
}

void MainShellWindow::popPage() {
  if (pageStack.empty() || !pageNavStack)
    return;

  PageEntry top = pageStack.back();
  pageStack.pop_back();

  // Transition UNDER_RIGHT (260ms — trang hiện tại trượt lùi về phải để lộ trang dưới)
  gtk_stack_set_transition_type(GTK_STACK(pageNavStack),
                                GTK_STACK_TRANSITION_TYPE_UNDER_RIGHT);
  gtk_stack_set_transition_duration(GTK_STACK(pageNavStack), 260);

  if (!pageStack.empty()) {
    gtk_stack_set_visible_child(GTK_STACK(pageNavStack),
                                pageStack.back().pageBox);
  } else {
    gtk_stack_set_visible_child_name(GTK_STACK(pageNavStack), "__main_views__");
  }

  // Giải phóng widget an toàn sau khi kết thúc animation (280ms)
  GtkWidget *oldPage = top.pageBox;
  g_object_ref(oldPage);

  PageCleanupRecord rec{0, pageNavStack, oldPage};
  pendingCleanups.push_back(rec);
  size_t recIdx = pendingCleanups.size() - 1;

  guint tid = g_timeout_add(
      280,
      +[](gpointer d) -> gboolean {
        GtkWidget *page = GTK_WIDGET(d);
        if (s_activeShellInstance) {
          auto &list = s_activeShellInstance->pendingCleanups;
          list.erase(std::remove_if(list.begin(), list.end(),
                                    [page](const PageCleanupRecord &r) {
                                      return r.page == page;
                                    }),
                     list.end());
          if (s_activeShellInstance->pageNavStack &&
              GTK_IS_STACK(s_activeShellInstance->pageNavStack) &&
              GTK_IS_WIDGET(page)) {
            if (gtk_widget_get_parent(page) ==
                s_activeShellInstance->pageNavStack) {
              gtk_stack_remove(
                  GTK_STACK(s_activeShellInstance->pageNavStack), page);
            }
          }
        }
        if (page && GTK_IS_WIDGET(page)) {
          g_object_unref(page);
        }
        return G_SOURCE_REMOVE;
      },
      oldPage);

  pendingCleanups[recIdx].timerId = tid;
}

void MainShellWindow::clearPageStack() {
  if (pageStack.empty() || !pageNavStack)
    return;

  gtk_stack_set_transition_type(GTK_STACK(pageNavStack),
                                GTK_STACK_TRANSITION_TYPE_NONE);
  gtk_stack_set_visible_child_name(GTK_STACK(pageNavStack), "__main_views__");

  for (auto &entry : pageStack) {
    if (entry.pageBox && GTK_IS_WIDGET(entry.pageBox)) {
      if (gtk_widget_get_parent(entry.pageBox) == pageNavStack) {
        gtk_stack_remove(GTK_STACK(pageNavStack), entry.pageBox);
      }
    }
  }
  pageStack.clear();
  gtk_stack_set_transition_duration(GTK_STACK(pageNavStack), 260);
}

void MainShellWindow::showOverlayDialog(GtkWidget *card) {
  if (!toastOverlay || !card)
    return;

  // Nếu đang có dialog khác mở, đóng trước
  closeOverlayDialog();

  currentOverlayWrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(currentOverlayWrapper, "overlay-scrim");
  gtk_widget_add_css_class(currentOverlayWrapper, "overlay-entering");
  gtk_widget_set_hexpand(currentOverlayWrapper, TRUE);
  gtk_widget_set_vexpand(currentOverlayWrapper, TRUE);
  gtk_widget_set_halign(currentOverlayWrapper, GTK_ALIGN_FILL);
  gtk_widget_set_valign(currentOverlayWrapper, GTK_ALIGN_FILL);

  currentOverlayCard = card;
  gtk_widget_add_css_class(card, "overlay-card");
  gtk_widget_set_halign(card, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(card, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(card, TRUE);
  gtk_widget_set_vexpand(card, TRUE);
  gtk_box_append(GTK_BOX(currentOverlayWrapper), card);

  // Gesture click vào nền mờ (scrim) bên ngoài card để đóng
  GtkGesture *click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);
  g_signal_connect(click, "pressed",
                   G_CALLBACK(+[](GtkGestureClick *, int, double x, double y,
                                  gpointer d) {
                     MainShellWindow *self = static_cast<MainShellWindow *>(d);
                     if (!self || !self->currentOverlayWrapper)
                       return;
                     GtkWidget *picked = gtk_widget_pick(
                         self->currentOverlayWrapper, x, y, GTK_PICK_DEFAULT);
                     if (picked == self->currentOverlayWrapper) {
                       self->closeOverlayDialog();
                     }
                   }),
                   this);
  gtk_widget_add_controller(currentOverlayWrapper,
                            GTK_EVENT_CONTROLLER(click));

  gtk_overlay_add_overlay(GTK_OVERLAY(toastOverlay), currentOverlayWrapper);

  // Hiệu ứng fade-in mượt mà cho lớp nền mờ (.overlay-scrim)
  GtkWidget *scrimRef = GTK_WIDGET(g_object_ref(currentOverlayWrapper));
  g_idle_add(
      +[](gpointer d) -> gboolean {
        GtkWidget *w = GTK_WIDGET(d);
        if (w && GTK_IS_WIDGET(w)) {
          gtk_widget_remove_css_class(w, "overlay-entering");
          g_object_unref(w);
        }
        return G_SOURCE_REMOVE;
      },
      scrimRef);

  gtk_widget_child_focus(card, GTK_DIR_TAB_FORWARD);
}

void MainShellWindow::closeOverlayDialog() {
  if (!currentOverlayWrapper)
    return;

  GtkWidget *wrapper = currentOverlayWrapper;
  currentOverlayWrapper = nullptr;
  currentOverlayCard = nullptr;

  if (toastOverlay && GTK_IS_OVERLAY(toastOverlay) &&
      gtk_widget_get_parent(wrapper) == toastOverlay) {
    gtk_overlay_remove_overlay(GTK_OVERLAY(toastOverlay), wrapper);
  }
}

gboolean MainShellWindow::on_key_pressed(GtkEventControllerKey *controller,
                                        guint keyval, guint keycode,
                                        GdkModifierType state, gpointer data) {
  (void)controller;
  (void)keycode;
  (void)state;
  MainShellWindow *self = static_cast<MainShellWindow *>(data);
  if (!self || s_activeShellInstance != self)
    return GDK_EVENT_PROPAGATE;

  if (keyval == GDK_KEY_Escape) {
    // Ưu tiên đóng overlay dialog trước
    if (self->isOverlayDialogOpen()) {
      self->closeOverlayDialog();
      return GDK_EVENT_STOP;
    }
    // Sau đó đến pop trang con
    if (self->hasPages()) {
      self->popPage();
      return GDK_EVENT_STOP;
    }
  }
  return GDK_EVENT_PROPAGATE;
}
