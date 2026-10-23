#include "ReportService.h"

ReportService::ReportService(OrderRepository& orders, InvoiceRepository& invoices, PhoneRepository& phones)
    : orderRepo(orders), invoiceRepo(invoices), phoneRepo(phones) {}

Result<Vector<ReportRow>> ReportService::revenueByPeriod(const Actor&, const DateTime&, const DateTime&) const {
    // TODO(logic): quyền ViewReport; cộng giá trị các đơn COMPLETED và PAID trong khoảng thời gian.
    return Result<Vector<ReportRow>>::fail(ErrorCode::NotImplemented, "TODO: ReportService::revenueByPeriod");
}

Result<Vector<ReportRow>> ReportService::orderCountByStatus(const Actor&) const {
    // TODO(logic): quyền ViewReport; đếm đơn theo OrderStatus.
    return Result<Vector<ReportRow>>::fail(ErrorCode::NotImplemented, "TODO: ReportService::orderCountByStatus");
}

Result<Vector<ReportRow>> ReportService::bestSellingProducts(const Actor&, int) const {
    // TODO(logic): quyền ViewReport; gộp số lượng bán theo sản phẩm, sắp xếp bằng Sort::mergeSort, lấy topN.
    return Result<Vector<ReportRow>>::fail(ErrorCode::NotImplemented, "TODO: ReportService::bestSellingProducts");
}

Result<Vector<ReportRow>> ReportService::stockSummary(const Actor&) const {
    // TODO(logic): quyền ViewReport; liệt kê tồn kho, giữ chỗ và cảnh báo sắp hết hàng.
    return Result<Vector<ReportRow>>::fail(ErrorCode::NotImplemented, "TODO: ReportService::stockSummary");
}
