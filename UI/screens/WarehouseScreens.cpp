/**
 * @file WarehouseScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ QUẢN LÝ KHO (WAREHOUSE MANAGEMENT)
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildDashboardView & refreshDashboard: Bàn làm việc Thủ kho, giám sát tổng
 * giá trị tồn và đơn chờ xuất.
 * - buildInventoryView & refreshInventory: Danh mục tồn kho sản phẩm, bộ lọc
 * hãng và tìm kiếm từ khóa.
 * - buildLowStockView & refreshLowStock: Cảnh báo tự động các sản phẩm có tồn
 * kho dưới ngưỡng an toàn.
 * - buildReceiptsView & refreshReceipts: Xem danh sách và chi tiết phiếu nhập
 * kho từ nhà cung cấp.
 * - buildCreateReceiptView: Nhập hàng mới, chọn nhà cung cấp, thêm chi tiết sản
 * phẩm và tính toán tổng tiền.
 * - buildFulfillmentView: Danh sách đóng gói đơn hàng online (chuyển trạng thái
 * sang SHIPPING).
 * =========================================================================================
 */

#include "WarehouseScreens.h"
#include "../../source/utils/Utf8Helper.h"
#include "../components/UIComponents.h"
#include "../styles/AppCss.h"
#include "MainShellWindow.h"


namespace {
const std::vector<UIComponents::TableColumnSpec> kWarehouseInvCols = {
    {"Tên thiết bị & Mã", 220, 0.0f, true}, {"Hãng", 100, 0.5f, false},
    {"Tình trạng", 130, 0.5f, false},       {"Giá nhập", 130, 1.0f, false},
    {"Thực tế", 96, 1.0f, false},           {"Giữ chỗ", 96, 1.0f, false},
    {"Có thể bán", 110, 1.0f, false}};

const std::vector<UIComponents::TableColumnSpec> kWarehouseLowCols = {
    {"Mã SP", 110, 0.0f, false},
    {"Tên điện thoại & Cấu hình", 220, 0.0f, true},
    {"Hãng", 110, 0.5f, false},
    {"Giá nhập tham khảo", 150, 1.0f, false},
    {"Còn khả dụng", 130, 1.0f, false},
    {"Thao tác", 140, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kWarehouseReceiptCols = {
    {"Mã phiếu", 130, 0.0f, false},       {"Nhà cung cấp", 190, 0.0f, true},
    {"Người nhập", 150, 0.0f, false},     {"Thời gian nhập", 150, 0.0f, false},
    {"Tổng tiền nhập", 150, 1.0f, false}, {"Thao tác", 96, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kDraftReceiptCols = {
    {"Mã máy", 120, 0.0f, false},
    {"Tên điện thoại nhập", 200, 0.0f, true},
    {"Đơn giá nhập", 150, 1.0f, false},
    {"Số lượng", 160, 0.5f, false},
    {"Thành tiền", 160, 1.0f, false},
    {"Xóa", 96, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kWarehouseFulfillCols = {
    {"Mã đơn", 120, 0.0f, false},
    {"Thông tin người nhận", 180, 0.0f, false},
    {"Danh sách máy cần lấy", 220, 0.0f, true},
    {"Trạng thái", 130, 0.5f, false},
    {"Giao nhận", 130, 0.5f, false},
    {"Thao tác xử lý", 180, 0.5f, false}};
} // namespace

WarehouseScreens::WarehouseScreens(AppController *ctrl,
                                   MainShellWindow *parentShell)
    : controller(ctrl), shell(parentShell), draftSupplierId("") {

  dashboardView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  inventoryView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(inventoryView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  lowStockView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(lowStockView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  receiptsView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(receiptsView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  createReceiptView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(createReceiptView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  fulfillmentView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(fulfillmentView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  buildDashboardView();
  buildInventoryView();
  buildLowStockView();
  buildReceiptsView();
  buildCreateReceiptView();
  buildFulfillmentView();
}

void WarehouseScreens::refreshAll() {
  refreshDashboard();
  refreshInventory();
  refreshLowStock();
  refreshReceipts();
  refreshFulfillmentQueue();
}

// ================= 1. WAREHOUSE DASHBOARD =================

void WarehouseScreens::buildDashboardView() {

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  const UserSessionViewModel &user = controller->getCurrentUser();
  std::string title = "Bàn làm việc Nhân viên Kho — " + user.fullName;
  GtkWidget *header = UIComponents::createSectionHeader(
      title, "Quản lý tồn kho, xuất nhập hàng và đóng gói đơn hàng cần xử lý.");
  gtk_box_append(GTK_BOX(content), header);

  // Metrics Grid (Role-specific KPIs for WAREHOUSE_EMPLOYEE)
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

  DashboardStatsViewModel stats = controller->getDashboardStats();
  std::vector<GoodsReceiptSummaryViewModel> receipts =
      controller->getGoodsReceipts();
  std::vector<OrderItemViewModel> orders = controller->getAllOrders();

  int prepareCount = 0;
  for (const auto &o : orders) {
    if (o.status == OrderStatus::CONFIRMED ||
        o.status == OrderStatus::PREPARING) {
      prepareCount++;
    }
  }

  GtkWidget *c1 = UIComponents::createStatCard(
      "Tổng lượng máy trong kho",
      UIComponents::formatNumber(stats.totalStockQuantity) + " máy",
      "Giá trị lưu kho: " + UIComponents::formatVND(stats.totalInventoryValue),
      "📦", "purple");
  GtkWidget *c2 = UIComponents::createStatCard(
      "Cảnh báo sắp hết hàng",
      UIComponents::formatNumber(stats.lowStockCount) + " mặt hàng",
      stats.lowStockCount > 0 ? "Dưới ngưỡng an toàn (<= 5 máy)"
                              : "Toàn bộ tồn kho ổn định",
      "⚠️", stats.lowStockCount > 0 ? "red" : "green");
  GtkWidget *c3 = UIComponents::createStatCard(
      "Phiếu nhập kho", UIComponents::formatNumber(receipts.size()) + " phiếu",
      "Lịch sử nhập từ nhà cung cấp", "📥", "blue");
  GtkWidget *c4 = UIComponents::createStatCard(
      "Đơn cần đóng gói", UIComponents::formatNumber(prepareCount) + " đơn",
      prepareCount > 0 ? "Chờ xuất kho & đóng gói"
                       : "Đã xử lý hết hàng đợi xuất",
      "🚚", prepareCount > 0 ? "amber" : "cyan");

  gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), c4, 3, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), grid);

  // Quick Actions
  GtkWidget *actionCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(actionCard, "card");

  GtkWidget *lblActTitle = gtk_label_new("⚡ Thao tác nghiệp vụ kho");
  gtk_widget_add_css_class(lblActTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblActTitle), 0.0f);
  gtk_box_append(GTK_BOX(actionCard), lblActTitle);

  GtkWidget *btnRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);

  GtkWidget *bInv = gtk_button_new_with_label("📦 Kiểm tra tồn kho");
  gtk_widget_add_css_class(bInv, "btn-primary");
  UIComponents::setButtonCursor(bInv);
  g_signal_connect_swapped(bInv, "clicked", G_CALLBACK(+[](MainShellWindow *s) {
                             s->navigateTo("warehouse_inventory");
                           }),
                           shell);
  gtk_box_append(GTK_BOX(btnRow), bInv);

  GtkWidget *bRec = gtk_button_new_with_label("➕ Lập phiếu nhập kho");
  gtk_widget_add_css_class(bRec, "btn-success");
  UIComponents::setButtonCursor(bRec);
  g_signal_connect_swapped(bRec, "clicked", G_CALLBACK(+[](MainShellWindow *s) {
                             s->navigateTo("warehouse_create_receipt");
                           }),
                           shell);
  gtk_box_append(GTK_BOX(btnRow), bRec);

  GtkWidget *bFulfill = gtk_button_new_with_label("🚚 Chuẩn bị đơn hàng");
  gtk_widget_add_css_class(bFulfill, "btn-secondary");
  UIComponents::setButtonCursor(bFulfill);
  g_signal_connect_swapped(bFulfill, "clicked",
                           G_CALLBACK(+[](MainShellWindow *s) {
                             s->navigateTo("warehouse_fulfillment");
                           }),
                           shell);
  gtk_box_append(GTK_BOX(btnRow), bFulfill);

  gtk_box_append(GTK_BOX(actionCard), btnRow);
  gtk_box_append(GTK_BOX(content), actionCard);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(dashboardView), content);
}

void WarehouseScreens::refreshDashboard() { buildDashboardView(); }

// ================= 2. QUẢN LÝ TỒN KHO =================

void WarehouseScreens::buildInventoryView() {
  inventoryView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(inventoryView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh sách tồn kho thiết bị",
      "Theo dõi số lượng thực tế, số lượng đã giữ chỗ và số lượng có thể bán.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên, hãng hoặc mã sản phẩm...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->refreshInventory(txt ? txt : "");
      }),
      this, nullptr, &inventoryCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kWarehouseInvCols));

  inventoryListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), inventoryListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(inventoryView), content);
  refreshInventory();
}

void WarehouseScreens::refreshInventory(const std::string &keyword,
                                        const std::string &brand) {
  if (!inventoryListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(inventoryListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(inventoryListBox), child);
    child = next;
  }

  ProductFilterRequest req;
  req.keyword = keyword;
  req.brand = brand;
  std::vector<ProductItemViewModel> prods = controller->getProducts(req);

  UIComponents::updateRecordCountBadge(inventoryCountBadge, prods.size(),
                                       "thiết bị");

  if (prods.empty()) {
    gtk_box_append(GTK_BOX(inventoryListBox),
                   UIComponents::createEmptyState(
                       "🔍", "Chưa có dữ liệu tồn kho",
                       "Không có sản phẩm nào khớp với từ khóa tìm kiếm."));
    return;
  }

  for (size_t i = 0; i < prods.size(); ++i) {
    const auto &p = prods[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Tên thiết bị & Mã
    gtk_box_append(
        GTK_BOX(row),
        UIComponents::createTableTwoLineCell(
            p.name, "Mã: " + p.id + (p.os.empty() ? "" : (" • HĐH: " + p.os)),
            kWarehouseInvCols[0]));

    // Col 1: Hãng badge
    GtkWidget *bBadge =
        UIComponents::createStatusBadge(p.brand, "role-pill-sales");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     bBadge, kWarehouseInvCols[1]));

    // Col 2: Tình trạng badge
    GtkWidget *sBadge = UIComponents::createStockStatusBadge(p.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     sBadge, kWarehouseInvCols[2]));

    // Col 3: Giá nhập (căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(p.importPrice),
                                     kWarehouseInvCols[3], "cell-number"));

    // Col 4: Thực tế (căn phải)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(p.actualStock) + " máy",
                       kWarehouseInvCols[4], "cell-number"));

    // Col 5: Giữ chỗ (căn phải)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(p.reservedStock) + " máy",
                       kWarehouseInvCols[5], "cell-number"));

    // Col 6: Có thể bán (căn phải, màu ngữ nghĩa)
    std::string stockCssClass = "stock-ok";
    if (p.status == StockDisplayStatus::OUT_OF_STOCK) {
      stockCssClass = "stock-critical";
    } else if (p.status == StockDisplayStatus::LOW_STOCK) {
      stockCssClass = "stock-low";
    }
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(p.availableStock) + " máy",
                       kWarehouseInvCols[6], stockCssClass));

    gtk_box_append(GTK_BOX(inventoryListBox), row);
  }
}

// ================= 3. SẢN PHẨM SẮP HẾT HÀNG =================

void WarehouseScreens::buildLowStockView() {
  lowStockView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(lowStockView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Cảnh báo sản phẩm sắp hết hàng",
      "Danh sách các thiết bị có số lượng tồn kho khả dụng dưới ngưỡng an toàn "
      "(<= 5 máy).");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm trong danh sách cảnh báo theo mã, tên máy hoặc hãng...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->lowStockKeyword = txt ? txt : "";
        self->refreshLowStock();
      }),
      this, nullptr, &lowStockCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kWarehouseLowCols));

  lowStockListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), lowStockListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(lowStockView), content);
  refreshLowStock();
}

void WarehouseScreens::refreshLowStock() {
  if (!lowStockListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(lowStockListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(lowStockListBox), child);
    child = next;
  }

  std::vector<ProductItemViewModel> allLowStock =
      controller->getInventory(true);
  std::vector<ProductItemViewModel> lowStock;
  for (const auto &p : allLowStock) {
    if (!lowStockKeyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(p.id, lowStockKeyword) &&
          !Utf8Helper::containsCaseInsensitive(p.name, lowStockKeyword) &&
          !Utf8Helper::containsCaseInsensitive(p.brand, lowStockKeyword)) {
        continue;
      }
    }
    lowStock.push_back(p);
  }

  UIComponents::updateRecordCountBadge(lowStockCountBadge, lowStock.size(),
                                       "mặt hàng sắp hết");

  if (lowStock.empty()) {
    std::string msg =
        lowStockKeyword.empty()
            ? "Tất cả sản phẩm hiện đều có số lượng khả dụng dồi dào trên 5 "
              "máy."
            : "Không có mặt hàng sắp hết nào khớp với từ khóa tìm kiếm.";
    GtkWidget *empty =
        UIComponents::createEmptyState("✅", "Tồn kho an toàn", msg);
    gtk_box_append(GTK_BOX(lowStockListBox), empty);
    return;
  }

  for (size_t i = 0; i < lowStock.size(); ++i) {
    const auto &p = lowStock[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã SP
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     p.id, kWarehouseLowCols[0], "cell-code"));

    // Col 1: Tên điện thoại & Cấu hình
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       p.name,
                       p.specs.empty()
                           ? ("Thực tế: " + std::to_string(p.actualStock) +
                              " • Giữ chỗ: " + std::to_string(p.reservedStock))
                           : p.specs,
                       kWarehouseLowCols[1]));

    // Col 2: Hãng
    GtkWidget *bBadge =
        UIComponents::createStatusBadge(p.brand, "role-pill-sales");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     bBadge, kWarehouseLowCols[2]));

    // Col 3: Giá nhập tham khảo (căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(p.importPrice),
                                     kWarehouseLowCols[3], "cell-number"));

    // Col 4: Còn khả dụng (căn phải, màu cảnh báo)
    std::string stockCssClass =
        (p.availableStock <= 0) ? "stock-critical" : "stock-low";
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(p.availableStock) + " máy",
                       kWarehouseLowCols[4], stockCssClass));

    // Col 5: Thao tác
    GtkWidget *btnReorder = UIComponents::createTableActionButton(
        "📥 Nhập hàng", "Chuyển tới màn hình lập phiếu nhập cho sản phẩm này",
        "btn-primary");
    gtk_widget_set_hexpand(btnReorder, TRUE);
    g_signal_connect_swapped(btnReorder, "clicked",
                             G_CALLBACK(+[](MainShellWindow *s) {
                               if (s)
                                 s->navigateTo("warehouse_create_receipt");
                             }),
                             shell);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnReorder, kWarehouseLowCols[5]));

    gtk_box_append(GTK_BOX(lowStockListBox), row);
  }
}

// ================= 4. DANH SÁCH PHIẾU NHẬP KHO =================

void WarehouseScreens::buildReceiptsView() {
  receiptsView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(receiptsView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnNew = gtk_button_new_with_label("➕ Lập phiếu nhập mới");
  gtk_widget_add_css_class(btnNew, "btn-primary");
  UIComponents::setButtonCursor(btnNew);
  g_signal_connect_swapped(btnNew, "clicked",
                           G_CALLBACK(+[](MainShellWindow *s) {
                             if (s)
                               s->navigateTo("warehouse_create_receipt");
                           }),
                           shell);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh sách phiếu nhập kho",
      "Theo dõi lịch sử nhập hàng từ các nhà cung cấp.", btnNew);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã phiếu, nhà cung cấp hoặc thủ kho nhập...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->receiptsKeyword = txt ? txt : "";
        self->refreshReceipts();
      }),
      this, nullptr, &receiptsCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kWarehouseReceiptCols));

  receiptsListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), receiptsListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(receiptsView), content);
  refreshReceipts();
}

void WarehouseScreens::refreshReceipts() {
  if (!receiptsListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(receiptsListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(receiptsListBox), child);
    child = next;
  }

  std::vector<GoodsReceiptSummaryViewModel> all =
      controller->getGoodsReceipts();
  std::vector<GoodsReceiptSummaryViewModel> list;
  for (const auto &r : all) {
    if (!receiptsKeyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(r.id, receiptsKeyword) &&
          !Utf8Helper::containsCaseInsensitive(r.supplierName,
                                               receiptsKeyword) &&
          !Utf8Helper::containsCaseInsensitive(r.employeeName,
                                               receiptsKeyword)) {
        continue;
      }
    }
    list.push_back(r);
  }

  UIComponents::updateRecordCountBadge(receiptsCountBadge, list.size(),
                                       "phiếu nhập");

  if (list.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "📋", "Chưa có phiếu nhập",
        "Không tìm thấy phiếu nhập kho nào khớp với bộ lọc.",
        "➕ Lập phiếu nhập mới", G_CALLBACK(+[](GtkButton *, gpointer d) {
          WarehouseScreens *s = static_cast<WarehouseScreens *>(d);
          if (s->shell)
            s->shell->navigateTo("warehouse_create_receipt");
        }),
        this);
    gtk_box_append(GTK_BOX(receiptsListBox), empty);
    return;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    const auto &r = list[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã phiếu
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       r.id, kWarehouseReceiptCols[0], "cell-code"));

    // Col 1: Nhà cung cấp
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       r.supplierName, kWarehouseReceiptCols[1], "cell-title"));

    // Col 2: Người nhập
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     r.employeeName, kWarehouseReceiptCols[2]));

    // Col 3: Thời gian
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       r.importTime, kWarehouseReceiptCols[3], "tabular-num"));

    // Col 4: Tổng tiền (căn phải)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatVND(r.totalAmount),
                       kWarehouseReceiptCols[4], "product-price"));

    // Col 5: Thao tác
    GtkWidget *btnDetail = UIComponents::createTableActionButton(
        "🔍 Xem", "Xem chi tiết mặt hàng và số lượng nhập", "btn-action-view");
    gtk_widget_set_hexpand(btnDetail, TRUE);

    struct RecCtx {
      WarehouseScreens *screens;
      GoodsReceiptSummaryViewModel item;
    };
    RecCtx *rCtx = new RecCtx{this, r};
    g_signal_connect_data(
        btnDetail, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          RecCtx *rc = static_cast<RecCtx *>(d);
          rc->screens->showReceiptDetailModal(rc->item);
        }),
        rCtx, [](gpointer d, GClosure *) { delete static_cast<RecCtx *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnDetail, kWarehouseReceiptCols[5]));
    gtk_box_append(GTK_BOX(receiptsListBox), row);
  }
}

void WarehouseScreens::showReceiptDetailModal(
    const GoodsReceiptSummaryViewModel &receipt) {
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win),
                       ("Chi tiết phiếu nhập — " + receipt.id).c_str());
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  if (shell) {
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
  }
  gtk_window_set_default_size(GTK_WINDOW(win), 480, 400);

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_margin_top(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_bottom(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_start(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_end(card, DesignTokens::SPACE_LG);

  GtkWidget *title = gtk_label_new(("PHIẾU NHẬP KHO " + receipt.id).c_str());
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

  addRow("Nhà cung cấp:", receipt.supplierName);
  addRow("Thủ kho nhập:", receipt.employeeName);
  addRow("Thời gian nhập:", receipt.importTime);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(card), sep);

  for (const auto &d : receipt.details) {
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

  addRow("Tổng tiền nhập:", UIComponents::formatVND(receipt.totalAmount));

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

// ================= 5. TẠO PHIẾU NHẬP KHO =================

void WarehouseScreens::buildCreateReceiptView() {
  createReceiptView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(createReceiptView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Lập phiếu nhập kho hàng", "Chọn nhà cung cấp, thêm các mã điện thoại, "
                                 "chỉnh số lượng và xác nhận nhập.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *formCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(formCard, "card");

  // Supplier selection
  std::vector<SupplierItemViewModel> sups = controller->getSuppliers();
  std::vector<const char *> supNames;
  receiptSupplierIds.clear();
  for (const auto &s : sups) {
    supNames.push_back(s.companyName.c_str());
    receiptSupplierIds.push_back(s.id);
  }
  supNames.push_back(nullptr);

  receiptComboSupplier = gtk_drop_down_new_from_strings(supNames.data());
  lastSelectedSupplierIndex = 0;
  gtk_box_append(GTK_BOX(formCard),
                 UIComponents::createFormField("Chọn nhà cung cấp *",
                                               receiptComboSupplier));

  g_signal_connect_data(
      receiptComboSupplier, "notify::selected",
      G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        guint newSel = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
        if (newSel == self->lastSelectedSupplierIndex)
          return;

        if (!self->draftReceiptItems.empty()) {
          gtk_drop_down_set_selected(GTK_DROP_DOWN(obj),
                                     self->lastSelectedSupplierIndex);
          UIComponents::showWarningDialog(
              self->shell ? self->shell->getWindow() : nullptr,
              "Không thể đổi nhà cung cấp",
              "Phiếu nhập đang có sản phẩm nháp. Vui lòng hoàn tất hoặc xóa "
              "các sản phẩm trong phiếu trước khi chọn nhà cung cấp khác!");
          return;
        }

        self->lastSelectedSupplierIndex = newSel;
      }),
      this, nullptr, (GConnectFlags)0);

  // Action button: Add item
  GtkWidget *itemHeaderBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  GtkWidget *lblItemTitle = gtk_label_new("📦 Danh sách sản phẩm nhập:");
  gtk_widget_add_css_class(lblItemTitle, "card-title");
  gtk_widget_set_hexpand(lblItemTitle, TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblItemTitle), 0.0f);

  receiptDraftCountBadge = gtk_label_new("Hiển thị: 0 mặt hàng");
  gtk_widget_add_css_class(receiptDraftCountBadge, "record-count-badge");
  gtk_widget_set_valign(receiptDraftCountBadge, GTK_ALIGN_CENTER);

  GtkWidget *btnAddItem =
      gtk_button_new_with_label("➕ Thêm sản phẩm vào phiếu");
  gtk_widget_add_css_class(btnAddItem, "btn-primary");
  UIComponents::setButtonCursor(btnAddItem);
  g_signal_connect_swapped(btnAddItem, "clicked",
                           G_CALLBACK(+[](WarehouseScreens *s) {
                             s->showAddProductToReceiptModal();
                           }),
                           this);

  gtk_box_append(GTK_BOX(itemHeaderBox), lblItemTitle);
  gtk_box_append(GTK_BOX(itemHeaderBox), receiptDraftCountBadge);
  gtk_box_append(GTK_BOX(itemHeaderBox), btnAddItem);
  gtk_box_append(GTK_BOX(formCard), itemHeaderBox);

  // Draft items table container
  GtkWidget *draftTableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(draftTableContainer, "table-container");
  gtk_box_append(GTK_BOX(draftTableContainer),
                 UIComponents::createTableHeaderRow(kDraftReceiptCols));

  receiptDraftItemsBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(draftTableContainer), receiptDraftItemsBox);
  gtk_box_append(GTK_BOX(formCard), draftTableContainer);

  // Total estimate
  lblReceiptEstimatedTotal = gtk_label_new("Tổng tiền nhập dự kiến: 0 ₫");
  gtk_widget_add_css_class(lblReceiptEstimatedTotal, "product-price");
  gtk_label_set_xalign(GTK_LABEL(lblReceiptEstimatedTotal), 1.0f);
  gtk_box_append(GTK_BOX(formCard), lblReceiptEstimatedTotal);

  // Submit Receipt
  GtkWidget *btnSubmitReceipt =
      gtk_button_new_with_label("📥 Xác nhận lưu phiếu nhập & Tăng tồn kho");
  gtk_widget_add_css_class(btnSubmitReceipt, "btn-success");
  UIComponents::setButtonCursor(btnSubmitReceipt);
  gtk_widget_set_margin_top(btnSubmitReceipt, DesignTokens::SPACE_SM);

  g_signal_connect_data(
      btnSubmitReceipt, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        if (self->draftReceiptItems.empty()) {
          UIComponents::showWarningDialog(
              self->shell ? self->shell->getWindow() : nullptr,
              "Thiếu sản phẩm",
              "Vui lòng thêm ít nhất 1 sản phẩm vào phiếu nhập!");
          return;
        }

        guint supSel = gtk_drop_down_get_selected(
            GTK_DROP_DOWN(self->receiptComboSupplier));
        std::string supId = (supSel < self->receiptSupplierIds.size())
                                ? self->receiptSupplierIds[supSel]
                                : "";
        if (supId.empty()) {
          std::vector<SupplierItemViewModel> sups =
              self->controller->getSuppliers();
          supId = (supSel < sups.size()) ? sups[supSel].id : "SUP-000001";
        }

        CreateReceiptRequest req;
        req.supplierId = supId;
        req.employeeId = self->controller->getCurrentUser().userId;
        req.items = self->draftReceiptItems;

        OperationResult res = self->controller->createGoodsReceipt(req);
        if (res.success) {
          self->draftReceiptItems.clear();
          self->refreshDraftReceiptItems();
          self->refreshReceipts();
          self->refreshInventory();
          self->refreshLowStock();
          self->refreshDashboard();

          UIComponents::showSuccessDialog(self->shell ? self->shell->getWindow()
                                                      : nullptr,
                                          "Thành công", res.message);
          if (self->shell)
            self->shell->navigateTo("warehouse_receipts");
        } else {
          UIComponents::showErrorDialog(self->shell ? self->shell->getWindow()
                                                    : nullptr,
                                        "Lỗi", res.message);
        }
      }),
      this, nullptr, (GConnectFlags)0);

  gtk_box_append(GTK_BOX(formCard), btnSubmitReceipt);
  gtk_box_append(GTK_BOX(content), formCard);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(createReceiptView),
                                content);
  refreshDraftReceiptItems();
}

void WarehouseScreens::refreshDraftReceiptItems() {
  if (!receiptDraftItemsBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(receiptDraftItemsBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(receiptDraftItemsBox), child);
    child = next;
  }

  UIComponents::updateRecordCountBadge(receiptDraftCountBadge,
                                       draftReceiptItems.size(), "mặt hàng");

  if (draftReceiptItems.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "📥", "Phiếu nhập đang trống",
        "Bấm nút 'Thêm sản phẩm vào phiếu' để chọn máy và số lượng cần nhập "
        "kho.",
        "➕ Thêm sản phẩm vào phiếu", G_CALLBACK(+[](GtkButton *, gpointer d) {
          WarehouseScreens *s = static_cast<WarehouseScreens *>(d);
          s->showAddProductToReceiptModal();
        }),
        this);
    gtk_box_append(GTK_BOX(receiptDraftItemsBox), empty);
    if (lblReceiptEstimatedTotal) {
      gtk_label_set_text(GTK_LABEL(lblReceiptEstimatedTotal),
                         "Tổng tiền nhập dự kiến: 0 ₫");
    }
    return;
  }

  double total = 0.0;
  for (size_t i = 0; i < draftReceiptItems.size(); ++i) {
    const auto &item = draftReceiptItems[i];
    total += item.subtotal;

    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã máy
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       item.phoneId, kDraftReceiptCols[0], "cell-code"));

    // Col 1: Tên điện thoại nhập
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       item.phoneName, kDraftReceiptCols[1], "cell-title"));

    // Col 2: Đơn giá nhập
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(item.importPrice),
                                     kDraftReceiptCols[2], "cell-number"));

    // Col 3: Số lượng (Stepper: − / qty / +)
    GtkWidget *stepperBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_valign(stepperBox, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(stepperBox, GTK_ALIGN_CENTER);

    GtkWidget *btnMinus = gtk_button_new_with_label("−");
    gtk_widget_add_css_class(btnMinus, "btn-secondary");
    gtk_widget_add_css_class(btnMinus, "btn-action-sm");
    gtk_widget_set_size_request(btnMinus, 36, 36);
    UIComponents::setButtonCursor(btnMinus);
    if (item.quantity <= 1) {
      gtk_widget_set_sensitive(btnMinus, FALSE);
    }

    GtkWidget *entryQty = gtk_entry_new();
    gtk_widget_set_size_request(entryQty, 64, 36);
    gtk_editable_set_text(GTK_EDITABLE(entryQty),
                          std::to_string(item.quantity).c_str());
    gtk_widget_add_css_class(entryQty, "cell-number");
    gtk_entry_set_alignment(GTK_ENTRY(entryQty), 0.5f);

    GtkWidget *btnPlus = gtk_button_new_with_label("+");
    gtk_widget_add_css_class(btnPlus, "btn-secondary");
    gtk_widget_add_css_class(btnPlus, "btn-action-sm");
    gtk_widget_set_size_request(btnPlus, 36, 36);
    UIComponents::setButtonCursor(btnPlus);

    struct StepperCtx {
      WarehouseScreens *screens;
      size_t idx;
      GtkWidget *entry;
    };
    StepperCtx *sctxMinus = new StepperCtx{this, i, entryQty};
    g_signal_connect_data(
        btnMinus, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          StepperCtx *c = static_cast<StepperCtx *>(d);
          if (c->idx < c->screens->draftReceiptItems.size()) {
            if (c->screens->draftReceiptItems[c->idx].quantity > 1) {
              c->screens->draftReceiptItems[c->idx].quantity--;
              c->screens->draftReceiptItems[c->idx].subtotal =
                  c->screens->draftReceiptItems[c->idx].quantity *
                  c->screens->draftReceiptItems[c->idx].importPrice;
              c->screens->refreshDraftReceiptItems();
            }
          }
        }),
        sctxMinus,
        [](gpointer d, GClosure *) { delete static_cast<StepperCtx *>(d); },
        (GConnectFlags)0);

    StepperCtx *sctxPlus = new StepperCtx{this, i, entryQty};
    g_signal_connect_data(
        btnPlus, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          StepperCtx *c = static_cast<StepperCtx *>(d);
          if (c->idx < c->screens->draftReceiptItems.size()) {
            c->screens->draftReceiptItems[c->idx].quantity++;
            c->screens->draftReceiptItems[c->idx].subtotal =
                c->screens->draftReceiptItems[c->idx].quantity *
                c->screens->draftReceiptItems[c->idx].importPrice;
            c->screens->refreshDraftReceiptItems();
          }
        }),
        sctxPlus,
        [](gpointer d, GClosure *) { delete static_cast<StepperCtx *>(d); },
        (GConnectFlags)0);

    StepperCtx *sctxEntry = new StepperCtx{this, i, entryQty};
    g_signal_connect_data(
        entryQty, "activate", G_CALLBACK(+[](GtkEntry *e, gpointer d) {
          StepperCtx *c = static_cast<StepperCtx *>(d);
          if (c->idx < c->screens->draftReceiptItems.size()) {
            const char *txt = gtk_editable_get_text(GTK_EDITABLE(e));
            int val = 0;
            try {
              val = std::stoi(txt ? txt : "1");
            } catch (...) {
              val = 0;
            }
            if (val <= 0)
              val = 1;
            c->screens->draftReceiptItems[c->idx].quantity = val;
            c->screens->draftReceiptItems[c->idx].subtotal =
                val * c->screens->draftReceiptItems[c->idx].importPrice;
            c->screens->refreshDraftReceiptItems();
          }
        }),
        sctxEntry,
        [](gpointer d, GClosure *) { delete static_cast<StepperCtx *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(stepperBox), btnMinus);
    gtk_box_append(GTK_BOX(stepperBox), entryQty);
    gtk_box_append(GTK_BOX(stepperBox), btnPlus);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     stepperBox, kDraftReceiptCols[3]));

    // Col 4: Thành tiền
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(item.subtotal),
                                     kDraftReceiptCols[4], "product-price"));

    // Col 5: Xóa
    GtkWidget *btnDel = UIComponents::createTableActionButton(
        "✕ Xóa", "Xóa mặt hàng khỏi phiếu nháp", "btn-action-delete");
    gtk_widget_set_hexpand(btnDel, TRUE);

    struct DelDraftCtx {
      WarehouseScreens *screens;
      size_t idx;
    };
    DelDraftCtx *dctx = new DelDraftCtx{this, i};
    g_signal_connect_data(
        btnDel, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
          DelDraftCtx *c = static_cast<DelDraftCtx *>(d);
          if (c->idx < c->screens->draftReceiptItems.size()) {
            c->screens->draftReceiptItems.erase(
                c->screens->draftReceiptItems.begin() + c->idx);
            c->screens->refreshDraftReceiptItems();
          }
        }),
        dctx,
        [](gpointer d, GClosure *) { delete static_cast<DelDraftCtx *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnDel, kDraftReceiptCols[5]));
    gtk_box_append(GTK_BOX(receiptDraftItemsBox), row);
  }

  if (lblReceiptEstimatedTotal) {
    gtk_label_set_text(
        GTK_LABEL(lblReceiptEstimatedTotal),
        ("Tổng tiền nhập dự kiến: " + UIComponents::formatVND(total)).c_str());
  }
}

void WarehouseScreens::showAddProductToReceiptModal() {
  guint supSel =
      receiptComboSupplier
          ? gtk_drop_down_get_selected(GTK_DROP_DOWN(receiptComboSupplier))
          : 0;
  std::string supId =
      (supSel < receiptSupplierIds.size()) ? receiptSupplierIds[supSel] : "";
  if (supId.empty()) {
    std::vector<SupplierItemViewModel> sups = controller->getSuppliers();
    supId = (supSel < sups.size()) ? sups[supSel].id : "";
  }
  std::vector<ProductItemViewModel> prods =
      controller->getProductsBySupplier(supId);
  if (prods.empty()) {
    UIComponents::showWarningDialog(shell ? shell->getWindow() : nullptr,
                                    "Chưa có sản phẩm",
                                    "Nhà cung cấp được chọn hiện chưa có dòng "
                                    "sản phẩm nào khả dụng để nhập kho!");
    return;
  }

  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win), "Thêm sản phẩm vào phiếu nhập");
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  if (shell) {
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
  }
  gtk_window_set_default_size(GTK_WINDOW(win), 440, 340);

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_margin_top(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_bottom(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_start(card, DesignTokens::SPACE_LG);
  gtk_widget_set_margin_end(card, DesignTokens::SPACE_LG);

  std::vector<const char *> prodNames;
  for (const auto &p : prods) {
    prodNames.push_back(p.name.c_str());
  }
  prodNames.push_back(nullptr);

  GtkWidget *errCombo = nullptr;
  GtkWidget *comboProd = gtk_drop_down_new_from_strings(prodNames.data());
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Chọn điện thoại *", comboProd, &errCombo));

  GtkWidget *errQty = nullptr;
  GtkWidget *entryQty = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryQty), "1");
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Số lượng nhập *", entryQty, &errQty));

  GtkWidget *errPrice = nullptr;
  GtkWidget *entryPrice = gtk_entry_new();
  std::string initPrice =
      !prods.empty()
          ? std::to_string(static_cast<long long>(prods[0].importPrice))
          : "10000000";
  gtk_editable_set_text(GTK_EDITABLE(entryPrice), initPrice.c_str());
  UIComponents::setupCurrencyInput(entryPrice);
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Giá nhập từng máy (VNĐ) *",
                                               entryPrice, &errPrice));

  struct ComboSyncCtx {
    GtkWidget *ePrice;
    std::vector<ProductItemViewModel> prods;
  };
  ComboSyncCtx *syncCtx = new ComboSyncCtx{entryPrice, prods};
  g_signal_connect_data(
      comboProd, "notify::selected",
      G_CALLBACK(+[](GObject *obj, GParamSpec *, gpointer d) {
        ComboSyncCtx *sc = static_cast<ComboSyncCtx *>(d);
        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
        if (sel < sc->prods.size()) {
          long long pr = static_cast<long long>(sc->prods[sel].importPrice);
          gtk_editable_set_text(GTK_EDITABLE(sc->ePrice),
                                std::to_string(pr).c_str());
        }
      }),
      syncCtx,
      [](gpointer d, GClosure *) { delete static_cast<ComboSyncCtx *>(d); },
      (GConnectFlags)0);

  GtkWidget *btnRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy (Esc)");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy),
                           win);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct AddProdReceiptCtx {
    WarehouseScreens *screens;
    GtkWidget *win;
    GtkWidget *combo;
    GtkWidget *eQty;
    GtkWidget *ePrice;
    GtkWidget *errCombo;
    GtkWidget *errQty;
    GtkWidget *errPrice;
    std::vector<ProductItemViewModel> prods;
  };
  AddProdReceiptCtx *ctx =
      new AddProdReceiptCtx{this,     win,    comboProd, entryQty, entryPrice,
                            errCombo, errQty, errPrice,  prods};

  GtkWidget *btnAdd = gtk_button_new_with_label("Thêm vào phiếu");
  gtk_widget_add_css_class(btnAdd, "btn-primary");
  UIComponents::setButtonCursor(btnAdd);
  g_signal_connect_data(
      btnAdd, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        AddProdReceiptCtx *c = static_cast<AddProdReceiptCtx *>(d);
        UIComponents::clearFieldError(c->errCombo, c->combo);
        UIComponents::clearFieldError(c->errQty, c->eQty);
        UIComponents::clearFieldError(c->errPrice, c->ePrice);

        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->combo));
        bool hasError = false;
        GtkWidget *firstErr = nullptr;

        if (sel >= c->prods.size()) {
          UIComponents::setFieldError(c->errCombo, c->combo,
                                      "Vui lòng chọn một dòng điện thoại");
          hasError = true;
          if (!firstErr)
            firstErr = c->combo;
        }

        int qty = 0;
        try {
          qty = std::stoi(gtk_editable_get_text(GTK_EDITABLE(c->eQty)));
        } catch (...) {
          qty = 0;
        }
        if (qty <= 0) {
          UIComponents::setFieldError(c->errQty, c->eQty,
                                      "Số lượng nhập phải lớn hơn 0");
          hasError = true;
          if (!firstErr)
            firstErr = c->eQty;
        }

        double price = UIComponents::parseCurrencyInput(c->ePrice);
        if (price <= 0) {
          UIComponents::setFieldError(c->errPrice, c->ePrice,
                                      "Giá nhập từng máy phải lớn hơn 0");
          hasError = true;
          if (!firstErr)
            firstErr = c->ePrice;
        }

        if (hasError) {
          if (firstErr)
            gtk_widget_grab_focus(firstErr);
          return;
        }

        bool merged = false;
        for (auto &existing : c->screens->draftReceiptItems) {
          if (existing.phoneId == c->prods[sel].id) {
            if (std::abs(existing.importPrice - price) < 0.001) {
              existing.quantity += qty;
              existing.subtotal = existing.quantity * existing.importPrice;
            } else {
              // Đơn giá khác nhau: tính bình quân gia quyền, không ghi đè giá
              // cũ bằng giá mới
              double totalCost = existing.subtotal + (qty * price);
              existing.quantity += qty;
              existing.importPrice =
                  totalCost / static_cast<double>(existing.quantity);
              existing.subtotal = totalCost;
            }
            merged = true;
            break;
          }
        }
        if (!merged) {
          GoodsReceiptDetailViewModel item;
          item.phoneId = c->prods[sel].id;
          item.phoneName = c->prods[sel].name;
          item.quantity = qty;
          item.importPrice = price;
          item.subtotal = qty * price;
          c->screens->draftReceiptItems.push_back(item);
        }

        c->screens->refreshDraftReceiptItems();
        gtk_window_destroy(GTK_WINDOW(c->win));
      }),
      ctx,
      [](gpointer d, GClosure *) {
        delete static_cast<AddProdReceiptCtx *>(d);
      },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnAdd);
  gtk_box_append(GTK_BOX(card), btnRow);

  gtk_window_set_child(GTK_WINDOW(win), card);
  UIComponents::setupDialogShortcuts(win, btnAdd, btnCancel);
  gtk_widget_grab_focus(comboProd);
  gtk_window_present(GTK_WINDOW(win));
}

// ================= 6. CHUẨN BỊ ĐƠN HÀNG (FULFILLMENT) =================

void WarehouseScreens::buildFulfillmentView() {
  fulfillmentView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(fulfillmentView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header =
      UIComponents::createSectionHeader("Chuẩn bị đơn hàng (Fulfillment)",
                                        "Lấy hàng, đóng gói và chuyển giao cho "
                                        "vận chuyển hoặc để tại quầy nhận.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã đơn, người nhận, SĐT hoặc tên máy cần lấy...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->fulfillmentKeyword = txt ? txt : "";
        self->refreshFulfillmentQueue();
      }),
      this, nullptr, &fulfillmentCountBadge);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kWarehouseFulfillCols));

  fulfillmentListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
  gtk_box_append(GTK_BOX(tableContainer), fulfillmentListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fulfillmentView), content);
  refreshFulfillmentQueue();
}

void WarehouseScreens::refreshFulfillmentQueue() {
  if (!fulfillmentListBox)
    return;
  GtkWidget *child = gtk_widget_get_first_child(fulfillmentListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(fulfillmentListBox), child);
    child = next;
  }

  std::vector<OrderItemViewModel> orders = controller->getAllOrders();
  std::vector<OrderItemViewModel> toFulfill;
  for (const auto &ord : orders) {
    if (ord.status != OrderStatus::CONFIRMED &&
        ord.status != OrderStatus::PREPARING) {
      continue;
    }
    if (!fulfillmentKeyword.empty()) {
      bool matchDetail = false;
      for (const auto &d : ord.details) {
        if (Utf8Helper::containsCaseInsensitive(d.phoneName,
                                                fulfillmentKeyword)) {
          matchDetail = true;
          break;
        }
      }
      if (!matchDetail &&
          !Utf8Helper::containsCaseInsensitive(ord.id, fulfillmentKeyword) &&
          !Utf8Helper::containsCaseInsensitive(ord.receiverName,
                                               fulfillmentKeyword) &&
          !Utf8Helper::containsCaseInsensitive(ord.receiverPhone,
                                               fulfillmentKeyword)) {
        continue;
      }
    }
    toFulfill.push_back(ord);
  }

  UIComponents::updateRecordCountBadge(fulfillmentCountBadge, toFulfill.size(),
                                       "đơn cần đóng gói");

  if (toFulfill.empty()) {
    gtk_box_append(
        GTK_BOX(fulfillmentListBox),
        UIComponents::createEmptyState("📦", "Không có đơn chờ đóng gói",
                                       "Tất cả đơn hàng đã xác nhận đều đã "
                                       "được xuất kho hoặc chuyển giao."));
    return;
  }

  for (size_t i = 0; i < toFulfill.size(); ++i) {
    const auto &ord = toFulfill[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // Col 0: Mã đơn
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       ord.id, kWarehouseFulfillCols[0], "cell-code-ord"));

    // Col 1: Thông tin người nhận
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       ord.receiverName, "SĐT: " + ord.receiverPhone,
                       kWarehouseFulfillCols[1]));

    // Col 2: Danh sách máy cần lấy
    std::string pickList = "";
    for (size_t j = 0; j < ord.details.size(); ++j) {
      if (j > 0)
        pickList += ", ";
      pickList += ord.details[j].phoneName + " [x" +
                  std::to_string(ord.details[j].quantity) + "]";
    }
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       pickList, "Thời gian đặt: " + ord.orderTime,
                       kWarehouseFulfillCols[2]));

    // Col 3: Trạng thái badge
    GtkWidget *stBadge = UIComponents::createOrderStatusBadge(ord.status);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     stBadge, kWarehouseFulfillCols[3]));

    // Col 4: Giao nhận badge
    GtkWidget *delBadge = UIComponents::createStatusBadge(
        ord.deliveryMethodText, "badge-confirmed");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     delBadge, kWarehouseFulfillCols[4]));

    // Col 5: Actions
    GtkWidget *actionBox =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_XS);
    gtk_widget_set_valign(actionBox, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(actionBox, GTK_ALIGN_CENTER);

    struct FulfillCtx {
      WarehouseScreens *screens;
      std::string orderId;
      OrderStatus nextStatus;
    };

    if (ord.status == OrderStatus::CONFIRMED) {
      GtkWidget *btnPack = UIComponents::createTableActionButton(
          "📦 Đang đóng gói", "Bắt đầu lấy máy và đóng gói đơn hàng",
          "btn-primary");
      FulfillCtx *fc = new FulfillCtx{this, ord.id, OrderStatus::PREPARING};
      g_signal_connect_data(
          btnPack, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            FulfillCtx *c = static_cast<FulfillCtx *>(d);
            c->screens->controller->updateOrderStatus(c->orderId,
                                                      c->nextStatus);
            c->screens->refreshFulfillmentQueue();
            c->screens->refreshDashboard();
          }),
          fc,
          [](gpointer d, GClosure *) { delete static_cast<FulfillCtx *>(d); },
          (GConnectFlags)0);
      gtk_box_append(GTK_BOX(actionBox), btnPack);
    } else if (ord.status == OrderStatus::PREPARING) {
      if (ord.deliveryMethod == DeliveryMethod::PICKUP) {
        GtkWidget *btnReady = UIComponents::createTableActionButton(
            "✅ Sẵn sàng nhận", "Hàng đã sẵn sàng tại quầy để khách tới lấy",
            "btn-primary");
        FulfillCtx *fc =
            new FulfillCtx{this, ord.id, OrderStatus::READY_FOR_PICKUP};
        g_signal_connect_data(
            btnReady, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
              (void)b;
              FulfillCtx *c = static_cast<FulfillCtx *>(d);
              c->screens->controller->updateOrderStatus(c->orderId,
                                                        c->nextStatus);
              c->screens->refreshFulfillmentQueue();
              c->screens->refreshDashboard();
            }),
            fc,
            [](gpointer d, GClosure *) { delete static_cast<FulfillCtx *>(d); },
            (GConnectFlags)0);
        gtk_box_append(GTK_BOX(actionBox), btnReady);
      } else {
        GtkWidget *btnShip = UIComponents::createTableActionButton(
            "🚚 Bàn giao ship", "Bàn giao kiện hàng cho đối tác vận chuyển",
            "btn-primary");
        FulfillCtx *fc = new FulfillCtx{this, ord.id, OrderStatus::SHIPPING};
        g_signal_connect_data(
            btnShip, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
              (void)b;
              FulfillCtx *c = static_cast<FulfillCtx *>(d);
              c->screens->controller->updateOrderStatus(c->orderId,
                                                        c->nextStatus);
              c->screens->refreshFulfillmentQueue();
              c->screens->refreshDashboard();
            }),
            fc,
            [](gpointer d, GClosure *) { delete static_cast<FulfillCtx *>(d); },
            (GConnectFlags)0);
        gtk_box_append(GTK_BOX(actionBox), btnShip);
      }
    }

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     actionBox, kWarehouseFulfillCols[5]));
    gtk_box_append(GTK_BOX(fulfillmentListBox), row);
  }
}

void WarehouseScreens::addDraftReceiptItem(
    const GoodsReceiptDetailViewModel &item) {
  for (auto &existing : draftReceiptItems) {
    if (existing.phoneId == item.phoneId) {
      if (std::abs(existing.importPrice - item.importPrice) < 0.001) {
        existing.quantity += item.quantity;
        existing.subtotal = existing.quantity * existing.importPrice;
      } else {
        double totalCost =
            existing.subtotal + (item.quantity * item.importPrice);
        existing.quantity += item.quantity;
        existing.importPrice =
            totalCost / static_cast<double>(existing.quantity);
        existing.subtotal = totalCost;
      }
      refreshDraftReceiptItems();
      return;
    }
  }
  draftReceiptItems.push_back(item);
  refreshDraftReceiptItems();
}

void WarehouseScreens::clearDraftReceiptItems() {
  draftReceiptItems.clear();
  refreshDraftReceiptItems();
}
