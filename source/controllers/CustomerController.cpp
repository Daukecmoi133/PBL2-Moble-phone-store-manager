#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === KHÁCH HÀNG ===

std::vector<CustomerItemViewModel> AppController::getCustomers(const std::string& keyword) {
    auto& store = MockDataStore::getInstance();
    if (keyword.empty()) return store.customers;

    std::vector<CustomerItemViewModel> result;
    for (const auto& c : store.customers) {
        if (Utf8Helper::containsCaseInsensitive(c.fullName, keyword) ||
            Utf8Helper::containsCaseInsensitive(c.id, keyword) ||
            Utf8Helper::containsCaseInsensitive(c.phone, keyword)) {
            result.push_back(c);
        }
    }
    return result;
}

CustomerItemViewModel AppController::getCustomerById(const std::string& id) {
    auto& store = MockDataStore::getInstance();
    for (const auto& c : store.customers) {
        if (c.id == id) return c;
    }
    return CustomerItemViewModel();
}

OperationResult AppController::addCustomer(const CustomerFormRequest& req) {
    if (req.phone.empty() || req.firstName.empty() || req.lastName.empty()) {
        return OperationResult::Fail("Họ, tên và số điện thoại là bắt buộc!");
    }

    auto& store = MockDataStore::getInstance();
    for (const auto& c : store.customers) {
        if (c.phone == req.phone) {
            return OperationResult::Fail("Số điện thoại '" + req.phone + "' đã được đăng ký bởi khách hàng khác!");
        }
    }

    std::string customerId;
    std::string idError;
    if (!IdGenerator::nextId("CUS", "", collectIds(store.customers), customerId, idError)) {
        return OperationResult::Fail(idError);
    }
    std::string accountId;
    if (!IdGenerator::nextId("ACC", "", collectIds(store.accounts), accountId, idError)) {
        return OperationResult::Fail(idError);
    }

    CustomerItemViewModel newC;
    newC.id = customerId;
    newC.firstName = req.firstName;
    newC.middleName = req.middleName;
    newC.lastName = req.lastName;
    newC.fullName = Name(req.firstName, req.middleName, req.lastName).getFullName();
    newC.phone = req.phone;
    newC.email = req.email;
    newC.address = req.address;
    newC.username = req.username.empty() ? ("cust_" + req.phone) : req.username;
    newC.points = 0;
    newC.updateTier();

    // Ghi xuống file qua CustomerService
    if (customerService != nullptr) {
        Name name(req.firstName, req.middleName, req.lastName);
        Customer cust(customerId, name, req.phone, req.email, req.address, newC.username, "123456", 0);
        cust.getAccount().setID(accountId);
        std::string serviceErr;
        if (!customerService->registerCustomer(cust, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi lưu khách hàng vào file dữ liệu!");
        }
    }

    store.customers.push_back(newC);
    AccountItemViewModel account;
    account.id = accountId;
    account.username = newC.username;
    account.role = RoleType::CUSTOMER;
    account.roleName = roleTypeToString(account.role);
    account.isActive = true;
    account.linkedEntityId = newC.id;
    account.linkedFullName = newC.fullName;
    store.accounts.push_back(account);
    return OperationResult::Ok("Thêm khách hàng '" + newC.fullName + "' thành công!");
}

OperationResult AppController::updateCustomer(const CustomerFormRequest& req) {
    if (!IdGenerator::isValidId(req.id, "CUS") || req.phone.empty()) {
        return OperationResult::Fail("Thông tin khách hàng không hợp lệ!");
    }

    // Cập nhật xuống file qua CustomerService
    if (customerService != nullptr) {
        Customer* existing = customerService->getCustomerById(req.id);
        std::string username = existing ? existing->getAccount().getUsername() : ("cust_" + req.phone);
        std::string password = existing ? existing->getAccount().getPasswordHash() : "123456";
        int pts = existing ? existing->getPoints() : 0;
        std::string accId = existing ? existing->getAccount().getID() : "";
        Name name(req.firstName, req.middleName, req.lastName);
        Customer updated(req.id, name, req.phone, req.email, req.address, username, password, pts);
        updated.getAccount().setID(accId);
        if (existing) {
            updated.getAccount().setActive(existing->getAccount().isActive());
        }
        std::string serviceErr;
        if (!customerService->updateCustomer(updated, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi cập nhật khách hàng vào file dữ liệu!");
        }
    }

    auto& store = MockDataStore::getInstance();
    for (auto& c : store.customers) {
        if (c.id == req.id) {
            c.firstName = req.firstName;
            c.middleName = req.middleName;
            c.lastName = req.lastName;
            c.fullName = Name(req.firstName, req.middleName, req.lastName).getFullName();
            c.phone = req.phone;
            c.email = req.email;
            c.address = req.address;
            return OperationResult::Ok("Cập nhật thông tin khách hàng thành công!");
        }
    }
    return OperationResult::Fail("Không tìm thấy khách hàng!");
}

OperationResult AppController::deleteCustomer(const std::string& id) {
    if (!IdGenerator::isValidId(id, "CUS")) {
        return OperationResult::Fail("Mã khách hàng không đúng định dạng CUS-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (const auto& order : store.orders) {
        if (order.customerId == id) {
            return OperationResult::Fail("Không thể xóa khách hàng đang được tham chiếu trong đơn hàng (" + order.id + ")!");
        }
    }
    for (const auto& invoice : store.invoices) {
        if (invoice.customerId == id) {
            return OperationResult::Fail("Không thể xóa khách hàng đang được tham chiếu trong hóa đơn (" + invoice.id + ")!");
        }
    }
    for (const auto& warranty : store.warranties) {
        if (warranty.customerId == id) {
            return OperationResult::Fail("Không thể xóa khách hàng đang được tham chiếu trong yêu cầu bảo hành (" + warranty.id + ")!");
        }
    }

    // Xóa khỏi file qua CustomerService
    if (customerService != nullptr) {
        std::string serviceErr;
        if (!customerService->deleteCustomer(id, serviceErr)) {
            return OperationResult::Fail(!serviceErr.empty() ? serviceErr : "Lỗi khi xóa khách hàng khỏi file dữ liệu!");
        }
    }

    for (auto it = store.customers.begin(); it != store.customers.end(); ++it) {
        if (it->id == id) {
            std::string name = it->fullName;
            store.customers.erase(it);
            return OperationResult::Ok("Đã xóa khách hàng '" + name + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy khách hàng để xóa!");
}
