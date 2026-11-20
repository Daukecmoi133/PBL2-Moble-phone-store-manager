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
#include "../../source/utils/ExportHelper.h"
#include "../../source/utils/Utf8Helper.h"
#include "../components/UIComponents.h"
#include "../mock/RealTimeManager.h"
#include "../styles/AppCss.h"
#include "MainShellWindow.h"

namespace {
const std::vector<UIComponents::TableColumnSpec> kWarehouseInvCols = {
    {"Tên thiết bị & Mã", 220, 0.0f, true}, {"Hãng", 110, 0.5f, false},
    {"Tình trạng", 145, 0.5f, false},       {"Giá nhập", 165, 1.0f, false},
    {"Thực tế", 105, 1.0f, false},          {"Giữ chỗ", 105, 1.0f, false},
    {"Có thể bán", 115, 1.0f, false},       {"Chi tiết", 120, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kWarehouseLowCols = {
    {"Mã SP", 125, 0.0f, false},
    {"Tên điện thoại & Cấu hình", 220, 0.0f, true},
    {"Hãng", 115, 0.5f, false},
    {"Giá nhập tham khảo", 160, 1.0f, false},
    {"Còn khả dụng", 140, 1.0f, false},
    {"Thao tác", 155, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kWarehouseReceiptCols = {
    {"Mã phiếu", 190, 0.0f, false},       {"Nhà cung cấp", 190, 0.0f, true},
    {"Người nhập", 160, 0.0f, false},     {"Thời gian nhập", 150, 0.0f, false},
    {"Tổng tiền nhập", 160, 1.0f, false}, {"Thao tác", 115, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kDraftReceiptCols = {
    {"Mã máy", 130, 0.0f, false},
    {"Tên điện thoại nhập", 200, 0.0f, true},
    {"Đơn giá nhập", 155, 1.0f, false},
    {"Số lượng", 160, 0.5f, false},
    {"Thành tiền", 165, 1.0f, false},
    {"Xóa", 100, 0.5f, false}};

const std::vector<UIComponents::TableColumnSpec> kWarehouseFulfillCols = {
    {"Mã đơn", 190, 0.0f, false},
    {"Thông tin người nhận", 190, 0.0f, false},
    {"Danh sách máy cần lấy", 220, 0.0f, true},
    {"Trạng thái", 155, 0.5f, false},
    {"Giao nhận", 155, 0.5f, false},
    {"Thao tác xử lý", 185, 0.5f, false}};
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

  std::vector<ProductItemViewModel> allInv = controller->getInventory(false);
  int totalStockQuantity = 0;
  double totalInventoryValue = 0.0;
  int lowStockCount = 0;
  for (const auto &p : allInv) {
    totalStockQuantity += p.actualStock;
    totalInventoryValue += (p.importPrice * p.actualStock);
    if (p.status == StockDisplayStatus::LOW_STOCK ||
        p.status == StockDisplayStatus::OUT_OF_STOCK ||
        p.availableStock <= 5) {
      lowStockCount++;
    }
  }

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

  GtkWidget *c1 = UIComponents::createClickableStatCard(
      "Tổng lượng máy trong kho",
      UIComponents::formatNumber(totalStockQuantity) + " máy",
      "Giá trị lưu kho: " + UIComponents::formatVND(totalInventoryValue),
      "📦", "purple",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        WarehouseScreens *s = static_cast<WarehouseScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("warehouse_inventory");
      }),
      this);
  gtk_widget_set_tooltip_text(c1, "Nhấp để xem danh sách tồn kho thiết bị");

  GtkWidget *c2 = UIComponents::createClickableStatCard(
      "Cảnh báo sắp hết hàng",
      UIComponents::formatNumber(lowStockCount) + " mặt hàng",
      lowStockCount > 0 ? "Dưới ngưỡng an toàn (<= 5 máy)"
                        : "Toàn bộ tồn kho ổn định",
      "⚠️", lowStockCount > 0 ? "red" : "green",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        WarehouseScreens *s = static_cast<WarehouseScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("warehouse_low_stock");
      }),
      this);
  gtk_widget_set_tooltip_text(c2,
                              "Nhấp để xem danh sách sản phẩm sắp hết hàng");

  GtkWidget *c3 = UIComponents::createClickableStatCard(
      "Phiếu nhập kho", UIComponents::formatNumber(receipts.size()) + " phiếu",
      "Lịch sử nhập từ nhà cung cấp", "📥", "blue",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        WarehouseScreens *s = static_cast<WarehouseScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("warehouse_receipts");
      }),
      this);
  gtk_widget_set_tooltip_text(c3, "Nhấp để xem lịch sử phiếu nhập kho");

  GtkWidget *c4 = UIComponents::createClickableStatCard(
      "Đơn cần đóng gói", UIComponents::formatNumber(prepareCount) + " đơn",
      prepareCount > 0 ? "Chờ xuất kho & đóng gói"
                       : "Đã xử lý hết hàng đợi xuất",
      "🚚", prepareCount > 0 ? "amber" : "cyan",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        WarehouseScreens *s = static_cast<WarehouseScreens *>(d);
        if (s && s->shell)
          s->shell->navigateTo("warehouse_fulfillment");
      }),
      this);
  gtk_widget_set_tooltip_text(c4,
                              "Nhấp để mở hàng đợi đóng gói & chuẩn bị đơn");

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
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnExportInv = UIComponents::createExportButton(
      "📥 Xuất CSV", "Xuất danh sách tồn kho thiết bị ra file .csv", [this]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName =
            "DanhSach_TonKho_" +
            ExportHelper::makeSafeDateToken(controller->getSimulatedDate()) +
            ".csv";
        UIComponents::showSaveFileDialog(
            win, "Xuất danh sách tồn kho (.csv)", defName, "Tệp CSV (*.csv)",
            "*.csv", [this, win](const std::string &path) {
              OperationResult res = controller->exportProductsCsv(path);
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
      "Danh sách tồn kho thiết bị",
      "Theo dõi số lượng thực tế, số lượng đã giữ chỗ và số lượng có thể bán.",
      btnExportInv);
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

  if (prods.empty()) {
    UIComponents::updateRecordCountBadge(inventoryCountBadge, 0, "dòng máy");
    gtk_box_append(GTK_BOX(inventoryListBox),
                   UIComponents::createEmptyState(
                       "🔍", "Chưa có dữ liệu tồn kho",
                       "Không có sản phẩm nào khớp với từ khóa tìm kiếm."));
    return;
  }

  struct WarehouseFamilyGroup {
    std::string baseModel;
    std::string brand;
    std::string os;
    std::vector<ProductItemViewModel> variants;
    int totalActual = 0;
    int totalReserved = 0;
    int totalAvailable = 0;
    double minImportPrice = 0.0;
    double maxImportPrice = 0.0;
  };

  std::vector<WarehouseFamilyGroup> groups;
  for (const auto &p : prods) {
    std::string bKey = p.baseModel.empty() ? p.name : p.baseModel;
    WarehouseFamilyGroup *target = nullptr;
    for (auto &g : groups) {
      if (g.baseModel == bKey && g.brand == p.brand) {
        target = &g;
        break;
      }
    }
    if (!target) {
      WarehouseFamilyGroup ng;
      ng.baseModel = bKey;
      ng.brand = p.brand;
      ng.os = p.os;
      ng.minImportPrice = p.importPrice;
      ng.maxImportPrice = p.importPrice;
      groups.push_back(ng);
      target = &groups.back();
    }
    target->variants.push_back(p);
    target->totalActual += p.actualStock;
    target->totalReserved += p.reservedStock;
    target->totalAvailable += p.availableStock;
    if (p.importPrice < target->minImportPrice)
      target->minImportPrice = p.importPrice;
    if (p.importPrice > target->maxImportPrice)
      target->maxImportPrice = p.importPrice;
  }

  UIComponents::updateRecordCountBadge(
      inventoryCountBadge, groups.size(),
      "dòng máy (" + std::to_string(prods.size()) + " tổ hợp)");

  for (size_t i = 0; i < groups.size(); ++i) {
    const auto &g = groups[i];
    GtkWidget *grpBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *row = UIComponents::createTableDataRow(i);

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
        capStr += "/";
      capStr += caps[c];
    }
    for (size_t c = 0; c < cols.size(); ++c) {
      if (c > 0)
        colStr += ", ";
      colStr += cols[c];
    }

    // Col 0: Tên dòng thiết bị & Tóm tắt tổ hợp
    std::string titleTxt =
        g.baseModel + " (" + std::to_string(g.variants.size()) + " tổ hợp)";
    std::string subTxt =
        (g.os.empty() ? "" : ("HĐH: " + g.os + " • ")) +
        (capStr.empty() ? "" : (capStr + " • ")) +
        (colStr.empty() ? "" : ("Màu: " + colStr));
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     titleTxt, subTxt, kWarehouseInvCols[0]));

    // Col 1: Hãng badge
    GtkWidget *bBadge =
        UIComponents::createStatusBadge(g.brand, "role-pill-sales");
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     bBadge, kWarehouseInvCols[1]));

    // Col 2: Tình trạng badge
    StockDisplayStatus grpStatus =
        (g.totalAvailable <= 0)
            ? StockDisplayStatus::OUT_OF_STOCK
            : (g.totalAvailable <= 5 ? StockDisplayStatus::LOW_STOCK
                                     : StockDisplayStatus::IN_STOCK);
    GtkWidget *sBadge = UIComponents::createStockStatusBadge(grpStatus);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     sBadge, kWarehouseInvCols[2]));

    // Col 3: Giá nhập (căn phải)
    std::string impTxt =
        (std::abs(g.minImportPrice - g.maxImportPrice) < 1.0)
            ? UIComponents::formatVND(g.minImportPrice)
            : (UIComponents::formatCompactVND(g.minImportPrice) + " – " +
               UIComponents::formatCompactVND(g.maxImportPrice));
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     impTxt, kWarehouseInvCols[3],
                                     "cell-number"));

    // Col 4: Thực tế (căn phải)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(g.totalActual) + " máy",
                       kWarehouseInvCols[4], "cell-number"));

    // Col 5: Giữ chỗ (căn phải)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(g.totalReserved) + " máy",
                       kWarehouseInvCols[5], "cell-number"));

    // Col 6: Có thể bán (căn phải, màu ngữ nghĩa)
    std::string stockCssClass = "stock-ok";
    if (grpStatus == StockDisplayStatus::OUT_OF_STOCK) {
      stockCssClass = "stock-critical";
    } else if (grpStatus == StockDisplayStatus::LOW_STOCK) {
      stockCssClass = "stock-low";
    }
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       UIComponents::formatNumber(g.totalAvailable) + " máy",
                       kWarehouseInvCols[6], stockCssClass));

    GtkWidget *detailBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_visible(detailBox, !keyword.empty());

    for (const auto &v : g.variants) {
      GtkWidget *subRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,
                                      DesignTokens::TABLE_CELL_SPACING);
      gtk_widget_add_css_class(subRow, "table-row");
      gtk_widget_add_css_class(subRow, "row-odd");

      std::string vTitle =
          "   ↳ 📦 Phiên bản: " +
          (v.capacity.empty() ? v.name : (v.capacity + " — " + v.color));
      std::string vSub = "   Mã SKU: " + v.id;
      gtk_box_append(GTK_BOX(subRow), UIComponents::createTableTwoLineCell(
                                          vTitle, vSub, kWarehouseInvCols[0]));

      GtkWidget *vBrandSlot = gtk_label_new("");
      gtk_box_append(GTK_BOX(subRow), UIComponents::createTableWidgetCell(
                                          vBrandSlot, kWarehouseInvCols[1]));

      GtkWidget *vBadge = UIComponents::createStockStatusBadge(v.status);
      gtk_box_append(GTK_BOX(subRow), UIComponents::createTableWidgetCell(
                                          vBadge, kWarehouseInvCols[2]));

      gtk_box_append(GTK_BOX(subRow),
                     UIComponents::createTableTextCell(
                         UIComponents::formatVND(v.importPrice),
                         kWarehouseInvCols[3], "cell-number"));

      gtk_box_append(GTK_BOX(subRow),
                     UIComponents::createTableTextCell(
                         UIComponents::formatNumber(v.actualStock) + " máy",
                         kWarehouseInvCols[4], "cell-number"));

      gtk_box_append(GTK_BOX(subRow),
                     UIComponents::createTableTextCell(
                         UIComponents::formatNumber(v.reservedStock) + " máy",
                         kWarehouseInvCols[5], "cell-number"));

      std::string vStockCss =
          (v.status == StockDisplayStatus::OUT_OF_STOCK)
              ? "stock-critical"
              : (v.status == StockDisplayStatus::LOW_STOCK ? "stock-low"
                                                           : "stock-ok");
      gtk_box_append(GTK_BOX(subRow),
                     UIComponents::createTableTextCell(
                         UIComponents::formatNumber(v.availableStock) + " máy",
                         kWarehouseInvCols[6], vStockCss));

      GtkWidget *vEmptyAct = gtk_label_new("");
      gtk_box_append(GTK_BOX(subRow), UIComponents::createTableWidgetCell(
                                          vEmptyAct, kWarehouseInvCols[7]));

      gtk_box_append(GTK_BOX(detailBox), subRow);
    }

    GtkWidget *btnExp = nullptr;
    if (g.variants.size() > 1) {
      btnExp = UIComponents::createTableActionButton(
          !keyword.empty() ? "▴ Thu gọn" : "▾ Chi tiết",
          "Xem chi tiết tồn kho từng tổ hợp", "btn-action-view");
      g_signal_connect(
          btnExp, "clicked",
          G_CALLBACK(+[](GtkButton *btn, gpointer d) {
            GtkWidget *box = static_cast<GtkWidget *>(d);
            bool vis = !gtk_widget_get_visible(box);
            gtk_widget_set_visible(box, vis);
            gtk_button_set_label(btn, vis ? "▴ Thu gọn" : "▾ Chi tiết");
          }),
          detailBox);
    } else {
      btnExp = gtk_label_new("1 mẫu");
      gtk_widget_add_css_class(btnExp, "cell-subtitle");
    }
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnExp, kWarehouseInvCols[7]));

    gtk_box_append(GTK_BOX(grpBox), row);
    gtk_box_append(GTK_BOX(grpBox), detailBox);
    gtk_box_append(GTK_BOX(inventoryListBox), grpBox);
  }
}

// ================= 3. SẢN PHẨM SẮP HẾT HÀNG =================

void WarehouseScreens::buildLowStockView() {
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
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *actionBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  GtkWidget *btnExportRec = UIComponents::createExportButton(
      "📥 Xuất CSV", "Xuất danh sách phiếu nhập kho ra file .csv", [this]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName =
            "DanhSach_PhieuNhapKho_" +
            ExportHelper::makeSafeDateToken(controller->getSimulatedDate()) +
            ".csv";
        UIComponents::showSaveFileDialog(
            win, "Xuất danh sách phiếu nhập kho (.csv)", defName,
            "Tệp CSV (*.csv)", "*.csv", [this, win](const std::string &path) {
              OperationResult res = controller->exportGoodsReceiptsCsv(
                  path, receiptsKeyword, receiptsFromDate, receiptsToDate);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(win, "Xuất CSV thành công",
                                                res.message);
              } else {
                UIComponents::showErrorDialog(win, "Lỗi xuất file",
                                              res.message);
              }
            });
      });
  gtk_box_append(GTK_BOX(actionBox), btnExportRec);

  GtkWidget *btnNew = gtk_button_new_with_label("➕ Lập phiếu nhập mới");
  gtk_widget_add_css_class(btnNew, "btn-primary");
  UIComponents::setButtonCursor(btnNew);
  g_signal_connect_swapped(btnNew, "clicked",
                           G_CALLBACK(+[](MainShellWindow *s) {
                             if (s)
                               s->navigateTo("warehouse_create_receipt");
                           }),
                           shell);
  gtk_box_append(GTK_BOX(actionBox), btnNew);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh sách phiếu nhập kho",
      "Theo dõi lịch sử nhập hàng từ các nhà cung cấp.", actionBox);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  GtkWidget *lblFrom = gtk_label_new("Từ:");
  gtk_widget_add_css_class(lblFrom, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblFrom);

  entryReceiptsFromDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryReceiptsFromDate);
  g_signal_connect(entryReceiptsFromDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     WarehouseScreens *self =
                         static_cast<WarehouseScreens *>(d);
                     self->receiptsFromDate = gtk_editable_get_text(e);
                     self->refreshReceipts();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryReceiptsFromDate);

  GtkWidget *lblTo = gtk_label_new("Đến:");
  gtk_widget_add_css_class(lblTo, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblTo);

  entryReceiptsToDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryReceiptsToDate);
  g_signal_connect(entryReceiptsToDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     WarehouseScreens *self =
                         static_cast<WarehouseScreens *>(d);
                     self->receiptsToDate = gtk_editable_get_text(e);
                     self->refreshReceipts();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryReceiptsToDate);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã phiếu, nhà cung cấp hoặc thủ kho nhập...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->receiptsKeyword = txt ? txt : "";
        self->refreshReceipts();
      }),
      this, nullptr, &receiptsCountBadge, extraControls);
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
    // Lọc theo khoảng ngày (ANTY.md mục 2)
    if (!receiptsFromDate.empty() || !receiptsToDate.empty()) {
      if (!RealTimeManager::isDateInRange(r.importTime, receiptsFromDate,
                                          receiptsToDate)) {
        continue;
      }
    }
    list.push_back(r);
  }

  // Tự động sắp xếp theo thời gian sớm nhất (ANTY.md mục 2)
  std::sort(list.begin(), list.end(),
            [](const GoodsReceiptSummaryViewModel &a,
               const GoodsReceiptSummaryViewModel &b) {
              return RealTimeManager::dateTimeToEpochSeconds(a.importTime) <
                     RealTimeManager::dateTimeToEpochSeconds(b.importTime);
            });

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

  GtkWidget *title = gtk_label_new(("📥 PHIẾU NHẬP KHO " + receipt.id).c_str());
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

  addRow("Nhà cung cấp:", receipt.supplierName);
  addRow("Thủ kho nhập:", receipt.employeeName);
  addRow("Thời gian nhập:", receipt.importTime);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(card), sep);

  GtkWidget *itemsBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  for (size_t idx = 0; idx < receipt.details.size(); ++idx) {
    const auto &d = receipt.details[idx];
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

  addRow("Tổng tiền nhập:", UIComponents::formatVND(receipt.totalAmount));

  GtkWidget *footerRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  gtk_widget_set_halign(footerRow, GTK_ALIGN_END);

  std::string recIdCopy = receipt.id;
  GtkWidget *btnExportTxt = UIComponents::createExportButton(
      "📄 Xuất phiếu nhập (.txt)",
      "Xuất phiếu nhập kho chi tiết sang tệp văn bản (.txt)",
      [this, recIdCopy]() {
        GtkWindow *win = shell ? shell->getWindow() : nullptr;
        std::string defName = "PhieuNhapKho_" + recIdCopy + ".txt";
        UIComponents::showSaveFileDialog(
            win, "Xuất phiếu nhập kho (.txt)", defName, "Tệp văn bản (*.txt)",
            "*.txt", [this, win, recIdCopy](const std::string &path) {
              OperationResult res =
                  controller->exportGoodsReceiptTxt(path, recIdCopy);
              if (res.isSuccess()) {
                UIComponents::showSuccessDialog(
                    win, "Xuất phiếu nhập thành công", res.message);
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
  shell->pushPage("warehouse_receipt_detail", scroll,
                  "Chi tiết phiếu nhập kho " + receipt.id);
}

// ================= 5. TẠO PHIẾU NHẬP KHO =================

void WarehouseScreens::buildCreateReceiptView() {
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
  if (!shell)
    return;

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

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *pageContent =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(pageContent, "page-container");

  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "pdp-card");
  gtk_widget_set_hexpand(card, TRUE);

  GtkWidget *title = gtk_label_new("📦 Thêm sản phẩm vào phiếu nhập kho");
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  GtkWidget *subtitle = gtk_label_new(
      "Tìm kiếm và chọn dòng điện thoại thuộc nhà cung cấp hiện tại, nhập số "
      "lượng và đơn giá nhập thực tế.");
  gtk_widget_add_css_class(subtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
  gtk_box_append(GTK_BOX(card), subtitle);

  auto formatProdOption = [](const ProductItemViewModel &p) -> std::string {
    return "[" + p.id + "] " + p.name + " — Tồn kho: " +
           UIComponents::formatNumber(p.actualStock) + " máy";
  };

  // Ô tìm kiếm sản phẩm trong nhập hàng
  GtkWidget *entryProdSearch = gtk_entry_new();
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(entryProdSearch),
      "🔍 Nhập tên máy, mã SP (PHN-...), hãng, dung lượng hoặc màu sắc...");
  gtk_entry_set_icon_from_icon_name(GTK_ENTRY(entryProdSearch),
                                    GTK_ENTRY_ICON_PRIMARY,
                                    "edit-find-symbolic");

  GtkWidget *lblProdSearchStatus = gtk_label_new(
      ("Hiển thị toàn bộ " + std::to_string(prods.size()) +
       " sản phẩm thuộc nhà cung cấp đã chọn.")
          .c_str());
  gtk_widget_add_css_class(lblProdSearchStatus, "form-hint");
  gtk_label_set_xalign(GTK_LABEL(lblProdSearchStatus), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblProdSearchStatus), TRUE);

  std::vector<std::string> initDispStrs;
  std::vector<const char *> prodNames;
  initDispStrs.reserve(prods.size());
  for (const auto &p : prods) {
    initDispStrs.push_back(formatProdOption(p));
  }
  for (const auto &s : initDispStrs) {
    prodNames.push_back(s.c_str());
  }
  prodNames.push_back(nullptr);

  GtkWidget *errCombo = nullptr;
  GtkWidget *comboProd = gtk_drop_down_new_from_strings(prodNames.data());
  gtk_drop_down_set_enable_search(GTK_DROP_DOWN(comboProd), TRUE);

  GtkWidget *prodPickerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_append(GTK_BOX(prodPickerBox), entryProdSearch);
  gtk_box_append(GTK_BOX(prodPickerBox), lblProdSearchStatus);
  gtk_box_append(GTK_BOX(prodPickerBox), comboProd);
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Chọn điện thoại nhập kho *",
                                               prodPickerBox, &errCombo));

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

  GtkWidget *btnRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 8);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy (Esc)");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->popPage(); }), shell);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  GtkWidget *btnAdd = gtk_button_new_with_label("Thêm vào phiếu");
  gtk_widget_add_css_class(btnAdd, "btn-primary");
  UIComponents::setButtonCursor(btnAdd);

  struct AddProdReceiptCtx {
    WarehouseScreens *screens;
    GtkWidget *eSearch;
    GtkWidget *lblSearchStatus;
    GtkWidget *combo;
    GtkWidget *eQty;
    GtkWidget *ePrice;
    GtkWidget *errCombo;
    GtkWidget *errQty;
    GtkWidget *errPrice;
    GtkWidget *btnAdd;
    std::vector<ProductItemViewModel> allProds;
    std::vector<ProductItemViewModel> filteredProds;
  };
  AddProdReceiptCtx *ctx = new AddProdReceiptCtx{
      this,     entryProdSearch, lblProdSearchStatus, comboProd,
      entryQty, entryPrice,      errCombo,            errQty,
      errPrice, btnAdd,          prods,               prods};

  // Gắn vòng đời bộ nhớ ctx vào card để tránh giải phóng sớm khi các signal cùng dùng chung
  g_object_set_data_full(
      G_OBJECT(card), "add_prod_receipt_ctx", ctx,
      [](gpointer d) { delete static_cast<AddProdReceiptCtx *>(d); });

  auto syncPriceFromSelection = [](AddProdReceiptCtx *c) {
    guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->combo));
    if (sel < c->filteredProds.size()) {
      long long pr = static_cast<long long>(c->filteredProds[sel].importPrice);
      gtk_editable_set_text(GTK_EDITABLE(c->ePrice),
                            UIComponents::formatNumber(pr).c_str());
    }
  };

  // Tìm kiếm trực tiếp sản phẩm theo tên, mã SP, hãng, dung lượng, màu sắc, cấu hình
  g_signal_connect_data(
      entryProdSearch, "changed",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        AddProdReceiptCtx *c = static_cast<AddProdReceiptCtx *>(d);
        const char *txt = gtk_editable_get_text(e);
        std::string kw = txt ? StringHelper::trim(txt) : "";

        c->filteredProds.clear();
        std::vector<std::string> dispStrs;
        std::vector<const char *> ptrs;

        for (const auto &p : c->allProds) {
          if (!kw.empty()) {
            bool match =
                Utf8Helper::containsCaseInsensitive(p.name, kw) ||
                Utf8Helper::containsCaseInsensitive(p.id, kw) ||
                Utf8Helper::containsCaseInsensitive(p.brand, kw) ||
                Utf8Helper::containsCaseInsensitive(p.baseModel, kw) ||
                Utf8Helper::containsCaseInsensitive(p.capacity, kw) ||
                Utf8Helper::containsCaseInsensitive(p.color, kw) ||
                Utf8Helper::containsCaseInsensitive(p.specs, kw);
            if (!match)
              continue;
          }
          c->filteredProds.push_back(p);
          dispStrs.push_back("[" + p.id + "] " + p.name + " — Tồn kho: " +
                             UIComponents::formatNumber(p.actualStock) +
                             " máy");
        }

        if (c->filteredProds.empty()) {
          dispStrs.push_back("(Không tìm thấy sản phẩm nào khớp với '" + kw +
                             "')");
          ptrs.push_back(dispStrs[0].c_str());
          ptrs.push_back(nullptr);
          GtkStringList *sList = gtk_string_list_new(ptrs.data());
          gtk_drop_down_set_model(GTK_DROP_DOWN(c->combo), G_LIST_MODEL(sList));
          g_object_unref(sList);
          gtk_label_set_text(
              GTK_LABEL(c->lblSearchStatus),
              ("⚠️ Không tìm thấy sản phẩm nào của nhà cung cấp khớp với '" +
               kw + "'.")
                  .c_str());
          gtk_widget_set_sensitive(c->btnAdd, FALSE);
          return;
        }

        for (const auto &s : dispStrs) {
          ptrs.push_back(s.c_str());
        }
        ptrs.push_back(nullptr);

        GtkStringList *sList = gtk_string_list_new(ptrs.data());
        gtk_drop_down_set_model(GTK_DROP_DOWN(c->combo), G_LIST_MODEL(sList));
        g_object_unref(sList);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(c->combo), 0);

        if (kw.empty()) {
          gtk_label_set_text(
              GTK_LABEL(c->lblSearchStatus),
              ("Hiển thị toàn bộ " + std::to_string(c->filteredProds.size()) +
               " sản phẩm thuộc nhà cung cấp đã chọn.")
                  .c_str());
        } else {
          gtk_label_set_text(
              GTK_LABEL(c->lblSearchStatus),
              ("✅ Tìm thấy " + std::to_string(c->filteredProds.size()) + "/" +
               std::to_string(c->allProds.size()) + " sản phẩm phù hợp.")
                  .c_str());
        }
        gtk_widget_set_sensitive(c->btnAdd, TRUE);
        long long pr =
            static_cast<long long>(c->filteredProds[0].importPrice);
        gtk_editable_set_text(GTK_EDITABLE(c->ePrice),
                              UIComponents::formatNumber(pr).c_str());
      }),
      ctx, nullptr, (GConnectFlags)0);

  g_signal_connect_data(
      comboProd, "notify::selected",
      G_CALLBACK(+[](GObject *, GParamSpec *, gpointer d) {
        AddProdReceiptCtx *c = static_cast<AddProdReceiptCtx *>(d);
        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->combo));
        if (sel < c->filteredProds.size()) {
          long long pr =
              static_cast<long long>(c->filteredProds[sel].importPrice);
          gtk_editable_set_text(GTK_EDITABLE(c->ePrice),
                                UIComponents::formatNumber(pr).c_str());
        }
      }),
      ctx, nullptr, (GConnectFlags)0);

  (void)syncPriceFromSelection;

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

        if (sel >= c->filteredProds.size()) {
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

        const ProductItemViewModel &chosenProd = c->filteredProds[sel];
        bool merged = false;
        for (auto &existing : c->screens->draftReceiptItems) {
          if (existing.phoneId == chosenProd.id) {
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
          item.phoneId = chosenProd.id;
          item.phoneName = chosenProd.name;
          item.quantity = qty;
          item.importPrice = price;
          item.subtotal = qty * price;
          c->screens->draftReceiptItems.push_back(item);
        }

        c->screens->refreshDraftReceiptItems();
        if (c->screens->shell) {
          c->screens->shell->popPage();
        }
      }),
      ctx, nullptr, (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnAdd);
  gtk_box_append(GTK_BOX(card), btnRow);

  UIComponents::setupDialogShortcuts(card, btnAdd, btnCancel);

  gtk_box_append(GTK_BOX(pageContent), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("warehouse_add_receipt_item", scroll,
                  "Thêm sản phẩm vào phiếu nhập");
  gtk_widget_grab_focus(entryProdSearch);
}

// ================= 6. CHUẨN BỊ ĐƠN HÀNG (FULFILLMENT) =================

void WarehouseScreens::buildFulfillmentView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header =
      UIComponents::createSectionHeader("Chuẩn bị đơn hàng (Fulfillment)",
                                        "Lấy hàng, đóng gói và chuyển giao cho "
                                        "vận chuyển hoặc để tại quầy nhận.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  GtkWidget *lblFrom = gtk_label_new("Từ:");
  gtk_widget_add_css_class(lblFrom, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblFrom);

  entryFulfillmentFromDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryFulfillmentFromDate);
  g_signal_connect(entryFulfillmentFromDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     WarehouseScreens *self =
                         static_cast<WarehouseScreens *>(d);
                     self->fulfillmentFromDate = gtk_editable_get_text(e);
                     self->refreshFulfillmentQueue();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryFulfillmentFromDate);

  GtkWidget *lblTo = gtk_label_new("Đến:");
  gtk_widget_add_css_class(lblTo, "stat-label");
  gtk_box_append(GTK_BOX(extraControls), lblTo);

  entryFulfillmentToDate = gtk_entry_new();
  UIComponents::setupDatePicker(entryFulfillmentToDate);
  g_signal_connect(entryFulfillmentToDate, "changed",
                   G_CALLBACK(+[](GtkEditable *e, gpointer d) {
                     WarehouseScreens *self =
                         static_cast<WarehouseScreens *>(d);
                     self->fulfillmentToDate = gtk_editable_get_text(e);
                     self->refreshFulfillmentQueue();
                   }),
                   this);
  gtk_box_append(GTK_BOX(extraControls), entryFulfillmentToDate);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã đơn, người nhận, SĐT hoặc tên máy cần lấy...",
      G_CALLBACK(+[](GtkEditable *e, gpointer d) {
        WarehouseScreens *self = static_cast<WarehouseScreens *>(d);
        const char *txt = gtk_editable_get_text(e);
        self->fulfillmentKeyword = txt ? txt : "";
        self->refreshFulfillmentQueue();
      }),
      this, nullptr, &fulfillmentCountBadge, extraControls);
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
    // Lọc theo khoảng ngày (ANTY.md mục 2)
    if (!fulfillmentFromDate.empty() || !fulfillmentToDate.empty()) {
      if (!RealTimeManager::isDateInRange(ord.orderTime, fulfillmentFromDate,
                                          fulfillmentToDate)) {
        continue;
      }
    }
    toFulfill.push_back(ord);
  }

  // Tự động sắp xếp theo thời gian sớm nhất (ANTY.md mục 2)
  std::sort(toFulfill.begin(), toFulfill.end(),
            [](const OrderItemViewModel &a, const OrderItemViewModel &b) {
              return RealTimeManager::dateTimeToEpochSeconds(a.orderTime) <
                     RealTimeManager::dateTimeToEpochSeconds(b.orderTime);
            });

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
