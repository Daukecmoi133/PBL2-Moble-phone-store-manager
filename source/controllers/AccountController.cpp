#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === TÀI KHOẢN & PHÂN QUYỀN ===

std::vector<AccountItemViewModel> AppController::getAccounts() {
  if (!currentSession.isAuthenticated || !currentSession.isAdmin()) {
    return {};
  }
  std::vector<AccountItemViewModel> accounts;
  if (employeeService != nullptr) {
    const Vector<Employee *> &employees = employeeService->getAllEmployees();
    for (int i = 0; i < employees.getSize(); ++i) {
      if (employees[i] != nullptr) {
        accounts.push_back(employeeToAccountViewModel(employees[i]));
      }
    }
  }
  if (customerService != nullptr) {
    const Vector<Customer> &customers = customerService->getAllCustomers();
    for (int i = 0; i < customers.getSize(); ++i) {
      if (employeeService != nullptr &&
          employeeService->getEmployeeByUsername(
              customers[i].getAccount().getUsername()) != nullptr) {
        continue;
      }
      accounts.push_back(customerToAccountViewModel(customers[i]));
    }
  }
  return accounts;
}

OperationResult
AppController::toggleAccountStatus(const std::string &username) {
  REQUIRE_ADMIN_ROLE();
  if (username == currentSession.username) {
    return OperationResult::Fail("Không thể khóa tài khoản của chính bạn!");
  }

  bool found = false;
  bool newStatus = false;
  std::string err;

  if (employeeService != nullptr) {
    Employee *emp = employeeService->getEmployeeByUsername(username);
    if (emp != nullptr) {
      found = true;
      newStatus = !emp->getAccount().isActive();
      if (!employeeService->setAccountActive(emp->getID(), newStatus, err)) {
        return OperationResult::Fail(
            !err.empty()
                ? err
                : "Lỗi khi cập nhật trạng thái tài khoản vào cơ sở dữ liệu!");
      }
    }
  }

  if (!found && customerService != nullptr) {
    Customer *cust = customerService->getCustomerByUsername(username);
    if (cust != nullptr) {
      found = true;
      newStatus = !cust->getAccount().isActive();
      if (!customerService->setAccountActive(cust->getID(), newStatus, err)) {
        return OperationResult::Fail(
            !err.empty()
                ? err
                : "Lỗi khi cập nhật trạng thái tài khoản vào cơ sở dữ liệu!");
      }
    }
  }

  if (!found) {
    return OperationResult::Fail("Không tìm thấy tài khoản!");
  }

  std::string msg = newStatus ? ("Đã mở khóa tài khoản '" + username + "'")
                              : ("Đã khóa tài khoản '" + username + "'");
  return OperationResult::Ok(msg);
}

OperationResult AppController::changeAccountRole(const std::string &username,
                                                 RoleType newRole) {
  REQUIRE_ADMIN_ROLE();
  if (username == currentSession.username) {
    return OperationResult::Fail(
        "Không thể tự đổi quyền tài khoản đang đăng nhập!");
  }
  if (newRole == RoleType::CUSTOMER) {
    return OperationResult::Fail(
        "Không thể chuyển đổi tài khoản nhân sự sang khách hàng!");
  }

  if (employeeService == nullptr) {
    return OperationResult::Fail("Dịch vụ nhân sự chưa sẵn sàng!");
  }

  Employee *emp = employeeService->getEmployeeByUsername(username);
  if (emp == nullptr) {
    return OperationResult::Fail("Không tìm thấy nhân viên có tên đăng nhập '" +
                                 username + "' để đổi quyền!");
  }

  if (emp->getRoleType() == newRole) {
    return OperationResult::Ok("Tài khoản đã có quyền này từ trước.");
  }

  Employee *newEmp = nullptr;
  if (newRole == RoleType::ADMIN) {
    newEmp = new AdminEmployee(
        emp->getID(), emp->getName(), emp->getPhoneNumber(), emp->getEmail(),
        emp->getAddress(), emp->getAccount().getUsername(),
        emp->getAccount().getPasswordHash(), emp->getSalary());
  } else if (newRole == RoleType::WAREHOUSE_EMPLOYEE) {
    newEmp = new WarehouseEmployee(
        emp->getID(), emp->getName(), emp->getPhoneNumber(), emp->getEmail(),
        emp->getAddress(), emp->getAccount().getUsername(),
        emp->getAccount().getPasswordHash(), emp->getSalary());
  } else if (newRole == RoleType::SALE_EMPLOYEE) {
    newEmp = new SalesEmployee(
        emp->getID(), emp->getName(), emp->getPhoneNumber(), emp->getEmail(),
        emp->getAddress(), emp->getAccount().getUsername(),
        emp->getAccount().getPasswordHash(), emp->getSalary());
  } else {
    return OperationResult::Fail("Loại quyền không hợp lệ cho nhân sự!");
  }

  newEmp->getAccount().setID(emp->getAccount().getID());
  newEmp->getAccount().setActive(emp->getAccount().isActive());

  std::string serviceErr;
  if (!employeeService->updateEmployee(newEmp, serviceErr)) {
    delete newEmp;
    return OperationResult::Fail(
        !serviceErr.empty()
            ? serviceErr
            : "Lỗi khi lưu chức vụ nhân viên vào file dữ liệu!");
  }

  return OperationResult::Ok("Đã đổi quyền tài khoản '" + username + "' thành " +
                             roleTypeToString(newRole));
}

// === BẢO HÀNH ===

std::vector<WarrantyItemViewModel> AppController::getWarranties() {
  if (!currentSession.isAuthenticated ||
      !(currentSession.isAdmin() || currentSession.isSales())) {
    return std::vector<WarrantyItemViewModel>();
  }
  std::vector<WarrantyItemViewModel> result;
  if (warrantyService == nullptr) {
    return result;
  }
  DateTime now = DateTime::fromString(getSimulatedTime());
  const Vector<WarrantyRequest> &requests = warrantyService->getAll();
  result.reserve(static_cast<size_t>(requests.getSize()));
  for (int i = requests.getSize() - 1; i >= 0; --i) {
    result.push_back(
        warrantyToViewModel(requests[i], customerService, warrantyService, now));
  }
  return result;
}

std::vector<WarrantyItemViewModel>
AppController::getCustomerWarranties(const std::string &customerId) {
  if (!currentSession.isAuthenticated) {
    return std::vector<WarrantyItemViewModel>();
  }

  std::string targetCustomerId;
  bool matchAllIfEmpty = false;

  if (currentSession.isCustomer()) {
    if (currentSession.userId.empty()) {
      return std::vector<WarrantyItemViewModel>();
    }
    targetCustomerId = currentSession.userId;
    matchAllIfEmpty = false;
  } else if (currentSession.isAdmin() || currentSession.isSales()) {
    targetCustomerId = trimCopy(customerId);
    matchAllIfEmpty = true;
  } else {
    return std::vector<WarrantyItemViewModel>();
  }

  std::vector<WarrantyItemViewModel> result;
  if (warrantyService == nullptr) {
    return result;
  }
  DateTime now = DateTime::fromString(getSimulatedTime());
  const Vector<WarrantyRequest> &requests = warrantyService->getAll();
  for (int i = requests.getSize() - 1; i >= 0; --i) {
    if (targetCustomerId.empty()) {
      if (matchAllIfEmpty) {
        result.push_back(warrantyToViewModel(requests[i], customerService,
                                             warrantyService, now));
      }
    } else if (requests[i].getCustomerId() == targetCustomerId) {
      result.push_back(warrantyToViewModel(requests[i], customerService,
                                           warrantyService, now));
    }
  }
  return result;
}

  std::vector<WarrantyEligibleItemViewModel>
  AppController::getEligibleWarrantyItems(const std::string &customerId) {
    if (!currentSession.isAuthenticated) {
      return std::vector<WarrantyEligibleItemViewModel>();
    }

    std::string targetCustomerId;
    if (currentSession.isCustomer()) {
      if (currentSession.userId.empty()) {
        return std::vector<WarrantyEligibleItemViewModel>();
      }
      targetCustomerId = currentSession.userId;
    } else if (currentSession.isAdmin() || currentSession.isSales()) {
      targetCustomerId = trimCopy(customerId);
    } else {
      return std::vector<WarrantyEligibleItemViewModel>();
    }

    if (warrantyService == nullptr) {
      return std::vector<WarrantyEligibleItemViewModel>();
    }

    DateTime now = DateTime::fromString(getSimulatedTime());
    Vector<WarrantyService::WarrantyCoverage> coverages =
        warrantyService->listEligibleForCustomer(targetCustomerId, now);

    std::vector<WarrantyEligibleItemViewModel> result;
    for (int i = 0; i < coverages.getSize(); ++i) {
      const auto &cov = coverages[i];
      WarrantyEligibleItemViewModel vm;
      vm.orderId = cov.orderId;
      vm.invoiceId = cov.invoiceId;
      vm.phoneId = cov.phoneId;
      vm.phoneName = cov.phoneName;
      vm.quantity = cov.quantity;
      vm.purchaseDate = cov.startDate.toDateString();
      vm.expireDate = cov.expireDate.toDateString();
      vm.remainingDays = calculateCivilDayDiff(now, cov.expireDate);
      vm.active = cov.active;
      vm.openRequests = cov.openRequests;
      result.push_back(vm);
    }
    return result;
  }

  OperationResult AppController::createWarranty(
      const CreateWarrantyRequest &req) {
      if (!currentSession.isAuthenticated) {
        return OperationResult::Fail(
            "Vui lòng đăng nhập để thực hiện thao tác này!");
      }
      if (currentSession.isWarehouse()) {
        return OperationResult::Fail(
            "Thủ kho không có quyền tạo phiếu bảo hành!");
      }

        std::string customerId;
        if (currentSession.isCustomer()) {
          if (currentSession.userId.empty()) {
            return OperationResult::Fail(
                "Không xác định được mã khách hàng của phiên đăng nhập!");
          }
          customerId = currentSession.userId;
        } else if (currentSession.isAdmin() || currentSession.isSales()) {
          customerId = trimCopy(req.customerId);
        } else {
          return OperationResult::Fail(
              "Từ chối truy cập: Bạn không có quyền tạo phiếu bảo hành!");
        }

          std::string orderId = trimCopy(req.orderId);
          std::string phoneId = trimCopy(req.phoneId);
          std::string issue = trimCopy(req.issueDescription);

          if (orderId.empty() || phoneId.empty()) {
            return OperationResult::Fail(
                "Vui lòng cung cấp đầy đủ mã đơn hàng và mã sản phẩm!");
          }
            if (issue.empty()) {
              return OperationResult::Fail("Mô tả sự cố không được để trống!");
            }

            if (warrantyService == nullptr) {
              return OperationResult::Fail(
                  "Dịch vụ bảo hành chưa sẵn sàng!");
            }

            DateTime now = DateTime::fromString(getSimulatedTime());
            std::string outId;
            std::string err;

            if (!warrantyService->createRequest(customerId, orderId, phoneId,
                                                issue, now, outId, err)) {
              return OperationResult::Fail(
                  !err.empty() ? err : "Lỗi khi tạo yêu cầu bảo hành!");
            }

            return OperationResult::Ok(
                "Gửi yêu cầu bảo hành thành công! Mã phiếu: " + outId, outId);
            }

            OperationResult AppController::updateWarrantyStatus(
                const std::string &id, WarrantyStatus status,
                const std::string &technicianNote,
                const std::string &resolution) {
              REQUIRE_ADMIN_OR_SALES_ROLE();

              if (!IdGenerator::isValidId(id, "WREQ", true)) {
                return OperationResult::Fail("Mã yêu cầu bảo hành không đúng "
                                             "định dạng WREQ-ddmmyyyy-000001!");
              }

              if (warrantyService == nullptr) {
                return OperationResult::Fail(
                    "Dịch vụ bảo hành chưa sẵn sàng!");
              }

              DateTime now = DateTime::fromString(getSimulatedTime());
              std::string err;
              if (!warrantyService->updateStatus(id, status, technicianNote,
                                                 resolution, now, err)) {
                return OperationResult::Fail(
                    !err.empty()
                        ? err
                        : "Lỗi khi cập nhật trạng thái phiếu bảo hành!");
              }

              return OperationResult::Ok("Cập nhật tiến độ bảo hành " + id +
                                         " thành công!");
            }
