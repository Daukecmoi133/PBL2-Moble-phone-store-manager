#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === NHÂN VIÊN ===

std::vector<EmployeeItemViewModel> AppController::getEmployees(const std::string& keyword) {
    if (currentSession.isAuthenticated && !currentSession.isAdmin()) {
        return {};
    }
    auto& store = MockDataStore::getInstance();
    if (keyword.empty()) return store.employees;

    std::vector<EmployeeItemViewModel> result;
    for (const auto& e : store.employees) {
        if (Utf8Helper::containsCaseInsensitive(e.fullName, keyword) ||
            Utf8Helper::containsCaseInsensitive(e.id, keyword) ||
            Utf8Helper::containsCaseInsensitive(e.phone, keyword)) {
            result.push_back(e);
        }
    }
    return result;
}

EmployeeItemViewModel AppController::getEmployeeById(const std::string& id) {
    if (currentSession.isAuthenticated && !currentSession.isAdmin() && currentSession.userId != id) {
        return EmployeeItemViewModel();
    }
    auto& store = MockDataStore::getInstance();
    for (const auto& e : store.employees) {
        if (e.id == id) return e;
    }
    return EmployeeItemViewModel();
}

OperationResult AppController::addEmployee(const EmployeeFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (req.firstName.empty() || req.lastName.empty() || req.username.empty()) {
        return OperationResult::Fail("Họ tên và tên đăng nhập không được để trống!");
    }

    auto& store = MockDataStore::getInstance();
    for (const auto& e : store.employees) {
        if (e.username == req.username) {
            return OperationResult::Fail("Tên đăng nhập '" + req.username + "' đã tồn tại!");
        }
    }

    std::string employeeId;
    std::string idError;
    if (!IdGenerator::nextId("EMP", "", collectIds(store.employees), employeeId, idError)) {
        return OperationResult::Fail(idError);
    }
    std::string accountId;
    if (!IdGenerator::nextId("ACC", "", collectIds(store.accounts), accountId, idError)) {
        return OperationResult::Fail(idError);
    }

    EmployeeItemViewModel newE;
    newE.id = employeeId;
    newE.firstName = req.firstName;
    newE.middleName = req.middleName;
    newE.lastName = req.lastName;
    newE.fullName = Name(req.firstName, req.middleName, req.lastName).getFullName();
    newE.phone = req.phone;
    newE.email = req.email;
    newE.address = req.address;
    newE.username = req.username;
    newE.role = req.role;
    newE.roleName = roleTypeToString(req.role);
    newE.salary = req.salary;
    newE.isActive = true;

    // Lưu xuống file qua EmployeeService
    if (employeeService != nullptr) {
        Name name(req.firstName, req.middleName, req.lastName);
        Employee* emp = nullptr;
        if (req.role == RoleType::ADMIN) {
            emp = new AdminEmployee(employeeId, name, req.phone, req.email, req.address, req.username, "123456", req.salary);
        } else if (req.role == RoleType::WAREHOUSE_EMPLOYEE) {
            emp = new WarehouseEmployee(employeeId, name, req.phone, req.email, req.address, req.username, "123456", req.salary);
        } else {
            emp = new SalesEmployee(employeeId, name, req.phone, req.email, req.address, req.username, "123456", req.salary);
        }
        emp->getAccount().setID(accountId);
        std::string serviceErr;
        if (!employeeService->addEmployee(emp, serviceErr)) {
            delete emp;
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu nhân viên vào file dữ liệu!");
        }
    }

    store.employees.push_back(newE);
    AccountItemViewModel account;
    account.id = accountId;
    account.username = newE.username;
    account.role = newE.role;
    account.roleName = newE.roleName;
    account.isActive = newE.isActive;
    account.linkedEntityId = newE.id;
    account.linkedFullName = newE.fullName;
    store.accounts.push_back(account);
    return OperationResult::Ok("Thêm nhân viên '" + newE.fullName + "' thành công!");
}

OperationResult AppController::updateEmployee(const EmployeeFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(req.id, "EMP")) {
        return OperationResult::Fail("Mã nhân viên không hợp lệ!");
    }

    // Cập nhật xuống file qua EmployeeService
    if (employeeService != nullptr) {
        Employee* existing = employeeService->getEmployeeById(req.id);
        std::string username = existing ? existing->getAccount().getUsername() : req.username;
        std::string password = existing ? existing->getAccount().getPasswordHash() : "123456";
        std::string accId = existing ? existing->getAccount().getID() : "";
        Name name(req.firstName, req.middleName, req.lastName);
        Employee* updated = nullptr;
        if (req.role == RoleType::ADMIN) {
            updated = new AdminEmployee(req.id, name, req.phone, req.email, req.address, username, password, req.salary);
        } else if (req.role == RoleType::WAREHOUSE_EMPLOYEE) {
            updated = new WarehouseEmployee(req.id, name, req.phone, req.email, req.address, username, password, req.salary);
        } else {
            updated = new SalesEmployee(req.id, name, req.phone, req.email, req.address, username, password, req.salary);
        }
        updated->getAccount().setID(accId);
        if (existing) {
            updated->getAccount().setActive(existing->getAccount().isActive());
        }
        std::string serviceErr;
        if (!employeeService->updateEmployee(updated, serviceErr)) {
            delete updated;
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật nhân viên vào file dữ liệu!");
        }
    }

    auto& store = MockDataStore::getInstance();
    for (auto& e : store.employees) {
        if (e.id == req.id) {
            e.firstName = req.firstName;
            e.middleName = req.middleName;
            e.lastName = req.lastName;
            e.fullName = Name(req.firstName, req.middleName, req.lastName).getFullName();
            e.phone = req.phone;
            e.email = req.email;
            e.address = req.address;
            e.role = req.role;
            e.roleName = roleTypeToString(req.role);
            e.salary = req.salary;
            return OperationResult::Ok("Cập nhật thông tin nhân viên thành công!");
        }
    }
    return OperationResult::Fail("Không tìm thấy nhân viên!");
}

OperationResult AppController::deleteEmployee(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(id, "EMP")) {
        return OperationResult::Fail("Mã nhân viên không đúng định dạng EMP-000001!");
    }
    if (id == currentSession.userId) {
        return OperationResult::Fail("Không thể xóa tài khoản của chính bạn đang đăng nhập!");
    }

    auto& store = MockDataStore::getInstance();
    for (const auto& invoice : store.invoices) {
        if (invoice.employeeId == id) {
            return OperationResult::Fail("Không thể xóa nhân viên đang được tham chiếu trong hóa đơn (" + invoice.id + ")!");
        }
    }
    for (const auto& receipt : store.receipts) {
        if (receipt.employeeId == id) {
            return OperationResult::Fail("Không thể xóa nhân viên đang được tham chiếu trong phiếu nhập (" + receipt.id + ")!");
        }
    }

    // Xóa khỏi file qua EmployeeService
    if (employeeService != nullptr) {
        std::string serviceErr;
        if (!employeeService->deleteEmployee(id, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi xóa nhân viên khỏi file dữ liệu!");
        }
    }

    for (auto it = store.employees.begin(); it != store.employees.end(); ++it) {
        if (it->id == id) {
            std::string name = it->fullName;
            store.employees.erase(it);
            return OperationResult::Ok("Đã xóa nhân viên '" + name + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy nhân viên!");
}
