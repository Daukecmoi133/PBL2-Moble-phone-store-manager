// SUPPLIER SERVICE - Tầng nghiệp vụ quản lý nhà cung cấp và phiếu nhập kho
// Đảm bảo: tăng tồn kho và lưu phiếu nhập là một giao dịch nguyên tử (rollback nếu lỗi)
#ifndef SUPPLIERSERVICE_H
#define SUPPLIERSERVICE_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "../models/Supplier.h"
#include "../models/ImportReceipt.h"
#include "../core/Vector.h"
#include "../repositories/SupplierRepository.h"
#include "../repositories/StockReceiptRepository.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "Check.h"
#include "PhoneService.h"
#include "../utils/TransactionCoordinator.h"

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

    // Validate thông tin nhà cung cấp
    bool validateSupplier(const Supplier& sup, std::string& errorMessage) const;

public:
    SupplierService(SupplierRepository& sRepo,
                    StockReceiptRepository& rRepo,
                    PhoneService* pService = nullptr);

    void setPhoneService(PhoneService* ps) { phoneService = ps; }

    SupplierRepository& getSupplierRepository() { return supplierRepo; }
    const SupplierRepository& getSupplierRepository() const { return supplierRepo; }
    StockReceiptRepository& getReceiptRepository() { return receiptRepo; }
    const StockReceiptRepository& getReceiptRepository() const { return receiptRepo; }
    void reload() { supplierRepo.loadFromFile(); receiptRepo.loadFromFile(); }

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

    // ==== SUPPLIER - PRODUCT RELATIONSHIP (BUG-CRITICAL-05) ====
    bool isProductSuppliedBy(const std::string& supplierId, const std::string& phoneId) const;
    Vector<Phone> getProductsBySupplier(const std::string& supplierId) const;

    // Nghiệp vụ nhập kho:
    //  1. Validate: supplier tồn tại, employee tồn tại, tất cả detail hợp lệ, thuộc danh mục NCC
    //  2. Lưu phiếu nhập vào repository
    //  3. Tăng tồn kho từng sản phẩm qua PhoneService
    //  4. Rollback (xóa phiếu + hoàn tồn kho) nếu bất kỳ bước nào thất bại
    bool receiveStock(ImportReceipt& receipt, std::string& errorMessage);
    bool receiveStock(ImportReceipt& receipt);
};

// ================= IMPLEMENTATION =================

// Validate nghiệp vụ nhà cung cấp
inline bool SupplierService::validateSupplier(const Supplier& sup, std::string& errorMessage) const {
    if (!isValidID(sup.getID(), "SUP", errorMessage)) return false;
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
    Supplier supplier = sup;
    if (supplier.getID().empty()) {
        std::vector<std::string> existingIds;
        const Vector<Supplier>& suppliers = supplierRepo.getAll();
        for (int i = 0; i < suppliers.getSize(); ++i) {
            existingIds.push_back(suppliers[i].getID());
        }
        std::string generatedId;
        if (!IdGenerator::nextId("SUP", "", existingIds, generatedId, errorMessage)) {
            return false;
        }
        supplier.setID(generatedId);
    }
    if (!validateSupplier(supplier, errorMessage)) return false;

    if (supplierRepo.findById(supplier.getID()) != nullptr) {
        errorMessage = "Ma nha cung cap [" + supplier.getID() + "] da ton tai!";
        return false;
    }

    if (!supplierRepo.addSupplier(supplier)) {
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

// ==== SUPPLIER - PRODUCT RELATIONSHIP (BUG-HIGH-04) ====
// Xác thực quyền cung cấp:
// 1. Confirmed Mapping: Có bản ghi xác nhận trong Device/SupplierProducts.txt
// 2. Historical Receipts: Có giao dịch nhập kho đã được ghi nhận trong StockReceipts.txt
// LƯU Ý: Brand affinity chỉ mang tính chất gợi ý UI, TUYỆT ĐỐI không xem là bằng chứng xác thực.
inline bool SupplierService::isProductSuppliedBy(const std::string& supplierId, const std::string& phoneId) const {
    Supplier* sup = const_cast<SupplierRepository&>(supplierRepo).findById(supplierId);
    if (sup == nullptr) return false;

    // 1. Kiểm tra File cấu hình liên kết xác nhận Device/SupplierProducts.txt (Confirmed Mapping)
    std::string mappingPath = PathHelper::resolveDataPath("Device/SupplierProducts.txt");
    std::ifstream mf(mappingPath);
    if (mf.is_open()) {
        std::string line;
        while (std::getline(mf, line)) {
            line = StringHelper::trim(line);
            if (line.empty() || line[0] == '#') continue;
            Vector<std::string> parts = StringHelper::split(line, '|');
            if (parts.getSize() >= 2) {
                if (StringHelper::trim(parts[0]) == supplierId && StringHelper::trim(parts[1]) == phoneId) {
                    return true;
                }
            }
        }
    }

    // 2. Kiểm tra lịch sử giao dịch nhập hàng thực tế (Historical Receipts)
    Vector<ImportReceipt> receipts = receiptRepo.findBySupplierId(supplierId);
    for (int i = 0; i < receipts.getSize(); ++i) {
        const Vector<OrderDetail>& details = receipts[i].getDetails();
        for (int j = 0; j < details.getSize(); ++j) {
            if (details[j].getPhoneId() == phoneId) {
                return true;
            }
        }
    }

    // Không tự gán nhà cung cấp khi chưa có bằng chứng (Không dùng Brand affinity làm bằng chứng)
    return false;
}

inline Vector<Phone> SupplierService::getProductsBySupplier(const std::string& supplierId) const {
    Vector<Phone> result;
    if (phoneService == nullptr) return result;
    const Vector<Phone>& allPhones = phoneService->getAllPhones();
    for (int i = 0; i < allPhones.getSize(); ++i) {
        if (isProductSuppliedBy(supplierId, allPhones[i].getID())) {
            result.push_back(allPhones[i]);
        }
    }
    return result;
}

// Nghiệp vụ nhập kho - GIAO DỊCH NGUYÊN TỬ (Atomic Transaction)
// Bước 1: Validate tất cả detail trước khi thực hiện bất kỳ thay đổi nào
// Bước 2: Lưu phiếu nhập
// Bước 3: Tăng tồn kho từng sản phẩm
// Bước 4: Nếu bất kỳ bước nào thất bại → rollback toàn bộ
inline bool SupplierService::receiveStock(ImportReceipt& receipt, std::string& errorMessage) {
    if (receipt.getID().empty()) {
        std::vector<std::string> existingIds;
        const Vector<ImportReceipt>& receipts = receiptRepo.getAll();
        for (int i = 0; i < receipts.getSize(); ++i) {
            existingIds.push_back(receipts[i].getID());
        }
        std::string generatedId;
        const std::string idDate = IdGenerator::currentDateKey();
        if (idDate.empty() ||
            !IdGenerator::nextId("IMP", idDate,
                                 existingIds, generatedId, errorMessage)) {
            if (errorMessage.empty()) errorMessage = "Không thể xác định ngày để sinh mã phiếu nhập.";
            return false;
        }
        receipt.setID(generatedId);
    } else if (!isValidID(receipt.getID(), "IMP", errorMessage, true)) {
        return false;
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
        // Kiểm tra ràng buộc nhà cung cấp - sản phẩm (BUG-CRITICAL-05)
        if (!isProductSuppliedBy(receipt.getSupplierId(), details[i].getPhoneId())) {
            errorMessage = "Sản phẩm [" + details[i].getPhoneName() + "] (" + details[i].getPhoneId() + 
                           ") không thuộc danh mục cung cấp của nhà cung cấp [" + receipt.getSupplierId() + "]!";
            return false;
        }
    }

    // GIAO DỊCH NGUYÊN TỬ VỚI FileTransaction
    FileTransaction tx;
    tx.addParticipant(receiptRepo.getFilePath());
    if (phoneService != nullptr) {
        tx.addParticipant(phoneService->getRepository().getFilePath());
    }
    tx.onRollback([this]() {
        this->receiptRepo.loadFromFile();
        if (this->phoneService != nullptr) {
            this->phoneService->reload();
        }
    });

    if (!tx.begin()) {
        errorMessage = "Không thể khởi tạo giao dịch cho nghiệp vụ nhập kho!";
        return false;
    }

    // BƯỚC 2: Lưu phiếu nhập kho
    if (!receiptRepo.addReceipt(receipt)) {
        tx.rollback();
        errorMessage = "Lỗi khi lưu phiếu nhập kho!";
        return false;
    }

    // BƯỚC 3: Tăng tồn kho từng sản phẩm
    if (phoneService != nullptr) {
        bool allSuccess = true;
        std::string stockErr;

        for (int i = 0; i < details.getSize(); ++i) {
            if (!phoneService->updateStock(details[i].getPhoneId(),
                                            details[i].getQuantity(),
                                            stockErr)) {
                allSuccess = false;
                errorMessage = "Lỗi khi tăng tồn kho sản phẩm [" + details[i].getPhoneName() + "]: " + stockErr;
                break;
            }
        }

        // BƯỚC 4: Rollback nếu tăng kho bị lỗi
        if (!allSuccess) {
            tx.rollback();
            errorMessage += " | Đã hoàn nguyên giao dịch nhập kho.";
            return false;
        }
    }

    if (!tx.commit()) {
        errorMessage = "Lỗi khi hoàn tất giao dịch nhập kho!";
        return false;
    }

    errorMessage = "Nhập kho thành công! Phiếu nhập [" + receipt.getID() + "] đã được ghi nhận.";
    return true;
}

inline bool SupplierService::receiveStock(ImportReceipt& receipt) {
    std::string err;
    return receiveStock(receipt, err);
}

#endif // SUPPLIERSERVICE_H
