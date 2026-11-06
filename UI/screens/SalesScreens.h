#ifndef SALES_SCREENS_H
#define SALES_SCREENS_H

#include <gtk/gtk.h>
#include <vector>
#include "../../source/controllers/AppController.h"

class MainShellWindow;

/**
 * @file SalesScreens.h
 * @brief ĐẶC TẢ HỆ THỐNG MÀN HÌNH NHÂN VIÊN BÁN HÀNG (SALES REPRESENTATIVE SUBSYSTEM)
 * 
 * =========================================================================================
 * 1. KIẾN TRÚC & PHẠM VI CHỨC NĂNG:
 *    - Cung cấp không gian làm việc chuyên biệt cho Nhân viên Bán hàng (Saler):
 *      1. Bàn làm việc (Dashboard): Thống kê doanh số cá nhân/trong ngày, đơn hàng cần xác nhận.
 *      2. Bán hàng tại quầy (Point-of-Sale - POS): Tạo đơn bán trực tiếp, chọn khách hàng,
 *         thêm sản phẩm vào giỏ, áp mã giảm giá, tính tiền thối và thanh toán.
 *      3. Tra cứu Sản phẩm (Catalog Lookup): Tìm kiếm thông số, mức giá, tồn kho khả dụng để tư vấn.
 *      4. Quản lý Khách hàng: Tạo hồ sơ khách hàng mới, tra cứu điểm tích lũy và thứ hạng.
 *      5. Hóa đơn bán hàng: Tra cứu, in lại phiếu thu, đối soát thông tin thanh toán.
 *      6. Đơn hàng Online: Tìm kiếm, xem chi tiết giỏ hàng/số lượng đơn đặt trước để hỗ trợ khách lấy tại quầy.
 *      7. Tiếp nhận Bảo hành: Tạo phiếu yêu cầu bảo hành/đổi trả cho khách hàng mua tại cửa hàng.
 * =========================================================================================
 */
class SalesScreens {
private:
    AppController* controller;
    MainShellWindow* shell;

    // View containers
    GtkWidget* dashboardView;
    GtkWidget* posView;
    GtkWidget* productLookupView;
    GtkWidget* customerMgmtView;
    GtkWidget* invoicesView;
    GtkWidget* onlineOrdersView;
    GtkWidget* warrantyIntakeView;

    // POS state
    std::string posSelectedCustomerId;
    std::vector<std::string> posCustomerIds; // Danh sách ID tương ứng 1-1 với items trong posComboCustomer
    std::vector<CartItemViewModel> posCartItems;
    std::string posAppliedCoupon;
    GtkWidget* posCartListBox = nullptr;
    GtkWidget* posCartCountBadge = nullptr;
    GtkWidget* posLblSubtotal = nullptr;
    GtkWidget* posLblDiscount = nullptr;
    GtkWidget* posLblGrandTotal = nullptr;
    GtkWidget* posEntryCash = nullptr;
    GtkWidget* posEntrySearch = nullptr;
    GtkWidget* posSearchPopover = nullptr;
    GtkWidget* posSearchListBox = nullptr;
    GtkWidget* posSearchStatusLbl = nullptr;
    GtkWidget* posBtnPay = nullptr;
    GtkWidget* posLblChange = nullptr;
    GtkWidget* posComboCustomer = nullptr;
    GtkWidget* posComboPayMethod = nullptr;
    GtkWidget* posEntryCoupon = nullptr;

    // Dynamic containers & record count badges
    GtkWidget* productLookupListBox = nullptr;
    GtkWidget* lookupCountBadge = nullptr;
    GtkWidget* customerListBox = nullptr;
    GtkWidget* customerCountBadge = nullptr;
    GtkWidget* invoiceListBox = nullptr;
    GtkWidget* invoiceCountBadge = nullptr;
    GtkWidget* ordersListBox = nullptr;
    GtkWidget* ordersCountBadge = nullptr;
    std::string onlineOrderKeyword;
    GtkWidget* warrantyListBox = nullptr;
    GtkWidget* warrantyCountBadge = nullptr;
    std::string warrantyKeyword;

public:
    SalesScreens(AppController* ctrl, MainShellWindow* parentShell);
    ~SalesScreens();

    GtkWidget* getDashboardView() { return dashboardView; }
    GtkWidget* getPosView() { return posView; }
    GtkWidget* getProductLookupView() { return productLookupView; }
    GtkWidget* getCustomerMgmtView() { return customerMgmtView; }
    GtkWidget* getInvoicesView() { return invoicesView; }
    GtkWidget* getOnlineOrdersView() { return onlineOrdersView; }
    GtkWidget* getWarrantyIntakeView() { return warrantyIntakeView; }

    void refreshAll();
    void refreshDashboard();
    void refreshPos();
    void refreshPosCustomers();
    bool addProductToPosCart(const ProductItemViewModel& prod);
    void updatePosSearchSuggestions(const std::string& keyword);
    void refreshProductLookup(const std::string& keyword = "");
    void refreshCustomers(const std::string& keyword = "");
    void refreshInvoices(const std::string& keyword = "");
    void refreshOnlineOrders();
    void refreshWarrantyIntake();
    void focusPosSearch();

    // Getters phục vụ kiểm thử và kiểm soát trạng thái POS
    const std::string& getPosSelectedCustomerId() const { return posSelectedCustomerId; }
    const std::vector<std::string>& getPosCustomerIds() const { return posCustomerIds; }
    const std::vector<CartItemViewModel>& getPosCartItems() const { return posCartItems; }

private:
    void buildDashboardView();
    void buildPosView();
    void buildProductLookupView();
    void buildCustomerMgmtView();
    void buildInvoicesView();
    void buildOnlineOrdersView();
    void buildWarrantyIntakeView();

    void showCustomerModal(const CustomerItemViewModel* existingCust = nullptr);
    void showCustomerDetailsModal(const CustomerItemViewModel& cust);
    void showInvoiceDetailModal(const InvoiceItemViewModel& inv);
    void showPosProductPickerModal();
};

#endif // SALES_SCREENS_H
