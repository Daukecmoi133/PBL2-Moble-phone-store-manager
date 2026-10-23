#ifndef WAREHOUSE_SCREENS_H
#define WAREHOUSE_SCREENS_H

#include <gtk/gtk.h>
#include <vector>
#include "../../controllers/AppController.h"

class MainShellWindow;

/**
 * @file WarehouseScreens.h
 * @brief ĐẶC TẢ HỆ THỐNG MÀN HÌNH QUẢN LÝ KHO HÀNG (WAREHOUSE MANAGEMENT SUBSYSTEM)
 * 
 * =========================================================================================
 * 1. KIẾN TRÚC & PHẠM VI CHỨC NĂNG:
 *    - Cung cấp không gian làm việc chuyên biệt cho Nhân viên Quản lý Kho (Warehouse Staff):
 *      1. Bàn làm việc (Dashboard): Theo dõi tổng giá trị lưu kho, số mặt hàng sắp hết, đơn chờ đóng gói.
 *      2. Danh mục Tồn kho (Inventory View): Tra cứu tồn kho theo hãng, mã sản phẩm, số lượng tồn thực tế.
 *      3. Cảnh báo Tồn thấp (Low Stock Alerts): Danh sách sản phẩm có số lượng <= ngưỡng an toàn, hỗ trợ lên kế hoạch nhập hàng.
 *      4. Lịch sử Nhập hàng (Goods Receipts): Bảng kê toàn bộ phiếu nhập kho từ các nhà cung cấp.
 *      5. Tạo Phiếu Nhập hàng (Create Receipt Draft): Chọn nhà cung cấp, thêm danh sách chi tiết nhiều sản phẩm,
 *         tính tổng tiền dự kiến và ghi nhận nhập kho nguyên tử.
 *      6. Hàng đợi Đóng gói & Xuất kho (Fulfillment Queue): Tiếp nhận đơn hàng CONFIRMED/PREPARING,
 *         xác nhận đóng gói xong và chuyển trạng thái sang SHIPPING.
 * =========================================================================================
 */
class WarehouseScreens {
private:
    AppController* controller;
    MainShellWindow* shell;

    // View containers
    GtkWidget* dashboardView;
    GtkWidget* inventoryView;
    GtkWidget* lowStockView;
    GtkWidget* receiptsView;
    GtkWidget* createReceiptView;
    GtkWidget* fulfillmentView;

    // Dynamic containers
    GtkWidget* inventoryListBox;
    GtkWidget* lowStockListBox;
    GtkWidget* receiptsListBox;
    GtkWidget* fulfillmentListBox;

    // Create receipt draft state
    std::string draftSupplierId;
    std::vector<GoodsReceiptDetailViewModel> draftReceiptItems;
    GtkWidget* receiptDraftItemsBox;
    GtkWidget* receiptComboSupplier;
    GtkWidget* lblReceiptEstimatedTotal;

public:
    WarehouseScreens(AppController* ctrl, MainShellWindow* parentShell);

    GtkWidget* getDashboardView() { return dashboardView; }
    GtkWidget* getInventoryView() { return inventoryView; }
    GtkWidget* getLowStockView() { return lowStockView; }
    GtkWidget* getReceiptsView() { return receiptsView; }
    GtkWidget* getCreateReceiptView() { return createReceiptView; }
    GtkWidget* getFulfillmentView() { return fulfillmentView; }

    void refreshAll();
    void refreshDashboard();
    void refreshInventory(const std::string& keyword = "", const std::string& brand = "Tất cả");
    void refreshLowStock();
    void refreshReceipts();
    void refreshFulfillmentQueue();

private:
    void buildDashboardView();
    void buildInventoryView();
    void buildLowStockView();
    void buildReceiptsView();
    void buildCreateReceiptView();
    void buildFulfillmentView();

    void showReceiptDetailModal(const GoodsReceiptSummaryViewModel& receipt);
    void confirmCancelReceipt(const std::string& receiptId);
    void showAddProductToReceiptModal();
};

#endif // WAREHOUSE_SCREENS_H
