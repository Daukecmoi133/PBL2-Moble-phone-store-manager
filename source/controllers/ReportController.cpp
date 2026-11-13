#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === THỐNG KÊ & BÁO CÁO ===

OperationResult AppController::validateReportRange(const std::string& fromDate, const std::string& toDate) {
    int d = 0, m = 0, y = 0;
    if (!fromDate.empty() && !parseDateDMY(fromDate, d, m, y)) {
        return OperationResult::Fail("'Từ ngày' không hợp lệ! Vui lòng nhập đúng định dạng DD/MM/YYYY (ví dụ 01/10/2026).");
    }
    if (!toDate.empty() && !parseDateDMY(toDate, d, m, y)) {
        return OperationResult::Fail("'Đến ngày' không hợp lệ! Vui lòng nhập đúng định dạng DD/MM/YYYY (ví dụ 31/10/2026).");
    }
    if (!fromDate.empty() && !toDate.empty() &&
        RealTimeManager::dateToEpochDays(fromDate) > RealTimeManager::dateToEpochDays(toDate)) {
        return OperationResult::Fail("'Từ ngày' phải nhỏ hơn hoặc bằng 'Đến ngày'!");
    }
    return OperationResult::Ok("Khoảng thời gian hợp lệ.");
}

DashboardStatsViewModel AppController::getDashboardStats(const std::string& fromDate, const std::string& toDate, int timelineMode) {
    if (currentSession.isAuthenticated && !currentSession.isAdmin()) {
        return DashboardStatsViewModel();
    }
    // Bộ lọc khoảng ngày (bao gồm cả hai đầu mút). Để trống cả hai = toàn thời gian.
    const bool hasFrom = !fromDate.empty();
    const bool hasTo = !toDate.empty();
    const long long fromDays = hasFrom ? RealTimeManager::dateToEpochDays(fromDate) : 0;
    const long long toDays = hasTo ? RealTimeManager::dateToEpochDays(toDate) : 0;
    auto inRange = [&](const std::string& orderTime) {
        if (!hasFrom && !hasTo) return true;
        long long days = RealTimeManager::dateToEpochDays(orderTime);
        if (days == 0) return false;
        if (hasFrom && days < fromDays) return false;
        if (hasTo && days > toDays) return false;
        return true;
    };
    auto& store = MockDataStore::getInstance();
    DashboardStatsViewModel stats;

    for (const auto& ord : store.orders) {
        if (inRange(ord.orderTime)) stats.totalOrders++;
    }
    stats.totalProducts = static_cast<int>(store.products.size());
    stats.totalCustomers = static_cast<int>(store.customers.size());
    stats.totalEmployees = static_cast<int>(store.employees.size());

    // Doanh thu thực tế: CHỈ tính các đơn COMPLETED / PAID theo spec mục 7
    double totalCostCompleted = 0.0;
    std::map<long long, TimelineStatPoint> timelineMap;
    struct BrandAcc {
        double revenue = 0.0;
        int soldQuantity = 0;
    };
    std::map<std::string, BrandAcc> brandMap;
    double totalBrandSales = 0.0;

    auto parseOrderDate = [timelineMode](const std::string& timeStr, std::string& outLabel, long long& outDays) -> bool {
        int y = 0, m = 0, d = 0;
        if (timeStr.empty()) return false;
        bool ok = false;
        if (sscanf(timeStr.c_str(), "%d-%d-%d", &y, &m, &d) == 3) {
            ok = true;
        } else if (sscanf(timeStr.c_str(), "%d/%d/%d", &d, &m, &y) == 3) {
            ok = true;
        }
        if (!ok) return false;

        char buf[32];
        if (timelineMode == 1) { // Theo Tháng
            snprintf(buf, sizeof(buf), "T%02d/%04d", m, y);
            outLabel = buf;
            outDays = (long long)y * 100 + m;
        } else if (timelineMode == 2) { // Theo Năm
            snprintf(buf, sizeof(buf), "Năm %04d", y);
            outLabel = buf;
            outDays = y;
        } else { // Theo Ngày
            snprintf(buf, sizeof(buf), "%02d/%02d", d, m);
            outLabel = buf;
            outDays = RealTimeManager::dateToEpochDays(timeStr.substr(0, 10));
        }
        return true;
    };

    for (const auto& ord : store.orders) {
        if (!inRange(ord.orderTime)) continue;
        if (ord.status == OrderStatus::COMPLETED && ord.paymentStatus == PaymentStatus::PAID) {
            stats.completedOrders++;
            stats.totalRevenue += ord.total;

            double orderCost = 0.0;
            for (const auto& d : ord.details) {
                ProductItemViewModel p = getProductById(d.phoneId);
                double itemCost = (p.importPrice * d.quantity);
                orderCost += itemCost;
                totalCostCompleted += itemCost;

                std::string brand = p.brand.empty() ? "Khác" : p.brand;
                brandMap[brand].revenue += d.subtotal;
                brandMap[brand].soldQuantity += d.quantity;
                totalBrandSales += d.subtotal;
            }

            std::string dayLabel;
            long long dayEpoch = 0;
            if (parseOrderDate(ord.orderTime, dayLabel, dayEpoch)) {
                auto& pt = timelineMap[dayEpoch];
                pt.label = dayLabel;
                pt.revenue += ord.total;
                pt.profit += (ord.total - orderCost);
                pt.orderCount += 1;
            }
        } else if (ord.status == OrderStatus::PENDING) {
            stats.pendingOrders++;
        } else if (ord.status == OrderStatus::SHIPPING || ord.status == OrderStatus::PREPARING || ord.status == OrderStatus::READY_FOR_PICKUP) {
            stats.shippingOrders++;
        } else if (ord.status == OrderStatus::CANCELLED) {
            stats.cancelledOrders++;
        }
    }

    stats.estimatedProfit = stats.totalRevenue - totalCostCompleted;

    // Chuyển timelineMap vào stats.timelineStats (đã sắp xếp theo ngày tăng dần)
    for (const auto& kv : timelineMap) {
        stats.timelineStats.push_back(kv.second);
    }

    // Chuyển brandMap vào stats.brandStats và sắp xếp giảm dần theo doanh thu
    for (const auto& kv : brandMap) {
        double pct = (totalBrandSales > 0.0) ? (kv.second.revenue * 100.0 / totalBrandSales) : 0.0;
        stats.brandStats.emplace_back(kv.first, kv.second.revenue, kv.second.soldQuantity, pct);
    }
    std::sort(stats.brandStats.begin(), stats.brandStats.end(), [](const BrandShareStatItem& a, const BrandShareStatItem& b) {
        return a.revenue > b.revenue;
    });

    // Tồn kho
    for (const auto& p : store.products) {
        stats.totalStockQuantity += p.actualStock;
        stats.totalInventoryValue += (p.importPrice * p.actualStock);
        if (p.status == StockDisplayStatus::LOW_STOCK || p.status == StockDisplayStatus::OUT_OF_STOCK) {
            stats.lowStockCount++;
        }
    }

    // Top sản phẩm bán chạy (dựa vào chi tiết các đơn COMPLETED)
    for (const auto& p : store.products) {
        int sold = 0;
        double rev = 0.0;
        for (const auto& ord : store.orders) {
            if (ord.status == OrderStatus::COMPLETED && inRange(ord.orderTime)) {
                for (const auto& d : ord.details) {
                    if (d.phoneId == p.id) {
                        sold += d.quantity;
                        rev += d.subtotal;
                    }
                }
            }
        }
        if (sold > 0) {
            TopProductStatItem t;
            t.phoneId = p.id;
            t.phoneName = p.name;
            t.brand = p.brand;
            t.soldQuantity = sold;
            t.revenue = rev;
            t.profit = rev - (p.importPrice * sold);
            stats.topProducts.push_back(t);
        }
    }

    std::sort(stats.topProducts.begin(), stats.topProducts.end(), [](const TopProductStatItem& a, const TopProductStatItem& b) {
        return a.soldQuantity > b.soldQuantity;
    });

    return stats;
}
