#ifndef GOODS_RECEIPT_VIEWMODEL_H
#define GOODS_RECEIPT_VIEWMODEL_H

#include <string>
#include <vector>

// Chi tiết mặt hàng trong phiếu nhập kho
struct GoodsReceiptDetailViewModel {
    std::string phoneId;
    std::string phoneName;
    int quantity;
    double importPrice;
    double subtotal;

    GoodsReceiptDetailViewModel() : quantity(0), importPrice(0.0), subtotal(0.0) {}
};

// ViewModel hiển thị một phiếu nhập kho
struct GoodsReceiptSummaryViewModel {
    std::string id;
    std::string supplierId;
    std::string supplierName;
    std::string employeeId;
    std::string employeeName;
    std::string importTime;
    double totalAmount;
    std::string status; // "HoatDong" hoặc "DaHuy"
    std::vector<GoodsReceiptDetailViewModel> details;

    GoodsReceiptSummaryViewModel() : totalAmount(0.0), status("HoatDong") {}
};

// Form tạo phiếu nhập kho
struct CreateReceiptRequest {
    std::string supplierId;
    std::string employeeId;
    std::vector<GoodsReceiptDetailViewModel> items;
};

#endif // GOODS_RECEIPT_VIEWMODEL_H
