#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

// === TÀI KHOẢN & PHÂN QUYỀN ===

std::vector<AccountItemViewModel> AppController::getAccounts() {
  auto &store = MockDataStore::getInstance();
  if (employeeService != nullptr || customerService != nullptr) {
    store.accounts.clear();
    if (employeeService != nullptr) {
      const Vector<Employee *> &employees = employeeService->getAllEmployees();
      for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] != nullptr) {
          store.accounts.push_back(employeeToAccountViewModel(employees[i]));
        }
      }
    }
    if (customerService != nullptr) {
      const Vector<Customer> &customers = customerService->getAllCustomers();
      for (int i = 0; i < customers.getSize(); ++i) {
        store.accounts.push_back(customerToAccountViewModel(customers[i]));
      }
    }
  }
  return store.accounts;
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

  auto &store = MockDataStore::getInstance();
  for (auto &a : store.accounts) {
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

OperationResult AppController::changeAccountRole(const std::string &username,
                                                 RoleType newRole) {
  REQUIRE_ADMIN_ROLE();
  if (username == currentSession.username) {
    return OperationResult::Fail(
        "Không thể tự đổi quyền tài khoản đang đăng nhập!");
  }

  auto &store = MockDataStore::getInstance();
  for (auto &a : store.accounts) {
    if (a.username == username) {
      a.role = newRole;
      a.roleName = roleTypeToString(newRole);
      return OperationResult::Ok("Đã đổi quyền tài khoản '" + username +
                                 "' thành " + a.roleName);
    }
  }
  return OperationResult::Fail("Không tìm thấy tài khoản!");
}

// === BẢO HÀNH ===

std::vector<WarrantyItemViewModel> AppController::getWarranties() {
  if (!currentSession.isAuthenticated ||
      !(currentSession.isAdmin() || currentSession.isSales())) {
    return std::vector<WarrantyItemViewModel>();
  }
  return MockDataStore::getInstance().warranties;
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

  auto &store = MockDataStore::getInstance();
  std::vector<WarrantyItemViewModel> result;
  for (const auto &w : store.warranties) {
      if (targetCustomerId.empty()) {
        if (matchAllIfEmpty) {
          result.push_back(w);
        }
      } else if (w.customerId == targetCustomerId) {
        result.push_back(w);
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

              // Nhánh mock khi warrantyService == nullptr (công tắc tắt & tương
              // thích backward)
              if (warrantyService == nullptr) {
                if (!IdGenerator::isValidId(phoneId, "PHN")) {
                  return OperationResult::Fail(
                      "Mã sản phẩm không đúng định dạng!");
                }
                auto &store = MockDataStore::getInstance();
                if (getProductById(phoneId).id.empty()) {
                  return OperationResult::Fail(
                      "Không tìm thấy sản phẩm được tham chiếu!");
                }
                if (!customerId.empty() &&
                    (!IdGenerator::isValidId(customerId, "CUS") ||
                     getCustomerById(customerId).id.empty())) {
                  return OperationResult::Fail(
                      "Khách hàng được tham chiếu không hợp lệ hoặc không tồn "
                      "tại!");
                }
                if (!orderId.empty() &&
                    (!IdGenerator::isValidId(orderId, "ORD", true) ||
                     getOrderById(orderId).id.empty())) {
                  return OperationResult::Fail("Đơn hàng được tham chiếu không "
                                               "hợp lệ hoặc không tồn tại!");
                }

                std::string warrantyRequestId;
                std::string idError;
                const std::string idDate = IdGenerator::currentDateKey();
                if (idDate.empty() ||
                    !IdGenerator::nextId("WREQ", idDate,
                                         collectIds(store.warranties),
                                         warrantyRequestId, idError)) {
                  return OperationResult::Fail(
                      idError.empty() ? "Không thể xác định ngày để sinh mã "
                                        "yêu cầu bảo hành."
                                      : idError);
                }

                DateTime now = DateTime::fromString(getSimulatedTime());
                ProductItemViewModel p = getProductById(phoneId);

                WarrantyItemViewModel w;
                w.id = warrantyRequestId;
                w.orderId = orderId;
                w.phoneId = phoneId;
                w.phoneName =
                    p.name.empty() ? ("Sản phẩm #" + phoneId) : p.name;
                w.customerId = customerId;
                w.customerName =
                    (currentSession.userId == customerId &&
                     !currentSession.fullName.empty())
                        ? currentSession.fullName
                        : (customerId.empty() ? "Khách vãng lai" : customerId);
                w.customerPhone = (currentSession.userId == customerId)
                                      ? currentSession.phone
                                      : "";
                w.requestDate = formatDateTimeMinute(now);
                w.issueDescription = issue;
                w.status = WarrantyStatus::RECEIVED;
                w.statusText = warrantyStatusToString(w.status);
                w.technicianNote = "Chờ kỹ thuật viên phân công kiểm tra";
                w.resolution = "Đang xử lý";
                w.createdAt = now.toString();
                w.updatedAt = now.toString();

                store.warranties.insert(store.warranties.begin(), w);
                return OperationResult::Ok(
                    "Gửi yêu cầu bảo hành thành công! Mã phiếu: " + w.id, w.id);
              }

              // Nhánh xử lý chính thức qua WarrantyService khi service !=
              // nullptr
              DateTime now = DateTime::fromString(getSimulatedTime());
              std::string outId;
              std::string err;

              if (!warrantyService->createRequest(customerId, orderId, phoneId,
                                                  issue, now, outId, err)) {
                return OperationResult::Fail(
                    !err.empty() ? err : "Lỗi khi tạo yêu cầu bảo hành!");
              }

              // Lấy thông tin phiếu vừa tạo để đồng bộ vào store.warranties
              // (cache)
              const WarrantyRequest *createdReq =
                  warrantyService->getById(outId);
              auto &store = MockDataStore::getInstance();
              if (createdReq != nullptr) {
                WarrantyItemViewModel vm = warrantyToViewModel(
                    *createdReq, customerService, warrantyService, now);
                store.warranties.insert(store.warranties.begin(), vm);
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

              // Nhánh mock khi warrantyService == nullptr
              if (warrantyService == nullptr) {
                auto &store = MockDataStore::getInstance();
                for (auto &w : store.warranties) {
                  if (w.id == id) {
                    w.status = status;
                    w.statusText = warrantyStatusToString(status);
                    if (!technicianNote.empty())
                      w.technicianNote = technicianNote;
                    if (!resolution.empty())
                      w.resolution = resolution;
                    return OperationResult::Ok("Cập nhật tiến độ bảo hành " +
                                               id + " thành công!");
                  }
                }
                return OperationResult::Fail("Không tìm thấy phiếu bảo hành!");
              }

              // Nhánh chính thức qua WarrantyService khi service != nullptr
              DateTime now = DateTime::fromString(getSimulatedTime());
              std::string err;
              if (!warrantyService->updateStatus(id, status, technicianNote,
                                                 resolution, now, err)) {
                return OperationResult::Fail(
                    !err.empty()
                        ? err
                        : "Lỗi khi cập nhật trạng thái phiếu bảo hành!");
              }

              // Đồng bộ lại cache sau khi thành công
              const WarrantyRequest *updatedReq = warrantyService->getById(id);
              auto &store = MockDataStore::getInstance();
              for (auto &w : store.warranties) {
                if (w.id == id) {
                  if (updatedReq != nullptr) {
                    w = warrantyToViewModel(*updatedReq, customerService,
                                            warrantyService, now);
                  } else {
                    w.status = status;
                    w.statusText = warrantyStatusToString(status);
                    if (!technicianNote.empty())
                      w.technicianNote =
                          WarrantyRequest::sanitizeText(technicianNote, 500);
                    if (!resolution.empty())
                      w.resolution =
                          WarrantyRequest::sanitizeText(resolution, 500);
                    w.updatedAt = now.toString();
                  }
                  break;
                }
              }

              return OperationResult::Ok("Cập nhật tiến độ bảo hành " + id +
                                         " thành công!");
            }
