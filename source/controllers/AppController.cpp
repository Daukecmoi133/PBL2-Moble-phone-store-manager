#include "AppController.h"
#include "../ui/mock/MockDataStore.h"
#include "../ui/mock/RealTimeManager.h"
#include "../utils/IdGenerator.h"
#include "../utils/Utf8Helper.h"
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cctype>

namespace {
// ---- Tiện ích xác thực dữ liệu đầu vào dùng chung ----
std::string trimCopy(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

// SĐT Việt Nam: đúng 10 chữ số, bắt đầu bằng 0
bool isValidPhone(const std::string& phone) {
    if (phone.size() != 10 || phone[0] != '0') return false;
    for (char c : phone) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

// Email cơ bản: có đúng 1 '@', phần tên khác rỗng, miền có dấu '.' không ở đầu/cuối, không có khoảng trắng
bool isValidEmail(const std::string& email) {
    size_t at = email.find('@');
    if (at == std::string::npos || at == 0 || email.find('@', at + 1) != std::string::npos) return false;
    for (char c : email) {
        if (std::isspace(static_cast<unsigned char>(c))) return false;
    }
    std::string domain = email.substr(at + 1);
    size_t dot = domain.find('.');
    return dot != std::string::npos && dot > 0 && domain.back() != '.';
}

// Phân tích ngày DD/MM/YYYY nghiêm ngặt (kiểm tra tháng, số ngày trong tháng, năm nhuận)
bool parseDateDMY(const std::string& s, int& d, int& m, int& y) {
    int consumed = 0;
    if (std::sscanf(s.c_str(), "%d/%d/%d%n", &d, &m, &y, &consumed) != 3) return false;
    if (static_cast<size_t>(consumed) != s.size()) return false;
    if (y < 1900 || y > 9999 || m < 1 || m > 12 || d < 1) return false;
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[m - 1];
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (m == 2 && leap) maxDay = 29;
    return d <= maxDay;
}

// Phân quyền ở tầng nghiệp vụ: chặn thao tác nhạy cảm dù giao diện có bị gọi trực tiếp hay không
#define REQUIRE_ADMIN_ROLE() \
    do { \
        if (!currentSession.isAuthenticated || !currentSession.isAdmin()) { \
            return OperationResult::Fail("Từ chối truy cập: Thao tác này chỉ dành cho Quản trị viên!"); \
        } \
    } while (0)

#define REQUIRE_ADMIN_OR_WAREHOUSE_ROLE() \
    do { \
        if (!currentSession.isAuthenticated || \
            !(currentSession.isAdmin() || currentSession.isWarehouse())) { \
            return OperationResult::Fail("Từ chối truy cập: Thao tác này chỉ dành cho Quản trị viên hoặc Thủ kho!"); \
        } \
    } while (0)

template <typename Records>
std::vector<std::string> collectIds(const Records& records) {
    std::vector<std::string> ids;
    ids.reserve(records.size());
    for (const auto& record : records) ids.push_back(record.id);
    return ids;
}

}

AppController::AppController(
    PhoneService* ps,
    OrderService* os,
    CustomerService* cs,
    EmployeeService* es,
    InvoiceService* is,
    SupplierService* ss,
    DiscountService* ds,
    ReportService* rs,
    AuthService* as
) : phoneService(ps),
    orderService(os),
    customerService(cs),
    employeeService(es),
    invoiceService(is),
    supplierService(ss),
    discountService(ds),
    reportService(rs),
    authService(as)
{}

// === XÁC THỰC & PHIÊN LÀM VIỆC ===

LoginResponse AppController::login(const LoginRequest& request) {
    LoginResponse response;

    if (request.username.empty() || request.password.empty()) {
        response.result = OperationResult::Fail("Tên đăng nhập và mật khẩu không được để trống!");
        return response;
    }

    // 1. Kiểm tra tài khoản trong MockDataStore (hỗ trợ demo presentation nhanh)
    auto& store = MockDataStore::getInstance();
    
    // Tìm trong danh sách nhân viên demo
    for (const auto& emp : store.employees) {
        if (emp.username == request.username) {
            // Kiểm tra mật khẩu demo hoặc nếu người dùng gõ đúng username
            const bool legacyPasswordOk = (request.password == "DemoOnly-Admin-2026" ||
                request.password == "DemoOnly-Sales-2026" ||
                request.password == "DemoOnly-Warehouse-2026" ||
                request.password == "123456" ||
                request.password == "admin" ||
                request.password == "sales" ||
                request.password == "warehouse" ||
                false);
            // Nếu tài khoản đã đổi/đặt lại mật khẩu thì CHỈ mật khẩu mới còn hiệu lực
            const bool passwordOk = store.hasCustomPassword(emp.username)
                ? store.verifyPassword(emp.username, request.password)
                : legacyPasswordOk;
            if (passwordOk) {

                currentSession.userId = emp.id;
                currentSession.username = emp.username;
                currentSession.fullName = emp.fullName;
                currentSession.role = emp.role;
                currentSession.roleName = emp.roleName;
                currentSession.phone = emp.phone;
                currentSession.email = emp.email;
                currentSession.address = emp.address;
                currentSession.isAuthenticated = true;

                response.result = OperationResult::Ok("Đăng nhập thành công! Chào mừng " + emp.fullName);
                response.session = currentSession;
                return response;
            } else {
                response.result = OperationResult::Fail("Mật khẩu không chính xác!");
                return response;
            }
        }
    }

    // Tìm trong danh sách khách hàng demo
    for (const auto& cust : store.customers) {
        if (cust.username == request.username) {
            const bool legacyPasswordOk = (request.password == "DemoOnly-Customer-001" ||
                request.password == "123456" ||
                request.password == "customer" ||
                false);
            const bool passwordOk = store.hasCustomPassword(cust.username)
                ? store.verifyPassword(cust.username, request.password)
                : legacyPasswordOk;
            if (passwordOk) {

                currentSession.userId = cust.id;
                currentSession.username = cust.username;
                currentSession.fullName = cust.fullName;
                currentSession.role = RoleType::CUSTOMER;
                currentSession.roleName = "Khách hàng";
                currentSession.phone = cust.phone;
                currentSession.email = cust.email;
                currentSession.address = cust.address;
                currentSession.isAuthenticated = true;

                response.result = OperationResult::Ok("Đăng nhập thành công! Chào mừng " + cust.fullName);
                response.session = currentSession;
                return response;
            } else {
                response.result = OperationResult::Fail("Mật khẩu không chính xác!");
                return response;
            }
        }
    }

    response.result = OperationResult::Fail("Tài khoản không tồn tại hoặc đã bị khóa!");
    return response;
}

OperationResult AppController::registerCustomer(const std::string& rawUsername, const std::string& password,
                                               const std::string& rawFullName, const std::string& rawPhone,
                                               const std::string& rawEmail) {
    // Cắt khoảng trắng thừa trước khi kiểm tra và lưu
    const std::string username = trimCopy(rawUsername);
    const std::string fullName = trimCopy(rawFullName);
    const std::string phone = trimCopy(rawPhone);
    const std::string email = trimCopy(rawEmail);

    if (username.empty() || password.empty() || fullName.empty() || phone.empty()) {
        return OperationResult::Fail("Vui lòng điền đầy đủ họ tên, số điện thoại, tên đăng nhập và mật khẩu!");
    }
    if (username.find_first_of(" \t") != std::string::npos) {
        return OperationResult::Fail("Tên đăng nhập không được chứa khoảng trắng!");
    }
    if (!isValidPhone(phone)) {
        return OperationResult::Fail("Số điện thoại không hợp lệ! Phải gồm đúng 10 chữ số và bắt đầu bằng 0 (ví dụ 0912345678).");
    }
    if (!email.empty() && !isValidEmail(email)) {
        return OperationResult::Fail("Email không hợp lệ! Ví dụ đúng: an.nguyen@example.com");
    }
    if (password.length() < 4) {
        return OperationResult::Fail("Mật khẩu bảo mật phải có ít nhất 4 ký tự!");
    }

    auto& store = MockDataStore::getInstance();
    bool ok = store.registerCustomer(username, password, fullName, phone, email);
    if (!ok) {
        return OperationResult::Fail("Tên đăng nhập '" + username + "' đã được sử dụng! Vui lòng chọn tên khác.");
    }
    return OperationResult::Ok("Đăng ký tài khoản thành công! Bạn có thể sử dụng tài khoản vừa tạo để đăng nhập.");
}

OperationResult AppController::resetPasswordSimulation(const std::string& rawUsername, const std::string& newPassword,
                                                       const std::string& rawVerifyContact) {
    const std::string username = trimCopy(rawUsername);
    const std::string contact = trimCopy(rawVerifyContact);
    if (username.empty() || newPassword.empty()) {
        return OperationResult::Fail("Tên đăng nhập và mật khẩu mới không được để trống!");
    }
    if (contact.empty()) {
        return OperationResult::Fail("Vui lòng nhập Số điện thoại hoặc Email đã đăng ký để xác minh danh tính!");
    }
    if (newPassword.length() < 4) {
        return OperationResult::Fail("Mật khẩu mới phải có ít nhất 4 ký tự!");
    }

    auto& store = MockDataStore::getInstance();
    auto sameText = [](const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    };

    bool found = false;
    bool verified = false;
    for (const auto& c : store.customers) {
        if (c.username == username) {
            found = true;
            verified = (!c.phone.empty() && c.phone == contact) || (!c.email.empty() && sameText(c.email, contact));
            break;
        }
    }
    if (!found) {
        for (const auto& e : store.employees) {
            if (e.username == username) {
                found = true;
                verified = (!e.phone.empty() && e.phone == contact) || (!e.email.empty() && sameText(e.email, contact));
                break;
            }
        }
    }
    if (!found) {
        return OperationResult::Fail("Không tìm thấy tài khoản với tên đăng nhập: " + username);
    }
    if (!verified) {
        return OperationResult::Fail("Thông tin xác minh (SĐT hoặc Email) không khớp với tài khoản '" + username + "'!");
    }
    store.resetPassword(username, newPassword);
    return OperationResult::Ok("Mô phỏng khôi phục mật khẩu thành công! Mật khẩu mới cho tài khoản '" + username + "' đã có hiệu lực.");
}

OperationResult AppController::changePassword(const std::string& oldPassword, const std::string& newPassword) {
    if (!currentSession.isAuthenticated) {
        return OperationResult::Fail("Bạn chưa đăng nhập!");
    }
    if (oldPassword.empty() || newPassword.empty()) {
        return OperationResult::Fail("Vui lòng nhập đầy đủ mật khẩu hiện tại và mật khẩu mới!");
    }
    if (newPassword.length() < 4) {
        return OperationResult::Fail("Mật khẩu mới phải có ít nhất 4 ký tự!");
    }
    if (newPassword == oldPassword) {
        return OperationResult::Fail("Mật khẩu mới phải khác mật khẩu hiện tại!");
    }

    // Xác thực lại mật khẩu cũ bằng đúng luồng đăng nhập, rồi khôi phục phiên hiện tại
    const UserSessionViewModel savedSession = currentSession;
    LoginRequest verifyReq;
    verifyReq.username = savedSession.username;
    verifyReq.password = oldPassword;
    LoginResponse verifyResp = login(verifyReq);
    currentSession = savedSession;
    if (!verifyResp.result.success) {
        return OperationResult::Fail("Mật khẩu hiện tại không chính xác!");
    }

    MockDataStore::getInstance().resetPassword(savedSession.username, newPassword);
    return OperationResult::Ok("Đổi mật khẩu thành công! Hãy dùng mật khẩu mới ở lần đăng nhập sau.");
}

void AppController::logout() {
    currentSession = UserSessionViewModel();
}

const UserSessionViewModel& AppController::getCurrentUser() const {
    return currentSession;
}

// === HỆ THỐNG KIỂM SOÁT THỜI GIAN THỰC (REAL-TIME SIMULATION) ===

std::string AppController::getSimulatedTime() {
    return RealTimeManager::getInstance().getCurrentDateTime();
}

std::string AppController::getSimulatedDate() {
    return RealTimeManager::getInstance().getCurrentDate();
}

OperationResult AppController::setSimulatedTime(const std::string& newTimeStr) {
    RealTimeManager::getInstance().setDateTime(newTimeStr);
    return OperationResult::Ok("Đã cập nhật mốc thời gian thực của hệ thống thành công!");
}

OperationResult AppController::advanceSimulatedDays(int days) {
    RealTimeManager::getInstance().advanceDays(days);
    return OperationResult::Ok("Đã điều chỉnh bước nhảy thời gian mô phỏng thành công!");
}

OperationResult AppController::resetSimulatedTimeToNow() {
    RealTimeManager::getInstance().resetToSystemNow();
    return OperationResult::Ok("Đã đồng bộ lại thời gian theo đồng hồ thực của máy tính!");
}

OperationResult AppController::updateProfile(const std::string& fullName, const std::string& phone,
                                            const std::string& email, const std::string& address) {
    if (!currentSession.isAuthenticated) {
        return OperationResult::Fail("Bạn chưa đăng nhập!");
    }
    if (fullName.empty() || phone.empty()) {
        return OperationResult::Fail("Họ tên và số điện thoại không được để trống!");
    }
    if (!isValidPhone(trimCopy(phone))) {
        return OperationResult::Fail("Số điện thoại không hợp lệ! Phải gồm đúng 10 chữ số và bắt đầu bằng 0.");
    }
    if (!trimCopy(email).empty() && !isValidEmail(trimCopy(email))) {
        return OperationResult::Fail("Email không hợp lệ!");
    }

    currentSession.fullName = fullName;
    currentSession.phone = phone;
    currentSession.email = email;
    currentSession.address = address;

    // Cập nhật trong Mock Store
    auto& store = MockDataStore::getInstance();
    if (currentSession.isCustomer()) {
        for (auto& c : store.customers) {
            if (c.id == currentSession.userId) {
                c.fullName = fullName;
                c.phone = phone;
                c.email = email;
                c.address = address;
                break;
            }
        }
    } else {
        for (auto& e : store.employees) {
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

// === SẢN PHẨM ===

std::vector<ProductItemViewModel> AppController::getProducts(const ProductFilterRequest& filter) {
    auto& store = MockDataStore::getInstance();
    std::vector<ProductItemViewModel> result;

    for (const auto& p : store.products) {
        // Keyword filter
        if (!filter.keyword.empty()) {
            if (!Utf8Helper::containsCaseInsensitive(p.name, filter.keyword) &&
                !Utf8Helper::containsCaseInsensitive(p.brand, filter.keyword) &&
                !Utf8Helper::containsCaseInsensitive(p.id, filter.keyword)) {
                continue;
            }
        }

        // Brand filter
        if (!filter.brand.empty() && filter.brand != "Tất cả") {
            if (p.brand != filter.brand) continue;
        }

        // Price range filter
        if (filter.minPrice > 0.0 && p.sellingPrice < filter.minPrice) continue;
        if (filter.maxPrice > 0.0 && p.sellingPrice > filter.maxPrice) continue;

        // In-stock only
        if (filter.onlyInStock && p.availableStock <= 0) continue;

        result.push_back(p);
    }

    // Sort
    if (filter.sortBy == "price_asc") {
        std::sort(result.begin(), result.end(), [](const ProductItemViewModel& a, const ProductItemViewModel& b) {
            return a.sellingPrice < b.sellingPrice;
        });
    } else if (filter.sortBy == "price_desc") {
        std::sort(result.begin(), result.end(), [](const ProductItemViewModel& a, const ProductItemViewModel& b) {
            return a.sellingPrice > b.sellingPrice;
        });
    } else if (filter.sortBy == "stock_desc") {
        std::sort(result.begin(), result.end(), [](const ProductItemViewModel& a, const ProductItemViewModel& b) {
            return a.availableStock > b.availableStock;
        });
    } else { // name_asc
        std::sort(result.begin(), result.end(), [](const ProductItemViewModel& a, const ProductItemViewModel& b) {
            return a.name < b.name;
        });
    }

    return result;
}

ProductItemViewModel AppController::getProductById(const std::string& id) {
    auto& store = MockDataStore::getInstance();
    for (const auto& p : store.products) {
        if (p.id == id) return p;
    }
    return ProductItemViewModel();
}

OperationResult AppController::addProduct(const ProductItemViewModel& rawProduct) {
    REQUIRE_ADMIN_ROLE();
    ProductItemViewModel product = rawProduct;
    product.name = trimCopy(product.name);
    product.brand = trimCopy(product.brand);
    if (product.name.empty() || product.brand.empty()) {
        return OperationResult::Fail("Tên và thương hiệu sản phẩm không được để trống!");
    }
    if (product.sellingPrice <= 0 || product.importPrice <= 0) {
        return OperationResult::Fail("Giá nhập và giá bán phải lớn hơn 0!");
    }
    if (product.actualStock < 0) {
        return OperationResult::Fail("Số lượng tồn kho không được âm!");
    }

    auto& store = MockDataStore::getInstance();
    std::string generatedId;
    std::string idError;
    if (!IdGenerator::nextId("PHN", "", collectIds(store.products), generatedId, idError)) {
        return OperationResult::Fail(idError);
    }

    ProductItemViewModel newItem = product;
    newItem.id = generatedId;
    newItem.reservedStock = 0;
    newItem.calculateStockStatus();
    store.products.push_back(newItem);

    return OperationResult::Ok("Thêm sản phẩm '" + product.name + "' thành công!");
}

OperationResult AppController::updateProduct(const ProductItemViewModel& rawProduct) {
    REQUIRE_ADMIN_ROLE();
    ProductItemViewModel product = rawProduct;
    product.name = trimCopy(product.name);
    product.brand = trimCopy(product.brand);
    if (!IdGenerator::isValidId(product.id, "PHN") || product.name.empty()) {
        return OperationResult::Fail("Thông tin sản phẩm không hợp lệ!");
    }
    if (product.sellingPrice <= 0) {
        return OperationResult::Fail("Giá bán phải lớn hơn 0!");
    }

    auto& store = MockDataStore::getInstance();
    for (auto& p : store.products) {
        if (p.id == product.id) {
            p.name = product.name;
            p.brand = product.brand;
            p.os = product.os;
            p.sellingPrice = product.sellingPrice;
            p.importPrice = product.importPrice;
            p.actualStock = product.actualStock;
            p.manufacturer = product.manufacturer;
            p.specs = product.specs;
            p.imageUrl = product.imageUrl;
            p.calculateStockStatus();
            return OperationResult::Ok("Cập nhật sản phẩm '" + product.name + "' thành công!");
        }
    }

    return OperationResult::Fail("Không tìm thấy sản phẩm có mã '" + product.id + "' để cập nhật!");
}

OperationResult AppController::deleteProduct(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(id, "PHN")) {
        return OperationResult::Fail("Mã sản phẩm không đúng định dạng PHN-000001!");
    }
    auto& store = MockDataStore::getInstance();

    // Preserve products referenced by transaction and warranty history.
    for (const auto& ord : store.orders) {
        for (const auto& d : ord.details) {
            if (d.phoneId == id) {
                return OperationResult::Fail("Không thể xóa sản phẩm '" + id +
                    "' vì đang được tham chiếu trong đơn hàng (" + ord.id + ")!");
            }
        }
    }
    for (const auto& invoice : store.invoices) {
        for (const auto& d : invoice.details) {
            if (d.phoneId == id) {
                return OperationResult::Fail("Không thể xóa sản phẩm '" + id +
                    "' vì đang được tham chiếu trong hóa đơn (" + invoice.id + ")!");
            }
        }
    }
    for (const auto& receipt : store.receipts) {
        for (const auto& d : receipt.details) {
            if (d.phoneId == id) {
                return OperationResult::Fail("Không thể xóa sản phẩm '" + id +
                    "' vì đang được tham chiếu trong phiếu nhập (" + receipt.id + ")!");
            }
        }
    }
    for (const auto& warranty : store.warranties) {
        if (warranty.phoneId == id) {
            return OperationResult::Fail("Không thể xóa sản phẩm '" + id +
                "' vì đang được tham chiếu trong yêu cầu bảo hành (" + warranty.id + ")!");
        }
    }

    for (auto it = store.products.begin(); it != store.products.end(); ++it) {
        if (it->id == id) {
            std::string name = it->name;
            store.products.erase(it);
            return OperationResult::Ok("Đã xóa sản phẩm '" + name + "'!");
        }
    }

    return OperationResult::Fail("Không tìm thấy sản phẩm để xóa!");
}

// === GIỎ HÀNG & CHECKOUT ===

CartSummaryViewModel AppController::getCart(const std::string& customerId) {
    auto& store = MockDataStore::getInstance();
    if (store.customerCart.customerId != customerId) {
        store.customerCart.customerId = customerId;
    }
    store.customerCart.recalculate();
    return store.customerCart;
}

OperationResult AppController::addToCart(const std::string& customerId, const std::string& phoneId, int quantity) {
    if (quantity <= 0) {
        return OperationResult::Fail("Số lượng thêm vào giỏ phải lớn hơn 0!");
    }

    ProductItemViewModel product = getProductById(phoneId);
    if (product.id.empty()) {
        return OperationResult::Fail("Không tìm thấy sản phẩm!");
    }
    if (product.availableStock < quantity) {
        return OperationResult::Fail("Không đủ số lượng có thể bán! Tồn kho khả dụng chỉ còn: " +
                                     std::to_string(product.availableStock));
    }

    auto& store = MockDataStore::getInstance();
    store.customerCart.customerId = customerId;

    bool found = false;
    for (auto& item : store.customerCart.items) {
        if (item.phoneId == phoneId) {
            if (item.quantity + quantity > product.availableStock) {
                return OperationResult::Fail("Tổng số lượng trong giỏ (" + std::to_string(item.quantity + quantity) +
                                             ") vượt quá tồn kho khả dụng (" + std::to_string(product.availableStock) + ")!");
            }
            item.quantity += quantity;
            item.calculateSubtotal();
            found = true;
            break;
        }
    }

    if (!found) {
        CartItemViewModel newItem;
        newItem.phoneId = product.id;
        newItem.phoneName = product.name;
        newItem.brand = product.brand;
        newItem.unitPrice = product.sellingPrice;
        newItem.quantity = quantity;
        newItem.availableStock = product.availableStock;
        newItem.calculateSubtotal();
        store.customerCart.items.push_back(newItem);
    }

    store.customerCart.recalculate();
    return OperationResult::Ok("Đã thêm '" + product.name + "' vào giỏ hàng!");
}

OperationResult AppController::updateCartQuantity(const std::string& customerId, const std::string& phoneId, int newQuantity) {
    auto& store = MockDataStore::getInstance();
    if (newQuantity <= 0) {
        return removeFromCart(customerId, phoneId);
    }

    ProductItemViewModel product = getProductById(phoneId);
    if (newQuantity > product.availableStock) {
        return OperationResult::Fail("Số lượng vượt quá tồn kho khả dụng (" + std::to_string(product.availableStock) + ")!");
    }

    for (auto& item : store.customerCart.items) {
        if (item.phoneId == phoneId) {
            item.quantity = newQuantity;
            item.calculateSubtotal();
            store.customerCart.recalculate();
            return OperationResult::Ok("Đã cập nhật số lượng!");
        }
    }

    return OperationResult::Fail("Sản phẩm không có trong giỏ hàng!");
}

OperationResult AppController::removeFromCart(const std::string& customerId, const std::string& phoneId) {
    (void)customerId;
    auto& store = MockDataStore::getInstance();
    for (auto it = store.customerCart.items.begin(); it != store.customerCart.items.end(); ++it) {
        if (it->phoneId == phoneId) {
            std::string name = it->phoneName;
            store.customerCart.items.erase(it);
            store.customerCart.recalculate();
            return OperationResult::Ok("Đã xóa '" + name + "' khỏi giỏ hàng!");
        }
    }
    return OperationResult::Fail("Không tìm thấy sản phẩm trong giỏ hàng!");
}

OperationResult AppController::applyCoupon(const std::string& customerId, const std::string& couponCode) {
    (void)customerId;
    auto& store = MockDataStore::getInstance();
    store.customerCart.recalculate();

    if (couponCode.empty()) {
        store.customerCart.appliedDiscountCode = "";
        store.customerCart.discountAmount = 0.0;
        store.customerCart.recalculate();
        return OperationResult::Ok("Đã hủy áp dụng mã giảm giá.");
    }

    double discount = 0.0;
    OperationResult res = validateDiscount(couponCode, store.customerCart.subtotal, discount);
    if (!res.success) {
        return res;
    }

    store.customerCart.appliedDiscountCode = couponCode;
    store.customerCart.discountAmount = discount;
    store.customerCart.recalculate();

    return OperationResult::Ok("Áp dụng mã giảm giá '" + couponCode + "' thành công!");
}

OperationResult AppController::checkout(const CheckoutRequest& request, std::string& outOrderId) {
    outOrderId.clear();
    auto& store = MockDataStore::getInstance();
    if (store.customerCart.items.empty()) {
        return OperationResult::Fail("Giỏ hàng của bạn đang trống! Hãy chọn sản phẩm trước.");
    }
    if (request.receiverName.empty() || request.receiverPhone.empty()) {
        return OperationResult::Fail("Họ tên và số điện thoại người nhận không được để trống!");
    }
    if (request.deliveryMethod == 1 && request.deliveryAddress.empty()) { // Delivery
        return OperationResult::Fail("Vui lòng nhập địa chỉ nhận hàng khi chọn giao tận nơi!");
    }
    const std::string customerId =
        request.customerId.empty() ? currentSession.userId : request.customerId;
    if (!IdGenerator::isValidId(customerId, "CUS") ||
        getCustomerById(customerId).id.empty()) {
        return OperationResult::Fail("Khách hàng đặt đơn không hợp lệ hoặc không tồn tại!");
    }

    // Kiểm tra tồn kho khả dụng lần cuối
    for (const auto& item : store.customerCart.items) {
        ProductItemViewModel p = getProductById(item.phoneId);
        if (p.availableStock < item.quantity) {
            return OperationResult::Fail("Sản phẩm '" + item.phoneName + "' không còn đủ số lượng khả dụng!");
        }
    }

    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("ORD", idDate, collectIds(store.orders), outOrderId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã đơn hàng." : idError);
    }

    // Giữ chỗ tồn kho (reservedStock += quantity)
    for (const auto& item : store.customerCart.items) {
        for (auto& p : store.products) {
            if (p.id == item.phoneId) {
                p.reservedStock += item.quantity;
                p.calculateStockStatus();
                break;
            }
        }
    }

    // Lấy thời gian hiện tại
    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    OrderItemViewModel newOrder;
    newOrder.id = outOrderId;
    newOrder.customerId = customerId;
    newOrder.customerName = request.receiverName;
    newOrder.orderTime = timeBuf;
    newOrder.status = OrderStatus::PENDING;
    newOrder.statusText = "Chờ xử lý";
    newOrder.deliveryMethod = (request.deliveryMethod == 0) ? DeliveryMethod::PICKUP : DeliveryMethod::DELIVERY;
    newOrder.deliveryMethodText = deliveryMethodToString(newOrder.deliveryMethod);
    newOrder.paymentMethod = (request.paymentMethod == 0) ? PaymentMethod::CASH : PaymentMethod::BANK_TRANSFER;
    newOrder.paymentMethodText = paymentMethodToString(newOrder.paymentMethod);
    newOrder.paymentStatus = (newOrder.paymentMethod == PaymentMethod::BANK_TRANSFER) ? PaymentStatus::PAID : PaymentStatus::UNPAID;
    newOrder.paymentStatusText = paymentStatusToString(newOrder.paymentStatus);
    newOrder.subtotal = store.customerCart.subtotal;
    newOrder.discountAmount = store.customerCart.discountAmount;
    newOrder.total = store.customerCart.grandTotal;
    newOrder.receiverName = request.receiverName;
    newOrder.receiverPhone = request.receiverPhone;
    newOrder.deliveryAddress = (request.deliveryMethod == 0)
        ? "Cửa hàng PBL2 Store - 54 Nguyễn Lương Bằng, Đà Nẵng"
        : request.deliveryAddress;

    for (const auto& cItem : store.customerCart.items) {
        OrderDetailItemViewModel d;
        d.phoneId = cItem.phoneId;
        d.phoneName = cItem.phoneName;
        d.quantity = cItem.quantity;
        d.unitPrice = cItem.unitPrice;
        d.subtotal = cItem.subtotal;
        newOrder.details.push_back(d);
    }

    store.orders.insert(store.orders.begin(), newOrder);

    // Xóa giỏ hàng
    store.customerCart.items.clear();
    store.customerCart.appliedDiscountCode = "";
    store.customerCart.discountAmount = 0.0;
    store.customerCart.recalculate();

    return OperationResult::Ok("Đặt hàng thành công! Mã đơn: " + outOrderId);
}

// === ĐƠN HÀNG ===

std::vector<OrderItemViewModel> AppController::getAllOrders() {
    return MockDataStore::getInstance().orders;
}

std::vector<OrderItemViewModel> AppController::filterOrders(const std::string& keyword, int statusFilter, int paymentFilter, int typeFilter) {
    auto& store = MockDataStore::getInstance();
    std::vector<OrderItemViewModel> result;

    for (const auto& ord : store.orders) {
        // Keyword filter: check id, customerName, receiverPhone
        if (!keyword.empty()) {
            bool match = Utf8Helper::containsCaseInsensitive(ord.id, keyword) ||
                         Utf8Helper::containsCaseInsensitive(ord.customerName, keyword) ||
                         Utf8Helper::containsCaseInsensitive(ord.receiverPhone, keyword);
            if (!match) continue;
        }

        // Status filter:
        if (statusFilter >= 0) {
            if (static_cast<int>(ord.status) != statusFilter) continue;
        }

        // Payment filter:
        if (paymentFilter >= 0) {
            if (static_cast<int>(ord.paymentStatus) != paymentFilter) continue;
        }

        // Type filter: 0 = In store (Tại quầy), 1 = Online
        if (typeFilter >= 0) {
            if (static_cast<int>(ord.deliveryMethod) != typeFilter) continue;
        }

        result.push_back(ord);
    }
    return result;
}

std::vector<OrderItemViewModel> AppController::getCustomerOrders(const std::string& customerId) {
    auto& store = MockDataStore::getInstance();
    std::vector<OrderItemViewModel> result;
    for (const auto& ord : store.orders) {
        if (ord.customerId == customerId || customerId.empty()) {
            result.push_back(ord);
        }
    }
    return result;
}

OrderItemViewModel AppController::getOrderById(const std::string& orderId) {
    auto& store = MockDataStore::getInstance();
    for (const auto& ord : store.orders) {
        if (ord.id == orderId) return ord;
    }
    return OrderItemViewModel();
}

OperationResult AppController::updateOrderStatus(const std::string& orderId, OrderStatus newStatus) {
    if (!IdGenerator::isValidId(orderId, "ORD", true)) {
        return OperationResult::Fail("Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (auto& ord : store.orders) {
        if (ord.id == orderId) {
            // Không cho phép sửa nếu đơn đã Hoàn thành hoặc Hủy
            if (ord.status == OrderStatus::COMPLETED) {
                return OperationResult::Fail("Đơn hàng đã hoàn thành, không thể thay đổi trạng thái!");
            }
            if (ord.status == OrderStatus::CANCELLED) {
                return OperationResult::Fail("Đơn hàng đã bị hủy, không thể thay đổi trạng thái!");
            }

            if (newStatus == OrderStatus::COMPLETED) {
                bool hasInvoice = false;
                for (const auto& invoice : store.invoices) {
                    if (invoice.orderId == ord.id) {
                        hasInvoice = true;
                        break;
                    }
                }
                if (!hasInvoice) {
                    OperationResult invoiceResult =
                        createInvoiceForOrder(ord.id, "EMP-000002", ord.paymentMethod);
                    if (!invoiceResult.success) return invoiceResult;
                }
            }

            OrderStatus oldStatus = ord.status;
            ord.status = newStatus;
            ord.statusText = orderStatusToString(newStatus);

            // Xử lý trừ kho hoặc giải phóng giữ chỗ theo quy tắc nghiệp vụ:
            if (newStatus == OrderStatus::COMPLETED) {
                // Trừ tồn kho thực tế, giải phóng giữ chỗ
                for (const auto& d : ord.details) {
                    for (auto& p : store.products) {
                        if (p.id == d.phoneId) {
                            p.actualStock -= d.quantity;
                            p.reservedStock -= d.quantity;
                            if (p.actualStock < 0) p.actualStock = 0;
                            if (p.reservedStock < 0) p.reservedStock = 0;
                            p.calculateStockStatus();
                            break;
                        }
                    }
                }
                ord.paymentStatus = PaymentStatus::PAID;
                ord.paymentStatusText = paymentStatusToString(ord.paymentStatus);

                // Cộng điểm tích lũy cho khách hàng (100.000 đ = 1 điểm)
                int pointsToAdd = static_cast<int>(ord.total / 100000.0);
                for (auto& c : store.customers) {
                    if (c.id == ord.customerId) {
                        c.points += pointsToAdd;
                        c.updateTier();
                        break;
                    }
                }
            } else if (newStatus == OrderStatus::CANCELLED) {
                // Giải phóng giữ chỗ
                for (const auto& d : ord.details) {
                    for (auto& p : store.products) {
                        if (p.id == d.phoneId) {
                            p.reservedStock -= d.quantity;
                            if (p.reservedStock < 0) p.reservedStock = 0;
                            p.calculateStockStatus();
                            break;
                        }
                    }
                }
            }

            return OperationResult::Ok("Chuyển trạng thái đơn " + orderId + " từ '" +
                                       orderStatusToString(oldStatus) + "' thành '" +
                                       orderStatusToString(newStatus) + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy đơn hàng " + orderId);
}

OperationResult AppController::cancelOrder(const std::string& orderId) {
    if (!IdGenerator::isValidId(orderId, "ORD", true)) {
        return OperationResult::Fail("Mã đơn hàng không đúng định dạng ORD-ddmmyyyy-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (const auto& ord : store.orders) {
        if (ord.id == orderId) {
            if (!ord.canCancel()) {
                return OperationResult::Fail("Chỉ có thể hủy đơn hàng đang ở trạng thái 'Chờ xử lý' (PENDING)!");
            }
            return updateOrderStatus(orderId, OrderStatus::CANCELLED);
        }
    }
    return OperationResult::Fail("Không tìm thấy đơn hàng!");
}

// === HÓA ĐƠN ===

std::vector<InvoiceItemViewModel> AppController::getAllInvoices() {
    return MockDataStore::getInstance().invoices;
}

InvoiceItemViewModel AppController::getInvoiceById(const std::string& invoiceId) {
    auto& store = MockDataStore::getInstance();
    for (const auto& inv : store.invoices) {
        if (inv.id == invoiceId) return inv;
    }
    return InvoiceItemViewModel();
}

OperationResult AppController::createInvoiceForOrder(const std::string& orderId, const std::string& employeeId, PaymentMethod method) {
    if (!IdGenerator::isValidId(orderId, "ORD", true) ||
        !IdGenerator::isValidId(employeeId, "EMP")) {
        return OperationResult::Fail("Mã đơn hàng hoặc nhân viên không đúng định dạng ID!");
    }
    auto& store = MockDataStore::getInstance();
    if (getEmployeeById(employeeId).id.empty()) {
        return OperationResult::Fail("Không tìm thấy nhân viên được tham chiếu!");
    }

    // Kiểm tra xem đơn hàng đã có hóa đơn chưa (mỗi đơn chỉ 1 hóa đơn)
    for (const auto& inv : store.invoices) {
        if (inv.orderId == orderId) {
            return OperationResult::Fail("Đơn hàng '" + orderId + "' đã được xuất hóa đơn (" + inv.id + ")!");
        }
    }

    OrderItemViewModel ord = getOrderById(orderId);
    if (ord.id.empty()) {
        return OperationResult::Fail("Không tìm thấy đơn hàng!");
    }
    if (!ord.customerId.empty() &&
        (!IdGenerator::isValidId(ord.customerId, "CUS") ||
         getCustomerById(ord.customerId).id.empty())) {
        return OperationResult::Fail("Khách hàng của đơn hàng không hợp lệ hoặc không tồn tại!");
    }

    std::string invId;
    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("INV", idDate, collectIds(store.invoices), invId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã hóa đơn." : idError);
    }

    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    InvoiceItemViewModel newInv;
    newInv.id = invId;
    newInv.orderId = ord.id;
    newInv.customerId = ord.customerId;
    newInv.customerName = ord.customerName;
    newInv.employeeId = employeeId;
    newInv.employeeName = "Nhân viên thu ngân";
    newInv.paymentMethod = method;
    newInv.paymentMethodText = paymentMethodToString(method);
    newInv.createTime = timeBuf;
    newInv.subtotal = ord.subtotal;
    newInv.discountAmount = ord.discountAmount;
    newInv.total = ord.total;
    newInv.details = ord.details;

    store.invoices.insert(store.invoices.begin(), newInv);
    return OperationResult::Ok("Đã xuất hóa đơn thành công! Mã hóa đơn: " + newInv.id);
}

OperationResult AppController::createPosInvoice(const std::string& customerId, const std::string& employeeId,
                                                const std::vector<CartItemViewModel>& items,
                                                const std::string& discountCode, PaymentMethod payMethod,
                                                double cashGiven, double& outChange, std::string& outInvoiceId) {
    outChange = 0.0;
    outInvoiceId.clear();
    if (items.empty()) {
        return OperationResult::Fail("Vui lòng chọn ít nhất 1 sản phẩm vào đơn bán tại quầy!");
    }

    auto& store = MockDataStore::getInstance();
    if (!IdGenerator::isValidId(employeeId, "EMP") ||
        getEmployeeById(employeeId).id.empty()) {
        return OperationResult::Fail("Nhân viên lập hóa đơn không hợp lệ hoặc không tồn tại!");
    }
    if (!customerId.empty() && customerId != "WALK-IN" &&
        (!IdGenerator::isValidId(customerId, "CUS") ||
         getCustomerById(customerId).id.empty())) {
        return OperationResult::Fail("Khách hàng được tham chiếu không hợp lệ hoặc không tồn tại!");
    }

    // Tính tổng tiền
    double subtotal = 0.0;
    for (const auto& it : items) {
        if (!IdGenerator::isValidId(it.phoneId, "PHN") || it.quantity <= 0) {
            return OperationResult::Fail("Chi tiết bán hàng chứa mã sản phẩm hoặc số lượng không hợp lệ!");
        }
        subtotal += it.subtotal;
        ProductItemViewModel p = getProductById(it.phoneId);
        if (p.availableStock < it.quantity) {
            return OperationResult::Fail("Sản phẩm '" + it.phoneName + "' không đủ tồn kho khả dụng!");
        }
    }

    // Mã giảm giá
    double discount = 0.0;
    if (!discountCode.empty()) {
        validateDiscount(discountCode, subtotal, discount);
    }

    double total = subtotal - discount;
    if (total < 0) total = 0;

    double change = 0.0;
    if (payMethod == PaymentMethod::CASH) {
        if (cashGiven < total) {
            return OperationResult::Fail("Số tiền khách đưa không đủ! Còn thiếu " +
                                         std::to_string(static_cast<long long>(total - cashGiven)) + " đ");
        }
        change = cashGiven - total;
    }

    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    std::string orderId;
    std::vector<std::string> invoiceIds = collectIds(store.invoices);
    std::vector<std::string> orderIds = collectIds(store.orders);
    if (idDate.empty() ||
        !IdGenerator::nextId("ORD", idDate, orderIds, orderId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã đơn hàng." : idError);
    }
    std::string invoiceId;
    if (!IdGenerator::nextId("INV", idDate, invoiceIds, invoiceId, idError)) {
        return OperationResult::Fail(idError);
    }
    outInvoiceId = invoiceId;
    outChange = change;

    // Trừ kho trực tiếp
    for (const auto& it : items) {
        for (auto& p : store.products) {
            if (p.id == it.phoneId) {
                p.actualStock -= it.quantity;
                p.calculateStockStatus();
                break;
            }
        }
    }

    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    std::string custName = "Khách vãng lai";
    if (!customerId.empty()) {
        CustomerItemViewModel c = getCustomerById(customerId);
        if (!c.fullName.empty()) custName = c.fullName;
    }

    InvoiceItemViewModel inv;
    inv.id = invoiceId;
    inv.orderId = orderId;
    inv.customerId = customerId == "WALK-IN" ? "" : customerId;
    inv.customerName = custName;
    inv.employeeId = employeeId;
    inv.employeeName = "Nhân viên Bán hàng";
    inv.paymentMethod = payMethod;
    inv.paymentMethodText = paymentMethodToString(payMethod);
    inv.createTime = timeBuf;
    inv.subtotal = subtotal;
    inv.discountAmount = discount;
    inv.total = total;

    for (const auto& it : items) {
        OrderDetailItemViewModel d;
        d.phoneId = it.phoneId;
        d.phoneName = it.phoneName;
        d.quantity = it.quantity;
        d.unitPrice = it.unitPrice;
        d.subtotal = it.subtotal;
        inv.details.push_back(d);
    }

    store.invoices.insert(store.invoices.begin(), inv);

    // Cũng tạo 1 bản ghi đơn hàng COMPLETED để đồng bộ thống kê
    OrderItemViewModel ord;
    ord.id = orderId;
    ord.customerId = inv.customerId;
    ord.customerName = inv.customerName;
    ord.orderTime = timeBuf;
    ord.status = OrderStatus::COMPLETED;
    ord.statusText = "Hoàn thành";
    ord.deliveryMethod = DeliveryMethod::PICKUP;
    ord.deliveryMethodText = "Nhận tại quầy";
    ord.paymentMethod = payMethod;
    ord.paymentMethodText = paymentMethodToString(payMethod);
    ord.paymentStatus = PaymentStatus::PAID;
    ord.paymentStatusText = "Đã thanh toán";
    ord.subtotal = subtotal;
    ord.discountAmount = discount;
    ord.total = total;
    ord.receiverName = custName;
    ord.receiverPhone = "-";
    ord.deliveryAddress = "Tại quầy thu ngân";
    ord.details = inv.details;
    store.orders.insert(store.orders.begin(), ord);

    // Tích điểm nếu có customer
    if (!customerId.empty() && customerId != "WALK-IN") {
        int points = static_cast<int>(total / 100000.0);
        for (auto& c : store.customers) {
            if (c.id == customerId) {
                c.points += points;
                c.updateTier();
                break;
            }
        }
    }

    return OperationResult::Ok("Giao dịch thanh toán tại quầy thành công! Mã HĐ: " + outInvoiceId);
}

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
    newC.fullName = req.lastName + " " + req.middleName + (req.middleName.empty() ? "" : " ") + req.firstName;
    newC.phone = req.phone;
    newC.email = req.email;
    newC.address = req.address;
    newC.username = req.username.empty() ? ("cust_" + req.phone) : req.username;
    newC.points = 0;
    newC.updateTier();

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

    auto& store = MockDataStore::getInstance();
    for (auto& c : store.customers) {
        if (c.id == req.id) {
            c.firstName = req.firstName;
            c.middleName = req.middleName;
            c.lastName = req.lastName;
            c.fullName = req.lastName + " " + req.middleName + (req.middleName.empty() ? "" : " ") + req.firstName;
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
    for (auto it = store.customers.begin(); it != store.customers.end(); ++it) {
        if (it->id == id) {
            std::string name = it->fullName;
            store.customers.erase(it);
            return OperationResult::Ok("Đã xóa khách hàng '" + name + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy khách hàng để xóa!");
}

// === NHÂN VIÊN ===

std::vector<EmployeeItemViewModel> AppController::getEmployees(const std::string& keyword) {
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
    newE.fullName = req.lastName + " " + req.middleName + (req.middleName.empty() ? "" : " ") + req.firstName;
    newE.phone = req.phone;
    newE.email = req.email;
    newE.address = req.address;
    newE.username = req.username;
    newE.role = req.role;
    newE.roleName = roleTypeToString(req.role);
    newE.salary = req.salary;
    newE.isActive = true;

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

    auto& store = MockDataStore::getInstance();
    for (auto& e : store.employees) {
        if (e.id == req.id) {
            e.firstName = req.firstName;
            e.middleName = req.middleName;
            e.lastName = req.lastName;
            e.fullName = req.lastName + " " + req.middleName + (req.middleName.empty() ? "" : " ") + req.firstName;
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
    for (auto it = store.employees.begin(); it != store.employees.end(); ++it) {
        if (it->id == id) {
            std::string name = it->fullName;
            store.employees.erase(it);
            return OperationResult::Ok("Đã xóa nhân viên '" + name + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy nhân viên!");
}

// === KHO & NHÀ CUNG CẤP ===

std::vector<ProductItemViewModel> AppController::getInventory(bool lowStockOnly) {
    auto& store = MockDataStore::getInstance();
    if (!lowStockOnly) return store.products;

    std::vector<ProductItemViewModel> lowStock;
    for (const auto& p : store.products) {
        if (p.status == StockDisplayStatus::LOW_STOCK || p.status == StockDisplayStatus::OUT_OF_STOCK) {
            lowStock.push_back(p);
        }
    }
    return lowStock;
}

std::vector<SupplierItemViewModel> AppController::getSuppliers(const std::string& keyword) {
    auto& store = MockDataStore::getInstance();
    if (keyword.empty()) return store.suppliers;

    std::vector<SupplierItemViewModel> result;
    for (const auto& s : store.suppliers) {
        if (Utf8Helper::containsCaseInsensitive(s.companyName, keyword) ||
            Utf8Helper::containsCaseInsensitive(s.id, keyword) ||
            Utf8Helper::containsCaseInsensitive(s.phone, keyword)) {
            result.push_back(s);
        }
    }
    return result;
}

OperationResult AppController::addSupplier(const SupplierFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (req.companyName.empty() || req.phone.empty()) {
        return OperationResult::Fail("Tên công ty và số điện thoại là bắt buộc!");
    }

    auto& store = MockDataStore::getInstance();
    std::string supplierId;
    std::string idError;
    if (!IdGenerator::nextId("SUP", "", collectIds(store.suppliers), supplierId, idError)) {
        return OperationResult::Fail(idError);
    }

    SupplierItemViewModel newS;
    newS.id = supplierId;
    newS.companyName = req.companyName;
    newS.taxCode = req.taxCode;
    newS.contactPerson = req.contactPerson;
    newS.phone = req.phone;
    newS.email = req.email;
    newS.address = req.address;

    store.suppliers.push_back(newS);
    return OperationResult::Ok("Thêm nhà cung cấp '" + newS.companyName + "' thành công!");
}

OperationResult AppController::updateSupplier(const SupplierFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(req.id, "SUP")) {
        return OperationResult::Fail("Mã nhà cung cấp không đúng định dạng SUP-000001!");
    }

    auto& store = MockDataStore::getInstance();
    for (auto& s : store.suppliers) {
        if (s.id == req.id) {
            s.companyName = req.companyName;
            s.taxCode = req.taxCode;
            s.contactPerson = req.contactPerson;
            s.phone = req.phone;
            s.email = req.email;
            s.address = req.address;
            return OperationResult::Ok("Cập nhật thông tin nhà cung cấp thành công!");
        }
    }
    return OperationResult::Fail("Không tìm thấy nhà cung cấp!");
}

OperationResult AppController::deleteSupplier(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    if (!IdGenerator::isValidId(id, "SUP")) {
        return OperationResult::Fail("Mã nhà cung cấp không đúng định dạng SUP-000001!");
    }
    auto& store = MockDataStore::getInstance();
    for (const auto& receipt : store.receipts) {
        if (receipt.supplierId == id) {
            return OperationResult::Fail("Không thể xóa nhà cung cấp đang được tham chiếu trong phiếu nhập (" + receipt.id + ")!");
        }
    }
    for (auto it = store.suppliers.begin(); it != store.suppliers.end(); ++it) {
        if (it->id == id) {
            std::string name = it->companyName;
            store.suppliers.erase(it);
            return OperationResult::Ok("Đã xóa nhà cung cấp '" + name + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy nhà cung cấp để xóa!");
}

std::vector<GoodsReceiptSummaryViewModel> AppController::getGoodsReceipts() {
    return MockDataStore::getInstance().receipts;
}

OperationResult AppController::createGoodsReceipt(const CreateReceiptRequest& req) {
    REQUIRE_ADMIN_OR_WAREHOUSE_ROLE();
    if (!IdGenerator::isValidId(req.supplierId, "SUP")) {
        return OperationResult::Fail("Vui lòng chọn nhà cung cấp!");
    }
    if (req.items.empty()) {
        return OperationResult::Fail("Vui lòng chọn ít nhất 1 sản phẩm để nhập kho!");
    }

    auto& store = MockDataStore::getInstance();
    bool supplierExists = false;
    for (const auto& supplier : store.suppliers) {
        if (supplier.id == req.supplierId) {
            supplierExists = true;
            break;
        }
    }
    if (!supplierExists) {
        return OperationResult::Fail("Không tìm thấy nhà cung cấp được tham chiếu!");
    }
    const std::string employeeId =
        req.employeeId.empty() ? currentSession.userId : req.employeeId;
    bool employeeExists = false;
    for (const auto& employee : store.employees) {
        if (employee.id == employeeId) {
            employeeExists = true;
            break;
        }
    }
    if (!IdGenerator::isValidId(employeeId, "EMP") || !employeeExists) {
        return OperationResult::Fail("Nhân viên lập phiếu nhập không hợp lệ hoặc không tồn tại!");
    }
    for (const auto& item : req.items) {
        if (!IdGenerator::isValidId(item.phoneId, "PHN") || item.quantity <= 0 ||
            item.importPrice < 0 || getProductById(item.phoneId).id.empty()) {
            return OperationResult::Fail("Chi tiết phiếu nhập chứa sản phẩm hoặc số lượng không hợp lệ!");
        }
    }
    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", tm_now);

    std::string receiptId;
    std::string idError;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("IMP", idDate, collectIds(store.receipts), receiptId, idError)) {
        return OperationResult::Fail(idError.empty() ? "Không thể xác định ngày để sinh mã phiếu nhập." : idError);
    }

    GoodsReceiptSummaryViewModel r;
    r.id = receiptId;
    r.supplierId = req.supplierId;
    r.employeeId = employeeId;
    r.employeeName = currentSession.fullName.empty() ? "Thủ kho" : currentSession.fullName;
    r.importTime = timeBuf;
    r.totalAmount = 0.0;

    for (const auto& sup : store.suppliers) {
        if (sup.id == req.supplierId) {
            r.supplierName = sup.companyName;
            break;
        }
    }

    for (const auto& item : req.items) {
        GoodsReceiptDetailViewModel d = item;
        d.subtotal = item.importPrice * item.quantity;
        r.totalAmount += d.subtotal;
        r.details.push_back(d);

        // Tăng tồn kho thực tế và cập nhật giá nhập mới
        for (auto& p : store.products) {
            if (p.id == item.phoneId) {
                p.actualStock += item.quantity;
                p.importPrice = item.importPrice;
                p.calculateStockStatus();
                break;
            }
        }
    }

    store.receipts.insert(store.receipts.begin(), r);
    return OperationResult::Ok("Tạo phiếu nhập kho thành công! Mã phiếu: " + r.id);
}

// === MÃ GIẢM GIÁ ===

std::vector<DiscountItemViewModel> AppController::getDiscounts() {
    auto& store = MockDataStore::getInstance();
    for (auto& d : store.discounts) {
        d.isValid = !RealTimeManager::getInstance().isDateExpired(d.expiryDate);
    }
    return store.discounts;
}

OperationResult AppController::updateDiscount(const DiscountFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (req.code.empty() || req.value <= 0) {
        return OperationResult::Fail("Mã voucher và giá trị giảm phải lớn hơn 0!");
    }
    auto& store = MockDataStore::getInstance();
    bool ok = store.updateDiscount(req);
    if (!ok) {
        return OperationResult::Fail("Không tìm thấy mã giảm giá cần cập nhật!");
    }
    return OperationResult::Ok("Cập nhật thời hạn hiệu lực và thông tin mã giảm giá thành công!");
}

OperationResult AppController::addDiscount(const DiscountFormRequest& req) {
    REQUIRE_ADMIN_ROLE();
    if (req.code.empty() || req.value <= 0) {
        return OperationResult::Fail("Mã voucher và giá trị giảm phải lớn hơn 0!");
    }

    auto& store = MockDataStore::getInstance();
    for (const auto& d : store.discounts) {
        if (d.code == req.code) {
            return OperationResult::Fail("Mã giảm giá '" + req.code + "' đã tồn tại!");
        }
    }

    DiscountItemViewModel d;
    d.id = req.code;
    d.code = req.code;
    d.typeName = (req.type == 0) ? "Giảm phần trăm (%)" : "Số tiền cố định";
    d.value = req.value;
    d.formattedValue = (req.type == 0) ? (std::to_string(static_cast<int>(req.value)) + "%")
                                        : (std::to_string(static_cast<long long>(req.value)) + " đ");
    d.minOrderValue = req.minOrderValue;
    d.maxDiscountAmount = req.maxDiscountAmount;
    d.expiryDate = req.expiryDate.empty() ? "31/12/2026" : req.expiryDate;
    d.usedCount = 0;
    d.maxUsage = req.maxUsage;
    d.isValid = true;
    d.description = req.description;

    store.discounts.push_back(d);
    return OperationResult::Ok("Thêm mã giảm giá '" + req.code + "' thành công!");
}

OperationResult AppController::deleteDiscount(const std::string& id) {
    REQUIRE_ADMIN_ROLE();
    auto& store = MockDataStore::getInstance();
    for (auto it = store.discounts.begin(); it != store.discounts.end(); ++it) {
        if (it->id == id) {
            if (it->usedCount > 0) {
                return OperationResult::Fail("Không thể xóa mã giảm giá đã có lịch sử sử dụng!");
            }
            std::string code = it->code;
            store.discounts.erase(it);
            return OperationResult::Ok("Đã xóa mã giảm giá '" + code + "'!");
        }
    }
    return OperationResult::Fail("Không tìm thấy mã giảm giá!");
}

OperationResult AppController::validateDiscount(const std::string& code, double orderTotal, double& outDiscount) {
    outDiscount = 0.0;
    auto& store = MockDataStore::getInstance();

    for (const auto& d : store.discounts) {
        if (d.code == code) {
            if (!d.isValid) {
                return OperationResult::Fail("Mã giảm giá '" + code + "' đã hết hạn sử dụng!");
            }
            if (d.usedCount >= d.maxUsage) {
                return OperationResult::Fail("Mã giảm giá '" + code + "' đã hết lượt sử dụng!");
            }
            if (orderTotal < d.minOrderValue) {
                return OperationResult::Fail("Đơn hàng chưa đạt giá trị tối thiểu " +
                                             std::to_string(static_cast<long long>(d.minOrderValue)) + " đ để áp dụng mã này!");
            }

            if (d.typeName.find("%") != std::string::npos) {
                outDiscount = (orderTotal * d.value) / 100.0;
                if (d.maxDiscountAmount > 0 && outDiscount > d.maxDiscountAmount) {
                    outDiscount = d.maxDiscountAmount;
                }
            } else {
                outDiscount = d.value;
                if (outDiscount > orderTotal) outDiscount = orderTotal;
            }
            return OperationResult::Ok("Mã giảm giá hợp lệ!");
        }
    }

    return OperationResult::Fail("Mã giảm giá '" + code + "' không tồn tại!");
}

// === TÀI KHOẢN & PHÂN QUYỀN ===

std::vector<AccountItemViewModel> AppController::getAccounts() {
    return MockDataStore::getInstance().accounts;
}

OperationResult AppController::toggleAccountStatus(const std::string& username) {
    REQUIRE_ADMIN_ROLE();
    if (username == currentSession.username) {
        return OperationResult::Fail("Không thể khóa tài khoản của chính bạn!");
    }

    auto& store = MockDataStore::getInstance();
    for (auto& a : store.accounts) {
        if (a.username == username) {
            a.isActive = !a.isActive;
            std::string msg = a.isActive ? ("Đã mở khóa tài khoản '" + username + "'")
                                         : ("Đã khóa tài khoản '" + username + "'");
            return OperationResult::Ok(msg);
        }
    }
    return OperationResult::Fail("Không tìm thấy tài khoản!");
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

// === THỐNG KÊ & BÁO CÁO ===

OperationResult AppController::validateReportRange(const std::string& fromDate, const std::string& toDate) {
    int d = 0, m = 0, y = 0;
    if (!fromDate.empty() && !parseDateDMY(fromDate, d, m, y)) {
        return OperationResult::Fail("'Từ ngày' không hợp lệ! Vui lòng nhập đúng định dạng DD/MM/YYYY (ví dụ 01/10/2026).");
    }
    if (!toDate.empty() && !parseDateDMY(toDate, d, m, y)) {
        return OperationResult::Fail("'Đến ngày' không hợp lệ! Vui lòng nhập đúng định dạng DD/MM/YYYY (ví dụ 31/10/2026).");
    }
    if (!fromDate.empty() && !toDate.empty() &&
        RealTimeManager::dateToEpochDays(fromDate) > RealTimeManager::dateToEpochDays(toDate)) {
        return OperationResult::Fail("'Từ ngày' phải nhỏ hơn hoặc bằng 'Đến ngày'!");
    }
    return OperationResult::Ok("Khoảng thời gian hợp lệ.");
}

DashboardStatsViewModel AppController::getDashboardStats(const std::string& fromDate, const std::string& toDate) {
    // Bộ lọc khoảng ngày (bao gồm cả hai đầu mút). Để trống cả hai = toàn thời gian.
    const bool hasFrom = !fromDate.empty();
    const bool hasTo = !toDate.empty();
    const long long fromDays = hasFrom ? RealTimeManager::dateToEpochDays(fromDate) : 0;
    const long long toDays = hasTo ? RealTimeManager::dateToEpochDays(toDate) : 0;
    auto inRange = [&](const std::string& orderTime) {
        if (!hasFrom && !hasTo) return true;
        long long days = RealTimeManager::dateToEpochDays(orderTime);
        if (days == 0) return false;
        if (hasFrom && days < fromDays) return false;
        if (hasTo && days > toDays) return false;
        return true;
    };
    auto& store = MockDataStore::getInstance();
    DashboardStatsViewModel stats;

    for (const auto& ord : store.orders) {
        if (inRange(ord.orderTime)) stats.totalOrders++;
    }
    stats.totalProducts = static_cast<int>(store.products.size());
    stats.totalCustomers = static_cast<int>(store.customers.size());
    stats.totalEmployees = static_cast<int>(store.employees.size());

    // Doanh thu thực tế: CHỈ tính các đơn COMPLETED / PAID theo spec mục 7
    double totalCostCompleted = 0.0;
    for (const auto& ord : store.orders) {
        if (!inRange(ord.orderTime)) continue;
        if (ord.status == OrderStatus::COMPLETED && ord.paymentStatus == PaymentStatus::PAID) {
            stats.completedOrders++;
            stats.totalRevenue += ord.total;

            for (const auto& d : ord.details) {
                ProductItemViewModel p = getProductById(d.phoneId);
                totalCostCompleted += (p.importPrice * d.quantity);
            }
        } else if (ord.status == OrderStatus::PENDING) {
            stats.pendingOrders++;
        } else if (ord.status == OrderStatus::SHIPPING || ord.status == OrderStatus::PREPARING || ord.status == OrderStatus::READY_FOR_PICKUP) {
            stats.shippingOrders++;
        } else if (ord.status == OrderStatus::CANCELLED) {
            stats.cancelledOrders++;
        }
    }

    stats.estimatedProfit = stats.totalRevenue - totalCostCompleted;

    // Tồn kho
    for (const auto& p : store.products) {
        stats.totalStockQuantity += p.actualStock;
        stats.totalInventoryValue += (p.importPrice * p.actualStock);
        if (p.status == StockDisplayStatus::LOW_STOCK || p.status == StockDisplayStatus::OUT_OF_STOCK) {
            stats.lowStockCount++;
        }
    }

    // Top sản phẩm bán chạy (dựa vào chi tiết các đơn COMPLETED)
    for (const auto& p : store.products) {
        int sold = 0;
        double rev = 0.0;
        for (const auto& ord : store.orders) {
            if (ord.status == OrderStatus::COMPLETED && inRange(ord.orderTime)) {
                for (const auto& d : ord.details) {
                    if (d.phoneId == p.id) {
                        sold += d.quantity;
                        rev += d.subtotal;
                    }
                }
            }
        }
        if (sold > 0) {
            TopProductStatItem t;
            t.phoneId = p.id;
            t.phoneName = p.name;
            t.brand = p.brand;
            t.soldQuantity = sold;
            t.revenue = rev;
            t.profit = rev - (p.importPrice * sold);
            stats.topProducts.push_back(t);
        }
    }

    std::sort(stats.topProducts.begin(), stats.topProducts.end(), [](const TopProductStatItem& a, const TopProductStatItem& b) {
        return a.soldQuantity > b.soldQuantity;
    });

    return stats;
}
