/**
 * @file AdminScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ QUẢN TRỊ VIÊN CẤP CAO (ADMINISTRATOR SUBSYSTEM)
 * 
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - Subview Swap Pattern: Chuyển đổi giữa danh sách và màn hình chi tiết/chỉnh sửa toàn phần
 *   (showProductDetailView, showCustomerDetailView, showDiscountEditView, showOrderDetailView)
 *   tích hợp nút Back Header để quay lại danh sách mượt mà.
 * - Quản lý Sản phẩm: Tìm kiếm, chỉnh sửa, thêm mới có hỗ trợ đường dẫn hình ảnh (image url).
 * - Quản lý Khách hàng: Huy hiệu màu Rank phân biệt theo từng hạng (Bronze, Silver, Gold, Diamond).
 * - Quản lý Voucher: Chỉnh sửa thời gian hiệu lực (startDate, endDate) và đối soát voucher.
 * - Quản lý Đơn hàng: Lọc theo từ khóa, trạng thái xử lý và trạng thái thanh toán.
 * - Thống kê & Báo cáo: Biểu đồ chỉ số, lợi nhuận, sản phẩm bán chạy, tồn kho thấp.
 * - Giả lập Thời gian thực: Màn hình kiểm soát tham số Real-time Clock cho toàn bộ hệ thống.
 * =========================================================================================
 */

#include "AdminScreens.h"
#include "MainShellWindow.h"
#include "../components/UIComponents.h"
#include "../styles/AppCss.h"
#include "../mock/RealTimeManager.h"
#include "../../source/utils/Utf8Helper.h"
#include <iostream>

namespace {
const std::vector<UIComponents::TableColumnSpec> kProductTableCols = {
    {"Ảnh", 60, 0.5f, false},
    {"Tên & Hãng sản phẩm", 240, 0.0f, true},
    {"Tồn kho", 100, 1.0f, false},
    {"Giá vốn", 130, 1.0f, false},
    {"Giá bán", 130, 1.0f, false},
    {"Trạng thái", 130, 0.5f, false},
    {"Thao tác", 110, 0.5f, false}
};

const std::vector<UIComponents::TableColumnSpec> kEmployeeTableCols = {
    {"Mã & Họ tên", 220, 0.0f, true},
    {"Tài khoản", 130, 0.0f, false},
    {"Số điện thoại", 125, 0.0f, false},
    {"Chức vụ", 135, 0.5f, false},
    {"Mức lương", 140, 1.0f, false},
    {"Thao tác", 105, 0.5f, false}
};

const std::vector<UIComponents::TableColumnSpec> kCustomerTableCols = {
    {"Mã & Họ tên", 220, 0.0f, true},
    {"Số điện thoại", 130, 0.0f, false},
    {"Địa chỉ", 200, 0.0f, true},
    {"Điểm tích lũy", 120, 1.0f, false},
    {"Hạng thành viên", 145, 0.5f, false},
    {"Thao tác", 110, 0.5f, false}
};

const std::vector<UIComponents::TableColumnSpec> kSupplierTableCols = {
    {"Công ty cung cấp", 240, 0.0f, true},
    {"Mã số thuế", 130, 0.0f, false},
    {"Người liên hệ", 155, 0.0f, false},
    {"Số điện thoại", 125, 0.0f, false},
    {"Địa chỉ", 210, 0.0f, true},
    {"Thao tác", 105, 0.5f, false}
};

const std::vector<UIComponents::TableColumnSpec> kDiscountTableCols = {
    {"Mã Voucher", 120, 0.0f, false},
    {"Mô tả chương trình", 210, 0.0f, true},
    {"Mức giảm", 110, 1.0f, false},
    {"Đơn tối thiểu", 130, 1.0f, false},
    {"Hạn dùng", 110, 0.5f, false},
    {"Đã dùng", 100, 1.0f, false},
    {"Trạng thái", 160, 0.5f, false},
    {"Thao tác", 110, 0.5f, false}
};

const std::vector<UIComponents::TableColumnSpec> kAccountTableCols = {
    {"Tên đăng nhập", 160, 0.0f, false},
    {"Họ và tên", 220, 0.0f, true},
    {"Mã liên kết", 140, 0.0f, false},
    {"Vai trò", 145, 0.5f, false},
    {"Trạng thái", 150, 0.5f, false},
    {"Thao tác", 115, 0.5f, false}
};

const std::vector<UIComponents::TableColumnSpec> kAdminOrderTableCols = {
    {"Mã đơn", 115, 0.0f, false},
    {"Khách hàng / SĐT", 180, 0.0f, true},
    {"Thời gian", 135, 0.0f, false},
    {"Giao nhận", 115, 0.0f, false},
    {"Trạng thái đơn", 120, 0.5f, false},
    {"Thanh toán", 120, 0.5f, false},
    {"Tổng tiền", 135, 1.0f, false},
    {"Thao tác", 100, 0.5f, false}
};

const std::vector<UIComponents::TableColumnSpec> kOrderDetailItemsCols = {
    {"Tên mặt hàng", 0, 0.0f, true},
    {"Số lượng", 110, 1.0f, false},
    {"Đơn giá", 140, 1.0f, false},
    {"Thành tiền", 150, 1.0f, false}
};

const std::vector<UIComponents::TableColumnSpec> kAdminInvoiceTableCols = {
    {"Mã hóa đơn", 130, 0.0f, false},
    {"Mã đơn hàng", 130, 0.0f, false},
    {"Khách hàng", 170, 0.0f, false},
    {"Thu ngân", 150, 0.0f, false},
    {"Phương thức", 140, 0.5f, false},
    {"Thời gian", 0, 0.0f, true},
    {"Tổng tiền", 150, 1.0f, false}
};

const std::vector<UIComponents::TableColumnSpec> kAdminWarrantyTableCols = {
    {"Mã phiếu", 120, 0.0f, false},
    {"Thiết bị", 185, 0.0f, false},
    {"Khách hàng", 175, 0.0f, false},
    {"Ngày hẹn", 130, 0.0f, false},
    {"Trạng thái", 145, 0.5f, false},
    {"Mô tả sự cố", 0, 0.0f, true}
};

const std::vector<UIComponents::TableColumnSpec> kReportTopProdCols = {
    {"Tên sản phẩm", 0, 0.0f, true},
    {"Hãng", 110, 0.0f, false},
    {"Đã bán", 100, 1.0f, false},
    {"Doanh thu", 150, 1.0f, false},
    {"Lợi nhuận", 150, 1.0f, false}
};
} // namespace

struct AdminNavData { MainShellWindow* sh; const char* tgt; };

AdminScreens::AdminScreens(AppController* ctrl, MainShellWindow* parentShell)
    : controller(ctrl), shell(parentShell),
      productContainerBox(nullptr), customerContainerBox(nullptr),
      discountContainerBox(nullptr), orderContainerBox(nullptr),
      productListBox(nullptr), productCountBadge(nullptr), employeeListBox(nullptr), customerListBox(nullptr),
      supplierListBox(nullptr), discountListBox(nullptr), accountListBox(nullptr),
      ordersListBox(nullptr), invoiceListBox(nullptr), warrantyListBox(nullptr),
      reportsListBox(nullptr), realTimeReconcileBox(nullptr),
      orderFilterKeyword(""), orderFilterStatus(-1), orderFilterPayment(-1) {

    // Khởi tạo cố định các ScrolledWindow gắn vào GtkStack của Shell
    dashboardView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    productMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    employeeMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(employeeMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    customerMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(customerMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    supplierMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(supplierMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    discountMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(discountMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    accountMgmtView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(accountMgmtView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    allOrdersView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(allOrdersView), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    allInvoicesView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(allInvoicesView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    warrantyMonitorView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyMonitorView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    reportsView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(reportsView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    realTimeView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(realTimeView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

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
}

void AdminScreens::refreshAll() {
    refreshDashboard();
    refreshProducts();
    refreshEmployees();
    refreshCustomers();
    refreshSuppliers();
    refreshDiscounts();
    refreshAccounts();
    refreshOrders();
    refreshInvoices();
    refreshWarrantyMonitor();
    refreshReports();
    refreshRealTime();
    if (shell) {
        shell->updateSimulatedTimeBadge();
    }
}

// ================= 1. ADMIN DASHBOARD =================

void AdminScreens::buildDashboardView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
    gtk_widget_add_css_class(content, "page-container");

    const UserSessionViewModel& user = controller->getCurrentUser();
    std::string title = "Trung tâm Quản trị Hệ thống (Admin) — " + user.fullName;
    GtkWidget* header = UIComponents::createSectionHeader(title, "Tổng hợp chỉ số kinh doanh, quản lý dữ liệu toàn bộ cửa hàng PBL2 Phone Store.");
    gtk_box_append(GTK_BOX(content), header);

    // KPI Cards Grid (Primary Financial & Operational KPIs)
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    DashboardStatsViewModel stats = controller->getDashboardStats();

    GtkWidget* c1 = UIComponents::createStatCard(
        "Doanh thu thực tế",
        UIComponents::formatVND(stats.totalRevenue),
        "Hoàn tất: " + UIComponents::formatNumber(stats.completedOrders) + " / " + UIComponents::formatNumber(stats.totalOrders) + " đơn",
        "💰",
        "green");
    GtkWidget* c2 = UIComponents::createStatCard(
        "Lợi nhuận gộp ước tính",
        UIComponents::formatVND(stats.estimatedProfit),
        "Doanh thu trừ giá vốn hàng bán",
        "📈",
        "green");
    GtkWidget* c3 = UIComponents::createStatCard(
        "Tổng số đơn hàng",
        UIComponents::formatNumber(stats.totalOrders) + " đơn",
        "Chờ xử lý: " + std::to_string(stats.pendingOrders) + " • Đang giao: " + std::to_string(stats.shippingOrders),
        "📦",
        stats.pendingOrders > 0 ? "amber" : "blue");
    GtkWidget* c4 = UIComponents::createStatCard(
        "Giá trị tồn kho",
        UIComponents::formatVND(stats.totalInventoryValue),
        "Tổng số máy: " + UIComponents::formatNumber(stats.totalStockQuantity) + " máy",
        "🏢",
        "purple");

    gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c4, 3, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), grid);

    // Secondary row of metrics (Catalog, CRM, HR & System Clock)
    GtkWidget* grid2 = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid2), DesignTokens::KPI_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(grid2), DesignTokens::KPI_GRID_SPACING);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid2), TRUE);

    GtkWidget* c5 = UIComponents::createStatCard(
        "Số mặt hàng điện thoại",
        UIComponents::formatNumber(stats.totalProducts) + " mẫu",
        stats.lowStockCount > 0 ? ("Cảnh báo sắp hết: " + std::to_string(stats.lowStockCount) + " mẫu") : "Tồn kho ổn định",
        "📱",
        stats.lowStockCount > 0 ? "red" : "blue");
    GtkWidget* c6 = UIComponents::createStatCard(
        "Khách hàng đăng ký",
        UIComponents::formatNumber(stats.totalCustomers) + " khách",
        "Đã tích điểm thành viên",
        "👥",
        "blue");
    GtkWidget* c7 = UIComponents::createStatCard(
        "Đội ngũ nhân sự",
        UIComponents::formatNumber(stats.totalEmployees) + " nhân sự",
        "Quản trị, Bán hàng & Kho",
        "💼",
        "cyan");
    GtkWidget* c8 = UIComponents::createStatCard(
        "Thời gian thực hệ thống",
        controller->getSimulatedDate(),
        "Mốc thời gian mô phỏng",
        "⏱️",
        "blue");

    gtk_grid_attach(GTK_GRID(grid2), c5, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), c6, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), c7, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), c8, 3, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), grid2);

    // Quick Management Shortcuts
    GtkWidget* cardShortcuts = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
    gtk_widget_add_css_class(cardShortcuts, "card");

    GtkWidget* lblS = gtk_label_new("🚀 Điều hướng quản trị nhanh");
    gtk_widget_add_css_class(lblS, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblS), 0.0f);
    gtk_box_append(GTK_BOX(cardShortcuts), lblS);

    GtkWidget* rowBtns = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(rowBtns), DesignTokens::SPACE_MD);
    gtk_grid_set_column_spacing(GTK_GRID(rowBtns), DesignTokens::SPACE_MD);
    gtk_grid_set_column_homogeneous(GTK_GRID(rowBtns), TRUE);

    int navIdx = 0;
    auto addNavBtn = [this, rowBtns, &navIdx](const char* label, const char* target) {
        GtkWidget* btn = gtk_button_new_with_label(label);
        gtk_widget_add_css_class(btn, "btn-outline");
        UIComponents::setButtonCursor(btn);
        AdminNavData* nd = new AdminNavData{shell, target};
        g_signal_connect_data(btn, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AdminNavData* data = static_cast<AdminNavData*>(d);
            data->sh->navigateTo(data->tgt);
        }), nd, [](gpointer d, GClosure*) { delete static_cast<AdminNavData*>(d); }, (GConnectFlags)0);
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

void AdminScreens::refreshDashboard() {
    buildDashboardView();
}

// ================= 2. QUẢN LÝ SẢN PHẨM =================

void AdminScreens::buildProductMgmtView() {
    productContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(productMgmtView), productContainerBox);
    showProductListView();
}

void AdminScreens::showProductListView() {
    productListBox = nullptr;
    productCountBadge = nullptr;

    // Clear container
    GtkWidget* child = gtk_widget_get_first_child(productContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(productContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* btnAdd = gtk_button_new_with_label("➕ Thêm sản phẩm mới");
    gtk_widget_add_css_class(btnAdd, "btn-primary");
    UIComponents::setButtonCursor(btnAdd);
    g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        ProductItemViewModel newProd;
        newProd.id = "PHN-NEW";
        s->showProductDetailView(newProd, true);
    }), this);

    GtkWidget* header = UIComponents::createSectionHeader("Quản lý danh mục sản phẩm", "Thêm, xem chi tiết, sửa thông tin, giá vốn, giá bán và import ảnh điện thoại.", btnAdd);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo tên, hãng hoặc mã sản phẩm...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->refreshProducts(txt ? txt : "");
        }),
        this,
        nullptr,
        &productCountBadge);
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kProductTableCols));

    productListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::TABLE_ROW_SPACING);
    gtk_box_append(GTK_BOX(tableContainer), productListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_box_append(GTK_BOX(productContainerBox), content);
    refreshProducts();
}

void AdminScreens::refreshProducts(const std::string& keyword) {
    if (!productListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(productListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(productListBox), child);
        child = next;
    }

    ProductFilterRequest filter;
    filter.keyword = keyword;
    std::vector<ProductItemViewModel> products = controller->getProducts(filter);

    UIComponents::updateRecordCountBadge(productCountBadge, products.size(), "sản phẩm");

    if (products.empty()) {
        std::string emptyMsg = keyword.empty() ? "Không tìm thấy sản phẩm nào trong hệ thống." : "Không tìm thấy sản phẩm nào khớp với từ khóa tìm kiếm.";
        GtkWidget* emptyWidget = UIComponents::createEmptyState(
            "📱", "Chưa có sản phẩm", emptyMsg,
            keyword.empty() ? "➕ Thêm sản phẩm đầu tiên" : "",
            G_CALLBACK(+[](GtkButton*, gpointer d) {
                AdminScreens* s = static_cast<AdminScreens*>(d);
                ProductItemViewModel newProd;
                newProd.id = "PHN-NEW";
                s->showProductDetailView(newProd, true);
            }), this);
        gtk_box_append(GTK_BOX(productListBox), emptyWidget);
        return;
    }

    for (size_t i = 0; i < products.size(); ++i) {
        const auto& p = products[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // Col 0: Ảnh (Thumbnail preview)
        GtkWidget* imgThumb = nullptr;
        if (!p.imageUrl.empty() && g_file_test(p.imageUrl.c_str(), G_FILE_TEST_EXISTS)) {
            imgThumb = gtk_image_new_from_file(p.imageUrl.c_str());
            gtk_image_set_pixel_size(GTK_IMAGE(imgThumb), 48);
        } else {
            imgThumb = gtk_label_new("📱");
            gtk_widget_set_size_request(imgThumb, 48, 48);
        }
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(imgThumb, kProductTableCols[0]));

        // Col 1: Tên & Hãng sản phẩm
        std::string sub = "[" + p.brand + "] Mã: " + p.id + (p.os.empty() ? "" : (" • " + p.os));
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(p.name, sub, kProductTableCols[1]));

        // Col 2: Tồn kho (Căn phải, màu ngữ nghĩa theo trạng thái tồn kho)
        std::string stockCssClass = "stock-ok";
        if (p.status == StockDisplayStatus::OUT_OF_STOCK) {
            stockCssClass = "stock-critical";
        } else if (p.status == StockDisplayStatus::LOW_STOCK) {
            stockCssClass = "stock-low";
        }
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatNumber(p.actualStock) + " máy", kProductTableCols[2], stockCssClass));

        // Col 3: Giá vốn (Căn phải, tabular numerals)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(p.importPrice), kProductTableCols[3], "cell-number"));

        // Col 4: Giá bán (Căn phải, màu nổi bật, tabular numerals)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(p.sellingPrice), kProductTableCols[4], "product-price"));

        // Col 5: Trạng thái kho (Huy hiệu màu)
        GtkWidget* badgeStock = UIComponents::createStockStatusBadge(p.status);
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(badgeStock, kProductTableCols[5]));

        // Col 6: Thao tác: Nút nhỏ gọn kèm tooltip
        GtkWidget* btnDetail = UIComponents::createTableActionButton("🔍 Sửa", "Xem chi tiết và chỉnh sửa thông tin sản phẩm", "btn-action-edit");
        gtk_widget_set_hexpand(btnDetail, TRUE);

        struct ProdClickData { AdminScreens* s; ProductItemViewModel p; };
        ProdClickData* pcd = new ProdClickData{this, p};
        g_signal_connect_data(btnDetail, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            ProdClickData* data = static_cast<ProdClickData*>(d);
            data->s->showProductDetailView(data->p, false);
        }), pcd, [](gpointer d, GClosure*) { delete static_cast<ProdClickData*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(btnDetail, kProductTableCols[6]));
        gtk_box_append(GTK_BOX(productListBox), row);
    }
}

void AdminScreens::showProductDetailView(const ProductItemViewModel& prod, bool isNew) {
    productListBox = nullptr;
    productCountBadge = nullptr;

    // Xóa nội dung danh sách, chuyển sang Màn hình Chi tiết & Chỉnh sửa Sản phẩm riêng biệt
    GtkWidget* child = gtk_widget_get_first_child(productContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(productContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_add_css_class(content, "page-container");

    std::string pageTitle = isNew ? "Thêm mới sản phẩm điện thoại" : ("Chi tiết & Chỉnh sửa: " + prod.name);
    GtkWidget* header = UIComponents::createBackHeader(pageTitle, "Tích hợp đầy đủ cấu hình thông số kỹ thuật, giá cả và import ảnh sản phẩm.",
        G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AdminScreens* self = static_cast<AdminScreens*>(d);
            self->showProductListView();
        }), this);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(card, "card");

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    // Row 1: Tên sản phẩm & Hãng
    GtkWidget* eName = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eName), isNew ? "" : prod.name.c_str());
    gtk_widget_set_hexpand(eName, TRUE);
    GtkWidget* errName = nullptr;
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Tên sản phẩm *", eName, &errName), 0, 0, 2, 1);

    GtkWidget* eBrand = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eBrand), isNew ? "Apple" : prod.brand.c_str());
    gtk_widget_set_hexpand(eBrand, TRUE);
    GtkWidget* errBrand = nullptr;
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Hãng sản xuất *", eBrand, &errBrand), 2, 0, 1, 1);

    // Row 2: Hệ điều hành & Nhà sản xuất
    GtkWidget* eOs = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eOs), isNew ? "iOS" : prod.os.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Hệ điều hành", eOs), 0, 1, 1, 1);

    GtkWidget* eManu = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eManu), isNew ? "Apple Inc." : prod.manufacturer.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Nhà sản xuất", eManu), 1, 1, 2, 1);

    // Row 3: Giá nhập, Giá bán, Tồn kho
    GtkWidget* eImport = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eImport), isNew ? "10000000" : std::to_string(static_cast<long long>(prod.importPrice)).c_str());
    UIComponents::setupCurrencyInput(eImport);
    GtkWidget* errImport = nullptr;
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Giá nhập (VND) *", eImport, &errImport), 0, 2, 1, 1);

    GtkWidget* eSelling = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eSelling), isNew ? "12000000" : std::to_string(static_cast<long long>(prod.sellingPrice)).c_str());
    UIComponents::setupCurrencyInput(eSelling);
    GtkWidget* errSelling = nullptr;
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Giá niêm yết bán (VND) *", eSelling, &errSelling), 1, 2, 1, 1);

    GtkWidget* eStock = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eStock), isNew ? "20" : std::to_string(prod.actualStock).c_str());
    GtkWidget* errStock = nullptr;
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Số lượng tồn kho *", eStock, &errStock), 2, 2, 1, 1);

    // Row 4: Thông số kỹ thuật
    GtkWidget* eSpecs = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eSpecs), isNew ? "Màn hình 6.7 inch, 8GB RAM, 256GB ROM, Chip cao cấp" : prod.specs.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Thông số kỹ thuật / Cấu hình máy", eSpecs), 0, 3, 3, 1);

    // Row 5: Import ảnh sản phẩm (File Chooser & Path)
    GtkWidget* imgBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget* eImgPath = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eImgPath), isNew ? "" : prod.imageUrl.c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(eImgPath), "Đường dẫn file ảnh sản phẩm (.png, .jpg)...");
    gtk_widget_set_hexpand(eImgPath, TRUE);

    GtkWidget* previewImg = nullptr;
    if (!prod.imageUrl.empty() && g_file_test(prod.imageUrl.c_str(), G_FILE_TEST_EXISTS)) {
        previewImg = gtk_image_new_from_file(prod.imageUrl.c_str());
        gtk_image_set_pixel_size(GTK_IMAGE(previewImg), 64);
    } else {
        previewImg = gtk_image_new_from_icon_name("image-x-generic");
        gtk_image_set_pixel_size(GTK_IMAGE(previewImg), 64);
    }

    GtkWidget* btnChooseImg = gtk_button_new_with_label("🖼️ Chọn ảnh từ máy...");
    gtk_widget_add_css_class(btnChooseImg, "btn-outline");
    UIComponents::setButtonCursor(btnChooseImg);

    struct FileCtx { GtkWidget* entry; GtkWidget* preview; MainShellWindow* shell; };
    FileCtx* fctx = new FileCtx{eImgPath, previewImg, shell};
    g_signal_connect_data(btnChooseImg, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        FileCtx* fc = static_cast<FileCtx*>(d);
        GtkFileChooserNative* native = gtk_file_chooser_native_new(
            "Chọn file ảnh sản phẩm",
            fc->shell ? fc->shell->getWindow() : nullptr,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            "Chọn file",
            "Hủy"
        );
        GtkFileFilter* filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Hình ảnh (*.png, *.jpg, *.jpeg, *.webp)");
        gtk_file_filter_add_mime_type(filter, "image/png");
        gtk_file_filter_add_mime_type(filter, "image/jpeg");
        gtk_file_filter_add_mime_type(filter, "image/webp");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

        g_signal_connect_data(native, "response", G_CALLBACK(+[](GtkNativeDialog* dialog, gint resId, gpointer data2) {
            if (resId == GTK_RESPONSE_ACCEPT) {
                GFile* f = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
                if (f) {
                    char* path = g_file_get_path(f);
                    if (path) {
                        FileCtx* c2 = static_cast<FileCtx*>(data2);
                        gtk_editable_set_text(GTK_EDITABLE(c2->entry), path);
                        gtk_image_set_from_file(GTK_IMAGE(c2->preview), path);
                        gtk_image_set_pixel_size(GTK_IMAGE(c2->preview), 64);
                        g_free(path);
                    }
                    g_object_unref(f);
                }
            }
            g_object_unref(dialog);
        }), fc, nullptr, (GConnectFlags)0);

        gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
    }), fctx, [](gpointer d, GClosure*) { delete static_cast<FileCtx*>(d); }, (GConnectFlags)0);

    g_signal_connect(eImgPath, "changed", G_CALLBACK(+[](GtkEditable* e, gpointer d) {
        GtkWidget* preview = static_cast<GtkWidget*>(d);
        const char* p = gtk_editable_get_text(e);
        if (p && strlen(p) > 0 && g_file_test(p, G_FILE_TEST_EXISTS)) {
            gtk_image_set_from_file(GTK_IMAGE(preview), p);
            gtk_image_set_pixel_size(GTK_IMAGE(preview), 64);
        }
    }), previewImg);

    gtk_box_append(GTK_BOX(imgBox), eImgPath);
    gtk_box_append(GTK_BOX(imgBox), btnChooseImg);
    gtk_box_append(GTK_BOX(imgBox), previewImg);

    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Ảnh đại diện sản phẩm (Import file)", imgBox), 0, 4, 3, 1);

    gtk_box_append(GTK_BOX(card), grid);

    // Action buttons bar
    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
    gtk_widget_set_margin_top(btnRow, 14);

    GtkWidget* btnCancel = gtk_button_new_with_label("← Hủy / Quay lại");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        s->showProductListView();
    }), this);
    gtk_box_append(GTK_BOX(btnRow), btnCancel);

    if (!isNew) {
        GtkWidget* btnDelete = gtk_button_new_with_label("🗑️ Xóa sản phẩm");
        gtk_widget_add_css_class(btnDelete, "btn-danger");
        UIComponents::setButtonCursor(btnDelete);
        struct DelProdCtx { AdminScreens* s; std::string id; };
        DelProdCtx* dctx = new DelProdCtx{this, prod.id};
        g_signal_connect_data(btnDelete, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            DelProdCtx* c = static_cast<DelProdCtx*>(d);
            UIComponents::showConfirmDialog(
                c->s->shell ? c->s->shell->getWindow() : nullptr,
                "Xác nhận xóa sản phẩm",
                "Bạn có chắc chắn muốn xóa sản phẩm này khỏi hệ thống không? Thao tác này không thể hoàn tác.",
                +[](gpointer data) {
                    DelProdCtx* inner = static_cast<DelProdCtx*>(data);
                    OperationResult res = inner->s->controller->deleteProduct(inner->id);
                    if (res.success) {
                        UIComponents::showSuccessDialog(inner->s->shell ? inner->s->shell->getWindow() : nullptr, "Đã xóa", res.message);
                        inner->s->showProductListView();
                    } else {
                        UIComponents::showErrorDialog(inner->s->shell ? inner->s->shell->getWindow() : nullptr, "Lỗi", res.message);
                    }
                }, c);
        }), dctx, [](gpointer d, GClosure*) { delete static_cast<DelProdCtx*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(btnRow), btnDelete);
    }

    struct SaveProdCtx {
        AdminScreens* s;
        ProductItemViewModel orig;
        bool isNew;
        GtkWidget* eName; GtkWidget* eBrand; GtkWidget* eOs;
        GtkWidget* eManu; GtkWidget* eImport; GtkWidget* eSelling;
        GtkWidget* eStock; GtkWidget* eSpecs; GtkWidget* eImg;
        GtkWidget* errName; GtkWidget* errBrand;
        GtkWidget* errImport; GtkWidget* errSelling; GtkWidget* errStock;
    };
    SaveProdCtx* sctx = new SaveProdCtx{
        this, prod, isNew,
        eName, eBrand, eOs, eManu, eImport, eSelling, eStock, eSpecs, eImgPath,
        errName, errBrand, errImport, errSelling, errStock
    };

    GtkWidget* btnSave = gtk_button_new_with_label(isNew ? "💾 Tạo sản phẩm mới" : "💾 Lưu thay đổi sản phẩm");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    g_signal_connect_data(btnSave, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        SaveProdCtx* c = static_cast<SaveProdCtx*>(d);
        UIComponents::clearFieldError(c->errName, c->eName);
        UIComponents::clearFieldError(c->errBrand, c->eBrand);
        UIComponents::clearFieldError(c->errImport, c->eImport);
        UIComponents::clearFieldError(c->errSelling, c->eSelling);
        UIComponents::clearFieldError(c->errStock, c->eStock);

        ProductItemViewModel p = c->orig;
        p.name = gtk_editable_get_text(GTK_EDITABLE(c->eName));
        p.brand = gtk_editable_get_text(GTK_EDITABLE(c->eBrand));
        p.os = gtk_editable_get_text(GTK_EDITABLE(c->eOs));
        p.manufacturer = gtk_editable_get_text(GTK_EDITABLE(c->eManu));
        p.specs = gtk_editable_get_text(GTK_EDITABLE(c->eSpecs));
        p.imageUrl = gtk_editable_get_text(GTK_EDITABLE(c->eImg));

        p.importPrice = UIComponents::parseCurrencyInput(c->eImport);
        p.sellingPrice = UIComponents::parseCurrencyInput(c->eSelling);
        try { p.actualStock = std::stoi(gtk_editable_get_text(GTK_EDITABLE(c->eStock))); } catch (...) { p.actualStock = -1; }
        p.calculateStockStatus();

        bool hasError = false;
        GtkWidget* firstErr = nullptr;
        if (p.name.empty()) {
            UIComponents::setFieldError(c->errName, c->eName, "Tên sản phẩm không được để trống");
            hasError = true;
            if (!firstErr) firstErr = c->eName;
        }
        if (p.brand.empty()) {
            UIComponents::setFieldError(c->errBrand, c->eBrand, "Hãng sản xuất không được để trống");
            hasError = true;
            if (!firstErr) firstErr = c->eBrand;
        }
        if (p.importPrice <= 0) {
            UIComponents::setFieldError(c->errImport, c->eImport, "Giá nhập phải lớn hơn 0");
            hasError = true;
            if (!firstErr) firstErr = c->eImport;
        }
        if (p.sellingPrice <= 0) {
            UIComponents::setFieldError(c->errSelling, c->eSelling, "Giá bán niêm yết phải lớn hơn 0");
            hasError = true;
            if (!firstErr) firstErr = c->eSelling;
        }
        if (p.actualStock < 0) {
            UIComponents::setFieldError(c->errStock, c->eStock, "Số lượng tồn kho phải là số không âm");
            hasError = true;
            if (!firstErr) firstErr = c->eStock;
        }

        if (hasError) {
            if (firstErr) gtk_widget_grab_focus(firstErr);
            return;
        }

        OperationResult res = c->isNew ? c->s->controller->addProduct(p) : c->s->controller->updateProduct(p);
        if (res.success) {
            UIComponents::showSuccessDialog(c->s->shell ? c->s->shell->getWindow() : nullptr, "Thành công", res.message);
            c->s->showProductListView();
        } else {
            UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow() : nullptr, "Lỗi", res.message);
        }
    }), sctx, [](gpointer d, GClosure*) { delete static_cast<SaveProdCtx*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(btnRow), btnSave);
    gtk_box_append(GTK_BOX(card), btnRow);

    gtk_box_append(GTK_BOX(content), card);
    gtk_box_append(GTK_BOX(productContainerBox), content);

    UIComponents::setupDialogShortcuts(content, btnSave, btnCancel);
    gtk_widget_grab_focus(eName);
}

// ================= 3. QUẢN LÝ NHÂN SỰ =================

void AdminScreens::buildEmployeeMgmtView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* btnAdd = gtk_button_new_with_label("➕ Thêm nhân viên");
    gtk_widget_add_css_class(btnAdd, "btn-primary");
    UIComponents::setButtonCursor(btnAdd);
    g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        s->showEmployeeFormModal(nullptr);
    }), this);

    GtkWidget* header = UIComponents::createSectionHeader("Quản lý đội ngũ nhân viên", "Thêm, sửa, xóa, phân quyền chức vụ (Bán hàng, Thủ kho, Quản trị).", btnAdd);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo tên, mã hoặc số điện thoại...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->refreshEmployees(txt ? txt : "");
        }),
        this,
        nullptr,
        &employeeCountBadge
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kEmployeeTableCols));

    employeeListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), employeeListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(employeeMgmtView), content);
    refreshEmployees();
}

void AdminScreens::refreshEmployees(const std::string& keyword) {
    if (!employeeListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(employeeListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(employeeListBox), child);
        child = next;
    }

    std::vector<EmployeeItemViewModel> emps = controller->getEmployees(keyword);
    UIComponents::updateRecordCountBadge(employeeCountBadge, emps.size(), "nhân viên");

    if (emps.empty()) {
        std::string emptyMsg = keyword.empty() ? "Không tìm thấy nhân viên nào trong hệ thống." : "Không có nhân viên nào khớp với từ khóa tìm kiếm.";
        GtkWidget* emptyWidget = UIComponents::createEmptyState(
            "👥", "Chưa có nhân viên", emptyMsg,
            keyword.empty() ? "➕ Thêm nhân viên đầu tiên" : "",
            G_CALLBACK(+[](GtkButton*, gpointer d) {
                AdminScreens* s = static_cast<AdminScreens*>(d);
                s->showEmployeeFormModal(nullptr);
            }), this);
        gtk_box_append(GTK_BOX(employeeListBox), emptyWidget);
        return;
    }

    for (size_t i = 0; i < emps.size(); ++i) {
        const auto& e = emps[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Mã & Họ tên
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
            e.fullName, "Mã: " + e.id, kEmployeeTableCols[0]));

        // 1: Tài khoản
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            e.username, kEmployeeTableCols[1], "cell-code"));

        // 2: Số điện thoại
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            e.phone, kEmployeeTableCols[2], "tabular-num"));

        // 3: Chức vụ (Huy hiệu)
        GtkWidget* roleBadge = UIComponents::createStatusBadge(e.roleName,
            e.role == RoleType::ADMIN ? "badge-cancelled" : (e.role == RoleType::SALE_EMPLOYEE ? "badge-confirmed" : "badge-pending"));
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(roleBadge, kEmployeeTableCols[3]));

        // 4: Mức lương (Căn phải)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(e.salary), kEmployeeTableCols[4], "product-price"));

        // 5: Thao tác
        GtkWidget* btnEdit = UIComponents::createTableActionButton("✏️ Sửa", "Chỉnh sửa thông tin và phân quyền nhân sự", "btn-action-edit");
        struct EmpClickData { AdminScreens* s; EmployeeItemViewModel e; };
        EmpClickData* ecd = new EmpClickData{this, e};
        g_signal_connect_data(btnEdit, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            EmpClickData* data = static_cast<EmpClickData*>(d);
            data->s->showEmployeeFormModal(&data->e);
        }), ecd, [](gpointer d, GClosure*) { delete static_cast<EmpClickData*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(btnEdit, kEmployeeTableCols[5]));
        gtk_box_append(GTK_BOX(employeeListBox), row);
    }
}

void AdminScreens::showEmployeeFormModal(const EmployeeItemViewModel* existingEmp) {
    bool isEdit = (existingEmp != nullptr);
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), isEdit ? "Chỉnh sửa nhân sự" : "Thêm nhân sự mới");
    gtk_window_set_default_size(GTK_WINDOW(win), 480, 560);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    if (shell) {
        gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    }

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(card, "dialog-content");

    GtkWidget* title = gtk_label_new(isEdit ? "Chỉnh sửa thông tin nhân sự" : "Tạo mới tài khoản nhân viên");
    gtk_widget_add_css_class(title, "dialog-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(card), title);

    GtkWidget* eFirst = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eFirst), isEdit ? existingEmp->firstName.c_str() : "");
    GtkWidget* errFirst = nullptr;
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Họ đệm *", eFirst, &errFirst));

    GtkWidget* eLast = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eLast), isEdit ? existingEmp->lastName.c_str() : "");
    GtkWidget* errLast = nullptr;
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Tên *", eLast, &errLast));

    GtkWidget* ePhone = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(ePhone), isEdit ? existingEmp->phone.c_str() : "");
    UIComponents::setupPhoneInput(ePhone);
    GtkWidget* errPhone = nullptr;
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Số điện thoại *", ePhone, &errPhone));

    GtkWidget* eEmail = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eEmail), isEdit ? existingEmp->email.c_str() : "");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Email", eEmail));

    GtkWidget* eUser = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eUser), isEdit ? existingEmp->username.c_str() : "");
    GtkWidget* errUser = nullptr;
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Tên đăng nhập *", eUser, &errUser));

    const char* roles[] = {"Quản trị viên (Admin)", "Nhân viên Bán hàng (Sales)", "Thủ kho (Warehouse)", nullptr};
    GtkWidget* comboRole = gtk_drop_down_new_from_strings(roles);
    if (isEdit) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(comboRole),
            existingEmp->role == RoleType::ADMIN ? 0 : (existingEmp->role == RoleType::SALE_EMPLOYEE ? 1 : 2));
    }
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Chức vụ *", comboRole));

    GtkWidget* eSalary = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eSalary), isEdit ? std::to_string(static_cast<long long>(existingEmp->salary)).c_str() : "8000000");
    UIComponents::setupCurrencyInput(eSalary);
    GtkWidget* errSalary = nullptr;
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Mức lương (VNĐ) *", eSalary, &errSalary));

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
    gtk_widget_set_margin_top(btnRow, 12);

    GtkWidget* btnCancel = gtk_button_new_with_label("Hủy");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(btnRow), btnCancel);

    struct EmpCtx {
        AdminScreens* s; GtkWidget* w; bool isEdit; std::string id;
        GtkWidget* eFirst; GtkWidget* eLast; GtkWidget* ePhone;
        GtkWidget* eEmail; GtkWidget* eUser; GtkWidget* cRole; GtkWidget* eSalary;
        GtkWidget* errFirst; GtkWidget* errLast; GtkWidget* errPhone;
        GtkWidget* errUser; GtkWidget* errSalary;
    };
    EmpCtx* ctx = new EmpCtx{
        this, win, isEdit, isEdit ? existingEmp->id : "",
        eFirst, eLast, ePhone, eEmail, eUser, comboRole, eSalary,
        errFirst, errLast, errPhone, errUser, errSalary
    };

    GtkWidget* btnSave = gtk_button_new_with_label("Lưu");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    g_signal_connect_data(btnSave, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        EmpCtx* c = static_cast<EmpCtx*>(d);
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
        req.role = (sel == 0) ? RoleType::ADMIN : (sel == 1 ? RoleType::SALE_EMPLOYEE : RoleType::WAREHOUSE_EMPLOYEE);
        req.salary = UIComponents::parseCurrencyInput(c->eSalary);

        bool hasError = false;
        GtkWidget* firstErr = nullptr;
        if (req.firstName.empty()) {
            UIComponents::setFieldError(c->errFirst, c->eFirst, "Họ đệm không được để trống");
            hasError = true;
            if (!firstErr) firstErr = c->eFirst;
        }
        if (req.lastName.empty()) {
            UIComponents::setFieldError(c->errLast, c->eLast, "Tên không được để trống");
            hasError = true;
            if (!firstErr) firstErr = c->eLast;
        }
        std::string phoneErr;
        if (!UIComponents::validatePhoneNumber(req.phone, phoneErr)) {
            UIComponents::setFieldError(c->errPhone, c->ePhone, phoneErr);
            hasError = true;
            if (!firstErr) firstErr = c->ePhone;
        }
        if (req.username.empty()) {
            UIComponents::setFieldError(c->errUser, c->eUser, "Tên đăng nhập không được để trống");
            hasError = true;
            if (!firstErr) firstErr = c->eUser;
        }
        if (req.salary <= 0) {
            UIComponents::setFieldError(c->errSalary, c->eSalary, "Mức lương phải lớn hơn 0");
            hasError = true;
            if (!firstErr) firstErr = c->eSalary;
        }

        if (hasError) {
            if (firstErr) gtk_widget_grab_focus(firstErr);
            return;
        }

        OperationResult res = c->isEdit ? c->s->controller->updateEmployee(req) : c->s->controller->addEmployee(req);
        if (res.success) {
            c->s->refreshEmployees();
            UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
            gtk_window_destroy(GTK_WINDOW(c->w));
        } else {
            if (res.message.find("đăng nhập") != std::string::npos) {
                UIComponents::setFieldError(c->errUser, c->eUser, res.message);
                gtk_widget_grab_focus(c->eUser);
            } else {
                UIComponents::showErrorDialog(GTK_WINDOW(c->w), "Lỗi", res.message);
            }
        }
    }), ctx, [](gpointer d, GClosure*) { delete static_cast<EmpCtx*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(btnRow), btnSave);
    gtk_box_append(GTK_BOX(card), btnRow);

    gtk_window_set_child(GTK_WINDOW(win), card);
    UIComponents::setupDialogShortcuts(win, btnSave, btnCancel);
    gtk_widget_grab_focus(eFirst);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 4. QUẢN LÝ KHÁCH HÀNG (ĐẶC TẢ MÀU RANK) =================

void AdminScreens::buildCustomerMgmtView() {
    customerContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(customerMgmtView), customerContainerBox);
    showCustomerListView();
}

void AdminScreens::showCustomerListView() {
    customerListBox = nullptr;
    customerCountBadge = nullptr;
    GtkWidget* child = gtk_widget_get_first_child(customerContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(customerContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Quản lý danh sách khách hàng", "Theo dõi thông tin liên lạc, lịch sử mua sắm và phân hạng thành viên chuẩn màu sắc.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm khách theo tên, mã hoặc số điện thoại...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->refreshCustomers(txt ? txt : "");
        }),
        this,
        nullptr,
        &customerCountBadge
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kCustomerTableCols));

    customerListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), customerListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_box_append(GTK_BOX(customerContainerBox), content);
    refreshCustomers();
}

void AdminScreens::refreshCustomers(const std::string& keyword) {
    if (!customerListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(customerListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(customerListBox), child);
        child = next;
    }

    std::vector<CustomerItemViewModel> custs = controller->getCustomers(keyword);
    UIComponents::updateRecordCountBadge(customerCountBadge, custs.size(), "khách hàng");

    if (custs.empty()) {
        std::string emptyMsg = keyword.empty() ? "Không tìm thấy khách hàng nào trong hệ thống." : "Không tìm thấy khách hàng nào khớp với từ khóa tìm kiếm.";
        gtk_box_append(GTK_BOX(customerListBox), UIComponents::createEmptyState("👥", "Chưa có khách hàng", emptyMsg));
        return;
    }

    for (size_t i = 0; i < custs.size(); ++i) {
        const auto& c = custs[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Mã & Họ tên
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
            c.fullName, "Mã: " + c.id, kCustomerTableCols[0]));

        // 1: Số điện thoại
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            c.phone, kCustomerTableCols[1], "tabular-num"));

        // 2: Địa chỉ
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            c.address, kCustomerTableCols[2], "cell-subtitle"));

        // 3: Điểm tích lũy (Căn phải)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatNumber(c.points) + " điểm", kCustomerTableCols[3], "cell-number"));

        // 4: Hạng thành viên (Badge)
        GtkWidget* rankBadge = UIComponents::createRankBadge(c.membershipTier);
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(rankBadge, kCustomerTableCols[4]));

        // 5: Thao tác
        GtkWidget* btnDetail = UIComponents::createTableActionButton("🔍 Chi tiết", "Xem thông tin chi tiết và lịch sử mua sắm của khách", "btn-action-view");
        struct CustClickData { AdminScreens* s; CustomerItemViewModel c; };
        CustClickData* ccd = new CustClickData{this, c};
        g_signal_connect_data(btnDetail, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            CustClickData* data = static_cast<CustClickData*>(d);
            data->s->showCustomerDetailView(data->c);
        }), ccd, [](gpointer d, GClosure*) { delete static_cast<CustClickData*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(btnDetail, kCustomerTableCols[5]));
        gtk_box_append(GTK_BOX(customerListBox), row);
    }
}

void AdminScreens::showCustomerDetailView(const CustomerItemViewModel& cust) {
    customerListBox = nullptr;
    customerCountBadge = nullptr;
    GtkWidget* child = gtk_widget_get_first_child(customerContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(customerContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createBackHeader("Hồ sơ khách hàng: " + cust.fullName, "Xem chi tiết hạng thành viên, điểm tích lũy và cập nhật thông tin liên hệ.",
        G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AdminScreens* self = static_cast<AdminScreens*>(d);
            self->showCustomerListView();
        }), this);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(card, "card");

    // Rank & Stats Banner
    GtkWidget* banner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_LG);
    GtkWidget* rankBadge = UIComponents::createRankBadge(cust.membershipTier);
    GtkWidget* lblPts = gtk_label_new(("Tổng điểm thưởng: " + UIComponents::formatNumber(cust.points) + " điểm").c_str());
    gtk_widget_add_css_class(lblPts, "card-title");
    gtk_widget_add_css_class(lblPts, "tabular-num");
    gtk_box_append(GTK_BOX(banner), rankBadge);
    gtk_box_append(GTK_BOX(banner), lblPts);
    gtk_box_append(GTK_BOX(card), banner);

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    GtkWidget* eName = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eName), cust.fullName.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Họ và tên *", eName), 0, 0, 2, 1);

    GtkWidget* ePhone = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(ePhone), cust.phone.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Số điện thoại *", ePhone), 2, 0, 1, 1);

    GtkWidget* eEmail = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eEmail), cust.email.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Email", eEmail), 0, 1, 1, 1);

    GtkWidget* eAddr = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eAddr), cust.address.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Địa chỉ liên hệ", eAddr), 1, 1, 2, 1);

    gtk_box_append(GTK_BOX(card), grid);

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);

    GtkWidget* btnBack = gtk_button_new_with_label("← Quay lại");
    gtk_widget_add_css_class(btnBack, "btn-secondary");
    UIComponents::setButtonCursor(btnBack);
    g_signal_connect_swapped(btnBack, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        s->showCustomerListView();
    }), this);
    gtk_box_append(GTK_BOX(btnRow), btnBack);

    struct SaveCustCtx { AdminScreens* s; CustomerItemViewModel cust; GtkWidget* eName; GtkWidget* ePhone; GtkWidget* eEmail; GtkWidget* eAddr; };
    SaveCustCtx* sctx = new SaveCustCtx{this, cust, eName, ePhone, eEmail, eAddr};

    GtkWidget* btnSave = gtk_button_new_with_label("💾 Lưu thông tin khách hàng");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    g_signal_connect_data(btnSave, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        SaveCustCtx* c = static_cast<SaveCustCtx*>(d);
        CustomerFormRequest req;
        req.id = c->cust.id;
        req.lastName = gtk_editable_get_text(GTK_EDITABLE(c->eName));
        req.phone = gtk_editable_get_text(GTK_EDITABLE(c->ePhone));
        req.email = gtk_editable_get_text(GTK_EDITABLE(c->eEmail));
        req.address = gtk_editable_get_text(GTK_EDITABLE(c->eAddr));

        OperationResult res = c->s->controller->updateCustomer(req);
        if (res.success) {
            UIComponents::showSuccessDialog(c->s->shell ? c->s->shell->getWindow() : nullptr, "Thành công", res.message);
            c->s->showCustomerListView();
        } else {
            UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow() : nullptr, "Lỗi", res.message);
        }
    }), sctx, [](gpointer d, GClosure*) { delete static_cast<SaveCustCtx*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(btnRow), btnSave);
    gtk_box_append(GTK_BOX(card), btnRow);

    gtk_box_append(GTK_BOX(content), card);
    gtk_box_append(GTK_BOX(customerContainerBox), content);
    UIComponents::setupDialogShortcuts(content, btnSave, btnBack);
}

// ================= 5. QUẢN LÝ NHÀ CUNG CẤP =================

void AdminScreens::buildSupplierMgmtView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* btnAdd = gtk_button_new_with_label("➕ Thêm nhà cung cấp");
    gtk_widget_add_css_class(btnAdd, "btn-primary");
    UIComponents::setButtonCursor(btnAdd);
    g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        s->showSupplierFormModal(nullptr);
    }), this);

    GtkWidget* header = UIComponents::createSectionHeader("Nhà cung cấp điện thoại", "Quản lý đối tác phân phối chính hãng (Apple, Samsung, Xiaomi...).", btnAdd);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo tên công ty hoặc mã số thuế...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->refreshSuppliers(txt ? txt : "");
        }),
        this,
        nullptr,
        &supplierCountBadge
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kSupplierTableCols));

    supplierListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), supplierListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(supplierMgmtView), content);
    refreshSuppliers();
}

void AdminScreens::refreshSuppliers(const std::string& keyword) {
    if (!supplierListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(supplierListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(supplierListBox), child);
        child = next;
    }

    std::vector<SupplierItemViewModel> sups = controller->getSuppliers(keyword);
    UIComponents::updateRecordCountBadge(supplierCountBadge, sups.size(), "nhà cung cấp");

    if (sups.empty()) {
        std::string emptyMsg = keyword.empty() ? "Không tìm thấy nhà cung cấp nào trong hệ thống." : "Không có nhà cung cấp nào khớp với từ khóa tìm kiếm.";
        GtkWidget* emptyWidget = UIComponents::createEmptyState(
            "🏢", "Chưa có nhà cung cấp", emptyMsg,
            keyword.empty() ? "➕ Thêm nhà cung cấp mới" : "",
            G_CALLBACK(+[](GtkButton*, gpointer d) {
                AdminScreens* s = static_cast<AdminScreens*>(d);
                s->showSupplierFormModal(nullptr);
            }), this);
        gtk_box_append(GTK_BOX(supplierListBox), emptyWidget);
        return;
    }

    for (size_t i = 0; i < sups.size(); ++i) {
        const auto& s = sups[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Công ty cung cấp
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
            s.companyName, "Mã: " + s.id, kSupplierTableCols[0]));

        // 1: Mã số thuế
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            s.taxCode, kSupplierTableCols[1], "cell-code"));

        // 2: Người liên hệ
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            s.contactPerson, kSupplierTableCols[2], "cell-title"));

        // 3: Số điện thoại
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            s.phone, kSupplierTableCols[3], "tabular-num"));

        // 4: Địa chỉ
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            s.address, kSupplierTableCols[4], "cell-subtitle"));

        // 5: Thao tác
        GtkWidget* btnEdit = UIComponents::createTableActionButton("✏️ Sửa", "Chỉnh sửa thông tin nhà cung cấp", "btn-action-edit");
        struct SupClickData { AdminScreens* s; SupplierItemViewModel sp; };
        SupClickData* scd = new SupClickData{this, s};
        g_signal_connect_data(btnEdit, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            SupClickData* data = static_cast<SupClickData*>(d);
            data->s->showSupplierFormModal(&data->sp);
        }), scd, [](gpointer d, GClosure*) { delete static_cast<SupClickData*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(btnEdit, kSupplierTableCols[5]));
        gtk_box_append(GTK_BOX(supplierListBox), row);
    }
}

void AdminScreens::showSupplierFormModal(const SupplierItemViewModel* existingSup) {
    bool isEdit = (existingSup != nullptr);
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), isEdit ? "Sửa nhà cung cấp" : "Thêm nhà cung cấp mới");
    gtk_window_set_default_size(GTK_WINDOW(win), 460, 420);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(card, "dialog-content");

    GtkWidget* title = gtk_label_new(isEdit ? "Chỉnh sửa nhà cung cấp" : "Tạo đối tác cung ứng mới");
    gtk_widget_add_css_class(title, "dialog-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(card), title);

    GtkWidget* eName = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eName), isEdit ? existingSup->companyName.c_str() : "");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Tên công ty / Doanh nghiệp *", eName));

    GtkWidget* eTax = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eTax), isEdit ? existingSup->taxCode.c_str() : "");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Mã số thuế *", eTax));

    GtkWidget* eContact = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eContact), isEdit ? existingSup->contactPerson.c_str() : "");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Người đại diện liên hệ", eContact));

    GtkWidget* ePhone = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(ePhone), isEdit ? existingSup->phone.c_str() : "");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Số điện thoại *", ePhone));

    GtkWidget* eAddr = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eAddr), isEdit ? existingSup->address.c_str() : "");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Địa chỉ trụ sở", eAddr));

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
    gtk_widget_set_margin_top(btnRow, 12);

    GtkWidget* btnCancel = gtk_button_new_with_label("Hủy");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(btnRow), btnCancel);

    struct SupCtx { AdminScreens* s; GtkWidget* w; bool isEdit; std::string id; GtkWidget* eN; GtkWidget* eT; GtkWidget* eC; GtkWidget* eP; GtkWidget* eA; };
    SupCtx* ctx = new SupCtx{this, win, isEdit, isEdit ? existingSup->id : "", eName, eTax, eContact, ePhone, eAddr};

    GtkWidget* btnSave = gtk_button_new_with_label("Lưu");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    g_signal_connect_data(btnSave, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        SupCtx* c = static_cast<SupCtx*>(d);
        SupplierFormRequest req;
        req.id = c->id;
        req.companyName = gtk_editable_get_text(GTK_EDITABLE(c->eN));
        req.taxCode = gtk_editable_get_text(GTK_EDITABLE(c->eT));
        req.contactPerson = gtk_editable_get_text(GTK_EDITABLE(c->eC));
        req.phone = gtk_editable_get_text(GTK_EDITABLE(c->eP));
        req.address = gtk_editable_get_text(GTK_EDITABLE(c->eA));

        OperationResult res = c->isEdit ? c->s->controller->updateSupplier(req) : c->s->controller->addSupplier(req);
        if (res.success) {
            c->s->refreshSuppliers();
            UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
            gtk_window_destroy(GTK_WINDOW(c->w));
        } else {
            UIComponents::showErrorDialog(GTK_WINDOW(c->w), "Lỗi", res.message);
        }
    }), ctx, [](gpointer d, GClosure*) { delete static_cast<SupCtx*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(btnRow), btnSave);
    gtk_box_append(GTK_BOX(card), btnRow);

    gtk_window_set_child(GTK_WINDOW(win), card);
    UIComponents::setupDialogShortcuts(win, btnSave, btnCancel);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 6. QUẢN LÝ MÃ GIẢM GIÁ (ĐỐI SOÁT REAL-TIME) =================

void AdminScreens::buildDiscountMgmtView() {
    discountContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(discountMgmtView), discountContainerBox);
    showDiscountListView();
}

void AdminScreens::showDiscountListView() {
    discountListBox = nullptr;
    discountCountBadge = nullptr;
    GtkWidget* child = gtk_widget_get_first_child(discountContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(discountContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* btnAdd = gtk_button_new_with_label("➕ Tạo mã giảm giá mới");
    gtk_widget_add_css_class(btnAdd, "btn-primary");
    UIComponents::setButtonCursor(btnAdd);
    g_signal_connect_swapped(btnAdd, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        DiscountItemViewModel newD;
        newD.code = "VOUCHER";
        newD.expiryDate = "31/12/2026";
        newD.value = 10;
        s->showDiscountEditView(newD, true);
    }), this);

    std::string sub = "Quản lý voucher, chỉnh sửa thời gian hiệu lực và đối soát tự động theo mốc thời gian thực (" + controller->getSimulatedDate() + ").";
    GtkWidget* header = UIComponents::createSectionHeader("Chương trình khuyến mãi & Voucher", sub, btnAdd);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo mã voucher hoặc mô tả chương trình...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->discountFilterKeyword = txt ? txt : "";
            self->refreshDiscounts();
        }),
        this,
        nullptr,
        &discountCountBadge
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kDiscountTableCols));

    discountListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), discountListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_box_append(GTK_BOX(discountContainerBox), content);
    refreshDiscounts();
}

void AdminScreens::refreshDiscounts() {
    if (!discountListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(discountListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(discountListBox), child);
        child = next;
    }

    std::vector<DiscountItemViewModel> allList = controller->getDiscounts();
    std::vector<DiscountItemViewModel> list;
    for (const auto& d : allList) {
        if (!discountFilterKeyword.empty()) {
            bool match = Utf8Helper::containsCaseInsensitive(d.code, discountFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(d.description, discountFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(d.expiryDate, discountFilterKeyword);
            if (!match) continue;
        }
        list.push_back(d);
    }

    UIComponents::updateRecordCountBadge(discountCountBadge, list.size(), "voucher");

    if (list.empty()) {
        GtkWidget* emptyWidget = UIComponents::createEmptyState(
            "🎟️",
            discountFilterKeyword.empty() ? "Chưa có mã giảm giá" : "Không tìm thấy voucher phù hợp",
            discountFilterKeyword.empty() ? "Không có mã giảm giá nào trong hệ thống." : "Thử tìm kiếm với mã voucher hoặc từ khóa khác.",
            discountFilterKeyword.empty() ? "➕ Tạo mã giảm giá mới" : "",
            G_CALLBACK(+[](GtkButton*, gpointer d) {
                AdminScreens* s = static_cast<AdminScreens*>(d);
                DiscountItemViewModel newD;
                newD.code = "VOUCHER";
                newD.expiryDate = "31/12/2026";
                newD.value = 10;
                s->showDiscountEditView(newD, true);
            }), this);
        gtk_box_append(GTK_BOX(discountListBox), emptyWidget);
        return;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& d = list[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Mã Voucher
        GtkWidget* codeBadge = UIComponents::createStatusBadge(d.code, "badge-confirmed");
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(codeBadge, kDiscountTableCols[0]));

        // 1: Mô tả chương trình
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            d.description, kDiscountTableCols[1], "cell-title"));

        // 2: Mức giảm (Căn phải)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            d.formattedValue, kDiscountTableCols[2], "product-price"));

        // 3: Đơn tối thiểu (Căn phải)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(d.minOrderValue), kDiscountTableCols[3], "tabular-num"));

        // 4: Hạn dùng (Căn giữa)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            d.expiryDate, kDiscountTableCols[4], "tabular-num"));

        // 5: Đã dùng (Căn phải)
        std::string usedStr = UIComponents::formatNumber(d.usedCount) + "/" + UIComponents::formatNumber(d.maxUsage);
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            usedStr, kDiscountTableCols[5], "cell-number"));

        // 6: Trạng thái
        int remDays = RealTimeManager::getInstance().getRemainingDays(d.expiryDate);
        std::string statusText = d.isValid ? ("🟢 Còn " + std::to_string(remDays) + " ngày")
                                           : ("🔴 Hết hạn");
        GtkWidget* statusBadge = UIComponents::createStatusBadge(statusText, d.isValid ? "badge-completed" : "badge-cancelled");
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(statusBadge, kDiscountTableCols[6]));

        // 7: Thao tác
        GtkWidget* btnEdit = UIComponents::createTableActionButton("📝 Sửa", "Chỉnh sửa voucher và đối soát thời gian thực", "btn-action-edit");
        struct DiscClickData { AdminScreens* s; DiscountItemViewModel d; };
        DiscClickData* dcd = new DiscClickData{this, d};
        g_signal_connect_data(btnEdit, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            DiscClickData* data = static_cast<DiscClickData*>(d);
            data->s->showDiscountEditView(data->d, false);
        }), dcd, [](gpointer d, GClosure*) { delete static_cast<DiscClickData*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(btnEdit, kDiscountTableCols[7]));
        gtk_box_append(GTK_BOX(discountListBox), row);
    }
}

void AdminScreens::showDiscountEditView(const DiscountItemViewModel& disc, bool isNew) {
    discountListBox = nullptr;
    discountCountBadge = nullptr;
    GtkWidget* child = gtk_widget_get_first_child(discountContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(discountContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
    gtk_widget_add_css_class(content, "page-container");

    std::string pageTitle = isNew ? "Tạo mã giảm giá mới" : ("Chỉnh sửa & Đối soát Voucher: " + disc.code);
    GtkWidget* header = UIComponents::createBackHeader(pageTitle, "Tùy chỉnh ngày bắt đầu, ngày hết hạn và kiểm tra đối soát với thời gian thực hệ thống.",
        G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AdminScreens* self = static_cast<AdminScreens*>(d);
            self->showDiscountListView();
        }), this);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(card, "card");

    // Real-Time Reconciliation Banner
    GtkWidget* reconBanner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(reconBanner, "realtime-clock-box");

    GtkWidget* bannerInfo = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget* bannerTitle = gtk_label_new("⏱️ ĐỐI SOÁT THỜI GIAN THỰC HỆ THỐNG");
    gtk_widget_add_css_class(bannerTitle, "stat-label");
    gtk_label_set_xalign(GTK_LABEL(bannerTitle), 0.0f);

    std::string simTimeInfo = "Thời gian giả lập hiện hành: " + controller->getSimulatedTime();
    GtkWidget* bannerTime = gtk_label_new(simTimeInfo.c_str());
    gtk_widget_add_css_class(bannerTime, "card-title");
    gtk_label_set_xalign(GTK_LABEL(bannerTime), 0.0f);

    int daysLeft = RealTimeManager::getInstance().getRemainingDays(disc.expiryDate);
    std::string statusNotice = (daysLeft >= 0) ? ("Tình trạng: 🟢 Hợp lệ (còn " + std::to_string(daysLeft) + " ngày)")
                                              : ("Tình trạng: 🔴 Đã hết hạn (quá hạn " + std::to_string(std::abs(daysLeft)) + " ngày)");
    GtkWidget* bannerStatus = gtk_label_new(statusNotice.c_str());
    gtk_widget_add_css_class(bannerStatus, "stat-trend");
    gtk_label_set_xalign(GTK_LABEL(bannerStatus), 0.0f);

    gtk_box_append(GTK_BOX(bannerInfo), bannerTitle);
    gtk_box_append(GTK_BOX(bannerInfo), bannerTime);
    gtk_box_append(GTK_BOX(bannerInfo), bannerStatus);
    gtk_box_append(GTK_BOX(reconBanner), bannerInfo);
    gtk_box_append(GTK_BOX(card), reconBanner);

    // Form inputs
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    GtkWidget* eCode = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eCode), isNew ? "GIAM10" : disc.code.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Mã Code Voucher *", eCode), 0, 0, 1, 1);

    const char* types[] = {"Giảm phần trăm (%)", "Số tiền cố định (VNĐ)", nullptr};
    GtkWidget* comboType = gtk_drop_down_new_from_strings(types);
    if (!isNew && disc.typeName.find("cố định") != std::string::npos) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(comboType), 1);
    }
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Loại giảm giá *", comboType), 1, 0, 1, 1);

    GtkWidget* eVal = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eVal), isNew ? "10" : std::to_string(static_cast<long long>(disc.value)).c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Giá trị giảm (% hoặc VNĐ) *", eVal), 2, 0, 1, 1);

    GtkWidget* eMinOrder = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eMinOrder), isNew ? "5000000" : std::to_string(static_cast<long long>(disc.minOrderValue)).c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Đơn hàng tối thiểu (VNĐ)", eMinOrder), 0, 1, 1, 1);

    GtkWidget* eMaxDisc = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eMaxDisc), isNew ? "500000" : std::to_string(static_cast<long long>(disc.maxDiscountAmount)).c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Mức giảm tối đa (VNĐ)", eMaxDisc), 1, 1, 1, 1);

    // Chỉnh sửa ngày hết hạn (hỗ trợ DD/MM/YYYY hoặc YYYY-MM-DD)
    GtkWidget* eExpiry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eExpiry), isNew ? "31/12/2026" : disc.expiryDate.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Ngày hết hạn (DD/MM/YYYY) *", eExpiry), 2, 1, 1, 1);

    GtkWidget* eUsage = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eUsage), isNew ? "100" : std::to_string(disc.maxUsage).c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Số lượt sử dụng tối đa", eUsage), 0, 2, 1, 1);

    GtkWidget* eDesc = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eDesc), isNew ? "Giảm giá đặc biệt" : disc.description.c_str());
    gtk_grid_attach(GTK_GRID(grid), UIComponents::createFormField("Mô tả chương trình ưu đãi", eDesc), 1, 2, 2, 1);

    gtk_box_append(GTK_BOX(card), grid);

    // Quick extension buttons
    GtkWidget* quickRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* lblQuick = gtk_label_new("Gia hạn nhanh:");
    gtk_widget_add_css_class(lblQuick, "stat-label");
    gtk_box_append(GTK_BOX(quickRow), lblQuick);

    struct ExtDateData { GtkWidget* entry; std::string date; };
    auto addExtBtn = [eExpiry](const char* label, const char* targetDate) {
        GtkWidget* btn = gtk_button_new_with_label(label);
        gtk_widget_add_css_class(btn, "btn-outline");
        UIComponents::setButtonCursor(btn);
        ExtDateData* ed = new ExtDateData{eExpiry, targetDate};
        g_signal_connect_data(btn, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            ExtDateData* data = static_cast<ExtDateData*>(d);
            gtk_editable_set_text(GTK_EDITABLE(data->entry), data->date.c_str());
        }), ed, [](gpointer d, GClosure*) { delete static_cast<ExtDateData*>(d); }, (GConnectFlags)0);
        return btn;
    };
    gtk_box_append(GTK_BOX(quickRow), addExtBtn("+30 ngày (30/11/2026)", "30/11/2026"));
    gtk_box_append(GTK_BOX(quickRow), addExtBtn("Hết năm 2026 (31/12/2026)", "31/12/2026"));
    gtk_box_append(GTK_BOX(quickRow), addExtBtn("Đến năm 2027 (31/12/2027)", "31/12/2027"));
    gtk_box_append(GTK_BOX(card), quickRow);

    // Bottom action buttons
    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
    gtk_widget_set_margin_top(btnRow, 14);

    GtkWidget* btnCancel = gtk_button_new_with_label("← Quay lại danh sách");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        s->showDiscountListView();
    }), this);
    gtk_box_append(GTK_BOX(btnRow), btnCancel);

    struct SaveDiscCtx {
        AdminScreens* s; DiscountItemViewModel orig; bool isNew;
        GtkWidget* eC; GtkWidget* cT; GtkWidget* eV; GtkWidget* eMin; GtkWidget* eMax;
        GtkWidget* eExp; GtkWidget* eUsg; GtkWidget* eD;
    };
    SaveDiscCtx* sctx = new SaveDiscCtx{this, disc, isNew, eCode, comboType, eVal, eMinOrder, eMaxDisc, eExpiry, eUsage, eDesc};

    GtkWidget* btnSave = gtk_button_new_with_label(isNew ? "💾 Tạo Voucher mới" : "💾 Lưu & Đối soát Voucher");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    g_signal_connect_data(btnSave, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        SaveDiscCtx* c = static_cast<SaveDiscCtx*>(d);
        DiscountFormRequest req;
        req.id = c->orig.id;
        req.code = gtk_editable_get_text(GTK_EDITABLE(c->eC));
        req.type = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->cT));
        try { req.value = std::stod(gtk_editable_get_text(GTK_EDITABLE(c->eV))); } catch (...) { req.value = 10; }
        try { req.minOrderValue = std::stod(gtk_editable_get_text(GTK_EDITABLE(c->eMin))); } catch (...) { req.minOrderValue = 0; }
        try { req.maxDiscountAmount = std::stod(gtk_editable_get_text(GTK_EDITABLE(c->eMax))); } catch (...) { req.maxDiscountAmount = 0; }
        req.expiryDate = gtk_editable_get_text(GTK_EDITABLE(c->eExp));
        try { req.maxUsage = std::stoi(gtk_editable_get_text(GTK_EDITABLE(c->eUsg))); } catch (...) { req.maxUsage = 100; }
        req.description = gtk_editable_get_text(GTK_EDITABLE(c->eD));

        OperationResult res = c->isNew ? c->s->controller->addDiscount(req) : c->s->controller->updateDiscount(req);
        if (res.success) {
            UIComponents::showSuccessDialog(c->s->shell ? c->s->shell->getWindow() : nullptr, "Thành công", res.message);
            c->s->showDiscountListView();
        } else {
            UIComponents::showErrorDialog(c->s->shell ? c->s->shell->getWindow() : nullptr, "Lỗi", res.message);
        }
    }), sctx, [](gpointer d, GClosure*) { delete static_cast<SaveDiscCtx*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(btnRow), btnSave);
    gtk_box_append(GTK_BOX(card), btnRow);

    gtk_box_append(GTK_BOX(content), card);
    gtk_box_append(GTK_BOX(discountContainerBox), content);
    UIComponents::setupDialogShortcuts(content, btnSave, btnCancel);
}

// ================= 7. QUẢN LÝ TÀI KHOẢN & PHÂN QUYỀN =================

void AdminScreens::buildAccountMgmtView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Quản trị tài khoản & Khóa người dùng", "Kiểm soát trạng thái hoạt động của toàn bộ tài khoản nhân viên và khách hàng.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo tên đăng nhập, họ tên hoặc mã liên kết...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->accountFilterKeyword = txt ? txt : "";
            self->refreshAccounts();
        }),
        this,
        nullptr,
        &accountCountBadge
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kAccountTableCols));

    accountListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), accountListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(accountMgmtView), content);
    refreshAccounts();
}

void AdminScreens::refreshAccounts() {
    if (!accountListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(accountListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(accountListBox), child);
        child = next;
    }

    std::vector<AccountItemViewModel> allList = controller->getAccounts();
    std::vector<AccountItemViewModel> list;
    for (const auto& a : allList) {
        if (!accountFilterKeyword.empty()) {
            bool match = Utf8Helper::containsCaseInsensitive(a.username, accountFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(a.linkedFullName, accountFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(a.linkedEntityId, accountFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(a.roleName, accountFilterKeyword);
            if (!match) continue;
        }
        list.push_back(a);
    }

    UIComponents::updateRecordCountBadge(accountCountBadge, list.size(), "tài khoản");

    if (list.empty()) {
        gtk_box_append(GTK_BOX(accountListBox), UIComponents::createEmptyState(
            "🔐", "Không tìm thấy tài khoản",
            accountFilterKeyword.empty() ? "Không tìm thấy tài khoản nào trong hệ thống." : "Thử tìm kiếm với từ khóa khác."));
        return;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& a = list[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Tên đăng nhập
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            a.username, kAccountTableCols[0], "cell-code"));

        // 1: Họ và tên
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            a.linkedFullName, kAccountTableCols[1], "cell-title"));

        // 2: Mã liên kết
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            a.linkedEntityId, kAccountTableCols[2], "cell-subtitle"));

        // 3: Vai trò
        GtkWidget* roleBadge = UIComponents::createStatusBadge(a.roleName, "badge-confirmed");
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(roleBadge, kAccountTableCols[3]));

        // 4: Trạng thái
        GtkWidget* statusBadge = UIComponents::createStatusBadge(a.isActive ? "Đang hoạt động" : "Đã bị khóa",
                                                                 a.isActive ? "badge-completed" : "badge-cancelled");
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(statusBadge, kAccountTableCols[4]));

        // 5: Thao tác
        GtkWidget* btnToggle = UIComponents::createTableActionButton(
            a.isActive ? "🔒 Khóa" : "🔓 Mở khóa",
            a.isActive ? "Khóa tài khoản này để ngừng truy cập" : "Mở khóa tài khoản này",
            a.isActive ? "btn-action-delete" : "btn-action-view"
        );

        struct AccClickData { AdminScreens* s; std::string u; };
        AccClickData* acd = new AccClickData{this, a.username};
        g_signal_connect_data(btnToggle, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AccClickData* data = static_cast<AccClickData*>(d);
            OperationResult res = data->s->controller->toggleAccountStatus(data->u);
            if (res.success) {
                data->s->refreshAccounts();
                UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
            } else {
                UIComponents::showErrorDialog(nullptr, "Lỗi", res.message);
            }
        }), acd, [](gpointer d, GClosure*) { delete static_cast<AccClickData*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(btnToggle, kAccountTableCols[5]));
        gtk_box_append(GTK_BOX(accountListBox), row);
    }
}

// ================= 8. QUẢN LÝ ĐƠN HÀNG (LỌC & TÌM KIẾM ĐA TIÊU CHÍ) =================

void AdminScreens::buildAllOrdersView() {
    orderContainerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(allOrdersView), orderContainerBox);
    showOrderListView();
}

void AdminScreens::showOrderListView() {
    ordersListBox = nullptr;
    ordersCountBadge = nullptr;
    GtkWidget* child = gtk_widget_get_first_child(orderContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(orderContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Quản lý toàn bộ đơn hàng", "Lọc theo trạng thái xử lý, trạng thái thanh toán và tìm kiếm từ khóa/mã đơn hàng.");
    gtk_box_append(GTK_BOX(content), header);

    // Extra Controls for Order Status & Payment Status
    GtkWidget* filterRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);

    const char* statusOptions[] = {
        "Tất cả trạng thái đơn", "Chờ xử lý (Pending)", "Đã xác nhận (Confirmed)",
        "Đang đóng gói (Preparing)", "Sẵn sàng nhận (Ready)", "Đang giao hàng (Shipping)",
        "Hoàn thành (Completed)", "Đã hủy (Cancelled)", nullptr
    };
    GtkWidget* dropStatus = gtk_drop_down_new_from_strings(statusOptions);
    if (orderFilterStatus >= 0) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(dropStatus), static_cast<guint>(orderFilterStatus + 1));
    }
    g_signal_connect_swapped(dropStatus, "notify::selected", G_CALLBACK(+[](AdminScreens* s, GParamSpec*, GtkDropDown* dd) {
        guint sel = gtk_drop_down_get_selected(dd);
        s->orderFilterStatus = (sel == 0) ? -1 : static_cast<int>(sel - 1);
        s->refreshOrders();
    }), this);

    const char* paymentOptions[] = {"Tất cả thanh toán", "Chưa thanh toán (Unpaid)", "Đã thanh toán (Paid)", nullptr};
    GtkWidget* dropPayment = gtk_drop_down_new_from_strings(paymentOptions);
    if (orderFilterPayment >= 0) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(dropPayment), static_cast<guint>(orderFilterPayment + 1));
    }
    g_signal_connect_swapped(dropPayment, "notify::selected", G_CALLBACK(+[](AdminScreens* s, GParamSpec*, GtkDropDown* dd) {
        guint sel = gtk_drop_down_get_selected(dd);
        s->orderFilterPayment = (sel == 0) ? -1 : (sel == 1 ? 0 : 1);
        s->refreshOrders();
    }), this);

    gtk_box_append(GTK_BOX(filterRow), dropStatus);
    gtk_box_append(GTK_BOX(filterRow), dropPayment);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo Mã đơn hàng (ORD-...), Tên khách hàng hoặc Số điện thoại...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->orderFilterKeyword = txt ? txt : "";
            self->refreshOrders();
        }),
        this,
        nullptr,
        &ordersCountBadge,
        filterRow
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kAdminOrderTableCols));

    ordersListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), ordersListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_box_append(GTK_BOX(orderContainerBox), content);
    refreshOrders();
}

void AdminScreens::refreshOrders() {
    if (!ordersListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(ordersListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(ordersListBox), child);
        child = next;
    }

    std::vector<OrderItemViewModel> list = controller->filterOrders(orderFilterKeyword, orderFilterStatus, orderFilterPayment, -1);
    UIComponents::updateRecordCountBadge(ordersCountBadge, list.size(), "đơn hàng");

    if (list.empty()) {
        gtk_box_append(GTK_BOX(ordersListBox), UIComponents::createEmptyState("📦", "Chưa có dữ liệu", "Không có đơn hàng nào khớp bộ lọc."));
        return;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& ord = list[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Mã đơn
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            ord.id, kAdminOrderTableCols[0], "cell-code"));

        // 1: Khách hàng / SĐT
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
            ord.customerName, "SĐT: " + ord.receiverPhone, kAdminOrderTableCols[1]));

        // 2: Thời gian
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            ord.orderTime, kAdminOrderTableCols[2], "tabular-num"));

        // 3: Giao nhận
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            ord.deliveryMethodText, kAdminOrderTableCols[3], "cell-subtitle"));

        // 4: Trạng thái đơn (Badge)
        GtkWidget* badgeStatus = UIComponents::createOrderStatusBadge(ord.status);
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(badgeStatus, kAdminOrderTableCols[4]));

        // 5: Thanh toán (Badge)
        GtkWidget* badgePay = UIComponents::createPaymentStatusBadge(ord.paymentStatus);
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(badgePay, kAdminOrderTableCols[5]));

        // 6: Tổng tiền (Căn phải)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(ord.total), kAdminOrderTableCols[6], "product-price"));

        // 7: Thao tác
        GtkWidget* btnDetail = UIComponents::createTableActionButton("🔍 Chi tiết", "Xem thông tin chi tiết và xử lý đơn hàng", "btn-action-view");
        struct OrdClickData { AdminScreens* s; OrderItemViewModel o; };
        OrdClickData* ocd = new OrdClickData{this, ord};
        g_signal_connect_data(btnDetail, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            OrdClickData* data = static_cast<OrdClickData*>(d);
            data->s->showOrderDetailView(data->o);
        }), ocd, [](gpointer d, GClosure*) { delete static_cast<OrdClickData*>(d); }, (GConnectFlags)0);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(btnDetail, kAdminOrderTableCols[7]));
        gtk_box_append(GTK_BOX(ordersListBox), row);
    }
}

void AdminScreens::showOrderDetailView(const OrderItemViewModel& ord) {
    ordersListBox = nullptr;
    ordersCountBadge = nullptr;
    GtkWidget* child = gtk_widget_get_first_child(orderContainerBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(orderContainerBox), child);
        child = next;
    }

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createBackHeader("Chi tiết đơn hàng " + ord.id, "Xem danh sách mặt hàng, trạng thái thanh toán và cập nhật tiến trình đơn.",
        G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AdminScreens* self = static_cast<AdminScreens*>(d);
            self->showOrderListView();
        }), this);
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(card, "card");

    // Header info row
    GtkWidget* top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget* bStatus = UIComponents::createOrderStatusBadge(ord.status);
    GtkWidget* bPay = UIComponents::createPaymentStatusBadge(ord.paymentStatus);
    GtkWidget* dateLbl = gtk_label_new(("Ngày đặt: " + ord.orderTime).c_str());
    gtk_widget_add_css_class(dateLbl, "stat-label");
    gtk_widget_add_css_class(dateLbl, "tabular-num");

    gtk_box_append(GTK_BOX(top), bStatus);
    gtk_box_append(GTK_BOX(top), bPay);
    gtk_box_append(GTK_BOX(top), dateLbl);
    gtk_box_append(GTK_BOX(card), top);

    // Customer info card
    GtkWidget* custBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    std::string cInfo = "Khách hàng: " + ord.customerName + " | SĐT: " + ord.receiverPhone +
                        " | Địa chỉ nhận: " + ord.deliveryAddress;
    GtkWidget* cLbl = gtk_label_new(cInfo.c_str());
    gtk_widget_add_css_class(cLbl, "product-name");
    gtk_label_set_xalign(GTK_LABEL(cLbl), 0.0f);
    gtk_box_append(GTK_BOX(custBox), cLbl);
    gtk_box_append(GTK_BOX(card), custBox);

    // Items table
    GtkWidget* itemsTable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(itemsTable, "table-container");
    gtk_box_append(GTK_BOX(itemsTable), UIComponents::createTableHeaderRow(kOrderDetailItemsCols));

    for (size_t i = 0; i < ord.details.size(); ++i) {
        const auto& item = ord.details[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            item.phoneName, kOrderDetailItemsCols[0], "cell-title"));
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatNumber(item.quantity) + " máy", kOrderDetailItemsCols[1], "cell-number"));
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(item.unitPrice), kOrderDetailItemsCols[2], "tabular-num"));
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(item.subtotal), kOrderDetailItemsCols[3], "product-price"));

        gtk_box_append(GTK_BOX(itemsTable), row);
    }
    gtk_box_append(GTK_BOX(card), itemsTable);

    // Total summary
    GtkWidget* sumBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_halign(sumBox, GTK_ALIGN_END);
    gtk_widget_set_margin_top(sumBox, 10);

    GtkWidget* subTotLbl = gtk_label_new(("Tạm tính: " + UIComponents::formatVND(ord.subtotal)).c_str());
    gtk_label_set_xalign(GTK_LABEL(subTotLbl), 1.0f);
    gtk_box_append(GTK_BOX(sumBox), subTotLbl);

    if (ord.discountAmount > 0) {
        GtkWidget* discLbl = gtk_label_new(("Giảm giá voucher: -" + UIComponents::formatVND(ord.discountAmount)).c_str());
        gtk_widget_add_css_class(discLbl, "stat-trend");
        gtk_label_set_xalign(GTK_LABEL(discLbl), 1.0f);
        gtk_box_append(GTK_BOX(sumBox), discLbl);
    }

    GtkWidget* finalTotLbl = gtk_label_new(("TỔNG CỘNG THANH TOÁN: " + UIComponents::formatVND(ord.total)).c_str());
    gtk_widget_add_css_class(finalTotLbl, "stat-value");
    gtk_label_set_xalign(GTK_LABEL(finalTotLbl), 1.0f);
    gtk_box_append(GTK_BOX(sumBox), finalTotLbl);
    gtk_box_append(GTK_BOX(card), sumBox);

    // Status update actions
    GtkWidget* actBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(actBox, GTK_ALIGN_END);
    gtk_widget_set_margin_top(actBox, 16);

    auto addStatusBtn = [this, ord, actBox](const char* label, OrderStatus targetStatus, const char* cssClass) {
        GtkWidget* btn = gtk_button_new_with_label(label);
        gtk_widget_add_css_class(btn, cssClass);
        UIComponents::setButtonCursor(btn);
        struct OrdStatusCtx { AdminScreens* s; std::string ordId; OrderStatus st; };
        OrdStatusCtx* ctx = new OrdStatusCtx{this, ord.id, targetStatus};
        g_signal_connect_data(btn, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            OrdStatusCtx* c = static_cast<OrdStatusCtx*>(d);
            if (c->st == OrderStatus::CANCELLED) {
                UIComponents::showConfirmDialog(
                    c->s->shell ? c->s->shell->getWindow() : nullptr,
                    "Xác nhận hủy đơn hàng",
                    "Bạn có chắc chắn muốn hủy đơn hàng " + c->ordId + " không? Thao tác này sẽ hủy đơn và cập nhật trạng thái.",
                    +[](gpointer data) {
                        OrdStatusCtx* inner = static_cast<OrdStatusCtx*>(data);
                        OperationResult res = inner->s->controller->updateOrderStatus(inner->ordId, inner->st);
                        if (res.success) {
                            UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
                            inner->s->showOrderListView();
                        } else {
                            UIComponents::showErrorDialog(nullptr, "Lỗi", res.message);
                        }
                    }, c);
                return;
            }
            OperationResult res = c->s->controller->updateOrderStatus(c->ordId, c->st);
            if (res.success) {
                UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
                c->s->showOrderListView();
            } else {
                UIComponents::showErrorDialog(nullptr, "Lỗi", res.message);
            }
        }), ctx, [](gpointer d, GClosure*) { delete static_cast<OrdStatusCtx*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(actBox), btn);
    };

    if (ord.status == OrderStatus::PENDING) {
        addStatusBtn("✅ Xác nhận đơn", OrderStatus::CONFIRMED, "btn-primary");
        addStatusBtn("❌ Hủy đơn", OrderStatus::CANCELLED, "btn-danger");
    } else if (ord.status == OrderStatus::CONFIRMED || ord.status == OrderStatus::PREPARING) {
        addStatusBtn("🚚 Giao hàng", OrderStatus::SHIPPING, "btn-primary");
        addStatusBtn("❌ Hủy đơn", OrderStatus::CANCELLED, "btn-danger");
    } else if (ord.status == OrderStatus::SHIPPING) {
        addStatusBtn("🎉 Hoàn tất giao & Thu tiền", OrderStatus::COMPLETED, "btn-success");
        addStatusBtn("❌ Hủy đơn", OrderStatus::CANCELLED, "btn-danger");
    }

    gtk_box_append(GTK_BOX(card), actBox);

    gtk_box_append(GTK_BOX(content), card);
    gtk_box_append(GTK_BOX(orderContainerBox), content);
}

// ================= 9. QUẢN LÝ HÓA ĐƠN =================

void AdminScreens::buildAllInvoicesView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Danh sách hóa đơn bán hàng", "Tra cứu chứng từ kế toán, doanh thu bán lẻ tại quầy và đơn hàng giao online.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo mã hóa đơn, mã đơn hàng, tên khách hàng hoặc thu ngân...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->invoiceFilterKeyword = txt ? txt : "";
            self->refreshInvoices();
        }),
        this,
        nullptr,
        &invoiceCountBadge
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kAdminInvoiceTableCols));

    invoiceListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), invoiceListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(allInvoicesView), content);
    refreshInvoices();
}

void AdminScreens::refreshInvoices() {
    if (!invoiceListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(invoiceListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(invoiceListBox), child);
        child = next;
    }

    std::vector<InvoiceItemViewModel> allList = controller->getAllInvoices();
    std::vector<InvoiceItemViewModel> list;
    for (const auto& inv : allList) {
        if (!invoiceFilterKeyword.empty()) {
            bool match = Utf8Helper::containsCaseInsensitive(inv.id, invoiceFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(inv.orderId, invoiceFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(inv.customerName, invoiceFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(inv.employeeName, invoiceFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(inv.paymentMethodText, invoiceFilterKeyword);
            if (!match) continue;
        }
        list.push_back(inv);
    }

    UIComponents::updateRecordCountBadge(invoiceCountBadge, list.size(), "hóa đơn");

    if (list.empty()) {
        gtk_box_append(GTK_BOX(invoiceListBox), UIComponents::createEmptyState(
            "🧾",
            invoiceFilterKeyword.empty() ? "Chưa có dữ liệu" : "Không tìm thấy hóa đơn phù hợp",
            invoiceFilterKeyword.empty() ? "Chưa có hóa đơn bán hàng nào trong hệ thống." : "Thử tìm kiếm với mã hóa đơn hoặc tên khách hàng khác."));
        return;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& inv = list[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Mã hóa đơn
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            inv.id, kAdminInvoiceTableCols[0], "cell-code"));

        // 1: Mã đơn hàng
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            inv.orderId, kAdminInvoiceTableCols[1], "cell-code"));

        // 2: Khách hàng
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            inv.customerName, kAdminInvoiceTableCols[2], "cell-title"));

        // 3: Thu ngân
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            inv.employeeName, kAdminInvoiceTableCols[3], "cell-subtitle"));

        // 4: Phương thức (Badge)
        GtkWidget* payBadge = UIComponents::createStatusBadge(inv.paymentMethodText, "badge-confirmed");
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(payBadge, kAdminInvoiceTableCols[4]));

        // 5: Thời gian
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            inv.createTime, kAdminInvoiceTableCols[5], "tabular-num"));

        // 6: Tổng tiền (Căn phải)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            UIComponents::formatVND(inv.total), kAdminInvoiceTableCols[6], "product-price"));

        gtk_box_append(GTK_BOX(invoiceListBox), row);
    }
}

// ================= 10. GIÁM SÁT BẢO HÀNH =================

void AdminScreens::buildWarrantyMonitorView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Giám sát dịch vụ bảo hành", "Theo dõi tiến độ tiếp nhận, xử lý lỗi kỹ thuật thiết bị điện thoại.");
    gtk_box_append(GTK_BOX(content), header);

    GtkWidget* filterToolbar = UIComponents::createFilterToolbar(
        "Tìm theo mã phiếu, thiết bị, tên khách hàng hoặc mô tả sự cố...",
        G_CALLBACK(+[](GtkEditable* e, gpointer d) {
            AdminScreens* self = static_cast<AdminScreens*>(d);
            const char* txt = gtk_editable_get_text(e);
            self->warrantyFilterKeyword = txt ? txt : "";
            self->refreshWarrantyMonitor();
        }),
        this,
        nullptr,
        &warrantyCountBadge
    );
    gtk_box_append(GTK_BOX(content), filterToolbar);

    GtkWidget* tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(tableContainer, "table-container");
    gtk_box_append(GTK_BOX(tableContainer), UIComponents::createTableHeaderRow(kAdminWarrantyTableCols));

    warrantyListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(tableContainer), warrantyListBox);
    gtk_box_append(GTK_BOX(content), tableContainer);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(warrantyMonitorView), content);
    refreshWarrantyMonitor();
}

void AdminScreens::refreshWarrantyMonitor() {
    if (!warrantyListBox) return;
    GtkWidget* child = gtk_widget_get_first_child(warrantyListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(warrantyListBox), child);
        child = next;
    }

    std::vector<WarrantyItemViewModel> allList = controller->getWarranties();
    std::vector<WarrantyItemViewModel> list;
    for (const auto& w : allList) {
        if (!warrantyFilterKeyword.empty()) {
            bool match = Utf8Helper::containsCaseInsensitive(w.id, warrantyFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(w.phoneName, warrantyFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(w.customerName, warrantyFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(w.customerPhone, warrantyFilterKeyword) ||
                         Utf8Helper::containsCaseInsensitive(w.issueDescription, warrantyFilterKeyword);
            if (!match) continue;
        }
        list.push_back(w);
    }

    UIComponents::updateRecordCountBadge(warrantyCountBadge, list.size(), "phiếu bảo hành");

    if (list.empty()) {
        gtk_box_append(GTK_BOX(warrantyListBox), UIComponents::createEmptyState(
            "🛡️",
            warrantyFilterKeyword.empty() ? "Chưa có dữ liệu" : "Không tìm thấy phiếu bảo hành phù hợp",
            warrantyFilterKeyword.empty() ? "Chưa có yêu cầu hoặc biên nhận bảo hành nào trong hệ thống." : "Thử tìm kiếm với từ khóa hoặc mã phiếu khác."));
        return;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& w = list[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        // 0: Mã phiếu
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            w.id, kAdminWarrantyTableCols[0], "cell-code"));

        // 1: Thiết bị
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            w.phoneName, kAdminWarrantyTableCols[1], "cell-title"));

        // 2: Khách hàng (tên + SĐT)
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
            w.customerName, w.customerPhone, kAdminWarrantyTableCols[2]));

        // 3: Ngày hẹn
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            w.requestDate, kAdminWarrantyTableCols[3], "tabular-num"));

        // 4: Trạng thái (Badge)
        GtkWidget* badge = UIComponents::createWarrantyStatusBadge(w.status);
        gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(badge, kAdminWarrantyTableCols[4]));

        // 5: Mô tả sự cố
        gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
            w.issueDescription, kAdminWarrantyTableCols[5], "cell-subtitle"));

        gtk_box_append(GTK_BOX(warrantyListBox), row);
    }
}

// ================= 11. BÁO CÁO THỐNG KÊ DOANH THU & LỢI NHUẬN =================

void AdminScreens::buildReportsView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Báo cáo Thống kê & Phân tích Lợi nhuận", "Phân tích doanh số bán hàng, mặt hàng bán chạy và hiệu quả kinh doanh cửa hàng.");
    gtk_box_append(GTK_BOX(content), header);

    // Thanh lọc báo cáo theo khoảng thời gian
    GtkWidget* filterBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
    gtk_widget_add_css_class(filterBar, "card");

    GtkWidget* eFrom = gtk_entry_new();
    gtk_widget_set_name(eFrom, "report_from_date_entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(eFrom), "DD/MM/YYYY");
    gtk_editable_set_text(GTK_EDITABLE(eFrom), reportFromDate.c_str());
    GtkWidget* fromField = UIComponents::createFormField("Từ ngày", eFrom);
    gtk_box_append(GTK_BOX(filterBar), fromField);

    GtkWidget* eTo = gtk_entry_new();
    gtk_widget_set_name(eTo, "report_to_date_entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(eTo), "DD/MM/YYYY");
    gtk_editable_set_text(GTK_EDITABLE(eTo), reportToDate.c_str());
    GtkWidget* toField = UIComponents::createFormField("Đến ngày", eTo);
    gtk_box_append(GTK_BOX(filterBar), toField);

    struct ReportFilterCtx {
        AdminScreens* s;
        GtkWidget* eFrom;
        GtkWidget* eTo;
    };
    ReportFilterCtx* fctx = new ReportFilterCtx{this, eFrom, eTo};

    GtkWidget* btnFilter = gtk_button_new_with_label("🔍 Lọc báo cáo");
    gtk_widget_add_css_class(btnFilter, "btn-primary");
    gtk_widget_set_valign(btnFilter, GTK_ALIGN_END);
    UIComponents::setButtonCursor(btnFilter);
    g_signal_connect_data(btnFilter, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
        ReportFilterCtx* c = static_cast<ReportFilterCtx*>(d);
        auto trimText = [](const char* raw) {
            std::string t = raw ? raw : "";
            size_t b = t.find_first_not_of(" \t");
            if (b == std::string::npos) return std::string();
            size_t e = t.find_last_not_of(" \t");
            return t.substr(b, e - b + 1);
        };
        std::string from = trimText(gtk_editable_get_text(GTK_EDITABLE(c->eFrom)));
        std::string to = trimText(gtk_editable_get_text(GTK_EDITABLE(c->eTo)));
        AdminScreens* screens = c->s;
        OperationResult res = screens->controller->validateReportRange(from, to);
        if (!res.success) {
            UIComponents::showErrorDialog(screens->shell->getWindow(), "Khoảng thời gian không hợp lệ", res.message);
            return;
        }
        screens->reportFromDate = from;
        screens->reportToDate = to;
        screens->refreshReports();
    }), fctx, [](gpointer d, GClosure*) { delete static_cast<ReportFilterCtx*>(d); }, (GConnectFlags)0);
    gtk_box_append(GTK_BOX(filterBar), btnFilter);

    GtkWidget* btnResetFilter = gtk_button_new_with_label("↺ Đặt lại");
    gtk_widget_add_css_class(btnResetFilter, "btn-secondary");
    gtk_widget_set_valign(btnResetFilter, GTK_ALIGN_END);
    UIComponents::setButtonCursor(btnResetFilter);
    g_signal_connect_swapped(btnResetFilter, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        s->reportFromDate.clear();
        s->reportToDate.clear();
        s->refreshReports();
    }), this);
    gtk_box_append(GTK_BOX(filterBar), btnResetFilter);
    gtk_box_append(GTK_BOX(content), filterBar);

    std::string rangeText = "Đang xem: ";
    if (reportFromDate.empty() && reportToDate.empty()) {
        rangeText += "Toàn thời gian";
    } else {
        rangeText += "từ " + (reportFromDate.empty() ? std::string("đầu kỳ") : reportFromDate) +
                     " đến " + (reportToDate.empty() ? std::string("hiện tại") : reportToDate);
    }
    GtkWidget* lblRange = gtk_label_new(rangeText.c_str());
    gtk_widget_add_css_class(lblRange, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblRange), 0.0f);
    gtk_box_append(GTK_BOX(content), lblRange);

    DashboardStatsViewModel stats = controller->getDashboardStats(reportFromDate, reportToDate);

    // Summary KPIs
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::KPI_GRID_SPACING);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    GtkWidget* c1 = UIComponents::createStatCard("Tổng doanh thu thực tế", UIComponents::formatVND(stats.totalRevenue), "Đơn đã giao thành công", "💰", "green");
    GtkWidget* c2 = UIComponents::createStatCard("Lợi nhuận gộp ước tính", UIComponents::formatVND(stats.estimatedProfit), "Doanh thu − Giá vốn", "📈", "purple");
    GtkWidget* c3 = UIComponents::createStatCard("Tỷ suất lợi nhuận gộp", (stats.totalRevenue > 0 ? (std::to_string(static_cast<int>((stats.estimatedProfit / stats.totalRevenue) * 100)) + "%") : "0%"), "Biên lợi nhuận gộp", "📊", "blue");

    gtk_grid_attach(GTK_GRID(grid), c1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c3, 2, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), grid);

    // Top Selling Products Table Card
    GtkWidget* topCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
    gtk_widget_add_css_class(topCard, "card");

    GtkWidget* lblTopHeader = gtk_label_new("🏆 Top sản phẩm bán chạy nhất");
    gtk_widget_add_css_class(lblTopHeader, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblTopHeader), 0.0f);
    gtk_box_append(GTK_BOX(topCard), lblTopHeader);

    GtkWidget* topTable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(topTable, "table-container");
    gtk_box_append(GTK_BOX(topTable), UIComponents::createTableHeaderRow(kReportTopProdCols));

    if (stats.topProducts.empty()) {
        gtk_box_append(GTK_BOX(topTable), UIComponents::createEmptyState(
            "📊", "Chưa có dữ liệu doanh số", "Không có sản phẩm bán ra trong khoảng thời gian đã chọn."));
    } else {
        for (size_t i = 0; i < stats.topProducts.size(); ++i) {
            const auto& item = stats.topProducts[i];
            GtkWidget* row = UIComponents::createTableDataRow(i);

            gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                item.phoneName, kReportTopProdCols[0], "cell-title"));
            gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                item.brand, kReportTopProdCols[1], "cell-subtitle"));
            gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                UIComponents::formatNumber(item.soldQuantity) + " máy", kReportTopProdCols[2], "cell-number"));
            gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                UIComponents::formatVND(item.revenue), kReportTopProdCols[3], "product-price"));
            gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                UIComponents::formatVND(item.profit), kReportTopProdCols[4], "stock-ok"));

            gtk_box_append(GTK_BOX(topTable), row);
        }
    }

    gtk_box_append(GTK_BOX(topCard), topTable);
    gtk_box_append(GTK_BOX(content), topCard);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(reportsView), content);
}

void AdminScreens::refreshReports() {
    buildReportsView();
}

// ================= 12. HỆ THỐNG KIỂM SOÁT THỜI GIAN THỰC (REAL-TIME SIMULATION) =================

void AdminScreens::buildRealTimeView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader(
        "⏱️ Kiểm soát & Quản lý Thời gian thực Hệ thống",
        "Tùy chỉnh ngày giờ mô phỏng (lưu tại data/realTime.txt) để đối soát hạn Voucher, đơn hàng và bảo hành."
    );
    gtk_box_append(GTK_BOX(content), header);

    // Large Digital Clock Banner
    GtkWidget* clockCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(clockCard, "realtime-clock-box");

    GtkWidget* lblClockTitle = gtk_label_new("MỐC THỜI GIAN THỰC HIỆN TẠI (SIMULATED REALTIME)");
    gtk_widget_add_css_class(lblClockTitle, "stat-label");
    gtk_label_set_xalign(GTK_LABEL(lblClockTitle), 0.0f);
    gtk_box_append(GTK_BOX(clockCard), lblClockTitle);

    std::string currentDt = controller->getSimulatedTime();
    GtkWidget* lblClockTime = gtk_label_new(currentDt.c_str());
    gtk_widget_add_css_class(lblClockTime, "realtime-clock-display");
    gtk_widget_add_css_class(lblClockTime, "tabular-num");
    gtk_label_set_xalign(GTK_LABEL(lblClockTime), 0.0f);
    gtk_box_append(GTK_BOX(clockCard), lblClockTime);

    GtkWidget* lblClockPath = gtk_label_new("Đang đồng bộ hóa trực tiếp với file: data/realTime.txt");
    gtk_widget_add_css_class(lblClockPath, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblClockPath), 0.0f);
    gtk_box_append(GTK_BOX(clockCard), lblClockPath);

    gtk_box_append(GTK_BOX(content), clockCard);

    // Control Adjustment Card
    GtkWidget* adjCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(adjCard, "card");

    GtkWidget* lblAdjTitle = gtk_label_new("⚙️ Điều chỉnh ngày giờ hệ thống");
    gtk_widget_add_css_class(lblAdjTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblAdjTitle), 0.0f);
    gtk_box_append(GTK_BOX(adjCard), lblAdjTitle);

    GtkWidget* rowInputs = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
    GtkWidget* eDate = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(eDate), controller->getSimulatedDate().c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(eDate), "YYYY-MM-DD (vd: 2026-10-17)");

    GtkWidget* eTime = gtk_entry_new();
    std::string timePart = (currentDt.length() >= 19) ? currentDt.substr(11, 8) : "11:30:00";
    gtk_editable_set_text(GTK_EDITABLE(eTime), timePart.c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(eTime), "HH:MM:SS (vd: 11:30:00)");

    gtk_box_append(GTK_BOX(rowInputs), UIComponents::createFormField("Ngày mô phỏng (YYYY-MM-DD) *", eDate));
    gtk_box_append(GTK_BOX(rowInputs), UIComponents::createFormField("Giờ mô phỏng (HH:MM:SS) *", eTime));

    struct TimeSaveCtx { AdminScreens* s; GtkWidget* eD; GtkWidget* eT; };
    TimeSaveCtx* tsctx = new TimeSaveCtx{this, eDate, eTime};

    GtkWidget* btnSaveTime = gtk_button_new_with_label("💾 Lưu mốc thời gian");
    gtk_widget_add_css_class(btnSaveTime, "btn-primary");
    UIComponents::setButtonCursor(btnSaveTime);
    gtk_widget_set_valign(btnSaveTime, GTK_ALIGN_END);
    g_signal_connect_data(btnSaveTime, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        TimeSaveCtx* c = static_cast<TimeSaveCtx*>(d);
        const char* dStr = gtk_editable_get_text(GTK_EDITABLE(c->eD));
        const char* tStr = gtk_editable_get_text(GTK_EDITABLE(c->eT));
        if (dStr && strlen(dStr) >= 10) {
            std::string full = std::string(dStr) + " " + (tStr && strlen(tStr) >= 5 ? tStr : "00:00:00");
            OperationResult res = c->s->controller->setSimulatedTime(full);
            UIComponents::showSuccessDialog(nullptr, "Đã lưu", res.message);
            c->s->refreshAll();
        }
    }), tsctx, [](gpointer d, GClosure*) { delete static_cast<TimeSaveCtx*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(rowInputs), btnSaveTime);
    gtk_box_append(GTK_BOX(adjCard), rowInputs);

    // Fast shift buttons
    GtkWidget* shiftRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* lblShift = gtk_label_new("Phím tắt tua thời gian:");
    gtk_widget_add_css_class(lblShift, "stat-label");
    gtk_box_append(GTK_BOX(shiftRow), lblShift);

    auto addShiftBtn = [this, shiftRow](const char* label, int days) {
        GtkWidget* btn = gtk_button_new_with_label(label);
        gtk_widget_add_css_class(btn, "btn-outline");
        UIComponents::setButtonCursor(btn);
        struct ShiftData { AdminScreens* s; int d; };
        ShiftData* sd = new ShiftData{this, days};
        g_signal_connect_data(btn, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            ShiftData* data = static_cast<ShiftData*>(d);
            data->s->controller->advanceSimulatedDays(data->d);
            data->s->refreshAll();
        }), sd, [](gpointer d, GClosure*) { delete static_cast<ShiftData*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(shiftRow), btn);
    };

    addShiftBtn("⏪ Tua lùi -7 ngày", -7);
    addShiftBtn("⏩ Tua tới +7 ngày", 7);
    addShiftBtn("⏩ Tua tới +30 ngày", 30);
    addShiftBtn("⏩ Tua tới +1 năm", 365);

    GtkWidget* btnResetNow = gtk_button_new_with_label("🔄 Đồng bộ theo giờ máy tính");
    gtk_widget_add_css_class(btnResetNow, "btn-secondary");
    UIComponents::setButtonCursor(btnResetNow);
    g_signal_connect_swapped(btnResetNow, "clicked", G_CALLBACK(+[](AdminScreens* s) {
        s->controller->resetSimulatedTimeToNow();
        s->refreshAll();
    }), this);
    gtk_box_append(GTK_BOX(shiftRow), btnResetNow);

    gtk_box_append(GTK_BOX(adjCard), shiftRow);
    gtk_box_append(GTK_BOX(content), adjCard);

    // Live Voucher Reconciliation Table
    GtkWidget* reconCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(reconCard, "card");

    GtkWidget* lblReconHeader = gtk_label_new("📋 Bảng đối soát hiệu lực Voucher theo mốc thời gian này");
    gtk_widget_add_css_class(lblReconHeader, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblReconHeader), 0.0f);
    gtk_box_append(GTK_BOX(reconCard), lblReconHeader);

    realTimeReconcileBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(realTimeReconcileBox, "table-container");
    gtk_box_append(GTK_BOX(reconCard), realTimeReconcileBox);
    gtk_box_append(GTK_BOX(content), reconCard);

    std::vector<DiscountItemViewModel> list = controller->getDiscounts();
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& d = list[i];
        GtkWidget* row = UIComponents::createTableDataRow(i);

        GtkWidget* codeBadge = UIComponents::createStatusBadge(d.code, "badge-confirmed");
        GtkWidget* descLbl = gtk_label_new(d.description.c_str());
        gtk_widget_add_css_class(descLbl, "cell-title");
        gtk_widget_set_hexpand(descLbl, TRUE);
        gtk_label_set_xalign(GTK_LABEL(descLbl), 0.0f);

        GtkWidget* expLbl = gtk_label_new(("HSD: " + d.expiryDate).c_str());
        gtk_widget_add_css_class(expLbl, "cell-subtitle");
        gtk_widget_add_css_class(expLbl, "tabular-num");

        int remDays = RealTimeManager::getInstance().getRemainingDays(d.expiryDate);
        std::string st = d.isValid ? ("🟢 Hợp lệ (còn " + std::to_string(remDays) + " ngày)")
                                   : ("🔴 Hết hạn (quá hạn " + std::to_string(std::abs(remDays)) + " ngày)");
        GtkWidget* badgeSt = UIComponents::createStatusBadge(st, d.isValid ? "badge-completed" : "badge-cancelled");

        gtk_box_append(GTK_BOX(row), codeBadge);
        gtk_box_append(GTK_BOX(row), descLbl);
        gtk_box_append(GTK_BOX(row), expLbl);
        gtk_box_append(GTK_BOX(row), badgeSt);

        gtk_box_append(GTK_BOX(realTimeReconcileBox), row);
    }

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(realTimeView), content);
}

void AdminScreens::refreshRealTime() {
    buildRealTimeView();
}
