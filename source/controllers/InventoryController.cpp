#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === KHO & NHÀ CUNG CẤP ===

std::vector<ProductItemViewModel> AppController::getInventory(bool lowStockOnly) {
    auto& store = MockDataStore::getInstance();
    if (!lowStockOnly) return store.products;

    std::vector<ProductItemViewModel> lowStock;
    for (const auto& p : store.products) {
        if (p.status == StockDisplayStatus::LOW_STOCK || p.status == StockDisplayStatus::OUT_OF_STOCK) {
            lowStock.push_back(p);
        }
    }
    return lowStock;
}

std::vector<ProductItemViewModel> AppController::getProductsBySupplier(const std::string& supplierId) {
    std::vector<ProductItemViewModel> all = getProducts();
    std::vector<ProductItemViewModel> filtered;
    for (const auto& p : all) {
        if (supplierService != nullptr) {
            if (supplierService->isProductSuppliedBy(supplierId, p.id)) {
                filtered.push_back(p);
            }
        } else {
            bool foundInReceipts = false;
            for (const auto& r : MockDataStore::getInstance().receipts) {
                if (r.supplierId == supplierId) {
                    for (const auto& d : r.details) {
                        if (d.phoneId == p.id) {
                            filtered.push_back(p);
                            foundInReceipts = true;
                            break;
                        }
                    }
                }
                if (foundInReceipts) break;
            }
        }
    }
    return filtered;
}

std::vector<SupplierItemViewModel> AppController::getSuppliers(const std::string& keyword) {
    auto& store = MockDataStore::getInstance();
    if (keyword.empty()) return store.suppliers;

    std::vector<SupplierItemViewModel> result;
    for (const auto& s : store.suppliers) {
        if (Utf8Helper::containsCaseInsensitive(s.companyName, keyword) ||
            Utf8Helper::containsCaseInsensitive(s.id, keyword) ||
            Utf8Helper::containsCaseInsensitive(s.phone, keyword)) {
            result.push_back(s);
        }
    }
    return result;
}

OperationResult AppController::addSupplier(const SupplierFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (req.companyName.empty() || req.phone.empty()) {
        return OperationResult::Fail("Tên công ty và số điện thoại là bắt buộc!");
    }

    auto& store = MockDataStore::getInstance();
    std::string supplierId;
    std::string idError;
    if (!IdGenerator::nextId("SUP", "", collectIds(store.suppliers), supplierId, idError)) {
        return OperationResult::Fail(idError);
    }

    SupplierItemViewModel newS;
    newS.id = supplierId;
    newS.companyName = req.companyName;
    newS.taxCode = req.taxCode;
    newS.contactPerson = req.contactPerson;
    newS.phone = req.phone;
    newS.email = req.email;
    newS.address = req.address;

    // Lưu xuống file qua SupplierService
    if (supplierService != nullptr) {
        Name contact(req.contactPerson, "", "");
        Supplier sup(supplierId, req.companyName, req.taxCode, contact, req.phone, req.email, req.address);
        std::string serviceErr;
        if (!supplierService->addSupplier(sup, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu nhà cung cấp vào file dữ liệu!");
        }
    }

    store.suppliers.push_back(newS);
    return OperationResult::Ok("Thêm nhà cung cấp '" + newS.companyName + "' thành công!");
}

OperationResult AppController::updateSupplier(const SupplierFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(req.id, "SUP")) {
        return OperationResult::Fail("Mã nhà cung cấp không đúng định dạng SUP-000001!");
    }

    // Cập nhật xuống file qua SupplierService
    if (supplierService != nullptr) {
        Name contact(req.contactPerson, "", "");
        Supplier updated(req.id, req.companyName, req.taxCode, contact, req.phone, req.email, req.address);
        std::string serviceErr;
        if (!supplierService->updateSupplier(updated, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật nhà cung cấp vào file dữ liệu!");
        }
    }

    auto& store = MockDataStore::getInstance();
    for (auto& s : store.suppliers) {
        if (s.id == req.id) {
            s.companyName = req.companyName;
            s.taxCode = req.taxCode;
            s.contactPerson = req.contactPerson;
            s.phone = req.phone;
            s.email = req.email;
            s.address = req.address;
            return OperationResult::Ok("Cập nhật thông tin nhà cung cấp thành công!");
        }
    }
    return OperationResult::Fail("Không tìm thấy nhà cung cấp!");
}

OperationResult AppController::deleteSupplier(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(id, "SUP")) {
        return OperationResult::Fail("Mã nhà cung cấp không đúng định dạng SUP-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (const auto& receipt : store.receipts) {
        if (receipt.supplierId == id) {
            return OperationResult::Fail("Không thể xóa nhà cung cấp đang được tham chiếu trong phiếu nhập (" + receipt.id + ")!");
        }
    }

    // Xóa khỏi file qua SupplierService
    if (supplierService != nullptr) {
        std::string serviceErr;
        if (!supplierService->deleteSupplier(id, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi xóa nhà cung cấp khỏi file dữ liệu!");
        }
    }

    for (auto it = store.suppliers.begin(); it != store.suppliers.end(); ++it) {
        if (it->id == id) {
            std::string name = it->companyName;
            store.suppliers.erase(it);
            return OperationResult::Ok("Đã xóa nhà cung cấp '" + name + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy nhà cung cấp để xóa!");
}

std::vector<GoodsReceiptSummaryViewModel> AppController::getGoodsReceipts() {
    return MockDataStore::getInstance().receipts;
}

OperationResult AppController::createGoodsReceipt(const CreateReceiptRequest& req) {
    REQUIRE_ADMIN_OR_WAREHOUSE_ROLE();
    if (!IdGenerator::isValidId(req.supplierId, "SUP")) {
        return OperationResult::Fail("Vui lòng chọn nhà cung cấp!");
    }
    if (req.items.empty()) {
        return OperationResult::Fail("Vui lòng chọn ít nhất 1 sản phẩm để nhập kho!");
    }

    auto& store = MockDataStore::getInstance();
    bool supplierExists = false;
    for (const auto& supplier : store.suppliers) {
        if (supplier.id == req.supplierId) {
            supplierExists = true;
            break;
        }
    }
    if (!supplierExists) {
        return OperationResult::Fail("Không tìm thấy nhà cung cấp được tham chiếu!");
    }
    const std::string employeeId =
        req.employeeId.empty() ? currentSession.userId : req.employeeId;
    bool employeeExists = false;
    for (const auto& employee : store.employees) {
        if (employee.id == employeeId) {
            employeeExists = true;
            break;
        }
    }
    if (!IdGenerator::isValidId(employeeId, "EMP") || !employeeExists) {
        return OperationResult::Fail("Nhân viên lập phiếu nhập không hợp lệ hoặc không tồn tại!");
    }
    for (const auto& item : req.items) {
        if (!IdGenerator::isValidId(item.phoneId, "PHN") || item.quantity <= 0 ||
            item.importPrice < 0 || getProductById(item.phoneId).id.empty()) {
            return OperationResult::Fail("Chi tiết phiếu nhập chứa sản phẩm hoặc số lượng không hợp lệ!");
        }
        if (supplierService != nullptr && !supplierService->isProductSuppliedBy(req.supplierId, item.phoneId)) {
            return OperationResult::Fail("Sản phẩm [" + item.phoneName + "] không thuộc danh mục cung cấp của nhà cung cấp đã chọn!");
        }
    }
    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    std::string receiptId;
    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("IMP", idDate, collectIds(store.receipts), receiptId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã phiếu nhập." : idError);
    }

    // Lưu phiếu nhập và cập nhật tồn kho qua SupplierService & PhoneService
    if (supplierService != nullptr) {
        ImportReceipt receipt(receiptId, req.supplierId, employeeId);
        for (const auto& item : req.items) {
            receipt.addDetail(OrderDetail(item.phoneId, item.phoneName, item.quantity, item.importPrice));
        }
        std::string serviceErr;
        if (!supplierService->receiveStock(receipt, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi tạo phiếu nhập kho!");
        }
    }

    GoodsReceiptSummaryViewModel r;
    r.id = receiptId;
    r.supplierId = req.supplierId;
    r.employeeId = employeeId;
    r.employeeName = currentSession.fullName.empty() ? "Thủ kho" : currentSession.fullName;
    r.importTime = timeBuf;
    r.totalAmount = 0.0;

    for (const auto& sup : store.suppliers) {
        if (sup.id == req.supplierId) {
            r.supplierName = sup.companyName;
            break;
        }
    }

    for (const auto& item : req.items) {
        GoodsReceiptDetailViewModel d = item;
        d.subtotal = item.importPrice * item.quantity;
        r.totalAmount += d.subtotal;
        r.details.push_back(d);

        // Tăng tồn kho thực tế và cập nhật giá nhập mới
        for (auto& p : store.products) {
            if (p.id == item.phoneId) {
                p.actualStock += item.quantity;
                p.importPrice = item.importPrice;
                p.calculateStockStatus();
                break;
            }
        }
    }

    store.receipts.insert(store.receipts.begin(), r);
    return OperationResult::Ok("Tạo phiếu nhập kho thành công! Mã phiếu: " + r.id);
}
