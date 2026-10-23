#ifndef ADMIN_SCREENS_H
#define ADMIN_SCREENS_H

#include <gtk/gtk.h>
#include <string>
#include "../../controllers/AppController.h"

class MainShellWindow;

/**
 * @file AdminScreens.h
 * @brief ĐẶC TẢ HỆ THỐNG MÀN HÌNH QUẢN TRỊ VIÊN CẤP CAO (ADMINISTRATOR SUBSYSTEM)
 * 
 * =========================================================================================
 * 1. KIẾN TRÚC & PHẠM VI CHỨC NĂNG:
 *    - Quản lý toàn bộ 12 phân hệ tác vụ quản trị của cửa hàng điện thoại:
 *      1. Dashboard: Tổng quan chỉ số doanh thu, đơn hàng, khách hàng, tồn kho và cảnh báo.
 *      2. Quản lý Sản phẩm (Products): Xem danh sách, tìm kiếm, xem chi tiết / chỉnh sửa riêng biệt (có nút Back),
 *         thêm mới sản phẩm, hỗ trợ trường URL/đường dẫn ảnh sản phẩm.
 *      3. Quản lý Nhân viên (Employees): Danh sách, lọc theo quyền hạn (Admin, Sale, Kho), thêm/sửa, phân quyền.
 *      4. Quản lý Khách hàng (Customers): Danh sách khách hàng với HUY HIỆU MÀU ĐẶC TẢ THEO HẠNG (Rank),
 *         màn hình xem chi tiết lịch sử mua hàng và điểm thưởng.
 *      5. Quản lý Nhà cung cấp (Suppliers): Danh bạ, thông tin liên hệ, lịch sử hợp tác.
 *      6. Quản lý Khuyến mãi (Discounts): Danh sách voucher, chỉnh sửa ngày hiệu lực (startDate - endDate),
 *         đối soát hiệu lực theo thời gian thực (realtime reconciliation).
 *      7. Quản lý Tài khoản (Accounts): Kiểm soát thông tin đăng nhập, trạng thái khóa/mở.
 *      8. Quản lý Toàn bộ Đơn hàng (Orders): Lọc đa tiêu chí (từ khóa, trạng thái, thanh toán), xem chi tiết đơn.
 *      9. Quản lý Hóa đơn (Invoices): Bảng kê hóa đơn GTGT, kiểm tra mã đơn liên kết.
 *      10. Giám sát Bảo hành (Warranties): Theo dõi tiến độ bảo hành, sửa chữa, thay thế linh kiện.
 *      11. Báo cáo & Thống kê (Reports): Doanh thu, lợi nhuận, danh sách sản phẩm bán chạy, tồn kho thấp.
 *      12. Kiểm soát Thời gian thực (Real-time Clock Simulation): Màn hình chuyên biệt điều chỉnh
 *          thời gian hệ thống phục vụ kiểm thử đối soát voucher, bảo hành và đơn hàng.
 * 
 * 2. CƠ CHẾ ĐỔI MÀN HÌNH CON (SUBVIEW SWAPPING PATTERN):
 *    - Khắc phục triệt để yêu cầu phản hồi: chức năng Thêm / Sửa / Xem chi tiết không dùng popup dễ lỗi,
 *      mà chuyển thành subview riêng biệt trong ScrolledWindow, tích hợp nút 'Quay lại' (Back Header).
 * =========================================================================================
 */
class AdminScreens {
private:
    AppController* controller;
    MainShellWindow* shell;

    // View containers (ScrolledWindows permanently attached to Stack)
    GtkWidget* dashboardView;
    GtkWidget* productMgmtView;
    GtkWidget* employeeMgmtView;
    GtkWidget* customerMgmtView;
    GtkWidget* supplierMgmtView;
    GtkWidget* discountMgmtView;
    GtkWidget* accountMgmtView;
    GtkWidget* allOrdersView;
    GtkWidget* allInvoicesView;
    GtkWidget* warrantyMonitorView;
    GtkWidget* reportsView;
    GtkWidget* realTimeView;

    // Subview containers for swap between List & Detail/Edit screens
    GtkWidget* productContainerBox;
    GtkWidget* customerContainerBox;
    GtkWidget* discountContainerBox;
    GtkWidget* orderContainerBox;

    // Dynamic list boxes
    GtkWidget* productListBox;
    GtkWidget* employeeListBox;
    GtkWidget* customerListBox;
    GtkWidget* supplierListBox;
    GtkWidget* discountListBox;
    GtkWidget* accountListBox;
    GtkWidget* ordersListBox;
    GtkWidget* invoiceListBox;
    GtkWidget* warrantyListBox;
    GtkWidget* reportsListBox;
    GtkWidget* realTimeReconcileBox;

    // Order filters
    std::string orderFilterKeyword;
    int orderFilterStatus;
    int orderFilterPayment;

    // Bộ lọc báo cáo doanh thu theo khoảng ngày (DD/MM/YYYY, để trống = toàn thời gian)
    std::string reportFromDate;
    std::string reportToDate;

public:
    AdminScreens(AppController* ctrl, MainShellWindow* parentShell);

    GtkWidget* getDashboardView() { return dashboardView; }
    GtkWidget* getProductMgmtView() { return productMgmtView; }
    GtkWidget* getEmployeeMgmtView() { return employeeMgmtView; }
    GtkWidget* getCustomerMgmtView() { return customerMgmtView; }
    GtkWidget* getSupplierMgmtView() { return supplierMgmtView; }
    GtkWidget* getDiscountMgmtView() { return discountMgmtView; }
    GtkWidget* getAccountMgmtView() { return accountMgmtView; }
    GtkWidget* getAllOrdersView() { return allOrdersView; }
    GtkWidget* getAllInvoicesView() { return allInvoicesView; }
    GtkWidget* getWarrantyMonitorView() { return warrantyMonitorView; }
    GtkWidget* getReportsView() { return reportsView; }
    GtkWidget* getRealTimeView() { return realTimeView; }

    void refreshAll();
    void refreshDashboard();
    void refreshProducts(const std::string& keyword = "");
    void refreshEmployees(const std::string& keyword = "");
    void refreshCustomers(const std::string& keyword = "");
    void refreshSuppliers(const std::string& keyword = "");
    void refreshDiscounts();
    void refreshAccounts();
    void refreshOrders();
    void refreshInvoices();
    void confirmCancelInvoice(const std::string& invoiceId);
    void showInvoiceDetailModal(const InvoiceItemViewModel& inv);
    void refreshWarrantyMonitor();
    void refreshReports();
    void refreshRealTime();

    // Full-screen detail & edit views with Back buttons
    void showProductListView();
    void showProductDetailView(const ProductItemViewModel& prod, bool isNew = false);

    void showCustomerListView();
    void showCustomerDetailView(const CustomerItemViewModel& cust);

    void showDiscountListView();
    void showDiscountEditView(const DiscountItemViewModel& disc, bool isNew = false);

    void showOrderListView();
    void showOrderDetailView(const OrderItemViewModel& ord);

private:
    void buildDashboardView();
    void buildProductMgmtView();
    void buildEmployeeMgmtView();
    void buildCustomerMgmtView();
    void buildSupplierMgmtView();
    void buildDiscountMgmtView();
    void buildAccountMgmtView();
    void buildAllOrdersView();
    void buildAllInvoicesView();
    void buildWarrantyMonitorView();
    void buildReportsView();
    void buildRealTimeView();

    void showEmployeeFormModal(const EmployeeItemViewModel* existingEmp = nullptr);
    void showSupplierFormModal(const SupplierItemViewModel* existingSup = nullptr);
};

#endif // ADMIN_SCREENS_H
