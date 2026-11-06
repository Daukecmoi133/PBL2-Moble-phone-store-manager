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
#include "../../source/utils/PathHelper.h"
#include "../../source/utils/StringHelper.h"
#include "../../source/utils/Utf8Helper.h"
#include "../components/UIComponents.h"
#include "../styles/AppCss.h"
#include "MainShellWindow.h"


namespace {
const std::vector<UIComponents::TableColumnSpec> kPosCartCols = {
    {"Sản phẩm & Mã máy", 200, 0.0f, true},
    {"Đơn giá", 140, 1.0f, false},
    {"Số lượng", 145, 0.5f, false},
    {"Thành tiền", 150, 1.0f, false},
    {"Xóa", 90, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesLookupCols = {
    {"Tên thiết bị & Cấu hình", 260, 0.0f, true},
    {"Trạng thái", 130, 0.5f, false},
    {"Tồn khả dụng", 120, 1.0f, false},
    {"Giá niêm yết", 140, 1.0f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesCustCols = {
    {"Mã KH", 110, 0.0f, false},
    {"Họ và tên", 180, 0.0f, false},
    {"Thông tin liên hệ (SĐT / Email / Địa chỉ)", 240, 0.0f, true},
    {"Hạng TV", 130, 0.5f, false},
    {"Điểm tích lũy", 110, 1.0f, false},
    {"Thao tác", 96, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesInvoiceCols = {
    {"Mã HĐ", 120, 0.0f, false},     {"Mã đơn", 120, 0.0f, false},
    {"Khách hàng", 180, 0.0f, true}, {"Hình thức TT", 140, 0.5f, false},
    {"Thời gian", 145, 0.0f, false}, {"Tổng tiền", 140, 1.0f, false},
    {"Thao tác", 96, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesOrderCols = {
    {"Mã đơn", 120, 0.0f, false},     {"Khách hàng / SĐT", 180, 0.0f, true},
    {"Giao nhận", 130, 0.0f, false},  {"Thời gian", 140, 0.0f, false},
    {"Trạng thái", 130, 0.5f, false}, {"Thanh toán", 130, 0.5f, false},
    {"Tổng tiền", 140, 1.0f, false},  {"Xử lý đơn", 160, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSalesWarrantyCols = {
    {"Mã phiếu", 120, 0.0f, false},   {"Thiết bị", 180, 0.0f, false},
    {"Khách hàng", 170, 0.0f, false}, {"Ngày nhận", 130, 0.0f, false},
    {"Trạng thái", 140, 0.5f, false}, {"Mô tả sự cố", 220, 0.0f, true}};
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
    todayRev += inv.total;
  }

  GtkWidget *c1 = UIComponents::createStatCard(
      "Doanh thu bán hàng", UIComponents::formatVND(todayRev),
      "Tổng " + UIComponents::formatNumber(invoices.size()) +
          " hóa đơn hợp lệ đã xuất",
      "💰", "green");
  GtkWidget *c2 = UIComponents::createStatCard(
      "Đơn online chờ duyệt",
      UIComponents::formatNumber(pendingOrders) + " đơn",
      pendingOrders > 0 ? "Cần xác nhận tiếp nhận ngay"
                        : "Đã xử lý hết đơn chờ",
      "⏳", pendingOrders > 0 ? "amber" : "blue");
  GtkWidget *c3 = UIComponents::createStatCard(
      "Đơn đang giao / Chờ nhận",
      UIComponents::formatNumber(activeDeliveryOrders) + " đơn",
      "Sẵn sàng hoàn tất giao khách", "🚚",
      activeDeliveryOrders > 0 ? "cyan" : "blue");
  GtkWidget *c4 = UIComponents::createStatCard(
      "Khách hàng hệ thống",
      UIComponents::formatNumber(custs.size()) + " khách",
      "Tra cứu SĐT & tích điểm tại quầy", "👥", "purple");

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
      "Tạo đơn hàng, thêm sản phẩm nhanh bằng bàn phím và xuất hóa đơn trực "
      "tiếp cho khách.");
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

  GtkWidget *btnAddProd = gtk_button_new_with_label("➕ Chọn sản phẩm (F2)");
  gtk_widget_add_css_class(btnAddProd, "btn-primary");
  UIComponents::setButtonCursor(btnAddProd);
  g_signal_connect_swapped(
      btnAddProd, "clicked",
      G_CALLBACK(+[](SalesScreens *s) { s->showPosProductPickerModal(); }),
      this);
  gtk_box_append(GTK_BOX(topBar), btnAddProd);

  // Ô tìm kiếm / quét mã: luôn được focus, Enter = thêm sản phẩm
  posEntrySearch = gtk_entry_new();
  gtk_widget_add_css_class(posEntrySearch, "pos-search");
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(posEntrySearch),
      "🔍 Quét mã hoặc nhập tên / mã / hãng sản phẩm (Enter để thêm)...");

  // Popover danh sách gợi ý khi gõ từ khóa
  posSearchPopover = gtk_popover_new();
  gtk_widget_set_parent(posSearchPopover, posEntrySearch);
  gtk_popover_set_position(GTK_POPOVER(posSearchPopover), GTK_POS_BOTTOM);
  gtk_popover_set_autohide(GTK_POPOVER(posSearchPopover), TRUE);

  GtkWidget *popoverCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_SM);
  gtk_widget_set_size_request(popoverCard, 520, -1);
  gtk_widget_set_margin_top(popoverCard, DesignTokens::SPACE_SM);
  gtk_widget_set_margin_bottom(popoverCard, DesignTokens::SPACE_SM);
  gtk_widget_set_margin_start(popoverCard, DesignTokens::SPACE_SM);
  gtk_widget_set_margin_end(popoverCard, DesignTokens::SPACE_SM);

  posSearchStatusLbl = gtk_label_new("Gợi ý sản phẩm:");
  gtk_widget_add_css_class(posSearchStatusLbl, "card-title");
  gtk_label_set_xalign(GTK_LABEL(posSearchStatusLbl), 0.0f);
  gtk_box_append(GTK_BOX(popoverCard), posSearchStatusLbl);

  GtkWidget *scrollSearch = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollSearch),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scrollSearch),
                                             260);
  gtk_scrolled_window_set_propagate_natural_height(
      GTK_SCROLLED_WINDOW(scrollSearch), TRUE);

  posSearchListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollSearch),
                                posSearchListBox);
  gtk_box_append(GTK_BOX(popoverCard), scrollSearch);

  gtk_popover_set_child(GTK_POPOVER(posSearchPopover), popoverCard);

  // Khi gõ từ khóa: Cập nhật danh sách gợi ý trực tiếp (KHÔNG thay đổi giỏ
  // hàng)
  g_signal_connect_data(posEntrySearch, "changed",
                        G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                          SalesScreens *self = static_cast<SalesScreens *>(d);
                          const char *txt = gtk_editable_get_text(e);
                          self->updatePosSearchSuggestions(txt ? txt : "");
                        }),
                        this, nullptr, (GConnectFlags)0);

  // Khi nhấn Enter / Quét Barcode hoàn tất:
  g_signal_connect_data(
      posEntrySearch, "activate", G_CALLBACK(+[](GtkEntry *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        std::string key =
            StringHelper::trim(gtk_editable_get_text(GTK_EDITABLE(e)));
        if (key.empty())
          return;

        std::string lk = Utf8Helper::toLower(key);
        std::vector<ProductItemViewModel> allProds =
            self->controller->getProducts();

        // 1. Ưu tiên khớp chính xác Product ID (Quét mã vạch hoặc gõ đúng mã)
        for (const auto &p : allProds) {
          if (Utf8Helper::toLower(p.id) == lk) {
            if (p.availableStock <= 0) {
              UIComponents::showWarningDialog(
                  self->shell ? self->shell->getWindow() : nullptr, "Hết hàng",
                  "Sản phẩm [" + p.name + "] đã hết hàng khả dụng!");
              return;
            }
            self->addProductToPosCart(p);
            gtk_popover_popdown(GTK_POPOVER(self->posSearchPopover));
            gtk_editable_set_text(GTK_EDITABLE(e), "");
            gtk_widget_grab_focus(GTK_WIDGET(e));
            return;
          }
        }

        // 2. Tìm kiếm danh sách các sản phẩm khớp với từ khóa
        std::vector<ProductItemViewModel> matches;
        for (const auto &p : allProds) {
          if (Utf8Helper::containsCaseInsensitive(p.name, key) ||
              Utf8Helper::containsCaseInsensitive(p.brand, key) ||
              Utf8Helper::containsCaseInsensitive(p.id, key)) {
            matches.push_back(p);
          }
        }

        if (matches.empty()) {
          gtk_popover_popdown(GTK_POPOVER(self->posSearchPopover));
          UIComponents::showWarningDialog(
              self->shell ? self->shell->getWindow() : nullptr,
              "Không tìm thấy",
              "Không có sản phẩm nào khớp với từ khóa: \"" + key + "\"");
          return;
        }

        // 3. Nếu chỉ có duy nhất 1 sản phẩm khớp rõ ràng -> Thêm nhanh
        if (matches.size() == 1) {
          const auto &p = matches[0];
          if (p.availableStock <= 0) {
            UIComponents::showWarningDialog(
                self->shell ? self->shell->getWindow() : nullptr, "Hết hàng",
                "Sản phẩm [" + p.name + "] đã hết hàng khả dụng!");
            return;
          }
          self->addProductToPosCart(p);
          gtk_popover_popdown(GTK_POPOVER(self->posSearchPopover));
          gtk_editable_set_text(GTK_EDITABLE(e), "");
          gtk_widget_grab_focus(GTK_WIDGET(e));
          return;
        }

        // 4. Nếu có nhiều kết quả gần giống nhau -> Hiển thị popover để nhân
        // viên lựa chọn, KHÔNG tự động thêm nhầm sản phẩm đầu tiên!
        self->updatePosSearchSuggestions(key);
        if (self->posSearchStatusLbl) {
          std::string alertText =
              "⚠️ Có " + std::to_string(matches.size()) +
              " sản phẩm khớp. Vui lòng bấm chọn sản phẩm chính xác:";
          gtk_label_set_text(GTK_LABEL(self->posSearchStatusLbl),
                             alertText.c_str());
        }
      }),
      this, nullptr, (GConnectFlags)0);
  g_signal_connect(
      posEntrySearch, "map",
      G_CALLBACK(+[](GtkWidget *w, gpointer) { gtk_widget_grab_focus(w); }),
      nullptr);

  gtk_box_append(GTK_BOX(leftCard), topBar);
  gtk_box_append(GTK_BOX(leftCard), posEntrySearch);

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
      "F9: Thanh toán & Xuất HĐ   •   Esc: Xóa giỏ hàng");
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
          self->controller->validateDiscount(self->posAppliedCoupon, subtotal,
                                             discount);
        }
        double grandTotal = subtotal - discount;
        if (grandTotal < 0)
          grandTotal = 0;
        double change = cashGiven - grandTotal;
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
      gtk_button_new_with_label("✓ Thanh toán & Xuất HĐ (F9)");
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
        if (keyval == GDK_KEY_F2) {
          self->showPosProductPickerModal();
          return TRUE;
        }
        if (keyval == GDK_KEY_F9) {
          g_signal_emit_by_name(self->posBtnPay, "clicked");
          return TRUE;
        }
        if (keyval == GDK_KEY_Escape) {
          // Nếu Popover gợi ý tìm kiếm đang hiển thị: Đóng popover trước
          if (self->posSearchPopover &&
              gtk_widget_get_visible(self->posSearchPopover)) {
            gtk_popover_popdown(GTK_POPOVER(self->posSearchPopover));
            gtk_editable_set_text(GTK_EDITABLE(self->posEntrySearch), "");
            gtk_widget_grab_focus(self->posEntrySearch);
            return TRUE;
          }
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
    controller->validateDiscount(posAppliedCoupon, subtotal, discount);
  }

  double grandTotal = subtotal - discount;
  if (grandTotal < 0)
    grandTotal = 0;

  gtk_label_set_text(
      GTK_LABEL(posLblSubtotal),
      ("Tạm tính: " + UIComponents::formatVND(subtotal)).c_str());
  gtk_label_set_text(
      GTK_LABEL(posLblDiscount),
      ("Giảm giá: -" + UIComponents::formatVND(discount)).c_str());
  gtk_label_set_text(GTK_LABEL(posLblGrandTotal),
                     UIComponents::formatVND(grandTotal).c_str());

  double cashGiven = UIComponents::parseCurrencyInput(posEntryCash);
  double change = cashGiven - grandTotal;
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

bool SalesScreens::addProductToPosCart(const ProductItemViewModel &prod) {
  if (prod.availableStock <= 0) {
    UIComponents::showWarningDialog(
        shell ? shell->getWindow() : nullptr, "Hết hàng",
        "Sản phẩm [" + prod.name + "] đã hết hàng khả dụng!");
    return false;
  }

  bool found = false;
  for (auto &item : posCartItems) {
    if (item.phoneId == prod.id) {
      if (item.quantity < prod.availableStock) {
        item.quantity += 1;
        item.calculateSubtotal();
      } else {
        UIComponents::showWarningDialog(
            shell ? shell->getWindow() : nullptr, "Vượt quá tồn kho khả dụng",
            "Sản phẩm " + prod.name + " chỉ còn tối đa " +
                std::to_string(prod.availableStock) + " máy khả dụng.");
        return false;
      }
      found = true;
      break;
    }
  }

  if (!found) {
    CartItemViewModel ci;
    ci.phoneId = prod.id;
    ci.phoneName = prod.name;
    ci.brand = prod.brand;
    ci.unitPrice = prod.sellingPrice;
    ci.quantity = 1;
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

void SalesScreens::showPosProductPickerModal() {
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win), "Chọn sản phẩm vào hóa đơn POS");
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  if (shell) {
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
  }
  gtk_window_set_default_size(GTK_WINDOW(win), 640, 520);

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_margin_top(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_bottom(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_start(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_end(card, DesignTokens::SPACE_LG);

  GtkWidget *lblTitle = gtk_label_new("📱 Chọn sản phẩm thêm vào giỏ POS");
  gtk_widget_add_css_class(lblTitle, "page-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(card), lblTitle);

  GtkWidget *pickerListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);

  struct PickerFilterCtx {
    SalesScreens *screens;
    GtkWidget *listBox;
    GtkWidget *dialog;
    GtkWidget *countBadge;
    void (*populate)(PickerFilterCtx *, const std::string &);
  };

  GtkWidget *pickerCountBadge = nullptr;
  GtkWidget *searchEntry = nullptr;
  PickerFilterCtx *fctx =
      new PickerFilterCtx{this, pickerListBox, win, nullptr, nullptr};

  auto populatePickerRows = +[](PickerFilterCtx *ctx, const std::string &kw) {
    GtkWidget *child = gtk_widget_get_first_child(ctx->listBox);
    while (child != nullptr) {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      gtk_box_remove(GTK_BOX(ctx->listBox), child);
      child = next;
    }

    ProductFilterRequest req;
    req.keyword = kw;
    std::vector<ProductItemViewModel> prods =
        ctx->screens->controller->getProducts(req);
    UIComponents::updateRecordCountBadge(ctx->countBadge, prods.size(),
                                         "sản phẩm");

    if (prods.empty()) {
      gtk_box_append(GTK_BOX(ctx->listBox),
                     UIComponents::createEmptyState(
                         "🔍", "Không tìm thấy sản phẩm",
                         "Thử nhập từ khóa tên máy hoặc thương hiệu khác."));
      return;
    }

    for (size_t i = 0; i < prods.size(); ++i) {
      const auto &p = prods[i];
      GtkWidget *row = UIComponents::createTableDataRow(i);

      std::string resolvedImg = PathHelper::resolveDataPath(p.imageUrl);
      if (!p.imageUrl.empty() &&
          g_file_test(resolvedImg.c_str(), G_FILE_TEST_EXISTS)) {
        GtkWidget *thumb = gtk_image_new_from_file(resolvedImg.c_str());
        gtk_image_set_pixel_size(GTK_IMAGE(thumb), 36);
        gtk_box_append(GTK_BOX(row), thumb);
      } else {
        GtkWidget *ico = gtk_label_new("📱");
        gtk_widget_set_size_request(ico, 36, 36);
        gtk_box_append(GTK_BOX(row), ico);
      }

      UIComponents::TableColumnSpec nameSpec{"Sản phẩm", 220, 0.0f, true};
      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableTwoLineCell(
                         p.name, "[" + p.brand + "] Mã: " + p.id, nameSpec));

      UIComponents::TableColumnSpec priceSpec{"Giá bán", 130, 1.0f, false};
      gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                       UIComponents::formatVND(p.sellingPrice),
                                       priceSpec, "product-price"));

      UIComponents::TableColumnSpec stockSpec{"Tồn", 90, 1.0f, false};
      std::string stCss =
          (p.availableStock <= 0)
              ? "stock-critical"
              : (p.availableStock <= 5 ? "stock-low" : "stock-ok");
      gtk_box_append(
          GTK_BOX(row),
          UIComponents::createTableTextCell(
              "Còn: " + std::to_string(p.availableStock), stockSpec, stCss));

      GtkWidget *btnAdd = gtk_button_new_with_label(
          p.availableStock > 0 ? "+ Thêm" : "Hết hàng");
      if (p.availableStock <= 0) {
        gtk_widget_set_sensitive(btnAdd, FALSE);
      } else {
        gtk_widget_add_css_class(btnAdd, "btn-primary");
        UIComponents::setButtonCursor(btnAdd);
      }

      struct PickCtx {
        SalesScreens *screens;
        ProductItemViewModel prod;
        GtkWidget *dialog;
      };
      PickCtx *pk = new PickCtx{ctx->screens, p, ctx->dialog};
      g_signal_connect_data(
          btnAdd, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            PickCtx *pdata = static_cast<PickCtx *>(d);
            pdata->screens->addProductToPosCart(pdata->prod);
            gtk_window_destroy(GTK_WINDOW(pdata->dialog));
          }),
          pk, [](gpointer d, GClosure *) { delete static_cast<PickCtx *>(d); },
          (GConnectFlags)0);

      gtk_box_append(GTK_BOX(row), btnAdd);
      gtk_box_append(GTK_BOX(ctx->listBox), row);
    }
  };
  fctx->populate = populatePickerRows;

  GtkWidget *toolbar = UIComponents::createFilterToolbar(
      "Tìm nhanh theo tên, hãng hoặc mã máy...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        PickerFilterCtx *ctx = static_cast<PickerFilterCtx *>(d);
        const char *txt = gtk_editable_get_text(e);
        if (ctx && ctx->populate) {
          ctx->populate(ctx, txt ? txt : "");
        }
      }),
      fctx, &searchEntry, &pickerCountBadge);
  fctx->countBadge = pickerCountBadge;
  g_object_set_data_full(
      G_OBJECT(win), "picker-filter-ctx", fctx,
      [](gpointer d) { delete static_cast<PickerFilterCtx *>(d); });
  gtk_box_append(GTK_BOX(card), toolbar);

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scroll, TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pickerListBox);
  gtk_box_append(GTK_BOX(card), scroll);

  populatePickerRows(fctx, "");

  GtkWidget *btnClose = gtk_button_new_with_label("Đóng (Esc)");
  gtk_widget_add_css_class(btnClose, "btn-secondary");
  UIComponents::setButtonCursor(btnClose);
  gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
  g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy),
                           win);
  gtk_box_append(GTK_BOX(card), btnClose);

  gtk_window_set_child(GTK_WINDOW(win), card);
  UIComponents::setupDialogShortcuts(win, nullptr, btnClose);
  if (searchEntry) {
    gtk_widget_grab_focus(searchEntry);
  }
  gtk_window_present(GTK_WINDOW(win));
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

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên, hãng hoặc mã sản phẩm...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->refreshProductLookup(txt ? txt : "");
      }),
      this, nullptr, &lookupCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kSalesLookupCols));

  productLookupListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), productLookupListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(productLookupView),
                                content);
  refreshProductLookup();
}

void SalesScreens::refreshProductLookup(const std::string &keyword) {
  if (!productLookupListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(productLookupListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(productLookupListBox), child);
    child = next;
  }

  ProductFilterRequest req;
  req.keyword = keyword;
  std::vector<ProductItemViewModel> prods = controller->getProducts(req);

  UIComponents::updateRecordCountBadge(lookupCountBadge, prods.size(),
                                       "sản phẩm");

  if (prods.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "🔍", "Không tìm thấy sản phẩm", "Thử tìm kiếm với từ khóa khác.");
    gtk_box_append(GTK_BOX(productLookupListBox), empty);
    return;
  }

  for (size_t i = 0; i < prods.size(); ++i) {
    const auto &p = prods[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Tên thiết bị & Cấu hình
    std::string subSpec = "[" + p.brand + "] Mã: " + p.id +
                          (p.specs.empty() ? "" : (" • " + p.specs));
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     p.name, subSpec, kSalesLookupCols[0]));

    // Col 1: Trạng thái badge
    GtkWidget *badge = UIComponents::createStockStatusBadge(p.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badge, kSalesLookupCols[1]));

    // Col 2: Tồn khả dụng (căn phải, màu ngữ nghĩa)
    std::string stockCssClass = "stock-ok";
    if (p.status == StockDisplayStatus::OUT_OF_STOCK) {
      stockCssClass = "stock-critical";
    } else if (p.status == StockDisplayStatus::LOW_STOCK) {
      stockCssClass = "stock-low";
    }
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(p.availableStock) + " máy",
                       kSalesLookupCols[2], stockCssClass));

    // Col 3: Giá niêm yết (căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(p.sellingPrice),
                                     kSalesLookupCols[3], "product-price"));

    gtk_box_append(GTK_BOX(productLookupListBox), row);
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

  GtkWidget *btnNew = gtk_button_new_with_label("➕ Thêm khách hàng mới");
  gtk_widget_add_css_class(btnNew, "btn-primary");
  UIComponents::setButtonCursor(btnNew);
  g_signal_connect_swapped(btnNew, "clicked", G_CALLBACK(+[](SalesScreens *s) {
                             s->showCustomerModal(nullptr);
                           }),
                           this);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý thông tin khách hàng",
      "Tìm kiếm, thêm mới và theo dõi lịch sử mua hàng, điểm tích lũy.",
      btnNew);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên, SĐT hoặc mã khách hàng...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->refreshCustomers(txt ? txt : "");
      }),
      this, nullptr, &customerCountBadge);
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
  GtkWidget *child = gtk_widget_get_first_child(customerListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(customerListBox), child);
    child = next;
  }

  std::vector<CustomerItemViewModel> custs = controller->getCustomers(keyword);

  UIComponents::updateRecordCountBadge(customerCountBadge, custs.size(),
                                       "khách hàng");

  if (custs.empty()) {
    std::string emptyMsg =
        keyword.empty()
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
  bool isEdit = (existingCust != nullptr);
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win),
                       isEdit ? "Chỉnh sửa khách hàng" : "Thêm khách hàng mới");
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
  gtk_window_set_default_size(GTK_WINDOW(win), 440, 480);

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_margin_top(card, 16);
  gtk_widget_set_margin_bottom(card, 16);
  gtk_widget_set_margin_start(card, 16);
  gtk_widget_set_margin_end(card, 16);

  GtkWidget *title = gtk_label_new(isEdit ? "Cập nhật thông tin khách hàng"
                                          : "Đăng ký khách hàng mới");
  gtk_widget_add_css_class(title, "page-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

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
  g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy),
                           win);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct CustModalCtx {
    SalesScreens *screens;
    GtkWidget *win;
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
      new CustModalCtx{this,    win,   isEdit,   isEdit ? existingCust->id : "",
                       eFirst,  eMid,  eLast,    ePhone,
                       eEmail,  eAddr, errFirst, errLast,
                       errPhone};

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
          UIComponents::showSuccessDialog(c->screens->shell->getWindow(),
                                          "Thành công", res.message);
          gtk_window_destroy(GTK_WINDOW(c->win));
        } else {
          if (res.message.find("điện thoại") != std::string::npos) {
            UIComponents::setFieldError(c->errPhone, c->ePhone, res.message);
            gtk_widget_grab_focus(c->ePhone);
          } else {
            UIComponents::showErrorDialog(c->screens->shell->getWindow(), "Lỗi",
                                          res.message);
          }
        }
      }),
      cm, [](gpointer d, GClosure *) { delete static_cast<CustModalCtx *>(d); },
      (GConnectFlags)0);

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
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(invoicesView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh sách hóa đơn thanh toán",
      "Tra cứu hóa đơn bán hàng tại quầy và đơn online đã thanh toán.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã hóa đơn, mã đơn hoặc tên khách hàng...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->refreshInvoices(txt ? txt : "");
      }),
      this, nullptr, &invoiceCountBadge);
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
  GtkWidget *child = gtk_widget_get_first_child(invoiceListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(invoiceListBox), child);
    child = next;
  }

  std::vector<InvoiceItemViewModel> invs = controller->getAllInvoices();
  std::vector<InvoiceItemViewModel> filtered;
  for (const auto &inv : invs) {
    if (!keyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(inv.id, keyword) &&
          !Utf8Helper::containsCaseInsensitive(inv.orderId, keyword) &&
          !Utf8Helper::containsCaseInsensitive(inv.customerName, keyword)) {
        continue;
      }
    }
    filtered.push_back(inv);
  }

  UIComponents::updateRecordCountBadge(invoiceCountBadge, filtered.size(),
                                       "hóa đơn");

  if (filtered.empty()) {
    gtk_box_append(GTK_BOX(invoiceListBox),
                   UIComponents::createEmptyState(
                       "🧾", "Chưa có dữ liệu hóa đơn",
                       "Không có hóa đơn nào khớp với từ khóa tìm kiếm."));
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
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win),
                       ("Chi tiết hóa đơn — " + inv.id).c_str());
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  if (shell) {
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
  }
  gtk_window_set_default_size(GTK_WINDOW(win), 480, 420);

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_margin_top(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_bottom(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_start(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_end(card, DesignTokens::SPACE_LG);

  GtkWidget *title = gtk_label_new(("HÓA ĐƠN BÁN HÀNG " + inv.id).c_str());
  gtk_widget_add_css_class(title, "page-title");
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

  for (const auto &d : inv.details) {
    GtkWidget *r =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
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
    gtk_box_append(GTK_BOX(card), r);
  }

  if (inv.discountAmount > 0) {
    addRow("Giảm giá:", "-" + UIComponents::formatVND(inv.discountAmount));
  }
  addRow("Tổng tiền:", UIComponents::formatVND(inv.total));

  GtkWidget *btnClose = gtk_button_new_with_label("Đóng (Esc)");
  gtk_widget_add_css_class(btnClose, "btn-secondary");
  UIComponents::setButtonCursor(btnClose);
  gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
  g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy),
                           win);
  gtk_box_append(GTK_BOX(card), btnClose);

  gtk_window_set_child(GTK_WINDOW(win), card);
  UIComponents::setupDialogShortcuts(win, nullptr, btnClose);
  gtk_window_present(GTK_WINDOW(win));
}

// ================= 6. QUẢN LÝ ĐƠN HÀNG ONLINE =================

void SalesScreens::buildOnlineOrdersView() {
  onlineOrdersView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(onlineOrdersView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý đơn hàng Online",
      "Tiếp nhận đơn, xác nhận đơn hàng và chuyển trạng thái xử lý.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã đơn, tên khách hàng hoặc SĐT người nhận...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->onlineOrderKeyword = txt ? txt : "";
        self->refreshOnlineOrders();
      }),
      this, nullptr, &ordersCountBadge);
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
    orders.push_back(ord);
  }

  UIComponents::updateRecordCountBadge(ordersCountBadge, orders.size(),
                                       "đơn hàng");

  if (orders.empty()) {
    gtk_box_append(
        GTK_BOX(ordersListBox),
        UIComponents::createEmptyState(
            "📦", "Chưa có dữ liệu đơn hàng",
            "Không có đơn hàng online nào khớp với bộ lọc hiện tại."));
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
            OperationResult res = a->screens->controller->updateOrderStatus(
                a->orderId, a->targetStatus);
            if (res.success) {
              a->screens->refreshOnlineOrders();
              a->screens->refreshDashboard();
              UIComponents::showSuccessDialog(
                  a->screens->shell ? a->screens->shell->getWindow() : nullptr,
                  "Đã cập nhật", res.message);
            } else {
              UIComponents::showErrorDialog(
                  a->screens->shell ? a->screens->shell->getWindow() : nullptr,
                  "Lỗi", res.message);
            }
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
               ord.status == OrderStatus::READY_FOR_PICKUP) {
      GtkWidget *btnComplete = UIComponents::createTableActionButton(
          "✓ Hoàn tất giao", "Đánh dấu đơn hàng đã được giao thành công",
          "btn-primary");
      OrderActionCtx *act =
          new OrderActionCtx{this, ord.id, OrderStatus::COMPLETED};
      g_signal_connect_data(
          btnComplete, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            OrderActionCtx *a = static_cast<OrderActionCtx *>(d);
            OperationResult res = a->screens->controller->updateOrderStatus(
                a->orderId, a->targetStatus);
            if (res.success) {
              a->screens->refreshOnlineOrders();
              a->screens->refreshDashboard();
              UIComponents::showSuccessDialog(
                  a->screens->shell ? a->screens->shell->getWindow() : nullptr,
                  "Đã hoàn thành", res.message);
            }
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

  GtkWidget *header = UIComponents::createSectionHeader(
      "Tiếp nhận yêu cầu bảo hành",
      "Ghi nhận và theo dõi thiết bị khách hàng mang tới cửa hàng bảo hành.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã phiếu, tên thiết bị hoặc khách hàng...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        SalesScreens *self = static_cast<SalesScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->warrantyKeyword = txt ? txt : "";
        self->refreshWarrantyIntake();
      }),
      this, nullptr, &warrantyCountBadge);
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
          !Utf8Helper::containsCaseInsensitive(w.phoneName, warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.customerName,
                                               warrantyKeyword) &&
          !Utf8Helper::containsCaseInsensitive(w.customerPhone,
                                               warrantyKeyword)) {
        continue;
      }
    }
    list.push_back(w);
  }

  UIComponents::updateRecordCountBadge(warrantyCountBadge, list.size(),
                                       "phiếu bảo hành");

  if (list.empty()) {
    gtk_box_append(GTK_BOX(warrantyListBox),
                   UIComponents::createEmptyState(
                       "🛡️", "Chưa có dữ liệu bảo hành",
                       "Không tìm thấy phiếu bảo hành nào khớp với từ khóa."));
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &w = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã phiếu
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     w.id, kSalesWarrantyCols[0], "cell-code"));

    // Col 1: Thiết bị
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.phoneName, kSalesWarrantyCols[1], "cell-title"));

    // Col 2: Khách hàng
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       w.customerName, w.customerPhone, kSalesWarrantyCols[2]));

    // Col 3: Ngày nhận
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.requestDate, kSalesWarrantyCols[3], "cell-subtitle"));

    // Col 4: Trạng thái badge
    GtkWidget *badge = UIComponents::createWarrantyStatusBadge(w.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badge, kSalesWarrantyCols[4]));

    // Col 5: Mô tả sự cố
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(w.issueDescription,
                                                     kSalesWarrantyCols[5]));

    gtk_box_append(GTK_BOX(warrantyListBox), row);
  }
}

void SalesScreens::focusPosSearch() {
  if (posEntrySearch && GTK_IS_WIDGET(posEntrySearch)) {
    gtk_widget_grab_focus(posEntrySearch);
  }
}
