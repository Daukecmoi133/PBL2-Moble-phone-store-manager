#pragma once
// Thống kê, chỉ đọc. Đọc thẳng Repository, KHÔNG gọi OrderService. Toàn bộ hàm hiện là stub.
// Quy tắc: doanh thu = tổng giá trị đơn COMPLETED và PAID.

#include "../common/Actor.h"
#include "../common/Result.h"
#include "../dto/ReportRow.h"
#include "../model/enums/OrderStatus.h"
#include "../repository/InvoiceRepository.h"
#include "../repository/OrderRepository.h"
#include "../repository/PhoneRepository.h"
#include "../storage/ds/Vector.h"
#include "../common/DateTime.h"

class ReportService {
public:
    ReportService(OrderRepository& orders, InvoiceRepository& invoices, PhoneRepository& phones);

    Result<Vector<ReportRow>> revenueByPeriod(const Actor& actor, const DateTime& from, const DateTime& to) const;
    Result<Vector<ReportRow>> orderCountByStatus(const Actor& actor) const;
    Result<Vector<ReportRow>> bestSellingProducts(const Actor& actor, int topN) const;
    Result<Vector<ReportRow>> stockSummary(const Actor& actor) const;

private:
    OrderRepository& orderRepo;
    InvoiceRepository& invoiceRepo;
    PhoneRepository& phoneRepo;
};
