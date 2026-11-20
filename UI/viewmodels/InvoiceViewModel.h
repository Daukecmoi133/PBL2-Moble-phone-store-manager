#ifndef INVOICE_VIEWMODEL_H
#define INVOICE_VIEWMODEL_H

#include <string>
#include <vector>
#include "OrderViewModel.h"
#include "../../source/models/Common.h"

// ViewModel hiển thị một hóa đơn
struct InvoiceItemViewModel {
    std::string id;
    std::string orderId;
    std::string customerId;
    std::string customerName;
    std::string employeeId;
    std::string employeeName;
    PaymentMethod paymentMethod;
    std::string paymentMethodText;
    std::string createTime;
    double subtotal;
    double discountAmount;
    std::string discountCode;
    double netRevenue;
    double vatRate;
    double vatAmount;
    bool isLegacyNoVat;
    double total;
    std::vector<OrderDetailItemViewModel> details;

    InvoiceItemViewModel()
        : paymentMethod(PaymentMethod::CASH), subtotal(0.0),
          discountAmount(0.0), netRevenue(0.0),
          vatRate(0.0), vatAmount(0.0), isLegacyNoVat(false), total(0.0) {}
};

#endif // INVOICE_VIEWMODEL_H
