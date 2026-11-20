#include "AppControllerSupport.h"
#include "../utils/ExportHelper.h"
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
    if (!currentSession.isAuthenticated || !currentSession.isAdmin()) {
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
    DashboardStatsViewModel stats;

    if (orderService != nullptr) {
        const Vector<Order>& orders = orderService->getAllOrders();
        for (int i = 0; i < orders.getSize(); ++i) {
            const std::string ordTimeStr = orders[i].getOrderTime().toString();
            if (!inRange(ordTimeStr)) continue;
            stats.totalOrders++;
            const OrderStatus ordStatus = orders[i].getStatus();
            if (ordStatus == OrderStatus::COMPLETED) {
                stats.completedOrders++;
            } else if (ordStatus == OrderStatus::PENDING) {
                stats.pendingOrders++;
            } else if (ordStatus == OrderStatus::SHIPPING ||
                       ordStatus == OrderStatus::PREPARING ||
                       ordStatus == OrderStatus::READY_FOR_PICKUP) {
                stats.shippingOrders++;
            } else if (ordStatus == OrderStatus::CANCELLED) {
                stats.cancelledOrders++;
            }
        }
    }
    stats.totalProducts = phoneService ? phoneService->getAllPhones().getSize() : 0;
    stats.totalCustomers = customerService ? customerService->getAllCustomers().getSize() : 0;
    stats.totalEmployees = employeeService ? employeeService->getAllEmployees().getSize() : 0;

    // Tài chính thống nhất qua ReportService::computeFinancialSummary (E1, AUD-03, AUD-13)
    if (reportService != nullptr) {
        const FinancialSummary fin =
            reportService->computeFinancialSummary(fromDate, toDate, timelineMode);
        stats.totalRevenue = fin.totalNetRevenue;
        stats.totalVat = fin.totalVatCollected;
        stats.totalGrossRevenue = fin.totalGrossRevenue;
        stats.estimatedProfit = fin.totalProfit;

        for (const auto& pt : fin.timeline) {
            stats.timelineStats.emplace_back(pt.label, pt.revenue, pt.profit, pt.orderCount);
        }
        for (const auto& b : fin.brands) {
            stats.brandStats.emplace_back(b.brand, b.revenue, b.soldQuantity, b.percentage);
        }
        for (const auto& p : fin.topProducts) {
            TopProductStatItem t;
            t.phoneId = p.phoneId;
            t.phoneName = p.phoneName;
            t.brand = p.brand;
            t.soldQuantity = p.soldQuantity;
            t.revenue = p.revenue;
            t.profit = p.profit;
            stats.topProducts.push_back(t);
        }
    }

    // Tồn kho
    if (phoneService != nullptr) {
        const std::unordered_map<std::string, int> reservedMap = computeReservedStockMap();
        const Vector<Phone>& phones = phoneService->getAllPhones();
        for (int i = 0; i < phones.getSize(); ++i) {
            const Phone& p = phones[i];
            const int actualStock = p.getQuantity();
            auto resIt = reservedMap.find(p.getID());
            const int reserved = (resIt != reservedMap.end()) ? resIt->second : 0;
            const int available = actualStock - reserved;
            stats.totalStockQuantity += actualStock;
            stats.totalInventoryValue += (p.getImportPrice() * actualStock);
            if (available <= 5) {
                stats.lowStockCount++;
            }
        }
    }

    return stats;
}

// === XUẤT DỮ LIỆU (.CSV) & HÓA ĐƠN / CHỨNG TỪ (.TXT) ===

namespace {

ExportHelper::ExportContext buildExportContext(AppController* ctrl,
                                               const UserSessionViewModel& session,
                                               const StoreSettingsRepository& settings,
                                               const std::string& filterSummary) {
    ExportHelper::ExportContext ctx;
    ctx.exportedAt = ctrl ? ctrl->getSimulatedTime() : "";
    if (session.isAuthenticated) {
        ctx.exportedBy = session.fullName.empty()
                             ? session.username
                             : (session.fullName + " (" + session.roleName + ")");
    } else {
        ctx.exportedBy = "Hệ thống";
    }
    ctx.filterSummary = filterSummary.empty() ? "Tất cả" : filterSummary;
    std::string ship = settings.get("store_shipping_policy");
    std::string ret = settings.get("store_return_policy");
    std::string war = settings.get("store_warranty_policy");
    if (!ship.empty()) ctx.shippingPolicy = ship;
    if (!ret.empty()) ctx.returnPolicy = ret;
    if (!war.empty()) ctx.warrantyPolicy = war;
    return ctx;
}

bool matchDateRangeExport(const std::string& timeStr,
                          const std::string& fromDate,
                          const std::string& toDate) {
    if (fromDate.empty() && toDate.empty()) return true;
    long long d = RealTimeManager::dateToEpochDays(timeStr);
    if (d == 0) return false;
    if (!fromDate.empty() && d < RealTimeManager::dateToEpochDays(fromDate)) return false;
    if (!toDate.empty() && d > RealTimeManager::dateToEpochDays(toDate)) return false;
    return true;
}

std::string buildFilterText(const std::string& keyword,
                            const std::string& fromDate = "",
                            const std::string& toDate = "",
                            const std::string& extra = "") {
    std::vector<std::string> parts;
    if (!keyword.empty()) parts.push_back("Từ khóa: '" + keyword + "'");
    if (!fromDate.empty() || !toDate.empty()) {
        std::string r = "Ngày: ";
        r += fromDate.empty() ? "..." : fromDate;
        r += " -> ";
        r += toDate.empty() ? "..." : toDate;
        parts.push_back(r);
    }
    if (!extra.empty()) parts.push_back(extra);
    if (parts.empty()) return "Tất cả";
    std::string out;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) out += " | ";
        out += parts[i];
    }
    return out;
}

} // namespace

OperationResult AppController::exportCustomersCsv(const std::string& filePath,
                                                  const std::string& keyword) {
    if (!currentSession.isAuthenticated ||
        (!currentSession.isAdmin() && !currentSession.isSales())) {
        return OperationResult::Fail("Chỉ Quản trị viên hoặc Nhân viên bán hàng mới có quyền xuất danh sách khách hàng!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto list = getCustomers(keyword);
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, buildFilterText(keyword));
    if (!ExportHelper::exportCustomersToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV khách hàng tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " khách hàng ra file:\n" + target);
}

OperationResult AppController::exportEmployeesCsv(const std::string& filePath,
                                                  const std::string& keyword) {
    if (!currentSession.isAuthenticated || !currentSession.isAdmin()) {
        return OperationResult::Fail("Chỉ Quản trị viên mới có quyền xuất danh sách nhân viên!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto list = getEmployees(keyword);
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, buildFilterText(keyword));
    if (!ExportHelper::exportEmployeesToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV nhân viên tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " nhân viên ra file:\n" + target);
}

OperationResult AppController::exportProductsCsv(const std::string& filePath,
                                                 const ProductFilterRequest& filter) {
    if (!currentSession.isAuthenticated || currentSession.isCustomer()) {
        return OperationResult::Fail("Khách hàng không có quyền xuất danh sách sản phẩm nội bộ!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto list = getProducts(filter);
    std::string extra;
    if (!filter.brand.empty() && filter.brand != "Tất cả") {
        extra = "Thương hiệu: " + filter.brand;
    }
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo,
                                  buildFilterText(filter.keyword, "", "", extra));
    if (!ExportHelper::exportProductsToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV sản phẩm tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " sản phẩm ra file:\n" + target);
}

OperationResult AppController::exportSuppliersCsv(const std::string& filePath,
                                                  const std::string& keyword) {
    if (!currentSession.isAuthenticated ||
        (!currentSession.isAdmin() && !currentSession.isWarehouse())) {
        return OperationResult::Fail("Chỉ Quản trị viên hoặc Thủ kho mới có quyền xuất danh sách nhà cung cấp!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto list = getSuppliers(keyword);
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, buildFilterText(keyword));
    if (!ExportHelper::exportSuppliersToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV nhà cung cấp tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " nhà cung cấp ra file:\n" + target);
}

OperationResult AppController::exportDiscountsCsv(const std::string& filePath) {
    if (!currentSession.isAuthenticated ||
        (!currentSession.isAdmin() && !currentSession.isSales())) {
        return OperationResult::Fail("Chỉ Quản trị viên hoặc Nhân viên bán hàng mới có quyền xuất danh sách voucher!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto list = getDiscounts();
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, "Tất cả mã giảm giá");
    if (!ExportHelper::exportDiscountsToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV khuyến mãi tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " mã giảm giá ra file:\n" + target);
}

OperationResult AppController::exportOrdersCsv(const std::string& filePath,
                                               const std::string& keyword,
                                               int statusFilter,
                                               int paymentFilter,
                                               const std::string& fromDate,
                                               const std::string& toDate) {
    if (!currentSession.isAuthenticated || currentSession.isCustomer()) {
        return OperationResult::Fail("Khách hàng không có quyền xuất toàn bộ danh sách đơn hàng!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto filtered = filterOrders(keyword, statusFilter, paymentFilter, -1);
    std::vector<OrderItemViewModel> list;
    for (const auto& o : filtered) {
        if (matchDateRangeExport(o.orderTime, fromDate, toDate)) {
            list.push_back(o);
        }
    }
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo,
                                  buildFilterText(keyword, fromDate, toDate));
    if (!ExportHelper::exportOrdersToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV đơn hàng tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " đơn hàng ra file:\n" + target);
}

OperationResult AppController::exportInvoicesCsv(const std::string& filePath,
                                                 const std::string& keyword,
                                                 const std::string& fromDate,
                                                 const std::string& toDate) {
    if (!currentSession.isAuthenticated ||
        (!currentSession.isAdmin() && !currentSession.isSales())) {
        return OperationResult::Fail("Chỉ Quản trị viên hoặc Nhân viên bán hàng mới có quyền xuất danh sách hóa đơn!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto all = getAllInvoices();
    std::vector<InvoiceItemViewModel> list;
    for (const auto& inv : all) {
        if (!matchDateRangeExport(inv.createTime, fromDate, toDate)) continue;
        if (!keyword.empty() &&
            !Utf8Helper::containsCaseInsensitive(inv.id, keyword) &&
            !Utf8Helper::containsCaseInsensitive(inv.orderId, keyword) &&
            !Utf8Helper::containsCaseInsensitive(inv.customerName, keyword) &&
            !Utf8Helper::containsCaseInsensitive(inv.employeeName, keyword)) {
            continue;
        }
        list.push_back(inv);
    }
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo,
                                  buildFilterText(keyword, fromDate, toDate));
    if (!ExportHelper::exportInvoicesToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV hóa đơn tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " hóa đơn ra file:\n" + target);
}

OperationResult AppController::exportGoodsReceiptsCsv(const std::string& filePath,
                                                      const std::string& keyword,
                                                      const std::string& fromDate,
                                                      const std::string& toDate) {
    if (!currentSession.isAuthenticated ||
        (!currentSession.isAdmin() && !currentSession.isWarehouse())) {
        return OperationResult::Fail("Chỉ Quản trị viên hoặc Thủ kho mới có quyền xuất danh sách phiếu nhập kho!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto all = getGoodsReceipts();
    std::vector<GoodsReceiptSummaryViewModel> list;
    for (const auto& r : all) {
        if (!matchDateRangeExport(r.importTime, fromDate, toDate)) continue;
        if (!keyword.empty() &&
            !Utf8Helper::containsCaseInsensitive(r.id, keyword) &&
            !Utf8Helper::containsCaseInsensitive(r.supplierName, keyword) &&
            !Utf8Helper::containsCaseInsensitive(r.employeeName, keyword)) {
            continue;
        }
        list.push_back(r);
    }
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo,
                                  buildFilterText(keyword, fromDate, toDate));
    if (!ExportHelper::exportGoodsReceiptsToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV phiếu nhập kho tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " phiếu nhập kho ra file:\n" + target);
}

OperationResult AppController::exportWarrantiesCsv(const std::string& filePath,
                                                   const std::string& keyword,
                                                   int statusFilter) {
    if (!currentSession.isAuthenticated ||
        (!currentSession.isAdmin() && !currentSession.isSales())) {
        return OperationResult::Fail("Chỉ Quản trị viên hoặc Nhân viên bán hàng mới có quyền xuất danh sách bảo hành!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto all = getWarranties();
    std::vector<WarrantyItemViewModel> list;
    for (const auto& w : all) {
        if (statusFilter >= 0 && static_cast<int>(w.status) != statusFilter) continue;
        if (!keyword.empty() &&
            !Utf8Helper::containsCaseInsensitive(w.id, keyword) &&
            !Utf8Helper::containsCaseInsensitive(w.orderId, keyword) &&
            !Utf8Helper::containsCaseInsensitive(w.phoneName, keyword) &&
            !Utf8Helper::containsCaseInsensitive(w.customerName, keyword) &&
            !Utf8Helper::containsCaseInsensitive(w.customerPhone, keyword)) {
            continue;
        }
        list.push_back(w);
    }
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, buildFilterText(keyword));
    if (!ExportHelper::exportWarrantiesToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV bảo hành tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " phiếu bảo hành ra file:\n" + target);
}

OperationResult AppController::exportAccountsCsv(const std::string& filePath) {
    if (!currentSession.isAuthenticated || !currentSession.isAdmin()) {
        return OperationResult::Fail("Chỉ Quản trị viên mới có quyền xuất danh sách tài khoản!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất không hợp lệ!");

    auto list = getAccounts();
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, "Toàn bộ tài khoản hệ thống");
    if (!ExportHelper::exportAccountsToCsv(target, list, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV tài khoản tại: " + target);
    }
    return OperationResult::Ok("Đã xuất " + std::to_string(list.size()) + " tài khoản ra file:\n" + target);
}

OperationResult AppController::exportRevenueReportCsv(const std::string& filePath,
                                                      const std::string& fromDate,
                                                      const std::string& toDate,
                                                      int timelineMode) {
    if (!currentSession.isAuthenticated || !currentSession.isAdmin()) {
        return OperationResult::Fail("Chỉ Quản trị viên mới có quyền xuất báo cáo doanh thu!");
    }
    OperationResult rangeCheck = validateReportRange(fromDate, toDate);
    if (!rangeCheck.isSuccess()) return rangeCheck;

    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file báo cáo không hợp lệ!");

    DashboardStatsViewModel stats = getDashboardStats(fromDate, toDate, timelineMode);
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo,
                                  buildFilterText("", fromDate, toDate));
    if (!ExportHelper::exportRevenueReportToCsv(target, stats, fromDate, toDate, timelineMode, ctx)) {
        return OperationResult::Fail("Không thể ghi file CSV báo cáo doanh thu tại: " + target);
    }
    return OperationResult::Ok("Đã xuất báo cáo doanh thu & lợi nhuận ra file:\n" + target);
}

OperationResult AppController::exportInvoiceTxt(const std::string& filePath,
                                                const std::string& invoiceId) {
    if (!currentSession.isAuthenticated || currentSession.isWarehouse()) {
        return OperationResult::Fail("Từ chối truy cập: Bạn không có quyền xuất hóa đơn!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất hóa đơn không hợp lệ!");

    InvoiceItemViewModel inv = getInvoiceById(invoiceId);
    if (inv.id.empty()) {
        return OperationResult::Fail("Không tìm thấy hóa đơn '" + invoiceId + "' để xuất!");
    }
    if (currentSession.isCustomer() &&
        inv.customerId != currentSession.userId) {
        return OperationResult::Fail("Bạn không có quyền xuất hóa đơn của khách hàng khác!");
    }

    std::string custPhone, custAddr;
    if (!inv.customerId.empty()) {
        CustomerItemViewModel c = getCustomerById(inv.customerId);
        custPhone = c.phone;
        custAddr = c.address;
    }
    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, "Hóa đơn " + inv.id);
    if (!ExportHelper::exportInvoiceToTxt(target, inv, custPhone, custAddr, ctx)) {
        return OperationResult::Fail("Không thể ghi file hóa đơn TXT tại: " + target);
    }
    return OperationResult::Ok("Đã xuất hóa đơn " + inv.id + " ra file:\n" + target);
}

OperationResult AppController::exportOrderTxt(const std::string& filePath,
                                              const std::string& orderId) {
    if (!currentSession.isAuthenticated) {
        return OperationResult::Fail("Từ chối truy cập: Vui lòng đăng nhập!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất phiếu đơn hàng không hợp lệ!");

    OrderItemViewModel ord = getOrderById(orderId);
    if (ord.id.empty()) {
        return OperationResult::Fail("Không tìm thấy đơn hàng '" + orderId + "' để xuất!");
    }
    if (currentSession.isCustomer() &&
        ord.customerId != currentSession.userId) {
        return OperationResult::Fail("Bạn không có quyền xuất đơn hàng của khách hàng khác!");
    }

    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, "Đơn hàng " + ord.id);
    if (!ExportHelper::exportOrderToTxt(target, ord, ctx)) {
        return OperationResult::Fail("Không thể ghi file phiếu đơn hàng TXT tại: " + target);
    }
    return OperationResult::Ok("Đã xuất phiếu đơn hàng " + ord.id + " ra file:\n" + target);
}

OperationResult AppController::exportGoodsReceiptTxt(const std::string& filePath,
                                                     const std::string& receiptId) {
    if (!currentSession.isAuthenticated ||
        (!currentSession.isAdmin() && !currentSession.isWarehouse())) {
        return OperationResult::Fail("Chỉ Quản trị viên hoặc Thủ kho mới có quyền xuất phiếu nhập kho!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất phiếu nhập kho không hợp lệ!");

    auto allReceipts = getGoodsReceipts();
    GoodsReceiptSummaryViewModel found;
    for (const auto& r : allReceipts) {
        if (r.id == receiptId) {
            found = r;
            break;
        }
    }
    if (found.id.empty()) {
        return OperationResult::Fail("Không tìm thấy phiếu nhập kho '" + receiptId + "' để xuất!");
    }

    SupplierItemViewModel sup;
    auto allSuppliers = getSuppliers("");
    for (const auto& s : allSuppliers) {
        if (s.id == found.supplierId) {
            sup = s;
            break;
        }
    }

    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, "Phiếu nhập " + found.id);
    if (!ExportHelper::exportGoodsReceiptToTxt(target, found, sup, ctx)) {
        return OperationResult::Fail("Không thể ghi file phiếu nhập kho TXT tại: " + target);
    }
    return OperationResult::Ok("Đã xuất phiếu nhập kho " + found.id + " ra file:\n" + target);
}

OperationResult AppController::exportWarrantyTxt(const std::string& filePath,
                                                 const std::string& warrantyId) {
    if (!currentSession.isAuthenticated || currentSession.isWarehouse()) {
        return OperationResult::Fail("Từ chối truy cập: Bạn không có quyền xuất phiếu bảo hành!");
    }
    std::string target = ExportHelper::ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return OperationResult::Fail("Đường dẫn file xuất phiếu bảo hành không hợp lệ!");

    auto allWarranties = getWarranties();
    WarrantyItemViewModel found;
    for (const auto& w : allWarranties) {
        if (w.id == warrantyId) {
            found = w;
            break;
        }
    }
    if (found.id.empty()) {
        return OperationResult::Fail("Không tìm thấy phiếu bảo hành '" + warrantyId + "' để xuất!");
    }
    if (currentSession.isCustomer() &&
        found.customerId != currentSession.userId) {
        return OperationResult::Fail("Bạn không có quyền xuất phiếu bảo hành của khách hàng khác!");
    }

    auto ctx = buildExportContext(this, currentSession, storeSettingsRepo, "Phiếu bảo hành " + found.id);
    if (!ExportHelper::exportWarrantyToTxt(target, found, ctx)) {
        return OperationResult::Fail("Không thể ghi file phiếu bảo hành TXT tại: " + target);
    }
    return OperationResult::Ok("Đã xuất phiếu bảo hành " + found.id + " ra file:\n" + target);
}
