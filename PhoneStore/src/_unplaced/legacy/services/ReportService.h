// REPORT SERVICE - Tầng tổng hợp báo cáo thống kê
// Tập trung logic báo cáo tại đây; OrderService.calculateTotalRevenue() được giữ
// tương thích nhưng ReportService là nơi chính thức tính doanh thu/thống kê.
#ifndef REPORTSERVICE_H
#define REPORTSERVICE_H

#include <iostream>
#include <string>
#include "../models/Order.h"
#include "../models/Invoice.h"
#include "../models/Phone.h"
#include "../core/Vector.h"
#include "../repositories/OrderRepository.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/PhoneRepository.h"
#include "../utils/DateTime.h"

// Cấu trúc lưu kết quả báo cáo sản phẩm bán chạy
struct ProductSalesReport {
    std::string phoneId;
    std::string phoneName;
    int totalSold;
    double totalRevenue;
};

// ReportService: Chỉ đọc dữ liệu, không thay đổi bất kỳ dữ liệu nào.
// Tất cả hàm đều const; không expose dữ liệu mutable.
class ReportService {
private:
    const OrderRepository& orderRepo;
    const InvoiceRepository& invoiceRepo;
    const PhoneRepository& phoneRepo;

public:
    ReportService(const OrderRepository& oRepo,
                  const InvoiceRepository& iRepo,
                  const PhoneRepository& pRepo);

    // ==== BÁO CÁO DOANH THU ====

    // Tổng doanh thu từ đơn COMPLETED (đã giao hàng thành công)
    // Định nghĩa: chỉ tính COMPLETED; CONFIRMED chưa giao xong
    double getTotalRevenue() const;

    // Doanh thu trong khoảng thời gian [from, to]
    double getRevenueByPeriod(const DateTime& from, const DateTime& to) const;

    // ==== BÁO CÁO ĐƠN HÀNG ====

    // Đếm số đơn theo từng trạng thái
    int countOrdersByStatus(OrderStatus status) const;

    // Lọc đơn trong khoảng thời gian
    Vector<Order> getOrdersByPeriod(const DateTime& from, const DateTime& to) const;

    // ==== BÁO CÁO SẢN PHẨM ====

    // Tổng hợp sản phẩm bán chạy (sắp xếp giảm dần theo số lượng đã bán)
    // Chỉ tính từ các đơn COMPLETED
    Vector<ProductSalesReport> getBestSellingProducts(int topN = 10) const;

    // Thống kê tồn kho hiện tại (sản phẩm sắp hết hàng: quantity <= threshold)
    Vector<Phone> getLowStockProducts(int threshold = 5) const;

    // Tổng số lượng sản phẩm trong kho
    int getTotalStockCount() const;

    // ==== BÁO CÁO TỔNG QUAN ====

    // In báo cáo tóm tắt ra console (dùng cho demo/debug)
    void printSummary() const;
};

// ================= IMPLEMENTATION =================

inline ReportService::ReportService(const OrderRepository& oRepo,
                                     const InvoiceRepository& iRepo,
                                     const PhoneRepository& pRepo)
    : orderRepo(oRepo), invoiceRepo(iRepo), phoneRepo(pRepo) {}

// Tổng doanh thu: chỉ tính đơn COMPLETED (đã giao/hoàn tất)
// Dùng calculateTotal() của Order để tính đúng (bao gồm discount)
inline double ReportService::getTotalRevenue() const {
    double total = 0.0;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == OrderStatus::COMPLETED) {
            total += orders[i].calculateTotal();
        }
    }
    return total;
}

// Doanh thu trong khoảng thời gian [from, to]
// Tính theo orderTime của các đơn COMPLETED
inline double ReportService::getRevenueByPeriod(const DateTime& from, const DateTime& to) const {
    if (to < from) return 0.0;
    double total = 0.0;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == OrderStatus::COMPLETED) {
            const DateTime& t = orders[i].getOrderTime();
            if (t >= from && t <= to) {
                total += orders[i].calculateTotal();
            }
        }
    }
    return total;
}

// Đếm số đơn theo trạng thái
inline int ReportService::countOrdersByStatus(OrderStatus status) const {
    int count = 0;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == status) ++count;
    }
    return count;
}

// Lọc đơn hàng trong khoảng thời gian [from, to]
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

// Tổng hợp sản phẩm bán chạy - chỉ từ đơn COMPLETED
// Dùng insertion sort đơn giản (topN thường nhỏ)
inline Vector<ProductSalesReport> ReportService::getBestSellingProducts(int topN) const {
    // Bước 1: Tổng hợp số lượng và doanh thu từng phoneId
    Vector<std::string> ids;
    Vector<int> quantities;
    Vector<double> revenues;

    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() != OrderStatus::COMPLETED) continue;
        const Vector<OrderDetail>& details = orders[i].getDetails();
        for (int j = 0; j < details.getSize(); ++j) {
            const std::string& pid = details[j].getPhoneId();
            bool found = false;
            for (int k = 0; k < ids.getSize(); ++k) {
                if (ids[k] == pid) {
                    quantities[k] += details[j].getQuantity();
                    revenues[k] += details[j].getUnitPrice() * details[j].getQuantity();
                    found = true;
                    break;
                }
            }
            if (!found) {
                ids.push_back(pid);
                quantities.push_back(details[j].getQuantity());
                revenues.push_back(details[j].getUnitPrice() * details[j].getQuantity());
            }
        }
    }

    // Bước 2: Sắp xếp giảm dần theo số lượng (bubble sort đơn giản)
    int n = ids.getSize();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (quantities[j] < quantities[j + 1]) {
                // Swap tất cả
                std::string tmpId = ids[j]; ids[j] = ids[j+1]; ids[j+1] = tmpId;
                int tmpQ = quantities[j]; quantities[j] = quantities[j+1]; quantities[j+1] = tmpQ;
                double tmpR = revenues[j]; revenues[j] = revenues[j+1]; revenues[j+1] = tmpR;
            }
        }
    }

    // Bước 3: Lấy topN kết quả
    Vector<ProductSalesReport> result;
    int limit = (topN < n) ? topN : n;
    for (int i = 0; i < limit; ++i) {
        ProductSalesReport r;
        r.phoneId = ids[i];
        r.totalSold = quantities[i];
        r.totalRevenue = revenues[i];
        // Lấy tên sản phẩm từ PhoneRepository
        const Phone* p = phoneRepo.findById(ids[i]);
        r.phoneName = (p != nullptr) ? p->getName() : ids[i];
        result.push_back(r);
    }
    return result;
}

// Sản phẩm tồn kho thấp (quantity <= threshold)
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

// Tổng số lượng sản phẩm trong kho
inline int ReportService::getTotalStockCount() const {
    int total = 0;
    const Vector<Phone>& phones = phoneRepo.getAll();
    for (int i = 0; i < phones.getSize(); ++i) {
        total += phones[i].getQuantity();
    }
    return total;
}

// In báo cáo tóm tắt ra console
inline void ReportService::printSummary() const {
    std::cout << "=== BAO CAO TONG QUAN ===" << std::endl;
    std::cout << "Tong doanh thu (don COMPLETED): " << getTotalRevenue() << " VND" << std::endl;
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
