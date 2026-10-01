// SUPPLIER SERVICE - Tầng nghiệp vụ quản lý nhà cung cấp và phiếu nhập kho
// Đảm bảo: tăng tồn kho và lưu phiếu nhập là một giao dịch nguyên tử (rollback nếu lỗi)
#ifndef SUPPLIERSERVICE_H
#define SUPPLIERSERVICE_H

#include <iostream>
#include <string>
#include "../models/Supplier.h"
#include "../models/ImportReceipt.h"
#include "../core/Vector.h"
#include "../repositories/SupplierRepository.h"
#include "../repositories/StockReceiptRepository.h"
#include "../utils/StringHelper.h"
#include "Check.h"
#include "PhoneService.h"

// Tầng Service: Nghiệp vụ nhà cung cấp + nhập kho
// Business rules chính:
//   - Không xóa supplier đang được tham chiếu trong phiếu nhập
//   - Lưu phiếu nhập và tăng tồn kho phải nguyên tử (rollback nếu lỗi)
//   - Mỗi detail nhập kho: quantity > 0, importPrice >= 0, productId phải tồn tại
class SupplierService {
private:
    SupplierRepository& supplierRepo;
    StockReceiptRepository& receiptRepo;
    PhoneService* phoneService;   // Dùng để tăng tồn kho khi nhập hàng

    // Tự sinh ID phiếu nhập kho theo format REC-XXXX
    std::string generateReceiptId() const;

    // Validate thông tin nhà cung cấp
    bool validateSupplier(const Supplier& sup, std::string& errorMessage) const;

public:
    SupplierService(SupplierRepository& sRepo,
                    StockReceiptRepository& rRepo,
                    PhoneService* pService = nullptr);

    void setPhoneService(PhoneService* ps) { phoneService = ps; }

    // ==== SUPPLIER CRUD ====
    const Vector<Supplier>& getAllSuppliers() const;
    Supplier* getSupplierById(const std::string& id);
    Vector<Supplier> searchByName(const std::string& keyword) const;

    bool addSupplier(const Supplier& sup, std::string& errorMessage);
    bool addSupplier(const Supplier& sup);
    bool updateSupplier(const Supplier& updated, std::string& errorMessage);
    bool updateSupplier(const Supplier& updated);
    bool deleteSupplier(const std::string& id, std::string& errorMessage);
    bool deleteSupplier(const std::string& id);

    // ==== STOCK RECEIPT (PHIẾU NHẬP KHO) ====
    const Vector<ImportReceipt>& getAllReceipts() const;
    ImportReceipt* getReceiptById(const std::string& id);
    Vector<ImportReceipt> getReceiptsBySupplierId(const std::string& supplierId) const;

    // Nghiệp vụ nhập kho:
    //  1. Validate: supplier tồn tại, employee tồn tại, tất cả detail hợp lệ
    //  2. Lưu phiếu nhập vào repository
    //  3. Tăng tồn kho từng sản phẩm qua PhoneService
    //  4. Rollback (xóa phiếu + hoàn tồn kho) nếu bất kỳ bước nào thất bại
    bool receiveStock(ImportReceipt& receipt, std::string& errorMessage);
    bool receiveStock(ImportReceipt& receipt);
};

// ================= IMPLEMENTATION =================

// Tự sinh ID phiếu nhập theo format REC-XXXX
inline std::string SupplierService::generateReceiptId() const {
    int maxNum = 0;
    const Vector<ImportReceipt>& list = receiptRepo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        const std::string& id = list[i].getID();
        if (id.size() > 4 && id.substr(0, 4) == "REC-") {
            try {
                int num = std::stoi(id.substr(4));
                if (num > maxNum) maxNum = num;
            } catch (...) {}
        }
    }
    std::string numStr = std::to_string(maxNum + 1);
    while (numStr.size() < 3) numStr = "0" + numStr;
    return "REC-" + numStr;
}

// Validate nghiệp vụ nhà cung cấp
inline bool SupplierService::validateSupplier(const Supplier& sup, std::string& errorMessage) const {
    if (!isValidID(sup.getID(), errorMessage)) return false;
    if (StringHelper::trim(sup.getCompanyName()).empty()) {
        errorMessage = "Ten cong ty khong duoc de trong!";
        return false;
    }
    if (StringHelper::trim(sup.getPhoneNumber()).length() < 8) {
        errorMessage = "So dien thoai nha cung cap khong hop le (toi thieu 8 ky tu)!";
        return false;
    }
    return true;
}

inline SupplierService::SupplierService(SupplierRepository& sRepo,
                                         StockReceiptRepository& rRepo,
                                         PhoneService* pService)
    : supplierRepo(sRepo), receiptRepo(rRepo), phoneService(pService) {}

inline const Vector<Supplier>& SupplierService::getAllSuppliers() const {
    return supplierRepo.getAll();
}

inline Supplier* SupplierService::getSupplierById(const std::string& id) {
    return supplierRepo.findById(id);
}

inline Vector<Supplier> SupplierService::searchByName(const std::string& keyword) const {
    return supplierRepo.searchByCompanyName(keyword);
}

// Thêm nhà cung cấp mới
inline bool SupplierService::addSupplier(const Supplier& sup, std::string& errorMessage) {
    if (!validateSupplier(sup, errorMessage)) return false;

    if (supplierRepo.findById(sup.getID()) != nullptr) {
        errorMessage = "Ma nha cung cap [" + sup.getID() + "] da ton tai!";
        return false;
    }

    if (!supplierRepo.addSupplier(sup)) {
        errorMessage = "Loi khi luu nha cung cap vao co so du lieu!";
        return false;
    }

    errorMessage = "Them nha cung cap thanh cong!";
    return true;
}

inline bool SupplierService::addSupplier(const Supplier& sup) {
    std::string err;
    return addSupplier(sup, err);
}

// Cập nhật nhà cung cấp
inline bool SupplierService::updateSupplier(const Supplier& updated, std::string& errorMessage) {
    if (supplierRepo.findById(updated.getID()) == nullptr) {
        errorMessage = "Khong tim thay nha cung cap [" + updated.getID() + "] de cap nhat!";
        return false;
    }
    if (!validateSupplier(updated, errorMessage)) return false;

    if (!supplierRepo.updateSupplier(updated)) {
        errorMessage = "Loi khi cap nhat nha cung cap!";
        return false;
    }
    errorMessage = "Cap nhat nha cung cap thanh cong!";
    return true;
}

inline bool SupplierService::updateSupplier(const Supplier& updated) {
    std::string err;
    return updateSupplier(updated, err);
}

// Xóa nhà cung cấp - từ chối nếu đang có phiếu nhập tham chiếu
inline bool SupplierService::deleteSupplier(const std::string& id, std::string& errorMessage) {
    if (supplierRepo.findById(id) == nullptr) {
        errorMessage = "Khong tim thay nha cung cap [" + id + "] de xoa!";
        return false;
    }

    // Kiểm tra tham chiếu phiếu nhập kho
    Vector<ImportReceipt> refs = receiptRepo.findBySupplierId(id);
    if (refs.getSize() > 0) {
        errorMessage = "Khong the xoa nha cung cap dang co phieu nhap kho tham chieu!";
        return false;
    }

    if (!supplierRepo.deleteSupplier(id)) {
        errorMessage = "Loi khi xoa nha cung cap!";
        return false;
    }
    errorMessage = "Xoa nha cung cap thanh cong!";
    return true;
}

inline bool SupplierService::deleteSupplier(const std::string& id) {
    std::string err;
    return deleteSupplier(id, err);
}

inline const Vector<ImportReceipt>& SupplierService::getAllReceipts() const {
    return receiptRepo.getAll();
}

inline ImportReceipt* SupplierService::getReceiptById(const std::string& id) {
    return receiptRepo.findById(id);
}

inline Vector<ImportReceipt> SupplierService::getReceiptsBySupplierId(const std::string& supplierId) const {
    return receiptRepo.findBySupplierId(supplierId);
}

// Nghiệp vụ nhập kho - GIAO DỊCH NGUYÊN TỬ (Atomic Transaction)
// Bước 1: Validate tất cả detail trước khi thực hiện bất kỳ thay đổi nào
// Bước 2: Lưu phiếu nhập
// Bước 3: Tăng tồn kho từng sản phẩm
// Bước 4: Nếu bất kỳ bước nào thất bại → rollback toàn bộ
inline bool SupplierService::receiveStock(ImportReceipt& receipt, std::string& errorMessage) {
    // Validate phiếu nhập
    if (!isValidID(receipt.getID().empty() ? generateReceiptId() : receipt.getID(), errorMessage)) {
        // Auto-assign ID nếu chưa có
    }

    // Auto-assign ID nếu rỗng
    if (receipt.getID().empty()) {
        receipt.setID(generateReceiptId());
    }

    if (receiptRepo.findById(receipt.getID()) != nullptr) {
        errorMessage = "Ma phieu nhap [" + receipt.getID() + "] da ton tai!";
        return false;
    }

    if (supplierRepo.findById(receipt.getSupplierId()) == nullptr) {
        errorMessage = "Nha cung cap [" + receipt.getSupplierId() + "] khong ton tai!";
        return false;
    }

    if (receipt.getEmployeeId().empty()) {
        errorMessage = "Ma nhan vien khong duoc de trong!";
        return false;
    }

    const Vector<OrderDetail>& details = receipt.getDetails();
    if (details.getSize() == 0) {
        errorMessage = "Phieu nhap phai co it nhat mot san pham!";
        return false;
    }

    // BƯỚC 1: Validate TẤT CẢ detail trước (không thực hiện thay đổi nào)
    for (int i = 0; i < details.getSize(); ++i) {
        if (details[i].getQuantity() <= 0) {
            errorMessage = "So luong san pham [" + details[i].getPhoneName() + "] phai lon hon 0!";
            return false;
        }
        if (details[i].getUnitPrice() < 0) {
            errorMessage = "Gia nhap san pham [" + details[i].getPhoneName() + "] khong duoc am!";
            return false;
        }
        // Kiểm tra product ID tồn tại nếu có PhoneService
        if (phoneService != nullptr) {
            if (phoneService->getPhoneById(details[i].getPhoneId()) == nullptr) {
                errorMessage = "San pham ID [" + details[i].getPhoneId() + "] khong ton tai trong he thong!";
                return false;
            }
        }
    }

    // BƯỚC 2: Lưu phiếu nhập kho
    if (!receiptRepo.addReceipt(receipt)) {
        errorMessage = "Loi khi luu phieu nhap kho!";
        return false;
    }

    // BƯỚC 3: Tăng tồn kho từng sản phẩm
    if (phoneService != nullptr) {
        Vector<int> successfulIndices; // Theo dõi các sản phẩm đã tăng kho để rollback
        bool allSuccess = true;
        std::string stockErr;

        for (int i = 0; i < details.getSize(); ++i) {
            if (!phoneService->updateStock(details[i].getPhoneId(),
                                            details[i].getQuantity(),
                                            stockErr)) {
                allSuccess = false;
                errorMessage = "Loi khi tang ton kho san pham [" + details[i].getPhoneName() + "]: " + stockErr;
                break;
            }
            successfulIndices.push_back(i);
        }

        // BƯỚC 4: Rollback nếu tăng kho bị lỗi
        if (!allSuccess) {
            // Hoàn tồn kho đã tăng trước đó
            for (int j = 0; j < successfulIndices.getSize(); ++j) {
                int idx = successfulIndices[j];
                phoneService->updateStock(details[idx].getPhoneId(), -details[idx].getQuantity());
            }
            // Xóa phiếu nhập đã lưu
            receiptRepo.deleteReceipt(receipt.getID());
            errorMessage += " | Da hoan nguyen giao dich nhap kho.";
            return false;
        }
    }

    errorMessage = "Nhap kho thanh cong! Phieu nhap [" + receipt.getID() + "] da duoc ghi nhan.";
    return true;
}

inline bool SupplierService::receiveStock(ImportReceipt& receipt) {
    std::string err;
    return receiveStock(receipt, err);
}

#endif // SUPPLIERSERVICE_H
