#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === KHÁCH HÀNG ===

std::vector<CustomerItemViewModel>
AppController::getCustomers(const std::string &keyword) {
  if (!currentSession.isAuthenticated ||
      (!currentSession.isAdmin() && !currentSession.isSales()) ||
      customerService == nullptr) {
    return {};
  }
  const Vector<Customer> &customers = customerService->getAllCustomers();
  std::vector<CustomerItemViewModel> result;
  result.reserve(static_cast<size_t>(customers.getSize()));
  for (int i = 0; i < customers.getSize(); ++i) {
    CustomerItemViewModel c = customerToViewModel(customers[i]);
    if (!keyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(c.fullName, keyword) &&
          !Utf8Helper::containsCaseInsensitive(c.id, keyword) &&
          !Utf8Helper::containsCaseInsensitive(c.phone, keyword)) {
        continue;
      }
    }
    result.push_back(c);
  }
  return result;
}

CustomerItemViewModel AppController::getCustomerById(const std::string &id) {
  if (!currentSession.isAuthenticated || currentSession.isWarehouse() ||
      (currentSession.isCustomer() && currentSession.userId != id) ||
      customerService == nullptr) {
    return CustomerItemViewModel();
  }
  Customer *c = customerService->getCustomerById(id);
  return (c != nullptr) ? customerToViewModel(*c) : CustomerItemViewModel();
}

OperationResult AppController::addCustomer(const CustomerFormRequest &req) {
  REQUIRE_ADMIN_OR_SALES_ROLE();
  std::string phone = trimCopy(req.phone);
  if (phone.empty() || req.firstName.empty() || req.lastName.empty()) {
    return OperationResult::Fail("Họ, tên và số điện thoại là bắt buộc!");
  }
  if (!isValidPhone(phone)) {
    return OperationResult::Fail("Số điện thoại không hợp lệ! Phải gồm đúng 10 "
                                 "chữ số và bắt đầu bằng 0.");
  }
  if (customerService == nullptr) {
    return OperationResult::Fail("Dịch vụ khách hàng chưa sẵn sàng!");
  }

  if (employeeService != nullptr &&
      employeeService->getEmployeeByPhone(phone) != nullptr) {
    return OperationResult::Fail(
        "Số điện thoại '" + phone +
        "' đã được sử dụng bởi nhân viên khác trong hệ thống!");
  }
  if (customerService->getCustomerByPhone(phone) != nullptr) {
    return OperationResult::Fail("Số điện thoại '" + phone +
                                 "' đã được đăng ký bởi khách hàng khác!");
  }

  std::string customerId;
  std::string idError;
  if (!IdGenerator::nextId("CUS", "",
                           collectModelIds(customerService->getAllCustomers()),
                           customerId, idError)) {
    return OperationResult::Fail(idError);
  }
  std::string accountId;
  if (!IdGenerator::nextId("ACC", "",
                           collectAllAccountIds(employeeService, customerService),
                           accountId, idError)) {
    return OperationResult::Fail(idError);
  }

  const std::string username =
      req.username.empty() ? ("cust_" + phone) : req.username;
  Name name(req.firstName, req.middleName, req.lastName);
  Customer cust(customerId, name, phone, req.email, req.address,
                username, "123456", 0);
  cust.getAccount().setID(accountId);
  std::string serviceErr;
  if (!customerService->registerCustomer(cust, serviceErr)) {
    return OperationResult::Fail(
        !serviceErr.empty() ? serviceErr
                            : "Lỗi khi lưu khách hàng vào file dữ liệu!");
  }

  return OperationResult::Ok("Thêm khách hàng '" + name.getFullName() +
                             "' thành công!");
}

OperationResult AppController::updateCustomer(const CustomerFormRequest &req) {
  REQUIRE_ADMIN_OR_SALES_ROLE();
  std::string phone = trimCopy(req.phone);
  if (!IdGenerator::isValidId(req.id, "CUS") || phone.empty()) {
    return OperationResult::Fail("Thông tin khách hàng không hợp lệ!");
  }
  if (!isValidPhone(phone)) {
    return OperationResult::Fail("Số điện thoại không hợp lệ! Phải gồm đúng 10 "
                                 "chữ số và bắt đầu bằng 0.");
  }
  if (customerService == nullptr) {
    return OperationResult::Fail("Dịch vụ khách hàng chưa sẵn sàng!");
  }

  if (employeeService != nullptr &&
      employeeService->getEmployeeByPhone(phone) != nullptr) {
    return OperationResult::Fail(
        "Số điện thoại '" + phone +
        "' đã được sử dụng bởi nhân viên khác trong hệ thống!");
  }
  Customer *matchCust = customerService->getCustomerByPhone(phone);
  if (matchCust != nullptr && matchCust->getID() != req.id) {
    return OperationResult::Fail("Số điện thoại '" + phone +
                                 "' đã được đăng ký cho khách hàng khác!");
  }

  Customer *existing = customerService->getCustomerById(req.id);
  if (existing == nullptr) {
    return OperationResult::Fail("Không tìm thấy khách hàng!");
  }

  Name name(req.firstName, req.middleName, req.lastName);
  Customer updated(req.id, name, phone, req.email, req.address,
                   existing->getAccount().getUsername(),
                   existing->getAccount().getPasswordHash(),
                   existing->getPoints());
  updated.getAccount().setID(existing->getAccount().getID());
  updated.getAccount().setActive(existing->getAccount().isActive());
  std::string serviceErr;
  if (!customerService->updateCustomer(updated, serviceErr)) {
    return OperationResult::Fail(
        !serviceErr.empty()
            ? serviceErr
            : "Lỗi khi cập nhật khách hàng vào file dữ liệu!");
  }

  return OperationResult::Ok("Cập nhật thông tin khách hàng thành công!");
}

OperationResult AppController::deleteCustomer(const std::string &id) {
  REQUIRE_ADMIN_ROLE();
  if (!IdGenerator::isValidId(id, "CUS")) {
    return OperationResult::Fail(
        "Mã khách hàng không đúng định dạng CUS-000001!");
  }
  if (customerService == nullptr) {
    return OperationResult::Fail("Dịch vụ khách hàng chưa sẵn sàng!");
  }

  if (orderService != nullptr) {
    const Vector<Order> &orders = orderService->getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
      if (orders[i].getCustomerId() == id) {
        return OperationResult::Fail(
            "Không thể xóa khách hàng đang được tham chiếu trong đơn hàng (" +
            orders[i].getID() + ")!");
      }
    }
  }
  if (invoiceService != nullptr) {
    const Vector<Invoice> &invoices = invoiceService->getAll();
    for (int i = 0; i < invoices.getSize(); ++i) {
      if (invoices[i].getCustomerId() == id) {
        return OperationResult::Fail(
            "Không thể xóa khách hàng đang được tham chiếu trong hóa đơn (" +
            invoices[i].getID() + ")!");
      }
    }
  }
  if (warrantyService != nullptr) {
    const Vector<WarrantyRequest> &warranties = warrantyService->getAll();
    for (int i = 0; i < warranties.getSize(); ++i) {
      if (warranties[i].getCustomerId() == id) {
        return OperationResult::Fail("Không thể xóa khách hàng đang được tham "
                                     "chiếu trong yêu cầu bảo hành (" +
                                     warranties[i].getId() + ")!");
      }
    }
  }

  Customer *existing = customerService->getCustomerById(id);
  if (existing == nullptr) {
    return OperationResult::Fail("Không tìm thấy khách hàng để xóa!");
  }
  const std::string name = existing->getName().getFullName();

  std::string serviceErr;
  if (!customerService->deleteCustomer(id, serviceErr)) {
    return OperationResult::Fail(
        !serviceErr.empty() ? serviceErr
                            : "Lỗi khi xóa khách hàng khỏi file dữ liệu!");
  }

  return OperationResult::Ok("Đã xóa khách hàng '" + name + "'!");
}
