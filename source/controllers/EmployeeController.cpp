#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === NHÂN VIÊN ===

std::vector<EmployeeItemViewModel> AppController::getEmployees(const std::string& keyword) {
    if (!currentSession.isAuthenticated || !currentSession.isAdmin() ||
        employeeService == nullptr) {
        return {};
    }
    const Vector<Employee*>& emps = employeeService->getAllEmployees();
    std::vector<EmployeeItemViewModel> result;
    result.reserve(static_cast<size_t>(emps.getSize()));
    for (int i = 0; i < emps.getSize(); ++i) {
        if (emps[i] == nullptr) continue;
        EmployeeItemViewModel e = employeeToViewModel(emps[i]);
        if (!keyword.empty()) {
            if (!Utf8Helper::containsCaseInsensitive(e.fullName, keyword) &&
                !Utf8Helper::containsCaseInsensitive(e.id, keyword) &&
                !Utf8Helper::containsCaseInsensitive(e.phone, keyword)) {
                continue;
            }
        }
        result.push_back(e);
    }
    return result;
}

EmployeeItemViewModel AppController::getEmployeeById(const std::string& id) {
    if (!currentSession.isAuthenticated || (!currentSession.isAdmin() && currentSession.userId != id) ||
        employeeService == nullptr) {
        return EmployeeItemViewModel();
    }
    Employee* e = employeeService->getEmployeeById(id);
    return (e != nullptr) ? employeeToViewModel(e) : EmployeeItemViewModel();
}

OperationResult AppController::addEmployee(const EmployeeFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    std::string targetUsername = trimCopy(req.username);
    if (targetUsername.empty()) {
        return OperationResult::Fail("Vui lòng chọn tài khoản cần cấp quyền nhân viên!");
    }
    if (employeeService == nullptr) {
        return OperationResult::Fail("Dịch vụ nhân sự chưa sẵn sàng!");
    }

    // 1. Nếu chỉ truyền username + role (luồng: Tìm tài khoản đã tạo -> Phân quyền thành nhân viên)
    if (req.firstName.empty() && req.lastName.empty()) {
        // Kiểm tra nếu tài khoản đã là nhân viên thì đổi role luôn
        if (Employee* existingEmp = employeeService->getEmployeeByUsername(targetUsername)) {
            if (existingEmp->getRoleType() == req.role) {
                return OperationResult::Fail("Tài khoản '" + targetUsername + "' đã là nhân viên với chức vụ " + roleTypeToString(req.role) + "!");
            }
            return changeAccountRole(targetUsername, req.role);
        }

        // Tìm tài khoản đã đăng ký (đang ở danh sách Khách hàng / Tài khoản hệ thống)
        Customer* existingCust = (customerService != nullptr)
                                     ? customerService->getCustomerByUsername(targetUsername)
                                     : nullptr;
        if (existingCust == nullptr) {
            return OperationResult::Fail("Không tìm thấy tài khoản '" + targetUsername + "' trong hệ thống! Vui lòng tạo tài khoản trước khi cấp quyền.");
        }

        std::string firstName = existingCust->getName().firstName;
        std::string middleName = existingCust->getName().middleName;
        std::string lastName = existingCust->getName().lastName;
        if (firstName.empty() && lastName.empty()) {
            firstName = targetUsername;
        }
        std::string phone = existingCust->getPhoneNumber();
        std::string email = existingCust->getEmail();
        std::string address = existingCust->getAddress();
        std::string passCred = existingCust->getAccount().getPasswordHash();
        std::string accountId = existingCust->getAccount().getID();
        bool isActive = existingCust->getAccount().isActive();
        std::string oldCustId = existingCust->getID();

        std::vector<std::string> empIds = collectPtrIds(employeeService->getAllEmployees());
        std::string employeeId;
        std::string idError;
        if (!IdGenerator::nextId("EMP", "", empIds, employeeId, idError)) {
            return OperationResult::Fail(idError);
        }

        if (!IdGenerator::isValidId(accountId, "ACC")) {
            if (!IdGenerator::nextId("ACC", "",
                                     collectAllAccountIds(employeeService, customerService),
                                     accountId, idError)) {
                return OperationResult::Fail(idError);
            }
        }

        double defaultSalary = (req.salary > 0)
                                   ? req.salary
                                   : (req.role == RoleType::ADMIN
                                          ? 15000000.0
                                          : (req.role == RoleType::SALE_EMPLOYEE ? 8000000.0 : 7500000.0));

        Name name(firstName, middleName, lastName);
        Employee* emp = nullptr;
        if (req.role == RoleType::ADMIN) {
            emp = new AdminEmployee(employeeId, name, phone, email, address, targetUsername, passCred, defaultSalary);
        } else if (req.role == RoleType::WAREHOUSE_EMPLOYEE) {
            emp = new WarehouseEmployee(employeeId, name, phone, email, address, targetUsername, passCred, defaultSalary);
        } else {
            emp = new SalesEmployee(employeeId, name, phone, email, address, targetUsername, passCred, defaultSalary);
        }
        emp->getAccount().setID(accountId);
        emp->getAccount().setActive(isActive);
        std::string serviceErr;
        if (!employeeService->addEmployee(emp, serviceErr)) {
            delete emp;
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu phân quyền nhân viên vào file dữ liệu!");
        }

        // Gỡ bản ghi khỏi bảng Khách hàng nếu chưa phát sinh đơn hàng/hóa đơn/bảo hành ràng buộc
        if (!oldCustId.empty() && customerService != nullptr) {
            std::string ignoreErr;
            customerService->deleteCustomer(oldCustId, ignoreErr);
        }

        return OperationResult::Ok("Đã cấp quyền '" + roleTypeToString(req.role) + "' cho tài khoản '" + targetUsername + "' (" + name.getFullName() + ") thành công!");
    }

    std::string empPhone = trimCopy(req.phone);
    if (req.firstName.empty() || req.lastName.empty() || targetUsername.empty() || empPhone.empty()) {
        return OperationResult::Fail("Họ tên, tên đăng nhập và số điện thoại không được để trống!");
    }
    if (!isValidPhone(empPhone)) {
        return OperationResult::Fail("Số điện thoại không hợp lệ! Phải gồm đúng 10 chữ số và bắt đầu bằng 0.");
    }

    if (employeeService->getEmployeeByUsername(targetUsername) != nullptr) {
        return OperationResult::Fail("Tên đăng nhập '" + targetUsername + "' đã tồn tại!");
    }
    if (customerService != nullptr && customerService->getCustomerByUsername(targetUsername) != nullptr) {
        return OperationResult::Fail("Tên đăng nhập '" + targetUsername + "' đã tồn tại trong danh sách khách hàng!");
    }
    if (employeeService->getEmployeeByPhone(empPhone) != nullptr) {
        return OperationResult::Fail("Số điện thoại '" + empPhone + "' đã được sử dụng bởi nhân viên khác!");
    }
    if (customerService != nullptr && customerService->getCustomerByPhone(empPhone) != nullptr) {
        return OperationResult::Fail("Số điện thoại '" + empPhone + "' đã được đăng ký bởi khách hàng khác!");
    }

    std::string employeeId;
    std::string idError;
    if (!IdGenerator::nextId("EMP", "",
                             collectPtrIds(employeeService->getAllEmployees()),
                             employeeId, idError)) {
        return OperationResult::Fail(idError);
    }
    std::string accountId;
    if (!IdGenerator::nextId("ACC", "",
                             collectAllAccountIds(employeeService, customerService),
                             accountId, idError)) {
        return OperationResult::Fail(idError);
    }

    Name name(req.firstName, req.middleName, req.lastName);
    Employee* emp = nullptr;
    if (req.role == RoleType::ADMIN) {
        emp = new AdminEmployee(employeeId, name, empPhone, req.email, req.address, targetUsername, "123456", req.salary);
    } else if (req.role == RoleType::WAREHOUSE_EMPLOYEE) {
        emp = new WarehouseEmployee(employeeId, name, empPhone, req.email, req.address, targetUsername, "123456", req.salary);
    } else {
        emp = new SalesEmployee(employeeId, name, empPhone, req.email, req.address, targetUsername, "123456", req.salary);
    }
    emp->getAccount().setID(accountId);
    std::string serviceErr;
    if (!employeeService->addEmployee(emp, serviceErr)) {
        delete emp;
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu nhân viên vào file dữ liệu!");
    }

    return OperationResult::Ok("Thêm nhân viên '" + name.getFullName() + "' thành công!");
}

OperationResult AppController::updateEmployee(const EmployeeFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    std::string empPhone = trimCopy(req.phone);
    if (!IdGenerator::isValidId(req.id, "EMP") || empPhone.empty()) {
        return OperationResult::Fail("Mã nhân viên và số điện thoại không hợp lệ!");
    }
    if (!isValidPhone(empPhone)) {
        return OperationResult::Fail("Số điện thoại không hợp lệ! Phải gồm đúng 10 chữ số và bắt đầu bằng 0.");
    }
    if (employeeService == nullptr) {
        return OperationResult::Fail("Dịch vụ nhân sự chưa sẵn sàng!");
    }

    Employee* matchEmp = employeeService->getEmployeeByPhone(empPhone);
    if (matchEmp != nullptr && matchEmp->getID() != req.id) {
        return OperationResult::Fail("Số điện thoại '" + empPhone + "' đã thuộc về nhân viên khác!");
    }
    if (customerService != nullptr && customerService->getCustomerByPhone(empPhone) != nullptr) {
        return OperationResult::Fail("Số điện thoại '" + empPhone + "' đã được đăng ký bởi khách hàng trong hệ thống!");
    }

    Employee* existing = employeeService->getEmployeeById(req.id);
    if (existing == nullptr) {
        return OperationResult::Fail("Không tìm thấy nhân viên!");
    }

    Name name(req.firstName, req.middleName, req.lastName);
    Employee* updated = nullptr;
    if (req.role == RoleType::ADMIN) {
        updated = new AdminEmployee(req.id, name, empPhone, req.email, req.address,
                                    existing->getAccount().getUsername(),
                                    existing->getAccount().getPasswordHash(), req.salary);
    } else if (req.role == RoleType::WAREHOUSE_EMPLOYEE) {
        updated = new WarehouseEmployee(req.id, name, empPhone, req.email, req.address,
                                        existing->getAccount().getUsername(),
                                        existing->getAccount().getPasswordHash(), req.salary);
    } else {
        updated = new SalesEmployee(req.id, name, empPhone, req.email, req.address,
                                    existing->getAccount().getUsername(),
                                    existing->getAccount().getPasswordHash(), req.salary);
    }
    updated->getAccount().setID(existing->getAccount().getID());
    updated->getAccount().setActive(existing->getAccount().isActive());
    std::string serviceErr;
    if (!employeeService->updateEmployee(updated, serviceErr)) {
        delete updated;
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật nhân viên vào file dữ liệu!");
    }

    return OperationResult::Ok("Cập nhật thông tin nhân viên thành công!");
}

OperationResult AppController::deleteEmployee(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(id, "EMP")) {
        return OperationResult::Fail("Mã nhân viên không đúng định dạng EMP-000001!");
    }
    if (id == currentSession.userId) {
        return OperationResult::Fail("Không thể xóa tài khoản của chính bạn đang đăng nhập!");
    }
    if (employeeService == nullptr) {
        return OperationResult::Fail("Dịch vụ nhân sự chưa sẵn sàng!");
    }

    if (invoiceService != nullptr) {
        const Vector<Invoice>& invoices = invoiceService->getAll();
        for (int i = 0; i < invoices.getSize(); ++i) {
            if (invoices[i].getEmployeeId() == id) {
                return OperationResult::Fail("Không thể xóa nhân viên đang được tham chiếu trong hóa đơn (" + invoices[i].getID() + ")!");
            }
        }
    }
    if (supplierService != nullptr) {
        const Vector<ImportReceipt>& receipts = supplierService->getAllReceipts();
        for (int i = 0; i < receipts.getSize(); ++i) {
            if (receipts[i].getEmployeeId() == id) {
                return OperationResult::Fail("Không thể xóa nhân viên đang được tham chiếu trong phiếu nhập (" + receipts[i].getID() + ")!");
            }
        }
    }

    Employee* existing = employeeService->getEmployeeById(id);
    if (existing == nullptr) {
        return OperationResult::Fail("Không tìm thấy nhân viên!");
    }
    const std::string name = existing->getName().getFullName();

    std::string serviceErr;
    if (!employeeService->deleteEmployee(id, serviceErr)) {
        return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi xóa nhân viên khỏi file dữ liệu!");
    }

    return OperationResult::Ok("Đã xóa nhân viên '" + name + "'!");
}
