// tests/test_persistence_hardening.cpp
// Automated Regression Test Suite for Phase A.1 - Persistence Hardening
// Verifies Multi-file Transactions, Rollbacks, Atomic Writes, Seed Data, and Process Restart

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <cstdlib>

#include "source/PBL2.h"
#include "source/controllers/AppController.h"
#include "source/utils/PathHelper.h"
#include "source/utils/FileHelper.h"
#include "source/utils/TransactionCoordinator.h"
#include "UI/mock/MockDataStore.h"

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

#define TEST_ASSERT_FAIL(res, msg) \
    do { \
        if (!res.isSuccess()) { \
            std::cout << "  [PASS] " << msg << " (Đúng như kỳ vọng: " << res.message << ")\n"; \
            g_passCount++; \
        } else { \
            std::cerr << "  [FAIL] " << msg << " -> Kỳ vọng thất bại nhưng lại thành công! (Line " << __LINE__ << ")\n"; \
            g_failCount++; \
        } \
    } while (0)

// Cấu trúc khởi tạo Repositories trong thư mục Sandbox biệt lập
struct SandboxRepositoriesHolder {
    std::string dir;
    PhoneRepository        phoneRepo;
    OrderRepository        orderRepo;
    InvoiceRepository      invoiceRepo;
    StockReceiptRepository stockRepo;
    CustomerRepository     customerRepo;
    EmployeeRepository     employeeRepo;
    SupplierRepository     supplierRepo;
    DiscountRepository     discountRepo;

    SandboxRepositoriesHolder(const std::string& sandboxDir)
        : dir(sandboxDir),
          phoneRepo(sandboxDir + "/Device/Products.txt"),
          orderRepo(sandboxDir + "/PAY/Order/Orders.txt"),
          invoiceRepo(sandboxDir + "/PAY/Invoices.txt"),
          stockRepo(sandboxDir + "/Device/StockReceipts.txt"),
          customerRepo(sandboxDir + "/people/Customers.txt"),
          employeeRepo(sandboxDir + "/people/Employee.txt"),
          supplierRepo(sandboxDir + "/people/Suppliers.txt"),
          discountRepo(sandboxDir + "/PAY/Discount.txt")
    {
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

struct SandboxAppSession {
    SandboxRepositoriesHolder repos;
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

    SandboxAppSession(const std::string& sandboxDir)
        : repos(sandboxDir),
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
          controller(&phoneService, &orderService, &customerService,
                     &employeeService, &invoiceService, &supplierService,
                     &discountService, &reportService, &authService)
    {
        LoginRequest loginReq;
        loginReq.username = "admin.demo";
        loginReq.password = "DemoOnly-Admin-2026";
        controller.login(loginReq);
    }
};

// Thiết lập thư mục Sandbox độc lập từ data/ (sao chép đệ quy toàn bộ thư mục)
static void setupSandbox(const std::string& srcDir, const std::string& dstDir) {
    std::error_code ec;
    std::filesystem::remove_all(dstDir, ec);
    std::filesystem::create_directories(dstDir, ec);
    std::filesystem::copy(srcDir, dstDir,
                          std::filesystem::copy_options::recursive |
                          std::filesystem::copy_options::overwrite_existing, ec);
}

// Dọn dẹp thư mục Sandbox
static void cleanupSandbox(const std::string& dstDir) {
    std::error_code ec;
    std::filesystem::remove_all(dstDir, ec);
}

// Đọc toàn bộ file thành string để kiểm tra tính toàn vẹn
static std::string readFileContents(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

// Đếm số dòng trong file
static int countFileLines(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) return 0;
    int lines = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) lines++;
    }
    return lines;
}

// =========================================================================
// CHẾ ĐỘ XÁC THỰC TIẾN TRÌNH CON ĐỘC LẬP (PROCESS-LEVEL RESTART VERIFIER)
// =========================================================================
static int runSubprocessVerification(const std::string& sandboxDir) {
    std::cout << "[Subprocess Verifier] Đang khởi chạy trong tiến trình mới (PID riêng)...\n";
    SandboxAppSession session(sandboxDir);

    // Xác nhận sản phẩm từ phiên trước vẫn tồn tại
    Phone* p = session.repos.phoneRepo.findById("PHN-PROC-999");
    if (!p) {
        std::cerr << "[Subprocess Verifier LỖI] Không tìm thấy PHN-PROC-999 sau khi mở tiến trình mới!\n";
        return 1;
    }
    if (p->getQuantity() != 77) {
        std::cerr << "[Subprocess Verifier LỖI] Số lượng tồn kho PHN-PROC-999 không khớp! (Thực tế: "
                  << p->getQuantity() << ", kỳ vọng: 77)\n";
        return 2;
    }

    // Xác nhận khách hàng từ phiên trước vẫn tồn tại
    Customer* c = session.repos.customerRepo.findById("CUS-PROC-888");
    if (!c) {
        std::cerr << "[Subprocess Verifier LỖI] Không tìm thấy CUS-PROC-888 sau khi mở tiến trình mới!\n";
        return 3;
    }

    std::cout << "[Subprocess Verifier] Xác thực thành công: Dữ liệu được nạp chính xác từ đĩa sang tiến trình mới!\n";
    return 0;
}

// =========================================================================
// TEST SUITE CHÍNH
// =========================================================================
int main(int argc, char* argv[]) {
    std::string sandboxDir = "tests/sandbox_data";

    // Nếu được gọi ở chế độ tiến trình con kiểm thử
    if (argc >= 3 && std::string(argv[1]) == "--verify-proc") {
        return runSubprocessVerification(argv[2]);
    }

    std::cout << "========================================================================\n";
    std::cout << "  PBL2 REGRESSION TEST SUITE: PHASE A.1 - PERSISTENCE HARDENING\n";
    std::cout << "========================================================================\n\n";

    // Khởi tạo Sandbox sạch từ data/
    std::string sourceDataDir = PathHelper::resolveDataPath("data");
    setupSandbox(sourceDataDir, sandboxDir);
    std::cout << "[Sandbox] Đã sao chép đệ quy dữ liệu từ '" << sourceDataDir << "' sang '" << sandboxDir << "'\n\n";

    // =========================================================================
    // TEST 1: ATOMIC FILE WRITE VÀ BẢO TOÀN DỮ LIỆU KHI GHI THẤT BẠI
    // =========================================================================
    std::cout << "--- [TEST 1] Atomic File Write: Bảo toàn file gốc khi ghi thất bại ---\n";
    {
        std::string targetFile = sandboxDir + "/Device/Products.txt";
        std::string originalContent = readFileContents(targetFile);
        TEST_ASSERT(!originalContent.empty(), "File Products.txt trong sandbox có dữ liệu hợp lệ ban đầu");

        // Bật mô phỏng lỗi ghi file
        FileHelper::simulateFailureOn("Products.txt");

        SandboxAppSession session(sandboxDir);
        bool saveResult = session.repos.phoneRepo.saveToFile();
        TEST_ASSERT(!saveResult, "saveToFile() trả về false khi gặp lỗi ghi file (không báo thành công giả)");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra file trên đĩa không bị truncate về 0 bytes, không bị mất nội dung
        std::string postFailureContent = readFileContents(targetFile);
        TEST_ASSERT(postFailureContent == originalContent, "File Products.txt trên đĩa được giữ nguyên 100%, không bị truncate hay biến mất");
        
        // Kiểm tra file tạm .tmp đã được dọn sạch
        std::error_code ec;
        TEST_ASSERT(!std::filesystem::exists(targetFile + ".tmp", ec), "File tạm Products.txt.tmp đã được dọn sạch");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 2: REPOSITORY SINGLE-FILE IN-RAM ROLLBACK KHI SAVE THẤT BẠI
    // =========================================================================
    std::cout << "--- [TEST 2] Repository Single-File In-RAM Rollback khi saveToFile() thất bại ---\n";
    {
        SandboxAppSession session(sandboxDir);
        int initialCustomerCount = session.repos.customerRepo.getAll().getSize();

        FileHelper::simulateFailureOn("Customers.txt");

        Name newName("Test", "Rollback", "User");
        Customer testCust("CUS-FAIL-001", newName, "0912999888", "test@fail.com", "Sandbox", "test_rb", "123456", 0);

        bool addResult = session.repos.customerRepo.addCustomer(testCust);
        TEST_ASSERT(!addResult, "addCustomer() trả về false khi ghi file thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra trạng thái RAM: CustomerRepository không được giữ bản ghi chưa lưu
        TEST_ASSERT(session.repos.customerRepo.getAll().getSize() == initialCustomerCount,
                    "RAM State của CustomerRepository được hoàn nguyên chính xác (số lượng không đổi)");
        TEST_ASSERT(session.repos.customerRepo.findById("CUS-FAIL-001") == nullptr,
                    "Không tồn tại bản ghi 'CUS-FAIL-001' trong bộ nhớ RAM của Repository");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 3: POS MULTI-FILE TRANSACTION - LỖI Ở BƯỚC 1 (PRODUCTS.TXT)
    // =========================================================================
    std::cout << "--- [TEST 3] POS Multi-file Transaction: Lỗi ở Bước 1 (Trừ kho Products.txt) ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        TEST_ASSERT(phone != nullptr, "Tìm thấy sản phẩm PHN-000101 để bán");
        int initialStock = phone->getQuantity();
        int initialOrders = countFileLines(sandboxDir + "/PAY/Order/Orders.txt");
        int initialInvoices = countFileLines(sandboxDir + "/PAY/Invoices.txt");

        // Giả lập POS cart
        std::vector<CartItemViewModel> cart;
        CartItemViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 2;
        item.unitPrice = phone->getSellingPrice();
        item.subtotal = item.quantity * item.unitPrice;
        cart.push_back(item);

        // Mô phỏng lỗi ghi file Products.txt
        FileHelper::simulateFailureOn("Products.txt");

        double outChange = 0.0;
        std::string outInvoiceId;
        OperationResult res = session.controller.createPosInvoice("WALK-IN", "EMP-000002", cart, "",
                                                                 PaymentMethod::CASH, 100000000.0,
                                                                 outChange, outInvoiceId);
        TEST_ASSERT_FAIL(res, "createPosInvoice() từ chối khi cập nhật kho Products.txt thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra tính nhất quán: Kho chưa bị trừ, Đơn hàng và Hóa đơn không được tạo
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock,
                    "Tồn kho RAM của PhoneRepository không bị trừ sai");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Order/Orders.txt") == initialOrders,
                    "File Orders.txt không bị ghi thêm đơn rác");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Invoices.txt") == initialInvoices,
                    "File Invoices.txt không bị ghi thêm hóa đơn rác");
        TEST_ASSERT(MockDataStore::getInstance().invoices.empty() ||
                    MockDataStore::getInstance().invoices.front().id != outInvoiceId,
                    "MockDataStore view cache không bị cập nhật hóa đơn chưa commit");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 4: POS MULTI-FILE TRANSACTION - LỖI Ở BƯỚC 2 (ORDERS.TXT)
    // =========================================================================
    std::cout << "--- [TEST 4] POS Multi-file Transaction: Lỗi ở Bước 2 (Lưu Order vào Orders.txt) ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialOrders = countFileLines(sandboxDir + "/PAY/Order/Orders.txt");
        int initialInvoices = countFileLines(sandboxDir + "/PAY/Invoices.txt");

        std::vector<CartItemViewModel> cart;
        CartItemViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 3;
        item.unitPrice = phone->getSellingPrice();
        item.subtotal = item.quantity * item.unitPrice;
        cart.push_back(item);

        // Mô phỏng lỗi ở Orders.txt (Products.txt sẽ ghi thành công trước, nhưng sau đó phải được rollback!)
        FileHelper::simulateFailureOn("Orders.txt");

        double outChange = 0.0;
        std::string outInvoiceId;
        OperationResult res = session.controller.createPosInvoice("WALK-IN", "EMP-000002", cart, "",
                                                                 PaymentMethod::CASH, 100000000.0,
                                                                 outChange, outInvoiceId);
        TEST_ASSERT_FAIL(res, "createPosInvoice() từ chối khi lưu Orders.txt thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra ROLLBACK thành công trên cả đĩa và RAM:
        // Products.txt trên đĩa phải được hoàn về ban đầu từ .tx_bak
        // PhoneRepository RAM phải được reload về initialStock
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock,
                    "ROLLBACK THÀNH CÔNG: Tồn kho sản phẩm được hoàn nguyên chính xác (không bị trừ oan)");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Order/Orders.txt") == initialOrders,
                    "Orders.txt không có đơn hàng mồ côi");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Invoices.txt") == initialInvoices,
                    "Invoices.txt không có hóa đơn mồ côi");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 5: POS MULTI-FILE TRANSACTION - LỖI Ở BƯỚC 3 (INVOICES.TXT)
    // =========================================================================
    std::cout << "--- [TEST 5] POS Multi-file Transaction: Lỗi ở Bước 3 (Lưu Invoice vào Invoices.txt) ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialOrders = countFileLines(sandboxDir + "/PAY/Order/Orders.txt");
        int initialInvoices = countFileLines(sandboxDir + "/PAY/Invoices.txt");

        std::vector<CartItemViewModel> cart;
        CartItemViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 1;
        item.unitPrice = phone->getSellingPrice();
        item.subtotal = item.quantity * item.unitPrice;
        cart.push_back(item);

        // Mô phỏng lỗi ở Invoices.txt (Products.txt và Orders.txt ghi thành công, nhưng đều phải rollback!)
        FileHelper::simulateFailureOn("Invoices.txt");

        double outChange = 0.0;
        std::string outInvoiceId;
        OperationResult res = session.controller.createPosInvoice("WALK-IN", "EMP-000002", cart, "",
                                                                 PaymentMethod::CASH, 100000000.0,
                                                                 outChange, outInvoiceId);
        TEST_ASSERT_FAIL(res, "createPosInvoice() từ chối khi lưu Invoices.txt thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra ROLLBACK đa file toàn diện:
        // Cả Products.txt và Orders.txt đều phải được khôi phục nguyên trạng!
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock,
                    "ROLLBACK ĐA FILE: Tồn kho Products.txt và RAM đã được hoàn nguyên");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Order/Orders.txt") == initialOrders,
                    "ROLLBACK ĐA FILE: Đơn hàng đã được xóa khỏi Orders.txt (không để lại đơn mồ côi)");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Invoices.txt") == initialInvoices,
                    "Invoices.txt giữ nguyên trạng thái trước giao dịch");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 5b: POS MULTI-FILE TRANSACTION - LỖI Ở BƯỚC 4 (TIÊU THỤ DISCOUNT.TXT)
    // =========================================================================
    std::cout << "--- [TEST 5b] POS Multi-file Transaction: Lỗi ở Bước 4 (Cập nhật voucher Discount.txt) ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialOrders = countFileLines(sandboxDir + "/PAY/Order/Orders.txt");
        int initialInvoices = countFileLines(sandboxDir + "/PAY/Invoices.txt");
        DiscountCode* dc = session.repos.discountRepo.findByCode("SALE10");
        int initialUsedCount = dc ? dc->getUsedCount() : 0;

        std::vector<CartItemViewModel> cart;
        CartItemViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 1;
        item.unitPrice = phone->getSellingPrice();
        item.subtotal = item.quantity * item.unitPrice;
        cart.push_back(item);

        // Mô phỏng lỗi ở Discount.txt (Products, Orders, Invoices đã ghi, nhưng phải rollback toàn bộ!)
        FileHelper::simulateFailureOn("Discount.txt");

        double outChange = 0.0;
        std::string outInvoiceId;
        OperationResult res = session.controller.createPosInvoice("WALK-IN", "EMP-000002", cart, "SALE10",
                                                                 PaymentMethod::CASH, 100000000.0,
                                                                 outChange, outInvoiceId);
        TEST_ASSERT_FAIL(res, "createPosInvoice() từ chối khi cập nhật Discount.txt thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra ROLLBACK đa file toàn diện khi bước 4 thất bại:
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock,
                    "ROLLBACK BƯỚC 4: Tồn kho Products.txt được hoàn nguyên");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Order/Orders.txt") == initialOrders,
                    "ROLLBACK BƯỚC 4: Đơn hàng đã được xóa khỏi Orders.txt");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Invoices.txt") == initialInvoices,
                    "ROLLBACK BƯỚC 4: Hóa đơn đã được xóa khỏi Invoices.txt");
        TEST_ASSERT(session.repos.discountRepo.findByCode("SALE10")->getUsedCount() == initialUsedCount,
                    "ROLLBACK BƯỚC 4: Lượt dùng voucher không bị trừ sai");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 5c: POS MULTI-FILE TRANSACTION - LỖI Ở BƯỚC 5 (TÍCH ĐIỂM CUSTOMERS.TXT)
    // =========================================================================
    std::cout << "--- [TEST 5c] POS Multi-file Transaction: Lỗi ở Bước 5 (Tích điểm Customers.txt) ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialOrders = countFileLines(sandboxDir + "/PAY/Order/Orders.txt");
        int initialInvoices = countFileLines(sandboxDir + "/PAY/Invoices.txt");
        Customer* cust = session.repos.customerRepo.findById("CUS-000001");
        int initialPoints = cust ? cust->getPoints() : 0;

        std::vector<CartItemViewModel> cart;
        CartItemViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 1;
        item.unitPrice = phone->getSellingPrice();
        item.subtotal = item.quantity * item.unitPrice;
        cart.push_back(item);

        // Mô phỏng lỗi ở Customers.txt (Products, Orders, Invoices đã ghi, nhưng phải rollback!)
        FileHelper::simulateFailureOn("Customers.txt");

        double outChange = 0.0;
        std::string outInvoiceId;
        OperationResult res = session.controller.createPosInvoice("CUS-000001", "EMP-000002", cart, "",
                                                                 PaymentMethod::CASH, 100000000.0,
                                                                 outChange, outInvoiceId);
        TEST_ASSERT_FAIL(res, "createPosInvoice() từ chối khi cập nhật Customers.txt thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra ROLLBACK toàn diện khi bước 5 thất bại:
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock,
                    "ROLLBACK BƯỚC 5: Tồn kho Products.txt được hoàn nguyên");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Order/Orders.txt") == initialOrders,
                    "ROLLBACK BƯỚC 5: Đơn hàng đã được xóa khỏi Orders.txt");
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Invoices.txt") == initialInvoices,
                    "ROLLBACK BƯỚC 5: Hóa đơn đã được xóa khỏi Invoices.txt");
        TEST_ASSERT(session.repos.customerRepo.findById("CUS-000001")->getPoints() == initialPoints,
                    "ROLLBACK BƯỚC 5: Điểm khách hàng giữ nguyên");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 6: THỬ LẠI GIAO DỊCH POS SAU KHI SỬA LỖI (RETRY WITHOUT DUPLICATES)
    // =========================================================================
    std::cout << "--- [TEST 6] Thử lại POS sau sự cố: Ghi thành công, không tạo trùng lặp ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialOrders = countFileLines(sandboxDir + "/PAY/Order/Orders.txt");
        int initialInvoices = countFileLines(sandboxDir + "/PAY/Invoices.txt");

        std::vector<CartItemViewModel> cart;
        CartItemViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 2;
        item.unitPrice = phone->getSellingPrice();
        item.subtotal = item.quantity * item.unitPrice;
        cart.push_back(item);

        double outChange = 0.0;
        std::string outInvoiceId;
        OperationResult res = session.controller.createPosInvoice("WALK-IN", "EMP-000002", cart, "",
                                                                 PaymentMethod::CASH, 100000000.0,
                                                                 outChange, outInvoiceId);
        TEST_ASSERT_OP(res, "Thử lại createPosInvoice() thành công 100%");
        TEST_ASSERT(!outInvoiceId.empty(), "Sinh mã hóa đơn hợp lệ: " + outInvoiceId);

        // Kiểm tra: Tồn kho bị trừ đúng 2 đơn vị
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock - 2,
                    "Tồn kho sản phẩm bị trừ chính xác 2 đơn vị");
        // Đúng 1 đơn hàng mới trong Orders.txt
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Order/Orders.txt") == initialOrders + 1,
                    "Tạo đúng 1 đơn hàng trong Orders.txt (không tạo trùng lặp)");
        // Đúng 1 hóa đơn mới trong Invoices.txt
        TEST_ASSERT(countFileLines(sandboxDir + "/PAY/Invoices.txt") == initialInvoices + 1,
                    "Tạo đúng 1 hóa đơn trong Invoices.txt");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 7: NHẬP KHO MULTI-FILE TRANSACTION - LỖI KHI CẬP NHẬT TỒN KHO
    // =========================================================================
    std::cout << "--- [TEST 7] Nhập kho Multi-file Transaction: Lỗi khi cập nhật tồn kho Products.txt ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialReceipts = countFileLines(sandboxDir + "/Device/StockReceipts.txt");

        CreateReceiptRequest req;
        req.supplierId = "SUP-000001";
        req.employeeId = "EMP-000003";
        GoodsReceiptDetailViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 15;
        item.importPrice = 18000000.0;
        req.items.push_back(item);

        // Mô phỏng lỗi ở Products.txt khi tăng kho
        FileHelper::simulateFailureOn("Products.txt");

        OperationResult res = session.controller.createGoodsReceipt(req);
        TEST_ASSERT_FAIL(res, "createGoodsReceipt() từ chối khi cập nhật tồn kho thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra ROLLBACK: Phiếu nhập không được lưu lại mồ côi
        TEST_ASSERT(countFileLines(sandboxDir + "/Device/StockReceipts.txt") == initialReceipts,
                    "ROLLBACK NHẬP KHO: Phiếu nhập không bị lưu lại trong StockReceipts.txt");
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock,
                    "Tồn kho sản phẩm giữ nguyên giá trị cũ");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 7b: NHẬP KHO MULTI-FILE TRANSACTION - LỖI NGAY BƯỚC 1 (STOCKRECEIPTS.TXT)
    // =========================================================================
    std::cout << "--- [TEST 7b] Nhập kho Multi-file Transaction: Lỗi ở Bước 1 (Ghi StockReceipts.txt) ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialReceipts = countFileLines(sandboxDir + "/Device/StockReceipts.txt");

        CreateReceiptRequest req;
        req.supplierId = "SUP-000001";
        req.employeeId = "EMP-000003";
        GoodsReceiptDetailViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 20;
        item.importPrice = 18000000.0;
        req.items.push_back(item);

        // Mô phỏng lỗi ở StockReceipts.txt
        FileHelper::simulateFailureOn("StockReceipts.txt");

        OperationResult res = session.controller.createGoodsReceipt(req);
        TEST_ASSERT_FAIL(res, "createGoodsReceipt() từ chối khi lưu StockReceipts.txt thất bại");

        FileHelper::clearSimulatedFailure();

        // Kiểm tra: Kho chưa hề bị tăng, không có phiếu rác
        TEST_ASSERT(countFileLines(sandboxDir + "/Device/StockReceipts.txt") == initialReceipts,
                    "StockReceipts.txt không có phiếu nhập rác");
        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock,
                    "Tồn kho sản phẩm không bị tăng sai khi bước 1 thất bại");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 8: THỬ LẠI NHẬP KHO THÀNH CÔNG VÀ ĐỒNG BỘ NHẤT QUÁN
    // =========================================================================
    std::cout << "--- [TEST 8] Thử lại nhập kho thành công sau khi sửa lỗi ---\n";
    {
        SandboxAppSession session(sandboxDir);
        std::string pId = "PHN-000101";
        Phone* phone = session.repos.phoneRepo.findById(pId);
        int initialStock = phone->getQuantity();
        int initialReceipts = countFileLines(sandboxDir + "/Device/StockReceipts.txt");

        CreateReceiptRequest req;
        req.supplierId = "SUP-000001";
        req.employeeId = "EMP-000003";
        GoodsReceiptDetailViewModel item;
        item.phoneId = pId;
        item.phoneName = phone->getName();
        item.quantity = 10;
        item.importPrice = 18000000.0;
        req.items.push_back(item);

        OperationResult res = session.controller.createGoodsReceipt(req);
        TEST_ASSERT_OP(res, "createGoodsReceipt() thành công");

        TEST_ASSERT(session.repos.phoneRepo.findById(pId)->getQuantity() == initialStock + 10,
                    "Tồn kho sản phẩm được tăng chính xác 10 đơn vị");
        TEST_ASSERT(countFileLines(sandboxDir + "/Device/StockReceipts.txt") == initialReceipts + 1,
                    "Đúng 1 phiếu nhập kho được lưu vào StockReceipts.txt");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 9: KIỂM SOÁT SEED DATA (FILE KHÔNG TỒN TẠI VS FILE TỒN TẠI NHƯNG RỖNG)
    // =========================================================================
    std::cout << "--- [TEST 9] Seed Data Refinement: Không tự ý seed lại khi người dùng đã chủ động xóa hết ---\n";
    {
        std::string discountFile = sandboxDir + "/PAY/Discount.txt";

        // 9a: Người dùng xóa hết voucher -> Ghi đè file chỉ còn header (0 bản ghi)
        FileHelper::atomicWrite(discountFile, [](std::ofstream& file) {
            file << "code|type|value|startDate|endDate|minOrderValue|maxDiscountAmount|maxUsage|usedCount\n";
            return true;
        });
        TEST_ASSERT(std::filesystem::exists(discountFile), "File Discount.txt tồn tại và chỉ chứa header");

        // Khởi tạo DiscountRepository nạp từ file rỗng này
        DiscountRepository emptyRepo(discountFile);
        bool loadOk = emptyRepo.loadFromFile();
        TEST_ASSERT(loadOk, "loadFromFile() trên file rỗng trả về true");
        TEST_ASSERT(emptyRepo.getAll().getSize() == 0,
                    "TÔN TRỌNG DỮ LIỆU NGƯỜI DÙNG: Không tự ý nạp lại voucher mặc định khi file đã tồn tại");

        // 9b: File hoàn toàn không tồn tại -> Hệ thống mới tạo seed data lần đầu
        std::error_code ec;
        std::filesystem::remove(discountFile, ec);
        TEST_ASSERT(!std::filesystem::exists(discountFile, ec), "File Discount.txt hoàn toàn không tồn tại");

        DiscountRepository missingRepo(discountFile);
        missingRepo.loadFromFile();
        TEST_ASSERT(missingRepo.getAll().getSize() > 0,
                    "SEED DATA HỢP LỆ: Tự động tạo voucher mặc định khi file chưa từng tồn tại trên đĩa");
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 10: TỰ ĐỘNG KHÔI PHỤC SAU SỰ CỐ DỰA TRÊN MANIFEST
    // =========================================================================
    std::cout << "--- [TEST 10] Crash Recovery: Xác thực 3 kịch bản sập nguồn/crash quan trọng ---\n";
    {
        // Kịch bản 10a: Crash sau khi Commit nhưng trước khi Cleanup (STATUS=COMMITTED)
        // Hệ thống TUYỆT ĐỐI KHÔNG ĐƯỢC rollback nhầm dữ liệu mới!
        {
            std::string mainFile = sandboxDir + "/CommitMarkerTest.txt";
            std::string bakFile = mainFile + ".tx_bak";
            std::string manifestFile = sandboxDir + "/.tx_manifest";

            // File chính đã mang dữ liệu MỚI đã commit
            FileHelper::atomicWrite(mainFile, [](std::ofstream& out) {
                out << "COMMITTED_NEW_DATA_MUST_BE_PRESERVED\n";
                return true;
            });

            // File backup cũ còn sót lại do crash trong cleanup
            FileHelper::atomicWrite(bakFile, [](std::ofstream& out) {
                out << "OLD_STALE_BACKUP_SHOULD_NOT_OVERWRITE\n";
                return true;
            });

            // Manifest ghi nhận rõ STATUS=COMMITTED
            FileHelper::atomicWrite(manifestFile, [&](std::ofstream& out) {
                out << "STATUS=COMMITTED\n";
                out << "PARTICIPANT|" << mainFile << "|" << bakFile << "|1\n";
                return true;
            });

            // Khởi động lại ứng dụng -> Crash recovery chạy
            FileTransaction::recoverPendingTransactions(sandboxDir);

            // Xác nhận: Dữ liệu mới đã commit được bảo toàn 100%, backup thừa đã bị xóa!
            std::string contentAfter = readFileContents(mainFile);
            TEST_ASSERT(contentAfter.find("COMMITTED_NEW_DATA_MUST_BE_PRESERVED") != std::string::npos,
                        "Kịch bản 10a: Dữ liệu đã commit được bảo toàn, KHÔNG bị rollback nhầm!");
            std::error_code ec;
            TEST_ASSERT(!std::filesystem::exists(bakFile, ec),
                        "Kịch bản 10a: File .tx_bak thừa đã được dọn sạch");
            TEST_ASSERT(!std::filesystem::exists(manifestFile, ec),
                        "Kịch bản 10a: Manifest đã được dọn sạch");

            std::filesystem::remove(mainFile, ec);
        }

        // Kịch bản 10b: Crash trong khi đang tạo snapshot (STATUS=PREPARING)
        // File backup có thể bị hỏng/truncate, nhưng file gốc an toàn -> KHÔNG restore từ backup hỏng!
        {
            std::string mainFile = sandboxDir + "/SnapshotCrashTest.txt";
            std::string bakFile = mainFile + ".tx_bak";
            std::string manifestFile = sandboxDir + "/.tx_manifest";

            // File chính nguyên bản lành lặn
            FileHelper::atomicWrite(mainFile, [](std::ofstream& out) {
                out << "INTACT_ORIGINAL_DATA_FILE\n";
                return true;
            });

            // File backup bị ghi dở dang/corrupted khi crash
            FileHelper::atomicWrite(bakFile, [](std::ofstream& out) {
                out << "CORRUPTED_INCOMPLETE_SNAPSHOT\n";
                return true;
            });

            // Manifest đang ở trạng thái PREPARING
            FileHelper::atomicWrite(manifestFile, [&](std::ofstream& out) {
                out << "STATUS=PREPARING\n";
                out << "PARTICIPANT|" << mainFile << "|" << bakFile << "|1\n";
                return true;
            });

            // Khởi động lại ứng dụng -> Crash recovery chạy
            FileTransaction::recoverPendingTransactions(sandboxDir);

            // Xác nhận: File chính không bị ghi đè bởi backup hỏng!
            std::string contentAfter = readFileContents(mainFile);
            TEST_ASSERT(contentAfter.find("INTACT_ORIGINAL_DATA_FILE") != std::string::npos,
                        "Kịch bản 10b: File gốc được bảo toàn, không bị ghi đè bởi snapshot hỏng!");
            std::error_code ec;
            TEST_ASSERT(!std::filesystem::exists(bakFile, ec),
                        "Kịch bản 10b: Snapshot dở dang đã được dọn sạch");
            TEST_ASSERT(!std::filesystem::exists(manifestFile, ec),
                        "Kịch bản 10b: Manifest đã được dọn sạch");

            std::filesystem::remove(mainFile, ec);
        }

        // Kịch bản 10c: Crash giữa chừng khi giao dịch chưa commit (STATUS=ACTIVE)
        // File chính bị ghi dở dang -> BẮT BUỘC ROLLBACK từ snapshot
        {
            std::string mainFile = sandboxDir + "/ActiveCrashTest.txt";
            std::string bakFile = mainFile + ".tx_bak";
            std::string manifestFile = sandboxDir + "/.tx_manifest";

            // File backup an toàn trước giao dịch
            FileHelper::atomicWrite(bakFile, [](std::ofstream& out) {
                out << "SAFE_ORIGINAL_DATA_BEFORE_TRANSACTION\n";
                return true;
            });

            // File chính bị hỏng/dở dang do crash giữa chừng
            FileHelper::atomicWrite(mainFile, [](std::ofstream& out) {
                out << "PARTIAL_UNCOMMITTED_GARBAGE_DATA\n";
                return true;
            });

            // Manifest ở trạng thái ACTIVE
            FileHelper::atomicWrite(manifestFile, [&](std::ofstream& out) {
                out << "STATUS=ACTIVE\n";
                out << "PARTICIPANT|" << mainFile << "|" << bakFile << "|1\n";
                return true;
            });

            // Khởi động lại ứng dụng -> Crash recovery chạy
            FileTransaction::recoverPendingTransactions(sandboxDir);

            // Xác nhận: File chính được rollback an toàn về trạng thái trước giao dịch!
            std::string contentAfter = readFileContents(mainFile);
            TEST_ASSERT(contentAfter.find("SAFE_ORIGINAL_DATA_BEFORE_TRANSACTION") != std::string::npos,
                        "Kịch bản 10c: Rollback thành công file chính về dữ liệu an toàn trước crash!");
            std::error_code ec;
            TEST_ASSERT(!std::filesystem::exists(bakFile, ec),
                        "Kịch bản 10c: File snapshot đã được dọn sạch");
            TEST_ASSERT(!std::filesystem::exists(manifestFile, ec),
                        "Kịch bản 10c: Manifest đã được dọn sạch");

            std::filesystem::remove(mainFile, ec);
        }
    }
    std::cout << "\n";

    // =========================================================================
    // TEST 11: TIẾN TRÌNH ĐỘC LẬP (PROCESS-LEVEL RESTART VERIFICATION)
    // =========================================================================
    std::cout << "--- [TEST 11] Multi-process Restart Verification: Kiểm tra bền vững qua ranh giới Process ---\n";
    {
        // Trong Tiến trình hiện tại (Process A): Tạo sản phẩm và khách hàng mới
        {
            SandboxAppSession session(sandboxDir);
            
            // Thêm sản phẩm PHN-PROC-999
            Phone newPhone("PHN-PROC-999", "Process Verification Phone", "ProBrand",
                           "Android 15", 20000000.0, 25000000.0, 77, "Google", "Test Spec");
            bool addPhoneOk = session.repos.phoneRepo.addPhone(newPhone);
            TEST_ASSERT(addPhoneOk, "Process A: Thêm sản phẩm PHN-PROC-999 thành công");

            // Thêm khách hàng CUS-PROC-888
            Name cName("Nguyễn", "Văn", "Tiến Trình");
            Customer newCust("CUS-PROC-888", cName, "0905123456", "process@test.com", "Sandbox", "proc_user", "123456", 100);
            bool addCustOk = session.repos.customerRepo.addCustomer(newCust);
            TEST_ASSERT(addCustOk, "Process A: Thêm khách hàng CUS-PROC-888 thành công");
        } // Process A đóng hoàn toàn scope session

        // Khởi chạy Tiến trình con mới (Process B) thông qua HĐH để đọc dữ liệu từ đĩa
        std::string selfExe = argv[0];
        std::string cmd = selfExe + " --verify-proc " + sandboxDir;
        int exitCode = std::system(cmd.c_str());
        TEST_ASSERT(exitCode == 0, "Process B (Tiến trình mới riêng biệt) nạp và xác nhận dữ liệu thành công với mã thoát 0");
    }
    std::cout << "\n";

    // Dọn dẹp thư mục Sandbox sau khi toàn bộ test hoàn tất
    cleanupSandbox(sandboxDir);
    std::cout << "[Sandbox] Đã dọn dẹp sạch thư mục sandbox: " << sandboxDir << "\n\n";

    // =========================================================================
    // TỔNG KẾT KẾT QUẢ TEST
    // =========================================================================
    std::cout << "========================================================================\n";
    std::cout << "  KẾT QUẢ KIỂM THỬ PHASE A.1 (PERSISTENCE HARDENING):\n";
    std::cout << "  - TỔNG SỐ TEST CASES ĐẠT : " << g_passCount << "\n";
    std::cout << "  - TỔNG SỐ TEST CASES LỖI : " << g_failCount << "\n";
    std::cout << "========================================================================\n";

    if (g_failCount == 0) {
        std::cout << ">>> TẤT CẢ TEST CASES GIAO DỊCH VÀ BỀN VỮNG ĐÃ VƯỢT QUA 100%! <<<\n\n";
        return 0;
    } else {
        std::cerr << ">>> CÓ " << g_failCount << " TEST CASES BỊ THẤT BẠI! <<<\n\n";
        return 1;
    }
}
