#ifndef CHECK_H
#define CHECK_H

#include <iostream>
#include <string>
#include <cctype>
#include "../repositories/OrderRepository.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/StockReceiptRepository.h"


// Kiểm tra tính hợp lệ của mã ID (chỉ chứa các ký tự chữ cái và số, không rỗng)
inline bool isValidID(const std::string& id, std::string& errorMessage) {
    if (id.empty()) {
        errorMessage = "Mã ID không được để trống!";
        return false;
    }
    for (char c : id) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_') {
            errorMessage = "Mã ID chỉ được chứa chữ cái, số và dấu gạch ngang!";
            return false;
        }
    }
    return true;
}

inline bool isValidID(const std::string& id) {
    std::string err;
    return isValidID(id, err);
}

// Lớp kiểm tra ràng buộc toàn vẹn tham chiếu (Reference Integrity Checker)
// Ngăn chặn xóa sản phẩm nếu đã có giao dịch trong Đơn hàng, Hóa đơn hoặc Phiếu nhập
class ReferenceChecker {
private:
    OrderRepository* orderRepo;
    InvoiceRepository* invoiceRepo;
    StockReceiptRepository* stockReceiptRepo;

public:
    ReferenceChecker(
        OrderRepository* oRepo = nullptr,
        InvoiceRepository* iRepo = nullptr,
        StockReceiptRepository* sRepo = nullptr
    ) : orderRepo(oRepo), invoiceRepo(iRepo), stockReceiptRepo(sRepo) {}

    void setOrderRepo(OrderRepository* repo) { orderRepo = repo; }
    void setInvoiceRepo(InvoiceRepository* repo) { invoiceRepo = repo; }
    void setStockReceiptRepo(StockReceiptRepository* repo) { stockReceiptRepo = repo; }

    bool isPhoneReferenced(const std::string& phoneID) const {
        if (orderRepo != nullptr && orderRepo->containsPhone(phoneID)) {
            return true;
        }
        if (invoiceRepo != nullptr && invoiceRepo->containsPhone(phoneID)) {
            return true;
        }
        if (stockReceiptRepo != nullptr && stockReceiptRepo->containsPhone(phoneID)) {
            return true;
        }
        return false;
    }
};

#endif // CHECK_H