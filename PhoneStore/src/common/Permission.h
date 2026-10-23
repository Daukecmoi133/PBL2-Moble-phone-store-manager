#pragma once
// Danh sách hành động và bảng quyền theo vai trò. Phân quyền thật nằm ở Service; View chỉ ẩn nút.

#include "../model/enums/Role.h"

enum class Action {
    ViewCatalog, ManagePhone, ManageCustomer, ManageEmployee, ManageDiscount,
    CreateOrder, ConfirmOrder, PrepareOrder, ShipOrder, CancelOrder, TakePayment,
    CreateCounterSale, ManageSupplier, CreateImportReceipt, CreateInvoice, ViewReport,
    BackupRestore, ImportExport, AssignRole
};

struct Permission {
    // Bảng quyền nằm trong Permission.cpp
    static bool isAllowed(Role role, Action action);
};
