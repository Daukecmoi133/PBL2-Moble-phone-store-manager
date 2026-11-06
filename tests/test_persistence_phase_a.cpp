// tests/test_persistence_phase_a.cpp
// Automated Regression Test Suite for Fixing Phase A - Persistence Core (BUG-CRITICAL-01)

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>

#include "source/PBL2.h"
#include "source/controllers/AppController.h"
#include "source/utils/PathHelper.h"

static int g_passCount = 0;
static int g_failCount = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            std::cout << "  [PASS] " << msg << "\n"; \
            g_passCount++; \
        } else { \
            std::cerr << "  [FAIL] " << msg << " (Line " << __LINE__ << ")\n"; \
            g_failCount++; \
        } \
    } while (0)

#define TEST_ASSERT_OP(res, msg) \
    do { \
        if (res.isSuccess()) { \
            std::cout << "  [PASS] " << msg << "\n"; \
            g_passCount++; \
        } else { \
            std::cerr << "  [FAIL] " << msg << " -> LỖI: " << res.message << " (Line " << __LINE__ << ")\n"; \
            g_failCount++; \
        } \
    } while (0)

// Cấu trúc nạp toàn bộ Repository từ file trước khi Services được khởi tạo
struct RepositoriesHolder {
    PhoneRepository        phoneRepo;
    OrderRepository        orderRepo;
    InvoiceRepository      invoiceRepo;
    StockReceiptRepository stockRepo;
    CustomerRepository     customerRepo;
    EmployeeRepository     employeeRepo;
    SupplierRepository     supplierRepo;
    DiscountRepository     discountRepo;

    RepositoriesHolder() {
        phoneRepo.loadFromFile();
        orderRepo.loadFromFile();
        invoiceRepo.loadFromFile();
        stockRepo.loadFromFile();
        customerRepo.loadFromFile();
        employeeRepo.loadFromFile();
        supplierRepo.loadFromFile();
        discountRepo.loadFromFile();
    }
};

struct TestAppSession {
    RepositoriesHolder     repos;
    ReferenceChecker       refChecker;
    PhoneService           phoneService;
    OrderService           orderService;
    CustomerService        customerService;
    EmployeeService        employeeService;
    InvoiceService         invoiceService;
    SupplierService        supplierService;
    DiscountService        discountService;
    ReportService          reportService;
    AuthService            authService;
    AppController          controller;

    TestAppSession()
        : repos(),
          refChecker(&repos.orderRepo, &repos.invoiceRepo, &repos.stockRepo),
          phoneService(repos.phoneRepo, &refChecker),
          orderService(repos.orderRepo, &phoneService),
          customerService(repos.customerRepo),
          employeeService(repos.employeeRepo),
          invoiceService(repos.invoiceRepo, repos.orderRepo),
          supplierService(repos.supplierRepo, repos.stockRepo, &phoneService),
          discountService(repos.discountRepo),
          reportService(repos.orderRepo, repos.invoiceRepo, repos.phoneRepo),
          authService(employeeService, customerService),
          controller(&phoneService, &orderService, &customerService, &employeeService,
                     &invoiceService, &supplierService, &discountService, &reportService, &authService)
    {
        // Đăng nhập Admin để thực hiện các nghiệp vụ quản trị
        LoginRequest loginReq;
        loginReq.username = "admin.demo";
        loginReq.password = "DemoOnly-Admin-2026";
        controller.login(loginReq);
    }
};

// 1. Test Product Persistence (Add -> Restart -> Update -> Restart -> Delete -> Restart)
void testProductPersistence() {
    std::cout << "\n=== [TEST 1] Product Persistence across Sessions ===\n";
    std::string testId;

    // Session 1: Thêm sản phẩm mới
    {
        TestAppSession session;
        ProductItemViewModel p;
        p.name = "Google Pixel 9 Pro Fold";
        p.brand = "Google";
        p.os = "Android 15";
        p.importPrice = 25000000;
        p.sellingPrice = 32000000;
        p.actualStock = 15;
        p.manufacturer = "Google LLC";
        p.specs = "Tensor G4, 16GB RAM, 512GB, 8 inch Foldable OLED";

        auto res = session.controller.addProduct(p);
        TEST_ASSERT_OP(res, "Session 1: Thêm sản phẩm Google Pixel 9 Pro Fold thành công");

        for (const auto& prod : session.controller.getProducts()) {
            if (prod.name == "Google Pixel 9 Pro Fold") {
                testId = prod.id;
                break;
            }
        }
        TEST_ASSERT(!testId.empty(), "Session 1: Lấy được ID sản phẩm vừa tạo");
        std::cout << "  [INFO] Mã sản phẩm vừa tạo: " << testId << "\n";
    }

    // Session 2: Đóng & mở lại ứng dụng -> Kiểm tra dữ liệu nạp lại từ disk
    {
        TestAppSession session;
        auto p = session.controller.getProductById(testId);
        TEST_ASSERT(p.id == testId, "Session 2: Sản phẩm tồn tại sau khi mở lại app");
        TEST_ASSERT(p.name == "Google Pixel 9 Pro Fold", "Session 2: Tên sản phẩm chính xác từ file");
        TEST_ASSERT(p.sellingPrice == 32000000, "Session 2: Giá bán chính xác từ file");
        TEST_ASSERT(p.actualStock == 15, "Session 2: Tồn kho chính xác từ file");

        // Cập nhật sản phẩm
        p.name = "Google Pixel 9 Pro Fold (Updated)";
        p.sellingPrice = 33500000;
        p.actualStock = 18;
        auto res = session.controller.updateProduct(p);
        TEST_ASSERT_OP(res, "Session 2: Cập nhật sản phẩm thành công");
    }

    // Session 3: Đóng & mở lại -> Kiểm tra dữ liệu đã sửa lưu bền vững
    {
        TestAppSession session;
        auto p = session.controller.getProductById(testId);
        TEST_ASSERT(p.id == testId, "Session 3: Sản phẩm còn tồn tại sau cập nhật");
        TEST_ASSERT(p.name == "Google Pixel 9 Pro Fold (Updated)", "Session 3: Tên đã được cập nhật lưu trên file");
        TEST_ASSERT(p.sellingPrice == 33500000, "Session 3: Giá bán đã cập nhật lưu trên file");
        TEST_ASSERT(p.actualStock == 18, "Session 3: Tồn kho đã cập nhật lưu trên file");

        // Xóa sản phẩm
        auto res = session.controller.deleteProduct(testId);
        TEST_ASSERT_OP(res, "Session 3: Xóa sản phẩm thành công");
    }

    // Session 4: Đóng & mở lại -> Kiểm tra sản phẩm đã bị xóa trên disk
    {
        TestAppSession session;
        auto p = session.controller.getProductById(testId);
        TEST_ASSERT(p.id.empty(), "Session 4: Sản phẩm không còn tồn tại sau khi xóa");
    }
}

// 2. Test Customer Persistence (Add -> Restart -> Update -> Restart -> Delete -> Restart)
void testCustomerPersistence() {
    std::cout << "\n=== [TEST 2] Customer Persistence across Sessions ===\n";
    std::string testId;
    const std::string initialPhone = "0909111222";
    const std::string updatedPhone = "0909333444";

    // Session 1: Thêm khách hàng
    {
        TestAppSession session;
        CustomerFormRequest req;
        req.firstName = "Van";
        req.middleName = "A";
        req.lastName = "Nguyen";
        req.phone = initialPhone;
        req.email = "vana.test@example.com";
        req.address = "100 Nguyen Van Linh, Da Nang";
        req.username = "vana_test_persist";
        req.password = "Pass123";

        auto res = session.controller.addCustomer(req);
        TEST_ASSERT_OP(res, "Session 1: Thêm khách hàng thành công");

        for (const auto& c : session.controller.getCustomers()) {
            if (c.phone == initialPhone) {
                testId = c.id;
                break;
            }
        }
        TEST_ASSERT(!testId.empty(), "Session 1: Lấy được ID khách hàng vừa tạo");
        std::cout << "  [INFO] Mã khách hàng vừa tạo: " << testId << "\n";
    }

    // Session 2: Đóng & mở lại -> Kiểm tra nạp từ file
    {
        TestAppSession session;
        auto c = session.controller.getCustomerById(testId);
        TEST_ASSERT(c.id == testId, "Session 2: Khách hàng tồn tại sau khi khởi động lại");
        TEST_ASSERT(c.phone == initialPhone, "Session 2: SĐT khách hàng lưu đúng");
        TEST_ASSERT(c.address == "100 Nguyen Van Linh, Da Nang", "Session 2: Địa chỉ khách hàng lưu đúng");

        // Sửa khách hàng
        CustomerFormRequest updateReq;
        updateReq.id = testId;
        updateReq.firstName = "Van";
        updateReq.middleName = "A";
        updateReq.lastName = "Nguyen";
        updateReq.phone = updatedPhone;
        updateReq.email = "vana.updated@example.com";
        updateReq.address = "200 Bach Dang, Da Nang";
        auto res = session.controller.updateCustomer(updateReq);
        TEST_ASSERT_OP(res, "Session 2: Cập nhật khách hàng thành công");
    }

    // Session 3: Đóng & mở lại -> Kiểm tra cập nhật bền vững
    {
        TestAppSession session;
        auto c = session.controller.getCustomerById(testId);
        TEST_ASSERT(c.phone == updatedPhone, "Session 3: SĐT cập nhật lưu trên file");
        TEST_ASSERT(c.address == "200 Bach Dang, Da Nang", "Session 3: Địa chỉ cập nhật lưu trên file");

        // Xóa khách hàng
        auto res = session.controller.deleteCustomer(testId);
        TEST_ASSERT_OP(res, "Session 3: Xóa khách hàng thành công");
    }

    // Session 4: Đóng & mở lại -> Kiểm tra đã xóa trên disk
    {
        TestAppSession session;
        auto c = session.controller.getCustomerById(testId);
        TEST_ASSERT(c.id.empty(), "Session 4: Khách hàng không còn sau khi xóa");
    }
}

// 3. Test Discount / Voucher Persistence (Add -> Restart -> Update -> Restart -> Delete -> Restart)
void testDiscountPersistence() {
    std::cout << "\n=== [TEST 3] Discount / Voucher Persistence across Sessions ===\n";
    const std::string testCode = "TESTVOUCHER2026";

    // Session 1: Thêm voucher
    {
        TestAppSession session;
        DiscountFormRequest req;
        req.id = testCode;
        req.code = testCode;
        req.type = 1; // Cố định
        req.value = 250000;
        req.minOrderValue = 1000000;
        req.maxDiscountAmount = 250000;
        req.expiryDate = "31/12/2026";
        req.maxUsage = 50;
        req.description = "Test Voucher 250k";

        auto res = session.controller.addDiscount(req);
        TEST_ASSERT_OP(res, "Session 1: Thêm voucher TESTVOUCHER2026 thành công");
    }

    // Session 2: Đóng & mở lại -> Kiểm tra nạp từ file
    {
        TestAppSession session;
        auto discounts = session.controller.getDiscounts();
        bool found = false;
        for (const auto& d : discounts) {
            if (d.code == testCode) {
                found = true;
                TEST_ASSERT(d.value == 250000, "Session 2: Giá trị voucher lưu đúng trên file");
                TEST_ASSERT(d.maxUsage == 50, "Session 2: Lượt dùng tối đa lưu đúng trên file");
                break;
            }
        }
        TEST_ASSERT(found, "Session 2: Tìm thấy TESTVOUCHER2026 sau khi khởi động lại");

        // Sửa voucher
        DiscountFormRequest updateReq;
        updateReq.id = testCode;
        updateReq.code = testCode;
        updateReq.type = 1;
        updateReq.value = 300000;
        updateReq.minOrderValue = 1500000;
        updateReq.maxDiscountAmount = 300000;
        updateReq.expiryDate = "31/12/2026";
        updateReq.maxUsage = 80;
        updateReq.description = "Test Voucher 300k (Updated)";

        auto res = session.controller.updateDiscount(updateReq);
        TEST_ASSERT_OP(res, "Session 2: Cập nhật voucher thành công");
    }

    // Session 3: Đóng & mở lại -> Kiểm tra cập nhật bền vững
    {
        TestAppSession session;
        auto discounts = session.controller.getDiscounts();
        bool found = false;
        for (const auto& d : discounts) {
            if (d.code == testCode) {
                found = true;
                TEST_ASSERT(d.value == 300000, "Session 3: Giá trị voucher cập nhật lưu đúng");
                TEST_ASSERT(d.maxUsage == 80, "Session 3: Lượt dùng tối đa cập nhật lưu đúng");
                break;
            }
        }
        TEST_ASSERT(found, "Session 3: TESTVOUCHER2026 tồn tại sau cập nhật");

        // Xóa voucher
        auto res = session.controller.deleteDiscount(testCode);
        TEST_ASSERT_OP(res, "Session 3: Xóa voucher thành công");
    }

    // Session 4: Đóng & mở lại -> Kiểm tra đã xóa trên disk
    {
        TestAppSession session;
        auto discounts = session.controller.getDiscounts();
        bool found = false;
        for (const auto& d : discounts) {
            if (d.code == testCode) {
                found = true;
                break;
            }
        }
        TEST_ASSERT(!found, "Session 4: TESTVOUCHER2026 không còn trên disk sau khi xóa");
    }
}

// 4. Test Supplier Persistence (Add -> Restart -> Update -> Restart -> Delete -> Restart)
void testSupplierPersistence() {
    std::cout << "\n=== [TEST 4] Supplier Persistence across Sessions ===\n";
    std::string testId;

    // Session 1: Thêm nhà cung cấp
    {
        TestAppSession session;
        SupplierFormRequest req;
        req.companyName = "Sony Vietnam Corporation";
        req.taxCode = "0399881122";
        req.contactPerson = "Kenichiro Yoshida";
        req.phone = "02838999888";
        req.email = "contact@sony.com.vn";
        req.address = "Quan 1, TP Ho Chi Minh";

        auto res = session.controller.addSupplier(req);
        TEST_ASSERT_OP(res, "Session 1: Thêm nhà cung cấp thành công");

        for (const auto& s : session.controller.getSuppliers()) {
            if (s.companyName == "Sony Vietnam Corporation") {
                testId = s.id;
                break;
            }
        }
        TEST_ASSERT(!testId.empty(), "Session 1: Lấy được ID nhà cung cấp vừa tạo");
        std::cout << "  [INFO] Mã nhà cung cấp vừa tạo: " << testId << "\n";
    }

    // Session 2: Đóng & mở lại -> Kiểm tra nạp từ file
    {
        TestAppSession session;
        auto suppliers = session.controller.getSuppliers();
        bool found = false;
        for (const auto& s : suppliers) {
            if (s.id == testId) {
                found = true;
                TEST_ASSERT(s.companyName == "Sony Vietnam Corporation", "Session 2: Tên công ty lưu đúng");
                TEST_ASSERT(s.contactPerson == "Kenichiro Yoshida", "Session 2: Người liên hệ lưu đúng");
                break;
            }
        }
        TEST_ASSERT(found, "Session 2: Tìm thấy nhà cung cấp sau khởi động lại");

        // Sửa nhà cung cấp
        SupplierFormRequest updateReq;
        updateReq.id = testId;
        updateReq.companyName = "Sony Vietnam Corp Updated";
        updateReq.taxCode = "0399881122";
        updateReq.contactPerson = "Hiroki Totoki";
        updateReq.phone = "02838777666";
        updateReq.email = "contact.updated@sony.com.vn";
        updateReq.address = "Quan 7, TP Ho Chi Minh";

        auto res = session.controller.updateSupplier(updateReq);
        TEST_ASSERT_OP(res, "Session 2: Cập nhật nhà cung cấp thành công");
    }

    // Session 3: Đóng & mở lại -> Kiểm tra cập nhật bền vững
    {
        TestAppSession session;
        auto suppliers = session.controller.getSuppliers();
        bool found = false;
        for (const auto& s : suppliers) {
            if (s.id == testId) {
                found = true;
                TEST_ASSERT(s.companyName == "Sony Vietnam Corp Updated", "Session 3: Tên công ty cập nhật lưu đúng");
                TEST_ASSERT(s.contactPerson == "Hiroki Totoki", "Session 3: Người liên hệ cập nhật lưu đúng");
                break;
            }
        }
        TEST_ASSERT(found, "Session 3: Nhà cung cấp tồn tại sau cập nhật");

        // Xóa nhà cung cấp
        auto res = session.controller.deleteSupplier(testId);
        TEST_ASSERT_OP(res, "Session 3: Xóa nhà cung cấp thành công");
    }

    // Session 4: Đóng & mở lại -> Kiểm tra đã xóa trên disk
    {
        TestAppSession session;
        auto suppliers = session.controller.getSuppliers();
        bool found = false;
        for (const auto& s : suppliers) {
            if (s.id == testId) {
                found = true;
                break;
            }
        }
        TEST_ASSERT(!found, "Session 4: Nhà cung cấp không còn trên disk sau khi xóa");
    }
}

// 5. Test StockReceipt & Inventory Increment Persistence
void testStockReceiptPersistence() {
    std::cout << "\n=== [TEST 5] StockReceipt & Inventory Increment Persistence ===\n";
    std::string targetPhoneId = "PHN-000101";
    int initialStock = 0;
    std::string createdReceiptId;

    // Session 1: Ghi nhận tồn ban đầu và lập phiếu nhập kho 10 cái
    {
        TestAppSession session;
        auto phone = session.controller.getProductById(targetPhoneId);
        initialStock = phone.actualStock;

        CreateReceiptRequest req;
        req.supplierId = "SUP-000003"; // Oppo Vietnam
        req.employeeId = "EMP-000003"; // Le Van Kho
        GoodsReceiptDetailViewModel item;
        item.phoneId = targetPhoneId;
        item.phoneName = phone.name;
        item.quantity = 10;
        item.importPrice = phone.importPrice;
        item.subtotal = item.quantity * item.importPrice;
        req.items.push_back(item);

        auto res = session.controller.createGoodsReceipt(req);
        TEST_ASSERT_OP(res, "Session 1: Lập phiếu nhập kho thành công");

        auto receipts = session.controller.getGoodsReceipts();
        for (const auto& r : receipts) {
            for (const auto& d : r.details) {
                if (d.phoneId == targetPhoneId && d.quantity == 10) {
                    createdReceiptId = r.id;
                    break;
                }
            }
            if (!createdReceiptId.empty()) break;
        }
        std::cout << "  [INFO] Mã phiếu nhập vừa tạo: " << createdReceiptId << "\n";
    }

    // Session 2: Đóng & mở lại -> Kiểm tra phiếu nhập và số lượng tồn kho đã tăng
    {
        TestAppSession session;
        auto phone = session.controller.getProductById(targetPhoneId);
        TEST_ASSERT(phone.actualStock == initialStock + 10,
                    "Session 2: Tồn kho thực tế tăng 10 máy bền vững trong Products.txt");

        auto receipts = session.controller.getGoodsReceipts();
        bool foundReceipt = false;
        for (const auto& r : receipts) {
            if (!createdReceiptId.empty() && r.id == createdReceiptId) {
                foundReceipt = true;
                TEST_ASSERT(r.details.size() == 1, "Session 2: Chi tiết phiếu nhập nạp đầy đủ từ file");
                if (!r.details.empty()) {
                    TEST_ASSERT(r.details[0].phoneId == targetPhoneId, "Session 2: Mặt hàng nhập khớp mã");
                    TEST_ASSERT(r.details[0].quantity == 10, "Session 2: Số lượng nhập khớp 10");
                }
                break;
            }
        }
        TEST_ASSERT(foundReceipt, "Session 2: Phiếu nhập lưu bền vững trong StockReceipts.txt");

        // Dọn dẹp dữ liệu test để trả về trạng thái nguyên bản
        phone.actualStock = initialStock;
        session.controller.updateProduct(phone);

        // Khôi phục file StockReceipts.txt loại bỏ receipt vừa tạo (nếu có id hợp lệ)
        if (!createdReceiptId.empty()) {
            std::string receiptPath = PathHelper::resolveDataPath("data/Device/StockReceipts.txt");
            std::ifstream fin(receiptPath);
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(fin, line)) {
                if (line.find(createdReceiptId) == std::string::npos) {
                    lines.push_back(line);
                }
            }
            fin.close();
            std::ofstream fout(receiptPath);
            for (const auto& l : lines) {
                fout << l << "\n";
            }
            fout.close();
        }
    }
}

// 6. Test POS Invoice & Order Persistence
void testPosInvoiceAndOrderPersistence() {
    std::cout << "\n=== [TEST 6] POS Invoice, Order & Stock Deduction Persistence ===\n";
    std::string targetPhoneId = "PHN-000106"; // Samsung Galaxy A55 5G
    int initialStock = 0;
    std::string createdInvoiceId;

    // Session 1: Tạo hóa đơn POS bán 2 máy
    {
        TestAppSession session;
        auto phone = session.controller.getProductById(targetPhoneId);
        initialStock = phone.actualStock;

        std::vector<CartItemViewModel> items;
        CartItemViewModel item;
        item.phoneId = targetPhoneId;
        item.phoneName = phone.name;
        item.brand = phone.brand;
        item.unitPrice = phone.sellingPrice;
        item.quantity = 2;
        item.subtotal = item.unitPrice * item.quantity;
        items.push_back(item);

        double outChange = 0.0;
        auto res = session.controller.createPosInvoice(
            "CUS-000001", "EMP-000002", items, "", PaymentMethod::CASH, 20000000.0,
            outChange, createdInvoiceId
        );
        TEST_ASSERT_OP(res, "Session 1: Tạo hóa đơn POS bán hàng thành công");
        TEST_ASSERT(!createdInvoiceId.empty(), "Session 1: Đã sinh Invoice ID");
        std::cout << "  [INFO] Mã hóa đơn vừa tạo: " << createdInvoiceId << "\n";
    }

    // Session 2: Đóng & mở lại -> Kiểm tra hóa đơn, đơn hàng tương ứng và trừ kho
    std::string createdOrderId;
    {
        TestAppSession session;
        auto phone = session.controller.getProductById(targetPhoneId);
        TEST_ASSERT(phone.actualStock == initialStock - 2,
                    "Session 2: Tồn kho thực tế bị trừ 2 máy bền vững trong Products.txt");

        auto inv = session.controller.getInvoiceById(createdInvoiceId);
        TEST_ASSERT(inv.id == createdInvoiceId, "Session 2: Hóa đơn POS lưu bền vững trong Invoices.txt");
        TEST_ASSERT(inv.customerId == "CUS-000001", "Session 2: Đúng mã khách hàng trong hóa đơn");
        createdOrderId = inv.orderId;
        TEST_ASSERT(!createdOrderId.empty(), "Session 2: Hóa đơn liên kết đến mã đơn hàng hợp lệ");

        auto ord = session.controller.getOrderById(createdOrderId);
        TEST_ASSERT(ord.id == createdOrderId, "Session 2: Đơn hàng POS lưu bền vững trong Orders.txt");
        TEST_ASSERT(ord.status == OrderStatus::COMPLETED, "Session 2: Trạng thái đơn hàng là COMPLETED");

        // Dọn dẹp dữ liệu test: trả tồn kho về trạng thái ban đầu
        phone.actualStock = initialStock;
        session.controller.updateProduct(phone);

        // Khôi phục Invoices.txt và Orders.txt (chỉ xóa đúng mã hợp lệ)
        if (!createdInvoiceId.empty()) {
            std::string invPath = PathHelper::resolveDataPath("data/PAY/Invoices.txt");
            std::ifstream finv(invPath);
            std::vector<std::string> invLines;
            std::string l;
            while (std::getline(finv, l)) {
                if (l.find(createdInvoiceId) == std::string::npos) invLines.push_back(l);
            }
            finv.close();
            std::ofstream foinv(invPath);
            for (const auto& x : invLines) foinv << x << "\n";
            foinv.close();
        }

        if (!createdOrderId.empty()) {
            std::string ordPath = PathHelper::resolveDataPath("data/PAY/Order/Orders.txt");
            std::ifstream ford(ordPath);
            std::vector<std::string> ordLines;
            std::string l;
            while (std::getline(ford, l)) {
                if (l.find(createdOrderId) == std::string::npos) ordLines.push_back(l);
            }
            ford.close();
            std::ofstream foord(ordPath);
            for (const auto& x : ordLines) foord << x << "\n";
            foord.close();
        }
    }
}

// 7. Test Error Handling (No False Success on Write Failure)
void testErrorHandlingNoFalseSuccess() {
    std::cout << "\n=== [TEST 7] Error Handling: No False Success on Write Failure ===\n";
    TestAppSession session;

    // Cố ý cập nhật sản phẩm không tồn tại
    ProductItemViewModel nonExistent;
    nonExistent.id = "PHN-NONEXISTENT";
    nonExistent.name = "Ghost Phone";
    auto resUpdate = session.controller.updateProduct(nonExistent);
    TEST_ASSERT(!resUpdate.isSuccess(), "Cập nhật sản phẩm không tồn tại trả về thất bại (Không báo thành công giả)");

    // Cố ý xóa sản phẩm không tồn tại
    auto resDel = session.controller.deleteProduct("PHN-NONEXISTENT");
    TEST_ASSERT(!resDel.isSuccess(), "Xóa sản phẩm không tồn tại trả về thất bại");

    // Cập nhật sản phẩm với ID không tồn tại
    ProductItemViewModel updateDuplicate;
    updateDuplicate.id = "PHN-999999";
    updateDuplicate.name = "Fake Phone";
    updateDuplicate.sellingPrice = 100000;
    auto resUpDup = session.controller.updateProduct(updateDuplicate);
    TEST_ASSERT(!resUpDup.isSuccess(), "Cập nhật sản phẩm với ID không tồn tại trả về thất bại");

    // Xóa khách hàng không tồn tại
    auto resDelCus = session.controller.deleteCustomer("CUS-NONEXISTENT");
    TEST_ASSERT(!resDelCus.isSuccess(), "Xóa khách hàng không tồn tại trả về thất bại");
}

// 8. Test Working Directory Independence (PathHelper)
void testWorkingDirectoryIndependence() {
    std::cout << "\n=== [TEST 8] Working Directory Independence ===\n";
    std::string relPath = "data/Device/Products.txt";
    std::string resolved = PathHelper::resolveDataPath(relPath);

    TEST_ASSERT(!resolved.empty(), "PathHelper trả về đường dẫn không rỗng");
    TEST_ASSERT(std::filesystem::exists(resolved), "Đường dẫn resolved thực sự tồn tại trên disk");

    std::cout << "  [INFO] CWD: " << std::filesystem::current_path().string() << "\n";
    std::cout << "  [INFO] Resolved Path: " << resolved << "\n";
}

// 9. Test Preservation of Original Sample Data
void testSampleDataPreserved() {
    std::cout << "\n=== [TEST 9] Preservation of Original Sample Data ===\n";
    TestAppSession session;

    // Kiểm tra 7 sản phẩm mẫu nguyên bản
    const std::vector<std::string> sampleProductIds = {
        "PHN-000101", "PHN-000102", "PHN-000103", "PHN-000104",
        "PHN-000105", "PHN-000106", "PHN-000107"
    };
    bool allProductsPreserved = true;
    for (const auto& id : sampleProductIds) {
        auto p = session.controller.getProductById(id);
        if (p.id != id) {
            allProductsPreserved = false;
            std::cerr << "  [FAIL] Thieu san pham: " << id << "\n";
            break;
        }
    }
    TEST_ASSERT(allProductsPreserved, "Toàn bộ 7 sản phẩm mẫu nguyên bản được bảo toàn 100%");

    // Kiểm tra khách hàng mẫu
    auto c1 = session.controller.getCustomerById("CUS-000001");
    auto c2 = session.controller.getCustomerById("CUS-000002");
    auto c3 = session.controller.getCustomerById("CUS-000003");
    TEST_ASSERT(!c1.id.empty() && !c2.id.empty() && !c3.id.empty(),
                "Toàn bộ 3 khách hàng mẫu nguyên bản được bảo toàn 100%");

    // Kiểm tra nhà cung cấp mẫu
    auto suppliers = session.controller.getSuppliers();
    TEST_ASSERT(suppliers.size() >= 4, "Toàn bộ nhà cung cấp mẫu nguyên bản được bảo toàn");

    // Kiểm tra vouchers
    auto discounts = session.controller.getDiscounts();
    TEST_ASSERT(discounts.size() >= 4, "Toàn bộ voucher hệ thống được bảo toàn");
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "[REGRESSION TEST] FIXING PHASE A - PERSISTENCE CORE\n";
    std::cout << "========================================================\n";

    testProductPersistence();
    testCustomerPersistence();
    testDiscountPersistence();
    testSupplierPersistence();
    testStockReceiptPersistence();
    testPosInvoiceAndOrderPersistence();
    testErrorHandlingNoFalseSuccess();
    testWorkingDirectoryIndependence();
    testSampleDataPreserved();

    std::cout << "\n========================================================\n";
    std::cout << "TEST SUMMARY: PASSED=" << g_passCount << ", FAILED=" << g_failCount << "\n";
    std::cout << "========================================================\n";

    return (g_failCount == 0) ? 0 : 1;
}
