#ifndef CUSTOMER_SCREENS_H
#define CUSTOMER_SCREENS_H

#include <gtk/gtk.h>
#include "../../source/controllers/AppController.h"

class MainShellWindow; // Forward declaration

/**
 * @file CustomerScreens.h
 * @brief ĐẶC TẢ CỔNG THÔNG TIN KHÁCH HÀNG (CUSTOMER PORTAL & E-COMMERCE EXPERIENCE)
 * 
 * =========================================================================================
 * 1. KIẾN TRÚC & PHẠM VI CHỨC NĂNG:
 *    - Cung cấp trải nghiệm mua sắm trực tuyến cho Khách hàng:
 *      1. Màn hình Tổng quan (Dashboard): Lời chào cá nhân hóa, điểm thưởng hiện có,
 *         thứ hạng thành viên (Bronze/Silver/Gold/Diamond), tổng chi tiêu và đơn hàng gần nhất.
 *      2. Danh mục Sản phẩm (Product Catalog): Tìm kiếm theo tên/hãng, lọc theo giá, hiển thị thẻ sản phẩm
 *         kèm trạng thái còn hàng/hết hàng và nút thêm vào giỏ.
 *      3. Giỏ hàng & Thanh toán (Cart & Checkout): Quản lý số lượng, xóa sản phẩm, nhập mã voucher,
 *         điền thông tin nhận hàng, chọn phương thức thanh toán và giao hàng.
 *      4. Lịch sử Đơn hàng (Order History): Theo dõi hành trình đơn (Pending -> Confirmed -> Shipping -> Completed),
 *         xem chi tiết từng mặt hàng và hủy đơn khi chưa giao.
 *      5. Hồ sơ Cá nhân (Profile): Cập nhật thông tin liên hệ, xem cấp bậc hội viên và chính sách ưu đãi tích điểm.
 *      6. Yêu cầu Bảo hành (Warranty Claims): Tra cứu các sản phẩm đã mua và đăng ký bảo hành trực tuyến.
 * =========================================================================================
 */
class CustomerScreens {
private:
    AppController* controller;
    MainShellWindow* shell;

    // View containers
    GtkWidget* dashboardView;
    GtkWidget* productsView;
    GtkWidget* cartView;
    GtkWidget* ordersView;
    GtkWidget* profileView;
    GtkWidget* warrantyView;

    // Dynamic containers for refresh
    GtkWidget* productsListBox;
    GtkWidget* cartListBox;
    GtkWidget* cartSummaryBox;
    GtkWidget* ordersListBox;
    GtkWidget* warrantyListBox;

    // Search & Filter state
    std::string currentSearchKeyword;
    std::string currentBrandFilter;
    std::string currentSortBy;

    // Cart / Checkout inputs
    GtkWidget* entryCoupon;
    GtkWidget* entryReceiverName;
    GtkWidget* entryReceiverPhone;
    GtkWidget* entryDeliveryAddress;
    GtkWidget* comboDeliveryMethod;
    GtkWidget* comboPaymentMethod;

public:
    CustomerScreens(AppController* ctrl, MainShellWindow* parentShell);

    GtkWidget* getDashboardView() { return dashboardView; }
    GtkWidget* getProductsView() { return productsView; }
    GtkWidget* getCartView() { return cartView; }
    GtkWidget* getOrdersView() { return ordersView; }
    GtkWidget* getProfileView() { return profileView; }
    GtkWidget* getWarrantyView() { return warrantyView; }

    void refreshAll();
    void refreshDashboard();
    void refreshProducts();
    void refreshCart();
    void refreshOrders();
    void refreshProfile();
    void refreshWarranty();

private:
    void buildDashboardView();
    void buildProductsView();
    void buildCartView();
    void buildOrdersView();
    void buildProfileView();
    void buildWarrantyView();

    void showProductDetailModal(const ProductItemViewModel& product);
    void showOrderDetailModal(const OrderItemViewModel& order);
    void showCreateWarrantyModal();
};

#endif // CUSTOMER_SCREENS_H
