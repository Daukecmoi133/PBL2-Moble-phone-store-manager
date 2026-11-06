#ifndef STATISTICS_VIEWMODEL_H
#define STATISTICS_VIEWMODEL_H

#include <string>
#include <vector>

// Dòng sản phẩm bán chạy trong báo cáo
struct TopProductStatItem {
    std::string phoneId;
    std::string phoneName;
    std::string brand;
    int soldQuantity;
    double revenue;
    double profit;
};

// Dữ liệu thống kê tổng hợp cho Dashboard và trang Báo cáo
struct DashboardStatsViewModel {
    double totalRevenue;        // Tổng doanh thu thực tế (chỉ tính đơn COMPLETED / PAID)
    int totalOrders;           // Tổng số đơn
    int completedOrders;       // Đơn thành công
    int pendingOrders;         // Đơn đang chờ xử lý
    int shippingOrders;        // Đơn đang giao
    int cancelledOrders;       // Đơn đã hủy
    int totalProducts;         // Tổng mã sản phẩm
    int totalStockQuantity;    // Tổng số lượng máy trong kho
    int lowStockCount;         // Số sản phẩm sắp hết hàng (<= 5)
    int totalCustomers;        // Tổng số khách hàng
    int totalEmployees;        // Tổng số nhân viên
    double totalInventoryValue;// Giá trị tồn kho ước tính (nhập)
    double estimatedProfit;    // Lợi nhuận ước tính (doanh thu - giá vốn các đơn hoàn tất)
    std::vector<TopProductStatItem> topProducts;

    DashboardStatsViewModel()
        : totalRevenue(0.0), totalOrders(0), completedOrders(0),
          pendingOrders(0), shippingOrders(0), cancelledOrders(0),
          totalProducts(0), totalStockQuantity(0), lowStockCount(0),
          totalCustomers(0), totalEmployees(0), totalInventoryValue(0.0),
          estimatedProfit(0.0) {}
};

#endif // STATISTICS_VIEWMODEL_H
