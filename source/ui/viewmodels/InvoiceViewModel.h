#ifndef INVOICE_VIEWMODEL_H
#define INVOICE_VIEWMODEL_H

#include <string>
#include <vector>
#include "OrderViewModel.h"
#include "../../models/Common.h"

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
    double subtotal;        // tamTinh
    double discountAmount;  // giamGia
    long long vatRate;      // thueSuat (8%)
    double vatAmount;       // tienThue
    double total;           // tongThanhToan
    std::string status;     // "HoatDong" hoặc "DaHuy"
    double totalCost;       // tổng giá vốn các dòng D
    double profit;          // lợi nhuận = (subtotal - discountAmount) - totalCost
    std::vector<OrderDetailItemViewModel> details;

    InvoiceItemViewModel()
        : paymentMethod(PaymentMethod::CASH), subtotal(0.0),
          discountAmount(0.0), vatRate(8), vatAmount(0.0), total(0.0),
          status("HoatDong"), totalCost(0.0), profit(0.0) {}
};

#endif // INVOICE_VIEWMODEL_H
