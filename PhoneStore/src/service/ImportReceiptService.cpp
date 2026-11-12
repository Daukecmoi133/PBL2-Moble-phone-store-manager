#include "ImportReceiptService.h"
#include "../common/Permission.h"
#include "../repository/UnitOfWork.h"
#include <sstream>
#include <iomanip>

ImportReceiptService::ImportReceiptService(ImportReceiptRepository& receipts, SupplierService& suppliers,
                                           PhoneService& phones)
    : receiptRepo(receipts), supplierService(suppliers), phoneService(phones) {}

Result<std::string> ImportReceiptService::createReceipt(const Actor& actor, const std::string& supplierId, const Vector<ImportLine>& lines) {
    if (!Permission::isAllowed(actor.role, Action::CreateImportReceipt)) {
        return Result<std::string>::fail(ErrorCode::PermissionDenied, "Không có quyền tạo phiếu nhập kho");
    }

    if (supplierId.empty()) {
        return Result<std::string>::fail(ErrorCode::InvalidInput, "Mã nhà cung cấp không được để trống");
    }
    auto supRes = supplierService.getSupplierById(actor, supplierId);
    if (!supRes.isOk()) {
        return Result<std::string>::fail(supRes.code(), "Nhà cung cấp không hợp lệ: " + supRes.message());
    }

    if (lines.getSize() == 0) {
        return Result<std::string>::fail(ErrorCode::InvalidInput, "Phiếu nhập phải có ít nhất một dòng hàng");
    }

    // Kiểm tra tính hợp lệ từng dòng và chống trùng sản phẩm trong cùng một phiếu
    Vector<std::string> seenIds;
    Vector<std::string> seenNewPhones;
    for (int i = 0; i < lines.getSize(); ++i) {
        const ImportLine& line = lines[i];
        if (line.quantity <= 0) {
            return Result<std::string>::fail(ErrorCode::InvalidInput, "Số lượng nhập phải lớn hơn 0 ở dòng " + std::to_string(i + 1));
        }
        if (line.unitPrice <= 0) {
            return Result<std::string>::fail(ErrorCode::InvalidInput, "Đơn giá nhập phải lớn hơn 0 ở dòng " + std::to_string(i + 1));
        }

        if (!line.phoneId.empty()) {
            for (int j = 0; j < seenIds.getSize(); ++j) {
                if (seenIds[j] == line.phoneId) {
                    return Result<std::string>::fail(ErrorCode::Duplicate, "Sản phẩm [" + line.phoneId + "] bị trùng lặp trong cùng phiếu nhập");
                }
            }
            seenIds.push_back(line.phoneId);
        } else {
            std::string key = StringHelper::toLower(StringHelper::trim(line.newPhone.name)) + "|" +
                              StringHelper::toLower(StringHelper::trim(line.newPhone.brand));
            for (int j = 0; j < seenNewPhones.getSize(); ++j) {
                if (seenNewPhones[j] == key) {
                    return Result<std::string>::fail(ErrorCode::Duplicate, "Sản phẩm mới [" + line.newPhone.name + "] bị trùng lặp trong cùng phiếu nhập");
                }
            }
            seenNewPhones.push_back(key);
        }
    }

    // Tạo ID cho phiếu nhập: IMP-DDMMYYYY-xxxxxx
    DateTime now = DateTime::now();
    std::ostringstream prefixOss;
    prefixOss << "IMP-" << std::setfill('0') << std::setw(2) << now.day
              << std::setw(2) << now.month << now.year << "-";
    std::string prefix = prefixOss.str();

    int maxNum = 0;
    Vector<ImportReceipt> all = receiptRepo.getAll();
    for (int i = 0; i < all.getSize(); ++i) {
        std::string id = all[i].getID();
        if (id.rfind(prefix, 0) == 0) {
            try {
                int n = std::stoi(id.substr(prefix.length()));
                if (n > maxNum) maxNum = n;
            } catch (...) {}
        }
    }
    std::ostringstream idOss;
    idOss << prefix << std::setfill('0') << std::setw(6) << (maxNum + 1);
    std::string receiptId = idOss.str();

    ImportReceipt receipt;
    receipt.setID(receiptId);
    receipt.setSupplierId(supplierId);
    receipt.setEmployeeId(actor.userId);
    receipt.setImportTime(now);

    // Một UnitOfWork duy nhất cho toàn bộ quá trình: thêm bản nháp, tăng kho, đổi giá nhập, lưu phiếu
    UnitOfWork uow;
    uow.enlist(receiptRepo);

    for (int i = 0; i < lines.getSize(); ++i) {
        const ImportLine& line = lines[i];
        std::string targetPhoneId;
        std::string phoneName;

        if (line.phoneId.empty()) {
            // Sản phẩm mới: tạo bản nháp trong UnitOfWork
            auto draftRes = phoneService.createDraftForImport(uow, line.newPhone, line.unitPrice);
            if (!draftRes.isOk()) {
                return Result<std::string>::fail(draftRes.code(), draftRes.message());
            }
            targetPhoneId = draftRes.value();
            phoneName = line.newPhone.name;
        } else {
            // Sản phẩm có sẵn: kiểm tra tồn tại
            targetPhoneId = line.phoneId;
            auto phoneRes = phoneService.getPhoneById(targetPhoneId);
            if (!phoneRes.isOk()) {
                return Result<std::string>::fail(ErrorCode::NotFound, "Không tìm thấy sản phẩm có sẵn: " + targetPhoneId);
            }
            phoneName = phoneRes.value().getName();
        }

        // Nhập kho: tăng số lượng tồn kho
        auto stockRes = phoneService.receiveStock(uow, targetPhoneId, line.quantity);
        if (!stockRes.isOk()) {
            return Result<std::string>::fail(stockRes.code(), stockRes.message());
        }

        // Cập nhật giá nhập mới nhất
        auto priceRes = phoneService.updateImportPrice(uow, targetPhoneId, line.unitPrice);
        if (!priceRes.isOk()) {
            return Result<std::string>::fail(priceRes.code(), priceRes.message());
        }

        // Thêm chi tiết vào phiếu nhập
        OrderDetail od(targetPhoneId, phoneName, line.quantity, static_cast<double>(line.unitPrice));
        receipt.addDetail(od);
    }

    auto addRes = receiptRepo.add(receipt);
    if (!addRes.isOk()) {
        return Result<std::string>::fail(addRes.code(), addRes.message());
    }

    auto commitRes = uow.commit();
    if (!commitRes.isOk()) {
        return Result<std::string>::fail(commitRes.code(), "Lỗi ghi dữ liệu phiếu nhập kho: " + commitRes.message());
    }

    return Result<std::string>::ok(receiptId);
}

Result<ImportReceipt> ImportReceiptService::getById(const Actor& actor, const std::string& receiptId) const {
    if (!Permission::isAllowed(actor.role, Action::CreateImportReceipt) &&
        !Permission::isAllowed(actor.role, Action::ViewReport)) {
        return Result<ImportReceipt>::fail(ErrorCode::PermissionDenied, "Không có quyền xem chi tiết phiếu nhập kho");
    }
    return receiptRepo.findById(receiptId);
}

Result<Vector<ImportReceipt>> ImportReceiptService::list(const Actor& actor) const {
    if (!Permission::isAllowed(actor.role, Action::CreateImportReceipt) &&
        !Permission::isAllowed(actor.role, Action::ViewReport)) {
        return Result<Vector<ImportReceipt>>::fail(ErrorCode::PermissionDenied, "Không có quyền xem danh sách phiếu nhập kho");
    }
    return Result<Vector<ImportReceipt>>::ok(receiptRepo.getAll());
}
