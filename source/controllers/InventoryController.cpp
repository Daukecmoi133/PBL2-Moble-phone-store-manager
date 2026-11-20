#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === KHO & NHÀ CUNG CẤP ===

std::vector<ProductItemViewModel> AppController::getInventory(bool lowStockOnly) {
    if (!currentSession.isAuthenticated ||
        !(currentSession.isAdmin() || currentSession.isWarehouse())) {
        return {};
    }
    std::vector<ProductItemViewModel> all = getProducts();
    if (!lowStockOnly) return all;

    std::vector<ProductItemViewModel> lowStock;
    for (const auto& p : all) {
        if (p.status == StockDisplayStatus::LOW_STOCK || p.status == StockDisplayStatus::OUT_OF_STOCK) {
            lowStock.push_back(p);
        }
    }
    return lowStock;
}

std::vector<ProductItemViewModel> AppController::getProductsBySupplier(const std::string& supplierId) {
    if (!currentSession.isAuthenticated ||
        !(currentSession.isAdmin() || currentSession.isWarehouse())) {
        return {};
    }
    if (supplierService == nullptr) return {};
    std::vector<ProductItemViewModel> all = getProducts();
    std::vector<ProductItemViewModel> filtered;
    for (const auto& p : all) {
        if (supplierService->isProductSuppliedBy(supplierId, p.id)) {
            filtered.push_back(p);
        }
    }
    return filtered;
}

std::vector<SupplierItemViewModel> AppController::getSuppliers(const std::string& keyword) {
    if (!currentSession.isAuthenticated ||
        !(currentSession.isAdmin() || currentSession.isWarehouse())) {
        return {};
    }
    std::vector<SupplierItemViewModel> result;
    if (supplierService == nullptr) return result;
    const Vector<Supplier>& suppliers = supplierService->getAllSuppliers();
    for (int i = 0; i < suppliers.getSize(); ++i) {
        SupplierItemViewModel s = supplierToViewModel(suppliers[i]);
        if (keyword.empty() ||
            Utf8Helper::containsCaseInsensitive(s.companyName, keyword) ||
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
    if (supplierService == nullptr) {
        return OperationResult::Fail("Dịch vụ nhà cung cấp chưa sẵn sàng!");
    }

    std::string supplierId;
    std::string idError;
    if (!IdGenerator::nextId("SUP", "", collectModelIds(supplierService->getAllSuppliers()), supplierId, idError)) {
        return OperationResult::Fail(idError);
    }

    Name contact(req.contactPerson, "", "");
    Supplier sup(supplierId, req.companyName, req.taxCode, contact, req.phone, req.email, req.address);
    std::string serviceErr;
    if (!supplierService->addSupplier(sup, serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu nhà cung cấp vào file dữ liệu!");
    }

    return OperationResult::Ok("Thêm nhà cung cấp '" + req.companyName + "' thành công!");
}

OperationResult AppController::updateSupplier(const SupplierFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(req.id, "SUP")) {
        return OperationResult::Fail("Mã nhà cung cấp không đúng định dạng SUP-000001!");
    }
    if (supplierService == nullptr) {
        return OperationResult::Fail("Dịch vụ nhà cung cấp chưa sẵn sàng!");
    }
    if (supplierService->getSupplierById(req.id) == nullptr) {
        return OperationResult::Fail("Không tìm thấy nhà cung cấp!");
    }

    Name contact(req.contactPerson, "", "");
    Supplier updated(req.id, req.companyName, req.taxCode, contact, req.phone, req.email, req.address);
    std::string serviceErr;
    if (!supplierService->updateSupplier(updated, serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật nhà cung cấp vào file dữ liệu!");
    }

    return OperationResult::Ok("Cập nhật thông tin nhà cung cấp thành công!");
}

OperationResult AppController::deleteSupplier(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(id, "SUP")) {
        return OperationResult::Fail("Mã nhà cung cấp không đúng định dạng SUP-000001!");
    }
    if (supplierService == nullptr) {
        return OperationResult::Fail("Dịch vụ nhà cung cấp chưa sẵn sàng!");
    }

    Vector<ImportReceipt> refs = supplierService->getReceiptsBySupplierId(id);
    if (refs.getSize() > 0) {
        return OperationResult::Fail("Không thể xóa nhà cung cấp đang được tham chiếu trong phiếu nhập (" + refs[0].getID() + ")!");
    }

    Supplier* existing = supplierService->getSupplierById(id);
    if (existing == nullptr) {
        return OperationResult::Fail("Không tìm thấy nhà cung cấp để xóa!");
    }
    std::string name = existing->getCompanyName();

    std::string serviceErr;
    if (!supplierService->deleteSupplier(id, serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi xóa nhà cung cấp khỏi file dữ liệu!");
    }

    return OperationResult::Ok("Đã xóa nhà cung cấp '" + name + "'!");
}

std::vector<GoodsReceiptSummaryViewModel> AppController::getGoodsReceipts() {
    if (!currentSession.isAuthenticated ||
        !(currentSession.isAdmin() || currentSession.isWarehouse())) {
        return {};
    }
    std::vector<GoodsReceiptSummaryViewModel> result;
    if (supplierService == nullptr) return result;
    const Vector<ImportReceipt>& receipts = supplierService->getAllReceipts();
    result.reserve(static_cast<size_t>(receipts.getSize()));
    for (int i = receipts.getSize() - 1; i >= 0; --i) {
        result.push_back(receiptToViewModel(receipts[i], supplierService, employeeService));
    }
    return result;
}

OperationResult AppController::createGoodsReceipt(const CreateReceiptRequest& req) {
    REQUIRE_ADMIN_OR_WAREHOUSE_ROLE();
    if (!IdGenerator::isValidId(req.supplierId, "SUP")) {
        return OperationResult::Fail("Vui lòng chọn nhà cung cấp!");
    }
    if (req.items.empty()) {
        return OperationResult::Fail("Vui lòng chọn ít nhất 1 sản phẩm để nhập kho!");
    }
    if (supplierService == nullptr) {
        return OperationResult::Fail("Dịch vụ nhà cung cấp chưa sẵn sàng!");
    }

    if (supplierService->getSupplierById(req.supplierId) == nullptr) {
        return OperationResult::Fail("Không tìm thấy nhà cung cấp được tham chiếu!");
    }
    const std::string employeeId =
        req.employeeId.empty() ? currentSession.userId : req.employeeId;
    if (!IdGenerator::isValidId(employeeId, "EMP") ||
        employeeService == nullptr ||
        employeeService->getEmployeeById(employeeId) == nullptr) {
        return OperationResult::Fail("Nhân viên lập phiếu nhập không hợp lệ hoặc không tồn tại!");
    }
    for (const auto& item : req.items) {
        if (!IdGenerator::isValidId(item.phoneId, "PHN") || item.quantity <= 0 ||
            item.importPrice < 0 || getProductById(item.phoneId).id.empty()) {
            return OperationResult::Fail("Chi tiết phiếu nhập chứa sản phẩm hoặc số lượng không hợp lệ!");
        }
        if (!supplierService->isProductSuppliedBy(req.supplierId, item.phoneId)) {
            return OperationResult::Fail("Sản phẩm [" + item.phoneName + "] không thuộc danh mục cung cấp của nhà cung cấp đã chọn!");
        }
    }

    std::string receiptId;
    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("IMP", idDate, collectModelIds(supplierService->getAllReceipts()), receiptId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã phiếu nhập." : idError);
    }

    ImportReceipt receipt(receiptId, req.supplierId, employeeId);
    for (const auto& item : req.items) {
        receipt.addDetail(OrderDetail(item.phoneId, item.phoneName, item.quantity, item.importPrice));
    }
    std::string serviceErr;
    if (!supplierService->receiveStock(receipt, serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi tạo phiếu nhập kho!");
    }

    return OperationResult::Ok("Tạo phiếu nhập kho thành công! Mã phiếu: " + receiptId);
}
