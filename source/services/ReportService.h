// REPORT SERVICE - Tầng tổng hợp báo cáo thống kê duy nhất (E1, AUD-03, AUD-13)
// Một nguồn sự thật tài chính cho Dashboard, ReportController và mọi lời gọi thống kê.
#ifndef REPORTSERVICE_H
#define REPORTSERVICE_H

#include "../core/Vector.h"
#include "../models/Invoice.h"
#include "../models/Order.h"
#include "../models/Phone.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/OrderRepository.h"
#include "../repositories/PhoneRepository.h"
#include "../utils/AppClock.h"
#include "../utils/DateTime.h"
#include "../utils/PricingCalculator.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Cấu trúc lưu kết quả báo cáo sản phẩm bán chạy (API tương thích)
struct ProductSalesReport {
    std::string phoneId;
    std::string phoneName;
    int totalSold;
    double totalRevenue;
};

struct FinancialTimelineEntry {
    long long sortKey = 0;
    std::string label;
    double revenue = 0.0;      // Doanh thu thuần (trước VAT, sau chiết khấu)
    double profit = 0.0;       // Lợi nhuận gộp (= doanh thu thuần - giá vốn snapshot)
    int orderCount = 0;
};

struct FinancialBrandEntry {
    std::string brand;
    double revenue = 0.0;      // Doanh thu thuần của hãng (đã phân bổ chiết khấu)
    int soldQuantity = 0;
    double percentage = 0.0;
};

struct FinancialProductEntry {
    std::string phoneId;
    std::string phoneName;
    std::string brand;
    int soldQuantity = 0;
    double revenue = 0.0;      // Doanh thu thuần đã phân bổ chiết khấu
    double cost = 0.0;         // Tổng giá vốn theo unitCost snapshot (fallback importPrice nếu bản ghi cũ thiếu)
    double profit = 0.0;       // revenue - cost
};

struct FinancialSummary {
    double totalNetRevenue = 0.0;    // Doanh thu thuần (trước VAT, sau chiết khấu)
    double totalVatCollected = 0.0;  // Thuế VAT thu hộ trên các hóa đơn có áp VAT
    double totalGrossRevenue = 0.0;  // Tổng tiền thanh toán khách trả (totalNetRevenue + totalVatCollected)
    double totalCost = 0.0;          // Tổng giá vốn hàng bán (COGS)
    double totalProfit = 0.0;        // totalNetRevenue - totalCost
    int invoiceCount = 0;
    std::vector<FinancialTimelineEntry> timeline;
    std::vector<FinancialBrandEntry> brands;
    std::vector<FinancialProductEntry> topProducts;
};

// ReportService: Chỉ đọc dữ liệu, không thay đổi bất kỳ dữ liệu nào.
class ReportService {
private:
    const OrderRepository& orderRepo;
    const InvoiceRepository& invoiceRepo;
    const PhoneRepository& phoneRepo;

    static bool formatTimelineBucket(const DateTime& dt,
                                     int timelineMode,
                                     std::string& outLabel,
                                     long long& outSortKey) {
        if (!dt.isValid()) return false;
        char buf[32];
        if (timelineMode == 1) { // Theo Tháng
            std::snprintf(buf, sizeof(buf), "T%02d/%04d", dt.month, dt.year);
            outLabel = buf;
            outSortKey = static_cast<long long>(dt.year) * 100 + dt.month;
        } else if (timelineMode == 2) { // Theo Năm
            std::snprintf(buf, sizeof(buf), "Năm %04d", dt.year);
            outLabel = buf;
            outSortKey = dt.year;
        } else { // Theo Ngày
            std::snprintf(buf, sizeof(buf), "%02d/%02d", dt.day, dt.month);
            outLabel = buf;
            outSortKey = AppClock::dateToEpochDays(dt.toDateString());
        }
        return true;
    }

public:
    ReportService(const OrderRepository& oRepo,
                  const InvoiceRepository& iRepo,
                  const PhoneRepository& pRepo);

    // Nguồn sự thật tài chính duy nhất (E1) tổng hợp từ Invoices.txt
    FinancialSummary computeFinancialSummary(const std::string& fromDate = "",
                                             const std::string& toDate = "",
                                             int timelineMode = 0) const;

    // ==== BÁO CÁO DOANH THU ====

    // Tổng doanh thu thuần (khớp 100% với Dashboard & tổng Invoices)
    double getTotalRevenue() const;

    // Doanh thu thuần trong khoảng thời gian [from, to]
    double getRevenueByPeriod(const DateTime& from, const DateTime& to) const;

    // ==== BÁO CÁO ĐƠN HÀNG ====

    // Đếm số đơn theo từng trạng thái
    int countOrdersByStatus(OrderStatus status) const;

    // Lọc đơn trong khoảng thời gian
    Vector<Order> getOrdersByPeriod(const DateTime& from, const DateTime& to) const;

    // ==== BÁO CÁO SẢN PHẨM ====

    // Tổng hợp sản phẩm bán chạy (sắp xếp giảm dần theo số lượng đã bán)
    Vector<ProductSalesReport> getBestSellingProducts(int topN = 10) const;

    // Thống kê tồn kho hiện tại (sản phẩm sắp hết hàng: quantity <= threshold)
    Vector<Phone> getLowStockProducts(int threshold = 5) const;

    // Tổng số lượng sản phẩm trong kho
    int getTotalStockCount() const;

    // ==== BÁO CÁO TỔNG QUAN ====
    void printSummary() const;
};

// ================= IMPLEMENTATION =================

inline ReportService::ReportService(const OrderRepository& oRepo,
                                     const InvoiceRepository& iRepo,
                                     const PhoneRepository& pRepo)
    : orderRepo(oRepo), invoiceRepo(iRepo), phoneRepo(pRepo) {}

inline FinancialSummary ReportService::computeFinancialSummary(const std::string& fromDate,
                                                               const std::string& toDate,
                                                               int timelineMode) const {
    FinancialSummary summary;

    const bool hasFrom = !fromDate.empty();
    const bool hasTo = !toDate.empty();
    const long long fromDays = hasFrom ? AppClock::dateToEpochDays(fromDate) : 0;
    const long long toDays = hasTo ? AppClock::dateToEpochDays(toDate) : 0;
    if ((hasFrom && fromDays == 0) || (hasTo && toDays == 0) ||
        (hasFrom && hasTo && fromDays > toDays)) {
        return summary;
    }

    auto inRange = [&](const DateTime& dt) {
        if (!hasFrom && !hasTo) return true;
        long long days = AppClock::dateToEpochDays(dt.toDateString());
        if (days == 0) return false;
        if (hasFrom && days < fromDays) return false;
        if (hasTo && days > toDays) return false;
        return true;
    };

    // Loại trừ hóa đơn gắn với đơn hàng đã hủy (nếu có)
    std::unordered_set<std::string> cancelledOrderIds;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == OrderStatus::CANCELLED && !orders[i].getID().empty()) {
            cancelledOrderIds.insert(orders[i].getID());
        }
    }

    std::unordered_map<std::string, const Phone*> phoneById;
    const Vector<Phone>& phones = phoneRepo.getAll();
    phoneById.reserve(static_cast<size_t>(phones.getSize()) + 16);
    for (int i = 0; i < phones.getSize(); ++i) {
        phoneById[phones[i].getID()] = &phones[i];
    }

    std::map<long long, FinancialTimelineEntry> timelineMap;
    std::map<std::string, FinancialBrandEntry> brandMap;
    std::map<std::string, FinancialProductEntry> productMap;
    double totalBrandSales = 0.0;

    const Vector<Invoice>& invoices = invoiceRepo.getAll();
    for (int ii = 0; ii < invoices.getSize(); ++ii) {
        const Invoice& inv = invoices[ii];
        if (!inv.getOrderId().empty() && cancelledOrderIds.count(inv.getOrderId()) > 0) {
            continue;
        }
        if (!inRange(inv.getCreateTime())) {
            continue;
        }

        const PricingBreakdown pb = inv.getPricingBreakdown();
        const double invNet = pb.taxableAmount;
        const double invVat = pb.vatAmount;
        const double invGross = pb.grandTotal;

        summary.invoiceCount++;
        summary.totalNetRevenue += invNet;
        summary.totalVatCollected += invVat;
        summary.totalGrossRevenue += invGross;

        const Vector<OrderDetail>& details = inv.getDetails();
        std::vector<double> lineGross;
        lineGross.reserve(static_cast<size_t>(details.getSize()));
        for (int idx = 0; idx < details.getSize(); ++idx) {
            lineGross.push_back(details[idx].getSubtotal());
        }
        const std::vector<double> allocatedNets =
            PricingCalculator::allocateLineAmounts(lineGross, invNet);

        double invCost = 0.0;
        for (int idx = 0; idx < details.getSize(); ++idx) {
            const OrderDetail& d = details[idx];
            auto pIt = phoneById.find(d.getPhoneId());
            const Phone* phonePtr = (pIt != phoneById.end()) ? pIt->second : nullptr;

            // AUD-03: Ưu tiên dùng unitCost snapshot tại thời điểm bán; chỉ fallback importPrice nếu thiếu
            const double unitCost = d.hasUnitCost()
                                        ? d.getUnitCost()
                                        : (phonePtr ? phonePtr->getImportPrice() : 0.0);
            const double lineCost = unitCost * d.getQuantity();
            const double lineNetRev = allocatedNets[static_cast<size_t>(idx)];

            invCost += lineCost;
            summary.totalCost += lineCost;

            const std::string brand =
                (phonePtr && !phonePtr->getBrand().empty()) ? phonePtr->getBrand() : "Khác";
            auto& bEntry = brandMap[brand];
            bEntry.brand = brand;
            bEntry.revenue += lineNetRev;
            bEntry.soldQuantity += d.getQuantity();
            totalBrandSales += lineNetRev;

            auto& pEntry = productMap[d.getPhoneId()];
            if (pEntry.phoneId.empty()) {
                pEntry.phoneId = d.getPhoneId();
                pEntry.phoneName = (phonePtr && !phonePtr->getName().empty())
                                       ? phonePtr->getName()
                                       : d.getPhoneName();
                pEntry.brand = brand;
            }
            pEntry.soldQuantity += d.getQuantity();
            pEntry.revenue += lineNetRev;
            pEntry.cost += lineCost;
        }

        std::string bucketLabel;
        long long bucketKey = 0;
        if (formatTimelineBucket(inv.getCreateTime(), timelineMode, bucketLabel, bucketKey)) {
            auto& pt = timelineMap[bucketKey];
            pt.sortKey = bucketKey;
            pt.label = bucketLabel;
            pt.revenue += invNet;
            pt.profit += (invNet - invCost);
            pt.orderCount += 1;
        }
    }

    summary.totalProfit = summary.totalNetRevenue - summary.totalCost;

    summary.timeline.reserve(timelineMap.size());
    for (const auto& kv : timelineMap) {
        summary.timeline.push_back(kv.second);
    }

    summary.brands.reserve(brandMap.size());
    for (auto& kv : brandMap) {
        kv.second.percentage = (totalBrandSales > 0.0)
                                   ? (kv.second.revenue * 100.0 / totalBrandSales)
                                   : 0.0;
        summary.brands.push_back(kv.second);
    }
    std::sort(summary.brands.begin(), summary.brands.end(),
              [](const FinancialBrandEntry& a, const FinancialBrandEntry& b) {
                  return a.revenue > b.revenue;
              });

    summary.topProducts.reserve(productMap.size());
    for (auto& kv : productMap) {
        if (kv.second.soldQuantity > 0) {
            kv.second.profit = kv.second.revenue - kv.second.cost;
            summary.topProducts.push_back(kv.second);
        }
    }
    std::sort(summary.topProducts.begin(), summary.topProducts.end(),
              [](const FinancialProductEntry& a, const FinancialProductEntry& b) {
                  if (a.soldQuantity != b.soldQuantity) return a.soldQuantity > b.soldQuantity;
                  return a.revenue > b.revenue;
              });

    return summary;
}

inline double ReportService::getTotalRevenue() const {
    return computeFinancialSummary().totalNetRevenue;
}

inline double ReportService::getRevenueByPeriod(const DateTime& from, const DateTime& to) const {
    if (to < from) return 0.0;
    return computeFinancialSummary(from.toDateString(), to.toDateString(), 0).totalNetRevenue;
}

inline int ReportService::countOrdersByStatus(OrderStatus status) const {
    int count = 0;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == status) ++count;
    }
    return count;
}

inline Vector<Order> ReportService::getOrdersByPeriod(const DateTime& from, const DateTime& to) const {
    Vector<Order> result;
    if (to < from) return result;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        const DateTime& t = orders[i].getOrderTime();
        if (t >= from && t <= to) {
            result.push_back(orders[i]);
        }
    }
    return result;
}

inline Vector<ProductSalesReport> ReportService::getBestSellingProducts(int topN) const {
    FinancialSummary summary = computeFinancialSummary();
    Vector<ProductSalesReport> result;
    if (topN <= 0) return result;
    const size_t limit = std::min(static_cast<size_t>(topN), summary.topProducts.size());
    for (size_t i = 0; i < limit; ++i) {
        ProductSalesReport r;
        r.phoneId = summary.topProducts[i].phoneId;
        r.phoneName = summary.topProducts[i].phoneName;
        r.totalSold = summary.topProducts[i].soldQuantity;
        r.totalRevenue = summary.topProducts[i].revenue;
        result.push_back(r);
    }
    return result;
}

inline Vector<Phone> ReportService::getLowStockProducts(int threshold) const {
    Vector<Phone> result;
    const Vector<Phone>& phones = phoneRepo.getAll();
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getQuantity() <= threshold) {
            result.push_back(phones[i]);
        }
    }
    return result;
}

inline int ReportService::getTotalStockCount() const {
    int total = 0;
    const Vector<Phone>& phones = phoneRepo.getAll();
    for (int i = 0; i < phones.getSize(); ++i) {
        total += phones[i].getQuantity();
    }
    return total;
}

inline void ReportService::printSummary() const {
    FinancialSummary fin = computeFinancialSummary();
    std::cout << "=== BAO CAO TONG QUAN ===" << std::endl;
    std::cout << "Tong doanh thu thuan (Invoices): " << fin.totalNetRevenue << " VND" << std::endl;
    std::cout << "Tong thue VAT thu ho: " << fin.totalVatCollected << " VND" << std::endl;
    std::cout << "Tong thanh toan (gom VAT): " << fin.totalGrossRevenue << " VND" << std::endl;
    std::cout << "Loi nhuan gop uoc tinh: " << fin.totalProfit << " VND" << std::endl;
    std::cout << "Don PENDING: " << countOrdersByStatus(OrderStatus::PENDING) << std::endl;
    std::cout << "Don CONFIRMED: " << countOrdersByStatus(OrderStatus::CONFIRMED) << std::endl;
    std::cout << "Don COMPLETED: " << countOrdersByStatus(OrderStatus::COMPLETED) << std::endl;
    std::cout << "Don CANCELLED: " << countOrdersByStatus(OrderStatus::CANCELLED) << std::endl;
    std::cout << "Tong san pham trong kho: " << getTotalStockCount() << std::endl;

    Vector<Phone> lowStock = getLowStockProducts(5);
    if (lowStock.getSize() > 0) {
        std::cout << "San pham gan het hang (<=5): " << lowStock.getSize() << " san pham" << std::endl;
        for (int i = 0; i < lowStock.getSize(); ++i) {
            std::cout << "  - " << lowStock[i].getName()
                      << " (con: " << lowStock[i].getQuantity() << ")" << std::endl;
        }
    }
    std::cout << "========================" << std::endl;
}

#endif // REPORTSERVICE_H
