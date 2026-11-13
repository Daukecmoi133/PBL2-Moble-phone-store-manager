#include "AppControllerSupport.h"
using namespace AppControllerHelpers;

AppController::AppController(PhoneService *ps, OrderService *os,
                             CustomerService *cs, EmployeeService *es,
                             InvoiceService *is, SupplierService *ss,
                             DiscountService *ds, ReportService *rs,
                             AuthService *as, WarrantyService *ws)
    : phoneService(ps), orderService(os), customerService(cs),
      employeeService(es), invoiceService(is), supplierService(ss),
      discountService(ds), reportService(rs), authService(as),
      warrantyService(ws) {
  // Quét và khôi phục các giao dịch file dở dang nếu xảy ra sự cố trước đó
  std::string dataDir = PathHelper::resolveDataPath("data");
  FileTransaction::recoverPendingTransactions(dataDir);

  // Nạp dữ liệu cấu hình cửa hàng, marketing và mô tả sản phẩm
  marketingRepo.loadFromFile();
  descriptionRepo.loadFromFile();
  storeSettingsRepo.loadFromFile();

  // Đồng bộ MockDataStore từ các Service chính thức (được nạp từ file)
  auto &store = MockDataStore::getInstance();

  if (phoneService != nullptr) {
    store.products.clear();
    const Vector<Phone> &phones = phoneService->getAllPhones();
    for (int i = 0; i < phones.getSize(); ++i) {
      store.products.push_back(phoneToViewModel(
          phones[i], phoneService, &marketingRepo, &descriptionRepo));
    }
  }

  if (customerService != nullptr) {
    store.customers.clear();
    const Vector<Customer> &customers = customerService->getAllCustomers();
    for (int i = 0; i < customers.getSize(); ++i) {
      store.customers.push_back(customerToViewModel(customers[i]));
    }
  }

  if (employeeService != nullptr) {
    store.employees.clear();
    const Vector<Employee *> &employees = employeeService->getAllEmployees();
    for (int i = 0; i < employees.getSize(); ++i) {
      if (employees[i] != nullptr) {
        store.employees.push_back(employeeToViewModel(employees[i]));
      }
    }
  }

  if (supplierService != nullptr) {
    store.suppliers.clear();
    const Vector<Supplier> &suppliers = supplierService->getAllSuppliers();
    for (int i = 0; i < suppliers.getSize(); ++i) {
      store.suppliers.push_back(supplierToViewModel(suppliers[i]));
    }

    store.receipts.clear();
    const Vector<ImportReceipt> &receipts = supplierService->getAllReceipts();
    for (int i = 0; i < receipts.getSize(); ++i) {
      store.receipts.push_back(
          receiptToViewModel(receipts[i], store.suppliers, store.employees));
    }
  }

  if (discountService != nullptr) {
    store.discounts.clear();
    const Vector<DiscountCode> &discounts = discountService->getAll();
    for (int i = 0; i < discounts.getSize(); ++i) {
      store.discounts.push_back(discountToViewModel(discounts[i]));
    }
  }

  if (orderService != nullptr) {
    store.orders.clear();
    const Vector<Order> &orders = orderService->getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
      store.orders.push_back(orderToViewModel(orders[i], store.customers));
    }
  }

  if (invoiceService != nullptr) {
    store.invoices.clear();
    const Vector<Invoice> &invoices = invoiceService->getAll();
    for (int i = 0; i < invoices.getSize(); ++i) {
      store.invoices.push_back(
          invoiceToViewModel(invoices[i], store.customers, store.employees));
    }
  }

  // Đồng bộ danh sách tài khoản từ nguồn lưu trữ chính thức
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

  if (warrantyService != nullptr) {
    store.warranties.clear();
    DateTime now = DateTime::fromString(
        RealTimeManager::getInstance().getCurrentDateTime());
    const Vector<WarrantyRequest> &requests = warrantyService->getAll();
    for (int i = 0; i < requests.getSize(); ++i) {
      store.warranties.push_back(warrantyToViewModel(
          requests[i], customerService, warrantyService, now));
    }
  } else {
    if (store.warranties.empty()) {
      store.initWarranties();
    }
  }

  // Chuyển đổi an toàn mật khẩu cũ sang hash PBKDF2 (không làm mất tài khoản
  // hay đổi mật khẩu)
  if (employeeService != nullptr) {
    int empMigrated = 0;
    std::string err;
    employeeService->migrateLegacyCredentials(empMigrated, err);
  }
  if (customerService != nullptr) {
    int custMigrated = 0;
    std::string err;
    customerService->migrateLegacyCredentials(custMigrated, err);
  }
}

// === XÁC THỰC & PHIÊN LÀM VIỆC ===

LoginResponse AppController::login(const LoginRequest &request) {
  LoginResponse response;

  const std::string username = trimCopy(request.username);
  if (username.empty() || request.password.empty()) {
    response.result =
        OperationResult::Fail("Tên đăng nhập và mật khẩu không được để trống!");
    return response;
  }

  // 1. Kiểm tra lỗi I/O đọc dữ liệu từ tệp tin lưu trữ chính thức
  if ((employeeService != nullptr && employeeService->hasLoadError()) ||
      (customerService != nullptr && customerService->hasLoadError())) {
    response.result =
        OperationResult::Fail("Lỗi đọc dữ liệu tài khoản từ tệp tin!");
    return response;
  }

  // 2. Tra cứu tài khoản nhân viên từ nguồn lưu trữ chính thức (Employee.txt)
  if (employeeService != nullptr) {
    Employee *emp = employeeService->getEmployeeByUsername(username);
    if (emp != nullptr) {
      if (!emp->getAccount().isActive()) {
        response.result = OperationResult::Fail("Tài khoản đã bị vô hiệu hóa!");
        return response;
      }
      if (!emp->getAccount().verifyPassword(request.password)) {
        response.result = OperationResult::Fail("Mật khẩu không chính xác!");
        return response;
      }
      // Nâng cấp an toàn legacy plaintext sang hash nếu chưa băm
      if (!emp->getAccount().isPasswordHashed()) {
        std::string newHash = PasswordHasher::hash(request.password);
        std::string err;
        employeeService->setPasswordCredential(emp->getID(), newHash, err);
      }

      currentSession.userId = emp->getID();
      currentSession.username = emp->getAccount().getUsername();
      currentSession.fullName = emp->getName().getFullName();
      currentSession.role = emp->getRoleType();
      currentSession.roleName = roleTypeToString(emp->getRoleType());
      currentSession.phone = emp->getPhoneNumber();
      currentSession.email = emp->getEmail();
      currentSession.address = emp->getAddress();
      currentSession.isAuthenticated = true;

      response.result = OperationResult::Ok("Đăng nhập thành công! Chào mừng " +
                                            emp->getName().getFullName());
      response.session = currentSession;
      return response;
    }
  }

  // 3. Tra cứu tài khoản khách hàng từ nguồn lưu trữ chính thức (Customers.txt)
  if (customerService != nullptr) {
    Customer *cust = customerService->getCustomerByUsername(username);
    if (cust != nullptr) {
      if (!cust->getAccount().isActive()) {
        response.result = OperationResult::Fail("Tài khoản đã bị vô hiệu hóa!");
        return response;
      }
      if (!cust->getAccount().verifyPassword(request.password)) {
        response.result = OperationResult::Fail("Mật khẩu không chính xác!");
        return response;
      }
      // Nâng cấp an toàn legacy plaintext sang hash nếu chưa băm
      if (!cust->getAccount().isPasswordHashed()) {
        std::string newHash = PasswordHasher::hash(request.password);
        std::string err;
        customerService->setPasswordCredential(cust->getID(), newHash, err);
      }

      currentSession.userId = cust->getID();
      currentSession.username = cust->getAccount().getUsername();
      currentSession.fullName = cust->getName().getFullName();
      currentSession.role = RoleType::CUSTOMER;
      currentSession.roleName = "Khách hàng";
      currentSession.phone = cust->getPhoneNumber();
      currentSession.email = cust->getEmail();
      currentSession.address = cust->getAddress();
      currentSession.isAuthenticated = true;

      response.result = OperationResult::Ok("Đăng nhập thành công! Chào mừng " +
                                            cust->getName().getFullName());
      response.session = currentSession;
      return response;
    }
  }

  // Fallback: Khi không cấu hình Service (chỉ trong unit test tách biệt thuần
  // mock)
  if (employeeService == nullptr && customerService == nullptr) {
    auto &store = MockDataStore::getInstance();
    for (const auto &emp : store.employees) {
      if (emp.username == username) {
        if (!emp.isActive) {
          response.result =
              OperationResult::Fail("Tài khoản đã bị vô hiệu hóa!");
          return response;
        }
        if (store.hasCustomPassword(emp.username) &&
            store.verifyPassword(emp.username, request.password)) {
          currentSession.userId = emp.id;
          currentSession.username = emp.username;
          currentSession.fullName = emp.fullName;
          currentSession.role = emp.role;
          currentSession.roleName = emp.roleName;
          currentSession.phone = emp.phone;
          currentSession.email = emp.email;
          currentSession.address = emp.address;
          currentSession.isAuthenticated = true;

          response.result = OperationResult::Ok(
              "Đăng nhập thành công! Chào mừng " + emp.fullName);
          response.session = currentSession;
          return response;
        } else {
          response.result = OperationResult::Fail("Mật khẩu không chính xác!");
          return response;
        }
      }
    }
    for (const auto &cust : store.customers) {
      if (cust.username == username) {
        if (store.hasCustomPassword(cust.username) &&
            store.verifyPassword(cust.username, request.password)) {
          currentSession.userId = cust.id;
          currentSession.username = cust.username;
          currentSession.fullName = cust.fullName;
          currentSession.role = RoleType::CUSTOMER;
          currentSession.roleName = "Khách hàng";
          currentSession.phone = cust.phone;
          currentSession.email = cust.email;
          currentSession.address = cust.address;
          currentSession.isAuthenticated = true;

          response.result = OperationResult::Ok(
              "Đăng nhập thành công! Chào mừng " + cust.fullName);
          response.session = currentSession;
          return response;
        } else {
          response.result = OperationResult::Fail("Mật khẩu không chính xác!");
          return response;
        }
      }
    }
  }

  response.result =
      OperationResult::Fail("Tài khoản không tồn tại hoặc đã bị khóa!");
  return response;
}

OperationResult AppController::registerCustomer(const std::string &rawUsername,
                                                const std::string &password,
                                                const std::string &rawFullName,
                                                const std::string &rawPhone,
                                                const std::string &rawEmail) {
  // Cắt khoảng trắng thừa trước khi kiểm tra và lưu
  const std::string username = trimCopy(rawUsername);
  const std::string fullName = trimCopy(rawFullName);
  const std::string phone = trimCopy(rawPhone);
  const std::string email = trimCopy(rawEmail);

  if (username.empty() || password.empty() || fullName.empty() ||
      phone.empty()) {
    return OperationResult::Fail("Vui lòng điền đầy đủ họ tên, số điện thoại, "
                                 "tên đăng nhập và mật khẩu!");
  }
  if (username.find_first_of(" \t") != std::string::npos) {
    return OperationResult::Fail("Tên đăng nhập không được chứa khoảng trắng!");
  }
  if (!isValidPhone(phone)) {
    return OperationResult::Fail(
        "Số điện thoại không hợp lệ! Phải gồm đúng 10 chữ số và bắt đầu bằng 0 "
        "(ví dụ 0912345678).");
  }
  if (!email.empty() && !isValidEmail(email)) {
    return OperationResult::Fail(
        "Email không hợp lệ! Ví dụ đúng: an.nguyen@example.com");
  }
  if (password.length() < 4) {
    return OperationResult::Fail("Mật khẩu bảo mật phải có ít nhất 4 ký tự!");
  }

  // Kiểm tra tên đăng nhập đã tồn tại trong toàn hệ thống (cả nhân viên lẫn
  // khách hàng)
  if ((employeeService != nullptr &&
       employeeService->getEmployeeByUsername(username) != nullptr) ||
      (customerService != nullptr &&
       customerService->getCustomerByUsername(username) != nullptr)) {
    return OperationResult::Fail("Tên đăng nhập '" + username +
                                 "' đã được sử dụng! Vui lòng chọn tên khác.");
  }

  auto &store = MockDataStore::getInstance();

  // Lưu xuống file qua CustomerService
  if (customerService != nullptr) {
    Vector<std::string> nameParts = StringHelper::split(fullName, ' ');
    std::string first = "", mid = "", last = "";
    if (nameParts.getSize() == 1) {
      first = nameParts[0];
    } else if (nameParts.getSize() == 2) {
      first = nameParts[0];
      last = nameParts[1];
    } else if (nameParts.getSize() >= 3) {
      first = nameParts[0];
      last = nameParts[nameParts.getSize() - 1];
      for (int i = 1; i < nameParts.getSize() - 1; ++i) {
        if (!mid.empty())
          mid += " ";
        mid += nameParts[i];
      }
    }
    std::vector<std::string> existingCustIds = collectIds(store.customers);
    const Vector<Customer> &allCust = customerService->getAllCustomers();
    for (int i = 0; i < allCust.getSize(); ++i) {
      existingCustIds.push_back(allCust[i].getID());
    }

    std::vector<std::string> existingAccIds = collectIds(store.accounts);
    for (int i = 0; i < allCust.getSize(); ++i) {
      existingAccIds.push_back(allCust[i].getAccount().getID());
    }
    if (employeeService != nullptr) {
      const Vector<Employee *> &allEmp = employeeService->getAllEmployees();
      for (int i = 0; i < allEmp.getSize(); ++i) {
        if (allEmp[i] != nullptr)
          existingAccIds.push_back(allEmp[i]->getAccount().getID());
      }
    }

    std::string customerId;
    std::string accountId;
    std::string idErr;
    if (!IdGenerator::nextId("CUS", "", existingCustIds, customerId, idErr)) {
      return OperationResult::Fail(idErr);
    }
    if (!IdGenerator::nextId("ACC", "", existingAccIds, accountId, idErr)) {
      return OperationResult::Fail(idErr);
    }
    Name name(first, mid, last);
    Customer cust(customerId, name, phone, email, "", username, password, 0);
    cust.getAccount().setID(accountId);
    std::string serviceErr;
    if (!customerService->registerCustomer(cust, serviceErr)) {
      return OperationResult::Fail(
          !serviceErr.empty() ? serviceErr : "Đăng ký tài khoản thất bại!");
    }
  }

  bool ok = store.registerCustomer(username, password, fullName, phone, email);
  if (!ok && customerService == nullptr) {
    return OperationResult::Fail("Tên đăng nhập '" + username +
                                 "' đã được sử dụng! Vui lòng chọn tên khác.");
  }
  return OperationResult::Ok("Đăng ký tài khoản thành công! Bạn có thể sử dụng "
                             "tài khoản vừa tạo để đăng nhập.");
}

OperationResult
AppController::resetPasswordSimulation(const std::string &rawUsername,
                                       const std::string &newPassword,
                                       const std::string &rawVerifyContact) {
  const std::string username = trimCopy(rawUsername);
  const std::string contact = trimCopy(rawVerifyContact);
  if (username.empty() || newPassword.empty()) {
    return OperationResult::Fail(
        "Tên đăng nhập và mật khẩu mới không được để trống!");
  }
  if (contact.empty()) {
    return OperationResult::Fail("Vui lòng nhập Số điện thoại hoặc Email đã "
                                 "đăng ký để xác minh danh tính!");
  }
  if (newPassword.length() < 4) {
    return OperationResult::Fail("Mật khẩu mới phải có ít nhất 4 ký tự!");
  }

  auto sameText = [](const std::string &a, const std::string &b) {
    if (a.size() != b.size())
      return false;
    for (size_t i = 0; i < a.size(); ++i) {
      if (std::tolower(static_cast<unsigned char>(a[i])) !=
          std::tolower(static_cast<unsigned char>(b[i])))
        return false;
    }
    return true;
  };

  std::string newHashed = PasswordHasher::hash(newPassword);
  if (newHashed.empty()) {
    return OperationResult::Fail("Lỗi khi mã hóa mật khẩu mới!");
  }

  std::string err;

  // 1. Kiểm tra trong CustomerService trước
  if (customerService != nullptr) {
    Customer *cust = customerService->getCustomerByUsername(username);
    if (cust != nullptr) {
      const bool verified =
          (!cust->getPhoneNumber().empty() &&
           cust->getPhoneNumber() == contact) ||
          (!cust->getEmail().empty() && sameText(cust->getEmail(), contact));
      if (!verified) {
        return OperationResult::Fail(
            "Thông tin xác minh (SĐT hoặc Email) không khớp với tài khoản '" +
            username + "'!");
      }
      if (!customerService->setPasswordCredential(cust->getID(), newHashed,
                                                  err)) {
        return OperationResult::Fail(
            !err.empty() ? err : "Lỗi khi lưu mật khẩu mới vào cơ sở dữ liệu!");
      }
      MockDataStore::getInstance().resetPassword(username, newPassword);
      return OperationResult::Ok("Mô phỏng khôi phục mật khẩu thành công! Mật "
                                 "khẩu mới cho tài khoản '" +
                                 username + "' đã có hiệu lực.");
    }
  }

  // 2. Kiểm tra trong EmployeeService
  if (employeeService != nullptr) {
    Employee *emp = employeeService->getEmployeeByUsername(username);
    if (emp != nullptr) {
      const bool verified =
          (!emp->getPhoneNumber().empty() &&
           emp->getPhoneNumber() == contact) ||
          (!emp->getEmail().empty() && sameText(emp->getEmail(), contact));
      if (!verified) {
        return OperationResult::Fail(
            "Thông tin xác minh (SĐT hoặc Email) không khớp với tài khoản '" +
            username + "'!");
      }
      if (!employeeService->setPasswordCredential(emp->getID(), newHashed,
                                                  err)) {
        return OperationResult::Fail(
            !err.empty() ? err : "Lỗi khi lưu mật khẩu mới vào cơ sở dữ liệu!");
      }
      MockDataStore::getInstance().resetPassword(username, newPassword);
      return OperationResult::Ok("Mô phỏng khôi phục mật khẩu thành công! Mật "
                                 "khẩu mới cho tài khoản '" +
                                 username + "' đã có hiệu lực.");
    }
  }

  // Fallback MockDataStore nếu không có service
  auto &store = MockDataStore::getInstance();
  bool found = false;
  bool verified = false;
  for (const auto &c : store.customers) {
    if (c.username == username) {
      found = true;
      verified = (!c.phone.empty() && c.phone == contact) ||
                 (!c.email.empty() && sameText(c.email, contact));
      break;
    }
  }
  if (!found) {
    for (const auto &e : store.employees) {
      if (e.username == username) {
        found = true;
        verified = (!e.phone.empty() && e.phone == contact) ||
                   (!e.email.empty() && sameText(e.email, contact));
        break;
      }
    }
  }
  if (!found) {
    return OperationResult::Fail(
        "Không tìm thấy tài khoản với tên đăng nhập: " + username);
  }
  if (!verified) {
    return OperationResult::Fail(
        "Thông tin xác minh (SĐT hoặc Email) không khớp với tài khoản '" +
        username + "'!");
  }
  store.resetPassword(username, newPassword);
  return OperationResult::Ok(
      "Mô phỏng khôi phục mật khẩu thành công! Mật khẩu mới cho tài khoản '" +
      username + "' đã có hiệu lực.");
}

OperationResult AppController::changePassword(const std::string &oldPassword,
                                              const std::string &newPassword) {
  if (!currentSession.isAuthenticated) {
    return OperationResult::Fail("Bạn chưa đăng nhập!");
  }
  if (oldPassword.empty() || newPassword.empty()) {
    return OperationResult::Fail(
        "Vui lòng nhập đầy đủ mật khẩu hiện tại và mật khẩu mới!");
  }
  if (newPassword.length() < 4) {
    return OperationResult::Fail("Mật khẩu mới phải có ít nhất 4 ký tự!");
  }
  if (newPassword == oldPassword) {
    return OperationResult::Fail("Mật khẩu mới phải khác mật khẩu hiện tại!");
  }

  // Xác thực lại mật khẩu cũ bằng đúng luồng đăng nhập, rồi khôi phục phiên
  // hiện tại
  const UserSessionViewModel savedSession = currentSession;
  LoginRequest verifyReq;
  verifyReq.username = savedSession.username;
  verifyReq.password = oldPassword;
  LoginResponse verifyResp = login(verifyReq);
  currentSession = savedSession;
  if (!verifyResp.result.success) {
    return OperationResult::Fail("Mật khẩu hiện tại không chính xác!");
  }

  std::string newHashed = PasswordHasher::hash(newPassword);
  if (newHashed.empty()) {
    return OperationResult::Fail("Lỗi khi mã hóa mật khẩu mới!");
  }

  std::string err;
  if (savedSession.role == RoleType::CUSTOMER) {
    Customer *cust = customerService
                         ? customerService->getCustomerById(savedSession.userId)
                         : nullptr;
    if (!cust && customerService) {
      cust = customerService->getCustomerByUsername(savedSession.username);
    }
    if (cust && customerService) {
      if (!customerService->setPasswordCredential(cust->getID(), newHashed,
                                                  err)) {
        return OperationResult::Fail(
            !err.empty() ? err : "Lỗi khi lưu mật khẩu mới vào cơ sở dữ liệu!");
      }
    }
  } else {
    Employee *emp = employeeService
                        ? employeeService->getEmployeeById(savedSession.userId)
                        : nullptr;
    if (!emp && employeeService) {
      emp = employeeService->getEmployeeByUsername(savedSession.username);
    }
    if (emp && employeeService) {
      if (!employeeService->setPasswordCredential(emp->getID(), newHashed,
                                                  err)) {
        return OperationResult::Fail(
            !err.empty() ? err : "Lỗi khi lưu mật khẩu mới vào cơ sở dữ liệu!");
      }
    }
  }

  MockDataStore::getInstance().resetPassword(savedSession.username,
                                             newPassword);
  return OperationResult::Ok(
      "Đổi mật khẩu thành công! Hãy dùng mật khẩu mới ở lần đăng nhập sau.");
}

void AppController::logout() {
  currentSession = UserSessionViewModel();
  MockDataStore::getInstance().customerCart = CartSummaryViewModel();
}

const UserSessionViewModel &AppController::getCurrentUser() const {
  return currentSession;
}

// === HỆ THỐNG KIỂM SOÁT THỜI GIAN THỰC (REAL-TIME SIMULATION) ===

std::string AppController::getSimulatedTime() {
  return RealTimeManager::getInstance().getCurrentDateTime();
}

std::string AppController::getSimulatedDate() {
  return RealTimeManager::getInstance().getCurrentDate();
}

OperationResult AppController::setSimulatedTime(const std::string &newTimeStr) {
  RealTimeManager::getInstance().setDateTime(newTimeStr);
  return OperationResult::Ok(
      "Đã cập nhật mốc thời gian thực của hệ thống thành công!");
}

OperationResult AppController::advanceSimulatedDays(int days) {
  RealTimeManager::getInstance().advanceDays(days);
  return OperationResult::Ok(
      "Đã điều chỉnh bước nhảy thời gian mô phỏng thành công!");
}

void AppController::advanceSimulatedSeconds(int seconds) {
  RealTimeManager::getInstance().advanceSeconds(seconds);
}

OperationResult AppController::resetSimulatedTimeToNow() {
  RealTimeManager::getInstance().resetToSystemNow();
  return OperationResult::Ok(
      "Đã đồng bộ lại thời gian theo đồng hồ thực của máy tính!");
}

OperationResult AppController::updateProfile(const std::string &fullName,
                                             const std::string &phone,
                                             const std::string &email,
                                             const std::string &address) {
  if (!currentSession.isAuthenticated) {
    return OperationResult::Fail("Bạn chưa đăng nhập!");
  }
  if (fullName.empty() || phone.empty()) {
    return OperationResult::Fail(
        "Họ tên và số điện thoại không được để trống!");
  }
  if (!isValidPhone(trimCopy(phone))) {
    return OperationResult::Fail("Số điện thoại không hợp lệ! Phải gồm đúng 10 "
                                 "chữ số và bắt đầu bằng 0.");
  }
  if (!trimCopy(email).empty() && !isValidEmail(trimCopy(email))) {
    return OperationResult::Fail("Email không hợp lệ!");
  }

  // Cập nhật xuống file qua Service trước
  if (currentSession.isCustomer() && customerService != nullptr) {
    Customer *cust = customerService->getCustomerById(currentSession.userId);
    if (cust != nullptr) {
      Vector<std::string> parts = StringHelper::split(fullName, ' ');
      std::string first = parts.getSize() > 0 ? parts[0] : "";
      std::string last = parts.getSize() > 1 ? parts[parts.getSize() - 1] : "";
      std::string mid = "";
      for (int i = 1; i < parts.getSize() - 1; ++i) {
        if (!mid.empty())
          mid += " ";
        mid += parts[i];
      }
      cust->setName(Name(first, mid, last));
      cust->setPhoneNumber(phone);
      cust->setEmail(email);
      cust->setAddress(address);
      std::string serviceErr;
      if (!customerService->updateCustomer(*cust, serviceErr)) {
        return OperationResult::Fail(
            !serviceErr.empty() ? serviceErr
                                : "Lỗi khi lưu thông tin hồ sơ vào file!");
      }
    }
  } else if (!currentSession.isCustomer() && employeeService != nullptr) {
    Employee *emp = employeeService->getEmployeeById(currentSession.userId);
    if (emp != nullptr) {
      Vector<std::string> parts = StringHelper::split(fullName, ' ');
      std::string first = parts.getSize() > 0 ? parts[0] : "";
      std::string last = parts.getSize() > 1 ? parts[parts.getSize() - 1] : "";
      std::string mid = "";
      for (int i = 1; i < parts.getSize() - 1; ++i) {
        if (!mid.empty())
          mid += " ";
        mid += parts[i];
      }
      emp->setName(Name(first, mid, last));
      emp->setPhoneNumber(phone);
      emp->setEmail(email);
      emp->setAddress(address);
      std::string serviceErr;
      if (!employeeService->updateEmployee(emp, serviceErr)) {
        return OperationResult::Fail(
            !serviceErr.empty() ? serviceErr
                                : "Lỗi khi lưu thông tin hồ sơ vào file!");
      }
    }
  }

  // Cập nhật session RAM sau khi lưu file thành công
  currentSession.fullName = fullName;
  currentSession.phone = phone;
  currentSession.email = email;
  currentSession.address = address;

  // Cập nhật trong Mock Store
  auto &store = MockDataStore::getInstance();
  if (currentSession.isCustomer()) {
    for (auto &c : store.customers) {
      if (c.id == currentSession.userId) {
        c.fullName = fullName;
        c.phone = phone;
        c.email = email;
        c.address = address;
        break;
      }
    }
  } else {
    for (auto &e : store.employees) {
      if (e.id == currentSession.userId) {
        e.fullName = fullName;
        e.phone = phone;
        e.email = email;
        e.address = address;
        break;
      }
    }
  }

  return OperationResult::Ok("Cập nhật thông tin hồ sơ thành công!");
}
