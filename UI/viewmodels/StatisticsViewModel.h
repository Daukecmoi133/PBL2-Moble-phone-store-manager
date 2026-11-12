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

// Điểm dữ liệu biểu đồ Doanh thu & Lợi nhuận theo mốc thời gian (T13)
struct TimelineStatPoint {
    std::string label;       // Mốc hiển thị (dd/MM hoặc MM/YYYY)
    double revenue;          // Doanh thu trong mốc
    double profit;           // Lợi nhuận gộp trong mốc
    int orderCount;          // Số lượng đơn hoàn tất trong mốc

    TimelineStatPoint() : label(""), revenue(0.0), profit(0.0), orderCount(0) {}
    TimelineStatPoint(const std::string& l, double r, double p, int cnt)
        : label(l), revenue(r), profit(p), orderCount(cnt) {}
};

// Cơ cấu doanh thu theo Thương hiệu điện thoại (T13)
struct BrandShareStatItem {
    std::string brand;       // Tên hãng (Apple, Samsung, Oppo, Xiaomi...)
    double revenue;          // Doanh thu bán được của hãng
    int soldQuantity;        // Số máy bán ra của hãng
    double percentage;       // Tỷ trọng % trên tổng doanh thu (0.0 - 100.0)

    BrandShareStatItem() : brand(""), revenue(0.0), soldQuantity(0), percentage(0.0) {}
    BrandShareStatItem(const std::string& b, double r, int qty, double pct)
        : brand(b), revenue(r), soldQuantity(qty), percentage(pct) {}
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
    std::vector<TimelineStatPoint> timelineStats;   // Thống kê theo dòng thời gian (biểu đồ cột)
    std::vector<BrandShareStatItem> brandStats;     // Thống kê theo thương hiệu (cơ cấu hãng)

    DashboardStatsViewModel()
        : totalRevenue(0.0), totalOrders(0), completedOrders(0),
          pendingOrders(0), shippingOrders(0), cancelledOrders(0),
          totalProducts(0), totalStockQuantity(0), lowStockCount(0),
          totalCustomers(0), totalEmployees(0), totalInventoryValue(0.0),
          estimatedProfit(0.0) {}
};

#endif // STATISTICS_VIEWMODEL_H
