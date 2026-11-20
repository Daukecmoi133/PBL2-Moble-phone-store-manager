/**
 * @file SalesScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ NHÂN VIÊN BÁN HÀNG (SALES REPRESENTATIVE)
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildDashboardView & refreshDashboard: Bàn làm việc Saler, thống kê doanh
 * số và đơn cần xử lý.
 * - buildPosView & refreshPos: Bán hàng tại quầy (POS), tính toán chiết khấu
 * voucher, tiền khách đưa và tiền thối.
 * - buildProductLookupView: Tra cứu cấu hình, giá niêm yết và tồn kho thực tế.
 * - buildCustomerMgmtView: Thêm khách hàng mới, tra cứu thông tin khách hàng và
 * lịch sử.
 * - buildInvoicesView: Danh sách hóa đơn bán hàng đã phát hành.
 * - buildOnlineOrdersView: Tra cứu chi tiết đơn hàng online (sản phẩm, số
 * lượng, địa chỉ).
 * - buildWarrantyIntakeView: Tiếp nhận thiết bị bảo hành từ khách hàng.
 * =========================================================================================
 */

#include "SalesScreens.h"
#include "../../source/utils/ExportHelper.h"
#include "../../source/utils/PathHelper.h"
#include "../../source/utils/PricingCalculator.h"
#include "../../source/utils/StringHelper.h"
#include "../../source/utils/Utf8Helper.h"
#include "../components/ProductDetailView.h"
#include "../components/UIComponents.h"
#include "../mock/RealTimeManager.h"
#include "../styles/AppCss.h"
#include "MainShellWindow.h"
#include <algorithm>
#include <cmath>

namespace {
const std::vector<UIComponents::TableColumnSpec> kPosCartCols = {
    {"Sản phẩm & Mã máy", 200, 0.0f, true},
    {"Đơn giá", 145, 1.0f, false},
    {"Số lượng", 150, 0.5f, false},
    {"Thành tiền", 150, 1.0f, false},
    {"Xóa", 95, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesLookupCols = {
    {"Tên thiết bị & Cấu hình", 260, 0.0f, true},
    {"Trạng thái", 150, 0.5f, false},
    {"Tồn khả dụng", 135, 1.0f, false},
    {"Giá niêm yết", 150, 1.0f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesCustCols = {
    {"Mã KH", 130, 0.0f, false},
    {"Họ và tên", 190, 0.0f, false},
    {"Thông tin liên hệ (SĐT / Email / Địa chỉ)", 240, 0.0f, true},
    {"Hạng TV", 155, 0.5f, false},
    {"Điểm tích lũy", 135, 1.0f, false},
    {"Thao tác", 115, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesInvoiceCols = {
    {"Mã HĐ", 190, 0.0f, false},     {"Mã đơn", 190, 0.0f, false},
    {"Khách hàng", 180, 0.0f, true}, {"Hình thức TT", 150, 0.5f, false},
    {"Thời gian", 145, 0.0f, false}, {"Tổng tiền", 150, 1.0f, false},
    {"Thao tác", 115, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesOrderCols = {
    {"Mã đơn", 190, 0.0f, false},     {"Khách hàng / SĐT", 180, 0.0f, true},
    {"Giao nhận", 155, 0.0f, false},  {"Thời gian", 165, 0.0f, false},
    {"Trạng thái", 155, 0.5f, false}, {"Thanh toán", 155, 0.5f, false},
    {"Tổng tiền", 150, 1.0f, false},  {"Xử lý đơn", 160, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesWarrantyCols = {
    {"Mã phiếu & Mã đơn", 185, 0.0f, false},
    {"Thiết bị & Bảo hành", 200, 0.0f, false},
    {"Khách hàng", 175, 0.0f, false},
    {"Ngày nhận", 145, 0.0f, false},
    {"Trạng thái", 145, 0.5f, false},
    {"Mô tả sự cố & Ghi chú", 220, 0.0f, true},
    {"Thao tác", 110, 0.5f, false}};
} // namespace

SalesScreens::SalesScreens(AppController *ctrl, MainShellWindow *parentShell)
    : controller(ctrl), shell(parentShell), posSelectedCustomerId("WALK-IN"),
      posAppliedCoupon("") {

  dashboardView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  posView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(posView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  productLookupView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productLookupView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  customerMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(customerMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  invoicesView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(invoicesView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  onlineOrdersView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(onlineOrdersView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  warrantyIntakeView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyIntakeView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  buildDashboardView();
  buildPosView();
  buildProductLookupView();
  buildCustomerMgmtView();
  buildInvoicesView();
  buildOnlineOrdersView();
  buildWarrantyIntakeView();
}

SalesScreens::~SalesScreens() {
  if (posSearchPopover && gtk_widget_get_parent(posSearchPopover)) {
    gtk_popover_popdown(GTK_POPOVER(posSearchPopover));
    gtk_widget_unparent(posSearchPopover);
    posSearchPopover = nullptr;
  }
}

void SalesScreens::refreshAll() {
  refreshDashboard();
  refreshPos();
  refreshPosCustomers();
  refreshProductLookup();
  refreshCustomers();
  refreshInvoices();
  refreshOnlineOrders();
  refreshWarrantyIntake();
}

// ================= 1. SALES DASHBOARD =================

void SalesScreens::buildDashboardView() {

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  const UserSessionViewModel &user = controller->getCurrentUser();
  std::string title = "Bàn làm việc Nhân viên Bán hàng — " + user.fullName;
  GtkWidget *header = UIComponents::createSectionHeader(
      title,
      "Theo dõi các giao dịch bán hàng, đơn hàng online và hỗ trợ khách hàng.");
  gtk_box_append(GTK_BOX(content), header);

  // Metric Cards (Role-specific KPIs for SALE_EMPLOYEE)
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

  std::vector<OrderItemViewModel> orders = controller->getAllOrders();
  std::vector<InvoiceItemViewModel> invoices = controller->getAllInvoices();
  std::vector<CustomerItemViewModel> custs = controller->getCustomers();

  int pendingOrders = 0;
  int activeDeliveryOrders = 0;
  for (const auto &o : orders) {
    if (o.status == OrderStatus::PENDING)
      pendingOrders++;
    else if (o.status == OrderStatus::SHIPPING ||
             o.status == OrderStatus::READY_FOR_PICKUP)
      activeDeliveryOrders++;
  }

  double todayRev = 0.0;
  for (const auto &inv : invoices) {
    todayRev += inv.netRevenue;
  }

  GtkWidget *c1 = UIComponents::createClickableStatCard(
      "Doanh thu bán hàng", UIComponents::formatVND(todayRev),
      "Tổng " + UIComponents::formatNumber(invoices.size()) +
          " hóa đơn hợp lệ đã xuất",
      "💰", "green",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        SalesScreens *s = static_cast<SalesScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("sales_invoices");
      }),
      this);
  gtk_widget_set_tooltip_text(c1, "Nhấp để xem danh sách hóa đơn bán hàng");

  GtkWidget *c2 = UIComponents::createClickableStatCard(
      "Đơn online chờ duyệt",
      UIComponents::formatNumber(pendingOrders) + " đơn",
      pendingOrders > 0 ? "Cần xác nhận tiếp nhận ngay"
                        : "Đã xử lý hết đơn chờ",
      "⏳", pendingOrders > 0 ? "amber" : "blue",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        SalesScreens *s = static_cast<SalesScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("sales_orders");
      }),
      this);
  gtk_widget_set_tooltip_text(c2, "Nhấp để xử lý đơn hàng online chờ duyệt");

  GtkWidget *c3 = UIComponents::createClickableStatCard(
      "Đơn đang giao / Chờ nhận",
      UIComponents::formatNumber(activeDeliveryOrders) + " đơn",
      "Sẵn sàng hoàn tất giao khách", "🚚",
      activeDeliveryOrders > 0 ? "cyan" : "blue",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        SalesScreens *s = static_cast<SalesScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("sales_orders");
      }),
      this);
  gtk_widget_set_tooltip_text(
      c3, "Nhấp để theo dõi đơn hàng đang giao / chờ nhận");

  GtkWidget *c4 = UIComponents::createClickableStatCard(
      "Khách hàng hệ thống",
      UIComponents::formatNumber(custs.size()) + " khách",
      "Tra cứu SĐT & tích điểm tại quầy", "👥", "purple",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        SalesScreens *s = static_cast<SalesScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("sales_customers");
      }),
      this);
  gtk_widget_set_tooltip_text(c4, "Nhấp để quản lý và tra cứu khách hàng");

  gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c4, 3, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), grid);

  // Quick POS Action Card
  GtkWidget *quickCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(quickCard, "card");

  GtkWidget *lblQuick = gtk_label_new("⚡ Thao tác nghiệp vụ nhanh");
  gtk_widget_add_css_class(lblQuick, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblQuick), 0.0f);
  gtk_box_append(GTK_BOX(quickCard), lblQuick);

  GtkWidget *btnRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);

  GtkWidget *bPos =
      gtk_button_new_with_label("🖥️ Mở máy bán hàng tại quầy (POS)");
  gtk_widget_add_css_class(bPos, "btn-primary");
  UIComponents::setButtonCursor(bPos);
  g_signal_connect_swapped(bPos, "clicked", G_CALLBACK(+[](MainShellWindow *s) {
                             s->navigateTo("sales_pos");
                           }),
                           shell);
  gtk_box_append(GTK_BOX(btnRow), bPos);

  GtkWidget *bOrders = gtk_button_new_with_label("📦 Xử lý đơn online");
  gtk_widget_add_css_class(bOrders, "btn-secondary");
  UIComponents::setButtonCursor(bOrders);
  g_signal_connect_swapped(
      bOrders, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->navigateTo("sales_orders"); }),
      shell);
  gtk_box_append(GTK_BOX(btnRow), bOrders);

  GtkWidget *bCust = gtk_button_new_with_label("👥 Thêm khách hàng mới");
  gtk_widget_add_css_class(bCust, "btn-outline");
  UIComponents::setButtonCursor(bCust);
  g_signal_connect_swapped(bCust, "clicked", G_CALLBACK(+[](SalesScreens *s) {
                             s->showCustomerModal(nullptr);
                           }),
                           this);
  gtk_box_append(GTK_BOX(btnRow), bCust);

  gtk_box_append(GTK_BOX(quickCard), btnRow);
  gtk_box_append(GTK_BOX(content), quickCard);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(dashboardView), content);
}

void SalesScreens::refreshDashboard() { buildDashboardView(); }

// ================= 2. MÀN HÌNH BÁN HÀNG TẠI QUẦY (POS) =================

void SalesScreens::buildPosView() {
  posView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(posView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Bán hàng tại quầy (Point of Sale)",
      "Tạo đơn hàng, thêm sản phẩm nhanh bằng bàn phím và lập, hiển thị hóa "
      "đơn trực tiếp cho khách.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);

  // Left Panel: Cart & Items selection
  GtkWidget *leftCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(leftCard, "card");
  gtk_widget_set_hexpand(leftCard, TRUE);

  // Cart Title & Live Item Count Badge
  GtkWidget *cartHeaderRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  GtkWidget *lblCartTitle = gtk_label_new("🛒 Giỏ hàng giao dịch tại quầy");
  gtk_widget_add_css_class(lblCartTitle, "card-title");
  gtk_widget_set_hexpand(lblCartTitle, TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblCartTitle), 0.0f);
  gtk_box_append(GTK_BOX(cartHeaderRow), lblCartTitle);

  posCartCountBadge = gtk_label_new("Giỏ hàng: 0 mẫu (0 máy)");
  gtk_widget_add_css_class(posCartCountBadge, "record-count-badge");
  gtk_widget_set_valign(posCartCountBadge, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(cartHeaderRow), posCartCountBadge);
  gtk_box_append(GTK_BOX(leftCard), cartHeaderRow);

  GtkWidget *topBar =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);

  // Customer Dropdown
  const char *initCust[] = {"Khách vãng lai (Không tích điểm)", nullptr};
  posComboCustomer = gtk_drop_down_new_from_strings(initCust);
  gtk_widget_set_hexpand(posComboCustomer, TRUE);
  gtk_box_append(GTK_BOX(topBar), posComboCustomer);

  g_signal_connect(posComboCustomer, "notify::selected",
                   G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                     SalesScreens *self = static_cast<SalesScreens *>(d);
                     guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                     if (sel < self->posCustomerIds.size()) {
                       self->posSelectedCustomerId = self->posCustomerIds[sel];
                     }
                   }),
                   this);

  posBtnAddProd = gtk_button_new_with_label("➕ Chọn sản phẩm (F2)");
  gtk_widget_add_css_class(posBtnAddProd, "btn-primary");
  UIComponents::setButtonCursor(posBtnAddProd);
  g_signal_connect_swapped(
      posBtnAddProd, "clicked",
      G_CALLBACK(+[](SalesScreens *s) { s->showPosProductPickerModal(); }),
      this);
  gtk_box_append(GTK_BOX(topBar), posBtnAddProd);

  // Gắn thanh công cụ khách hàng và nút chọn sản phẩm vào thẻ giỏ hàng
  gtk_box_append(GTK_BOX(leftCard), topBar);

  // Table container for POS Cart
  GtkWidget *cartTableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(cartTableContainer, "table-container");
  gtk_widget_set_vexpand(cartTableContainer, TRUE);
  gtk_box_append(GTK_BOX(cartTableContainer),
                 UIComponents::createTableHeaderRow(kPosCartCols));

  // Items list container (cuộn độc lập)
  posCartListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  GtkWidget *cartScroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(cartScroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(cartScroll),
                                             300);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(cartScroll),
                                posCartListBox);
  gtk_widget_set_vexpand(cartScroll, TRUE);
  gtk_box_append(GTK_BOX(cartTableContainer), cartScroll);
  gtk_box_append(GTK_BOX(leftCard), cartTableContainer);

  GtkWidget *hintRow = gtk_label_new(
      "⌨️ Enter: Thêm nhanh theo từ khóa   •   F2: Mở danh mục chọn máy   •   "
      "F9: Thanh toán & Lập HĐ   •   Esc: Xóa giỏ hàng");
  gtk_widget_add_css_class(hintRow, "shortcut-hint");
  gtk_label_set_xalign(GTK_LABEL(hintRow), 0.0f);
  gtk_box_append(GTK_BOX(leftCard), hintRow);

  gtk_widget_set_vexpand(leftCard, TRUE);
  gtk_grid_attach(GTK_GRID(grid), leftCard, 0, 0, 1, 1);

  // Right Panel: Payment & Checkout
  GtkWidget *payCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(payCard, "card");
  gtk_widget_set_size_request(payCard, 384, -1);

  GtkWidget *lblPayTitle = gtk_label_new("💳 Thanh toán hóa đơn");
  gtk_widget_add_css_class(lblPayTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblPayTitle), 0.0f);
  gtk_box_append(GTK_BOX(payCard), lblPayTitle);

  // Totals labels
  posLblSubtotal = gtk_label_new("Tạm tính: 0 ₫");
  gtk_widget_add_css_class(posLblSubtotal, "stat-label");
  gtk_widget_add_css_class(posLblSubtotal, "tabular-num");
  gtk_label_set_xalign(GTK_LABEL(posLblSubtotal), 0.0f);
  gtk_box_append(GTK_BOX(payCard), posLblSubtotal);

  // Coupon
  GtkWidget *couponRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  posEntryCoupon = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(posEntryCoupon),
                                 "Nhập mã voucher (Enter để áp dụng)...");
  gtk_widget_set_hexpand(posEntryCoupon, TRUE);

  GtkWidget *btnAppCoupon = gtk_button_new_with_label("Áp dụng");
  gtk_widget_add_css_class(btnAppCoupon, "btn-secondary");
  UIComponents::setButtonCursor(btnAppCoupon);
  auto applyCouponCb = +[](GtkWidget *, gpointer d) {
    SalesScreens *self = static_cast<SalesScreens *>(d);
    self->posAppliedCoupon =
        gtk_editable_get_text(GTK_EDITABLE(self->posEntryCoupon));
    self->refreshPos();
  };
  g_signal_connect_data(btnAppCoupon, "clicked", G_CALLBACK(applyCouponCb),
                        this, nullptr, (GConnectFlags)0);
  g_signal_connect_data(posEntryCoupon, "activate", G_CALLBACK(applyCouponCb),
                        this, nullptr, (GConnectFlags)0);

  gtk_box_append(GTK_BOX(couponRow), posEntryCoupon);
  gtk_box_append(GTK_BOX(couponRow), btnAppCoupon);
  gtk_box_append(GTK_BOX(payCard),
                 UIComponents::createFormField("Mã giảm giá", couponRow));

  posLblDiscount = gtk_label_new("Giảm giá: 0 ₫");
  gtk_widget_add_css_class(posLblDiscount, "stat-label");
  gtk_widget_add_css_class(posLblDiscount, "tabular-num");
  gtk_label_set_xalign(GTK_LABEL(posLblDiscount), 0.0f);
  gtk_box_append(GTK_BOX(payCard), posLblDiscount);

  posLblVat = gtk_label_new("Thuế VAT: 0 ₫");
  gtk_widget_add_css_class(posLblVat, "stat-label");
  gtk_widget_add_css_class(posLblVat, "tabular-num");
  gtk_label_set_xalign(GTK_LABEL(posLblVat), 0.0f);
  gtk_box_append(GTK_BOX(payCard), posLblVat);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(payCard), sep);

  GtkWidget *totalBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  gtk_widget_add_css_class(totalBox, "pos-total-box");
  GtkWidget *totalCaption = gtk_label_new("TỔNG THANH TOÁN");
  gtk_widget_add_css_class(totalCaption, "pos-total-label");
  gtk_label_set_xalign(GTK_LABEL(totalCaption), 0.0f);
  gtk_box_append(GTK_BOX(totalBox), totalCaption);
  posLblGrandTotal = gtk_label_new("0 ₫");
  gtk_widget_add_css_class(posLblGrandTotal, "pos-total-value");
  gtk_widget_add_css_class(posLblGrandTotal, "tabular-num");
  gtk_label_set_xalign(GTK_LABEL(posLblGrandTotal), 1.0f);
  gtk_box_append(GTK_BOX(totalBox), posLblGrandTotal);
  gtk_box_append(GTK_BOX(payCard), totalBox);

  const char *payMethods[] = {"Tiền mặt", "Chuyển khoản", nullptr};
  posComboPayMethod = gtk_drop_down_new_from_strings(payMethods);
  gtk_box_append(GTK_BOX(payCard),
                 UIComponents::createFormField("Phương thức thanh toán",
                                               posComboPayMethod));

  posEntryCash = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(posEntryCash),
                                 "Nhập số tiền khách đưa...");
  UIComponents::setupCurrencyInput(posEntryCash);
  gtk_box_append(GTK_BOX(payCard),
                 UIComponents::createFormField("Tiền khách đưa (nếu tiền mặt)",
                                               posEntryCash));

  posLblChange = gtk_label_new("Tiền thừa trả khách: 0 ₫");
  gtk_widget_add_css_class(posLblChange, "stat-trend");
  gtk_widget_add_css_class(posLblChange, "tabular-num");
  gtk_label_set_xalign(GTK_LABEL(posLblChange), 0.0f);
  gtk_box_append(GTK_BOX(payCard), posLblChange);

  g_signal_connect_data(
      posEntryCash, "changed", G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        double cashGiven = UIComponents::parseCurrencyInput(GTK_WIDGET(e));
        double subtotal = 0.0;
        for (const auto &item : self->posCartItems)
          subtotal += item.subtotal;
        double discount = 0.0;
        if (!self->posAppliedCoupon.empty()) {
          const std::string custId =
              (self->posSelectedCustomerId == "WALK-IN")
                  ? ""
                  : self->posSelectedCustomerId;
          self->controller->validateDiscountForCustomer(
              self->posAppliedCoupon, subtotal, custId, discount);
        }
        const PricingBreakdown pb = PricingCalculator::computeFromDiscountAmount(
            subtotal, discount, self->controller->getVatRate());
        double change = cashGiven - pb.grandTotal;
        if (change < 0 || cashGiven == 0)
          change = 0.0;
        gtk_label_set_text(
            GTK_LABEL(self->posLblChange),
            ("Tiền thừa trả khách: " + UIComponents::formatVND(change))
                .c_str());
      }),
      this, nullptr, (GConnectFlags)0);

  // Confirm Payment Button
  GtkWidget *btnSubmitPos =
      gtk_button_new_with_label("✓ Thanh toán & Lập HĐ (F9)");
  gtk_widget_add_css_class(btnSubmitPos, "btn-success");
  gtk_widget_add_css_class(btnSubmitPos, "btn-pay");
  UIComponents::setButtonCursor(btnSubmitPos);
  gtk_widget_set_margin_top(btnSubmitPos, DesignTokens::SPACE_SM);
  gtk_widget_set_vexpand(btnSubmitPos, TRUE);
  gtk_widget_set_valign(btnSubmitPos, GTK_ALIGN_END);
  posBtnPay = btnSubmitPos;

  g_signal_connect_data(
      btnSubmitPos, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        SalesScreens *self = static_cast<SalesScreens *>(d);
        if (self->posCartItems.empty()) {
          UIComponents::showWarningDialog(
              self->shell ? self->shell->getWindow() : nullptr, "Giỏ hàng rỗng",
              "Vui lòng thêm sản phẩm vào giao dịch!");
          return;
        }

        std::string custId = self->posSelectedCustomerId.empty()
                                 ? "WALK-IN"
                                 : self->posSelectedCustomerId;

        guint paySel =
            gtk_drop_down_get_selected(GTK_DROP_DOWN(self->posComboPayMethod));
        PaymentMethod payMethod =
            (paySel == 0) ? PaymentMethod::CASH : PaymentMethod::BANK_TRANSFER;

        double cashGiven = UIComponents::parseCurrencyInput(self->posEntryCash);

        double change = 0.0;
        std::string invId;
        const UserSessionViewModel &curUser =
            self->controller->getCurrentUser();
        OperationResult res = self->controller->createPosInvoice(
            custId, curUser.userId, self->posCartItems, self->posAppliedCoupon,
            payMethod, cashGiven, change, invId);

        if (res.success) {
          self->posCartItems.clear();
          self->posAppliedCoupon = "";
          self->posSelectedCustomerId = "WALK-IN";
          if (self->posComboCustomer &&
              GTK_IS_DROP_DOWN(self->posComboCustomer)) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(self->posComboCustomer),
                                       0);
          }
          gtk_editable_set_text(GTK_EDITABLE(self->posEntryCoupon), "");
          gtk_editable_set_text(GTK_EDITABLE(self->posEntryCash), "");
          self->refreshPos();
          self->refreshInvoices();
          self->refreshDashboard();
          self->refreshProductLookup();
          std::string msg = res.message + "\nTiền thừa trả khách: " +
                            UIComponents::formatVND(change);
          UIComponents::showSuccessDialog(self->shell ? self->shell->getWindow()
                                                      : nullptr,
                                          "Hoàn tất thanh toán", msg);
          self->focusPosSearch();
        } else {
          UIComponents::showErrorDialog(self->shell ? self->shell->getWindow()
                                                    : nullptr,
                                        "Lỗi thanh toán", res.message);
        }
      }),
      this, nullptr, (GConnectFlags)0);

  gtk_box_append(GTK_BOX(payCard), btnSubmitPos);

  gtk_grid_attach(GTK_GRID(grid), payCard, 1, 0, 1, 1);
  gtk_widget_set_vexpand(grid, TRUE);
  gtk_box_append(GTK_BOX(content), grid);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(posView), content);

  // Phím tắt (chỉ hoạt động khi focus nằm trong màn hình POS)
  GtkEventController *keys = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(keys, GTK_PHASE_CAPTURE);
  g_signal_connect(
      keys, "key-pressed",
      G_CALLBACK(+[](GtkEventControllerKey *, guint keyval, guint,
                     GdkModifierType, gpointer d) -> gboolean {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        if (self->shell &&
            (self->shell->isOverlayDialogOpen() || self->shell->hasPages())) {
          return FALSE;
        }
        if (keyval == GDK_KEY_F2) {
          self->showPosProductPickerModal();
          return TRUE;
        }
        if (keyval == GDK_KEY_F9) {
          g_signal_emit_by_name(self->posBtnPay, "clicked");
          return TRUE;
        }
        if (keyval == GDK_KEY_Escape) {
          if (!self->posCartItems.empty()) {
            UIComponents::showConfirmDialog(
                self->shell ? self->shell->getWindow() : nullptr,
                "Xác nhận xóa giỏ hàng",
                "Bạn có chắc chắn muốn xóa toàn bộ sản phẩm trong giỏ hàng POS "
                "hiện tại?",
                +[](gpointer data) {
                  SalesScreens *s = static_cast<SalesScreens *>(data);
                  s->posCartItems.clear();
                  s->posAppliedCoupon = "";
                  s->refreshPos();
                  s->focusPosSearch();
                },
                self);
          }
          return TRUE;
        }
        return FALSE;
      }),
      this);
  gtk_widget_add_controller(posView, keys);
  refreshPosCustomers();
  refreshPos();
}

void SalesScreens::refreshPos() {
  if (!posCartListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(posCartListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(posCartListBox), child);
    child = next;
  }

  int totalUnits = 0;
  for (const auto &item : posCartItems) {
    totalUnits += item.quantity;
  }
  if (posCartCountBadge && GTK_IS_LABEL(posCartCountBadge)) {
    std::string badgeTxt = "Giỏ hàng: " + std::to_string(posCartItems.size()) +
                           " mẫu (" + std::to_string(totalUnits) + " máy)";
    gtk_label_set_text(GTK_LABEL(posCartCountBadge), badgeTxt.c_str());
  }

  if (posCartItems.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "🛒", "Chưa có sản phẩm nào trong giỏ POS",
        "Quét mã, gõ tên sản phẩm ở ô tìm kiếm rồi nhấn Enter, hoặc bấm F2 để "
        "chọn từ danh mục.",
        "➕ Chọn sản phẩm (F2)", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          SalesScreens *self = static_cast<SalesScreens *>(d);
          self->showPosProductPickerModal();
        }),
        this);
    gtk_box_append(GTK_BOX(posCartListBox), empty);
    gtk_label_set_text(GTK_LABEL(posLblSubtotal), "Tạm tính: 0 ₫");
    gtk_label_set_text(GTK_LABEL(posLblDiscount), "Giảm giá: 0 ₫");
    gtk_label_set_text(GTK_LABEL(posLblGrandTotal), "0 ₫");
    gtk_label_set_text(GTK_LABEL(posLblChange), "Tiền thừa trả khách: 0 ₫");
    return;
  }

  double subtotal = 0.0;
  for (size_t i = 0; i < posCartItems.size(); ++i) {
    const auto &item = posCartItems[i];
    subtotal += item.subtotal;

    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Sản phẩm & Mã máy
    std::string subInfo = "[" + item.brand + "] Mã: " + item.phoneId +
                          " • Khả dụng: " + std::to_string(item.availableStock);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     item.phoneName, subInfo, kPosCartCols[0]));

    // Col 1: Đơn giá
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(item.unitPrice),
                                     kPosCartCols[1], "cell-number"));

    // Col 2: Số lượng (Stepper - / qty / +)
    GtkWidget *qtyBox =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_XS);
    gtk_widget_set_valign(qtyBox, GTK_ALIGN_CENTER);

    GtkWidget *btnDec = UIComponents::createTableActionButton(
        "−", "Giảm số lượng", "btn-secondary");
    gtk_widget_set_sensitive(btnDec, item.quantity > 1);

    GtkWidget *qtyLbl = gtk_label_new(std::to_string(item.quantity).c_str());
    gtk_widget_add_css_class(qtyLbl, "cell-number");
    gtk_widget_set_size_request(qtyLbl, 28, -1);
    gtk_label_set_xalign(GTK_LABEL(qtyLbl), 0.5f);

    GtkWidget *btnInc = UIComponents::createTableActionButton(
        "+", "Tăng số lượng", "btn-secondary");
    gtk_widget_set_sensitive(btnInc, item.quantity < item.availableStock);

    struct PosQtyCtx {
      SalesScreens *screens;
      size_t index;
      int delta;
    };
    PosQtyCtx *decCtx = new PosQtyCtx{this, i, -1};
    g_signal_connect_data(
        btnDec, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          PosQtyCtx *c = static_cast<PosQtyCtx *>(d);
          if (c->index < c->screens->posCartItems.size()) {
            auto &it = c->screens->posCartItems[c->index];
            if (it.quantity + c->delta >= 1) {
              it.quantity += c->delta;
              it.calculateSubtotal();
              c->screens->refreshPos();
            }
          }
        }),
        decCtx,
        [](gpointer d, GClosure *) { delete static_cast<PosQtyCtx *>(d); },
        (GConnectFlags)0);

    PosQtyCtx *incCtx = new PosQtyCtx{this, i, 1};
    g_signal_connect_data(
        btnInc, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          PosQtyCtx *c = static_cast<PosQtyCtx *>(d);
          if (c->index < c->screens->posCartItems.size()) {
            auto &it = c->screens->posCartItems[c->index];
            if (it.quantity + c->delta <= it.availableStock) {
              it.quantity += c->delta;
              it.calculateSubtotal();
              c->screens->refreshPos();
            }
          }
        }),
        incCtx,
        [](gpointer d, GClosure *) { delete static_cast<PosQtyCtx *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(qtyBox), btnDec);
    gtk_box_append(GTK_BOX(qtyBox), qtyLbl);
    gtk_box_append(GTK_BOX(qtyBox), btnInc);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     qtyBox, kPosCartCols[2]));

    // Col 3: Thành tiền
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(item.subtotal),
                                     kPosCartCols[3], "product-price"));

    // Col 4: Xóa sản phẩm
    GtkWidget *btnDel = UIComponents::createTableActionButton(
        "✕ Xóa", "Xóa sản phẩm khỏi giỏ hàng", "btn-action-delete");
    gtk_widget_set_hexpand(btnDel, TRUE);

    struct PosDelCtx {
      SalesScreens *screens;
      size_t index;
    };
    PosDelCtx *pCtx = new PosDelCtx{this, i};
    g_signal_connect_data(
        btnDel, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          PosDelCtx *pd = static_cast<PosDelCtx *>(d);
          if (pd->index < pd->screens->posCartItems.size()) {
            pd->screens->posCartItems.erase(pd->screens->posCartItems.begin() +
                                            pd->index);
            pd->screens->refreshPos();
          }
        }),
        pCtx,
        [](gpointer d, GClosure *) { delete static_cast<PosDelCtx *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnDel, kPosCartCols[4]));
    gtk_box_append(GTK_BOX(posCartListBox), row);
  }

  double discount = 0.0;
  if (!posAppliedCoupon.empty()) {
    const std::string custId =
        (posSelectedCustomerId == "WALK-IN") ? "" : posSelectedCustomerId;
    controller->validateDiscountForCustomer(posAppliedCoupon, subtotal, custId,
                                            discount);
  }

  const double vatRate = controller->getVatRate();
  const PricingBreakdown pb =
      PricingCalculator::computeFromDiscountAmount(subtotal, discount, vatRate);

  gtk_label_set_text(
      GTK_LABEL(posLblSubtotal),
      ("Tạm tính: " + UIComponents::formatVND(pb.subtotal)).c_str());
  gtk_label_set_text(
      GTK_LABEL(posLblDiscount),
      ("Giảm giá: -" + UIComponents::formatVND(pb.discountAmount)).c_str());
  if (posLblVat) {
    gtk_label_set_text(
        GTK_LABEL(posLblVat),
        ("Thuế VAT (" + PricingCalculator::formatVatPercent(pb.vatRate) +
         "): " + UIComponents::formatVND(pb.vatAmount))
            .c_str());
  }
  gtk_label_set_text(GTK_LABEL(posLblGrandTotal),
                     UIComponents::formatVND(pb.grandTotal).c_str());

  double cashGiven = UIComponents::parseCurrencyInput(posEntryCash);
  double change = cashGiven - pb.grandTotal;
  if (change < 0 || cashGiven == 0)
    change = 0.0;
  gtk_label_set_text(
      GTK_LABEL(posLblChange),
      ("Tiền thừa trả khách: " + UIComponents::formatVND(change)).c_str());
}

void SalesScreens::refreshPosCustomers() {
  if (!posComboCustomer || !GTK_IS_DROP_DOWN(posComboCustomer))
    return;

  // 1. Lưu lại customer ID đang được chọn
  std::string previouslySelectedId = posSelectedCustomerId;
  if (previouslySelectedId.empty()) {
    guint curIdx = gtk_drop_down_get_selected(GTK_DROP_DOWN(posComboCustomer));
    if (curIdx < posCustomerIds.size()) {
      previouslySelectedId = posCustomerIds[curIdx];
    } else {
      previouslySelectedId = "WALK-IN";
    }
  }

  // 2. Lấy danh sách khách hàng mới nhất từ controller
  std::vector<CustomerItemViewModel> custs = controller->getCustomers();

  // 3. Xây dựng danh sách chuỗi hiển thị và mảng ID song song
  posCustomerIds.clear();
  std::vector<std::string> displayStrings;
  std::vector<const char *> cStrPointers;

  // Phần tử đầu tiên luôn là Khách vãng lai
  posCustomerIds.push_back("WALK-IN");
  displayStrings.push_back("Khách vãng lai (Không tích điểm)");

  for (const auto &c : custs) {
    posCustomerIds.push_back(c.id);
    std::string disp = c.fullName + " (" + c.phone + " - " + c.id + ")";
    displayStrings.push_back(disp);
  }

  for (const auto &s : displayStrings) {
    cStrPointers.push_back(s.c_str());
  }
  cStrPointers.push_back(nullptr);

  // 4. Tạo GtkStringList mới và gán vào DropDown
  GtkStringList *strList = gtk_string_list_new(cStrPointers.data());
  gtk_drop_down_set_model(GTK_DROP_DOWN(posComboCustomer),
                          G_LIST_MODEL(strList));
  g_object_unref(strList); // GtkDropDown nhận quyền sở hữu

  // 5. Khôi phục lựa chọn cũ nếu còn tồn tại
  guint selectIndex = 0;
  for (size_t i = 0; i < posCustomerIds.size(); ++i) {
    if (posCustomerIds[i] == previouslySelectedId) {
      selectIndex = static_cast<guint>(i);
      break;
    }
  }
  gtk_drop_down_set_selected(GTK_DROP_DOWN(posComboCustomer), selectIndex);
  posSelectedCustomerId = posCustomerIds[selectIndex];
}

bool SalesScreens::addProductToPosCart(const ProductItemViewModel &prod,
                                       int quantity) {
  if (quantity <= 0)
    quantity = 1;
  if (prod.availableStock <= 0) {
    UIComponents::showWarningDialog(
        shell ? shell->getWindow() : nullptr, "Hết hàng",
        "Sản phẩm [" + prod.name + "] đã hết hàng khả dụng!");
    return false;
  }

  bool found = false;
  for (auto &item : posCartItems) {
    if (item.phoneId == prod.id) {
      if (item.quantity + quantity <= prod.availableStock) {
        item.quantity += quantity;
        item.calculateSubtotal();
      } else {
        UIComponents::showWarningDialog(
            shell ? shell->getWindow() : nullptr, "Vượt quá tồn kho khả dụng",
            "Sản phẩm " + prod.name + " chỉ còn tối đa " +
                std::to_string(prod.availableStock) + " máy khả dụng (trong giỏ đã có " +
                std::to_string(item.quantity) + " máy).");
        return false;
      }
      found = true;
      break;
    }
  }

  if (!found) {
    if (quantity > prod.availableStock) {
      UIComponents::showWarningDialog(
          shell ? shell->getWindow() : nullptr, "Vượt quá tồn kho khả dụng",
          "Sản phẩm " + prod.name + " chỉ còn tối đa " +
              std::to_string(prod.availableStock) + " máy khả dụng.");
      return false;
    }
    CartItemViewModel ci;
    ci.phoneId = prod.id;
    ci.phoneName = prod.name;
    ci.brand = prod.brand;
    ci.unitPrice = prod.sellingPrice;
    ci.quantity = quantity;
    ci.availableStock = prod.availableStock;
    ci.calculateSubtotal();
    posCartItems.push_back(ci);
  }

  refreshPos();
  return true;
}

void SalesScreens::updatePosSearchSuggestions(const std::string &key) {
  if (!posSearchListBox || !posSearchPopover)
    return;

  // Xóa danh sách gợi ý cũ
  GtkWidget *child = gtk_widget_get_first_child(posSearchListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(posSearchListBox), child);
    child = next;
  }

  std::string trimmed = StringHelper::trim(key);
  if (trimmed.empty()) {
    gtk_popover_popdown(GTK_POPOVER(posSearchPopover));
    return;
  }

  std::string lk = Utf8Helper::toLower(trimmed);
  std::vector<ProductItemViewModel> allProds = controller->getProducts();
  std::vector<ProductItemViewModel> matches;

  // 1. Ưu tiên khớp chính xác Product ID
  for (const auto &p : allProds) {
    if (Utf8Helper::toLower(p.id) == lk) {
      matches.push_back(p);
      break;
    }
  }

  // 2. Tìm theo tên, mã máy, thương hiệu
  if (matches.empty()) {
    for (const auto &p : allProds) {
      if (Utf8Helper::containsCaseInsensitive(p.id, trimmed) ||
          Utf8Helper::containsCaseInsensitive(p.name, trimmed) ||
          Utf8Helper::containsCaseInsensitive(p.brand, trimmed)) {
        matches.push_back(p);
      }
    }
  }

  if (matches.empty()) {
    if (posSearchStatusLbl) {
      gtk_label_set_text(GTK_LABEL(posSearchStatusLbl), "⚠️ Kết quả tìm kiếm:");
    }
    GtkWidget *emptyRow = UIComponents::createEmptyState(
        "🔍", "Không tìm thấy sản phẩm",
        "Không có sản phẩm nào khớp với: \"" + trimmed + "\"");
    gtk_box_append(GTK_BOX(posSearchListBox), emptyRow);
    gtk_popover_present(GTK_POPOVER(posSearchPopover));
    return;
  }

  if (posSearchStatusLbl) {
    std::string statusText =
        "📦 Gợi ý (" + std::to_string(matches.size()) + " sản phẩm khớp):";
    gtk_label_set_text(GTK_LABEL(posSearchStatusLbl), statusText.c_str());
  }

  size_t maxDisplay = (matches.size() > 8) ? 8 : matches.size();
  for (size_t i = 0; i < maxDisplay; ++i) {
    const auto &p = matches[i];
    GtkWidget *rowBtn = gtk_button_new();
    gtk_widget_add_css_class(rowBtn, "table-data-row");

    GtkWidget *rowBox =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
    gtk_widget_set_hexpand(rowBox, TRUE);

    GtkWidget *ico = gtk_label_new("📱");
    gtk_box_append(GTK_BOX(rowBox), ico);

    GtkWidget *textVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_hexpand(textVBox, TRUE);

    GtkWidget *lblName = gtk_label_new(p.name.c_str());
    gtk_widget_add_css_class(lblName, "cell-title");
    gtk_label_set_xalign(GTK_LABEL(lblName), 0.0f);
    gtk_box_append(GTK_BOX(textVBox), lblName);

    std::string sub = "[" + p.brand + "] Mã: " + p.id +
                      " • Giá: " + UIComponents::formatVND(p.sellingPrice) +
                      " • Khả dụng: " + std::to_string(p.availableStock);
    GtkWidget *lblSub = gtk_label_new(sub.c_str());
    gtk_widget_add_css_class(lblSub, "cell-subtext");
    gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
    gtk_box_append(GTK_BOX(textVBox), lblSub);
    gtk_box_append(GTK_BOX(rowBox), textVBox);

    if (p.availableStock <= 0) {
      GtkWidget *badge = gtk_label_new("Hết hàng");
      gtk_widget_add_css_class(badge, "stock-critical");
      gtk_box_append(GTK_BOX(rowBox), badge);
      gtk_widget_set_sensitive(rowBtn, FALSE);
    } else {
      GtkWidget *btnSelect = gtk_label_new("+ Chọn");
      gtk_widget_add_css_class(btnSelect, "btn-primary");
      gtk_box_append(GTK_BOX(rowBox), btnSelect);
    }

    gtk_button_set_child(GTK_BUTTON(rowBtn), rowBox);

    struct SuggestClickCtx {
      SalesScreens *screens;
      ProductItemViewModel product;
    };
    SuggestClickCtx *sctx = new SuggestClickCtx{this, p};
    g_signal_connect_data(
        rowBtn, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          SuggestClickCtx *c = static_cast<SuggestClickCtx *>(d);
          c->screens->addProductToPosCart(c->product);
          gtk_popover_popdown(GTK_POPOVER(c->screens->posSearchPopover));
          gtk_editable_set_text(GTK_EDITABLE(c->screens->posEntrySearch), "");
          gtk_widget_grab_focus(c->screens->posEntrySearch);
        }),
        sctx,
        [](gpointer d, GClosure *) {
          delete static_cast<SuggestClickCtx *>(d);
        },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(posSearchListBox), rowBtn);
  }

  gtk_popover_present(GTK_POPOVER(posSearchPopover));
}

void SalesScreens::showSalesProductDetailModal(
    const ProductItemViewModel &product, bool fromPosPicker) {
  if (!shell)
    return;

  ProductDetailView::Callbacks callbacks;
  callbacks.onBrandSelected = [this](const std::string &brand) {
    productLookupBrandFilter = brand.empty() ? "Tất cả" : brand;
    refreshProductLookup();
    if (shell)
      shell->popPage();
  };
  callbacks.onNavigateHome = [this]() {
    if (shell)
      shell->popPage();
  };
  callbacks.onProductSelected = [this,
                                 fromPosPicker](const std::string &productId) {
    if (controller) {
      ProductItemViewModel nextProd = controller->getProductById(productId);
      showSalesProductDetailModal(nextProd, fromPosPicker);
    }
  };
  callbacks.onAddToPosCart = [this, fromPosPicker](
                                 const ProductItemViewModel &variant, int qty) {
    if (addProductToPosCart(variant, qty)) {
      if (shell) {
        shell->showToast("Đã thêm " + std::to_string(qty) + "x " +
                         variant.name + " vào giỏ POS");
        if (fromPosPicker) {
          shell->clearPageStack();
          focusPosSearch();
        } else {
          shell->popPage();
        }
      }
    }
  };

  GtkWidget *detailView =
      ProductDetailView::create(controller, shell, product,
                                ProductDetailMode::SalesPos, false, callbacks);
  shell->pushPage("sales_product_detail", detailView, product.name);
}

void SalesScreens::showPosProductPickerModal() {
  if (!shell)
    return;

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnClose = gtk_button_new_with_label("← Quay lại quầy POS (Esc)");
  gtk_widget_add_css_class(btnClose, "btn-secondary");
  UIComponents::setButtonCursor(btnClose);
  g_signal_connect_swapped(
      btnClose, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->popPage(); }), shell);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh mục Sản phẩm Điện thoại — Chọn thêm vào giỏ POS",
      "Tìm kiếm theo dòng máy, thương hiệu, lọc khoảng giá và chọn phiên bản "
      "thêm vào giỏ hàng.",
      btnClose);
  gtk_box_append(GTK_BOX(content), header);

  double maxSellingPrice = 0.0;
  if (controller) {
    auto allProds = controller->getProducts();
    for (const auto &p : allProds) {
      if (p.sellingPrice > maxSellingPrice)
        maxSellingPrice = p.sellingPrice;
    }
  }
  if (maxSellingPrice <= 0.0)
    maxSellingPrice = 43990000.0;
  double maxLimit = std::ceil(maxSellingPrice / 100000000.0) * 100000000.0;
  if (maxLimit < 100000000.0)
    maxLimit = 100000000.0;

  struct PickerFilterCtx {
    SalesScreens *screens;
    GtkWidget *listBox;
    GtkWidget *countBadge;
    GtkWidget *brandButtonsBox;
    std::string keyword;
    std::string brandFilter;
    std::string stockFilter;
    std::string sortBy;
    double minPrice;
    double maxPrice;
    void (*populate)(PickerFilterCtx *);
  };

  GtkWidget *pickerListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  PickerFilterCtx *fctx = new PickerFilterCtx{
      this,     pickerListBox, nullptr,    nullptr,
      "",       "Tất cả",      "Tất cả",   "price_asc",
      0.0,      maxLimit,      nullptr};

  auto populatePickerCards = +[](PickerFilterCtx *ctx) {
    if (ctx->brandButtonsBox) {
      GtkWidget *btn = gtk_widget_get_first_child(ctx->brandButtonsBox);
      while (btn != nullptr) {
        if (GTK_IS_BUTTON(btn)) {
          const char *lbl = gtk_button_get_label(GTK_BUTTON(btn));
          bool isActive = (lbl && ctx->brandFilter == lbl);
          gtk_widget_remove_css_class(btn,
                                      isActive ? "btn-outline" : "btn-primary");
          gtk_widget_add_css_class(btn,
                                   isActive ? "btn-primary" : "btn-outline");
        }
        btn = gtk_widget_get_next_sibling(btn);
      }
    }

    GtkWidget *child = gtk_widget_get_first_child(ctx->listBox);
    while (child != nullptr) {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      gtk_box_remove(GTK_BOX(ctx->listBox), child);
      child = next;
    }

    ProductFilterRequest req;
    req.keyword = ctx->keyword;
    req.brand = ctx->brandFilter;
    req.sortBy = ctx->sortBy;

    std::vector<ProductItemViewModel> products =
        ctx->screens->controller->getProducts(req);

    if (!ctx->stockFilter.empty() && ctx->stockFilter != "Tất cả") {
      std::vector<ProductItemViewModel> filtered;
      for (const auto &p : products) {
        if (ctx->stockFilter == "Còn hàng" &&
            p.status == StockDisplayStatus::IN_STOCK) {
          filtered.push_back(p);
        } else if (ctx->stockFilter == "Sắp hết hàng" &&
                   p.status == StockDisplayStatus::LOW_STOCK) {
          filtered.push_back(p);
        } else if (ctx->stockFilter == "Hết hàng" &&
                   p.status == StockDisplayStatus::OUT_OF_STOCK) {
          filtered.push_back(p);
        }
      }
      products = std::move(filtered);
    }

    // Lọc theo khoảng giá
    std::vector<ProductItemViewModel> priceFiltered;
    for (const auto &p : products) {
      if (p.sellingPrice >= ctx->minPrice && p.sellingPrice <= ctx->maxPrice) {
        priceFiltered.push_back(p);
      }
    }
    products = std::move(priceFiltered);

    // Sắp xếp
    if (ctx->sortBy == "price_asc") {
      std::sort(products.begin(), products.end(),
                [](const auto &a, const auto &b) {
                  return a.sellingPrice < b.sellingPrice;
                });
    } else if (ctx->sortBy == "name_asc") {
      std::sort(products.begin(), products.end(),
                [](const auto &a, const auto &b) { return a.name < b.name; });
    } else if (ctx->sortBy == "stock_desc") {
      std::sort(products.begin(), products.end(),
                [](const auto &a, const auto &b) {
                  return a.availableStock > b.availableStock;
                });
    } else {
      std::sort(products.begin(), products.end(),
                [](const auto &a, const auto &b) {
                  return a.sellingPrice < b.sellingPrice;
                });
    }

    if (products.empty()) {
      UIComponents::updateRecordCountBadge(ctx->countBadge, 0, "dòng sản phẩm");
      gtk_box_append(
          GTK_BOX(ctx->listBox),
          UIComponents::createEmptyState(
              "🔍", "Không tìm thấy sản phẩm phù hợp",
              "Thử tìm kiếm với từ khóa khác hoặc xóa bộ lọc thương hiệu/giá."));
      return;
    }

    struct FamilyGroup {
      std::string baseModel;
      std::string brand;
      std::string specs;
      std::string imageUrl;
      ProductItemViewModel representative;
      std::vector<ProductItemViewModel> variants;
      int totalAvailableStock = 0;
      double minPrice = 0.0;
      double maxPrice = 0.0;
    };

    std::vector<FamilyGroup> groups;
    for (const auto &p : products) {
      std::string bKey = p.baseModel.empty() ? p.name : p.baseModel;
      FamilyGroup *target = nullptr;
      for (auto &g : groups) {
        if (g.baseModel == bKey && g.brand == p.brand) {
          target = &g;
          break;
        }
      }
      if (!target) {
        FamilyGroup ng;
        ng.baseModel = bKey;
        ng.brand = p.brand;
        ng.specs = p.specs;
        ng.imageUrl = p.imageUrl;
        ng.representative = p;
        ng.minPrice = p.sellingPrice;
        ng.maxPrice = p.sellingPrice;
        groups.push_back(ng);
        target = &groups.back();
      }
      if (target->imageUrl.empty() && !p.imageUrl.empty()) {
        target->imageUrl = p.imageUrl;
      }
      target->variants.push_back(p);
      target->totalAvailableStock += p.availableStock;
      if (p.sellingPrice < target->minPrice) {
        target->minPrice = p.sellingPrice;
        target->representative = p;
      }
      if (p.sellingPrice > target->maxPrice) {
        target->maxPrice = p.sellingPrice;
      }
    }

    UIComponents::updateRecordCountBadge(
        ctx->countBadge, groups.size(),
        "dòng máy (" + std::to_string(products.size()) + " phiên bản)");

    for (const auto &g : groups) {
      const auto &p = g.representative;
      GtkWidget *card =
          gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_LG);
      gtk_widget_add_css_class(card, "product-card");

      GtkWidget *imgThumb = UIComponents::createCachedThumbnail(g.imageUrl, 56);
      gtk_box_append(GTK_BOX(card), imgThumb);

      GtkWidget *infoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
      gtk_widget_set_hexpand(infoBox, TRUE);

      GtkWidget *topRow =
          gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
      GtkWidget *lblName = gtk_label_new(g.baseModel.c_str());
      gtk_widget_add_css_class(lblName, "product-name");
      gtk_label_set_xalign(GTK_LABEL(lblName), 0.0f);

      StockDisplayStatus grpStatus =
          (g.totalAvailableStock <= 0)
              ? StockDisplayStatus::OUT_OF_STOCK
              : (g.totalAvailableStock <= 5 ? StockDisplayStatus::LOW_STOCK
                                            : StockDisplayStatus::IN_STOCK);

      GtkWidget *badgeBrand =
          UIComponents::createStatusBadge(g.brand, "role-pill-sales");
      GtkWidget *badgeStock = UIComponents::createStockStatusBadge(grpStatus);

      gtk_box_append(GTK_BOX(topRow), lblName);
      gtk_box_append(GTK_BOX(topRow), badgeBrand);
      gtk_box_append(GTK_BOX(topRow), badgeStock);
      gtk_box_append(GTK_BOX(infoBox), topRow);

      std::vector<std::string> caps, cols;
      for (const auto &v : g.variants) {
        if (!v.capacity.empty() &&
            std::find(caps.begin(), caps.end(), v.capacity) == caps.end())
          caps.push_back(v.capacity);
        if (!v.color.empty() &&
            std::find(cols.begin(), cols.end(), v.color) == cols.end())
          cols.push_back(v.color);
      }
      std::string capStr, colStr;
      for (size_t c = 0; c < caps.size(); ++c) {
        if (c > 0)
          capStr += " / ";
        capStr += caps[c];
      }
      for (size_t c = 0; c < cols.size(); ++c) {
        if (c > 0)
          colStr += ", ";
        colStr += cols[c];
      }

      std::string variantMeta =
          (capStr.empty() ? "" : ("Dung lượng: " + capStr)) +
          (colStr.empty() ? "" : (" • Màu: " + colStr)) +
          (g.specs.empty() ? "" : (" • " + g.specs));

      GtkWidget *lblSpecs = gtk_label_new(variantMeta.c_str());
      gtk_widget_add_css_class(lblSpecs, "page-subtitle");
      gtk_label_set_xalign(GTK_LABEL(lblSpecs), 0.0f);
      gtk_label_set_wrap(GTK_LABEL(lblSpecs), TRUE);
      gtk_box_append(GTK_BOX(infoBox), lblSpecs);

      const char *availClass = (g.totalAvailableStock <= 0)  ? "stock-critical"
                               : (g.totalAvailableStock <= 5) ? "stock-low"
                                                              : "stock-ok";
      GtkWidget *lblAvail =
          gtk_label_new(("Tổng số lượng có thể bán: " +
                         UIComponents::formatNumber(g.totalAvailableStock) +
                         " máy (" + std::to_string(g.variants.size()) +
                         " tổ hợp)")
                            .c_str());
      gtk_widget_add_css_class(lblAvail, availClass);
      gtk_widget_add_css_class(lblAvail, "tabular-num");
      gtk_label_set_xalign(GTK_LABEL(lblAvail), 0.0f);
      gtk_box_append(GTK_BOX(infoBox), lblAvail);
      gtk_box_append(GTK_BOX(card), infoBox);

      GtkWidget *actionBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
      gtk_widget_set_valign(actionBox, GTK_ALIGN_CENTER);

      std::string priceDisp =
          (std::abs(g.minPrice - g.maxPrice) < 1.0)
              ? UIComponents::formatVND(g.minPrice)
              : (UIComponents::formatVND(g.minPrice) + " – " +
                 UIComponents::formatCompactVND(g.maxPrice));
      GtkWidget *lblPrice = gtk_label_new(priceDisp.c_str());
      gtk_widget_add_css_class(lblPrice, "product-price");
      gtk_widget_add_css_class(lblPrice, "tabular-num");
      gtk_label_set_xalign(GTK_LABEL(lblPrice), 1.0f);
      gtk_box_append(GTK_BOX(actionBox), lblPrice);

      // Riêng SalesScreen: Thu gọn thành 1 nút thêm vào giỏ hàng POS
      GtkWidget *btnBuy = gtk_button_new_with_label("🛍️ Thêm vào giỏ");
      gtk_widget_add_css_class(btnBuy, "btn-primary");
      UIComponents::setButtonCursor(btnBuy);

      struct OpenDetailCtx {
        SalesScreens *screens;
        std::string phoneId;
      };
      OpenDetailCtx *od = new OpenDetailCtx{ctx->screens, p.id};
      g_signal_connect_data(
          btnBuy, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
            OpenDetailCtx *c = static_cast<OpenDetailCtx *>(d);
            ProductItemViewModel prod =
                c->screens->controller->getProductById(c->phoneId);
            c->screens->showSalesProductDetailModal(prod, true);
          }),
          od,
          [](gpointer d, GClosure *) { delete static_cast<OpenDetailCtx *>(d); },
          (GConnectFlags)0);

      gtk_box_append(GTK_BOX(actionBox), btnBuy);
      gtk_box_append(GTK_BOX(card), actionBox);
      gtk_box_append(GTK_BOX(ctx->listBox), card);
    }
  };
  fctx->populate = populatePickerCards;

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  // Bộ lọc Tra cứu sản phẩm hỗ trợ chuẩn:
  // "Tất cả thương hiệu", "Tất cả tồn kho", "Còn hàng (>5)"
  // "Giá: Thấp → Cao", "Giá: Cao → Thấp", "Tồn kho: Nhiều → Ít"
  // Logic lọc theo hãng Apple/Samsung/Xiaomi/Oppo: (productLookupBrand == 1)
  // Logic lọc theo tồn kho khả dụng: (productLookupStock == 1)

  const char *stockOptions[] = {"Tất cả tình trạng", "Còn hàng", "Sắp hết hàng",
                                "Hết hàng", nullptr};
  GtkWidget *dropStock = gtk_drop_down_new_from_strings(stockOptions);
  UIComponents::applyFilterDropdownBadges(
      dropStock, {"", "badge-in-stock", "badge-low-stock", "badge-out-of-stock"});
  auto onPickerStockChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    PickerFilterCtx *ctx = static_cast<PickerFilterCtx *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Tất cả", "Còn hàng", "Sắp hết hàng", "Hết hàng"};
    if (idx < 4) {
      ctx->stockFilter = opts[idx];
      ctx->populate(ctx);
    }
  };
  g_signal_connect(dropStock, "notify::selected",
                   G_CALLBACK(onPickerStockChanged), fctx);
  gtk_box_append(GTK_BOX(extraControls), dropStock);

  const char *sortOptions[] = {"Tự động (Giá thấp → cao)", "Tên: A-Z",
                               "Tồn kho: Nhiều nhất", nullptr};
  GtkWidget *dropSort = gtk_drop_down_new_from_strings(sortOptions);
  auto onPickerSortChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    PickerFilterCtx *ctx = static_cast<PickerFilterCtx *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"price_asc", "name_asc", "stock_desc"};
    if (idx < 3) {
      ctx->sortBy = opts[idx];
      ctx->populate(ctx);
    }
  };
  g_signal_connect(dropSort, "notify::selected",
                   G_CALLBACK(onPickerSortChanged), fctx);
  gtk_box_append(GTK_BOX(extraControls), dropSort);

  GtkWidget *pickerCountBadge = nullptr;
  GtkWidget *searchEntry = nullptr;
  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên điện thoại, thương hiệu hoặc mã...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        PickerFilterCtx *ctx = static_cast<PickerFilterCtx *>(d);
        const char *txt = gtk_editable_get_text(e);
        ctx->keyword = txt ? txt : "";
        ctx->populate(ctx);
      }),
      fctx, &searchEntry, &pickerCountBadge, extraControls);
  fctx->countBadge = pickerCountBadge;
  gtk_box_append(GTK_BOX(content), filterToolbar);

  // Thanh trượt 2 đầu lọc khoảng giá (dùng component mới chuẩn thiết kế)
  GtkWidget *sliderCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_add_css_class(sliderCard, "card");
  gtk_widget_add_css_class(sliderCard, "dual-range-card");

  GtkWidget *lblSliderTitle = gtk_label_new("💰 Khoảng giá lọc:");
  gtk_widget_add_css_class(lblSliderTitle, "form-label");
  gtk_label_set_xalign(GTK_LABEL(lblSliderTitle), 0.0f);
  gtk_box_append(GTK_BOX(sliderCard), lblSliderTitle);

  GtkWidget *dualSlider = UIComponents::createDualRangeSlider(
      0.0, maxLimit, fctx->minPrice, fctx->maxPrice, 100000.0,
      [fctx](double minVal, double maxVal) {
        fctx->minPrice = minVal;
        fctx->maxPrice = maxVal;
        fctx->populate(fctx);
      });
  gtk_box_append(GTK_BOX(sliderCard), dualSlider);
  gtk_box_append(GTK_BOX(content), sliderCard);

  // Thanh nhãn hãng giống CustomerScreens
  GtkWidget *brandButtonsBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  fctx->brandButtonsBox = brandButtonsBox;
  const char *brands[] = {"Tất cả", "Apple", "Samsung", "Oppo", "Xiaomi"};
  for (const char *b : brands) {
    GtkWidget *btnB = gtk_button_new_with_label(b);
    gtk_widget_add_css_class(
        btnB, fctx->brandFilter == b ? "btn-primary" : "btn-outline");
    UIComponents::setButtonCursor(btnB);
    g_signal_connect_data(
        btnB, "clicked", G_CALLBACK(+[](GtkButton *btn, gpointer data) {
          PickerFilterCtx *ctx = static_cast<PickerFilterCtx *>(data);
          ctx->brandFilter = gtk_button_get_label(btn);
          ctx->populate(ctx);
        }),
        fctx, nullptr, (GConnectFlags)0);
    gtk_box_append(GTK_BOX(brandButtonsBox), btnB);
  }
  gtk_box_append(GTK_BOX(content), brandButtonsBox);

  fctx->populate(fctx);

  gtk_box_append(GTK_BOX(content), pickerListBox);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), content);

  UIComponents::setupDialogShortcuts(scroll, nullptr, btnClose);
  shell->pushPage("sales_pos_picker", scroll, "Chọn sản phẩm vào hóa đơn POS");
  if (searchEntry) {
    gtk_widget_grab_focus(searchEntry);
  }
}

// ================= 3. TRA CỨU SẢN PHẨM =================

void SalesScreens::buildProductLookupView() {
  productLookupView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productLookupView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Tra cứu sản phẩm & Giá bán",
      "Tra cứu nhanh thông số kỹ thuật, giá niêm yết và tồn kho khả dụng để tư "
      "vấn khách hàng.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  // Bộ lọc Tra cứu sản phẩm hỗ trợ chuẩn:
  // "Tất cả thương hiệu", "Tất cả tồn kho", "Còn hàng (>5)"
  // "Giá: Thấp → Cao", "Giá: Cao → Thấp", "Tồn kho: Nhiều → Ít"
  // Logic lọc theo hãng Apple/Samsung/Xiaomi/Oppo: (productLookupBrand == 1)
  // Logic lọc theo tồn kho khả dụng: (productLookupStock == 1)

  const char *stockOptions[] = {"Tất cả tình trạng", "Còn hàng", "Sắp hết hàng",
                                "Hết hàng", nullptr};
  GtkWidget *dropStock = gtk_drop_down_new_from_strings(stockOptions);
  UIComponents::applyFilterDropdownBadges(
      dropStock, {"", "badge-in-stock", "badge-low-stock", "badge-out-of-stock"});
  auto onStockChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    SalesScreens *self = static_cast<SalesScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Tất cả", "Còn hàng", "Sắp hết hàng", "Hết hàng"};
    if (idx < 4) {
      self->productLookupStockFilter = opts[idx];
      self->refreshProductLookup();
    }
  };
  g_signal_connect(dropStock, "notify::selected", G_CALLBACK(onStockChanged),
                   this);
  gtk_box_append(GTK_BOX(extraControls), dropStock);

  const char *sortOptions[] = {"Tự động (Giá thấp → cao)", "Tên: A-Z",
                               "Tồn kho: Nhiều nhất", nullptr};
  GtkWidget *dropSort = gtk_drop_down_new_from_strings(sortOptions);
  auto onSortChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    SalesScreens *self = static_cast<SalesScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"price_asc", "name_asc", "stock_desc"};
    if (idx < 3) {
      self->productLookupSortBy = opts[idx];
      self->refreshProductLookup();
    }
  };
  g_signal_connect(dropSort, "notify::selected", G_CALLBACK(onSortChanged),
                   this);
  gtk_box_append(GTK_BOX(extraControls), dropSort);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên, hãng hoặc mã sản phẩm...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->productLookupKeyword = txt ? txt : "";
        self->refreshProductLookup();
      }),
      this, nullptr, &lookupCountBadge, extraControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  double maxSellingPrice = 0.0;
  if (controller) {
    auto allProds = controller->getProducts();
    for (const auto &p : allProds) {
      if (p.sellingPrice > maxSellingPrice)
        maxSellingPrice = p.sellingPrice;
    }
  }
  if (maxSellingPrice <= 0.0)
    maxSellingPrice = 43990000.0;
  lookupMaxPriceLimit = std::ceil(maxSellingPrice / 100000000.0) * 100000000.0;
  if (lookupMaxPriceLimit < 100000000.0)
    lookupMaxPriceLimit = 100000000.0;
  lookupFilterMinPrice = 0.0;
  lookupFilterMaxPrice = lookupMaxPriceLimit;

  // Thanh trượt 2 đầu lọc khoảng giá
  GtkWidget *sliderCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_add_css_class(sliderCard, "card");
  gtk_widget_add_css_class(sliderCard, "dual-range-card");

  GtkWidget *lblSliderTitle = gtk_label_new("💰 Khoảng giá lọc:");
  gtk_widget_add_css_class(lblSliderTitle, "form-label");
  gtk_label_set_xalign(GTK_LABEL(lblSliderTitle), 0.0f);
  gtk_box_append(GTK_BOX(sliderCard), lblSliderTitle);

  GtkWidget *dualSlider = UIComponents::createDualRangeSlider(
      0.0, lookupMaxPriceLimit, lookupFilterMinPrice, lookupFilterMaxPrice, 100000.0,
      [this](double minVal, double maxVal) {
        this->lookupFilterMinPrice = minVal;
        this->lookupFilterMaxPrice = maxVal;
        this->refreshProductLookup();
      });
  gtk_box_append(GTK_BOX(sliderCard), dualSlider);
  gtk_box_append(GTK_BOX(content), sliderCard);

  // Thanh nhãn hãng giống CustomerScreens
  lookupBrandButtonsBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  const char *brands[] = {"Tất cả", "Apple", "Samsung", "Oppo", "Xiaomi"};
  for (const char *b : brands) {
    GtkWidget *btnB = gtk_button_new_with_label(b);
    gtk_widget_add_css_class(
        btnB, productLookupBrandFilter == b ? "btn-primary" : "btn-outline");
    UIComponents::setButtonCursor(btnB);
    g_signal_connect_data(
        btnB, "clicked", G_CALLBACK(+[](GtkButton *btn, gpointer data) {
          SalesScreens *self = static_cast<SalesScreens *>(data);
          self->productLookupBrandFilter = gtk_button_get_label(btn);
          self->refreshProductLookup();
        }),
        this, nullptr, (GConnectFlags)0);
    gtk_box_append(GTK_BOX(lookupBrandButtonsBox), btnB);
  }
  gtk_box_append(GTK_BOX(content), lookupBrandButtonsBox);

  productLookupListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_box_append(GTK_BOX(content), productLookupListBox);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(productLookupView),
                                content);
  refreshProductLookup();
}

void SalesScreens::refreshProductLookup(const std::string &keyword) {
  if (!productLookupListBox)
    return;
  if (!keyword.empty()) {
    productLookupKeyword = keyword;
  }

  if (lookupBrandButtonsBox) {
    GtkWidget *btn = gtk_widget_get_first_child(lookupBrandButtonsBox);
    while (btn != nullptr) {
      if (GTK_IS_BUTTON(btn)) {
        const char *lbl = gtk_button_get_label(GTK_BUTTON(btn));
        bool isActive = (lbl && productLookupBrandFilter == lbl);
        gtk_widget_remove_css_class(btn,
                                    isActive ? "btn-outline" : "btn-primary");
        gtk_widget_add_css_class(btn,
                                 isActive ? "btn-primary" : "btn-outline");
      }
      btn = gtk_widget_get_next_sibling(btn);
    }
  }

  GtkWidget *child = gtk_widget_get_first_child(productLookupListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(productLookupListBox), child);
    child = next;
  }

  ProductFilterRequest req;
  req.keyword = productLookupKeyword;
  req.brand = productLookupBrandFilter;
  req.sortBy = productLookupSortBy;

  std::vector<ProductItemViewModel> products = controller->getProducts(req);

  std::vector<ProductItemViewModel> priceFiltered;
  for (const auto &p : products) {
    if (p.sellingPrice >= lookupFilterMinPrice &&
        p.sellingPrice <= lookupFilterMaxPrice) {
      priceFiltered.push_back(p);
    }
  }
  products = std::move(priceFiltered);

  if (productLookupSortBy == "price_asc") {
    std::sort(products.begin(), products.end(),
              [](const auto &a, const auto &b) {
                return a.sellingPrice < b.sellingPrice;
              });
  } else if (productLookupSortBy == "name_asc") {
    std::sort(products.begin(), products.end(),
              [](const auto &a, const auto &b) { return a.name < b.name; });
  } else if (productLookupSortBy == "stock_desc") {
    std::sort(products.begin(), products.end(),
              [](const auto &a, const auto &b) {
                return a.availableStock > b.availableStock;
              });
  } else {
    std::sort(products.begin(), products.end(),
              [](const auto &a, const auto &b) {
                return a.sellingPrice < b.sellingPrice;
              });
  }

  if (products.empty()) {
    UIComponents::updateRecordCountBadge(lookupCountBadge, 0, "dòng máy");
    GtkWidget *empty = UIComponents::createEmptyState(
        "🔍", "Không tìm thấy sản phẩm",
        "Thử thay đổi từ khóa hoặc xóa bớt tiêu chí lọc.");
    gtk_box_append(GTK_BOX(productLookupListBox), empty);
    return;
  }

  struct FamilyGroup {
    std::string baseModel;
    std::string brand;
    std::string specs;
    std::string imageUrl;
    ProductItemViewModel representative;
    std::vector<ProductItemViewModel> variants;
    int totalAvailableStock = 0;
    double minPrice = 0.0;
    double maxPrice = 0.0;
  };

  std::vector<FamilyGroup> groups;
  for (const auto &p : products) {
    std::string bKey = p.baseModel.empty() ? p.name : p.baseModel;
    FamilyGroup *target = nullptr;
    for (auto &g : groups) {
      if (g.baseModel == bKey && g.brand == p.brand) {
        target = &g;
        break;
      }
    }
    if (!target) {
      FamilyGroup ng;
      ng.baseModel = bKey;
      ng.brand = p.brand;
      ng.specs = p.specs;
      ng.imageUrl = p.imageUrl;
      ng.representative = p;
      ng.minPrice = p.sellingPrice;
      ng.maxPrice = p.sellingPrice;
      groups.push_back(ng);
      target = &groups.back();
    }
    if (target->imageUrl.empty() && !p.imageUrl.empty()) {
      target->imageUrl = p.imageUrl;
    }
    target->variants.push_back(p);
    target->totalAvailableStock += p.availableStock;
    if (p.sellingPrice < target->minPrice) {
      target->minPrice = p.sellingPrice;
      target->representative = p;
    }
    if (p.sellingPrice > target->maxPrice) {
      target->maxPrice = p.sellingPrice;
    }
  }

  if (!productLookupStockFilter.empty() &&
      productLookupStockFilter != "Tất cả") {
    groups.erase(
        std::remove_if(groups.begin(), groups.end(), [&](const auto &g) {
          StockDisplayStatus status =
              (g.totalAvailableStock <= 0)
                  ? StockDisplayStatus::OUT_OF_STOCK
                  : (g.totalAvailableStock <= 5
                         ? StockDisplayStatus::LOW_STOCK
                         : StockDisplayStatus::IN_STOCK);
          return (productLookupStockFilter == "Còn hàng" &&
                  status != StockDisplayStatus::IN_STOCK) ||
                 (productLookupStockFilter == "Sắp hết hàng" &&
                  status != StockDisplayStatus::LOW_STOCK) ||
                 (productLookupStockFilter == "Hết hàng" &&
                  status != StockDisplayStatus::OUT_OF_STOCK);
        }),
        groups.end());
  }

  if (groups.empty()) {
    UIComponents::updateRecordCountBadge(lookupCountBadge, 0, "dòng máy");
    GtkWidget *empty = UIComponents::createEmptyState(
        "🔍", "Không tìm thấy sản phẩm",
        "Thử thay đổi từ khóa hoặc xóa bớt tiêu chí lọc.");
    gtk_box_append(GTK_BOX(productLookupListBox), empty);
    return;
  }

  size_t visibleVariantCount = 0;
  for (const auto &g : groups) {
    visibleVariantCount += g.variants.size();
  }

  UIComponents::updateRecordCountBadge(
      lookupCountBadge, groups.size(),
      "dòng máy (" + std::to_string(visibleVariantCount) + " tổ hợp)");

  for (const auto &g : groups) {
    const auto &p = g.representative;
    GtkWidget *card =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(card, "product-card");

    GtkWidget *imgThumb = UIComponents::createCachedThumbnail(g.imageUrl, 56);
    gtk_box_append(GTK_BOX(card), imgThumb);

    GtkWidget *infoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(infoBox, TRUE);

    GtkWidget *topRow =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
    GtkWidget *lblName = gtk_label_new(g.baseModel.c_str());
    gtk_widget_add_css_class(lblName, "product-name");
    gtk_label_set_xalign(GTK_LABEL(lblName), 0.0f);

    StockDisplayStatus grpStatus =
        (g.totalAvailableStock <= 0)
            ? StockDisplayStatus::OUT_OF_STOCK
            : (g.totalAvailableStock <= 5 ? StockDisplayStatus::LOW_STOCK
                                          : StockDisplayStatus::IN_STOCK);

    GtkWidget *badgeBrand =
        UIComponents::createStatusBadge(g.brand, "role-pill-sales");
    GtkWidget *badgeStock = UIComponents::createStockStatusBadge(grpStatus);

    gtk_box_append(GTK_BOX(topRow), lblName);
    gtk_box_append(GTK_BOX(topRow), badgeBrand);
    gtk_box_append(GTK_BOX(topRow), badgeStock);
    gtk_box_append(GTK_BOX(infoBox), topRow);

    std::vector<std::string> caps, cols;
    for (const auto &v : g.variants) {
      if (!v.capacity.empty() &&
          std::find(caps.begin(), caps.end(), v.capacity) == caps.end())
        caps.push_back(v.capacity);
      if (!v.color.empty() &&
          std::find(cols.begin(), cols.end(), v.color) == cols.end())
        cols.push_back(v.color);
    }
    std::string capStr, colStr;
    for (size_t c = 0; c < caps.size(); ++c) {
      if (c > 0)
        capStr += " / ";
      capStr += caps[c];
    }
    for (size_t c = 0; c < cols.size(); ++c) {
      if (c > 0)
        colStr += ", ";
      colStr += cols[c];
    }

    std::string variantMeta =
        (capStr.empty() ? "" : ("Dung lượng: " + capStr)) +
        (colStr.empty() ? "" : (" • Màu: " + colStr)) +
        (g.specs.empty() ? "" : (" • " + g.specs));

    GtkWidget *lblSpecs = gtk_label_new(variantMeta.c_str());
    gtk_widget_add_css_class(lblSpecs, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblSpecs), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(lblSpecs), TRUE);
    gtk_box_append(GTK_BOX(infoBox), lblSpecs);

    const char *availClass = (g.totalAvailableStock <= 0)  ? "stock-critical"
                             : (g.totalAvailableStock <= 5) ? "stock-low"
                                                            : "stock-ok";
    GtkWidget *lblAvail =
        gtk_label_new(("Tổng số lượng có thể bán: " +
                       UIComponents::formatNumber(g.totalAvailableStock) +
                       " máy (" + std::to_string(g.variants.size()) +
                       " tổ hợp)")
                          .c_str());
    gtk_widget_add_css_class(lblAvail, availClass);
    gtk_widget_add_css_class(lblAvail, "tabular-num");
    gtk_label_set_xalign(GTK_LABEL(lblAvail), 0.0f);
    gtk_box_append(GTK_BOX(infoBox), lblAvail);
    gtk_box_append(GTK_BOX(card), infoBox);

    GtkWidget *actionBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_valign(actionBox, GTK_ALIGN_CENTER);

    std::string priceDisp =
        (std::abs(g.minPrice - g.maxPrice) < 1.0)
            ? UIComponents::formatVND(g.minPrice)
            : (UIComponents::formatVND(g.minPrice) + " – " +
               UIComponents::formatCompactVND(g.maxPrice));
    GtkWidget *lblPrice = gtk_label_new(priceDisp.c_str());
    gtk_widget_add_css_class(lblPrice, "product-price");
    gtk_widget_add_css_class(lblPrice, "tabular-num");
    gtk_label_set_xalign(GTK_LABEL(lblPrice), 1.0f);
    gtk_box_append(GTK_BOX(actionBox), lblPrice);

    GtkWidget *btnBuy = gtk_button_new_with_label("🛍️ Mua hàng");
    gtk_widget_add_css_class(btnBuy, "btn-primary");
    UIComponents::setButtonCursor(btnBuy);

    struct LookupOpenDetailCtx {
      SalesScreens *screens;
      std::string phoneId;
    };
    LookupOpenDetailCtx *lod = new LookupOpenDetailCtx{this, p.id};
    g_signal_connect_data(
        btnBuy, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          LookupOpenDetailCtx *c = static_cast<LookupOpenDetailCtx *>(d);
          ProductItemViewModel prod =
              c->screens->controller->getProductById(c->phoneId);
          c->screens->showSalesProductDetailModal(prod, false);
        }),
        lod,
        [](gpointer d, GClosure *) {
          delete static_cast<LookupOpenDetailCtx *>(d);
        },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(actionBox), btnBuy);
    gtk_box_append(GTK_BOX(card), actionBox);
    gtk_box_append(GTK_BOX(productLookupListBox), card);
  }
}

// ================= 4. QUẢN LÝ KHÁCH HÀNG =================

void SalesScreens::buildCustomerMgmtView() {
  customerMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(customerMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *actionBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  GtkWidget *btnExport = UIComponents::createExportButton(
      "📥 Xuất CSV", "Xuất danh sách khách hàng ra file .csv", [this]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName =
            "DanhSach_KhachHang_" +
            ExportHelper::makeSafeDateToken(controller->getSimulatedDate()) +
            ".csv";
        UIComponents::showSaveFileDialog(
            win, "Xuất danh sách khách hàng (.csv)", defName, "Tệp CSV (*.csv)",
            "*.csv", [this, win](const std::string &path) {
              OperationResult res =
                  controller->exportCustomersCsv(path, customerFilterKeyword);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(win, "Xuất CSV thành công",
                                                res.message);
              } else {
                UIComponents::showErrorDialog(win, "Lỗi xuất file",
                                              res.message);
              }
            });
      });
  gtk_box_append(GTK_BOX(actionBox), btnExport);

  GtkWidget *btnNew = gtk_button_new_with_label("➕ Thêm khách hàng mới");
  gtk_widget_add_css_class(btnNew, "btn-primary");
  UIComponents::setButtonCursor(btnNew);
  g_signal_connect_swapped(btnNew, "clicked", G_CALLBACK(+[](SalesScreens *s) {
                             s->showCustomerModal(nullptr);
                           }),
                           this);
  gtk_box_append(GTK_BOX(actionBox), btnNew);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý thông tin khách hàng",
      "Tìm kiếm, thêm mới và theo dõi lịch sử mua hàng, điểm tích lũy.",
      actionBox);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  const char *tiers[] = {"Tất cả các hạng", "Đồng", "Bạc", "Vàng",
                         "Kim Cương (VIP)", nullptr};
  GtkWidget *comboTier = gtk_drop_down_new_from_strings(tiers);
  gtk_widget_add_css_class(comboTier, "filter-dropdown");
  UIComponents::applyFilterDropdownBadges(
      comboTier,
      {"", "rank-bronze", "rank-silver", "rank-gold", "rank-diamond"});
  g_signal_connect_data(comboTier, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          self->customerFilterTier =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshCustomers();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(extraControls), comboTier);

  const char *sortOptions[] = {"Mặc định", "Điểm: Cao → Thấp",
                               "Điểm: Thấp → Cao", "Tên: A → Z", nullptr};
  GtkWidget *comboSort = gtk_drop_down_new_from_strings(sortOptions);
  gtk_widget_add_css_class(comboSort, "filter-dropdown");
  g_signal_connect_data(comboSort, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          self->customerSortBy =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshCustomers();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(extraControls), comboSort);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên, SĐT hoặc mã khách hàng...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->customerFilterKeyword = txt ? txt : "";
        self->refreshCustomers();
      }),
      this, nullptr, &customerCountBadge, extraControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kSalesCustCols));

  customerListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), customerListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(customerMgmtView), content);
  refreshCustomers();
}

void SalesScreens::refreshCustomers(const std::string &keyword) {
  if (!customerListBox)
    return;
  if (!keyword.empty()) {
    customerFilterKeyword = keyword;
  }
  GtkWidget *child = gtk_widget_get_first_child(customerListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(customerListBox), child);
    child = next;
  }

  std::vector<CustomerItemViewModel> custs =
      controller->getCustomers(customerFilterKeyword);

  // Lọc theo hạng thành viên
  if (customerFilterTier > 0) {
    std::vector<CustomerItemViewModel> filtered;
    for (const auto &c : custs) {
      if (customerFilterTier == 1 &&
          c.membershipTier.find("Đồng") != std::string::npos) {
        filtered.push_back(c);
      } else if (customerFilterTier == 2 &&
                 c.membershipTier.find("Bạc") != std::string::npos) {
        filtered.push_back(c);
      } else if (customerFilterTier == 3 &&
                 c.membershipTier.find("Vàng") != std::string::npos) {
        filtered.push_back(c);
      } else if (customerFilterTier == 4 &&
                 (c.membershipTier.find("Kim Cương") != std::string::npos ||
                  c.membershipTier.find("VIP") != std::string::npos)) {
        filtered.push_back(c);
      }
    }
    custs = std::move(filtered);
  }

  // Sắp xếp
  if (customerSortBy == 1) { // Điểm: Cao -> Thấp
    std::sort(
        custs.begin(), custs.end(),
        [](const CustomerItemViewModel &a, const CustomerItemViewModel &b) {
          return a.points > b.points;
        });
  } else if (customerSortBy == 2) { // Điểm: Thấp -> Cao
    std::sort(
        custs.begin(), custs.end(),
        [](const CustomerItemViewModel &a, const CustomerItemViewModel &b) {
          return a.points < b.points;
        });
  } else if (customerSortBy == 3) { // Tên: A -> Z
    std::sort(
        custs.begin(), custs.end(),
        [](const CustomerItemViewModel &a, const CustomerItemViewModel &b) {
          return a.fullName < b.fullName;
        });
  }

  UIComponents::updateRecordCountBadge(customerCountBadge, custs.size(),
                                       "khách hàng");

  if (custs.empty()) {
    std::string emptyMsg =
        customerFilterKeyword.empty()
            ? "Chưa có thông tin khách hàng nào trong hệ thống."
            : "Không tìm thấy khách hàng nào khớp với từ khóa tìm kiếm.";
    GtkWidget *empty = UIComponents::createEmptyState(
        "👥", "Không tìm thấy khách hàng", emptyMsg,
        "➕ Đăng ký khách hàng mới", G_CALLBACK(+[](GtkButton *, gpointer d) {
          SalesScreens *s = static_cast<SalesScreens *>(d);
          s->showCustomerModal(nullptr);
        }),
        this);
    gtk_box_append(GTK_BOX(customerListBox), empty);
    return;
  }

  for (size_t i = 0; i < custs.size(); ++i) {
    const auto &c = custs[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã KH
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     c.id, kSalesCustCols[0], "cell-code"));

    // Col 1: Họ và tên
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       c.fullName, kSalesCustCols[1], "cell-title"));

    // Col 2: Liên hệ (SĐT / Email / Địa chỉ)
    std::string line1 =
        "SĐT: " + c.phone + (c.email.empty() ? "" : (" • " + c.email));
    std::string line2 = c.address.empty() ? "" : ("ĐC: " + c.address);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     line1, line2, kSalesCustCols[2]));

    // Col 3: Hạng TV badge
    GtkWidget *badgeTier = UIComponents::createRankBadge(c.membershipTier);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badgeTier, kSalesCustCols[3]));

    // Col 4: Điểm tích lũy (căn phải, chữ số đẳng rộng)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(c.points) + " điểm",
                       kSalesCustCols[4], "cell-number"));

    // Col 5: Nút Sửa
    GtkWidget *btnEdit = UIComponents::createTableActionButton(
        "✏️ Sửa", "Chỉnh sửa thông tin khách hàng", "btn-action-edit");
    gtk_widget_set_hexpand(btnEdit, TRUE);

    struct CustCtx {
      SalesScreens *screens;
      CustomerItemViewModel item;
    };
    CustCtx *eCtx = new CustCtx{this, c};
    g_signal_connect_data(
        btnEdit, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CustCtx *cx = static_cast<CustCtx *>(d);
          cx->screens->showCustomerModal(&cx->item);
        }),
        eCtx, [](gpointer d, GClosure *) { delete static_cast<CustCtx *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnEdit, kSalesCustCols[5]));
    gtk_box_append(GTK_BOX(customerListBox), row);
  }
}

void SalesScreens::showCustomerModal(
    const CustomerItemViewModel *existingCust) {
  if (!shell)
    return;

  bool isEdit = (existingCust != nullptr);

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *pageContent =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(pageContent, "page-container");

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "pdp-card");
  gtk_widget_set_hexpand(card, TRUE);

  GtkWidget *title = gtk_label_new(isEdit ? "✏️ Cập nhật thông tin khách hàng"
                                          : "➕ Đăng ký khách hàng mới");
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  GtkWidget *subtitle = gtk_label_new(
      "Điền đầy đủ họ tên và số điện thoại hợp lệ để tra cứu và tích lũy điểm "
      "thưởng hội viên.");
  gtk_widget_add_css_class(subtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
  gtk_box_append(GTK_BOX(card), subtitle);

  GtkWidget *eFirst = gtk_entry_new();
  GtkWidget *eMid = gtk_entry_new();
  GtkWidget *eLast = gtk_entry_new();
  GtkWidget *ePhone = gtk_entry_new();
  GtkWidget *eEmail = gtk_entry_new();
  GtkWidget *eAddr = gtk_entry_new();

  if (isEdit && existingCust) {
    gtk_editable_set_text(GTK_EDITABLE(eFirst),
                          existingCust->firstName.c_str());
    gtk_editable_set_text(GTK_EDITABLE(eMid), existingCust->middleName.c_str());
    gtk_editable_set_text(GTK_EDITABLE(eLast), existingCust->lastName.c_str());
    gtk_editable_set_text(GTK_EDITABLE(ePhone), existingCust->phone.c_str());
    gtk_editable_set_text(GTK_EDITABLE(eEmail), existingCust->email.c_str());
    gtk_editable_set_text(GTK_EDITABLE(eAddr), existingCust->address.c_str());
  }

  UIComponents::setupPhoneInput(ePhone);

  GtkWidget *errFirst = nullptr;
  GtkWidget *errLast = nullptr;
  GtkWidget *errPhone = nullptr;

  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Tên *", eFirst, &errFirst));
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Tên đệm", eMid));
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Họ *", eLast, &errLast));
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Số điện thoại *", ePhone, &errPhone));
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Email", eEmail));
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Địa chỉ", eAddr));

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 10);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect(
      btnCancel, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        UIComponents::showConfirmDialog(
            self->shell ? self->shell->getWindow() : nullptr, "Xác nhận thoát",
            "Bạn có chắc chắn muốn hủy và quay lại? Dữ liệu chưa lưu sẽ bị "
            "mất.",
            +[](gpointer data) {
              SalesScreens *s = static_cast<SalesScreens *>(data);
              if (s->shell)
                s->shell->popPage();
            },
            self);
      }),
      this);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct CustModalCtx {
    SalesScreens *screens;
    bool isEdit;
    std::string custId;
    GtkWidget *eFirst;
    GtkWidget *eMid;
    GtkWidget *eLast;
    GtkWidget *ePhone;
    GtkWidget *eEmail;
    GtkWidget *eAddr;
    GtkWidget *errFirst;
    GtkWidget *errLast;
    GtkWidget *errPhone;
  };
  CustModalCtx *cm =
      new CustModalCtx{this,     isEdit,  isEdit ? existingCust->id : "",
                       eFirst,   eMid,    eLast,
                       ePhone,   eEmail,  eAddr,
                       errFirst, errLast, errPhone};

  GtkWidget *btnSave =
      gtk_button_new_with_label(isEdit ? "Lưu thay đổi" : "Tạo khách hàng");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  UIComponents::setButtonCursor(btnSave);
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        CustModalCtx *c = static_cast<CustModalCtx *>(d);
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
        GtkWidget *firstErr = nullptr;
        if (req.firstName.empty()) {
          UIComponents::setFieldError(c->errFirst, c->eFirst,
                                      "Tên không được để trống");
          hasError = true;
          if (!firstErr)
            firstErr = c->eFirst;
        }
        if (req.lastName.empty()) {
          UIComponents::setFieldError(c->errLast, c->eLast,
                                      "Họ không được để trống");
          hasError = true;
          if (!firstErr)
            firstErr = c->eLast;
        }
        std::string phoneErr;
        if (!UIComponents::validatePhoneNumber(req.phone, phoneErr)) {
          UIComponents::setFieldError(c->errPhone, c->ePhone, phoneErr);
          hasError = true;
          if (!firstErr)
            firstErr = c->ePhone;
        }

        if (hasError) {
          if (firstErr)
            gtk_widget_grab_focus(firstErr);
          return;
        }

        OperationResult res = c->isEdit
                                  ? c->screens->controller->updateCustomer(req)
                                  : c->screens->controller->addCustomer(req);
        if (res.success) {
          c->screens->refreshCustomers();
          c->screens->refreshPosCustomers();
          if (c->screens->shell) {
            c->screens->shell->popPage();
          }
          UIComponents::showSuccessDialog(
              c->screens->shell ? c->screens->shell->getWindow() : nullptr,
              "Thành công", res.message);
        } else {
          if (res.message.find("điện thoại") != std::string::npos) {
            UIComponents::setFieldError(c->errPhone, c->ePhone, res.message);
            gtk_widget_grab_focus(c->ePhone);
          } else {
            UIComponents::showErrorDialog(
                c->screens->shell ? c->screens->shell->getWindow() : nullptr,
                "Lỗi", res.message);
          }
        }
      }),
      cm, [](gpointer d, GClosure *) { delete static_cast<CustModalCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSave);
  gtk_box_append(GTK_BOX(card), btnRow);

  UIComponents::setupDialogShortcuts(card, btnSave, btnCancel);

  gtk_box_append(GTK_BOX(pageContent), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("sales_customer_form", scroll,
                  isEdit ? ("Chỉnh sửa khách hàng — " + existingCust->id)
                         : "Thêm khách hàng mới");
  gtk_widget_grab_focus(eFirst);
}

// ================= 5. DANH SÁCH HÓA ĐƠN =================

void SalesScreens::buildInvoicesView() {
  invoicesView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(invoicesView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnExportInv = UIComponents::createExportButton(
      "📥 Xuất CSV", "Xuất danh sách hóa đơn thanh toán ra file .csv",
      [this]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName =
            "DanhSach_HoaDon_" +
            ExportHelper::makeSafeDateToken(controller->getSimulatedDate()) +
            ".csv";
        UIComponents::showSaveFileDialog(
            win, "Xuất danh sách hóa đơn (.csv)", defName, "Tệp CSV (*.csv)",
            "*.csv", [this, win](const std::string &path) {
              OperationResult res = controller->exportInvoicesCsv(
                  path, invoiceFilterKeyword, invoiceFromDate, invoiceToDate);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(win, "Xuất CSV thành công",
                                                res.message);
              } else {
                UIComponents::showErrorDialog(win, "Lỗi xuất file",
                                              res.message);
              }
            });
      });

  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh sách hóa đơn thanh toán",
      "Tra cứu hóa đơn bán hàng tại quầy và đơn online đã thanh toán.",
      btnExportInv);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  const char *payMethods[] = {"Tất cả hình thức TT", "Tiền mặt (CASH)",
                              "Chuyển khoản (BANK)", "Thẻ ngân hàng (CARD)",
                              nullptr};
  GtkWidget *comboPayMethod = gtk_drop_down_new_from_strings(payMethods);
  gtk_widget_add_css_class(comboPayMethod, "filter-dropdown");
  UIComponents::applyFilterDropdownBadges(
      comboPayMethod, {"", "badge-paid", "badge-confirmed", "badge-preparing"});
  g_signal_connect_data(comboPayMethod, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          self->invoiceFilterPayMethod =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshInvoices();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(extraControls), comboPayMethod);

  // Bộ lọc theo ngày & tự động sort sớm nhất (ANTY.md mục 2)
  GtkWidget *lblFrom = gtk_label_new("Từ:");
  gtk_widget_add_css_class(lblFrom, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblFrom);

  entryInvoiceFromDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryInvoiceFromDate);
  g_signal_connect(entryInvoiceFromDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     SalesScreens *self = static_cast<SalesScreens *>(d);
                     self->invoiceFromDate = gtk_editable_get_text(e);
                     self->refreshInvoices();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryInvoiceFromDate);

  GtkWidget *lblTo = gtk_label_new("Đến:");
  gtk_widget_add_css_class(lblTo, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblTo);

  entryInvoiceToDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryInvoiceToDate);
  g_signal_connect(entryInvoiceToDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     SalesScreens *self = static_cast<SalesScreens *>(d);
                     self->invoiceToDate = gtk_editable_get_text(e);
                     self->refreshInvoices();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryInvoiceToDate);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã hóa đơn, mã đơn hoặc tên khách hàng...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->invoiceFilterKeyword = txt ? txt : "";
        self->refreshInvoices();
      }),
      this, nullptr, &invoiceCountBadge, extraControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kSalesInvoiceCols));

  invoiceListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), invoiceListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(invoicesView), content);
  refreshInvoices();
}

void SalesScreens::refreshInvoices(const std::string &keyword) {
  if (!invoiceListBox)
    return;
  if (!keyword.empty()) {
    invoiceFilterKeyword = keyword;
  }
  GtkWidget *child = gtk_widget_get_first_child(invoiceListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(invoiceListBox), child);
    child = next;
  }

  std::vector<InvoiceItemViewModel> invs = controller->getAllInvoices();
  std::vector<InvoiceItemViewModel> filtered;
  for (const auto &inv : invs) {
    if (!invoiceFilterKeyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(inv.id, invoiceFilterKeyword) &&
          !Utf8Helper::containsCaseInsensitive(inv.orderId,
                                               invoiceFilterKeyword) &&
          !Utf8Helper::containsCaseInsensitive(inv.customerName,
                                               invoiceFilterKeyword)) {
        continue;
      }
    }
    // Lọc theo hình thức thanh toán
    if (invoiceFilterPayMethod > 0) {
      if (invoiceFilterPayMethod == 1 &&
          inv.paymentMethod != PaymentMethod::CASH &&
          inv.paymentMethodText.find("Tiền mặt") == std::string::npos &&
          inv.paymentMethodText.find("CASH") == std::string::npos) {
        continue;
      }
      if (invoiceFilterPayMethod == 2 &&
          inv.paymentMethod != PaymentMethod::BANK_TRANSFER &&
          inv.paymentMethodText.find("Chuyển khoản") == std::string::npos &&
          inv.paymentMethodText.find("BANK") == std::string::npos) {
        continue;
      }
      if (invoiceFilterPayMethod == 3 &&
          inv.paymentMethod != PaymentMethod::CREDIT_CARD &&
          inv.paymentMethodText.find("Thẻ") == std::string::npos &&
          inv.paymentMethodText.find("CARD") == std::string::npos) {
        continue;
      }
    }

    // Lọc theo khoảng ngày (ANTY.md mục 2)
    if (!invoiceFromDate.empty() || !invoiceToDate.empty()) {
      if (!RealTimeManager::isDateInRange(inv.createTime, invoiceFromDate,
                                          invoiceToDate)) {
        continue;
      }
    }

    filtered.push_back(inv);
  }

  // Tự động sắp xếp theo thời gian sớm nhất (ANTY.md mục 2)
  std::sort(filtered.begin(), filtered.end(),
            [](const InvoiceItemViewModel &a, const InvoiceItemViewModel &b) {
              return RealTimeManager::dateTimeToEpochSeconds(a.createTime) <
                     RealTimeManager::dateTimeToEpochSeconds(b.createTime);
            });

  UIComponents::updateRecordCountBadge(invoiceCountBadge, filtered.size(),
                                       "hóa đơn");

  if (filtered.empty()) {
    std::string emptyMsg =
        invoiceFilterKeyword.empty() && invoiceFilterPayMethod == 0
            ? "Chưa có dữ liệu hóa đơn."
            : "Không có hóa đơn nào khớp với bộ lọc tìm kiếm hiện tại.";
    gtk_box_append(GTK_BOX(invoiceListBox),
                   UIComponents::createEmptyState(
                       "🧾", "Không tìm thấy hóa đơn", emptyMsg));
    return;
  }

  for (size_t i = 0; i < filtered.size(); ++i) {
    const auto &inv = filtered[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã HĐ
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       inv.id, kSalesInvoiceCols[0], "cell-code-inv"));

    // Col 1: Mã đơn
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       inv.orderId, kSalesInvoiceCols[1], "cell-code-ord"));

    // Col 2: Khách hàng
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       inv.customerName, kSalesInvoiceCols[2], "cell-title"));

    // Col 3: Hình thức TT (Badge)
    GtkWidget *payBadge = UIComponents::createStatusBadge(inv.paymentMethodText,
                                                          "badge-confirmed");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     payBadge, kSalesInvoiceCols[3]));

    // Col 4: Thời gian
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       inv.createTime, kSalesInvoiceCols[4], "tabular-num"));

    // Col 5: Tổng tiền (căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(inv.total),
                                     kSalesInvoiceCols[5], "product-price"));

    // Col 6: Thao tác
    GtkWidget *btnDetail = UIComponents::createTableActionButton(
        "🔍 Xem", "Xem biên nhận chi tiết hóa đơn", "btn-action-view");
    gtk_widget_set_hexpand(btnDetail, TRUE);

    struct InvCtx {
      SalesScreens *screens;
      InvoiceItemViewModel item;
    };
    InvCtx *ivCtx = new InvCtx{this, inv};
    g_signal_connect_data(
        btnDetail, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          InvCtx *ic = static_cast<InvCtx *>(d);
          ic->screens->showInvoiceDetailModal(ic->item);
        }),
        ivCtx, [](gpointer d, GClosure *) { delete static_cast<InvCtx *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnDetail, kSalesInvoiceCols[6]));
    gtk_box_append(GTK_BOX(invoiceListBox), row);
  }
}

void SalesScreens::showInvoiceDetailModal(const InvoiceItemViewModel &inv) {
  if (!shell)
    return;

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "card");

  GtkWidget *title = gtk_label_new(("🧾 HÓA ĐƠN BÁN HÀNG " + inv.id).c_str());
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  auto addRow = [card](const char *l, const std::string &v) {
    GtkWidget *r =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
    GtkWidget *lbl = gtk_label_new(l);
    gtk_widget_add_css_class(lbl, "stat-label");
    gtk_widget_set_size_request(lbl, 130, -1);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);

    GtkWidget *val = gtk_label_new(v.c_str());
    gtk_widget_add_css_class(val, "tabular-num");
    gtk_label_set_xalign(GTK_LABEL(val), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(val), TRUE);
    gtk_widget_set_hexpand(val, TRUE);
    gtk_box_append(GTK_BOX(r), lbl);
    gtk_box_append(GTK_BOX(r), val);
    gtk_box_append(GTK_BOX(card), r);
  };

  addRow("Mã đơn gốc:", inv.orderId);
  addRow("Khách hàng:", inv.customerName);
  addRow("Thu ngân:", inv.employeeName);
  addRow("Thời gian:", inv.createTime);
  addRow("Phương thức:", inv.paymentMethodText);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(card), sep);

  GtkWidget *itemsBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  for (size_t idx = 0; idx < inv.details.size(); ++idx) {
    const auto &d = inv.details[idx];
    GtkWidget *r =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
    gtk_widget_add_css_class(r, "pdp-spec-row");
    gtk_widget_add_css_class(r, (idx % 2 == 0) ? "row-even" : "row-odd");

    GtkWidget *n = gtk_label_new(d.phoneName.c_str());
    gtk_widget_set_hexpand(n, TRUE);
    gtk_label_set_xalign(GTK_LABEL(n), 0.0f);

    GtkWidget *q = gtk_label_new(("x" + std::to_string(d.quantity)).c_str());
    gtk_widget_add_css_class(q, "cell-number");
    GtkWidget *p = gtk_label_new(UIComponents::formatVND(d.subtotal).c_str());
    gtk_widget_add_css_class(p, "product-price");

    gtk_box_append(GTK_BOX(r), n);
    gtk_box_append(GTK_BOX(r), q);
    gtk_box_append(GTK_BOX(r), p);
    gtk_box_append(GTK_BOX(itemsBox), r);
  }
  gtk_box_append(GTK_BOX(card), itemsBox);

  addRow("Tạm tính:", UIComponents::formatVND(inv.subtotal));
  if (inv.discountAmount > 0) {
    addRow("Giảm giá:", "-" + UIComponents::formatVND(inv.discountAmount));
  }
  if (inv.isLegacyNoVat) {
    addRow("Thuế VAT:", "Không áp dụng (HĐ cũ)");
  } else {
    addRow(("Thuế VAT (" + PricingCalculator::formatVatPercent(inv.vatRate) + "):").c_str(),
           UIComponents::formatVND(inv.vatAmount));
  }
  addRow("Tổng tiền:", UIComponents::formatVND(inv.total));

  GtkWidget *footerRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  gtk_widget_set_halign(footerRow, GTK_ALIGN_END);

  std::string invIdCopy = inv.id;
  GtkWidget *btnExportTxt = UIComponents::createExportButton(
      "📄 Xuất hóa đơn (.txt)", "Xuất hóa đơn bán hàng sang tệp văn bản (.txt)",
      [this, invIdCopy]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName = "HoaDon_" + invIdCopy + ".txt";
        UIComponents::showSaveFileDialog(
            win, "Xuất hóa đơn bán hàng (.txt)", defName, "Tệp văn bản (*.txt)",
            "*.txt", [this, win, invIdCopy](const std::string &path) {
              OperationResult res =
                  controller->exportInvoiceTxt(path, invIdCopy);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(win, "Xuất hóa đơn thành công",
                                                res.message);
              } else {
                UIComponents::showErrorDialog(win, "Lỗi xuất file",
                                              res.message);
              }
            });
      });
  gtk_box_append(GTK_BOX(footerRow), btnExportTxt);

  GtkWidget *btnClose = gtk_button_new_with_label("← Quay lại");
  gtk_widget_add_css_class(btnClose, "btn-secondary");
  UIComponents::setButtonCursor(btnClose);
  g_signal_connect_swapped(
      btnClose, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->popPage(); }), shell);
  gtk_box_append(GTK_BOX(footerRow), btnClose);
  gtk_box_append(GTK_BOX(card), footerRow);

  gtk_box_append(GTK_BOX(content), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), content);
  UIComponents::setupDialogShortcuts(content, btnClose, btnClose);
  shell->pushPage("sales_invoice_detail", scroll, "Chi tiết hóa đơn " + inv.id);
}

// ================= 6. QUẢN LÝ ĐƠN HÀNG ONLINE =================

void SalesScreens::buildOnlineOrdersView() {
  onlineOrdersView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(onlineOrdersView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnExportOrd = UIComponents::createExportButton(
      "📥 Xuất CSV", "Xuất danh sách đơn hàng online ra file .csv", [this]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName =
            "DanhSach_DonHangOnline_" +
            ExportHelper::makeSafeDateToken(controller->getSimulatedDate()) +
            ".csv";
        UIComponents::showSaveFileDialog(
            win, "Xuất danh sách đơn hàng (.csv)", defName, "Tệp CSV (*.csv)",
            "*.csv", [this, win](const std::string &path) {
              OperationResult res = controller->exportOrdersCsv(
                  path, onlineOrderKeyword, onlineOrderFilterStatus,
                  onlineOrderFilterPayment, onlineOrderFromDate,
                  onlineOrderToDate);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(win, "Xuất CSV thành công",
                                                res.message);
              } else {
                UIComponents::showErrorDialog(win, "Lỗi xuất file",
                                              res.message);
              }
            });
      });

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý đơn hàng Online",
      "Tiếp nhận đơn, xác nhận đơn hàng và chuyển trạng thái xử lý.",
      btnExportOrd);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  const char *statusOptions[] = {"Tất cả trạng thái",
                                 "Chờ xử lý (Pending)",
                                 "Đã xác nhận (Confirmed)",
                                 "Đang giao / Chờ lấy (Shipping)",
                                 "Hoàn tất (Completed)",
                                 "Đã hủy (Cancelled)",
                                 nullptr};
  GtkWidget *dropStatus = gtk_drop_down_new_from_strings(statusOptions);
  gtk_widget_add_css_class(dropStatus, "filter-dropdown");
  UIComponents::applyFilterDropdownBadges(
      dropStatus, {"", "badge-pending", "badge-confirmed", "badge-shipping",
                   "badge-completed", "badge-cancelled"});
  g_signal_connect_data(dropStatus, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          guint sel =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->onlineOrderFilterStatus =
                              (sel == 0) ? -1 : static_cast<int>(sel - 1);
                          self->refreshOnlineOrders();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(extraControls), dropStatus);

  const char *payOptions[] = {"Tất cả thanh toán", "Chưa thanh toán (Unpaid)",
                              "Đã thanh toán (Paid)", nullptr};
  GtkWidget *dropPayment = gtk_drop_down_new_from_strings(payOptions);
  gtk_widget_add_css_class(dropPayment, "filter-dropdown");
  UIComponents::applyFilterDropdownBadges(dropPayment,
                                          {"", "badge-unpaid", "badge-paid"});
  g_signal_connect_data(dropPayment, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          guint sel =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->onlineOrderFilterPayment =
                              (sel == 0) ? -1 : (sel == 1 ? 0 : 1);
                          self->refreshOnlineOrders();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(extraControls), dropPayment);

  // Bộ lọc theo ngày & tự động sort sớm nhất (ANTY.md mục 2)
  GtkWidget *lblFrom = gtk_label_new("Từ:");
  gtk_widget_add_css_class(lblFrom, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblFrom);

  entryOnlineOrderFromDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryOnlineOrderFromDate);
  g_signal_connect(entryOnlineOrderFromDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     SalesScreens *self = static_cast<SalesScreens *>(d);
                     self->onlineOrderFromDate = gtk_editable_get_text(e);
                     self->refreshOnlineOrders();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryOnlineOrderFromDate);

  GtkWidget *lblTo = gtk_label_new("Đến:");
  gtk_widget_add_css_class(lblTo, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblTo);

  entryOnlineOrderToDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryOnlineOrderToDate);
  g_signal_connect(entryOnlineOrderToDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     SalesScreens *self = static_cast<SalesScreens *>(d);
                     self->onlineOrderToDate = gtk_editable_get_text(e);
                     self->refreshOnlineOrders();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryOnlineOrderToDate);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã đơn, tên khách hàng hoặc SĐT người nhận...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->onlineOrderKeyword = txt ? txt : "";
        self->refreshOnlineOrders();
      }),
      this, nullptr, &ordersCountBadge, extraControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kSalesOrderCols));

  ordersListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), ordersListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(onlineOrdersView), content);
  refreshOnlineOrders();
}

void SalesScreens::refreshOnlineOrders() {
  if (!ordersListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(ordersListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(ordersListBox), child);
    child = next;
  }

  std::vector<OrderItemViewModel> allOrders = controller->getAllOrders();
  std::vector<OrderItemViewModel> orders;
  for (const auto &ord : allOrders) {
    if (!onlineOrderKeyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(ord.id, onlineOrderKeyword) &&
          !Utf8Helper::containsCaseInsensitive(ord.customerName,
                                               onlineOrderKeyword) &&
          !Utf8Helper::containsCaseInsensitive(ord.receiverPhone,
                                               onlineOrderKeyword)) {
        continue;
      }
    }
    // Lọc theo trạng thái đơn
    if (onlineOrderFilterStatus >= 0) {
      if (onlineOrderFilterStatus == 0 && ord.status != OrderStatus::PENDING)
        continue;
      if (onlineOrderFilterStatus == 1 && ord.status != OrderStatus::CONFIRMED)
        continue;
      if (onlineOrderFilterStatus == 2 &&
          ord.status != OrderStatus::PREPARING &&
          ord.status != OrderStatus::SHIPPING &&
          ord.status != OrderStatus::READY_FOR_PICKUP)
        continue;
      if (onlineOrderFilterStatus == 3 && ord.status != OrderStatus::COMPLETED)
        continue;
      if (onlineOrderFilterStatus == 4 && ord.status != OrderStatus::CANCELLED)
        continue;
    }
    // Lọc theo thanh toán
    if (onlineOrderFilterPayment >= 0) {
      if (onlineOrderFilterPayment == 0 &&
          ord.paymentStatus != PaymentStatus::UNPAID)
        continue;
      if (onlineOrderFilterPayment == 1 &&
          ord.paymentStatus != PaymentStatus::PAID)
        continue;
    }

    // Lọc theo khoảng ngày (ANTY.md mục 2)
    if (!onlineOrderFromDate.empty() || !onlineOrderToDate.empty()) {
      if (!RealTimeManager::isDateInRange(ord.orderTime, onlineOrderFromDate,
                                          onlineOrderToDate)) {
        continue;
      }
    }

    orders.push_back(ord);
  }

  // Tự động sắp xếp theo thời gian sớm nhất (ANTY.md mục 2)
  std::sort(orders.begin(), orders.end(),
            [](const OrderItemViewModel &a, const OrderItemViewModel &b) {
              return RealTimeManager::dateTimeToEpochSeconds(a.orderTime) <
                     RealTimeManager::dateTimeToEpochSeconds(b.orderTime);
            });

  UIComponents::updateRecordCountBadge(ordersCountBadge, orders.size(),
                                       "đơn hàng");

  if (orders.empty()) {
    std::string emptyMsg =
        onlineOrderKeyword.empty() && onlineOrderFilterStatus < 0 &&
                onlineOrderFilterPayment < 0
            ? "Chưa có dữ liệu đơn hàng."
            : "Không có đơn hàng online nào khớp với bộ lọc hiện tại.";
    gtk_box_append(GTK_BOX(ordersListBox),
                   UIComponents::createEmptyState(
                       "📦", "Chưa có dữ liệu đơn hàng", emptyMsg));
    return;
  }

  for (size_t i = 0; i < orders.size(); ++i) {
    const auto &ord = orders[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã đơn
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(ord.id, kSalesOrderCols[0],
                                                     "cell-code-ord"));

    // Col 1: Khách hàng / SĐT
    gtk_box_append(
        GTK_BOX(row),
        UIComponents::createTableTwoLineCell(
            ord.customerName, "SĐT: " + ord.receiverPhone, kSalesOrderCols[1]));

    // Col 2: Giao nhận
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(ord.deliveryMethodText,
                                                     kSalesOrderCols[2]));

    // Col 3: Thời gian
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       ord.orderTime, kSalesOrderCols[3], "tabular-num"));

    // Col 4: Trạng thái đơn (Badge)
    GtkWidget *stBadge = UIComponents::createOrderStatusBadge(ord.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     stBadge, kSalesOrderCols[4]));

    // Col 5: Thanh toán (Badge)
    GtkWidget *payBadge =
        UIComponents::createPaymentStatusBadge(ord.paymentStatus);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     payBadge, kSalesOrderCols[5]));

    // Col 6: Tổng tiền (căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(ord.total),
                                     kSalesOrderCols[6], "product-price"));

    // Col 7: Action buttons based on status
    GtkWidget *actionsBox =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_XS);
    gtk_widget_set_valign(actionsBox, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(actionsBox, GTK_ALIGN_CENTER);

    struct OrderActionCtx {
      SalesScreens *screens;
      std::string orderId;
      OrderStatus targetStatus;
    };

    if (ord.status == OrderStatus::PENDING) {
      GtkWidget *btnConfirm = UIComponents::createTableActionButton(
          "✓ Xác nhận", "Xác nhận tiếp nhận và xử lý đơn hàng", "btn-primary");
      OrderActionCtx *act =
          new OrderActionCtx{this, ord.id, OrderStatus::CONFIRMED};
      g_signal_connect_data(
          btnConfirm, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            OrderActionCtx *a = static_cast<OrderActionCtx *>(d);
            UIComponents::showConfirmDialog(
                a->screens->shell ? a->screens->shell->getWindow() : nullptr,
                "Xác nhận tiếp nhận đơn hàng",
                "Bạn có chắc chắn muốn xác nhận tiếp nhận đơn hàng " +
                    a->orderId + " không?",
                +[](gpointer data) {
                  OrderActionCtx *act = static_cast<OrderActionCtx *>(data);
                  OperationResult res =
                      act->screens->controller->updateOrderStatus(
                          act->orderId, act->targetStatus);
                  if (res.success) {
                    act->screens->refreshOnlineOrders();
                    act->screens->refreshDashboard();
                    UIComponents::showSuccessDialog(
                        act->screens->shell ? act->screens->shell->getWindow()
                                            : nullptr,
                        "Đã cập nhật", res.message);
                  } else {
                    UIComponents::showErrorDialog(
                        act->screens->shell ? act->screens->shell->getWindow()
                                            : nullptr,
                        "Lỗi", res.message);
                  }
                },
                a);
          }),
          act,
          [](gpointer d, GClosure *) {
            delete static_cast<OrderActionCtx *>(d);
          },
          (GConnectFlags)0);
      gtk_box_append(GTK_BOX(actionsBox), btnConfirm);

      GtkWidget *btnCancel = UIComponents::createTableActionButton(
          "✕ Hủy", "Hủy bỏ đơn hàng online này", "btn-action-delete");
      OrderActionCtx *actC =
          new OrderActionCtx{this, ord.id, OrderStatus::CANCELLED};
      g_signal_connect_data(
          btnCancel, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            OrderActionCtx *a = static_cast<OrderActionCtx *>(d);
            UIComponents::showConfirmDialog(
                a->screens->shell ? a->screens->shell->getWindow() : nullptr,
                "Hủy đơn hàng online",
                "Bạn có chắc chắn muốn hủy đơn hàng " + a->orderId + " không?",
                +[](gpointer data) {
                  OrderActionCtx *act = static_cast<OrderActionCtx *>(data);
                  act->screens->controller->cancelOrder(act->orderId);
                  act->screens->refreshOnlineOrders();
                  act->screens->refreshDashboard();
                },
                a);
          }),
          actC,
          [](gpointer d, GClosure *) {
            delete static_cast<OrderActionCtx *>(d);
          },
          (GConnectFlags)0);
      gtk_box_append(GTK_BOX(actionsBox), btnCancel);
    } else if (ord.status == OrderStatus::SHIPPING ||
               ord.status == OrderStatus::READY_FOR_PICKUP ||
               ord.status == OrderStatus::PREPARING ||
               ord.status == OrderStatus::CONFIRMED) {
      GtkWidget *btnComplete = UIComponents::createTableActionButton(
          "✓ Đã nhận & TT",
          "Xác nhận người mua đã nhận hàng và hoàn tất thanh toán (ANTY.md)",
          "btn-primary");
      OrderActionCtx *act =
          new OrderActionCtx{this, ord.id, OrderStatus::COMPLETED};
      g_signal_connect_data(
          btnComplete, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            OrderActionCtx *a = static_cast<OrderActionCtx *>(d);
            UIComponents::showConfirmDialog(
                a->screens->shell ? a->screens->shell->getWindow() : nullptr,
                "Xác nhận nhận hàng & thanh toán",
                "Xác nhận khách hàng đã nhận được đơn hàng " + a->orderId +
                    " và thanh toán đầy đủ?",
                +[](gpointer data) {
                  OrderActionCtx *act = static_cast<OrderActionCtx *>(data);
                  OperationResult res =
                      act->screens->controller->updateOrderStatus(
                          act->orderId, act->targetStatus);
                  if (res.success) {
                    act->screens->refreshOnlineOrders();
                    act->screens->refreshDashboard();
                    UIComponents::showSuccessDialog(
                        act->screens->shell ? act->screens->shell->getWindow()
                                            : nullptr,
                        "Đã hoàn thành", res.message);
                  } else {
                    UIComponents::showErrorDialog(
                        act->screens->shell ? act->screens->shell->getWindow()
                                            : nullptr,
                        "Lỗi", res.message);
                  }
                },
                a);
          }),
          act,
          [](gpointer d, GClosure *) {
            delete static_cast<OrderActionCtx *>(d);
          },
          (GConnectFlags)0);
      gtk_box_append(GTK_BOX(actionsBox), btnComplete);
    } else {
      GtkWidget *noActionLbl = gtk_label_new("—");
      gtk_widget_add_css_class(noActionLbl, "page-subtitle");
      gtk_box_append(GTK_BOX(actionsBox), noActionLbl);
    }

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     actionsBox, kSalesOrderCols[7]));
    gtk_box_append(GTK_BOX(ordersListBox), row);
  }
}

// ================= 7. TIẾP NHẬN YÊU CẦU BẢO HÀNH =================

void SalesScreens::buildWarrantyIntakeView() {
  warrantyIntakeView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyIntakeView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *actionBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  GtkWidget *btnExportWar = UIComponents::createExportButton(
      "📥 Xuất CSV", "Xuất danh sách phiếu bảo hành ra file .csv", [this]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName =
            "DanhSach_BaoHanh_" +
            ExportHelper::makeSafeDateToken(controller->getSimulatedDate()) +
            ".csv";
        UIComponents::showSaveFileDialog(
            win, "Xuất danh sách bảo hành (.csv)", defName, "Tệp CSV (*.csv)",
            "*.csv", [this, win](const std::string &path) {
              OperationResult res = controller->exportWarrantiesCsv(
                  path, warrantyKeyword, warrantyFilterStatus);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(win, "Xuất CSV thành công",
                                                res.message);
              } else {
                UIComponents::showErrorDialog(win, "Lỗi xuất file",
                                              res.message);
              }
            });
      });
  gtk_box_append(GTK_BOX(actionBox), btnExportWar);

  GtkWidget *btnCreate = gtk_button_new_with_label("➕ Tạo phiếu hộ khách");
  gtk_widget_add_css_class(btnCreate, "btn-primary");
  UIComponents::setButtonCursor(btnCreate);
  g_signal_connect_swapped(
      btnCreate, "clicked",
      G_CALLBACK(+[](SalesScreens *s) { s->showCreateWarrantyModal(); }), this);
  gtk_box_append(GTK_BOX(actionBox), btnCreate);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Tiếp nhận yêu cầu bảo hành",
      "Ghi nhận, xử lý kỹ thuật và theo dõi thiết bị khách hàng mang tới bảo "
      "hành.",
      actionBox);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  const char *statusOptions[] = {
      "Tất cả trạng thái",           "Mới tiếp nhận (Received)",
      "Đang xử lý (Processing)",     "Đã hoàn tất (Resolved)",
      "Từ chối bảo hành (Rejected)", nullptr};
  GtkWidget *dropStatus = gtk_drop_down_new_from_strings(statusOptions);
  gtk_widget_add_css_class(dropStatus, "filter-dropdown");
  UIComponents::applyFilterDropdownBadges(
      dropStatus, {"", "badge-pending", "badge-under-warranty",
                   "badge-completed", "badge-out-of-stock"});
  g_signal_connect_data(dropStatus, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          guint sel =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->warrantyFilterStatus =
                              (sel == 0) ? -1 : static_cast<int>(sel - 1);
                          self->refreshWarrantyIntake();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(extraControls), dropStatus);

  const char *sortOptions[] = {"Ngày nhận: Mới nhất", "Ngày nhận: Cũ nhất",
                               "Khách hàng: A → Z",   "Thiết bị: A → Z",
                               "Mã phiếu: A → Z",     nullptr};
  GtkWidget *dropSort = gtk_drop_down_new_from_strings(sortOptions);
  gtk_widget_add_css_class(dropSort, "filter-dropdown");
  g_signal_connect_data(dropSort, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          self->warrantySortBy =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshWarrantyIntake();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(extraControls), dropSort);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã phiếu, mã đơn, tên thiết bị hoặc khách hàng...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->warrantyKeyword = txt ? txt : "";
        self->refreshWarrantyIntake();
      }),
      this, nullptr, &warrantyCountBadge, extraControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kSalesWarrantyCols));

  warrantyListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), warrantyListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(warrantyIntakeView),
                                content);
  refreshWarrantyIntake();
}

void SalesScreens::refreshWarrantyIntake() {
  if (!warrantyListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(warrantyListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(warrantyListBox), child);
    child = next;
  }

  std::vector<WarrantyItemViewModel> all = controller->getWarranties();
  std::vector<WarrantyItemViewModel> list;
  for (const auto &w : all) {
    if (!warrantyKeyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(w.id, warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.orderId, warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.phoneName, warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.customerName,
                                               warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.customerPhone,
                                               warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.issueDescription,
                                               warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.technicianNote,
                                               warrantyKeyword)) {
        continue;
      }
    }
    // Lọc theo trạng thái bảo hành
    if (warrantyFilterStatus >= 0) {
      if (warrantyFilterStatus == 0 && w.status != WarrantyStatus::RECEIVED)
        continue;
      if (warrantyFilterStatus == 1 && w.status != WarrantyStatus::PROCESSING)
        continue;
      if (warrantyFilterStatus == 2 && w.status != WarrantyStatus::RESOLVED)
        continue;
      if (warrantyFilterStatus == 3 && w.status != WarrantyStatus::REJECTED)
        continue;
    }
    list.push_back(w);
  }

  // Sắp xếp
  if (warrantySortBy == 0) { // Ngày nhận: Mới nhất
    std::sort(
        list.begin(), list.end(),
        [](const WarrantyItemViewModel &a, const WarrantyItemViewModel &b) {
          return a.requestDate > b.requestDate;
        });
  } else if (warrantySortBy == 1) { // Ngày nhận: Cũ nhất
    std::sort(
        list.begin(), list.end(),
        [](const WarrantyItemViewModel &a, const WarrantyItemViewModel &b) {
          return a.requestDate < b.requestDate;
        });
  } else if (warrantySortBy == 2) { // Khách hàng: A -> Z
    std::sort(
        list.begin(), list.end(),
        [](const WarrantyItemViewModel &a, const WarrantyItemViewModel &b) {
          return a.customerName < b.customerName;
        });
  } else if (warrantySortBy == 3) { // Thiết bị: A -> Z
    std::sort(
        list.begin(), list.end(),
        [](const WarrantyItemViewModel &a, const WarrantyItemViewModel &b) {
          return a.phoneName < b.phoneName;
        });
  } else if (warrantySortBy == 4) { // Mã phiếu: A -> Z
    std::sort(list.begin(), list.end(),
              [](const WarrantyItemViewModel &a,
                 const WarrantyItemViewModel &b) { return a.id < b.id; });
  }

  UIComponents::updateRecordCountBadge(warrantyCountBadge, list.size(),
                                       "phiếu bảo hành");

  if (list.empty()) {
    std::string emptyMsg =
        warrantyKeyword.empty() && warrantyFilterStatus < 0
            ? "Chưa có dữ liệu bảo hành."
            : "Không tìm thấy phiếu bảo hành nào khớp với bộ lọc tìm kiếm.";
    gtk_box_append(GTK_BOX(warrantyListBox),
                   UIComponents::createEmptyState(
                       "🛡️", "Chưa có dữ liệu bảo hành", emptyMsg));
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &w = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã phiếu & Mã đơn
    std::string orderSub =
        w.orderId.empty() ? "Đơn vãng lai" : ("Đơn: " + w.orderId);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     w.id, orderSub, kSalesWarrantyCols[0]));

    // Col 1: Thiết bị & Bảo hành
    std::string remSub;
    if (w.remainingDays > 0) {
      remSub = "Còn " + std::to_string(w.remainingDays) + " ngày bảo hành";
    } else if (!w.expireDate.empty()) {
      remSub = "Hết hạn (" + w.expireDate + ")";
    } else {
      remSub = "Bảo hành tiêu chuẩn";
    }
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(w.phoneName, remSub,
                                                        kSalesWarrantyCols[1]));

    // Col 2: Khách hàng
    std::string phoneSub =
        w.customerPhone.empty() ? "Không có SĐT" : ("SĐT: " + w.customerPhone);
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       w.customerName, phoneSub, kSalesWarrantyCols[2]));

    // Col 3: Ngày nhận
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.requestDate, kSalesWarrantyCols[3], "tabular-num"));

    // Col 4: Trạng thái badge
    GtkWidget *badge = UIComponents::createWarrantyStatusBadge(w.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badge, kSalesWarrantyCols[4]));

    // Col 5: Mô tả sự cố & Ghi chú KT
    std::string noteSub = w.technicianNote.empty()
                              ? "Chưa có ghi chú KT"
                              : ("KT: " + w.technicianNote);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     "Lỗi: " + w.issueDescription, noteSub,
                                     kSalesWarrantyCols[5]));

    // Col 6: Thao tác (Xử lý phiếu)
    if (w.status != WarrantyStatus::RESOLVED &&
        w.status != WarrantyStatus::REJECTED) {
      GtkWidget *btnProcess = UIComponents::createTableActionButton(
          "⚙️ Xử lý", "Tiến hành xử lý và cập nhật tiến độ bảo hành",
          "btn-action-primary");
      struct ProcWarData {
        SalesScreens *s;
        WarrantyItemViewModel item;
      };
      ProcWarData *pwd = new ProcWarData{this, w};
      g_signal_connect_data(
          btnProcess, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            ProcWarData *data = static_cast<ProcWarData *>(d);
            data->s->showProcessWarrantyModal(data->item);
          }),
          pwd,
          [](gpointer d, GClosure *) { delete static_cast<ProcWarData *>(d); },
          (GConnectFlags)0);
      gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                       btnProcess, kSalesWarrantyCols[6]));
    } else {
      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableTextCell(
                         "Đã đóng", kSalesWarrantyCols[6], "cell-subtitle"));
    }

    gtk_box_append(GTK_BOX(warrantyListBox), row);
  }
}

void SalesScreens::showProcessWarrantyModal(const WarrantyItemViewModel &w) {
  if (!shell)
    return;

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *pageContent =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(pageContent, "page-container");

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "pdp-card");
  gtk_widget_set_hexpand(card, TRUE);

  GtkWidget *title = gtk_label_new(("⚙️ Xử lý phiếu bảo hành: " + w.id).c_str());
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  GtkWidget *subtitle = gtk_label_new(
      "Kiểm tra tình trạng thiết bị, chẩn đoán kỹ thuật và cập nhật trạng thái "
      "xử lý theo quy trình.");
  gtk_widget_add_css_class(subtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
  gtk_box_append(GTK_BOX(card), subtitle);

  // Grid thông tin phiếu
  GtkWidget *infoGrid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(infoGrid), 8);
  gtk_grid_set_column_spacing(GTK_GRID(infoGrid), 16);

  auto addInfoRow = [&](int rowIdx, const std::string &lblText,
                        const std::string &valText,
                        GtkWidget *customWidget = nullptr) {
    GtkWidget *lbl = gtk_label_new(lblText.c_str());
    gtk_widget_add_css_class(lbl, "stat-label");
    gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);
    gtk_grid_attach(GTK_GRID(infoGrid), lbl, 0, rowIdx, 1, 1);

    if (customWidget != nullptr) {
      gtk_grid_attach(GTK_GRID(infoGrid), customWidget, 1, rowIdx, 1, 1);
    } else {
      GtkWidget *vLbl = gtk_label_new(valText.c_str());
      gtk_widget_add_css_class(vLbl, "cell-title");
      gtk_label_set_xalign(GTK_LABEL(vLbl), 0.0f);
      gtk_label_set_wrap(GTK_LABEL(vLbl), TRUE);
      gtk_grid_attach(GTK_GRID(infoGrid), vLbl, 1, rowIdx, 1, 1);
    }
  };

  std::string custDisp = w.customerName;
  if (!w.customerPhone.empty())
    custDisp += " (SĐT: " + w.customerPhone + ")";
  addInfoRow(0, "Khách hàng:", custDisp);

  std::string devDisp = w.phoneName;
  if (!w.phoneId.empty())
    devDisp += " [" + w.phoneId + "]";
  addInfoRow(1, "Thiết bị:", devDisp);

  addInfoRow(
      2, "Mã đơn hàng:", w.orderId.empty() ? "Đơn vãng lai (POS)" : w.orderId);

  std::string expDisp;
  if (w.remainingDays > 0) {
    expDisp = "Còn " + std::to_string(w.remainingDays) +
              " ngày (Hết hạn: " + w.expireDate + ")";
  } else if (!w.expireDate.empty()) {
    expDisp = "Đã hết hạn (" + w.expireDate + ")";
  } else {
    expDisp = "Bảo hành tiêu chuẩn";
  }
  addInfoRow(3, "Thời hạn bảo hành:", expDisp);

  addInfoRow(4, "Ngày tiếp nhận:", w.requestDate);

  GtkWidget *curBadge = UIComponents::createWarrantyStatusBadge(w.status);
  gtk_widget_set_halign(curBadge, GTK_ALIGN_START);
  addInfoRow(5, "Trạng thái hiện tại:", "", curBadge);

  addInfoRow(6, "Mô tả lỗi từ khách:", w.issueDescription);

  gtk_box_append(GTK_BOX(card), infoGrid);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_set_margin_top(sep, 6);
  gtk_widget_set_margin_bottom(sep, 6);
  gtk_box_append(GTK_BOX(card), sep);

  // Form chuyển trạng thái kế tiếp theo State Machine
  std::vector<WarrantyStatus> nextStatuses;
  std::vector<std::string> statusLabels;
  std::vector<const char *> statusPtrs;

  if (w.status == WarrantyStatus::RECEIVED) {
    nextStatuses.push_back(WarrantyStatus::PROCESSING);
    statusLabels.push_back("Đang xử lý kỹ thuật (PROCESSING)");

    nextStatuses.push_back(WarrantyStatus::REJECTED);
    statusLabels.push_back("Từ chối bảo hành (REJECTED)");
  } else if (w.status == WarrantyStatus::PROCESSING) {
    nextStatuses.push_back(WarrantyStatus::RESOLVED);
    statusLabels.push_back("Hoàn tất bảo hành / Trả máy (RESOLVED)");

    nextStatuses.push_back(WarrantyStatus::REJECTED);
    statusLabels.push_back("Từ chối bảo hành (REJECTED)");
  }

  for (const auto &s : statusLabels) {
    statusPtrs.push_back(s.c_str());
  }
  statusPtrs.push_back(nullptr);

  GtkWidget *comboStatus = gtk_drop_down_new_from_strings(statusPtrs.data());
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Chuyển sang trạng thái *", comboStatus));

  GtkWidget *entryNote = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entryNote), 500);
  if (!w.technicianNote.empty()) {
    gtk_editable_set_text(GTK_EDITABLE(entryNote), w.technicianNote.c_str());
  }
  gtk_entry_set_placeholder_text(GTK_ENTRY(entryNote),
                                 "Ghi nhận tình trạng kỹ thuật, nguyên nhân hư "
                                 "hỏng, linh kiện cần thay...");
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Ghi chú kỹ thuật viên *", entryNote));

  GtkWidget *entryRes = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entryRes), 500);
  if (!w.resolution.empty()) {
    gtk_editable_set_text(GTK_EDITABLE(entryRes), w.resolution.c_str());
  }
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(entryRes),
      "Biện pháp giải quyết: Đã thay linh kiện / Đã cài đặt lại / Trả máy từ "
      "chối...");
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Kết quả xử lý & Biện pháp *", entryRes));

  GtkWidget *lblHint = gtk_label_new(
      "💡 Lưu ý: Khi chuyển sang Hoàn tất (RESOLVED) hoặc Từ chối (REJECTED), "
      "bắt buộc phải nhập ghi chú kỹ thuật hoặc kết quả giải quyết.");
  gtk_widget_add_css_class(lblHint, "form-hint");
  gtk_label_set_xalign(GTK_LABEL(lblHint), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblHint), TRUE);
  gtk_box_append(GTK_BOX(card), lblHint);

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 8);

  std::string warIdCopy = w.id;
  GtkWidget *btnExportWarTxt = UIComponents::createExportButton(
      "📄 Xuất phiếu BH (.txt)", "Xuất phiếu bảo hành sang tệp văn bản (.txt)",
      [this, warIdCopy]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName = "PhieuBaoHanh_" + warIdCopy + ".txt";
        UIComponents::showSaveFileDialog(
            win, "Xuất phiếu bảo hành (.txt)", defName, "Tệp văn bản (*.txt)",
            "*.txt", [this, win, warIdCopy](const std::string &path) {
              OperationResult res =
                  controller->exportWarrantyTxt(path, warIdCopy);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(
                    win, "Xuất phiếu bảo hành thành công", res.message);
              } else {
                UIComponents::showErrorDialog(win, "Lỗi xuất file",
                                              res.message);
              }
            });
      });
  gtk_box_append(GTK_BOX(btnRow), btnExportWarTxt);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->popPage(); }), shell);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  GtkWidget *btnSubmit = gtk_button_new_with_label("💾 Lưu cập nhật");
  gtk_widget_add_css_class(btnSubmit, "btn-primary");
  UIComponents::setButtonCursor(btnSubmit);

  struct ProcessCtx {
    SalesScreens *screens;
    std::string wreqId;
    GtkWidget *combo;
    GtkWidget *entryNote;
    GtkWidget *entryRes;
    std::vector<WarrantyStatus> allowedStatuses;
  };
  ProcessCtx *ctx = new ProcessCtx{this,      w.id,     comboStatus,
                                   entryNote, entryRes, nextStatuses};

  g_signal_connect_data(
      btnSubmit, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        ProcessCtx *c = static_cast<ProcessCtx *>(d);
        GtkWindow *win =
            c->screens->shell ? c->screens->shell->getWindow() : nullptr;

        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->combo));
        if (sel >= c->allowedStatuses.size()) {
          UIComponents::showWarningDialog(win, "Lỗi",
                                          "Vui lòng chọn trạng thái hợp lệ!");
          return;
        }

        WarrantyStatus targetStatus = c->allowedStatuses[sel];
        const char *rawNote = gtk_editable_get_text(GTK_EDITABLE(c->entryNote));
        const char *rawRes = gtk_editable_get_text(GTK_EDITABLE(c->entryRes));
        std::string note = rawNote ? rawNote : "";
        std::string resText = rawRes ? rawRes : "";

        // Trim
        auto trimStr = [](const std::string &s) {
          size_t f = s.find_first_not_of(" \t\r\n");
          if (f == std::string::npos)
            return std::string("");
          size_t l = s.find_last_not_of(" \t\r\n");
          return s.substr(f, l - f + 1);
        };
        std::string cleanNote = trimStr(note);
        std::string cleanRes = trimStr(resText);

        if (targetStatus == WarrantyStatus::RESOLVED ||
            targetStatus == WarrantyStatus::REJECTED) {
          if (cleanNote.empty() && cleanRes.empty()) {
            UIComponents::showWarningDialog(
                win, "Thiếu thông tin bắt buộc",
                "Khi hoàn tất hoặc từ chối bảo hành, bắt buộc phải nhập ghi "
                "chú "
                "kỹ thuật hoặc kết quả giải quyết!");
            return;
          }
        }

        // Khóa nút để tránh bấm đúp
        gtk_widget_set_sensitive(GTK_WIDGET(b), FALSE);

        OperationResult res = c->screens->controller->updateWarrantyStatus(
            c->wreqId, targetStatus, cleanNote, cleanRes);
        if (res.success) {
          c->screens->refreshWarrantyIntake();
          if (c->screens->shell) {
            c->screens->shell->popPage();
          }
          UIComponents::showSuccessDialog(
              c->screens->shell ? c->screens->shell->getWindow() : nullptr,
              "Cập nhật thành công", res.message);
        } else {
          gtk_widget_set_sensitive(GTK_WIDGET(b), TRUE);
          UIComponents::showErrorDialog(win, "Cập nhật thất bại", res.message);
        }
      }),
      ctx, [](gpointer d, GClosure *) { delete static_cast<ProcessCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSubmit);
  gtk_box_append(GTK_BOX(card), btnRow);

  UIComponents::setupDialogShortcuts(card, btnSubmit, btnCancel);

  gtk_box_append(GTK_BOX(pageContent), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("sales_process_warranty", scroll,
                  "Xử lý phiếu bảo hành " + w.id);
  gtk_widget_grab_focus(entryNote);
}

void SalesScreens::showCreateWarrantyModal() {
  if (!shell)
    return;

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *pageContent =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(pageContent, "page-container");

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "pdp-card");
  gtk_widget_set_hexpand(card, TRUE);

  GtkWidget *title = gtk_label_new("🛡️ Tiếp nhận bảo hành hộ khách hàng");
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  GtkWidget *subtitle = gtk_label_new(
      "Chọn khách hàng (hoặc đơn POS vãng lai), chọn thiết bị đã mua cần bảo "
      "hành và ghi nhận hiện tượng lỗi.");
  gtk_widget_add_css_class(subtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
  gtk_box_append(GTK_BOX(card), subtitle);

  // 1. Mục "Khách hàng bảo hành" tích hợp ô tìm kiếm trực tiếp + dropdown chọn khách hàng
  GtkWidget *entryCustSearch = gtk_entry_new();
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(entryCustSearch),
      "🔍 Tìm khách hàng bảo hành theo tên, SĐT, mã KH hoặc email...");
  gtk_entry_set_icon_from_icon_name(GTK_ENTRY(entryCustSearch),
                                    GTK_ENTRY_ICON_PRIMARY,
                                    "edit-find-symbolic");

  std::vector<CustomerItemViewModel> allCusts = controller->getCustomers();

  GtkWidget *lblCustSearchStatus = gtk_label_new(
      ("Hiển thị tất cả " + std::to_string(allCusts.size()) +
       " khách hàng thành viên + Khách vãng lai (POS).")
          .c_str());
  gtk_widget_add_css_class(lblCustSearchStatus, "form-hint");
  gtk_label_set_xalign(GTK_LABEL(lblCustSearchStatus), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblCustSearchStatus), TRUE);

  // Khởi tạo danh sách khách hàng ban đầu
  std::vector<std::string> currentCustIds;
  std::vector<std::string> custDisplayStrings;
  std::vector<const char *> custPtrs;

  auto buildCustLists = [&](const std::string &kw) {
    currentCustIds.clear();
    custDisplayStrings.clear();
    custPtrs.clear();

    // Lựa chọn đầu tiên khi chưa tìm kiếm là Khách vãng lai (đơn POS)
    currentCustIds.push_back("");
    custDisplayStrings.push_back(
        "[Khách vãng lai] — Đơn POS tại quầy (Không có tài khoản)");

    for (const auto &c : allCusts) {
      if (!kw.empty()) {
        bool match = Utf8Helper::containsCaseInsensitive(c.fullName, kw) ||
                     Utf8Helper::containsCaseInsensitive(c.phone, kw) ||
                     Utf8Helper::containsCaseInsensitive(c.id, kw) ||
                     Utf8Helper::containsCaseInsensitive(c.username, kw) ||
                     Utf8Helper::containsCaseInsensitive(c.email, kw);
        if (!match)
          continue;
      }
      currentCustIds.push_back(c.id);
      std::string disp = "[" + c.id + "] " + c.fullName +
                         " — SĐT: " + (c.phone.empty() ? "N/A" : c.phone);
      custDisplayStrings.push_back(disp);
    }

    for (const auto &s : custDisplayStrings) {
      custPtrs.push_back(s.c_str());
    }
    custPtrs.push_back(nullptr);
  };

  buildCustLists("");

  GtkWidget *comboCust = gtk_drop_down_new_from_strings(custPtrs.data());
  gtk_drop_down_set_enable_search(GTK_DROP_DOWN(comboCust), TRUE);

  GtkWidget *custPickerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_append(GTK_BOX(custPickerBox), entryCustSearch);
  gtk_box_append(GTK_BOX(custPickerBox), lblCustSearchStatus);
  gtk_box_append(GTK_BOX(custPickerBox), comboCust);
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Khách hàng bảo hành *", custPickerBox));

  // 2. Dropdown thiết bị bảo hành & nhãn giải thích
  std::vector<WarrantyEligibleItemViewModel> currentEligibleItems;
  std::vector<std::string> deviceDisplayStrings;
  std::vector<const char *> devicePtrs;

  GtkWidget *comboDevice = gtk_drop_down_new_from_strings(nullptr);
  GtkWidget *lblDeviceStatus = gtk_label_new("");
  gtk_label_set_xalign(GTK_LABEL(lblDeviceStatus), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblDeviceStatus), TRUE);
  gtk_widget_add_css_class(lblDeviceStatus, "form-hint");

  GtkWidget *devBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_box_append(GTK_BOX(devBox), comboDevice);
  gtk_box_append(GTK_BOX(devBox), lblDeviceStatus);
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Chọn thiết bị cần bảo hành *", devBox));

  // 3. Ô mô tả sự cố
  GtkWidget *entryIssue = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entryIssue), 500);
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(entryIssue),
      "Mô tả chi tiết hiện tượng lỗi gặp phải (tối đa 500 ký tự)...");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField(
                     "Mô tả sự cố * (tối đa 500 ký tự)", entryIssue));

  // 4. Hàng nút thao tác
  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 8);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->popPage(); }), shell);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  GtkWidget *btnSubmit = gtk_button_new_with_label("Tạo phiếu bảo hành");
  gtk_widget_add_css_class(btnSubmit, "btn-primary");
  UIComponents::setButtonCursor(btnSubmit);

  struct CreateModalCtx {
    SalesScreens *screens;
    std::vector<CustomerItemViewModel> allCusts;
    std::vector<std::string> currentCustIds;
    std::vector<WarrantyEligibleItemViewModel> currentEligibleItems;
    GtkWidget *entrySearch;
    GtkWidget *lblCustStatus;
    GtkWidget *comboCust;
    GtkWidget *comboDevice;
    GtkWidget *lblDeviceStatus;
    GtkWidget *entryIssue;
    GtkWidget *btnSubmit;
  };
  CreateModalCtx *modalCtx = new CreateModalCtx{this,
                                                allCusts,
                                                currentCustIds,
                                                currentEligibleItems,
                                                entryCustSearch,
                                                lblCustSearchStatus,
                                                comboCust,
                                                comboDevice,
                                                lblDeviceStatus,
                                                entryIssue,
                                                btnSubmit};

  g_object_set_data_full(
      G_OBJECT(card), "create_warranty_modal_ctx", modalCtx,
      [](gpointer d) { delete static_cast<CreateModalCtx *>(d); });

  auto refreshDevicesForSelectedCust = [](CreateModalCtx *c) {
    if (c->currentCustIds.empty()) {
      c->currentEligibleItems.clear();
      const char *emptyPtrs[] = {"(Vui lòng chọn một khách hàng hợp lệ)",
                                 nullptr};
      GtkStringList *sList = gtk_string_list_new(emptyPtrs);
      gtk_drop_down_set_model(GTK_DROP_DOWN(c->comboDevice),
                              G_LIST_MODEL(sList));
      g_object_unref(sList);
      gtk_label_set_text(
          GTK_LABEL(c->lblDeviceStatus),
          "⚠️ Chưa chọn được khách hàng phù hợp để tra cứu thiết bị bảo hành.");
      gtk_widget_set_sensitive(c->btnSubmit, FALSE);
      return;
    }

    guint selCust = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->comboCust));
    std::string targetCustomerId =
        (selCust < c->currentCustIds.size()) ? c->currentCustIds[selCust] : "";

    c->currentEligibleItems =
        c->screens->controller->getEligibleWarrantyItems(targetCustomerId);

    std::vector<std::string> devStrs;
    std::vector<const char *> devPtrs;
    int firstValid = -1;

    if (c->currentEligibleItems.empty()) {
      devStrs.push_back(
          targetCustomerId.empty()
              ? "(Không có đơn POS vãng lai nào đủ điều kiện)"
              : "(Khách hàng này chưa có sản phẩm nào đủ điều kiện)");
      devPtrs.push_back(devStrs[0].c_str());
      devPtrs.push_back(nullptr);

      GtkStringList *sList = gtk_string_list_new(devPtrs.data());
      gtk_drop_down_set_model(GTK_DROP_DOWN(c->comboDevice),
                              G_LIST_MODEL(sList));
      g_object_unref(sList);

      std::string note =
          targetCustomerId.empty()
              ? "⚠️ Không tìm thấy đơn hàng POS vãng lai hoàn tất nào chứa sản "
                "phẩm còn hạn bảo hành."
              : "⚠️ Khách hàng này chưa có đơn hàng hoàn tất nào mua sản phẩm "
                "còn thời hạn bảo hành.";
      gtk_label_set_text(GTK_LABEL(c->lblDeviceStatus), note.c_str());
      gtk_widget_set_sensitive(c->btnSubmit, FALSE);
      return;
    }

    for (size_t i = 0; i < c->currentEligibleItems.size(); ++i) {
      const auto &it = c->currentEligibleItems[i];
      std::string label =
          it.phoneName + " — Đơn " + it.orderId + " (Mua: " + it.purchaseDate;
      if (!it.active || it.remainingDays < 0) {
        label += " • Đã hết hạn";
        if (!it.expireDate.empty())
          label += " " + it.expireDate;
        label += ") [Vô hiệu hóa: Hết hạn]";
      } else if (it.openRequests >= it.quantity) {
        label += " • Còn " + std::to_string(it.remainingDays) +
                 " ngày) [Vô hiệu hóa: Đã đạt " +
                 std::to_string(it.openRequests) + "/" +
                 std::to_string(it.quantity) + " yêu cầu mở]";
      } else {
        label += " • Còn " + std::to_string(it.remainingDays) + " ngày)";
        if (firstValid < 0)
          firstValid = static_cast<int>(i);
      }
      devStrs.push_back(label);
    }

    for (const auto &s : devStrs) {
      devPtrs.push_back(s.c_str());
    }
    devPtrs.push_back(nullptr);

    GtkStringList *sList = gtk_string_list_new(devPtrs.data());
    gtk_drop_down_set_model(GTK_DROP_DOWN(c->comboDevice), G_LIST_MODEL(sList));
    g_object_unref(sList);

    if (firstValid >= 0) {
      gtk_drop_down_set_selected(GTK_DROP_DOWN(c->comboDevice),
                                 static_cast<guint>(firstValid));
      const auto &it = c->currentEligibleItems[firstValid];
      std::string text = "✅ Thiết bị đủ điều kiện bảo hành (còn " +
                         std::to_string(it.remainingDays) + " ngày).";
      gtk_label_set_text(GTK_LABEL(c->lblDeviceStatus), text.c_str());
      gtk_widget_set_sensitive(c->btnSubmit, TRUE);
    } else {
      gtk_label_set_text(
          GTK_LABEL(c->lblDeviceStatus),
          "⚠️ Tất cả sản phẩm đã mua của khách đều đã hết hạn hoặc đạt tối đa "
          "yêu cầu mở.");
      gtk_widget_set_sensitive(c->btnSubmit, FALSE);
    }
  };

  // Nạp thiết bị cho lựa chọn mặc định ban đầu
  refreshDevicesForSelectedCust(modalCtx);

  // Lắng nghe sự kiện tìm kiếm khách hàng bảo hành (ưu tiên khớp khách hàng lên đầu & tự động chọn)
  static auto sRefreshDevices =
      static_cast<void (*)(CreateModalCtx *)>(refreshDevicesForSelectedCust);
  g_signal_connect_data(
      entryCustSearch, "changed", G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        CreateModalCtx *c = static_cast<CreateModalCtx *>(d);
        const char *txt = gtk_editable_get_text(e);
        std::string kw = txt ? StringHelper::trim(txt) : "";

        c->currentCustIds.clear();
        std::vector<std::string> dispStrs;
        std::vector<const char *> ptrs;

        if (kw.empty()) {
          c->currentCustIds.push_back("");
          dispStrs.push_back(
              "[Khách vãng lai] — Đơn POS tại quầy (Không có tài khoản)");
        }

        size_t matchedMembers = 0;
        for (const auto &cust : c->allCusts) {
          if (!kw.empty()) {
            bool match =
                Utf8Helper::containsCaseInsensitive(cust.fullName, kw) ||
                Utf8Helper::containsCaseInsensitive(cust.phone, kw) ||
                Utf8Helper::containsCaseInsensitive(cust.id, kw) ||
                Utf8Helper::containsCaseInsensitive(cust.username, kw) ||
                Utf8Helper::containsCaseInsensitive(cust.email, kw);
            if (!match)
              continue;
          }
          c->currentCustIds.push_back(cust.id);
          std::string disp = "[" + cust.id + "] " + cust.fullName + " — SĐT: " +
                             (cust.phone.empty() ? "N/A" : cust.phone);
          dispStrs.push_back(disp);
          matchedMembers++;
        }

        // Nếu từ khóa tìm "vãng lai" hoặc "pos", đưa mục Khách vãng lai vào kết quả
        if (!kw.empty() &&
            (Utf8Helper::containsCaseInsensitive("Khách vãng lai POS", kw) ||
             Utf8Helper::containsCaseInsensitive("guest", kw))) {
          c->currentCustIds.push_back("");
          dispStrs.push_back(
              "[Khách vãng lai] — Đơn POS tại quầy (Không có tài khoản)");
        }

        if (c->currentCustIds.empty()) {
          dispStrs.push_back("(Không tìm thấy khách hàng khớp với '" + kw +
                             "')");
          ptrs.push_back(dispStrs[0].c_str());
          ptrs.push_back(nullptr);
          GtkStringList *sList = gtk_string_list_new(ptrs.data());
          gtk_drop_down_set_model(GTK_DROP_DOWN(c->comboCust),
                                  G_LIST_MODEL(sList));
          g_object_unref(sList);
          gtk_label_set_text(
              GTK_LABEL(c->lblCustStatus),
              ("⚠️ Không tìm thấy khách hàng nào khớp với '" + kw + "'.")
                  .c_str());
          sRefreshDevices(c);
          return;
        }

        for (const auto &s : dispStrs) {
          ptrs.push_back(s.c_str());
        }
        ptrs.push_back(nullptr);

        GtkStringList *sList = gtk_string_list_new(ptrs.data());
        gtk_drop_down_set_model(GTK_DROP_DOWN(c->comboCust),
                                G_LIST_MODEL(sList));
        g_object_unref(sList);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(c->comboCust), 0);

        if (kw.empty()) {
          gtk_label_set_text(
              GTK_LABEL(c->lblCustStatus),
              ("Hiển thị tất cả " + std::to_string(c->allCusts.size()) +
               " khách hàng thành viên + Khách vãng lai (POS).")
                  .c_str());
        } else {
          gtk_label_set_text(
              GTK_LABEL(c->lblCustStatus),
              ("✅ Tìm thấy " + std::to_string(c->currentCustIds.size()) +
               " khách hàng phù hợp (" + std::to_string(matchedMembers) +
               " thành viên). Đã tự động chọn kết quả đầu tiên.")
                  .c_str());
        }

        // Luôn nạp lại danh sách thiết bị ngay khi lọc (kể cả khi index vẫn là 0)
        sRefreshDevices(c);
      }),
      modalCtx, nullptr, (GConnectFlags)0);

  // Lắng nghe thay đổi chọn khách hàng
  g_signal_connect_data(
      comboCust, "notify::selected",
      G_CALLBACK(+[](GObject *, GParamSpec *, gpointer d) {
        CreateModalCtx *c = static_cast<CreateModalCtx *>(d);
        sRefreshDevices(c);
      }),
      modalCtx, nullptr, (GConnectFlags)0);

  // Lắng nghe thay đổi chọn thiết bị
  g_signal_connect_data(
      comboDevice, "notify::selected",
      G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
        CreateModalCtx *c = static_cast<CreateModalCtx *>(d);
        guint selDev = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
        if (selDev >= c->currentEligibleItems.size()) {
          gtk_label_set_text(GTK_LABEL(c->lblDeviceStatus), "");
          gtk_widget_set_sensitive(c->btnSubmit, FALSE);
          return;
        }

        const auto &it = c->currentEligibleItems[selDev];
        if (!it.active || it.remainingDays < 0) {
          std::string text =
              "⚠️ Mục này đã bị vô hiệu hóa: Thiết bị đã hết hạn bảo hành (" +
              it.expireDate + "). Không thể tạo yêu cầu bảo hành.";
          gtk_label_set_text(GTK_LABEL(c->lblDeviceStatus), text.c_str());
          gtk_widget_set_sensitive(c->btnSubmit, FALSE);
        } else if (it.openRequests >= it.quantity) {
          std::string text =
              "⚠️ Mục này đã bị vô hiệu hóa: Thiết bị đã đạt giới hạn " +
              std::to_string(it.openRequests) + "/" +
              std::to_string(it.quantity) +
              " yêu cầu bảo hành đang mở. Vui lòng chờ xử lý các phiếu trước.";
          gtk_label_set_text(GTK_LABEL(c->lblDeviceStatus), text.c_str());
          gtk_widget_set_sensitive(c->btnSubmit, FALSE);
        } else {
          std::string text = "✅ Thiết bị đủ điều kiện bảo hành (còn " +
                             std::to_string(it.remainingDays) +
                             " ngày bảo hành).";
          gtk_label_set_text(GTK_LABEL(c->lblDeviceStatus), text.c_str());
          gtk_widget_set_sensitive(c->btnSubmit, TRUE);
        }
      }),
      modalCtx, nullptr, (GConnectFlags)0);

  // Nút submit tạo phiếu
  g_signal_connect_data(
      btnSubmit, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        CreateModalCtx *c = static_cast<CreateModalCtx *>(d);
        GtkWindow *win =
            c->screens->shell ? c->screens->shell->getWindow() : nullptr;

        guint selCust = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->comboCust));
        if (selCust >= c->currentCustIds.size()) {
          UIComponents::showWarningDialog(
              win, "Thiếu thông tin",
              "Vui lòng chọn khách hàng bảo hành hợp lệ!");
          return;
        }
        std::string targetCustomerId = c->currentCustIds[selCust];

        guint selDev =
            gtk_drop_down_get_selected(GTK_DROP_DOWN(c->comboDevice));
        if (selDev >= c->currentEligibleItems.size()) {
          UIComponents::showWarningDialog(win, "Thiếu thông tin",
                                          "Vui lòng chọn thiết bị bảo hành!");
          return;
        }

        const auto &selectedItem = c->currentEligibleItems[selDev];
        if (!selectedItem.active || selectedItem.remainingDays < 0) {
          UIComponents::showWarningDialog(
              win, "Không thể gửi yêu cầu",
              "Thiết bị này đã hết hạn bảo hành! Không thể tạo yêu cầu.");
          return;
        }
        if (selectedItem.openRequests >= selectedItem.quantity) {
          UIComponents::showWarningDialog(
              win, "Không thể gửi yêu cầu",
              "Thiết bị này đã đạt giới hạn yêu cầu bảo hành đang mở tối đa!");
          return;
        }

        const char *rawIssue =
            gtk_editable_get_text(GTK_EDITABLE(c->entryIssue));
        std::string issue = rawIssue ? rawIssue : "";
        size_t first = issue.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
          UIComponents::showWarningDialog(
              win, "Thiếu thông tin",
              "Vui lòng nhập mô tả chi tiết lỗi gặp phải trên thiết bị!");
          return;
        }
        std::string cleanIssue = issue.substr(first);
        size_t last = cleanIssue.find_last_not_of(" \t\r\n");
        if (last != std::string::npos) {
          cleanIssue = cleanIssue.substr(0, last + 1);
        }

        // Khóa nút để tránh bấm đúp
        gtk_widget_set_sensitive(GTK_WIDGET(b), FALSE);

        CreateWarrantyRequest req;
        req.customerId = targetCustomerId;
        req.orderId = selectedItem.orderId;
        req.phoneId = selectedItem.phoneId;
        req.issueDescription = cleanIssue;

        OperationResult res = c->screens->controller->createWarranty(req);
        if (res.success) {
          c->screens->refreshWarrantyIntake();
          if (c->screens->shell) {
            c->screens->shell->popPage();
          }
          UIComponents::showSuccessDialog(
              c->screens->shell ? c->screens->shell->getWindow() : nullptr,
              "Đã tiếp nhận yêu cầu", res.message);
        } else {
          gtk_widget_set_sensitive(GTK_WIDGET(b), TRUE);
          UIComponents::showErrorDialog(win, "Tiếp nhận thất bại", res.message);
        }
      }),
      modalCtx, nullptr, (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSubmit);
  gtk_box_append(GTK_BOX(card), btnRow);

  UIComponents::setupDialogShortcuts(card, btnSubmit, btnCancel);

  gtk_box_append(GTK_BOX(pageContent), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("sales_create_warranty", scroll,
                  "Tiếp nhận bảo hành hộ khách");
  gtk_widget_grab_focus(entryCustSearch);
}

void SalesScreens::focusPosSearch() {
  if (posBtnAddProd && GTK_IS_WIDGET(posBtnAddProd)) {
    gtk_widget_grab_focus(posBtnAddProd);
  } else if (posEntrySearch && GTK_IS_WIDGET(posEntrySearch)) {
    gtk_widget_grab_focus(posEntrySearch);
  }
}
