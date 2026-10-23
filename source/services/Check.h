#ifndef CHECK_H
#define CHECK_H

#include <iostream>
#include <string>
#include <cctype>
#include "../repositories/OrderRepository.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/StockReceiptRepository.h"
#include "../utils/IdGenerator.h"


inline bool isValidID(const std::string& id, const std::string& prefix,
                      std::string& errorMessage, bool includesDate = false) {
    if (!IdGenerator::isValidId(id, prefix, includesDate)) {
        errorMessage = "Mã ID phải đúng định dạng " + prefix +
                       (includesDate ? "-ddmmyyyy-000001." : "-000001.");
        return false;
    }
    return true;
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