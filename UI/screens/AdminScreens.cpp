/**
 * @file AdminScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ QUẢN TRỊ VIÊN CẤP CAO (ADMINISTRATOR
 * SUBSYSTEM)
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - Subview Swap Pattern: Chuyển đổi giữa danh sách và màn hình chi tiết/chỉnh
 * sửa toàn phần (showProductDetailView, showCustomerDetailView,
 * showDiscountEditView, showOrderDetailView) tích hợp nút Back Header để quay
 * lại danh sách mượt mà.
 * - Quản lý Sản phẩm: Tìm kiếm, chỉnh sửa, thêm mới có hỗ trợ đường dẫn hình
 * ảnh (image url).
 * - Quản lý Khách hàng: Huy hiệu màu Rank phân biệt theo từng hạng (Bronze,
 * Silver, Gold, Diamond).
 * - Quản lý Voucher: Chỉnh sửa thời gian hiệu lực (startDate, endDate) và đối
 * soát voucher.
 * - Quản lý Đơn hàng: Lọc theo từ khóa, trạng thái xử lý và trạng thái thanh
 * toán.
 * - Thống kê & Báo cáo: Biểu đồ chỉ số, lợi nhuận, sản phẩm bán chạy, tồn kho
 * thấp.
 * - Giả lập Thời gian thực: Màn hình kiểm soát tham số Real-time Clock cho toàn
 * bộ hệ thống.
 * =========================================================================================
 */

#include "AdminScreens.h"
#include "../../source/utils/DateTime.h"
#include "../../source/utils/PathHelper.h"
#include "../../source/utils/StringHelper.h"
#include "../../source/utils/Utf8Helper.h"
#include "../components/ProductDetailView.h"
#include "../components/UIComponents.h"
#include "../mock/RealTimeManager.h"
#include "../styles/AppCss.h"
#include "MainShellWindow.h"
#include <iostream>

namespace {
const std::vector<UIComponents::TableColumnSpec> kProductTableCols = {
    {"Ảnh", 64, 0.5f, false},      {"Tên & Hãng sản phẩm", 220, 0.0f, true},
    {"Tồn kho", 110, 0.5f, false}, {"Giá vốn", 145, 0.5f, false},
    {"Giá bán", 145, 0.5f, false}, {"Trạng thái", 145, 0.5f, false},
    {"Thao tác", 120, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kEmployeeTableCols = {
    {"Mã & Họ tên", 210, 0.0f, true},    {"Tài khoản", 155, 0.0f, false},
    {"Số điện thoại", 130, 0.0f, false}, {"Chức vụ", 175, 0.5f, false},
    {"Mức lương", 140, 0.5f, false},     {"Trạng thái", 140, 0.5f, false},
    {"Thao tác", 195, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kCustomerTableCols = {
    {"Mã & Họ tên", 200, 0.0f, false},     {"Số điện thoại", 130, 0.0f, false},
    {"Địa chỉ", 200, 0.0f, true},          {"Điểm tích lũy", 125, 0.5f, false},
    {"Hạng thành viên", 155, 0.5f, false}, {"Trạng thái", 140, 0.5f, false},
    {"Thao tác", 195, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kSupplierTableCols = {
    {"Công ty cung cấp", 220, 0.0f, false}, {"Mã số thuế", 140, 0.0f, false},
    {"Người liên hệ", 160, 0.0f, false},    {"Số điện thoại", 130, 0.0f, false},
    {"Địa chỉ", 200, 0.0f, true},           {"Thao tác", 175, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kCustomerOrderHistoryCols = {
    {"Mã đơn", 185, 0.0f, false},         {"Thời gian đặt", 160, 0.0f, false},
    {"Sản phẩm đã mua", 240, 0.0f, true}, {"Tổng tiền", 150, 1.0f, false},
    {"Trạng thái đơn", 150, 0.5f, false}, {"Thanh toán", 150, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kCustomerReviewHistoryCols = {
    {"Sản phẩm", 200, 0.0f, false},
    {"Đánh giá", 145, 0.5f, false},
    {"Nội dung nhận xét", 240, 0.0f, true},
    {"Thời gian", 160, 0.0f, false}};

const std::vector<UIComponents::TableColumnSpec> kSupplierReceiptHistoryCols = {
    {"Mã phiếu", 185, 0.0f, false},
    {"Thời gian nhập", 160, 0.0f, false},
    {"Người lập phiếu", 175, 0.0f, false},
    {"Sản phẩm nhập", 240, 0.0f, true},
    {"Tổng tiền nhập", 155, 1.0f, false}};

const std::vector<UIComponents::TableColumnSpec> kDiscountTableCols = {
    {"Mã Voucher", 140, 0.0f, false}, {"Mô tả chương trình", 220, 0.0f, true},
    {"Mức giảm", 125, 0.5f, false},   {"Đơn tối thiểu", 145, 0.5f, false},
    {"Hạn dùng", 120, 0.5f, false},   {"Đã dùng", 115, 0.5f, false},
    {"Trạng thái", 165, 0.5f, false}, {"Thao tác", 120, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kAccountTableCols = {
    {"Tên đăng nhập", 185, 0.0f, false}, {"Họ và tên", 220, 0.0f, true},
    {"Mã liên kết", 145, 0.0f, false},   {"Vai trò", 185, 0.5f, false},
    {"Trạng thái", 160, 0.5f, false},    {"Thao tác", 130, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kAdminOrderTableCols = {
    {"Mã đơn", 190, 0.0f, false},         {"Khách hàng / SĐT", 190, 0.0f, true},
    {"Thời gian", 165, 0.0f, false},      {"Giao nhận", 155, 0.0f, false},
    {"Trạng thái đơn", 155, 0.5f, false}, {"Thanh toán", 155, 0.5f, false},
    {"Tổng tiền", 150, 0.5f, false},      {"Thao tác", 120, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kOrderDetailItemsCols = {
    {"Tên mặt hàng", 200, 0.0f, true},
    {"Số lượng", 120, 0.5f, false},
    {"Đơn giá", 150, 0.5f, false},
    {"Thành tiền", 160, 1.0f, false}};

const std::vector<UIComponents::TableColumnSpec> kAdminInvoiceTableCols = {
    {"Mã hóa đơn", 190, 0.0f, false},  {"Mã đơn hàng", 190, 0.0f, false},
    {"Khách hàng", 180, 0.0f, true},   {"Thu ngân", 165, 0.0f, false},
    {"Phương thức", 150, 0.5f, false}, {"Thời gian", 145, 0.0f, false},
    {"Tổng tiền", 155, 1.0f, false}};

const std::vector<UIComponents::TableColumnSpec> kAdminWarrantyTableCols = {
    {"Mã phiếu", 195, 0.0f, false},   {"Thiết bị", 210, 0.0f, false},
    {"Khách hàng", 185, 0.0f, false}, {"Ngày hẹn", 165, 0.0f, false},
    {"Trạng thái", 155, 0.5f, false}, {"Mô tả sự cố", 200, 0.0f, true}};

const std::vector<UIComponents::TableColumnSpec> kReportTopProdCols = {
    {"Tên sản phẩm", 220, 0.0f, true},
    {"Hãng", 120, 0.0f, false},
    {"Đã bán", 110, 1.0f, false},
    {"Doanh thu", 155, 1.0f, false},
    {"Lợi nhuận", 155, 1.0f, false}};

void confirmAndExitForm(MainShellWindow *shell,
                        const std::string &customMsg = "") {
  if (!shell)
    return;
  std::string msg =
      customMsg.empty()
          ? "Bạn có chắc muốn hủy bỏ thao tác? Các thay đổi chưa lưu sẽ không "
            "được ghi lại."
          : customMsg;
  UIComponents::showConfirmDialog(
      shell->getWindow(), "Xác nhận thoát", msg,
      +[](gpointer data) {
        MainShellWindow *sh = static_cast<MainShellWindow *>(data);
        if (sh) {
          sh->popPage();
        }
      },
      shell);
}

bool isAccountActiveForEntity(AppController *ctrl, const std::string &username,
                              const std::string &entityId) {
  if (!ctrl)
    return true;
  auto accounts = ctrl->getAccounts();
  if (!username.empty()) {
    for (const auto &a : accounts) {
      if (a.username == username)
        return a.isActive;
    }
  }
  if (!entityId.empty()) {
    for (const auto &a : accounts) {
      if (a.linkedEntityId == entityId)
        return a.isActive;
    }
  }
  return true;
}

std::string resolveAccountUsername(AppController *ctrl,
                                   const std::string &username,
                                   const std::string &entityId) {
  if (!username.empty())
    return username;
  if (!ctrl || entityId.empty())
    return "";
  auto accounts = ctrl->getAccounts();
  for (const auto &a : accounts) {
    if (a.linkedEntityId == entityId)
      return a.username;
  }
  return "";
}
} // namespace

struct AdminNavData {
  MainShellWindow *sh;
  const char *tgt;
};

AdminScreens::AdminScreens(AppController *ctrl, MainShellWindow *parentShell)
    : controller(ctrl), shell(parentShell), productContainerBox(nullptr),
      customerContainerBox(nullptr), discountContainerBox(nullptr),
      orderContainerBox(nullptr), productListBox(nullptr),
      productCountBadge(nullptr), employeeListBox(nullptr),
      customerListBox(nullptr), supplierListBox(nullptr),
      discountListBox(nullptr), accountListBox(nullptr), ordersListBox(nullptr),
      invoiceListBox(nullptr), warrantyListBox(nullptr),
      reportsListBox(nullptr), realTimeReconcileBox(nullptr),
      orderFilterKeyword(""), orderFilterStatus(-1), orderFilterPayment(-1) {

  // Khởi tạo cố định các ScrolledWindow gắn vào GtkStack của Shell
  dashboardView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  productMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  employeeMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(employeeMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  customerMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(customerMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  supplierMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(supplierMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  discountMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(discountMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  accountMgmtView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(accountMgmtView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  allOrdersView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(allOrdersView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  allInvoicesView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(allInvoicesView), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  warrantyMonitorView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyMonitorView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  reportsView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(reportsView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  realTimeView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(realTimeView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  adminProfileView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(adminProfileView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  buildDashboardView();
  buildProductMgmtView();
  buildEmployeeMgmtView();
  buildCustomerMgmtView();
  buildSupplierMgmtView();
  buildDiscountMgmtView();
  buildAccountMgmtView();
  buildAllOrdersView();
  buildAllInvoicesView();
  buildWarrantyMonitorView();
  buildReportsView();
  buildRealTimeView();
  buildAdminProfileView();
}

void AdminScreens::refreshAll() {
  refreshDashboard();
  if (!productListBox)
    showProductListView();
  else
    refreshProducts();
  refreshEmployees();
  if (!customerListBox)
    showCustomerListView();
  else
    refreshCustomers();
  refreshSuppliers();
  if (!discountListBox)
    showDiscountListView();
  else
    refreshDiscounts();
  refreshAccounts();
  if (!ordersListBox)
    showOrderListView();
  else
    refreshOrders();
  refreshInvoices();
  refreshWarrantyMonitor();
  refreshReports();
  refreshRealTime();
  refreshProfile();
  if (shell) {
    shell->updateSimulatedTimeBadge();
  }
}

// ================= 1. ADMIN DASHBOARD =================

void AdminScreens::buildDashboardView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  const UserSessionViewModel &user = controller->getCurrentUser();
  std::string title = "Trung tâm Quản trị Hệ thống — " + user.fullName;
  GtkWidget *header = UIComponents::createSectionHeader(
      title,
      "Tổng quan hoạt động kinh doanh và chỉ số vận hành toàn hệ thống.");
  gtk_box_append(GTK_BOX(content), header);

  // KPI Cards Grid (Primary Financial & Operational KPIs)
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

  DashboardStatsViewModel stats = controller->getDashboardStats();

  GtkWidget *c1 = UIComponents::createStatCard(
      "Doanh thu thực tế", UIComponents::formatVND(stats.totalRevenue),
      "Hoàn tất: " + UIComponents::formatNumber(stats.completedOrders) + " / " +
          UIComponents::formatNumber(stats.totalOrders) + " đơn",
      "💰", "green");
  GtkWidget *c2 = UIComponents::createStatCard(
      "Lợi nhuận gộp ước tính", UIComponents::formatVND(stats.estimatedProfit),
      "Doanh thu trừ giá vốn hàng bán", "📈", "green");
  GtkWidget *c3 = UIComponents::createStatCard(
      "Tổng số đơn hàng",
      UIComponents::formatNumber(stats.totalOrders) + " đơn",
      "Chờ xử lý: " + std::to_string(stats.pendingOrders) +
          " • Đang giao: " + std::to_string(stats.shippingOrders),
      "📦", stats.pendingOrders > 0 ? "amber" : "blue");
  GtkWidget *c4 = UIComponents::createStatCard(
      "Giá trị tồn kho", UIComponents::formatVND(stats.totalInventoryValue),
      "Tổng số máy: " + UIComponents::formatNumber(stats.totalStockQuantity) +
          " máy",
      "🏢", "purple");

  gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c4, 3, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), grid);

  // Thanh tỷ lệ trạng thái đơn hàng (Order Pipeline Segmented Bar - DESIGN.md T17)
  GtkWidget *pipelineBar = UIComponents::createOrderStatusPipelineBar(
      stats.completedOrders, stats.shippingOrders, stats.pendingOrders, stats.cancelledOrders);
  gtk_box_append(GTK_BOX(content), pipelineBar);

  // Secondary row of metrics (Catalog, CRM, HR & System Clock)
  GtkWidget *grid2 = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid2), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(grid2), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid2), TRUE);

  GtkWidget *c5 = UIComponents::createStatCard(
      "Số mặt hàng điện thoại",
      UIComponents::formatNumber(stats.totalProducts) + " mẫu",
      stats.lowStockCount > 0 ? ("Cảnh báo sắp hết: " +
                                 std::to_string(stats.lowStockCount) + " mẫu")
                              : "Tồn kho ổn định",
      "📱", stats.lowStockCount > 0 ? "red" : "blue");
  GtkWidget *c6 = UIComponents::createStatCard(
      "Khách hàng đăng ký",
      UIComponents::formatNumber(stats.totalCustomers) + " khách",
      "Đã tích điểm thành viên", "👥", "blue");
  GtkWidget *c7 = UIComponents::createStatCard(
      "Đội ngũ nhân sự",
      UIComponents::formatNumber(stats.totalEmployees) + " nhân sự",
      "Quản trị, Bán hàng & Kho", "💼", "cyan");
  GtkWidget *c8 = UIComponents::createStatCard(
      "Thời gian thực hệ thống", controller->getSimulatedDate(),
      "Mốc thời gian mô phỏng", "⏱️", "blue");

  gtk_grid_attach(GTK_GRID(grid2), c5, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), c6, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), c7, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), c8, 3, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), grid2);

  // Quick Management Shortcuts
  GtkWidget *cardShortcuts =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(cardShortcuts, "card");

  GtkWidget *lblS = gtk_label_new("🚀 Điều hướng quản trị nhanh");
  gtk_widget_add_css_class(lblS, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblS), 0.0f);
  gtk_box_append(GTK_BOX(cardShortcuts), lblS);

  GtkWidget *rowBtns = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(rowBtns), DesignTokens::SPACE_MD);
  gtk_grid_set_column_spacing(GTK_GRID(rowBtns), DesignTokens::SPACE_MD);
  gtk_grid_set_column_homogeneous(GTK_GRID(rowBtns), TRUE);

  int navIdx = 0;
  auto addNavBtn = [this, rowBtns, &navIdx](const char *label,
                                            const char *target) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_add_css_class(btn, "btn-outline");
    UIComponents::setButtonCursor(btn);
    AdminNavData *nd = new AdminNavData{shell, target};
    g_signal_connect_data(
        btn, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          AdminNavData *data = static_cast<AdminNavData *>(d);
          data->sh->navigateTo(data->tgt);
        }),
        nd,
        [](gpointer d, GClosure *) { delete static_cast<AdminNavData *>(d); },
        (GConnectFlags)0);
    gtk_grid_attach(GTK_GRID(rowBtns), btn, navIdx % 3, navIdx / 3, 1, 1);
    navIdx++;
  };

  addNavBtn("📱 Quản lý Sản phẩm", "admin_products");
  addNavBtn("👥 Quản lý Nhân sự", "admin_employees");
  addNavBtn("👤 Quản lý Khách hàng", "admin_customers");
  addNavBtn("🎟️ Mã giảm giá", "admin_discounts");
  addNavBtn("⏱️ Thời gian thực", "admin_realtime");
  addNavBtn("📈 Thống kê & Lợi nhuận", "admin_reports");

  gtk_box_append(GTK_BOX(cardShortcuts), rowBtns);
  gtk_box_append(GTK_BOX(content), cardShortcuts);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(dashboardView), content);
}

void AdminScreens::refreshDashboard() { buildDashboardView(); }

// ================= 2. QUẢN LÝ SẢN PHẨM =================

void AdminScreens::buildProductMgmtView() {
  productContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(productMgmtView),
                                productContainerBox);
  showProductListView();
}

void AdminScreens::showProductListView() {
  productListBox = nullptr;
  productCountBadge = nullptr;

  // Clear container
  GtkWidget *child = gtk_widget_get_first_child(productContainerBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(productContainerBox), child);
    child = next;
  }

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnAdd = gtk_button_new_with_label("➕ Thêm sản phẩm mới");
  gtk_widget_add_css_class(btnAdd, "btn-primary");
  UIComponents::setButtonCursor(btnAdd);
  g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens *s) {
                             ProductItemViewModel newProd;
                             newProd.id = "PHN-NEW";
                             s->showProductDetailView(newProd, true);
                           }),
                           this);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý danh mục sản phẩm",
      "Danh mục thiết bị di động, tình trạng kho và chính sách giá.", btnAdd);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên, hãng hoặc mã sản phẩm...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->refreshProducts(txt ? txt : "");
      }),
      this, nullptr, &productCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kProductTableCols));

  productListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), productListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_box_append(GTK_BOX(productContainerBox), content);
  refreshProducts();
}

void AdminScreens::refreshProducts(const std::string &keyword) {
  if (!productListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(productListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(productListBox), child);
    child = next;
  }

  ProductFilterRequest filter;
  filter.keyword = keyword;
  std::vector<ProductItemViewModel> products = controller->getProducts(filter);

  UIComponents::updateRecordCountBadge(productCountBadge, products.size(),
                                       "sản phẩm");

  if (products.empty()) {
    std::string emptyMsg =
        keyword.empty()
            ? "Không tìm thấy sản phẩm nào trong hệ thống."
            : "Không tìm thấy sản phẩm nào khớp với từ khóa tìm kiếm.";
    GtkWidget *emptyWidget = UIComponents::createEmptyState(
        "📱", "Chưa có sản phẩm", emptyMsg,
        keyword.empty() ? "➕ Thêm sản phẩm đầu tiên" : "",
        G_CALLBACK(+[](GtkButton *, gpointer d) {
          AdminScreens *s = static_cast<AdminScreens *>(d);
          ProductItemViewModel newProd;
          newProd.id = "PHN-NEW";
          s->showProductDetailView(newProd, true);
        }),
        this);
    gtk_box_append(GTK_BOX(productListBox), emptyWidget);
    return;
  }

  for (size_t i = 0; i < products.size(); ++i) {
    const auto &p = products[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Ảnh (Thumbnail preview)
    GtkWidget *imgThumb = nullptr;
    std::string resolvedImg = PathHelper::resolveDataPath(p.imageUrl);
    if (!p.imageUrl.empty() &&
        g_file_test(resolvedImg.c_str(), G_FILE_TEST_EXISTS)) {
      imgThumb = gtk_image_new_from_file(resolvedImg.c_str());
      gtk_image_set_pixel_size(GTK_IMAGE(imgThumb), 48);
    } else {
      imgThumb = gtk_label_new("📱");
      gtk_widget_set_size_request(imgThumb, 48, 48);
    }
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     imgThumb, kProductTableCols[0]));

    // Col 1: Tên & Hãng sản phẩm
    std::string sub =
        "[" + p.brand + "] Mã: " + p.id + (p.os.empty() ? "" : (" • " + p.os));
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     p.name, sub, kProductTableCols[1]));

    // Col 2: Tồn kho (Căn phải, màu ngữ nghĩa theo trạng thái tồn kho)
    std::string stockCssClass = "stock-ok";
    if (p.status == StockDisplayStatus::OUT_OF_STOCK) {
      stockCssClass = "stock-critical";
    } else if (p.status == StockDisplayStatus::LOW_STOCK) {
      stockCssClass = "stock-low";
    }
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(p.actualStock) + " máy",
                       kProductTableCols[2], stockCssClass));

    // Col 3: Giá vốn (Căn phải, tabular numerals)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(p.importPrice),
                                     kProductTableCols[3], "cell-number"));

    // Col 4: Giá bán (Căn phải, màu nổi bật, tabular numerals)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(p.sellingPrice),
                                     kProductTableCols[4], "product-price"));

    // Col 5: Trạng thái kho (Huy hiệu màu)
    GtkWidget *badgeStock = UIComponents::createStockStatusBadge(p.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badgeStock, kProductTableCols[5]));

    // Col 6: Thao tác: Nút nhỏ gọn kèm tooltip
    GtkWidget *btnDetail = UIComponents::createTableActionButton(
        "🔍 Sửa", "Xem chi tiết và chỉnh sửa thông tin sản phẩm",
        "btn-action-edit");
    gtk_widget_set_hexpand(btnDetail, TRUE);

    struct ProdClickData {
      AdminScreens *s;
      ProductItemViewModel p;
    };
    ProdClickData *pcd = new ProdClickData{this, p};
    g_signal_connect_data(
        btnDetail, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          ProdClickData *data = static_cast<ProdClickData *>(d);
          data->s->showProductDetailView(data->p, false);
        }),
        pcd,
        [](gpointer d, GClosure *) { delete static_cast<ProdClickData *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnDetail, kProductTableCols[6]));
    gtk_box_append(GTK_BOX(productListBox), row);
  }
}

void AdminScreens::showProductDetailView(const ProductItemViewModel &prod,
                                         bool isNew) {
  if (!shell)
    return;

  ProductItemViewModel workingProd = prod;
  if (isNew) {
    workingProd = ProductItemViewModel();
    workingProd.id = "PHN-NEW";
    workingProd.name = "";
    workingProd.brand = "";
    workingProd.os = "";
    workingProd.manufacturer = "";
    workingProd.importPrice = 0;
    workingProd.sellingPrice = 0;
    workingProd.listPrice = 0;
    workingProd.actualStock = 0;
    workingProd.reservedStock = 0;
    workingProd.availableStock = 0;
    workingProd.warrantyMonths = 12;
    workingProd.returnDays = 30;
  }

  ProductDetailView::Callbacks callbacks;
  callbacks.onSaved = [this]() {
    refreshProducts();
    if (shell) shell->popPage();
  };
  callbacks.onDeleted = [this]() {
    refreshProducts();
    if (shell) shell->popPage();
  };
  callbacks.onCancelled = [this]() {
    if (shell) shell->popPage();
  };
  callbacks.onBrandSelected = [this](const std::string &brand) {
    refreshProducts(brand);
  };
  callbacks.onNavigateHome = [this]() {
    if (shell) shell->navigateTo("admin_dashboard");
  };

  GtkWidget *detailView = ProductDetailView::create(
      controller, shell, workingProd, ProductDetailMode::AdminEdit, isNew, callbacks);

  std::string pageId = isNew ? "admin_product_add" : "admin_product_edit";
  std::string pageTitle = isNew ? "Thêm mới sản phẩm điện thoại" : ("Chỉnh sửa: " + prod.name);
  shell->pushPage(pageId, detailView, pageTitle, false);
}

// ================= 3. QUẢN LÝ NHÂN SỰ =================

void AdminScreens::buildEmployeeMgmtView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnAdd = gtk_button_new_with_label("➕ Thêm nhân viên");
  gtk_widget_add_css_class(btnAdd, "btn-primary");
  UIComponents::setButtonCursor(btnAdd);
  g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens *s) {
                             s->showEmployeeFormModal(nullptr);
                           }),
                           this);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý đội ngũ nhân viên",
      "Danh sách nhân viên, vị trí công tác và quyền truy cập.", btnAdd);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên, mã hoặc số điện thoại...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->refreshEmployees(txt ? txt : "");
      }),
      this, nullptr, &employeeCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kEmployeeTableCols));

  employeeListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), employeeListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(employeeMgmtView), content);
  refreshEmployees();
}

void AdminScreens::refreshEmployees(const std::string &keyword) {
  if (!employeeListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(employeeListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(employeeListBox), child);
    child = next;
  }

  std::vector<EmployeeItemViewModel> emps = controller->getEmployees(keyword);
  UIComponents::updateRecordCountBadge(employeeCountBadge, emps.size(),
                                       "nhân viên");

  if (emps.empty()) {
    std::string emptyMsg =
        keyword.empty() ? "Không tìm thấy nhân viên nào trong hệ thống."
                        : "Không có nhân viên nào khớp với từ khóa tìm kiếm.";
    GtkWidget *emptyWidget = UIComponents::createEmptyState(
        "👥", "Chưa có nhân viên", emptyMsg,
        keyword.empty() ? "➕ Thêm nhân viên đầu tiên" : "",
        G_CALLBACK(+[](GtkButton *, gpointer d) {
          AdminScreens *s = static_cast<AdminScreens *>(d);
          s->showEmployeeFormModal(nullptr);
        }),
        this);
    gtk_box_append(GTK_BOX(employeeListBox), emptyWidget);
    return;
  }

  for (size_t i = 0; i < emps.size(); ++i) {
    const auto &e = emps[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã & Họ tên
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       e.fullName, "Mã: " + e.id, kEmployeeTableCols[0]));

    // 1: Tài khoản
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       e.username, kEmployeeTableCols[1], "cell-code"));

    // 2: Số điện thoại
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       e.phone, kEmployeeTableCols[2], "tabular-num"));

    // 3: Chức vụ (Huy hiệu)
    GtkWidget *roleBadge = UIComponents::createStatusBadge(
        e.roleName,
        e.role == RoleType::ADMIN
            ? "badge-cancelled"
            : (e.role == RoleType::SALE_EMPLOYEE ? "badge-confirmed"
                                                 : "badge-pending"));
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     roleBadge, kEmployeeTableCols[3]));

    // 4: Mức lương (Căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(e.salary),
                                     kEmployeeTableCols[4], "product-price"));

    // 5: Trạng thái tài khoản (Huy hiệu)
    bool isActive = isAccountActiveForEntity(controller, e.username, e.id);
    GtkWidget *statusBadge = UIComponents::createStatusBadge(
        isActive ? "🟢 Hoạt động" : "🔴 Đã khóa",
        isActive ? "badge-completed" : "badge-cancelled");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     statusBadge, kEmployeeTableCols[5]));

    // 6: Thao tác (Sửa + Khóa/Mở tài khoản)
    GtkWidget *actBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(actBox, GTK_ALIGN_CENTER);

    GtkWidget *btnEdit = UIComponents::createTableActionButton(
        "✏️ Sửa", "Chỉnh sửa thông tin và phân quyền nhân sự",
        "btn-action-edit");
    struct EmpClickData {
      AdminScreens *s;
      EmployeeItemViewModel e;
    };
    EmpClickData *ecd = new EmpClickData{this, e};
    g_signal_connect_data(
        btnEdit, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          EmpClickData *data = static_cast<EmpClickData *>(d);
          data->s->showEmployeeFormModal(&data->e);
        }),
        ecd,
        [](gpointer d, GClosure *) { delete static_cast<EmpClickData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actBox), btnEdit);

    std::string currentLoggedUser = controller->getCurrentUser().username;
    bool isSelf = (e.username == currentLoggedUser);
    GtkWidget *btnLock = UIComponents::createTableActionButton(
        isActive ? "🔒 Khóa" : "🔓 Mở",
        isSelf ? "Không thể tự khóa tài khoản đang đăng nhập"
               : (isActive ? "Khóa tài khoản nhân viên"
                           : "Mở khóa tài khoản nhân viên"),
        isActive ? "btn-action-delete" : "btn-action-view");
    if (isSelf) {
      gtk_widget_set_sensitive(btnLock, FALSE);
    } else {
      struct EmpLockCtx {
        AdminScreens *s;
        std::string username;
        std::string name;
        bool currentlyActive;
      };
      EmpLockCtx *lctx = new EmpLockCtx{this, e.username, e.fullName, isActive};
      g_signal_connect_data(
          btnLock, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
            EmpLockCtx *c = static_cast<EmpLockCtx *>(d);
            std::string actionTitle = c->currentlyActive
                                          ? "Xác nhận khóa tài khoản"
                                          : "Xác nhận mở khóa tài khoản";
            std::string actionMsg =
                c->currentlyActive
                    ? ("Bạn có chắc chắn muốn khóa tài khoản '" + c->username +
                       "' của nhân viên " + c->name +
                       "? Nhân viên này sẽ không thể tiếp tục đăng nhập hệ "
                       "thống.")
                    : ("Bạn có chắc chắn muốn mở khóa tài khoản '" +
                       c->username + "' của nhân viên " + c->name + "?");
            UIComponents::showConfirmDialog(
                c->s->shell ? c->s->shell->getWindow() : nullptr, actionTitle,
                actionMsg,
                +[](gpointer data) {
                  EmpLockCtx *inner = static_cast<EmpLockCtx *>(data);
                  OperationResult res =
                      inner->s->controller->toggleAccountStatus(
                          inner->username);
                  if (res.success) {
                    inner->s->refreshEmployees();
                    UIComponents::showSuccessDialog(
                        inner->s->shell ? inner->s->shell->getWindow()
                                        : nullptr,
                        "Thành công", res.message);
                  } else {
                    UIComponents::showErrorDialog(
                        inner->s->shell ? inner->s->shell->getWindow()
                                        : nullptr,
                        "Lỗi", res.message);
                  }
                },
                c);
          }),
          lctx,
          [](gpointer d, GClosure *) { delete static_cast<EmpLockCtx *>(d); },
          (GConnectFlags)0);
    }
    gtk_box_append(GTK_BOX(actBox), btnLock);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     actBox, kEmployeeTableCols[6]));
    gtk_box_append(GTK_BOX(employeeListBox), row);
  }
}

void AdminScreens::showEmployeeFormModal(
    const EmployeeItemViewModel *existingEmp) {
  if (!shell)
    return;

  bool isEdit = (existingEmp != nullptr);

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

  GtkWidget *title = gtk_label_new(isEdit ? "✏️ Chỉnh sửa thông tin nhân sự"
                                          : "➕ Tạo mới tài khoản nhân viên");
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  GtkWidget *subtitle = gtk_label_new(
      "Thông tin nhân sự, vị trí đảm nhiệm và phân quyền hệ thống.");
  gtk_widget_add_css_class(subtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
  gtk_box_append(GTK_BOX(card), subtitle);

  GtkWidget *eFirst = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eFirst),
                        isEdit ? existingEmp->firstName.c_str() : "");
  GtkWidget *errFirst = nullptr;
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Họ đệm *", eFirst, &errFirst));

  GtkWidget *eLast = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eLast),
                        isEdit ? existingEmp->lastName.c_str() : "");
  GtkWidget *errLast = nullptr;
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Tên *", eLast, &errLast));

  GtkWidget *ePhone = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(ePhone),
                        isEdit ? existingEmp->phone.c_str() : "");
  UIComponents::setupPhoneInput(ePhone);
  GtkWidget *errPhone = nullptr;
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Số điện thoại *", ePhone, &errPhone));

  GtkWidget *eEmail = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eEmail),
                        isEdit ? existingEmp->email.c_str() : "");
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Email", eEmail));

  GtkWidget *eUser = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eUser),
                        isEdit ? existingEmp->username.c_str() : "");
  GtkWidget *errUser = nullptr;
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Tên đăng nhập *",
                                                              eUser, &errUser));

  const char *roles[] = {"Quản trị viên (Admin)", "Nhân viên Bán hàng (Sales)",
                         "Thủ kho (Warehouse)", nullptr};
  GtkWidget *comboRole = gtk_drop_down_new_from_strings(roles);
  if (isEdit) {
    gtk_drop_down_set_selected(
        GTK_DROP_DOWN(comboRole),
        existingEmp->role == RoleType::ADMIN
            ? 0
            : (existingEmp->role == RoleType::SALE_EMPLOYEE ? 1 : 2));
  }
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Chức vụ *", comboRole));

  GtkWidget *eSalary = gtk_entry_new();
  gtk_editable_set_text(
      GTK_EDITABLE(eSalary),
      isEdit
          ? std::to_string(static_cast<long long>(existingEmp->salary)).c_str()
          : "8000000");
  UIComponents::setupCurrencyInput(eSalary);
  GtkWidget *errSalary = nullptr;
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Mức lương (VNĐ) *", eSalary, &errSalary));

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 12);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy bỏ");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked", G_CALLBACK(+[](AdminScreens *s) {
        confirmAndExitForm(s->shell,
                           "Bạn có chắc muốn hủy bỏ thao tác thiết lập thông "
                           "tin nhân sự? Dữ liệu chưa lưu sẽ bị mất.");
      }),
      this);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct EmpCtx {
    AdminScreens *s;
    bool isEdit;
    std::string id;
    GtkWidget *eFirst;
    GtkWidget *eLast;
    GtkWidget *ePhone;
    GtkWidget *eEmail;
    GtkWidget *eUser;
    GtkWidget *cRole;
    GtkWidget *eSalary;
    GtkWidget *errFirst;
    GtkWidget *errLast;
    GtkWidget *errPhone;
    GtkWidget *errUser;
    GtkWidget *errSalary;
  };
  EmpCtx *ctx = new EmpCtx{this,     isEdit,   isEdit ? existingEmp->id : "",
                           eFirst,   eLast,    ePhone,
                           eEmail,   eUser,    comboRole,
                           eSalary,  errFirst, errLast,
                           errPhone, errUser,  errSalary};

  GtkWidget *btnSave = gtk_button_new_with_label("Lưu");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  UIComponents::setButtonCursor(btnSave);
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        EmpCtx *c = static_cast<EmpCtx *>(d);
        UIComponents::clearFieldError(c->errFirst, c->eFirst);
        UIComponents::clearFieldError(c->errLast, c->eLast);
        UIComponents::clearFieldError(c->errPhone, c->ePhone);
        UIComponents::clearFieldError(c->errUser, c->eUser);
        UIComponents::clearFieldError(c->errSalary, c->eSalary);

        EmployeeFormRequest req;
        req.id = c->id;
        req.firstName = gtk_editable_get_text(GTK_EDITABLE(c->eFirst));
        req.lastName = gtk_editable_get_text(GTK_EDITABLE(c->eLast));
        req.phone = gtk_editable_get_text(GTK_EDITABLE(c->ePhone));
        req.email = gtk_editable_get_text(GTK_EDITABLE(c->eEmail));
        req.username = gtk_editable_get_text(GTK_EDITABLE(c->eUser));
        req.password = "123456";

        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->cRole));
        req.role = (sel == 0) ? RoleType::ADMIN
                              : (sel == 1 ? RoleType::SALE_EMPLOYEE
                                          : RoleType::WAREHOUSE_EMPLOYEE);
        req.salary = UIComponents::parseCurrencyInput(c->eSalary);

        bool hasError = false;
        GtkWidget *firstErr = nullptr;
        if (req.firstName.empty()) {
          UIComponents::setFieldError(c->errFirst, c->eFirst,
                                      "Họ đệm không được để trống");
          hasError = true;
          if (!firstErr)
            firstErr = c->eFirst;
        }
        if (req.lastName.empty()) {
          UIComponents::setFieldError(c->errLast, c->eLast,
                                      "Tên không được để trống");
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
        if (req.username.empty()) {
          UIComponents::setFieldError(c->errUser, c->eUser,
                                      "Tên đăng nhập không được để trống");
          hasError = true;
          if (!firstErr)
            firstErr = c->eUser;
        }
        if (req.salary <= 0) {
          UIComponents::setFieldError(c->errSalary, c->eSalary,
                                      "Mức lương phải lớn hơn 0");
          hasError = true;
          if (!firstErr)
            firstErr = c->eSalary;
        }

        if (hasError) {
          if (firstErr)
            gtk_widget_grab_focus(firstErr);
          return;
        }

        OperationResult res = c->isEdit ? c->s->controller->updateEmployee(req)
                                        : c->s->controller->addEmployee(req);
        if (res.success) {
          c->s->refreshEmployees();
          if (c->s->shell) {
            c->s->shell->popPage();
          }
          UIComponents::showSuccessDialog(c->s->shell ? c->s->shell->getWindow()
                                                      : nullptr,
                                          "Thành công", res.message);
        } else {
          if (res.message.find("đăng nhập") != std::string::npos) {
            UIComponents::setFieldError(c->errUser, c->eUser, res.message);
            gtk_widget_grab_focus(c->eUser);
          } else {
            UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow()
                                                      : nullptr,
                                          "Lỗi", res.message);
          }
        }
      }),
      ctx, [](gpointer d, GClosure *) { delete static_cast<EmpCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSave);
  gtk_box_append(GTK_BOX(card), btnRow);

  UIComponents::setupDialogShortcuts(card, btnSave, btnCancel);

  gtk_box_append(GTK_BOX(pageContent), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("admin_employee_form", scroll,
                  isEdit ? ("Chỉnh sửa nhân sự — " + existingEmp->id)
                         : "Thêm nhân sự mới",
                  false);
  gtk_widget_grab_focus(eFirst);
}

// ================= 4. QUẢN LÝ KHÁCH HÀNG (ĐẶC TẢ MÀU RANK) =================

void AdminScreens::buildCustomerMgmtView() {
  customerContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(customerMgmtView),
                                customerContainerBox);
  showCustomerListView();
}

void AdminScreens::showCustomerListView() {
  customerListBox = nullptr;
  customerCountBadge = nullptr;
  GtkWidget *child = gtk_widget_get_first_child(customerContainerBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(customerContainerBox), child);
    child = next;
  }

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý danh sách khách hàng",
      "Danh sách khách hàng, phân hạng thành viên và lịch sử giao dịch.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm khách theo tên, mã hoặc số điện thoại...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->customerFilterKeyword = txt ? txt : "";
        self->refreshCustomers();
      }),
      this, nullptr, &customerCountBadge);

  const char *tiers[] = {"Tất cả các hạng", "Đồng", "Bạc", "Vàng",
                         "Kim Cương (VIP)", nullptr};
  GtkWidget *comboTier = gtk_drop_down_new_from_strings(tiers);
  gtk_widget_add_css_class(comboTier, "filter-dropdown");
  g_signal_connect_data(comboTier, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          AdminScreens *self = static_cast<AdminScreens *>(d);
                          self->customerFilterTier =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshCustomers();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(filterToolbar), comboTier);

  const char *sortOptions[] = {"Mặc định", "Điểm: Cao → Thấp",
                               "Điểm: Thấp → Cao", "Tên: A → Z", nullptr};
  GtkWidget *comboSort = gtk_drop_down_new_from_strings(sortOptions);
  gtk_widget_add_css_class(comboSort, "filter-dropdown");
  g_signal_connect_data(comboSort, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          AdminScreens *self = static_cast<AdminScreens *>(d);
                          self->customerSortBy =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshCustomers();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(filterToolbar), comboSort);

  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kCustomerTableCols));

  customerListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), customerListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_box_append(GTK_BOX(customerContainerBox), content);
  refreshCustomers();
}

void AdminScreens::refreshCustomers(const std::string &keyword) {
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
            ? "Không tìm thấy khách hàng nào trong hệ thống."
            : "Không tìm thấy khách hàng nào khớp với điều kiện lọc.";
    gtk_box_append(
        GTK_BOX(customerListBox),
        UIComponents::createEmptyState("👥", "Chưa có khách hàng", emptyMsg));
    return;
  }

  for (size_t i = 0; i < custs.size(); ++i) {
    const auto &c = custs[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã & Họ tên
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       c.fullName, "Mã: " + c.id, kCustomerTableCols[0]));

    // 1: Số điện thoại
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       c.phone, kCustomerTableCols[1], "tabular-num"));

    // 2: Địa chỉ
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       c.address, kCustomerTableCols[2], "cell-subtitle"));

    // 3: Điểm tích lũy (Căn phải)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(c.points) + " điểm",
                       kCustomerTableCols[3], "cell-number"));

    // 4: Hạng thành viên (Badge)
    GtkWidget *rankBadge = UIComponents::createRankBadge(c.membershipTier);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     rankBadge, kCustomerTableCols[4]));

    // 5: Trạng thái tài khoản (Badge)
    bool isActive = isAccountActiveForEntity(controller, c.username, c.id);
    GtkWidget *statusBadge = UIComponents::createStatusBadge(
        isActive ? "🟢 Hoạt động" : "🔴 Đã khóa",
        isActive ? "badge-completed" : "badge-cancelled");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     statusBadge, kCustomerTableCols[5]));

    // 6: Thao tác (Chi tiết + Khóa/Mở)
    GtkWidget *actBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(actBox, GTK_ALIGN_CENTER);

    GtkWidget *btnDetail = UIComponents::createTableActionButton(
        "🔍 Chi tiết", "Xem thông tin chi tiết và lịch sử mua sắm của khách",
        "btn-action-view");
    struct CustClickData {
      AdminScreens *s;
      CustomerItemViewModel c;
    };
    CustClickData *ccd = new CustClickData{this, c};
    g_signal_connect_data(
        btnDetail, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CustClickData *data = static_cast<CustClickData *>(d);
          data->s->showCustomerDetailView(data->c);
        }),
        ccd,
        [](gpointer d, GClosure *) { delete static_cast<CustClickData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actBox), btnDetail);

    std::string accUser = resolveAccountUsername(controller, c.username, c.id);
    if (!accUser.empty()) {
      GtkWidget *btnLock = UIComponents::createTableActionButton(
          isActive ? "🔒 Khóa" : "🔓 Mở",
          isActive ? "Khóa tài khoản khách hàng"
                   : "Mở khóa tài khoản khách hàng",
          isActive ? "btn-action-delete" : "btn-action-confirmed");
      struct CustLockCtx {
        AdminScreens *s;
        std::string username;
        std::string name;
        bool currentlyActive;
      };
      CustLockCtx *lctx = new CustLockCtx{this, accUser, c.fullName, isActive};
      g_signal_connect_data(
          btnLock, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
            CustLockCtx *ctx = static_cast<CustLockCtx *>(d);
            std::string actionTitle = ctx->currentlyActive
                                          ? "Xác nhận khóa tài khoản"
                                          : "Xác nhận mở khóa tài khoản";
            std::string actionMsg =
                ctx->currentlyActive
                    ? ("Bạn có chắc chắn muốn khóa tài khoản '" +
                       ctx->username + "' của khách hàng " + ctx->name +
                       "? Khách hàng sẽ không thể đăng nhập mua sắm.")
                    : ("Bạn có chắc chắn muốn mở khóa tài khoản '" +
                       ctx->username + "' của khách hàng " + ctx->name + "?");
            UIComponents::showConfirmDialog(
                ctx->s->shell ? ctx->s->shell->getWindow() : nullptr,
                actionTitle, actionMsg,
                +[](gpointer data) {
                  CustLockCtx *inner = static_cast<CustLockCtx *>(data);
                  OperationResult res =
                      inner->s->controller->toggleAccountStatus(
                          inner->username);
                  if (res.success) {
                    inner->s->refreshCustomers();
                    UIComponents::showSuccessDialog(
                        inner->s->shell ? inner->s->shell->getWindow()
                                        : nullptr,
                        "Thành công", res.message);
                  } else {
                    UIComponents::showErrorDialog(
                        inner->s->shell ? inner->s->shell->getWindow()
                                        : nullptr,
                        "Lỗi", res.message);
                  }
                },
                ctx);
          }),
          lctx,
          [](gpointer d, GClosure *) { delete static_cast<CustLockCtx *>(d); },
          (GConnectFlags)0);
      gtk_box_append(GTK_BOX(actBox), btnLock);
    }

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     actBox, kCustomerTableCols[6]));
    gtk_box_append(GTK_BOX(customerListBox), row);
  }
}

void AdminScreens::showCustomerDetailView(const CustomerItemViewModel &cust) {
  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Hồ sơ khách hàng: " + cust.fullName,
      "Hồ sơ hội viên, cập nhật thông tin và lịch sử giao dịch.");
  gtk_box_append(GTK_BOX(content), header);

  // Card 1: Thông tin khách hàng & Chỉnh sửa
  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(card, "card");

  // Rank & Stats Banner
  GtkWidget *banner =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_LG);
  GtkWidget *rankBadge = UIComponents::createRankBadge(cust.membershipTier);
  GtkWidget *lblPts = gtk_label_new(
      ("Tổng điểm thưởng: " + UIComponents::formatNumber(cust.points) + " điểm")
          .c_str());
  gtk_widget_add_css_class(lblPts, "card-title");
  gtk_widget_add_css_class(lblPts, "tabular-num");
  gtk_box_append(GTK_BOX(banner), rankBadge);
  gtk_box_append(GTK_BOX(banner), lblPts);
  gtk_box_append(GTK_BOX(card), banner);

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

  GtkWidget *eName = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eName), cust.fullName.c_str());
  gtk_grid_attach(GTK_GRID(grid),
                  UIComponents::createFormField("Họ và tên *", eName), 0, 0, 2,
                  1);

  GtkWidget *ePhone = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(ePhone), cust.phone.c_str());
  gtk_grid_attach(GTK_GRID(grid),
                  UIComponents::createFormField("Số điện thoại *", ePhone), 2,
                  0, 1, 1);

  GtkWidget *eEmail = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eEmail), cust.email.c_str());
  gtk_grid_attach(GTK_GRID(grid),
                  UIComponents::createFormField("Email", eEmail), 0, 1, 1, 1);

  GtkWidget *eAddr = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eAddr), cust.address.c_str());
  gtk_grid_attach(GTK_GRID(grid),
                  UIComponents::createFormField("Địa chỉ liên hệ", eAddr), 1, 1,
                  2, 1);

  gtk_box_append(GTK_BOX(card), grid);

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy bỏ");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked", G_CALLBACK(+[](AdminScreens *s) {
        confirmAndExitForm(s->shell,
                           "Bạn có chắc muốn hủy chỉnh sửa hồ sơ khách hàng? "
                           "Các thay đổi chưa lưu sẽ bị mất.");
      }),
      this);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct SaveCustCtx {
    AdminScreens *s;
    CustomerItemViewModel cust;
    GtkWidget *eName;
    GtkWidget *ePhone;
    GtkWidget *eEmail;
    GtkWidget *eAddr;
  };
  SaveCustCtx *sctx = new SaveCustCtx{this, cust, eName, ePhone, eEmail, eAddr};

  GtkWidget *btnSave = gtk_button_new_with_label("💾 Lưu thông tin khách hàng");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  UIComponents::setButtonCursor(btnSave);
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        SaveCustCtx *c = static_cast<SaveCustCtx *>(d);
        CustomerFormRequest req;
        req.id = c->cust.id;
        req.lastName = gtk_editable_get_text(GTK_EDITABLE(c->eName));
        req.phone = gtk_editable_get_text(GTK_EDITABLE(c->ePhone));
        req.email = gtk_editable_get_text(GTK_EDITABLE(c->eEmail));
        req.address = gtk_editable_get_text(GTK_EDITABLE(c->eAddr));

        OperationResult res = c->s->controller->updateCustomer(req);
        if (res.success) {
          c->s->refreshCustomers();
          if (c->s->shell)
            c->s->shell->popPage();
          UIComponents::showSuccessDialog(c->s->shell ? c->s->shell->getWindow()
                                                      : nullptr,
                                          "Thành công", res.message);
        } else {
          UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow()
                                                    : nullptr,
                                        "Lỗi", res.message);
        }
      }),
      sctx,
      [](gpointer d, GClosure *) { delete static_cast<SaveCustCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSave);
  gtk_box_append(GTK_BOX(card), btnRow);
  gtk_box_append(GTK_BOX(content), card);

  // Card 2: Lịch sử mua hàng (Gần nhất -> Lâu nhất)
  GtkWidget *orderCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(orderCard, "card");

  GtkWidget *lblOrderTitle =
      gtk_label_new("🛒 Lịch sử mua hàng (Gần nhất → Lâu nhất)");
  gtk_widget_add_css_class(lblOrderTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblOrderTitle), 0.0f);
  gtk_box_append(GTK_BOX(orderCard), lblOrderTitle);

  std::vector<OrderItemViewModel> allOrders = controller->getAllOrders();
  std::vector<OrderItemViewModel> custOrders;
  for (const auto &o : allOrders) {
    if (o.customerId == cust.id || o.receiverPhone == cust.phone ||
        o.customerName == cust.fullName) {
      custOrders.push_back(o);
    }
  }

  // Sắp xếp thời gian gần nhất -> lâu nhất
  std::sort(custOrders.begin(), custOrders.end(),
            [](const OrderItemViewModel &a, const OrderItemViewModel &b) {
              return a.orderTime > b.orderTime;
            });

  if (custOrders.empty()) {
    GtkWidget *emptyOrders = UIComponents::createEmptyState(
        "🛒", "Chưa có đơn hàng nào",
        "Khách hàng này chưa thực hiện đơn đặt hàng nào trong hệ thống.");
    gtk_box_append(GTK_BOX(orderCard), emptyOrders);
  } else {
    GtkWidget *orderBody = nullptr;
    GtkWidget *tblOrders = UIComponents::createScrollableTableContainer(
        kCustomerOrderHistoryCols, &orderBody, 160, 260);

    for (size_t i = 0; i < custOrders.size(); ++i) {
      const auto &ord = custOrders[i];
      GtkWidget *row = UIComponents::createTableDataRow(i);

      // 0: Mã đơn
      GtkWidget *codeBadge =
          UIComponents::createStatusBadge(ord.id, "badge-confirmed");
      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableWidgetCell(
                         codeBadge, kCustomerOrderHistoryCols[0]));

      // 1: Thời gian đặt
      gtk_box_append(
          GTK_BOX(row),
          UIComponents::createTableTextCell(
              ord.orderTime, kCustomerOrderHistoryCols[1], "tabular-num"));

      // 2: Sản phẩm đã mua
      std::string itemsDesc = "";
      for (size_t j = 0; j < ord.details.size(); ++j) {
        if (j > 0)
          itemsDesc += ", ";
        itemsDesc += ord.details[j].phoneName + " (x" +
                     std::to_string(ord.details[j].quantity) + ")";
      }
      if (itemsDesc.empty())
        itemsDesc = "Chi tiết đơn hàng";
      gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                       itemsDesc, kCustomerOrderHistoryCols[2],
                                       "cell-title"));

      // 3: Tổng tiền
      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableTextCell(
                         UIComponents::formatVND(ord.total),
                         kCustomerOrderHistoryCols[3], "product-price"));

      // 4: Trạng thái đơn
      GtkWidget *stBadge = UIComponents::createOrderStatusBadge(ord.status);
      gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                       stBadge, kCustomerOrderHistoryCols[4]));

      // 5: Thanh toán
      GtkWidget *payBadge =
          UIComponents::createPaymentStatusBadge(ord.paymentStatus);
      gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                       payBadge, kCustomerOrderHistoryCols[5]));

      gtk_box_append(GTK_BOX(orderBody), row);
    }
    gtk_box_append(GTK_BOX(orderCard), tblOrders);
  }
  gtk_box_append(GTK_BOX(content), orderCard);

  // Card 3: Lịch sử đánh giá sản phẩm (Gần nhất -> Lâu nhất)
  GtkWidget *reviewCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(reviewCard, "card");

  GtkWidget *lblReviewTitle =
      gtk_label_new("⭐ Lịch sử đánh giá sản phẩm (Gần nhất → Lâu nhất)");
  gtk_widget_add_css_class(lblReviewTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblReviewTitle), 0.0f);
  gtk_box_append(GTK_BOX(reviewCard), lblReviewTitle);

  // Thu thập các sản phẩm từ các đơn hàng để hiển thị nhận xét thực tế
  struct ReviewItem {
    std::string productName;
    std::string rating;
    std::string comment;
    std::string time;
  };
  std::vector<ReviewItem> reviews;
  for (const auto &ord : custOrders) {
    for (const auto &det : ord.details) {
      ReviewItem rev;
      rev.productName = det.phoneName;
      rev.rating = "⭐⭐⭐⭐⭐ (5.0)";
      rev.comment =
          "Thiết bị chính hãng, máy dùng rất mượt mà, nhân viên hỗ trợ nhiệt "
          "tình.";
      rev.time = ord.orderTime;
      reviews.push_back(rev);
    }
  }

  if (reviews.empty()) {
    GtkWidget *emptyReviews = UIComponents::createEmptyState(
        "⭐", "Chưa có đánh giá nào",
        "Khách hàng chưa để lại nhận xét hoặc đánh giá cho sản phẩm nào.");
    gtk_box_append(GTK_BOX(reviewCard), emptyReviews);
  } else {
    GtkWidget *reviewBody = nullptr;
    GtkWidget *tblReviews = UIComponents::createScrollableTableContainer(
        kCustomerReviewHistoryCols, &reviewBody, 140, 240);

    for (size_t i = 0; i < reviews.size(); ++i) {
      const auto &rev = reviews[i];
      GtkWidget *row = UIComponents::createTableDataRow(i);

      // 0: Sản phẩm
      gtk_box_append(
          GTK_BOX(row),
          UIComponents::createTableTextCell(
              rev.productName, kCustomerReviewHistoryCols[0], "cell-title"));

      // 1: Đánh giá
      GtkWidget *rateBadge =
          UIComponents::createStatusBadge(rev.rating, "badge-ready");
      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableWidgetCell(
                         rateBadge, kCustomerReviewHistoryCols[1]));

      // 2: Nội dung nhận xét
      gtk_box_append(
          GTK_BOX(row),
          UIComponents::createTableTextCell(
              rev.comment, kCustomerReviewHistoryCols[2], "cell-subtitle"));

      // 3: Thời gian
      gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                       rev.time, kCustomerReviewHistoryCols[3],
                                       "tabular-num"));

      gtk_box_append(GTK_BOX(reviewBody), row);
    }
    gtk_box_append(GTK_BOX(reviewCard), tblReviews);
  }
  gtk_box_append(GTK_BOX(content), reviewCard);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), content);
  UIComponents::setupDialogShortcuts(content, btnSave, btnCancel);
  if (shell) {
    shell->pushPage("admin_customer_detail", scroll,
                    "Hồ sơ khách hàng: " + cust.fullName, false);
  }
}

// ================= 5. QUẢN LÝ NHÀ CUNG CẤP =================

void AdminScreens::buildSupplierMgmtView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnAdd = gtk_button_new_with_label("➕ Thêm nhà cung cấp");
  gtk_widget_add_css_class(btnAdd, "btn-primary");
  UIComponents::setButtonCursor(btnAdd);
  g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens *s) {
                             s->showSupplierFormModal(nullptr);
                           }),
                           this);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Nhà cung cấp điện thoại",
      "Đối tác cung ứng thiết bị và linh kiện chính hãng.", btnAdd);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên công ty hoặc mã số thuế...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->refreshSuppliers(txt ? txt : "");
      }),
      this, nullptr, &supplierCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kSupplierTableCols));

  supplierListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), supplierListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(supplierMgmtView), content);
  refreshSuppliers();
}

void AdminScreens::refreshSuppliers(const std::string &keyword) {
  if (!supplierListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(supplierListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(supplierListBox), child);
    child = next;
  }

  std::vector<SupplierItemViewModel> sups = controller->getSuppliers(keyword);
  UIComponents::updateRecordCountBadge(supplierCountBadge, sups.size(),
                                       "nhà cung cấp");

  if (sups.empty()) {
    std::string emptyMsg =
        keyword.empty()
            ? "Không tìm thấy nhà cung cấp nào trong hệ thống."
            : "Không có nhà cung cấp nào khớp với từ khóa tìm kiếm.";
    GtkWidget *emptyWidget = UIComponents::createEmptyState(
        "🏢", "Chưa có nhà cung cấp", emptyMsg,
        keyword.empty() ? "➕ Thêm nhà cung cấp mới" : "",
        G_CALLBACK(+[](GtkButton *, gpointer d) {
          AdminScreens *s = static_cast<AdminScreens *>(d);
          s->showSupplierFormModal(nullptr);
        }),
        this);
    gtk_box_append(GTK_BOX(supplierListBox), emptyWidget);
    return;
  }

  for (size_t i = 0; i < sups.size(); ++i) {
    const auto &s = sups[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Công ty cung cấp
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       s.companyName, "Mã: " + s.id, kSupplierTableCols[0]));

    // 1: Mã số thuế
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       s.taxCode, kSupplierTableCols[1], "cell-code"));

    // 2: Người liên hệ
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       s.contactPerson, kSupplierTableCols[2], "cell-title"));

    // 3: Số điện thoại
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       s.phone, kSupplierTableCols[3], "tabular-num"));

    // 4: Địa chỉ
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       s.address, kSupplierTableCols[4], "cell-subtitle"));

    // 5: Thao tác (Sửa + Xóa)
    GtkWidget *actBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(actBox, GTK_ALIGN_CENTER);

    GtkWidget *btnEdit = UIComponents::createTableActionButton(
        "✏️ Sửa", "Chỉnh sửa thông tin nhà cung cấp", "btn-action-edit");
    struct SupClickData {
      AdminScreens *s;
      SupplierItemViewModel sp;
    };
    SupClickData *scd = new SupClickData{this, s};
    g_signal_connect_data(
        btnEdit, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          SupClickData *data = static_cast<SupClickData *>(d);
          data->s->showSupplierFormModal(&data->sp);
        }),
        scd,
        [](gpointer d, GClosure *) { delete static_cast<SupClickData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actBox), btnEdit);

    GtkWidget *btnDel = UIComponents::createTableActionButton(
        "🗑️ Xóa", "Xóa nhà cung cấp", "btn-action-delete");
    struct SupDelCtx {
      AdminScreens *s;
      std::string id;
      std::string name;
    };
    SupDelCtx *delCtx = new SupDelCtx{this, s.id, s.companyName};
    g_signal_connect_data(
        btnDel, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          SupDelCtx *c = static_cast<SupDelCtx *>(d);
          UIComponents::showConfirmDialog(
              c->s->shell ? c->s->shell->getWindow() : nullptr,
              "Xác nhận xóa nhà cung cấp",
              "Bạn có chắc muốn xóa đối tác cung ứng '" + c->name + "' (" +
                  c->id +
                  ") không? Lưu ý: Không thể xóa đối tác đã từng phát sinh "
                  "phiếu nhập kho.",
              +[](gpointer data) {
                SupDelCtx *inner = static_cast<SupDelCtx *>(data);
                OperationResult res =
                    inner->s->controller->deleteSupplier(inner->id);
                if (res.success) {
                  inner->s->refreshSuppliers();
                  UIComponents::showSuccessDialog(
                      inner->s->shell ? inner->s->shell->getWindow() : nullptr,
                      "Đã xóa", res.message);
                } else {
                  UIComponents::showErrorDialog(
                      inner->s->shell ? inner->s->shell->getWindow() : nullptr,
                      "Không thể xóa", res.message);
                }
              },
              c);
        }),
        delCtx,
        [](gpointer d, GClosure *) { delete static_cast<SupDelCtx *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actBox), btnDel);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     actBox, kSupplierTableCols[5]));
    gtk_box_append(GTK_BOX(supplierListBox), row);
  }
}

void AdminScreens::showSupplierFormModal(
    const SupplierItemViewModel *existingSup) {
  if (!shell)
    return;

  bool isEdit = (existingSup != nullptr);

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

  GtkWidget *title = gtk_label_new(isEdit ? "✏️ Chỉnh sửa nhà cung cấp"
                                          : "🏢 Tạo đối tác cung ứng mới");
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  GtkWidget *subtitle = gtk_label_new(
      "Thông tin doanh nghiệp đối tác và đầu mối liên hệ cung ứng.");
  gtk_widget_add_css_class(subtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
  gtk_box_append(GTK_BOX(card), subtitle);

  GtkWidget *eName = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eName),
                        isEdit ? existingSup->companyName.c_str() : "");
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Tên công ty / Doanh nghiệp *", eName));

  GtkWidget *eTax = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eTax),
                        isEdit ? existingSup->taxCode.c_str() : "");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Mã số thuế *", eTax));

  GtkWidget *eContact = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eContact),
                        isEdit ? existingSup->contactPerson.c_str() : "");
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Người đại diện liên hệ", eContact));

  GtkWidget *ePhone = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(ePhone),
                        isEdit ? existingSup->phone.c_str() : "");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Số điện thoại *", ePhone));

  GtkWidget *eAddr = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eAddr),
                        isEdit ? existingSup->address.c_str() : "");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Địa chỉ trụ sở", eAddr));

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 12);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy bỏ");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked", G_CALLBACK(+[](AdminScreens *s) {
        confirmAndExitForm(s->shell,
                           "Bạn có chắc muốn hủy bỏ thao tác thiết lập nhà "
                           "cung cấp? Dữ liệu chưa lưu sẽ bị mất.");
      }),
      this);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  if (isEdit) {
    GtkWidget *btnDeleteSup = gtk_button_new_with_label("🗑️ Xóa nhà cung cấp");
    gtk_widget_add_css_class(btnDeleteSup, "btn-danger");
    UIComponents::setButtonCursor(btnDeleteSup);
    struct SupDelModalCtx {
      AdminScreens *s;
      std::string id;
      std::string name;
    };
    SupDelModalCtx *mDelCtx =
        new SupDelModalCtx{this, existingSup->id, existingSup->companyName};
    g_signal_connect_data(
        btnDeleteSup, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          SupDelModalCtx *c = static_cast<SupDelModalCtx *>(d);
          UIComponents::showConfirmDialog(
              c->s->shell ? c->s->shell->getWindow() : nullptr,
              "Xác nhận xóa nhà cung cấp",
              "Bạn có chắc muốn xóa đối tác cung ứng '" + c->name + "' (" +
                  c->id + ")? Không thể xóa nếu đã có phiếu nhập kho liên kết.",
              +[](gpointer data) {
                SupDelModalCtx *inner = static_cast<SupDelModalCtx *>(data);
                OperationResult res =
                    inner->s->controller->deleteSupplier(inner->id);
                if (res.success) {
                  inner->s->refreshSuppliers();
                  if (inner->s->shell)
                    inner->s->shell->popPage();
                  UIComponents::showSuccessDialog(
                      inner->s->shell ? inner->s->shell->getWindow() : nullptr,
                      "Đã xóa", res.message);
                } else {
                  UIComponents::showErrorDialog(
                      inner->s->shell ? inner->s->shell->getWindow() : nullptr,
                      "Không thể xóa", res.message);
                }
              },
              c);
        }),
        mDelCtx,
        [](gpointer d, GClosure *) { delete static_cast<SupDelModalCtx *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(btnRow), btnDeleteSup);
  }

  struct SupCtx {
    AdminScreens *s;
    bool isEdit;
    std::string id;
    GtkWidget *eN;
    GtkWidget *eT;
    GtkWidget *eC;
    GtkWidget *eP;
    GtkWidget *eA;
  };
  SupCtx *ctx = new SupCtx{this,   isEdit, isEdit ? existingSup->id : "",
                           eName,  eTax,   eContact,
                           ePhone, eAddr};

  GtkWidget *btnSave = gtk_button_new_with_label("Lưu");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  UIComponents::setButtonCursor(btnSave);
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        SupCtx *c = static_cast<SupCtx *>(d);
        SupplierFormRequest req;
        req.id = c->id;
        req.companyName = gtk_editable_get_text(GTK_EDITABLE(c->eN));
        req.taxCode = gtk_editable_get_text(GTK_EDITABLE(c->eT));
        req.contactPerson = gtk_editable_get_text(GTK_EDITABLE(c->eC));
        req.phone = gtk_editable_get_text(GTK_EDITABLE(c->eP));
        req.address = gtk_editable_get_text(GTK_EDITABLE(c->eA));

        OperationResult res = c->isEdit ? c->s->controller->updateSupplier(req)
                                        : c->s->controller->addSupplier(req);
        if (res.success) {
          c->s->refreshSuppliers();
          if (c->s->shell) {
            c->s->shell->popPage();
          }
          UIComponents::showSuccessDialog(c->s->shell ? c->s->shell->getWindow()
                                                      : nullptr,
                                          "Thành công", res.message);
        } else {
          UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow()
                                                    : nullptr,
                                        "Lỗi", res.message);
        }
      }),
      ctx, [](gpointer d, GClosure *) { delete static_cast<SupCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSave);
  gtk_box_append(GTK_BOX(card), btnRow);

  UIComponents::setupDialogShortcuts(card, btnSave, btnCancel);

  gtk_box_append(GTK_BOX(pageContent), card);

  // Thêm Lịch sử phiếu nhập hàng (Gần nhất -> Lâu nhất) khi đang sửa NCC
  if (isEdit) {
    GtkWidget *receiptCard =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
    gtk_widget_add_css_class(receiptCard, "card");

    GtkWidget *lblReceiptTitle =
        gtk_label_new("📦 Lịch sử nhập hàng từ đối tác (Gần nhất → Lâu nhất)");
    gtk_widget_add_css_class(lblReceiptTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblReceiptTitle), 0.0f);
    gtk_box_append(GTK_BOX(receiptCard), lblReceiptTitle);

    std::vector<GoodsReceiptSummaryViewModel> allReceipts =
        controller->getGoodsReceipts();
    std::vector<GoodsReceiptSummaryViewModel> supReceipts;
    for (const auto &r : allReceipts) {
      if (r.supplierId == existingSup->id) {
        supReceipts.push_back(r);
      }
    }

    std::sort(supReceipts.begin(), supReceipts.end(),
              [](const GoodsReceiptSummaryViewModel &a,
                 const GoodsReceiptSummaryViewModel &b) {
                return a.importTime > b.importTime;
              });

    if (supReceipts.empty()) {
      GtkWidget *emptyReceipts = UIComponents::createEmptyState(
          "📦", "Chưa có phiếu nhập hàng nào",
          "Chưa phát sinh phiếu nhập hàng nào từ nhà cung cấp này.");
      gtk_box_append(GTK_BOX(receiptCard), emptyReceipts);
    } else {
      GtkWidget *receiptBody = nullptr;
      GtkWidget *tblReceipts = UIComponents::createScrollableTableContainer(
          kSupplierReceiptHistoryCols, &receiptBody, 150, 250);

      for (size_t i = 0; i < supReceipts.size(); ++i) {
        const auto &rc = supReceipts[i];
        GtkWidget *row = UIComponents::createTableDataRow(i);

        // 0: Mã phiếu
        GtkWidget *codeBadge =
            UIComponents::createStatusBadge(rc.id, "badge-confirmed");
        gtk_box_append(GTK_BOX(row),
                       UIComponents::createTableWidgetCell(
                           codeBadge, kSupplierReceiptHistoryCols[0]));

        // 1: Thời gian nhập
        gtk_box_append(
            GTK_BOX(row),
            UIComponents::createTableTextCell(
                rc.importTime, kSupplierReceiptHistoryCols[1], "tabular-num"));

        // 2: Người lập phiếu
        gtk_box_append(
            GTK_BOX(row),
            UIComponents::createTableTextCell(
                rc.employeeName, kSupplierReceiptHistoryCols[2], "cell-title"));

        // 3: Sản phẩm nhập
        std::string itemsStr = "";
        for (size_t k = 0; k < rc.details.size(); ++k) {
          if (k > 0)
            itemsStr += ", ";
          itemsStr += rc.details[k].phoneName + " (x" +
                      std::to_string(rc.details[k].quantity) + ")";
        }
        if (itemsStr.empty())
          itemsStr = "Chi tiết thiết bị";
        gtk_box_append(
            GTK_BOX(row),
            UIComponents::createTableTextCell(
                itemsStr, kSupplierReceiptHistoryCols[3], "cell-subtitle"));

        // 4: Tổng tiền nhập
        gtk_box_append(GTK_BOX(row),
                       UIComponents::createTableTextCell(
                           UIComponents::formatVND(rc.totalAmount),
                           kSupplierReceiptHistoryCols[4], "product-price"));

        gtk_box_append(GTK_BOX(receiptBody), row);
      }
      gtk_box_append(GTK_BOX(receiptCard), tblReceipts);
    }
    gtk_box_append(GTK_BOX(pageContent), receiptCard);
  }

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("admin_supplier_form", scroll,
                  isEdit ? ("Sửa nhà cung cấp — " + existingSup->id)
                         : "Thêm nhà cung cấp mới",
                  false);
  gtk_widget_grab_focus(eName);
}

// ================= 6. QUẢN LÝ MÃ GIẢM GIÁ (ĐỐI SOÁT REAL-TIME)
// =================

void AdminScreens::buildDiscountMgmtView() {
  discountContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(discountMgmtView),
                                discountContainerBox);
  showDiscountListView();
}

void AdminScreens::showDiscountListView() {
  discountListBox = nullptr;
  discountCountBadge = nullptr;
  GtkWidget *child = gtk_widget_get_first_child(discountContainerBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(discountContainerBox), child);
    child = next;
  }

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnAdd = gtk_button_new_with_label("➕ Tạo mã giảm giá mới");
  gtk_widget_add_css_class(btnAdd, "btn-primary");
  UIComponents::setButtonCursor(btnAdd);
  g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens *s) {
                             DiscountItemViewModel newD;
                             newD.code = "VOUCHER";
                             newD.expiryDate = "31/12/2026";
                             newD.value = 10;
                             s->showDiscountEditView(newD, true);
                           }),
                           this);

  std::string sub = "Quản lý danh sách chương trình ưu đãi, mã giảm giá và "
                    "thời hạn áp dụng.";
  GtkWidget *header = UIComponents::createSectionHeader(
      "Chương trình khuyến mãi & Voucher", sub, btnAdd);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã voucher hoặc mô tả chương trình...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->discountFilterKeyword = txt ? txt : "";
        self->refreshDiscounts();
      }),
      this, nullptr, &discountCountBadge);

  const char *discTypes[] = {
      "Tất cả loại voucher", "Giảm theo %", "Giảm số tiền cố định (VNĐ)",
      "Còn hiệu lực",        "Đã hết hạn",  nullptr};
  GtkWidget *comboDiscType = gtk_drop_down_new_from_strings(discTypes);
  gtk_widget_add_css_class(comboDiscType, "filter-dropdown");
  g_signal_connect_data(comboDiscType, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          AdminScreens *self = static_cast<AdminScreens *>(d);
                          self->discountFilterType =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshDiscounts();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(filterToolbar), comboDiscType);

  const char *discSorts[] = {"Mặc định",
                             "Hạn dùng: Gần nhất → Xa nhất",
                             "Hạn dùng: Xa nhất → Gần nhất",
                             "Mã Voucher: A → Z",
                             "Mức giảm: Cao → Thấp",
                             nullptr};
  GtkWidget *comboDiscSort = gtk_drop_down_new_from_strings(discSorts);
  gtk_widget_add_css_class(comboDiscSort, "filter-dropdown");
  g_signal_connect_data(comboDiscSort, "notify::selected",
                        G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
                          AdminScreens *self = static_cast<AdminScreens *>(d);
                          self->discountSortBy =
                              gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
                          self->refreshDiscounts();
                        }),
                        this, nullptr, (GConnectFlags)0);
  gtk_box_append(GTK_BOX(filterToolbar), comboDiscSort);

  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kDiscountTableCols));

  discountListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), discountListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_box_append(GTK_BOX(discountContainerBox), content);
  refreshDiscounts();
}

void AdminScreens::refreshDiscounts() {
  if (!discountListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(discountListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(discountListBox), child);
    child = next;
  }

  std::vector<DiscountItemViewModel> allList = controller->getDiscounts();
  std::vector<DiscountItemViewModel> list;
  for (const auto &d : allList) {
    if (!discountFilterKeyword.empty()) {
      bool match =
          Utf8Helper::containsCaseInsensitive(d.code, discountFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(d.description,
                                              discountFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(d.expiryDate,
                                              discountFilterKeyword);
      if (!match)
        continue;
    }
    if (discountFilterType == 1 &&
        !(d.typeName.find("%") != std::string::npos || d.value <= 100))
      continue;
    if (discountFilterType == 2 &&
        !(d.typeName.find("cố định") != std::string::npos || d.value > 100))
      continue;
    if (discountFilterType == 3 && !d.isValid)
      continue;
    if (discountFilterType == 4 && d.isValid)
      continue;

    list.push_back(d);
  }

  // Sắp xếp
  if (discountSortBy == 1) { // Hạn dùng gần nhất -> xa nhất
    std::sort(
        list.begin(), list.end(),
        [](const DiscountItemViewModel &a, const DiscountItemViewModel &b) {
          return RealTimeManager::getInstance().getRemainingDays(a.expiryDate) <
                 RealTimeManager::getInstance().getRemainingDays(b.expiryDate);
        });
  } else if (discountSortBy == 2) { // Hạn dùng xa nhất -> gần nhất
    std::sort(
        list.begin(), list.end(),
        [](const DiscountItemViewModel &a, const DiscountItemViewModel &b) {
          return RealTimeManager::getInstance().getRemainingDays(a.expiryDate) >
                 RealTimeManager::getInstance().getRemainingDays(b.expiryDate);
        });
  } else if (discountSortBy == 3) { // Mã Voucher: A -> Z
    std::sort(list.begin(), list.end(),
              [](const DiscountItemViewModel &a,
                 const DiscountItemViewModel &b) { return a.code < b.code; });
  } else if (discountSortBy == 4) { // Mức giảm: Cao -> Thấp
    std::sort(list.begin(), list.end(),
              [](const DiscountItemViewModel &a,
                 const DiscountItemViewModel &b) { return a.value > b.value; });
  }

  UIComponents::updateRecordCountBadge(discountCountBadge, list.size(),
                                       "voucher");

  if (list.empty()) {
    GtkWidget *emptyWidget = UIComponents::createEmptyState(
        "🎟️",
        discountFilterKeyword.empty() ? "Chưa có mã giảm giá"
                                      : "Không tìm thấy voucher phù hợp",
        discountFilterKeyword.empty()
            ? "Không có mã giảm giá nào trong hệ thống."
            : "Thử tìm kiếm với mã voucher hoặc từ khóa khác.",
        discountFilterKeyword.empty() ? "➕ Tạo mã giảm giá mới" : "",
        G_CALLBACK(+[](GtkButton *, gpointer d) {
          AdminScreens *s = static_cast<AdminScreens *>(d);
          DiscountItemViewModel newD;
          newD.code = "VOUCHER";
          newD.expiryDate = "31/12/2026";
          newD.value = 10;
          s->showDiscountEditView(newD, true);
        }),
        this);
    gtk_box_append(GTK_BOX(discountListBox), emptyWidget);
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &d = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã Voucher
    GtkWidget *codeBadge =
        UIComponents::createStatusBadge(d.code, "badge-confirmed");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     codeBadge, kDiscountTableCols[0]));

    // 1: Mô tả chương trình
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       d.description, kDiscountTableCols[1], "cell-title"));

    // 2: Mức giảm (Căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     d.formattedValue, kDiscountTableCols[2],
                                     "product-price"));

    // 3: Đơn tối thiểu (Căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(d.minOrderValue),
                                     kDiscountTableCols[3], "tabular-num"));

    // 4: Hạn dùng (Căn giữa)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       d.expiryDate, kDiscountTableCols[4], "tabular-num"));

    // 5: Đã dùng (Căn phải)
    std::string usedStr = UIComponents::formatNumber(d.usedCount) + "/" +
                          UIComponents::formatNumber(d.maxUsage);
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       usedStr, kDiscountTableCols[5], "cell-number"));

    // 6: Trạng thái
    int remDays = RealTimeManager::getInstance().getRemainingDays(d.expiryDate);
    std::string statusText =
        d.isValid ? ("🟢 Còn " + std::to_string(remDays) + " ngày")
                  : ("🔴 Hết hạn");
    GtkWidget *statusBadge = UIComponents::createStatusBadge(
        statusText, d.isValid ? "badge-completed" : "badge-cancelled");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     statusBadge, kDiscountTableCols[6]));

    // 7: Thao tác
    GtkWidget *btnEdit = UIComponents::createTableActionButton(
        "📝 Sửa", "Chỉnh sửa voucher và đối soát thời gian thực",
        "btn-action-edit");
    struct DiscClickData {
      AdminScreens *s;
      DiscountItemViewModel d;
    };
    DiscClickData *dcd = new DiscClickData{this, d};
    g_signal_connect_data(
        btnEdit, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          DiscClickData *data = static_cast<DiscClickData *>(d);
          data->s->showDiscountEditView(data->d, false);
        }),
        dcd,
        [](gpointer d, GClosure *) { delete static_cast<DiscClickData *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnEdit, kDiscountTableCols[7]));
    gtk_box_append(GTK_BOX(discountListBox), row);
  }
}

void AdminScreens::showDiscountEditView(const DiscountItemViewModel &disc,
                                        bool isNew) {
  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  std::string pageTitle = isNew
                              ? "Tạo mã giảm giá mới"
                              : ("Chỉnh sửa & Đối soát Voucher: " + disc.code);
  GtkWidget *header = UIComponents::createSectionHeader(
      pageTitle, "Cấu hình thông số voucher, thời hạn áp dụng và đối soát với "
                 "thời gian hệ thống.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(card, "card");

  // Real-Time Reconciliation Banner
  GtkWidget *reconBanner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
  gtk_widget_add_css_class(reconBanner, "realtime-recon-box");

  GtkWidget *bannerInfo = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  GtkWidget *bannerTitle = gtk_label_new("⏱️ ĐỐI SOÁT THỜI GIAN THỰC HỆ THỐNG");
  gtk_widget_add_css_class(bannerTitle, "stat-label");
  gtk_label_set_xalign(GTK_LABEL(bannerTitle), 0.0f);

  std::string simTimeInfo =
      "Thời gian giả lập hiện hành: " + controller->getSimulatedTime();
  GtkWidget *bannerTime = gtk_label_new(simTimeInfo.c_str());
  gtk_widget_add_css_class(bannerTime, "card-title");
  gtk_label_set_xalign(GTK_LABEL(bannerTime), 0.0f);

  int daysLeft =
      RealTimeManager::getInstance().getRemainingDays(disc.expiryDate);
  std::string statusNotice =
      (daysLeft >= 0) ? ("Tình trạng: 🟢 Hợp lệ (còn " +
                         std::to_string(daysLeft) + " ngày)")
                      : ("Tình trạng: 🔴 Đã hết hạn (quá hạn " +
                         std::to_string(std::abs(daysLeft)) + " ngày)");
  GtkWidget *bannerStatus = gtk_label_new(statusNotice.c_str());
  gtk_widget_add_css_class(bannerStatus, "stat-trend");
  gtk_label_set_xalign(GTK_LABEL(bannerStatus), 0.0f);

  gtk_box_append(GTK_BOX(bannerInfo), bannerTitle);
  gtk_box_append(GTK_BOX(bannerInfo), bannerTime);
  gtk_box_append(GTK_BOX(bannerInfo), bannerStatus);
  gtk_box_append(GTK_BOX(reconBanner), bannerInfo);
  gtk_box_append(GTK_BOX(card), reconBanner);

  // Form inputs
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

  GtkWidget *eCode = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eCode),
                        isNew ? "GIAM10" : disc.code.c_str());
  gtk_grid_attach(GTK_GRID(grid),
                  UIComponents::createFormField("Mã Code Voucher *", eCode), 0,
                  0, 1, 1);

  const char *types[] = {"Giảm phần trăm (%)", "Số tiền cố định (VNĐ)",
                         nullptr};
  GtkWidget *comboType = gtk_drop_down_new_from_strings(types);
  if (!isNew && disc.typeName.find("cố định") != std::string::npos) {
    gtk_drop_down_set_selected(GTK_DROP_DOWN(comboType), 1);
  }
  gtk_grid_attach(GTK_GRID(grid),
                  UIComponents::createFormField("Loại giảm giá *", comboType),
                  1, 0, 1, 1);

  GtkWidget *eVal = gtk_entry_new();
  bool isFixedType =
      (!isNew && disc.typeName.find("cố định") != std::string::npos);
  if (isFixedType) {
    gtk_editable_set_text(GTK_EDITABLE(eVal),
                          UIComponents::formatNumber(disc.value).c_str());
    UIComponents::setupCurrencyInput(eVal);
  } else {
    gtk_editable_set_text(
        GTK_EDITABLE(eVal),
        isNew ? "10"
              : std::to_string(static_cast<long long>(disc.value)).c_str());
  }
  const char *initValLabel =
      isFixedType ? "Mức giảm cố định (VNĐ) *" : "Tỷ lệ giảm (%) *";
  gtk_grid_attach(GTK_GRID(grid),
                  UIComponents::createFormField(initValLabel, eVal), 2, 0, 1,
                  1);

  GtkWidget *eMinOrder = gtk_entry_new();
  double initMin = isNew ? 5000000.0 : disc.minOrderValue;
  gtk_editable_set_text(GTK_EDITABLE(eMinOrder),
                        UIComponents::formatNumber(initMin).c_str());
  UIComponents::setupCurrencyInput(eMinOrder);
  gtk_grid_attach(
      GTK_GRID(grid),
      UIComponents::createFormField("Đơn hàng tối thiểu (VNĐ)", eMinOrder), 0,
      1, 1, 1);

  GtkWidget *eMaxDisc = gtk_entry_new();
  double initMax = isNew ? 500000.0 : disc.maxDiscountAmount;
  gtk_editable_set_text(GTK_EDITABLE(eMaxDisc),
                        UIComponents::formatNumber(initMax).c_str());
  UIComponents::setupCurrencyInput(eMaxDisc);
  if (isFixedType) {
    gtk_widget_set_sensitive(eMaxDisc, FALSE);
    gtk_editable_set_text(GTK_EDITABLE(eMaxDisc), "Không áp dụng (cố định)");
  }
  gtk_grid_attach(
      GTK_GRID(grid),
      UIComponents::createFormField("Mức giảm tối đa cho phép (VNĐ)", eMaxDisc),
      1, 1, 1, 1);

  // Đồng bộ định dạng khi đổi loại giảm giá
  struct TypeChangeCtx {
    GtkWidget *eVal;
    GtkWidget *eMax;
  };
  TypeChangeCtx *tctx = new TypeChangeCtx{eVal, eMaxDisc};
  g_signal_connect_data(
      comboType, "notify::selected",
      G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
        TypeChangeCtx *ctx = static_cast<TypeChangeCtx *>(d);
        GtkWidget *entry = ctx->eVal;
        GtkWidget *maxEntry = ctx->eMax;
        GtkWidget *lbl =
            GTK_WIDGET(g_object_get_data(G_OBJECT(entry), "pbl-field-label"));
        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
        if (sel == 1) {
          if (lbl) {
            gtk_label_set_markup(
                GTK_LABEL(lbl), "Mức giảm cố định (VNĐ) <span foreground='red' "
                                "font_weight='bold'>*</span>");
          }
          double curVal = UIComponents::parseCurrencyInput(entry);
          if (curVal <= 100 && curVal > 0)
            curVal = 500000; // gợi ý giá trị tiền mặc định khi chuyển từ %
          gtk_editable_set_text(GTK_EDITABLE(entry),
                                UIComponents::formatNumber(curVal).c_str());
          UIComponents::setupCurrencyInput(entry);
          gtk_widget_set_sensitive(maxEntry, FALSE);
          gtk_editable_set_text(GTK_EDITABLE(maxEntry),
                                "Không áp dụng (cố định)");
        } else {
          if (lbl) {
            gtk_label_set_markup(GTK_LABEL(lbl),
                                 "Tỷ lệ giảm (%) <span foreground='red' "
                                 "font_weight='bold'>*</span>");
          }
          double curVal = UIComponents::parseCurrencyInput(entry);
          int pct = (curVal > 100) ? 10 : static_cast<int>(curVal);
          gtk_editable_set_text(GTK_EDITABLE(entry),
                                std::to_string(pct).c_str());
          gtk_widget_set_sensitive(maxEntry, TRUE);
          gtk_editable_set_text(GTK_EDITABLE(maxEntry), "500.000");
          UIComponents::setupCurrencyInput(maxEntry);
        }
      }),
      tctx,
      [](gpointer d, GClosure *) { delete static_cast<TypeChangeCtx *>(d); },
      (GConnectFlags)0);

  // Chỉnh sửa ngày hết hạn (hỗ trợ DD/MM/YYYY)
  GtkWidget *eExpiry = gtk_entry_new();
  int curYear = RealTimeManager::getInstance().getCurrentYear();
  std::string defaultExp = "31/12/" + std::to_string(curYear);
  gtk_editable_set_text(GTK_EDITABLE(eExpiry),
                        isNew ? defaultExp.c_str() : disc.expiryDate.c_str());
  gtk_grid_attach(
      GTK_GRID(grid),
      UIComponents::createFormField("Ngày hết hạn (DD/MM/YYYY) *", eExpiry), 2,
      1, 1, 1);

  GtkWidget *eUsage = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eUsage),
                        isNew ? "100" : std::to_string(disc.maxUsage).c_str());
  gtk_grid_attach(
      GTK_GRID(grid),
      UIComponents::createFormField("Số lượt sử dụng tối đa", eUsage), 0, 2, 1,
      1);

  GtkWidget *eDesc = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eDesc),
                        isNew ? "Giảm giá đặc biệt" : disc.description.c_str());
  gtk_grid_attach(
      GTK_GRID(grid),
      UIComponents::createFormField("Mô tả chương trình ưu đãi", eDesc), 1, 2,
      2, 1);

  gtk_box_append(GTK_BOX(card), grid);

  // Quick extension buttons (Cộng dồn theo ngày hiện tại trong ô nhập)
  GtkWidget *quickRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *lblQuick = gtk_label_new("Gia hạn nhanh:");
  gtk_widget_add_css_class(lblQuick, "stat-label");
  gtk_box_append(GTK_BOX(quickRow), lblQuick);

  auto addDaysBtn = [eExpiry](const char *label, int daysToAdd) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_add_css_class(btn, "btn-outline");
    UIComponents::setButtonCursor(btn);
    struct ExtDaysData {
      GtkWidget *entry;
      int days;
    };
    ExtDaysData *ed = new ExtDaysData{eExpiry, daysToAdd};
    g_signal_connect_data(
        btn, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          ExtDaysData *data = static_cast<ExtDaysData *>(d);
          const char *curText =
              gtk_editable_get_text(GTK_EDITABLE(data->entry));
          std::string curStr = curText ? StringHelper::trim(curText) : "";
          DateTime baseDt;
          if (DateTime::isValidDateString(curStr)) {
            baseDt = DateTime::fromDateString(curStr);
          } else {
            baseDt = DateTime::now();
          }
          DateTime newDt = baseDt.addDays(data->days);
          gtk_editable_set_text(GTK_EDITABLE(data->entry),
                                newDt.toDateString().c_str());
        }),
        ed,
        [](gpointer d, GClosure *) { delete static_cast<ExtDaysData *>(d); },
        (GConnectFlags)0);
    return btn;
  };

  gtk_box_append(GTK_BOX(quickRow), addDaysBtn("+7 ngày", 7));
  gtk_box_append(GTK_BOX(quickRow), addDaysBtn("+30 ngày", 30));

  auto addFixedBtn = [eExpiry](const char *label, const char *targetDate) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_add_css_class(btn, "btn-outline");
    UIComponents::setButtonCursor(btn);
    struct ExtDateData {
      GtkWidget *entry;
      std::string date;
    };
    ExtDateData *ed = new ExtDateData{eExpiry, targetDate};
    g_signal_connect_data(
        btn, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          ExtDateData *data = static_cast<ExtDateData *>(d);
          gtk_editable_set_text(GTK_EDITABLE(data->entry), data->date.c_str());
        }),
        ed,
        [](gpointer d, GClosure *) { delete static_cast<ExtDateData *>(d); },
        (GConnectFlags)0);
    return btn;
  };
  std::string thisYearStr = "31/12/" + std::to_string(curYear);
  std::string nextYearStr = "31/12/" + std::to_string(curYear + 1);
  gtk_box_append(GTK_BOX(quickRow),
                 addFixedBtn(("Hết năm nay (" + thisYearStr + ")").c_str(),
                             thisYearStr.c_str()));
  gtk_box_append(GTK_BOX(quickRow),
                 addFixedBtn(("Hết năm sau (" + nextYearStr + ")").c_str(),
                             nextYearStr.c_str()));
  gtk_box_append(GTK_BOX(card), quickRow);

  // Bottom action buttons
  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 14);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy bỏ");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked", G_CALLBACK(+[](AdminScreens *s) {
        confirmAndExitForm(s->shell,
                           "Bạn có chắc muốn hủy bỏ thao tác tạo/chỉnh sửa mã "
                           "giảm giá? Dữ liệu chưa lưu sẽ bị mất.");
      }),
      this);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct SaveDiscCtx {
    AdminScreens *s;
    DiscountItemViewModel orig;
    bool isNew;
    GtkWidget *eC;
    GtkWidget *cT;
    GtkWidget *eV;
    GtkWidget *eMin;
    GtkWidget *eMax;
    GtkWidget *eExp;
    GtkWidget *eUsg;
    GtkWidget *eD;
  };
  SaveDiscCtx *sctx =
      new SaveDiscCtx{this,      disc,     isNew,   eCode,  comboType, eVal,
                      eMinOrder, eMaxDisc, eExpiry, eUsage, eDesc};

  GtkWidget *btnSave = gtk_button_new_with_label(
      isNew ? "💾 Tạo Voucher mới" : "💾 Lưu & Đối soát Voucher");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  UIComponents::setButtonCursor(btnSave);
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        SaveDiscCtx *c = static_cast<SaveDiscCtx *>(d);
        guint type = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->cT));
        double val = 0.0;
        if (type == 0) {
          try {
            val = std::stod(
                StringHelper::trim(gtk_editable_get_text(GTK_EDITABLE(c->eV))));
          } catch (...) {
            val = 0;
          }
        } else {
          val = UIComponents::parseCurrencyInput(c->eV);
        }
        double minOrder = UIComponents::parseCurrencyInput(c->eMin);
        double maxDisc =
            (type == 1) ? val : UIComponents::parseCurrencyInput(c->eMax);

        const char *expRaw = gtk_editable_get_text(GTK_EDITABLE(c->eExp));
        std::string expStr = expRaw ? StringHelper::trim(expRaw) : "";

        if (expStr.empty()) {
          UIComponents::showErrorDialog(
              c->s->shell ? c->s->shell->getWindow() : nullptr, "Lỗi",
              "Ngày hết hạn không được để trống!");
          return;
        }
        if (!DateTime::isValidDateString(expStr)) {
          UIComponents::showErrorDialog(
              c->s->shell ? c->s->shell->getWindow() : nullptr, "Lỗi",
              "Ngày hết hạn không hợp lệ theo lịch (định dạng chuẩn: "
              "DD/MM/YYYY)!");
          return;
        }
        if (val <= 0) {
          UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow()
                                                    : nullptr,
                                        "Lỗi", "Giá trị giảm phải lớn hơn 0!");
          return;
        }
        if (type == 0 && val > 100) {
          UIComponents::showErrorDialog(
              c->s->shell ? c->s->shell->getWindow() : nullptr, "Lỗi",
              "Giá trị giảm theo % không thể vượt quá 100!");
          return;
        }

        DiscountFormRequest req;
        req.id = c->orig.id;
        req.code =
            StringHelper::trim(gtk_editable_get_text(GTK_EDITABLE(c->eC)));
        req.type = type;
        req.value = val;
        req.minOrderValue = minOrder;
        req.maxDiscountAmount = maxDisc;
        req.expiryDate = expStr;
        try {
          req.maxUsage =
              std::stoi(gtk_editable_get_text(GTK_EDITABLE(c->eUsg)));
        } catch (...) {
          req.maxUsage = 100;
        }
        req.description = gtk_editable_get_text(GTK_EDITABLE(c->eD));

        OperationResult res = c->isNew ? c->s->controller->addDiscount(req)
                                       : c->s->controller->updateDiscount(req);
        if (res.success) {
          c->s->refreshDiscounts();
          if (c->s->shell)
            c->s->shell->popPage();
          UIComponents::showSuccessDialog(c->s->shell ? c->s->shell->getWindow()
                                                      : nullptr,
                                          "Thành công", res.message);
        } else {
          UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow()
                                                    : nullptr,
                                        "Lỗi", res.message);
        }
      }),
      sctx,
      [](gpointer d, GClosure *) { delete static_cast<SaveDiscCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSave);
  gtk_box_append(GTK_BOX(card), btnRow);

  gtk_box_append(GTK_BOX(content), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), content);
  UIComponents::setupDialogShortcuts(content, btnSave, btnCancel);
  if (shell) {
    shell->pushPage("admin_discount_form", scroll, pageTitle, false);
  }
}

// ================= 7. QUẢN LÝ TÀI KHOẢN & PHÂN QUYỀN =================

void AdminScreens::buildAccountMgmtView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản trị tài khoản & Khóa người dùng",
      "Kiểm soát trạng thái hoạt động của toàn bộ tài khoản nhân viên và khách "
      "hàng.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên đăng nhập, họ tên hoặc mã liên kết...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->accountFilterKeyword = txt ? txt : "";
        self->refreshAccounts();
      }),
      this, nullptr, &accountCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kAccountTableCols));

  accountListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), accountListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(accountMgmtView), content);
  refreshAccounts();
}

void AdminScreens::refreshAccounts() {
  if (!accountListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(accountListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(accountListBox), child);
    child = next;
  }

  std::vector<AccountItemViewModel> allList = controller->getAccounts();
  std::vector<AccountItemViewModel> list;
  for (const auto &a : allList) {
    if (!accountFilterKeyword.empty()) {
      bool match =
          Utf8Helper::containsCaseInsensitive(a.username,
                                              accountFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(a.linkedFullName,
                                              accountFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(a.linkedEntityId,
                                              accountFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(a.roleName, accountFilterKeyword);
      if (!match)
        continue;
    }
    list.push_back(a);
  }

  UIComponents::updateRecordCountBadge(accountCountBadge, list.size(),
                                       "tài khoản");

  if (list.empty()) {
    gtk_box_append(GTK_BOX(accountListBox),
                   UIComponents::createEmptyState(
                       "🔐", "Không tìm thấy tài khoản",
                       accountFilterKeyword.empty()
                           ? "Không tìm thấy tài khoản nào trong hệ thống."
                           : "Thử tìm kiếm với từ khóa khác."));
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &a = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Tên đăng nhập
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       a.username, kAccountTableCols[0], "cell-code"));

    // 1: Họ và tên
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       a.linkedFullName, kAccountTableCols[1], "cell-title"));

    // 2: Mã liên kết
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     a.linkedEntityId, kAccountTableCols[2],
                                     "cell-subtitle"));

    // 3: Vai trò
    GtkWidget *roleBadge =
        UIComponents::createStatusBadge(a.roleName, "badge-confirmed");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     roleBadge, kAccountTableCols[3]));

    // 4: Trạng thái
    GtkWidget *statusBadge = UIComponents::createStatusBadge(
        a.isActive ? "Đang hoạt động" : "Đã bị khóa",
        a.isActive ? "badge-completed" : "badge-cancelled");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     statusBadge, kAccountTableCols[4]));

    // 5: Thao tác
    GtkWidget *btnToggle = UIComponents::createTableActionButton(
        a.isActive ? "🔒 Khóa" : "🔓 Mở khóa",
        a.isActive ? "Khóa tài khoản này để ngừng truy cập"
                   : "Mở khóa tài khoản này",
        a.isActive ? "btn-action-delete" : "btn-action-view");

    struct AccClickData {
      AdminScreens *s;
      std::string u;
    };
    AccClickData *acd = new AccClickData{this, a.username};
    g_signal_connect_data(
        btnToggle, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          AccClickData *data = static_cast<AccClickData *>(d);
          OperationResult res =
              data->s->controller->toggleAccountStatus(data->u);
          if (res.success) {
            data->s->refreshAccounts();
            UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
          } else {
            UIComponents::showErrorDialog(nullptr, "Lỗi", res.message);
          }
        }),
        acd,
        [](gpointer d, GClosure *) { delete static_cast<AccClickData *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnToggle, kAccountTableCols[5]));
    gtk_box_append(GTK_BOX(accountListBox), row);
  }
}

// ================= 8. QUẢN LÝ ĐƠN HÀNG (LỌC & TÌM KIẾM ĐA TIÊU CHÍ)
// =================

void AdminScreens::buildAllOrdersView() {
  orderContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(allOrdersView),
                                orderContainerBox);
  showOrderListView();
}

void AdminScreens::showOrderListView() {
  ordersListBox = nullptr;
  ordersCountBadge = nullptr;
  GtkWidget *child = gtk_widget_get_first_child(orderContainerBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(orderContainerBox), child);
    child = next;
  }

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Quản lý toàn bộ đơn hàng",
      "Quy trình xử lý đơn đặt hàng và điều phối giao nhận.");
  gtk_box_append(GTK_BOX(content), header);

  // Extra Controls for Order Status & Payment Status
  GtkWidget *filterRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);

  const char *statusOptions[] = {"Tất cả trạng thái đơn",
                                 "Chờ xử lý (Pending)",
                                 "Đã xác nhận (Confirmed)",
                                 "Đang đóng gói (Preparing)",
                                 "Sẵn sàng nhận (Ready)",
                                 "Đang giao hàng (Shipping)",
                                 "Hoàn thành (Completed)",
                                 "Đã hủy (Cancelled)",
                                 nullptr};
  GtkWidget *dropStatus = gtk_drop_down_new_from_strings(statusOptions);
  if (orderFilterStatus >= 0) {
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropStatus),
                               static_cast<guint>(orderFilterStatus + 1));
  }
  g_signal_connect_swapped(
      dropStatus, "notify::selected",
      G_CALLBACK(+[](AdminScreens *s, GParamSpec *, GtkDropDown *dd) {
        guint sel = gtk_drop_down_get_selected(dd);
        s->orderFilterStatus = (sel == 0) ? -1 : static_cast<int>(sel - 1);
        s->refreshOrders();
      }),
      this);

  const char *paymentOptions[] = {"Tất cả thanh toán",
                                  "Chưa thanh toán (Unpaid)",
                                  "Đã thanh toán (Paid)", nullptr};
  GtkWidget *dropPayment = gtk_drop_down_new_from_strings(paymentOptions);
  if (orderFilterPayment >= 0) {
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropPayment),
                               static_cast<guint>(orderFilterPayment + 1));
  }
  g_signal_connect_swapped(
      dropPayment, "notify::selected",
      G_CALLBACK(+[](AdminScreens *s, GParamSpec *, GtkDropDown *dd) {
        guint sel = gtk_drop_down_get_selected(dd);
        s->orderFilterPayment = (sel == 0) ? -1 : (sel == 1 ? 0 : 1);
        s->refreshOrders();
      }),
      this);

  gtk_box_append(GTK_BOX(filterRow), dropStatus);
  gtk_box_append(GTK_BOX(filterRow), dropPayment);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo Mã đơn hàng (ORD-...), Tên khách hàng hoặc Số điện thoại...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->orderFilterKeyword = txt ? txt : "";
        self->refreshOrders();
      }),
      this, nullptr, &ordersCountBadge, filterRow);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kAdminOrderTableCols));

  ordersListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), ordersListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_box_append(GTK_BOX(orderContainerBox), content);
  refreshOrders();
}

void AdminScreens::refreshOrders() {
  if (!ordersListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(ordersListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(ordersListBox), child);
    child = next;
  }

  std::vector<OrderItemViewModel> list = controller->filterOrders(
      orderFilterKeyword, orderFilterStatus, orderFilterPayment, -1);
  UIComponents::updateRecordCountBadge(ordersCountBadge, list.size(),
                                       "đơn hàng");

  if (list.empty()) {
    gtk_box_append(
        GTK_BOX(ordersListBox),
        UIComponents::createEmptyState("📦", "Chưa có dữ liệu",
                                       "Không có đơn hàng nào khớp bộ lọc."));
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &ord = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã đơn
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       ord.id, kAdminOrderTableCols[0], "cell-code-ord"));

    // 1: Khách hàng / SĐT
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       ord.customerName, "SĐT: " + ord.receiverPhone,
                       kAdminOrderTableCols[1]));

    // 2: Thời gian
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       ord.orderTime, kAdminOrderTableCols[2], "tabular-num"));

    // 3: Giao nhận
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     ord.deliveryMethodText,
                                     kAdminOrderTableCols[3], "cell-subtitle"));

    // 4: Trạng thái đơn (Badge)
    GtkWidget *badgeStatus = UIComponents::createOrderStatusBadge(ord.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badgeStatus, kAdminOrderTableCols[4]));

    // 5: Thanh toán (Badge)
    GtkWidget *badgePay =
        UIComponents::createPaymentStatusBadge(ord.paymentStatus);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badgePay, kAdminOrderTableCols[5]));

    // 6: Tổng tiền (Căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(ord.total),
                                     kAdminOrderTableCols[6], "product-price"));

    // 7: Thao tác
    GtkWidget *btnDetail = UIComponents::createTableActionButton(
        "🔍 Chi tiết", "Xem thông tin chi tiết và xử lý đơn hàng",
        "btn-action-view");
    struct OrdClickData {
      AdminScreens *s;
      OrderItemViewModel o;
    };
    OrdClickData *ocd = new OrdClickData{this, ord};
    g_signal_connect_data(
        btnDetail, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          OrdClickData *data = static_cast<OrdClickData *>(d);
          data->s->showOrderDetailView(data->o);
        }),
        ocd,
        [](gpointer d, GClosure *) { delete static_cast<OrdClickData *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnDetail, kAdminOrderTableCols[7]));
    gtk_box_append(GTK_BOX(ordersListBox), row);
  }
}

void AdminScreens::showOrderDetailView(const OrderItemViewModel &ord) {
  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Chi tiết đơn hàng " + ord.id,
      "Xem danh sách mặt hàng, trạng thái thanh toán và cập nhật tiến trình "
      "đơn.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(card, "card");

  // Header info row
  GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  GtkWidget *bStatus = UIComponents::createOrderStatusBadge(ord.status);
  GtkWidget *bPay = UIComponents::createPaymentStatusBadge(ord.paymentStatus);
  GtkWidget *dateLbl = gtk_label_new(("Ngày đặt: " + ord.orderTime).c_str());
  gtk_widget_add_css_class(dateLbl, "stat-label");
  gtk_widget_add_css_class(dateLbl, "tabular-num");

  gtk_box_append(GTK_BOX(top), bStatus);
  gtk_box_append(GTK_BOX(top), bPay);
  gtk_box_append(GTK_BOX(top), dateLbl);
  gtk_box_append(GTK_BOX(card), top);

  // Customer info card
  GtkWidget *custBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  std::string cInfo = "Khách hàng: " + ord.customerName +
                      " | SĐT: " + ord.receiverPhone +
                      " | Địa chỉ nhận: " + ord.deliveryAddress;
  GtkWidget *cLbl = gtk_label_new(cInfo.c_str());
  gtk_widget_add_css_class(cLbl, "product-name");
  gtk_label_set_xalign(GTK_LABEL(cLbl), 0.0f);
  gtk_box_append(GTK_BOX(custBox), cLbl);
  gtk_box_append(GTK_BOX(card), custBox);

  // Items table
  GtkWidget *itemsTable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(itemsTable, "table-container");
  gtk_box_append(GTK_BOX(itemsTable),
                 UIComponents::createTableHeaderRow(kOrderDetailItemsCols));

  for (size_t i = 0; i < ord.details.size(); ++i) {
    const auto &item = ord.details[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       item.phoneName, kOrderDetailItemsCols[0], "cell-title"));
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(item.quantity) + " máy",
                       kOrderDetailItemsCols[1], "cell-number"));
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(item.unitPrice),
                                     kOrderDetailItemsCols[2], "tabular-num"));
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatVND(item.subtotal),
                       kOrderDetailItemsCols[3], "product-price"));

    gtk_box_append(GTK_BOX(itemsTable), row);
  }
  gtk_box_append(GTK_BOX(card), itemsTable);

  // Total summary
  GtkWidget *sumBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_set_halign(sumBox, GTK_ALIGN_END);
  gtk_widget_set_margin_top(sumBox, 10);

  GtkWidget *subTotLbl = gtk_label_new(
      ("Tạm tính: " + UIComponents::formatVND(ord.subtotal)).c_str());
  gtk_label_set_xalign(GTK_LABEL(subTotLbl), 1.0f);
  gtk_box_append(GTK_BOX(sumBox), subTotLbl);

  if (ord.discountAmount > 0) {
    GtkWidget *discLbl = gtk_label_new(
        ("Giảm giá voucher: -" + UIComponents::formatVND(ord.discountAmount))
            .c_str());
    gtk_widget_add_css_class(discLbl, "stat-trend");
    gtk_label_set_xalign(GTK_LABEL(discLbl), 1.0f);
    gtk_box_append(GTK_BOX(sumBox), discLbl);
  }

  GtkWidget *finalTotLbl = gtk_label_new(
      ("TỔNG CỘNG THANH TOÁN: " + UIComponents::formatVND(ord.total)).c_str());
  gtk_widget_add_css_class(finalTotLbl, "stat-value");
  gtk_label_set_xalign(GTK_LABEL(finalTotLbl), 1.0f);
  gtk_box_append(GTK_BOX(sumBox), finalTotLbl);
  gtk_box_append(GTK_BOX(card), sumBox);

  // Status update actions
  GtkWidget *actBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(actBox, GTK_ALIGN_END);
  gtk_widget_set_margin_top(actBox, 16);

  GtkWidget *btnBack = gtk_button_new_with_label("← Quay lại");
  gtk_widget_add_css_class(btnBack, "btn-secondary");
  UIComponents::setButtonCursor(btnBack);
  g_signal_connect_swapped(btnBack, "clicked", G_CALLBACK(+[](AdminScreens *s) {
                             if (s->shell)
                               s->shell->popPage();
                           }),
                           this);
  gtk_box_append(GTK_BOX(actBox), btnBack);

  auto addStatusBtn = [this, ord, actBox](const char *label,
                                          OrderStatus targetStatus,
                                          const char *cssClass) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_add_css_class(btn, cssClass);
    UIComponents::setButtonCursor(btn);
    struct OrdStatusCtx {
      AdminScreens *s;
      std::string ordId;
      OrderStatus st;
    };
    OrdStatusCtx *ctx = new OrdStatusCtx{this, ord.id, targetStatus};
    g_signal_connect_data(
        btn, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          OrdStatusCtx *c = static_cast<OrdStatusCtx *>(d);
          std::string title;
          std::string msg;
          if (c->st == OrderStatus::CANCELLED) {
            title = "Xác nhận hủy đơn hàng";
            msg = "Bạn có chắc chắn muốn hủy đơn hàng " + c->ordId +
                  " không? Thao tác này sẽ hủy đơn và cập nhật trạng thái.";
          } else if (c->st == OrderStatus::CONFIRMED) {
            title = "Xác nhận duyệt đơn hàng";
            msg = "Bạn có chắc chắn muốn duyệt và xác nhận đơn hàng " +
                  c->ordId + " không?";
          } else if (c->st == OrderStatus::SHIPPING) {
            title = "Xác nhận giao hàng";
            msg = "Bạn có chắc chắn muốn chuyển đơn hàng " + c->ordId +
                  " sang trạng thái đang giao hàng không?";
          } else if (c->st == OrderStatus::COMPLETED) {
            title = "Xác nhận hoàn tất đơn hàng";
            msg = "Bạn có chắc chắn muốn hoàn tất giao hàng và thu tiền cho "
                  "đơn " +
                  c->ordId + " không?";
          } else {
            title = "Xác nhận cập nhật trạng thái";
            msg = "Bạn có chắc muốn cập nhật trạng thái cho đơn hàng " +
                  c->ordId + " không?";
          }

          UIComponents::showConfirmDialog(
              c->s->shell ? c->s->shell->getWindow() : nullptr, title, msg,
              +[](gpointer data) {
                OrdStatusCtx *inner = static_cast<OrdStatusCtx *>(data);
                OperationResult res = inner->s->controller->updateOrderStatus(
                    inner->ordId, inner->st);
                if (res.success) {
                  inner->s->refreshOrders();
                  if (inner->s->shell)
                    inner->s->shell->popPage();
                  UIComponents::showSuccessDialog(nullptr, "Thành công",
                                                  res.message);
                } else {
                  UIComponents::showErrorDialog(nullptr, "Lỗi", res.message);
                }
              },
              c);
        }),
        ctx,
        [](gpointer d, GClosure *) { delete static_cast<OrdStatusCtx *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actBox), btn);
  };

  if (ord.status == OrderStatus::PENDING) {
    addStatusBtn("✅ Xác nhận đơn", OrderStatus::CONFIRMED, "btn-primary");
    addStatusBtn("❌ Hủy đơn", OrderStatus::CANCELLED, "btn-danger");
  } else if (ord.status == OrderStatus::CONFIRMED ||
             ord.status == OrderStatus::PREPARING) {
    addStatusBtn("🚚 Giao hàng", OrderStatus::SHIPPING, "btn-primary");
    addStatusBtn("❌ Hủy đơn", OrderStatus::CANCELLED, "btn-danger");
  } else if (ord.status == OrderStatus::SHIPPING) {
    addStatusBtn("🎉 Hoàn tất giao & Thu tiền", OrderStatus::COMPLETED,
                 "btn-success");
    addStatusBtn("❌ Hủy đơn", OrderStatus::CANCELLED, "btn-danger");
  }

  gtk_box_append(GTK_BOX(card), actBox);

  gtk_box_append(GTK_BOX(content), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), content);
  if (shell) {
    shell->pushPage("admin_order_detail", scroll, "Chi tiết đơn hàng " + ord.id,
                    false);
  }
}

// ================= 9. QUẢN LÝ HÓA ĐƠN =================

void AdminScreens::buildAllInvoicesView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh sách hóa đơn bán hàng",
      "Lịch sử hóa đơn bán lẻ và chứng từ thanh toán.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã hóa đơn, mã đơn hàng, tên khách hàng hoặc thu ngân...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->invoiceFilterKeyword = txt ? txt : "";
        self->refreshInvoices();
      }),
      this, nullptr, &invoiceCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kAdminInvoiceTableCols));

  invoiceListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), invoiceListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(allInvoicesView), content);
  refreshInvoices();
}

void AdminScreens::refreshInvoices() {
  if (!invoiceListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(invoiceListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(invoiceListBox), child);
    child = next;
  }

  std::vector<InvoiceItemViewModel> allList = controller->getAllInvoices();
  std::vector<InvoiceItemViewModel> list;
  for (const auto &inv : allList) {
    if (!invoiceFilterKeyword.empty()) {
      bool match =
          Utf8Helper::containsCaseInsensitive(inv.id, invoiceFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(inv.orderId,
                                              invoiceFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(inv.customerName,
                                              invoiceFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(inv.employeeName,
                                              invoiceFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(inv.paymentMethodText,
                                              invoiceFilterKeyword);
      if (!match)
        continue;
    }
    list.push_back(inv);
  }

  UIComponents::updateRecordCountBadge(invoiceCountBadge, list.size(),
                                       "hóa đơn");

  if (list.empty()) {
    gtk_box_append(
        GTK_BOX(invoiceListBox),
        UIComponents::createEmptyState(
            "🧾",
            invoiceFilterKeyword.empty() ? "Chưa có dữ liệu"
                                         : "Không tìm thấy hóa đơn phù hợp",
            invoiceFilterKeyword.empty()
                ? "Chưa có hóa đơn bán hàng nào trong hệ thống."
                : "Thử tìm kiếm với mã hóa đơn hoặc tên khách hàng khác."));
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &inv = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã hóa đơn
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       inv.id, kAdminInvoiceTableCols[0], "cell-code-inv"));

    // 1: Mã đơn hàng
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     inv.orderId, kAdminInvoiceTableCols[1],
                                     "cell-code-ord"));

    // 2: Khách hàng
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     inv.customerName,
                                     kAdminInvoiceTableCols[2], "cell-title"));

    // 3: Thu ngân
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(inv.employeeName,
                                                     kAdminInvoiceTableCols[3],
                                                     "cell-subtitle"));

    // 4: Phương thức (Badge)
    GtkWidget *payBadge = UIComponents::createStatusBadge(inv.paymentMethodText,
                                                          "badge-confirmed");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     payBadge, kAdminInvoiceTableCols[4]));

    // 5: Thời gian
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     inv.createTime, kAdminInvoiceTableCols[5],
                                     "tabular-num"));

    // 6: Tổng tiền (Căn phải)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatVND(inv.total),
                       kAdminInvoiceTableCols[6], "product-price"));

    gtk_box_append(GTK_BOX(invoiceListBox), row);
  }
}

// ================= 10. GIÁM SÁT BẢO HÀNH =================

void AdminScreens::buildWarrantyMonitorView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Giám sát dịch vụ bảo hành",
      "Theo dõi tiếp nhận và xử lý bảo hành thiết bị.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã phiếu, thiết bị, tên khách hàng hoặc mô tả sự cố...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AdminScreens *self = static_cast<AdminScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->warrantyFilterKeyword = txt ? txt : "";
        self->refreshWarrantyMonitor();
      }),
      this, nullptr, &warrantyCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kAdminWarrantyTableCols));

  warrantyListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), warrantyListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(warrantyMonitorView),
                                content);
  refreshWarrantyMonitor();
}

void AdminScreens::refreshWarrantyMonitor() {
  if (!warrantyListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(warrantyListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(warrantyListBox), child);
    child = next;
  }

  std::vector<WarrantyItemViewModel> allList = controller->getWarranties();
  std::vector<WarrantyItemViewModel> list;
  for (const auto &w : allList) {
    if (!warrantyFilterKeyword.empty()) {
      bool match =
          Utf8Helper::containsCaseInsensitive(w.id, warrantyFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.phoneName,
                                              warrantyFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.customerName,
                                              warrantyFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.customerPhone,
                                              warrantyFilterKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.issueDescription,
                                              warrantyFilterKeyword);
      if (!match)
        continue;
    }
    list.push_back(w);
  }

  UIComponents::updateRecordCountBadge(warrantyCountBadge, list.size(),
                                       "phiếu bảo hành");

  if (list.empty()) {
    gtk_box_append(
        GTK_BOX(warrantyListBox),
        UIComponents::createEmptyState(
            "🛡️",
            warrantyFilterKeyword.empty()
                ? "Chưa có dữ liệu"
                : "Không tìm thấy phiếu bảo hành phù hợp",
            warrantyFilterKeyword.empty()
                ? "Chưa có yêu cầu hoặc biên nhận bảo hành nào trong hệ thống."
                : "Thử tìm kiếm với từ khóa hoặc mã phiếu khác."));
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &w = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã phiếu
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.id, kAdminWarrantyTableCols[0], "cell-code"));

    // 1: Thiết bị
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.phoneName, kAdminWarrantyTableCols[1], "cell-title"));

    // 2: Khách hàng (tên + SĐT)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     w.customerName, w.customerPhone,
                                     kAdminWarrantyTableCols[2]));

    // 3: Ngày hẹn
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     w.requestDate, kAdminWarrantyTableCols[3],
                                     "tabular-num"));

    // 4: Trạng thái (Badge)
    GtkWidget *badge = UIComponents::createWarrantyStatusBadge(w.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     badge, kAdminWarrantyTableCols[4]));

    // 5: Mô tả sự cố
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(w.issueDescription,
                                                     kAdminWarrantyTableCols[5],
                                                     "cell-subtitle"));

    gtk_box_append(GTK_BOX(warrantyListBox), row);
  }
}

// ================= 11. BÁO CÁO THỐNG KÊ DOANH THU & LỢI NHUẬN
// =================

void AdminScreens::buildReportsView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Báo cáo Thống kê & Phân tích Lợi nhuận",
      "Báo cáo tài chính, doanh thu bán hàng và phân tích tồn kho.");
  gtk_box_append(GTK_BOX(content), header);

  // Thanh lọc báo cáo theo khoảng thời gian
  GtkWidget *filterBar =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(filterBar, "card");

  GtkWidget *eFrom = gtk_entry_new();
  gtk_widget_set_name(eFrom, "report_from_date_entry");
  gtk_entry_set_placeholder_text(GTK_ENTRY(eFrom), "DD/MM/YYYY");
  gtk_editable_set_text(GTK_EDITABLE(eFrom), reportFromDate.c_str());
  GtkWidget *fromField = UIComponents::createFormField("Từ ngày", eFrom);
  gtk_box_append(GTK_BOX(filterBar), fromField);

  GtkWidget *eTo = gtk_entry_new();
  gtk_widget_set_name(eTo, "report_to_date_entry");
  gtk_entry_set_placeholder_text(GTK_ENTRY(eTo), "DD/MM/YYYY");
  gtk_editable_set_text(GTK_EDITABLE(eTo), reportToDate.c_str());
  GtkWidget *toField = UIComponents::createFormField("Đến ngày", eTo);
  gtk_box_append(GTK_BOX(filterBar), toField);

  struct ReportFilterCtx {
    AdminScreens *s;
    GtkWidget *eFrom;
    GtkWidget *eTo;
  };
  ReportFilterCtx *fctx = new ReportFilterCtx{this, eFrom, eTo};

  GtkWidget *btnFilter = gtk_button_new_with_label("🔍 Lọc báo cáo");
  gtk_widget_add_css_class(btnFilter, "btn-primary");
  gtk_widget_set_valign(btnFilter, GTK_ALIGN_END);
  UIComponents::setButtonCursor(btnFilter);
  g_signal_connect_data(
      btnFilter, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
        ReportFilterCtx *c = static_cast<ReportFilterCtx *>(d);
        auto trimText = [](const char *raw) {
          std::string t = raw ? raw : "";
          size_t b = t.find_first_not_of(" \t");
          if (b == std::string::npos)
            return std::string();
          size_t e = t.find_last_not_of(" \t");
          return t.substr(b, e - b + 1);
        };
        std::string from =
            trimText(gtk_editable_get_text(GTK_EDITABLE(c->eFrom)));
        std::string to = trimText(gtk_editable_get_text(GTK_EDITABLE(c->eTo)));
        AdminScreens *screens = c->s;
        OperationResult res =
            screens->controller->validateReportRange(from, to);
        if (!res.success) {
          UIComponents::showErrorDialog(screens->shell->getWindow(),
                                        "Khoảng thời gian không hợp lệ",
                                        res.message);
          return;
        }
        screens->reportFromDate = from;
        screens->reportToDate = to;
        screens->refreshReports();
      }),
      fctx,
      [](gpointer d, GClosure *) { delete static_cast<ReportFilterCtx *>(d); },
      (GConnectFlags)0);
  auto onActivateFilter =
      +[](GtkEntry *, gpointer btn) { g_signal_emit_by_name(btn, "clicked"); };
  g_signal_connect(eFrom, "activate", G_CALLBACK(onActivateFilter), btnFilter);
  g_signal_connect(eTo, "activate", G_CALLBACK(onActivateFilter), btnFilter);
  gtk_box_append(GTK_BOX(filterBar), btnFilter);

  GtkWidget *btnResetFilter = gtk_button_new_with_label("↺ Đặt lại");
  gtk_widget_add_css_class(btnResetFilter, "btn-secondary");
  gtk_widget_set_valign(btnResetFilter, GTK_ALIGN_END);
  UIComponents::setButtonCursor(btnResetFilter);
  g_signal_connect_swapped(btnResetFilter, "clicked",
                           G_CALLBACK(+[](AdminScreens *s) {
                             s->reportFromDate.clear();
                             s->reportToDate.clear();
                             s->refreshReports();
                           }),
                           this);
  gtk_box_append(GTK_BOX(filterBar), btnResetFilter);
  gtk_box_append(GTK_BOX(content), filterBar);

  std::string rangeText = "Đang xem: ";
  if (reportFromDate.empty() && reportToDate.empty()) {
    rangeText += "Toàn thời gian";
  } else {
    rangeText +=
        "từ " +
        (reportFromDate.empty() ? std::string("đầu kỳ") : reportFromDate) +
        " đến " +
        (reportToDate.empty() ? std::string("hiện tại") : reportToDate);
  }
  GtkWidget *lblRange = gtk_label_new(rangeText.c_str());
  gtk_widget_add_css_class(lblRange, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(lblRange), 0.0f);
  gtk_box_append(GTK_BOX(content), lblRange);

  DashboardStatsViewModel stats =
      controller->getDashboardStats(reportFromDate, reportToDate);

  // Summary KPIs
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

  GtkWidget *c1 = UIComponents::createStatCard(
      "Tổng doanh thu thực tế", UIComponents::formatVND(stats.totalRevenue),
      "Đơn đã giao thành công", "💰", "green");
  GtkWidget *c2 = UIComponents::createStatCard(
      "Lợi nhuận gộp ước tính", UIComponents::formatVND(stats.estimatedProfit),
      "Doanh thu − Giá vốn", "📈", "purple");
  GtkWidget *c3 = UIComponents::createStatCard(
      "Tỷ suất lợi nhuận gộp",
      (stats.totalRevenue > 0
           ? (std::to_string(static_cast<int>(
                  (stats.estimatedProfit / stats.totalRevenue) * 100)) +
              "%")
           : "0%"),
      "Biên lợi nhuận gộp", "📊", "blue");

  gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), grid);

  // Hàng biểu đồ phân tích kinh doanh (Grid 2 cột: Cột trái 65% - Cột phải 35%) (DESIGN.md T16)
  GtkWidget *chartGrid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(chartGrid), DesignTokens::SPACE_LG);
  gtk_grid_set_column_spacing(GTK_GRID(chartGrid), DesignTokens::SPACE_LG);
  gtk_grid_set_column_homogeneous(GTK_GRID(chartGrid), FALSE);

  GtkWidget *revChart = UIComponents::createRevenueBarChart(stats.timelineStats);
  gtk_widget_set_hexpand(revChart, TRUE);

  GtkWidget *brandChart = UIComponents::createBrandDistributionCard(stats.brandStats);
  gtk_widget_set_hexpand(brandChart, FALSE);
  gtk_widget_set_size_request(brandChart, 380, -1);

  gtk_grid_attach(GTK_GRID(chartGrid), revChart, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(chartGrid), brandChart, 1, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), chartGrid);

  // Top Selling Products Table Card
  GtkWidget *topCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(topCard, "card");

  GtkWidget *lblTopHeader = gtk_label_new("🏆 Top sản phẩm bán chạy nhất");
  gtk_widget_add_css_class(lblTopHeader, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblTopHeader), 0.0f);
  gtk_box_append(GTK_BOX(topCard), lblTopHeader);

  GtkWidget *topTable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(topTable, "table-container");
  gtk_box_append(GTK_BOX(topTable),
                 UIComponents::createTableHeaderRow(kReportTopProdCols));

  if (stats.topProducts.empty()) {
    gtk_box_append(
        GTK_BOX(topTable),
        UIComponents::createEmptyState(
            "📊", "Chưa có dữ liệu doanh số",
            "Không có sản phẩm bán ra trong khoảng thời gian đã chọn."));
  } else {
    for (size_t i = 0; i < stats.topProducts.size(); ++i) {
      const auto &item = stats.topProducts[i];
      GtkWidget *row = UIComponents::createTableDataRow(i);

      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableTextCell(
                         item.phoneName, kReportTopProdCols[0], "cell-title"));
      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableTextCell(
                         item.brand, kReportTopProdCols[1], "cell-subtitle"));
      gtk_box_append(GTK_BOX(row),
                     UIComponents::createTableTextCell(
                         UIComponents::formatNumber(item.soldQuantity) + " máy",
                         kReportTopProdCols[2], "cell-number"));
      gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                       UIComponents::formatVND(item.revenue),
                                       kReportTopProdCols[3], "product-price"));
      gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                       UIComponents::formatVND(item.profit),
                                       kReportTopProdCols[4], "stock-ok"));

      gtk_box_append(GTK_BOX(topTable), row);
    }
  }

  gtk_box_append(GTK_BOX(topCard), topTable);
  gtk_box_append(GTK_BOX(content), topCard);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(reportsView), content);
}

void AdminScreens::refreshReports() { buildReportsView(); }

// ================= 12. HỆ THỐNG KIỂM SOÁT THỜI GIAN THỰC (REAL-TIME
// SIMULATION) =================

void AdminScreens::buildRealTimeView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "⏱️ Kiểm soát & Quản lý Thời gian thực Hệ thống",
      "Kiểm soát mốc thời gian vận hành và đối soát toàn hệ thống.");
  gtk_box_append(GTK_BOX(content), header);

  // Large Digital Clock Banner
  GtkWidget *clockCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(clockCard, "realtime-clock-box");

  GtkWidget *lblClockTitle =
      gtk_label_new("MỐC THỜI GIAN THỰC HIỆN TẠI (SIMULATED REALTIME)");
  gtk_widget_add_css_class(lblClockTitle, "stat-label");
  gtk_label_set_xalign(GTK_LABEL(lblClockTitle), 0.0f);
  gtk_box_append(GTK_BOX(clockCard), lblClockTitle);

  std::string currentDt = controller->getSimulatedTime();
  lblClockDisplay = gtk_label_new(currentDt.c_str());
  GtkWidget *lblClockTime = lblClockDisplay;
  gtk_widget_add_css_class(lblClockTime, "realtime-clock-display");
  gtk_widget_add_css_class(lblClockTime, "tabular-num");
  gtk_label_set_xalign(GTK_LABEL(lblClockTime), 0.0f);
  gtk_box_append(GTK_BOX(clockCard), lblClockTime);

  GtkWidget *lblClockPath =
      gtk_label_new("Đang đồng bộ hóa trực tiếp với file: data/realTime.txt");
  gtk_widget_add_css_class(lblClockPath, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(lblClockPath), 0.0f);
  gtk_box_append(GTK_BOX(clockCard), lblClockPath);

  gtk_box_append(GTK_BOX(content), clockCard);

  // Control Adjustment Card
  GtkWidget *adjCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
  gtk_widget_add_css_class(adjCard, "card");

  GtkWidget *lblAdjTitle = gtk_label_new("⚙️ Điều chỉnh ngày giờ hệ thống");
  gtk_widget_add_css_class(lblAdjTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblAdjTitle), 0.0f);
  gtk_box_append(GTK_BOX(adjCard), lblAdjTitle);

  GtkWidget *rowInputs = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
  GtkWidget *eDate = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(eDate),
                        controller->getSimulatedDate().c_str());
  gtk_entry_set_placeholder_text(GTK_ENTRY(eDate),
                                 "YYYY-MM-DD (vd: 2026-10-17)");

  GtkWidget *eTime = gtk_entry_new();
  std::string timePart =
      (currentDt.length() >= 19) ? currentDt.substr(11, 8) : "11:30:00";
  gtk_editable_set_text(GTK_EDITABLE(eTime), timePart.c_str());
  gtk_entry_set_placeholder_text(GTK_ENTRY(eTime), "HH:MM:SS (vd: 11:30:00)");

  gtk_box_append(
      GTK_BOX(rowInputs),
      UIComponents::createFormField("Ngày mô phỏng (YYYY-MM-DD) *", eDate));
  gtk_box_append(GTK_BOX(rowInputs), UIComponents::createFormField(
                                         "Giờ mô phỏng (HH:MM:SS) *", eTime));

  struct TimeSaveCtx {
    AdminScreens *s;
    GtkWidget *eD;
    GtkWidget *eT;
  };
  TimeSaveCtx *tsctx = new TimeSaveCtx{this, eDate, eTime};

  GtkWidget *btnSaveTime = gtk_button_new_with_label("💾 Lưu mốc thời gian");
  gtk_widget_add_css_class(btnSaveTime, "btn-primary");
  UIComponents::setButtonCursor(btnSaveTime);
  gtk_widget_set_valign(btnSaveTime, GTK_ALIGN_END);
  g_signal_connect_data(
      btnSaveTime, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        TimeSaveCtx *c = static_cast<TimeSaveCtx *>(d);
        const char *dStr = gtk_editable_get_text(GTK_EDITABLE(c->eD));
        const char *tStr = gtk_editable_get_text(GTK_EDITABLE(c->eT));
        if (dStr && strlen(dStr) >= 10) {
          std::string full = std::string(dStr) + " " +
                             (tStr && strlen(tStr) >= 5 ? tStr : "00:00:00");
          OperationResult res = c->s->controller->setSimulatedTime(full);
          UIComponents::showSuccessDialog(nullptr, "Đã lưu", res.message);
          c->s->refreshAll();
        }
      }),
      tsctx,
      [](gpointer d, GClosure *) { delete static_cast<TimeSaveCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(rowInputs), btnSaveTime);
  gtk_box_append(GTK_BOX(adjCard), rowInputs);

  // Fast shift buttons
  GtkWidget *shiftRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *lblShift = gtk_label_new("Phím tắt tua thời gian:");
  gtk_widget_add_css_class(lblShift, "stat-label");
  gtk_box_append(GTK_BOX(shiftRow), lblShift);

  auto addShiftBtn = [this, shiftRow](const char *label, int days) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_add_css_class(btn, "btn-outline");
    UIComponents::setButtonCursor(btn);
    struct ShiftData {
      AdminScreens *s;
      int d;
    };
    ShiftData *sd = new ShiftData{this, days};
    g_signal_connect_data(
        btn, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          ShiftData *data = static_cast<ShiftData *>(d);
          data->s->controller->advanceSimulatedDays(data->d);
          data->s->refreshAll();
        }),
        sd, [](gpointer d, GClosure *) { delete static_cast<ShiftData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(shiftRow), btn);
  };

  addShiftBtn("⏪ Tua lùi -7 ngày", -7);
  addShiftBtn("⏩ Tua tới +7 ngày", 7);
  addShiftBtn("⏩ Tua tới +30 ngày", 30);
  addShiftBtn("⏩ Tua tới +1 năm", 365);

  GtkWidget *btnResetNow =
      gtk_button_new_with_label("🔄 Đồng bộ theo giờ máy tính");
  gtk_widget_add_css_class(btnResetNow, "btn-secondary");
  UIComponents::setButtonCursor(btnResetNow);
  g_signal_connect_swapped(btnResetNow, "clicked",
                           G_CALLBACK(+[](AdminScreens *s) {
                             s->controller->resetSimulatedTimeToNow();
                             s->refreshAll();
                           }),
                           this);
  gtk_box_append(GTK_BOX(shiftRow), btnResetNow);

  gtk_box_append(GTK_BOX(adjCard), shiftRow);
  gtk_box_append(GTK_BOX(content), adjCard);

  // Live Voucher Reconciliation Table
  GtkWidget *reconCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(reconCard, "card");

  GtkWidget *lblReconHeader =
      gtk_label_new("📋 Bảng đối soát hiệu lực Voucher theo mốc thời gian này");
  gtk_widget_add_css_class(lblReconHeader, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblReconHeader), 0.0f);
  gtk_box_append(GTK_BOX(reconCard), lblReconHeader);

  realTimeReconcileBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(realTimeReconcileBox, "table-container");
  gtk_box_append(GTK_BOX(reconCard), realTimeReconcileBox);
  gtk_box_append(GTK_BOX(content), reconCard);

  std::vector<DiscountItemViewModel> list = controller->getDiscounts();
  for (size_t i = 0; i < list.size(); ++i) {
    const auto &d = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    GtkWidget *codeBadge =
        UIComponents::createStatusBadge(d.code, "badge-confirmed");
    GtkWidget *descLbl = gtk_label_new(d.description.c_str());
    gtk_widget_add_css_class(descLbl, "cell-title");
    gtk_widget_set_hexpand(descLbl, TRUE);
    gtk_label_set_xalign(GTK_LABEL(descLbl), 0.0f);

    GtkWidget *expLbl = gtk_label_new(("HSD: " + d.expiryDate).c_str());
    gtk_widget_add_css_class(expLbl, "cell-subtitle");
    gtk_widget_add_css_class(expLbl, "tabular-num");

    int remDays = RealTimeManager::getInstance().getRemainingDays(d.expiryDate);
    std::string st =
        d.isValid ? ("🟢 Hợp lệ (còn " + std::to_string(remDays) + " ngày)")
                  : ("🔴 Hết hạn (quá hạn " +
                     std::to_string(std::abs(remDays)) + " ngày)");
    GtkWidget *badgeSt = UIComponents::createStatusBadge(
        st, d.isValid ? "badge-completed" : "badge-cancelled");

    gtk_box_append(GTK_BOX(row), codeBadge);
    gtk_box_append(GTK_BOX(row), descLbl);
    gtk_box_append(GTK_BOX(row), expLbl);
    gtk_box_append(GTK_BOX(row), badgeSt);

    gtk_box_append(GTK_BOX(realTimeReconcileBox), row);
  }

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(realTimeView), content);
}

void AdminScreens::refreshRealTime() { buildRealTimeView(); }

void AdminScreens::updateClockDisplay(const std::string &timeStr) {
  if (lblClockDisplay && GTK_IS_LABEL(lblClockDisplay)) {
    gtk_label_set_text(GTK_LABEL(lblClockDisplay), timeStr.c_str());
  }
}

// ================= 13. HỒ SƠ QUẢN TRỊ VIÊN =================

void AdminScreens::buildAdminProfileView() {
  GtkWidget *child =
      gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(adminProfileView));
  if (child) {
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(adminProfileView),
                                  nullptr);
  }

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Hồ sơ tài khoản Quản trị viên",
      "Quản lý thông tin định danh, bảo mật tài khoản và phân quyền hệ thống.");
  gtk_box_append(GTK_BOX(content), header);

  const UserSessionViewModel &user = controller->getCurrentUser();

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::SPACE_XL);
  gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::SPACE_XL);

  // Left Card: Thông tin tài khoản & chỉnh sửa hồ sơ
  GtkWidget *formCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(formCard, "card");
  gtk_widget_set_hexpand(formCard, TRUE);

  GtkWidget *lblFormTitle = gtk_label_new("Chỉnh sửa thông tin quản trị");
  gtk_widget_add_css_class(lblFormTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblFormTitle), 0.0f);
  gtk_box_append(GTK_BOX(formCard), lblFormTitle);

  // Username (Read-only)
  GtkWidget *entryUser = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryUser), user.username.c_str());
  gtk_widget_set_sensitive(entryUser, FALSE);
  gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField(
                                        "Tên đăng nhập hệ thống", entryUser));

  // Mã nhân sự / ID (Read-only)
  GtkWidget *entryId = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryId), user.userId.c_str());
  gtk_widget_set_sensitive(entryId, FALSE);
  gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField(
                                        "Mã định danh (User ID)", entryId));

  // Họ và tên
  GtkWidget *entryName = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryName), user.fullName.c_str());
  gtk_box_append(GTK_BOX(formCard),
                 UIComponents::createFormField("Họ và tên *", entryName));

  // Số điện thoại
  GtkWidget *entryPhone = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryPhone), user.phone.c_str());
  gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField(
                                        "Số điện thoại liên hệ *", entryPhone));

  // Email
  GtkWidget *entryEmail = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryEmail), user.email.c_str());
  gtk_box_append(GTK_BOX(formCard),
                 UIComponents::createFormField("Địa chỉ Email *", entryEmail));

  // Địa chỉ
  GtkWidget *entryAddr = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryAddr), user.address.c_str());
  gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField(
                                        "Địa chỉ cơ quan / Nơi ở", entryAddr));

  // Nút hành động
  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_margin_top(btnRow, 10);

  GtkWidget *btnSave = gtk_button_new_with_label("💾 Lưu thay đổi hồ sơ");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  UIComponents::setButtonCursor(btnSave);

  struct AdminProfileSaveCtx {
    AdminScreens *screens;
    GtkWidget *eName;
    GtkWidget *ePhone;
    GtkWidget *eEmail;
    GtkWidget *eAddr;
  };
  AdminProfileSaveCtx *pctx = new AdminProfileSaveCtx{
      this, entryName, entryPhone, entryEmail, entryAddr};
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        AdminProfileSaveCtx *ctx = static_cast<AdminProfileSaveCtx *>(d);
        const char *n = gtk_editable_get_text(GTK_EDITABLE(ctx->eName));
        const char *p = gtk_editable_get_text(GTK_EDITABLE(ctx->ePhone));
        const char *e = gtk_editable_get_text(GTK_EDITABLE(ctx->eEmail));
        const char *a = gtk_editable_get_text(GTK_EDITABLE(ctx->eAddr));

        OperationResult res = ctx->screens->controller->updateProfile(
            n ? n : "", p ? p : "", e ? e : "", a ? a : "");
        if (res.success) {
          UIComponents::showSuccessDialog(
              ctx->screens->shell ? ctx->screens->shell->getWindow() : nullptr,
              "Thành công", res.message);
          ctx->screens->refreshProfile();
        } else {
          UIComponents::showErrorDialog(
              ctx->screens->shell ? ctx->screens->shell->getWindow() : nullptr,
              "Lỗi", res.message);
        }
      }),
      pctx,
      [](gpointer d, GClosure *) {
        delete static_cast<AdminProfileSaveCtx *>(d);
      },
      (GConnectFlags)0);
  gtk_box_append(GTK_BOX(btnRow), btnSave);

  GtkWidget *btnChangePw = gtk_button_new_with_label("🔑 Đổi mật khẩu");
  gtk_widget_add_css_class(btnChangePw, "btn-secondary");
  UIComponents::setButtonCursor(btnChangePw);
  g_signal_connect_swapped(
      btnChangePw, "clicked", G_CALLBACK(+[](AdminScreens *s) {
        UIComponents::showChangePasswordDialog(
            s->shell ? s->shell->getWindow() : nullptr, s->controller);
      }),
      this);
  gtk_box_append(GTK_BOX(btnRow), btnChangePw);

  gtk_box_append(GTK_BOX(formCard), btnRow);
  gtk_grid_attach(GTK_GRID(grid), formCard, 0, 0, 1, 1);

  // Right Card: Quyền hạn & Phân hệ quản trị
  GtkWidget *permCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(permCard, "card");
  gtk_widget_set_size_request(permCard, 380, -1);

  GtkWidget *lblPermTitle = gtk_label_new("Phân quyền & Bảo mật hệ thống");
  gtk_widget_add_css_class(lblPermTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblPermTitle), 0.0f);
  gtk_box_append(GTK_BOX(permCard), lblPermTitle);

  GtkWidget *roleBadge = UIComponents::createStatusBadge(
      "🛡️ Quản trị viên cấp cao (ADMIN)", "badge-confirmed");
  gtk_box_append(GTK_BOX(permCard), roleBadge);

  GtkWidget *permStat = UIComponents::createStatCard(
      "Phạm vi quyền hạn", "Toàn quyền quản trị (SUPER_ADMIN)",
      "Quản lý toàn bộ dữ liệu & phân hệ", "🔐", "purple");
  gtk_box_append(GTK_BOX(permCard), permStat);

  GtkWidget *lblDetails =
      gtk_label_new("Đặc quyền tài khoản Quản trị viên:\n"
                    "• Quản lý danh mục sản phẩm, biến thể và mức giá\n"
                    "• Quản lý danh sách khách hàng và điểm tích lũy\n"
                    "• Quản lý nhân sự, phân quyền và khóa/mở tài khoản\n"
                    "• Quản lý nhà cung cấp và nhập kho hàng hóa\n"
                    "• Kiểm soát mã giảm giá và đối soát thời gian thực\n"
                    "• Giám sát đơn hàng, hóa đơn và bảo hành thiết bị\n"
                    "• Xem báo cáo phân tích doanh thu và lợi nhuận");
  gtk_widget_add_css_class(lblDetails, "page-subtitle");
  gtk_label_set_wrap(GTK_LABEL(lblDetails), TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblDetails), 0.0f);
  gtk_box_append(GTK_BOX(permCard), lblDetails);

  gtk_grid_attach(GTK_GRID(grid), permCard, 1, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), grid);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(adminProfileView), content);
}

void AdminScreens::refreshProfile() { buildAdminProfileView(); }
