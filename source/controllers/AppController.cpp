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
  if (orderService != nullptr && discountService != nullptr) {
    orderService->setDiscountService(discountService);
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

std::unordered_map<std::string, int> AppController::computeReservedStockMap() const {
  if (orderService == nullptr) {
    return {};
  }
  return orderService->getReservedStockMap();
}

int AppController::computeReservedStockForPhone(const std::string &phoneId) const {
  if (orderService == nullptr || phoneId.empty()) {
    return 0;
  }
  return orderService->getReservedStockForPhone(phoneId);
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
    if (emp == nullptr) {
      emp = employeeService->getEmployeeByPhone(username); // Khóa số điện thoại
    }
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
    if (cust == nullptr) {
      cust = customerService->getCustomerByPhone(username); // Khóa số điện thoại
    }
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

  // RÀNG BUỘC TOÀN HỆ THỐNG: Mỗi tài khoản lấy số điện thoại làm khóa, KHÔNG có 2 tài khoản cho 1 số điện thoại
  if ((employeeService != nullptr &&
       employeeService->getEmployeeByPhone(phone) != nullptr) ||
      (customerService != nullptr &&
       customerService->getCustomerByPhone(phone) != nullptr)) {
    return OperationResult::Fail("Số điện thoại '" + phone +
                                 "' đã được đăng ký cho một tài khoản khác trong hệ thống!");
  }

  if (customerService == nullptr) {
    return OperationResult::Fail("Dịch vụ khách hàng chưa sẵn sàng!");
  }

  // Lưu xuống file qua CustomerService (Single Source of Truth)
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

  std::vector<std::string> existingCustIds =
      collectModelIds(customerService->getAllCustomers());
  std::vector<std::string> existingAccIds =
      collectAllAccountIds(employeeService, customerService);

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
    if (cust == nullptr) {
      cust = customerService->getCustomerByPhone(username);
    }
    if (cust != nullptr) {
      if (!cust->getAccount().isActive()) {
        return OperationResult::Fail(
            "Tài khoản đã bị vô hiệu hóa, không thể đặt lại mật khẩu!");
      }
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
      return OperationResult::Ok("Mô phỏng khôi phục mật khẩu thành công! Mật "
                                 "khẩu mới cho tài khoản '" +
                                 username + "' đã có hiệu lực.");
    }
  }

  // 2. Tài khoản nội bộ (Quản trị viên / Nhân viên) bị cấm tự reset để tránh chiếm quyền
  if (employeeService != nullptr) {
    Employee *emp = employeeService->getEmployeeByUsername(username);
    if (emp == nullptr) {
      emp = employeeService->getEmployeeByPhone(username);
    }
    if (emp != nullptr) {
      return OperationResult::Fail(
          "Tài khoản nội bộ (Quản trị viên / Nhân viên) không được phép tự đặt lại mật khẩu. "
          "Vui lòng liên hệ Quản trị viên!");
    }
  }

  return OperationResult::Fail(
      "Không tìm thấy tài khoản với tên đăng nhập: " + username);
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

  return OperationResult::Ok(
      "Đổi mật khẩu thành công! Hãy dùng mật khẩu mới ở lần đăng nhập sau.");
}

void AppController::logout() {
  currentSession = UserSessionViewModel();
  userCarts.erase("GUEST");
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
  REQUIRE_ADMIN_ROLE();
  if (!RealTimeManager::getInstance().setDateTime(newTimeStr)) {
    return OperationResult::Fail(
        "Định dạng ngày giờ không hợp lệ hoặc không thể lưu file thời gian "
        "(hỗ trợ DD/MM/YYYY HH:MM:SS hoặc YYYY-MM-DD HH:MM:SS)!");
  }
  return OperationResult::Ok(
      "Đã cập nhật mốc thời gian thực của hệ thống thành công!");
}

OperationResult AppController::advanceSimulatedDays(int days) {
  REQUIRE_ADMIN_ROLE();
  if (!RealTimeManager::getInstance().advanceDays(days)) {
    return OperationResult::Fail(
        "Không thể điều chỉnh hoặc lưu bước nhảy thời gian mô phỏng!");
  }
  return OperationResult::Ok(
      "Đã điều chỉnh bước nhảy thời gian mô phỏng thành công!");
}

void AppController::advanceSimulatedSeconds(int seconds) {
  if (!currentSession.isAuthenticated) {
    return;
  }
  if (!currentSession.isAdmin() && (seconds <= 0 || seconds > 60)) {
    return;
  }
  RealTimeManager::getInstance().advanceSeconds(seconds);
}

OperationResult AppController::resetSimulatedTimeToNow() {
  REQUIRE_ADMIN_ROLE();
  if (!RealTimeManager::getInstance().resetToSystemNow()) {
    return OperationResult::Fail(
        "Không thể đồng bộ hoặc lưu lại thời gian theo đồng hồ thực của máy tính!");
  }
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

  // Cập nhật xuống file qua Service trước (dùng bản sao để tránh UAF/Double-Free và bảo toàn RAM khi ghi lỗi)
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
      Customer updatedCust = *cust;
      updatedCust.setName(Name(first, mid, last));
      updatedCust.setPhoneNumber(phone);
      updatedCust.setEmail(email);
      updatedCust.setAddress(address);
      std::string serviceErr;
      if (!customerService->updateCustomer(updatedCust, serviceErr)) {
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
      Employee *updatedEmp = emp->clone();
      updatedEmp->setName(Name(first, mid, last));
      updatedEmp->setPhoneNumber(phone);
      updatedEmp->setEmail(email);
      updatedEmp->setAddress(address);
      std::string serviceErr;
      if (!employeeService->updateEmployee(updatedEmp, serviceErr)) {
        delete updatedEmp;
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

  return OperationResult::Ok("Cập nhật thông tin hồ sơ thành công!");
}
