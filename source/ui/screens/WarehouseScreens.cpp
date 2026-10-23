/**
 * @file WarehouseScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ QUẢN LÝ KHO (WAREHOUSE MANAGEMENT)
 * 
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildDashboardView & refreshDashboard: Bàn làm việc Thủ kho, giám sát tổng giá trị tồn và đơn chờ xuất.
 * - buildInventoryView & refreshInventory: Danh mục tồn kho sản phẩm, bộ lọc hãng và tìm kiếm từ khóa.
 * - buildLowStockView & refreshLowStock: Cảnh báo tự động các sản phẩm có tồn kho dưới ngưỡng an toàn.
 * - buildReceiptsView & refreshReceipts: Xem danh sách và chi tiết phiếu nhập kho từ nhà cung cấp.
 * - buildCreateReceiptView: Nhập hàng mới, chọn nhà cung cấp, thêm chi tiết sản phẩm và tính toán tổng tiền.
 * - buildFulfillmentView: Danh sách đóng gói đơn hàng online (chuyển trạng thái sang SHIPPING).
 * =========================================================================================
 */

#include "WarehouseScreens.h"
#include "MainShellWindow.h"
#include "../components/UIComponents.h"

WarehouseScreens::WarehouseScreens(AppController* ctrl, MainShellWindow* parentShell)
    : controller(ctrl), shell(parentShell), draftSupplierId("") {

    dashboardView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    inventoryView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(inventoryView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    lowStockView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(lowStockView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    receiptsView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(receiptsView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    createReceiptView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(createReceiptView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    fulfillmentView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(fulfillmentView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

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

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_add_css_class(content, "page-container");

    const UserSessionViewModel& user = controller->getCurrentUser();
    std::string title = "Bàn làm việc Nhân viên Kho — " + user.fullName;
    GtkWidget* header = UIComponents::createSectionHeader(title, "Quản lý tồn kho, xuất nhập hàng và đóng gói đơn hàng cần xử lý.");
    gtk_box_append(GTK_BOX(content), header);

    // Metrics Grid
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    DashboardStatsViewModel stats = controller->getDashboardStats();
    std::vector<GoodsReceiptSummaryViewModel> receipts = controller->getGoodsReceipts();
    std::vector<OrderItemViewModel> orders = controller->getAllOrders();

    int prepareCount = 0;
    for (const auto& o : orders) {
        if (o.status == OrderStatus::CONFIRMED || o.status == OrderStatus::PREPARING) {
            prepareCount++;
        }
    }

    GtkWidget* c1 = UIComponents::createStatCard("Tổng lượng máy trong kho", std::to_string(stats.totalStockQuantity) + " máy", "Đang lưu kho", "📦");
    GtkWidget* c2 = UIComponents::createStatCard("Cảnh báo sắp hết hàng", std::to_string(stats.lowStockCount) + " mặt hàng", "Cần nhập thêm", "⚠️", "red");
    GtkWidget* c3 = UIComponents::createStatCard("Phiếu nhập kho", std::to_string(receipts.size()) + " phiếu", "Lịch sử nhập", "📥");
    GtkWidget* c4 = UIComponents::createStatCard("Đơn cần đóng gói", std::to_string(prepareCount) + " đơn", "Chờ fulfillment", "🚚");

    gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c4, 3, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), grid);

    // Quick Actions
    GtkWidget* actionCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(actionCard, "card");

    GtkWidget* lblActTitle = gtk_label_new("⚡ Thao tác nghiệp vụ kho");
    gtk_widget_add_css_class(lblActTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblActTitle), 0.0f);
    gtk_box_append(GTK_BOX(actionCard), lblActTitle);

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    GtkWidget* bInv = gtk_button_new_with_label("📦 Kiểm tra tồn kho");
    gtk_widget_add_css_class(bInv, "btn-primary");
    UIComponents::setButtonCursor(bInv);
    g_signal_connect_swapped(bInv, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("warehouse_inventory");
    }), shell);
    gtk_box_append(GTK_BOX(btnRow), bInv);

    GtkWidget* bRec = gtk_button_new_with_label("➕ Lập phiếu nhập kho");
    gtk_widget_add_css_class(bRec, "btn-success");
    UIComponents::setButtonCursor(bRec);
    g_signal_connect_swapped(bRec, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("warehouse_create_receipt");
    }), shell);
    gtk_box_append(GTK_BOX(btnRow), bRec);

    GtkWidget* bFulfill = gtk_button_new_with_label("🚚 Chuẩn bị đơn hàng");
    gtk_widget_add_css_class(bFulfill, "btn-secondary");
    UIComponents::setButtonCursor(bFulfill);
    g_signal_connect_swapped(bFulfill, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("warehouse_fulfillment");
    }), shell);
    gtk_box_append(GTK_BOX(btnRow), bFulfill);

    gtk_box_append(GTK_BOX(actionCard), btnRow);
    gtk_box_append(GTK_BOX(content), actionCard);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(dashboardView), content);
}

void WarehouseScreens::refreshDashboard() {
    buildDashboardView();
}

// ================= 2. QUẢN LÝ TỒN KHO =================

void WarehouseScreens::buildInventoryView() {
    inventoryView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(inventoryView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Danh sách tồn kho thiết bị", "Theo dõi số lượng thực tế, số lượng đã giữ chỗ và số lượng có thể bán.");
    gtk_box_append(GTK_BOX(content), header);

    // Search bar
    GtkWidget* searchBar = UIComponents::createSearchBar("Tìm theo tên điện thoại, mã hoặc thương hiệu...", G_CALLBACK(+[](GtkEditable* e, gpointer d) {
        WarehouseScreens* self = static_cast<WarehouseScreens*>(d);
        self->refreshInventory(gtk_editable_get_text(e));
    }), this);
    gtk_box_append(GTK_BOX(content), searchBar);

    std::vector<UIComponents::TableColumnSpec> invCols = {
        {"Tên thiết bị & Mã", 0, 0.0f, true},
        {"Hãng", 100, 0.5f, false},
        {"Tình trạng", 130, 0.5f, false},
        {"Giá nhập", 130, 1.0f, false},
        {"Thực tế", 90, 1.0f, false},
        {"Giữ chỗ", 90, 1.0f, false},
        {"Có thể bán", 110, 1.0f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(invCols));

    inventoryListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), inventoryListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(inventoryView), content);
    refreshInventory();
}

void WarehouseScreens::refreshInventory(const std::string& keyword, const std::string& brand) {
    GtkWidget* child = gtk_widget_get_first_child(inventoryListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(inventoryListBox), child);
        child = next;
    }

    ProductFilterRequest req;
    req.keyword = keyword;
    req.brand = brand;
    std::vector<ProductItemViewModel> prods = controller->getProducts(req);

    if (prods.empty()) {
        gtk_box_append(GTK_BOX(inventoryListBox), UIComponents::createEmptyState("🔍", "Chưa có dữ liệu", "Không có sản phẩm nào khớp với tìm kiếm."));
        return;
    }

    for (size_t i = 0; i < prods.size(); ++i) {
        const auto& p = prods[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Tên thiết bị & Mã
        GtkWidget* info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_hexpand(info, TRUE);
        gtk_widget_set_valign(info, GTK_ALIGN_CENTER);
        GtkWidget* nLbl = gtk_label_new(p.name.c_str());
        gtk_widget_add_css_class(nLbl, "product-name");
        gtk_label_set_xalign(GTK_LABEL(nLbl), 0.0f);
        GtkWidget* sub = gtk_label_new(("Mã: " + p.id + " | HĐH: " + p.os).c_str());
        gtk_widget_add_css_class(sub, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(sub), 0.0f);
        gtk_box_append(GTK_BOX(info), nLbl);
        gtk_box_append(GTK_BOX(info), sub);
        gtk_box_append(GTK_BOX(row), info);

        // Hãng badge
        GtkWidget* bBadge = UIComponents::createStatusBadge(p.brand, "role-pill-sales");
        GtkWidget* bBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(bBox, 100, -1);
        gtk_widget_set_valign(bBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(bBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(bBox), bBadge);
        gtk_box_append(GTK_BOX(row), bBox);

        // Tình trạng badge
        GtkWidget* sBadge = UIComponents::createStockStatusBadge(p.status);
        GtkWidget* sBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(sBox, 130, -1);
        gtk_widget_set_valign(sBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(sBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(sBox), sBadge);
        gtk_box_append(GTK_BOX(row), sBox);

        // Giá nhập (căn phải)
        GtkWidget* pLbl = gtk_label_new(UIComponents::formatVND(p.importPrice).c_str());
        gtk_widget_set_size_request(pLbl, 130, -1);
        gtk_label_set_xalign(GTK_LABEL(pLbl), 1.0f);
        gtk_widget_set_valign(pLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), pLbl);

        // Thực tế (căn phải)
        GtkWidget* actLbl = gtk_label_new((std::to_string(p.actualStock) + " máy").c_str());
        gtk_widget_add_css_class(actLbl, "card-title");
        gtk_widget_set_size_request(actLbl, 90, -1);
        gtk_label_set_xalign(GTK_LABEL(actLbl), 1.0f);
        gtk_widget_set_valign(actLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), actLbl);

        // Giữ chỗ (căn phải)
        GtkWidget* resLbl = gtk_label_new((std::to_string(p.reservedStock) + " máy").c_str());
        gtk_widget_add_css_class(resLbl, "stat-label");
        gtk_widget_set_size_request(resLbl, 90, -1);
        gtk_label_set_xalign(GTK_LABEL(resLbl), 1.0f);
        gtk_widget_set_valign(resLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), resLbl);

        // Có thể bán (căn phải)
        GtkWidget* avaLbl = gtk_label_new((std::to_string(p.availableStock) + " máy").c_str());
        gtk_widget_add_css_class(avaLbl, "product-price");
        gtk_widget_set_size_request(avaLbl, 110, -1);
        gtk_label_set_xalign(GTK_LABEL(avaLbl), 1.0f);
        gtk_widget_set_valign(avaLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), avaLbl);

        gtk_box_append(GTK_BOX(inventoryListBox), row);
    }
}

// ================= 3. SẢN PHẨM SẮP HẾT HÀNG =================

void WarehouseScreens::buildLowStockView() {
    lowStockView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(lowStockView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Cảnh báo sản phẩm sắp hết hàng", "Danh sách các thiết bị có số lượng tồn kho khả dụng dưới ngưỡng an toàn (<= 5 máy).");
    gtk_box_append(GTK_BOX(content), header);

    std::vector<UIComponents::TableColumnSpec> lowCols = {
        {"Mã SP", 110, 0.0f, false},
        {"Tên điện thoại", 0, 0.0f, true},
        {"Hãng", 110, 0.0f, false},
        {"Giá nhập tham khảo", 150, 1.0f, false},
        {"Còn khả dụng", 130, 1.0f, false},
        {"Thao tác", 140, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(lowCols));

    lowStockListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), lowStockListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(lowStockView), content);
    refreshLowStock();
}

void WarehouseScreens::refreshLowStock() {
    GtkWidget* child = gtk_widget_get_first_child(lowStockListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(lowStockListBox), child);
        child = next;
    }

    std::vector<ProductItemViewModel> lowStock = controller->getInventory(true);

    if (lowStock.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState("✅", "Tồn kho an toàn", "Tất cả sản phẩm hiện đều có số lượng khả dụng dồi dào trên 5 máy.");
        gtk_box_append(GTK_BOX(lowStockListBox), empty);
        return;
    }

    for (size_t i = 0; i < lowStock.size(); ++i) {
        const auto& p = lowStock[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Mã SP
        GtkWidget* idLbl = gtk_label_new(p.id.c_str());
        gtk_widget_add_css_class(idLbl, "product-name");
        gtk_widget_set_size_request(idLbl, 110, -1);
        gtk_label_set_xalign(GTK_LABEL(idLbl), 0.0f);
        gtk_widget_set_valign(idLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), idLbl);

        // Tên điện thoại
        GtkWidget* nameLbl = gtk_label_new(p.name.c_str());
        gtk_widget_add_css_class(nameLbl, "product-name");
        gtk_widget_set_hexpand(nameLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(nameLbl), 0.0f);
        gtk_widget_set_valign(nameLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), nameLbl);

        // Hãng
        GtkWidget* brandLbl = gtk_label_new(p.brand.c_str());
        gtk_widget_set_size_request(brandLbl, 110, -1);
        gtk_label_set_xalign(GTK_LABEL(brandLbl), 0.0f);
        gtk_widget_set_valign(brandLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), brandLbl);

        // Giá nhập tham khảo (căn phải)
        GtkWidget* priceLbl = gtk_label_new(UIComponents::formatVND(p.importPrice).c_str());
        gtk_widget_set_size_request(priceLbl, 150, -1);
        gtk_label_set_xalign(GTK_LABEL(priceLbl), 1.0f);
        gtk_widget_set_valign(priceLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), priceLbl);

        // Còn khả dụng (căn phải)
        GtkWidget* availLbl = gtk_label_new((std::to_string(p.availableStock) + " máy").c_str());
        gtk_widget_add_css_class(availLbl, "product-price");
        gtk_widget_set_size_request(availLbl, 130, -1);
        gtk_label_set_xalign(GTK_LABEL(availLbl), 1.0f);
        gtk_widget_set_valign(availLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), availLbl);

        // Thao tác
        GtkWidget* btnReorder = UIComponents::createTableActionButton("📥 Nhập hàng", "Chuyển tới màn hình lập phiếu nhập cho sản phẩm này", "btn-primary");
        gtk_widget_set_size_request(btnReorder, 140, -1);
        g_signal_connect_swapped(btnReorder, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
            s->navigateTo("warehouse_create_receipt");
        }), shell);
        gtk_box_append(GTK_BOX(row), btnReorder);

        gtk_box_append(GTK_BOX(lowStockListBox), row);
    }
}

// ================= 4. DANH SÁCH PHIẾU NHẬP KHO =================

void WarehouseScreens::buildReceiptsView() {
    receiptsView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(receiptsView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* btnNew = gtk_button_new_with_label("➕ Lập phiếu nhập mới");
    gtk_widget_add_css_class(btnNew, "btn-primary");
    UIComponents::setButtonCursor(btnNew);
    g_signal_connect_swapped(btnNew, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("warehouse_create_receipt");
    }), shell);

    GtkWidget* header = UIComponents::createSectionHeader("Danh sách phiếu nhập kho", "Theo dõi lịch sử nhập hàng từ các nhà cung cấp.", btnNew);
    gtk_box_append(GTK_BOX(content), header);

    std::vector<UIComponents::TableColumnSpec> recCols = {
        {"Mã phiếu", 120, 0.0f, false},
        {"Nhà cung cấp", 160, 0.0f, false},
        {"Người nhập", 130, 0.0f, false},
        {"Thời gian", 0, 0.0f, true},
        {"Tổng tiền nhập", 140, 1.0f, false},
        {"Trạng thái", 110, 0.5f, false},
        {"Thao tác", 160, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(recCols));

    receiptsListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), receiptsListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(receiptsView), content);
    refreshReceipts();
}

void WarehouseScreens::refreshReceipts() {
    GtkWidget* child = gtk_widget_get_first_child(receiptsListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(receiptsListBox), child);
        child = next;
    }

    std::vector<GoodsReceiptSummaryViewModel> list = controller->getGoodsReceipts();
    if (list.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState(
            "📋", "Chưa có phiếu nhập", "Chưa có phiếu nhập kho nào trong hệ thống.",
            "➕ Lập phiếu nhập mới",
            G_CALLBACK(+[](GtkButton*, gpointer d) {
                WarehouseScreens* s = static_cast<WarehouseScreens*>(d);
                s->shell->navigateTo("warehouse_create_receipt");
            }), this);
        gtk_box_append(GTK_BOX(receiptsListBox), empty);
        return;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& r = list[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Mã phiếu
        GtkWidget* idLbl = gtk_label_new(r.id.c_str());
        gtk_widget_add_css_class(idLbl, "product-name");
        gtk_widget_set_size_request(idLbl, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(idLbl), 0.0f);
        gtk_widget_set_valign(idLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), idLbl);

        // Nhà cung cấp
        GtkWidget* supLbl = gtk_label_new(r.supplierName.c_str());
        gtk_widget_set_size_request(supLbl, 160, -1);
        gtk_label_set_xalign(GTK_LABEL(supLbl), 0.0f);
        gtk_widget_set_valign(supLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), supLbl);

        // Người nhập
        GtkWidget* empLbl = gtk_label_new(r.employeeName.c_str());
        gtk_widget_set_size_request(empLbl, 130, -1);
        gtk_label_set_xalign(GTK_LABEL(empLbl), 0.0f);
        gtk_widget_set_valign(empLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), empLbl);

        // Thời gian
        GtkWidget* timeLbl = gtk_label_new(r.importTime.c_str());
        gtk_widget_set_hexpand(timeLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(timeLbl), 0.0f);
        gtk_widget_set_valign(timeLbl, GTK_ALIGN_CENTER);
        gtk_widget_add_css_class(timeLbl, "page-subtitle");
        gtk_box_append(GTK_BOX(row), timeLbl);

        // Tổng tiền (căn phải)
        GtkWidget* totalLbl = gtk_label_new(UIComponents::formatVND(r.totalAmount).c_str());
        gtk_widget_add_css_class(totalLbl, "product-price");
        gtk_widget_set_size_request(totalLbl, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(totalLbl), 1.0f);
        gtk_widget_set_valign(totalLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), totalLbl);

        // Trạng thái badge
        const bool isCancelled = (r.status == "DaHuy");
        GtkWidget* stBadge = UIComponents::createStatusBadge(
            isCancelled ? "Đã hủy" : "Hoạt động",
            isCancelled ? "status-pill-cancelled" : "status-pill-completed"
        );
        GtkWidget* stBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(stBox, 110, -1);
        gtk_widget_set_valign(stBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(stBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(stBox), stBadge);
        gtk_box_append(GTK_BOX(row), stBox);

        // Thao tác
        GtkWidget* actionBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_widget_set_size_request(actionBox, 160, -1);
        gtk_widget_set_valign(actionBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(actionBox, GTK_ALIGN_CENTER);

        GtkWidget* btnDetail = UIComponents::createTableActionButton("🔍 Xem", "Xem chi tiết mặt hàng và số lượng nhập", "btn-action-edit");
        gtk_widget_set_size_request(btnDetail, 70, -1);

        struct RecCtx { WarehouseScreens* screens; GoodsReceiptSummaryViewModel item; };
        RecCtx* rCtx = new RecCtx{this, r};
        g_signal_connect_data(btnDetail, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            RecCtx* rc = static_cast<RecCtx*>(d);
            rc->screens->showReceiptDetailModal(rc->item);
        }), rCtx, [](gpointer d, GClosure*) { delete static_cast<RecCtx*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(actionBox), btnDetail);

        if (!isCancelled) {
            GtkWidget* btnCancel = UIComponents::createTableActionButton("❌ Hủy", "Hủy phiếu nhập kho này", "btn-action-delete");
            gtk_widget_set_size_request(btnCancel, 70, -1);
            RecCtx* cCtx = new RecCtx{this, r};
            g_signal_connect_data(btnCancel, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                RecCtx* rc = static_cast<RecCtx*>(d);
                rc->screens->confirmCancelReceipt(rc->item.id);
            }), cCtx, [](gpointer d, GClosure*) { delete static_cast<RecCtx*>(d); }, (GConnectFlags)0);
            gtk_box_append(GTK_BOX(actionBox), btnCancel);
        }

        gtk_box_append(GTK_BOX(row), actionBox);
        gtk_box_append(GTK_BOX(receiptsListBox), row);
    }
}

void WarehouseScreens::confirmCancelReceipt(const std::string& receiptId) {
    struct CancelCtx { WarehouseScreens* screens; std::string rId; };
    CancelCtx* ctx = new CancelCtx{this, receiptId};

    UIComponents::showConfirmDialog(
        shell->getWindow(),
        "Xác nhận hủy phiếu nhập",
        "Bạn có chắc chắn muốn hủy phiếu nhập kho [" + receiptId + "] không?\n\n"
        "Lưu ý: Tồn kho và giá vốn sẽ được tự động tính toán lại. "
        "Nếu làm tồn kho của sản phẩm bị âm, hệ thống sẽ từ chối thao tác.",
        +[](gpointer data) {
            CancelCtx* c = static_cast<CancelCtx*>(data);
            OperationResult res = c->screens->controller->cancelGoodsReceipt(c->rId);
            if (res.success) {
                UIComponents::showToast(c->screens->shell->getWindow(), res.message, "success");
                c->screens->refreshReceipts();
                c->screens->refreshInventory();
                c->screens->refreshDashboard();
            } else {
                UIComponents::showToast(c->screens->shell->getWindow(), res.message, "danger");
            }
            delete c;
        },
        ctx
    );
}

void WarehouseScreens::showReceiptDetailModal(const GoodsReceiptSummaryViewModel& receipt) {
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), ("Chi tiết phiếu nhập — " + receipt.id).c_str());
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 480, 420);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    GtkWidget* title = gtk_label_new(("PHIẾU NHẬP KHO " + receipt.id).c_str());
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

    addRow("Nhà cung cấp:", receipt.supplierName);
    addRow("Thủ kho nhập:", receipt.employeeName);
    addRow("Thời gian nhập:", receipt.importTime);
    addRow("Trạng thái phiếu:", receipt.status == "DaHuy" ? "Đã hủy" : "Hoạt động");

    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(card), sep);

    for (const auto& d : receipt.details) {
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

    addRow("Tổng tiền nhập:", UIComponents::formatVND(receipt.totalAmount));

    GtkWidget* btnClose = gtk_button_new_with_label("Đóng");
    gtk_widget_add_css_class(btnClose, "btn-secondary");
    UIComponents::setButtonCursor(btnClose);
    gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
    g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(card), btnClose);

    gtk_window_set_child(GTK_WINDOW(win), card);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 5. TẠO PHIẾU NHẬP KHO =================

void WarehouseScreens::buildCreateReceiptView() {
    createReceiptView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(createReceiptView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Lập phiếu nhập kho hàng", "Chọn nhà cung cấp, thêm các mã điện thoại, chỉnh số lượng và xác nhận nhập.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* formCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(formCard, "card");

    // Supplier selection
    std::vector<SupplierItemViewModel> sups = controller->getSuppliers();
    std::vector<const char*> supNames;
    for (const auto& s : sups) {
        supNames.push_back(s.companyName.c_str());
    }
    supNames.push_back(nullptr);

    receiptComboSupplier = gtk_drop_down_new_from_strings(supNames.data());
    gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField("Chọn nhà cung cấp *", receiptComboSupplier));

    // Action button: Add item
    GtkWidget* itemHeaderBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget* lblItemTitle = gtk_label_new("Danh sách sản phẩm nhập:");
    gtk_widget_add_css_class(lblItemTitle, "card-title");
    gtk_widget_set_hexpand(lblItemTitle, TRUE);
    gtk_label_set_xalign(GTK_LABEL(lblItemTitle), 0.0f);

    GtkWidget* btnAddItem = gtk_button_new_with_label("➕ Thêm sản phẩm vào phiếu");
    gtk_widget_add_css_class(btnAddItem, "btn-primary");
    UIComponents::setButtonCursor(btnAddItem);
    g_signal_connect_swapped(btnAddItem, "clicked", G_CALLBACK(+[](WarehouseScreens* s) {
        s->showAddProductToReceiptModal();
    }), this);

    gtk_box_append(GTK_BOX(itemHeaderBox), lblItemTitle);
    gtk_box_append(GTK_BOX(itemHeaderBox), btnAddItem);
    gtk_box_append(GTK_BOX(formCard), itemHeaderBox);

    // Draft items container
    receiptDraftItemsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_append(GTK_BOX(formCard), receiptDraftItemsBox);

    // Total estimate
    lblReceiptEstimatedTotal = gtk_label_new("Tổng tiền nhập dự kiến: 0 ₫");
    gtk_widget_add_css_class(lblReceiptEstimatedTotal, "product-price");
    gtk_label_set_xalign(GTK_LABEL(lblReceiptEstimatedTotal), 0.0f);
    gtk_box_append(GTK_BOX(formCard), lblReceiptEstimatedTotal);

    // Submit Receipt
    GtkWidget* btnSubmitReceipt = gtk_button_new_with_label("📥 Xác nhận lưu phiếu nhập & Tăng tồn kho");
    gtk_widget_add_css_class(btnSubmitReceipt, "btn-success");
    UIComponents::setButtonCursor(btnSubmitReceipt);
    gtk_widget_set_margin_top(btnSubmitReceipt, 10);

    g_signal_connect_data(btnSubmitReceipt, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        WarehouseScreens* self = static_cast<WarehouseScreens*>(d);
        if (self->draftReceiptItems.empty()) {
            UIComponents::showWarningDialog(self->shell->getWindow(), "Thiếu sản phẩm", "Vui lòng thêm ít nhất 1 sản phẩm vào phiếu nhập!");
            return;
        }

        // Chống bấm nhiều lần: vô hiệu hóa nút trong lúc xử lý
        gtk_widget_set_sensitive(GTK_WIDGET(b), FALSE);

        guint supSel = gtk_drop_down_get_selected(GTK_DROP_DOWN(self->receiptComboSupplier));
        std::vector<SupplierItemViewModel> sups;
        try {
            sups = self->controller->getSuppliers();
        } catch (const std::exception& ex) {
            UIComponents::showErrorDialog(self->shell->getWindow(), "Lỗi đọc file nhà cung cấp", ex.what());
            gtk_widget_set_sensitive(GTK_WIDGET(b), TRUE);
            return;
        }
        std::string supId = (supSel < sups.size()) ? sups[supSel].id : "SUP-000001";

        CreateReceiptRequest req;
        req.supplierId = supId;
        req.employeeId = self->controller->getCurrentUser().userId;
        req.items = self->draftReceiptItems;

        OperationResult res;
        try {
            res = self->controller->createGoodsReceipt(req);
        } catch (const std::exception& ex) {
            res = OperationResult::Fail(std::string("Lỗi đọc/ghi file: ") + ex.what());
        }

        // Bật lại nút sau khi xử lý xong
        gtk_widget_set_sensitive(GTK_WIDGET(b), TRUE);

        if (res.success) {
            self->draftReceiptItems.clear();
            // Tải lại dữ liệu tươi mới từ Service
            self->refreshReceipts();
            self->refreshInventory();
            self->refreshLowStock();
            self->refreshDashboard();

            // Xóa draft items trên UI
            GtkWidget* child = gtk_widget_get_first_child(self->receiptDraftItemsBox);
            while (child != nullptr) {
                GtkWidget* next = gtk_widget_get_next_sibling(child);
                gtk_box_remove(GTK_BOX(self->receiptDraftItemsBox), child);
                child = next;
            }
            gtk_label_set_text(GTK_LABEL(self->lblReceiptEstimatedTotal), "Tổng tiền nhập dự kiến: 0 ₫");

            UIComponents::showSuccessDialog(self->shell->getWindow(), "Thành công", res.message);
            self->shell->navigateTo("warehouse_receipts");
        } else {
            UIComponents::showErrorDialog(self->shell->getWindow(), "Lỗi tạo phiếu nhập", res.message);
        }
    }), this, nullptr, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(formCard), btnSubmitReceipt);
    gtk_box_append(GTK_BOX(content), formCard);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(createReceiptView), content);
}

void WarehouseScreens::showAddProductToReceiptModal() {
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), "Thêm sản phẩm vào phiếu nhập");
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 440, 320);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    std::vector<ProductItemViewModel> prods = controller->getProducts();
    std::vector<const char*> prodNames;
    for (const auto& p : prods) {
        prodNames.push_back(p.name.c_str());
    }
    prodNames.push_back(nullptr);

    GtkWidget* errCombo = nullptr;
    GtkWidget* comboProd = gtk_drop_down_new_from_strings(prodNames.data());
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Chọn điện thoại *", comboProd, &errCombo));

    GtkWidget* errQty = nullptr;
    GtkWidget* entryQty = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryQty), "10");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Số lượng nhập *", entryQty, &errQty));

    GtkWidget* errPrice = nullptr;
    GtkWidget* entryPrice = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryPrice), "10000000");
    UIComponents::setupCurrencyInput(entryPrice);
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Giá nhập từng máy (VNĐ) *", entryPrice, &errPrice));

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);

    GtkWidget* btnCancel = gtk_button_new_with_label("Hủy");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(btnRow), btnCancel);

    struct AddProdReceiptCtx {
        WarehouseScreens* screens;
        GtkWidget* win;
        GtkWidget* combo;
        GtkWidget* eQty;
        GtkWidget* ePrice;
        GtkWidget* errCombo;
        GtkWidget* errQty;
        GtkWidget* errPrice;
        std::vector<ProductItemViewModel> prods;
    };
    AddProdReceiptCtx* ctx = new AddProdReceiptCtx{
        this, win, comboProd, entryQty, entryPrice,
        errCombo, errQty, errPrice, prods
    };

    GtkWidget* btnAdd = gtk_button_new_with_label("Thêm vào phiếu");
    gtk_widget_add_css_class(btnAdd, "btn-primary");
    UIComponents::setButtonCursor(btnAdd);
    g_signal_connect_data(btnAdd, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        AddProdReceiptCtx* c = static_cast<AddProdReceiptCtx*>(d);
        UIComponents::clearFieldError(c->errCombo, c->combo);
        UIComponents::clearFieldError(c->errQty, c->eQty);
        UIComponents::clearFieldError(c->errPrice, c->ePrice);

        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->combo));
        bool hasError = false;
        GtkWidget* firstErr = nullptr;

        if (sel >= c->prods.size()) {
            UIComponents::setFieldError(c->errCombo, c->combo, "Vui lòng chọn một dòng điện thoại");
            hasError = true;
            if (!firstErr) firstErr = c->combo;
        }

        int qty = 0;
        try { qty = std::stoi(gtk_editable_get_text(GTK_EDITABLE(c->eQty))); } catch (...) { qty = 0; }
        if (qty <= 0) {
            UIComponents::setFieldError(c->errQty, c->eQty, "Số lượng nhập phải lớn hơn 0");
            hasError = true;
            if (!firstErr) firstErr = c->eQty;
        }

        double price = UIComponents::parseCurrencyInput(c->ePrice);
        if (price <= 0) {
            UIComponents::setFieldError(c->errPrice, c->ePrice, "Giá nhập từng máy phải lớn hơn 0");
            hasError = true;
            if (!firstErr) firstErr = c->ePrice;
        }

        if (hasError) {
            if (firstErr) gtk_widget_grab_focus(firstErr);
            return;
        }

        GoodsReceiptDetailViewModel item;
        item.phoneId = c->prods[sel].id;
        item.phoneName = c->prods[sel].name;
        item.quantity = qty;
        item.importPrice = price;
        item.subtotal = qty * price;
        c->screens->draftReceiptItems.push_back(item);

        // Cập nhật giao diện draft
        size_t idx = c->screens->draftReceiptItems.size() - 1;
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (idx % 2 == 0) ? "row-even" : "row-odd");

        GtkWidget* nLbl = gtk_label_new(item.phoneName.c_str());
        gtk_widget_set_hexpand(nLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(nLbl), 0.0f);
        gtk_box_append(GTK_BOX(row), nLbl);

        GtkWidget* qLbl = gtk_label_new(("x" + std::to_string(item.quantity)).c_str());
        gtk_box_append(GTK_BOX(row), qLbl);

        GtkWidget* pLbl = gtk_label_new(UIComponents::formatVND(item.subtotal).c_str());
        gtk_widget_add_css_class(pLbl, "product-price");
        gtk_label_set_xalign(GTK_LABEL(pLbl), 1.0f);
        gtk_box_append(GTK_BOX(row), pLbl);

        gtk_box_append(GTK_BOX(c->screens->receiptDraftItemsBox), row);

        double total = 0.0;
        for (const auto& it : c->screens->draftReceiptItems) total += it.subtotal;
        gtk_label_set_text(GTK_LABEL(c->screens->lblReceiptEstimatedTotal),
                           ("Tổng tiền nhập dự kiến: " + UIComponents::formatVND(total)).c_str());

        gtk_window_destroy(GTK_WINDOW(c->win));
    }), ctx, [](gpointer d, GClosure*) { delete static_cast<AddProdReceiptCtx*>(d); }, (GConnectFlags)0);

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
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(fulfillmentView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Chuẩn bị đơn hàng (Fulfillment)", "Lấy hàng, đóng gói và chuyển giao cho vận chuyển hoặc để tại quầy nhận.");
    gtk_box_append(GTK_BOX(content), header);

    std::vector<UIComponents::TableColumnSpec> fulCols = {
        {"Mã đơn", 120, 0.0f, false},
        {"Thông tin người nhận", 180, 0.0f, false},
        {"Danh sách máy cần lấy", 0, 0.0f, true},
        {"Trạng thái", 130, 0.5f, false},
        {"Giao nhận", 130, 0.5f, false},
        {"Thao tác xử lý", 180, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(fulCols));

    fulfillmentListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), fulfillmentListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fulfillmentView), content);
    refreshFulfillmentQueue();
}

void WarehouseScreens::refreshFulfillmentQueue() {
    GtkWidget* child = gtk_widget_get_first_child(fulfillmentListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(fulfillmentListBox), child);
        child = next;
    }

    std::vector<OrderItemViewModel> orders = controller->getAllOrders();
    std::vector<OrderItemViewModel> toFulfill;
    for (const auto& ord : orders) {
        if (ord.status == OrderStatus::CONFIRMED || ord.status == OrderStatus::PREPARING) {
            toFulfill.push_back(ord);
        }
    }

    if (toFulfill.empty()) {
        gtk_box_append(GTK_BOX(fulfillmentListBox), UIComponents::createEmptyState("📦", "Chưa có dữ liệu", "Không có đơn hàng nào cần đóng gói hoặc xuất kho."));
        return;
    }

    for (size_t i = 0; i < toFulfill.size(); ++i) {
        const auto& ord = toFulfill[i];
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

        // Thông tin người nhận
        GtkWidget* recBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_size_request(recBox, 180, -1);
        gtk_widget_set_valign(recBox, GTK_ALIGN_CENTER);
        GtkWidget* rName = gtk_label_new(ord.receiverName.c_str());
        gtk_label_set_xalign(GTK_LABEL(rName), 0.0f);
        GtkWidget* rPhone = gtk_label_new(("SĐT: " + ord.receiverPhone).c_str());
        gtk_widget_add_css_class(rPhone, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(rPhone), 0.0f);
        gtk_box_append(GTK_BOX(recBox), rName);
        gtk_box_append(GTK_BOX(recBox), rPhone);
        gtk_box_append(GTK_BOX(row), recBox);

        // Danh sách máy cần lấy
        std::string pickList = "";
        for (size_t j = 0; j < ord.details.size(); ++j) {
            if (j > 0) pickList += ", ";
            pickList += ord.details[j].phoneName + " [x" + std::to_string(ord.details[j].quantity) + "]";
        }
        GtkWidget* pickLbl = gtk_label_new(pickList.c_str());
        gtk_widget_set_hexpand(pickLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(pickLbl), 0.0f);
        gtk_widget_set_valign(pickLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), pickLbl);

        // Trạng thái badge
        GtkWidget* stBadge = UIComponents::createOrderStatusBadge(ord.status);
        GtkWidget* stBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(stBox, 130, -1);
        gtk_widget_set_valign(stBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(stBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(stBox), stBadge);
        gtk_box_append(GTK_BOX(row), stBox);

        // Giao nhận badge
        GtkWidget* delBadge = UIComponents::createStatusBadge(ord.deliveryMethodText, "badge-confirmed");
        GtkWidget* delBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(delBox, 130, -1);
        gtk_widget_set_valign(delBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(delBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(delBox), delBadge);
        gtk_box_append(GTK_BOX(row), delBox);

        // Actions
        GtkWidget* actionBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_widget_set_size_request(actionBox, 180, -1);
        gtk_widget_set_valign(actionBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(actionBox, GTK_ALIGN_CENTER);

        struct FulfillCtx { WarehouseScreens* screens; std::string orderId; OrderStatus nextStatus; };

        if (ord.status == OrderStatus::CONFIRMED) {
            GtkWidget* btnPack = UIComponents::createTableActionButton("📦 Đang đóng gói", "Bắt đầu lấy máy và đóng gói đơn hàng", "btn-primary");
            FulfillCtx* fc = new FulfillCtx{this, ord.id, OrderStatus::PREPARING};
            g_signal_connect_data(btnPack, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                FulfillCtx* c = static_cast<FulfillCtx*>(d);
                c->screens->controller->updateOrderStatus(c->orderId, c->nextStatus);
                c->screens->refreshFulfillmentQueue();
            }), fc, [](gpointer d, GClosure*) { delete static_cast<FulfillCtx*>(d); }, (GConnectFlags)0);
            gtk_box_append(GTK_BOX(actionBox), btnPack);
        } else if (ord.status == OrderStatus::PREPARING) {
            if (ord.deliveryMethod == DeliveryMethod::PICKUP) {
                GtkWidget* btnReady = UIComponents::createTableActionButton("✅ Sẵn sàng nhận", "Hàng đã sẵn sàng tại quầy để khách tới lấy", "btn-primary");
                FulfillCtx* fc = new FulfillCtx{this, ord.id, OrderStatus::READY_FOR_PICKUP};
                g_signal_connect_data(btnReady, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                    (void)b;
                    FulfillCtx* c = static_cast<FulfillCtx*>(d);
                    c->screens->controller->updateOrderStatus(c->orderId, c->nextStatus);
                    c->screens->refreshFulfillmentQueue();
                }), fc, [](gpointer d, GClosure*) { delete static_cast<FulfillCtx*>(d); }, (GConnectFlags)0);
                gtk_box_append(GTK_BOX(actionBox), btnReady);
            } else {
                GtkWidget* btnShip = UIComponents::createTableActionButton("🚚 Bàn giao ship", "Bàn giao kiện hàng cho đối tác vận chuyển", "btn-primary");
                FulfillCtx* fc = new FulfillCtx{this, ord.id, OrderStatus::SHIPPING};
                g_signal_connect_data(btnShip, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                    (void)b;
                    FulfillCtx* c = static_cast<FulfillCtx*>(d);
                    c->screens->controller->updateOrderStatus(c->orderId, c->nextStatus);
                    c->screens->refreshFulfillmentQueue();
                }), fc, [](gpointer d, GClosure*) { delete static_cast<FulfillCtx*>(d); }, (GConnectFlags)0);
                gtk_box_append(GTK_BOX(actionBox), btnShip);
            }
        }

        gtk_box_append(GTK_BOX(row), actionBox);
        gtk_box_append(GTK_BOX(fulfillmentListBox), row);
    }
}
