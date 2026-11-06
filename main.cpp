// main.cpp — Entry point chính của ứng dụng GTK4 (PBL2 Phone Store Manager)
// Thứ tự khởi tạo: Repositories → Services → AppController → AppRunner → g_application_run

#include <iostream>
#include <glib.h>
#include "source/PBL2.h"
#include "source/controllers/AppController.h"
#include "source/utils/TransactionCoordinator.h"
#include "source/utils/PathHelper.h"
#include "UI/AppRunner.h"

int main(int argc, char *argv[]) {
    // Tránh crash 'GTK_IS_IM_CONTEXT_IME' khi gõ tiếng Việt trên Windows: dùng IM module "simple".
    // Bắt buộc đặt TRƯỚC khi GTK được khởi tạo (trước AppRunner).
    g_setenv("GTK_IM_MODULE", "simple", TRUE);

    std::cout << "========================================================\n";
    std::cout << "[PBL2] Khởi tạo Phone Store Management System (GTK4)...\n";
    std::cout << "========================================================\n";

    // ─────────────────────────────────────────────────────────────────────────
    // Bước 0: Quét & khôi phục các giao dịch file dở dang nếu ứng dụng crash
    // ─────────────────────────────────────────────────────────────────────────
    FileTransaction::recoverPendingTransactions(PathHelper::resolveDataPath("data"));

    // ─────────────────────────────────────────────────────────────────────────
    // Bước 1: Khởi tạo Repositories và nạp dữ liệu từ data/
    // ─────────────────────────────────────────────────────────────────────────
    PhoneRepository        phoneRepo;
    OrderRepository        orderRepo;
    InvoiceRepository      invoiceRepo;
    StockReceiptRepository stockRepo;
    CustomerRepository     customerRepo;
    EmployeeRepository     employeeRepo;
    SupplierRepository     supplierRepo;
    DiscountRepository     discountRepo;

    if (!phoneRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu sản phẩm từ file!\n";
    }
    if (!orderRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu đơn hàng từ file!\n";
    }
    if (!invoiceRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu hóa đơn từ file!\n";
    }
    if (!stockRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu phiếu nhập từ file!\n";
    }
    if (!customerRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu khách hàng từ file!\n";
    }
    if (!employeeRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu nhân viên từ file!\n";
    }
    if (!supplierRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu nhà cung cấp từ file!\n";
    }
    if (!discountRepo.loadFromFile()) {
        std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu mã giảm giá từ file!\n";
    }
    std::cout << "[PBL2] Nạp dữ liệu các Repositories hoàn tất.\n";

    // ─────────────────────────────────────────────────────────────────────────
    // Bước 2: Khởi tạo tầng Services với ReferenceChecker
    // ─────────────────────────────────────────────────────────────────────────
    ReferenceChecker refChecker(&orderRepo, &invoiceRepo, &stockRepo);
    PhoneService    phoneService(phoneRepo, &refChecker);
    OrderService    orderService(orderRepo, &phoneService);
    CustomerService customerService(customerRepo);
    EmployeeService employeeService(employeeRepo);
    InvoiceService  invoiceService(invoiceRepo, orderRepo);
    SupplierService supplierService(supplierRepo, stockRepo, &phoneService);
    DiscountService discountService(discountRepo);
    ReportService   reportService(orderRepo, invoiceRepo, phoneRepo);
    AuthService     authService(employeeService, customerService);
    std::cout << "[PBL2] Khởi tạo các Core Services thành công.\n";

    // ─────────────────────────────────────────────────────────────────────────
    // Bước 3: Khởi tạo Controller điều phối và AppRunner UI
    // ─────────────────────────────────────────────────────────────────────────
    AppController controller(
        &phoneService,
        &orderService,
        &customerService,
        &employeeService,
        &invoiceService,
        &supplierService,
        &discountService,
        &reportService,
        &authService
    );

    AppRunner appRunner(&controller);
    std::cout << "[PBL2] Giao diện GTK4 đang khởi động...\n";

    // ─────────────────────────────────────────────────────────────────────────
    // Bước 4: Vòng lặp sự kiện GTK4
    // ─────────────────────────────────────────────────────────────────────────
    int status = appRunner.run(argc, argv);

    std::cout << "[PBL2] Ứng dụng kết thúc an toàn. Mã thoát: " << status << "\n";
    return status;
}
