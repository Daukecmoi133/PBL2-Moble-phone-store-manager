#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === TÀI KHOẢN & PHÂN QUYỀN ===

std::vector<AccountItemViewModel> AppController::getAccounts() {
    auto& store = MockDataStore::getInstance();
    if (employeeService != nullptr || customerService != nullptr) {
        store.accounts.clear();
        if (employeeService != nullptr) {
            const Vector<Employee*>& employees = employeeService->getAllEmployees();
            for (int i = 0; i < employees.getSize(); ++i) {
                if (employees[i] != nullptr) {
                    store.accounts.push_back(employeeToAccountViewModel(employees[i]));
                }
            }
        }
        if (customerService != nullptr) {
            const Vector<Customer>& customers = customerService->getAllCustomers();
            for (int i = 0; i < customers.getSize(); ++i) {
                store.accounts.push_back(customerToAccountViewModel(customers[i]));
            }
        }
    }
    return store.accounts;
}

OperationResult AppController::toggleAccountStatus(const std::string& username) {
    REQUIRE_ADMIN_ROLE();
    if (username == currentSession.username) {
        return OperationResult::Fail("Không thể khóa tài khoản của chính bạn!");
    }

    bool found = false;
    bool newStatus = false;
    std::string err;

    if (employeeService != nullptr) {
        Employee* emp = employeeService->getEmployeeByUsername(username);
        if (emp != nullptr) {
            found = true;
            newStatus = !emp->getAccount().isActive();
            if (!employeeService->setAccountActive(emp->getID(), newStatus, err)) {
                return OperationResult::Fail(!err.empty() ? err : "Lỗi khi cập nhật trạng thái tài khoản vào cơ sở dữ liệu!");
            }
        }
    }

    if (!found && customerService != nullptr) {
        Customer* cust = customerService->getCustomerByUsername(username);
        if (cust != nullptr) {
            found = true;
            newStatus = !cust->getAccount().isActive();
            if (!customerService->setAccountActive(cust->getID(), newStatus, err)) {
                return OperationResult::Fail(!err.empty() ? err : "Lỗi khi cập nhật trạng thái tài khoản vào cơ sở dữ liệu!");
            }
        }
    }

    auto& store = MockDataStore::getInstance();
    for (auto& a : store.accounts) {
        if (a.username == username) {
            if (!found) {
                found = true;
                newStatus = !a.isActive;
            }
            a.isActive = newStatus;
            break;
        }
    }

    if (!found) {
        return OperationResult::Fail("Không tìm thấy tài khoản!");
    }

    std::string msg = newStatus ? ("Đã mở khóa tài khoản '" + username + "'")
                                : ("Đã khóa tài khoản '" + username + "'");
    return OperationResult::Ok(msg);
}

OperationResult AppController::changeAccountRole(const std::string& username, RoleType newRole) {
    REQUIRE_ADMIN_ROLE();
    if (username == currentSession.username) {
        return OperationResult::Fail("Không thể tự đổi quyền tài khoản đang đăng nhập!");
    }

    auto& store = MockDataStore::getInstance();
    for (auto& a : store.accounts) {
        if (a.username == username) {
            a.role = newRole;
            a.roleName = roleTypeToString(newRole);
            return OperationResult::Ok("Đã đổi quyền tài khoản '" + username + "' thành " + a.roleName);
        }
    }
    return OperationResult::Fail("Không tìm thấy tài khoản!");
}

// === BẢO HÀNH ===

std::vector<WarrantyItemViewModel> AppController::getWarranties() {
    return MockDataStore::getInstance().warranties;
}

std::vector<WarrantyItemViewModel> AppController::getCustomerWarranties(const std::string& customerId) {
    auto& store = MockDataStore::getInstance();
    std::vector<WarrantyItemViewModel> result;
    for (const auto& w : store.warranties) {
        if (w.customerId == customerId || customerId.empty()) {
            result.push_back(w);
        }
    }
    return result;
}

OperationResult AppController::createWarranty(const CreateWarrantyRequest& req) {
    if (!IdGenerator::isValidId(req.phoneId, "PHN") || req.issueDescription.empty()) {
        return OperationResult::Fail("Vui lòng chọn sản phẩm và mô tả lỗi gặp phải!");
    }

    auto& store = MockDataStore::getInstance();
    if (getProductById(req.phoneId).id.empty()) {
        return OperationResult::Fail("Không tìm thấy sản phẩm được tham chiếu!");
    }
    const std::string customerId = req.customerId.empty() ? currentSession.userId : req.customerId;
    if (!customerId.empty() &&
        (!IdGenerator::isValidId(customerId, "CUS") || getCustomerById(customerId).id.empty())) {
        return OperationResult::Fail("Khách hàng được tham chiếu không hợp lệ hoặc không tồn tại!");
    }
    if (!req.orderId.empty() &&
        (!IdGenerator::isValidId(req.orderId, "ORD", true) || getOrderById(req.orderId).id.empty())) {
        return OperationResult::Fail("Đơn hàng được tham chiếu không hợp lệ hoặc không tồn tại!");
    }
    std::string warrantyRequestId;
    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("WREQ", idDate, collectIds(store.warranties),
                             warrantyRequestId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã yêu cầu bảo hành." : idError);
    }

    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    ProductItemViewModel p = getProductById(req.phoneId);

    WarrantyItemViewModel w;
    w.id = warrantyRequestId;
    w.orderId = req.orderId;
    w.phoneId = req.phoneId;
    w.phoneName = p.name.empty() ? ("Sản phẩm #" + req.phoneId) : p.name;
    w.customerId = customerId;
    w.customerName = currentSession.fullName.empty() ? "Khách hàng" : currentSession.fullName;
    w.customerPhone = currentSession.phone;
    w.requestDate = timeBuf;
    w.issueDescription = req.issueDescription;
    w.status = WarrantyStatus::RECEIVED;
    w.statusText = warrantyStatusToString(w.status);
    w.technicianNote = "Chờ kỹ thuật viên phân công kiểm tra";
    w.resolution = "Đang xử lý";

    store.warranties.insert(store.warranties.begin(), w);
    return OperationResult::Ok("Gửi yêu cầu bảo hành thành công! Mã phiếu: " + w.id);
}

OperationResult AppController::updateWarrantyStatus(const std::string& id, WarrantyStatus status,
                                                   const std::string& technicianNote, const std::string& resolution) {
    if (!IdGenerator::isValidId(id, "WREQ", true)) {
        return OperationResult::Fail("Mã yêu cầu bảo hành không đúng định dạng WREQ-ddmmyyyy-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (auto& w : store.warranties) {
        if (w.id == id) {
            w.status = status;
            w.statusText = warrantyStatusToString(status);
            if (!technicianNote.empty()) w.technicianNote = technicianNote;
            if (!resolution.empty()) w.resolution = resolution;
            return OperationResult::Ok("Cập nhật tiến độ bảo hành " + id + " thành công!");
        }
    }
    return OperationResult::Fail("Không tìm thấy phiếu bảo hành!");
}
