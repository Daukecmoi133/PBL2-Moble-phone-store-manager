#include "InvoiceService.h"

InvoiceService::InvoiceService(InvoiceRepository& invoices, OrderService& orders, CustomerService& customers,
                               EmployeeService& employees)
    : invoiceRepo(invoices), orderService(orders), customerService(customers), employeeService(employees) {}

Result<std::string> InvoiceService::createInvoiceFromOrder(const Actor&, const std::string&) {
    // TODO(logic): quyền CreateInvoice; đơn phải COMPLETED/PAID và chưa có hóa đơn; tạo ID; add + save.
    return Result<std::string>::fail(ErrorCode::NotImplemented, "TODO: InvoiceService::createInvoiceFromOrder");
}

Result<Invoice> InvoiceService::getByOrderId(const Actor&, const std::string&) const {
    // TODO(logic): kiểm tra quyền xem; invoiceRepo.findByOrderId.
    return Result<Invoice>::fail(ErrorCode::NotImplemented, "TODO: InvoiceService::getByOrderId");
}

Result<Vector<Invoice>> InvoiceService::list(const Actor&) const {
    // TODO(logic): kiểm tra quyền rồi trả danh sách hóa đơn.
    return Result<Vector<Invoice>>::fail(ErrorCode::NotImplemented, "TODO: InvoiceService::list");
}
