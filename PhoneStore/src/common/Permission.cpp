#include "Permission.h"

// Bảng quyền: chỉ mã hóa những gì đặc tả (mục 7) đã nêu rõ.
// Các hành động chưa được chủ dự án chốt vai trò: tạm chỉ ADMIN (xem docs/OPEN_QUESTIONS.md).
bool Permission::isAllowed(Role role, Action action) {
    if (role == Role::ADMIN) return true;

    switch (action) {
        case Action::ViewCatalog:
            return true; // mọi vai trò xem được danh mục
        case Action::CreateOrder:
            return role == Role::CUSTOMER;
        case Action::ConfirmOrder:
        case Action::TakePayment:
        case Action::CreateCounterSale:
            return role == Role::SALE_EMPLOYEE;
        case Action::PrepareOrder:
        case Action::ShipOrder:
            return role == Role::WAREHOUSE_EMPLOYEE;
        case Action::CancelOrder:
            // Customer chỉ được hủy đơn của chính mình: việc kiểm tra sở hữu nằm ở OrderService.
            return role == Role::CUSTOMER || role == Role::SALE_EMPLOYEE;
        case Action::ManagePhone:
        case Action::ManageCustomer:
        case Action::ManageEmployee:
        case Action::ManageDiscount:
        case Action::ManageSupplier:
        case Action::CreateImportReceipt:
        case Action::CreateInvoice:
        case Action::ViewReport:
        case Action::BackupRestore:
        case Action::ImportExport:
        case Action::AssignRole:
            // TODO(logic): chủ dự án chưa chốt quyền cho Sales/Warehouse ở các hành động này.
            return false;
    }
    return false;
}
