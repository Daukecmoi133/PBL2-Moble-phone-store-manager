#ifndef CHECK_H
#define CHECK_H

#include <iostream>
#include <string>
#include <cctype>
#include "../repositories/OrderRepository.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/StockReceiptRepository.h"
#include "../repositories/WarrantyRequestRepository.h"
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
// Ngăn chặn xóa Sản phẩm, Khách hàng, Nhân viên, Nhà cung cấp hoặc Mã giảm giá khi đã phát sinh giao dịch
class ReferenceChecker {
private:
    OrderRepository* orderRepo;
    InvoiceRepository* invoiceRepo;
    StockReceiptRepository* stockReceiptRepo;
    WarrantyRequestRepository* warrantyRequestRepo;

public:
    ReferenceChecker(
        OrderRepository* oRepo = nullptr,
        InvoiceRepository* iRepo = nullptr,
        StockReceiptRepository* sRepo = nullptr,
        WarrantyRequestRepository* wRepo = nullptr
    ) : orderRepo(oRepo),
        invoiceRepo(iRepo),
        stockReceiptRepo(sRepo),
        warrantyRequestRepo(wRepo) {}

    void setOrderRepo(OrderRepository* repo) { orderRepo = repo; }
    void setInvoiceRepo(InvoiceRepository* repo) { invoiceRepo = repo; }
    void setStockReceiptRepo(StockReceiptRepository* repo) { stockReceiptRepo = repo; }
    void setWarrantyRequestRepo(WarrantyRequestRepository* repo) { warrantyRequestRepo = repo; }

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
        if (warrantyRequestRepo != nullptr) {
            const Vector<WarrantyRequest>& requests = warrantyRequestRepo->getAll();
            for (int i = 0; i < requests.getSize(); ++i) {
                if (requests[i].getPhoneId() == phoneID) {
                    return true;
                }
            }
        }
        return false;
    }

    bool isCustomerReferenced(const std::string& customerId) const {
        if (orderRepo != nullptr) {
            const Vector<Order>& orders = orderRepo->getAllOrders();
            for (int i = 0; i < orders.getSize(); ++i) {
                if (orders[i].getCustomerId() == customerId) {
                    return true;
                }
            }
        }
        if (invoiceRepo != nullptr) {
            const Vector<Invoice>& invoices = invoiceRepo->getAll();
            for (int i = 0; i < invoices.getSize(); ++i) {
                if (invoices[i].getCustomerId() == customerId) {
                    return true;
                }
            }
        }
        if (warrantyRequestRepo != nullptr) {
            const Vector<WarrantyRequest>& requests = warrantyRequestRepo->getAll();
            for (int i = 0; i < requests.getSize(); ++i) {
                if (requests[i].getCustomerId() == customerId) {
                    return true;
                }
            }
        }
        return false;
    }

    bool isEmployeeReferenced(const std::string& employeeId) const {
        if (invoiceRepo != nullptr) {
            const Vector<Invoice>& invoices = invoiceRepo->getAll();
            for (int i = 0; i < invoices.getSize(); ++i) {
                if (invoices[i].getEmployeeId() == employeeId) {
                    return true;
                }
            }
        }
        if (stockReceiptRepo != nullptr) {
            const Vector<ImportReceipt>& receipts = stockReceiptRepo->getAll();
            for (int i = 0; i < receipts.getSize(); ++i) {
                if (receipts[i].getEmployeeId() == employeeId) {
                    return true;
                }
            }
        }
        return false;
    }

    bool isSupplierReferenced(const std::string& supplierId) const {
        if (stockReceiptRepo != nullptr) {
            const Vector<ImportReceipt>& receipts = stockReceiptRepo->getAll();
            for (int i = 0; i < receipts.getSize(); ++i) {
                if (receipts[i].getSupplierId() == supplierId) {
                    return true;
                }
            }
        }
        return false;
    }

    bool isDiscountReferenced(const std::string& code) const {
        if (code.empty()) {
            return false;
        }
        if (orderRepo != nullptr) {
            const Vector<Order>& orders = orderRepo->getAllOrders();
            for (int i = 0; i < orders.getSize(); ++i) {
                if (orders[i].getDiscountCode() == code) {
                    return true;
                }
            }
        }
        if (invoiceRepo != nullptr) {
            const Vector<Invoice>& invoices = invoiceRepo->getAll();
            for (int i = 0; i < invoices.getSize(); ++i) {
                if (invoices[i].getDiscountCode() == code) {
                    return true;
                }
            }
        }
        return false;
    }
};

#endif // CHECK_H