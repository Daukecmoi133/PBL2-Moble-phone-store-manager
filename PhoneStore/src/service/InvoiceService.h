#pragma once
// Nghiệp vụ hóa đơn (gọi Order, Customer, Employee). Toàn bộ hàm hiện là stub.

#include <string>

#include "CustomerService.h"
#include "EmployeeService.h"
#include "OrderService.h"
#include "../common/Actor.h"
#include "../common/Result.h"
#include "../model/sales/Invoice.h"
#include "../repository/InvoiceRepository.h"
#include "../storage/ds/Vector.h"

class InvoiceService {
public:
    InvoiceService(InvoiceRepository& invoices, OrderService& orders, CustomerService& customers,
                   EmployeeService& employees);

    Result<std::string> createInvoiceFromOrder(const Actor& actor, const std::string& orderId);
    Result<Invoice> getByOrderId(const Actor& actor, const std::string& orderId) const;
    Result<Vector<Invoice>> list(const Actor& actor) const;

private:
    InvoiceRepository& invoiceRepo;
    OrderService& orderService;
    CustomerService& customerService;
    EmployeeService& employeeService;
};
