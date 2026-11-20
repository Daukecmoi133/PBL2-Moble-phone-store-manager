// main.cpp — Entry point chính của ứng dụng GTK4 (PBL2 Phone Store Manager)
// Thứ tự khởi tạo: Repositories → Services → AppController → AppRunner →
// g_application_run

#include "UI/AppRunner.h"
#include "UI/FontSetup.h"
#include "source/PBL2.h"
#include "source/controllers/AppController.h"
#include "source/utils/PathHelper.h"
#include "source/utils/TransactionCoordinator.h"
#include "source/utils/AppLock.h"
#include "source/utils/BackupHelper.h"
#include "source/utils/DataMigrator.h"
#include <glib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  // ─────────────────────────────────────────────────────────────────────────
  // Xử lý các cờ dòng lệnh tiện ích: --backup, --verify-backup, --migrate, --help
  // ─────────────────────────────────────────────────────────────────────────
  bool runMigrateCli = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--backup" || arg == "-b") {
      std::string srcData = PathHelper::resolveDataPath("data");
      std::string backupPath, err;
      std::cout << "[BACKUP] Đang sao lưu thư mục dữ liệu: " << srcData << " ...\n";
      if (BackupHelper::createBackup(srcData, "backups", backupPath, err)) {
        std::cout << "[BACKUP THÀNH CÔNG] Đã tạo bản sao lưu tại: " << backupPath << "\n";
        std::cout << "[BACKUP THÀNH CÔNG] Manifest SHA-256: " << backupPath << "/CHECKSUMS.sha256\n";
        return 0;
      } else {
        std::cerr << "[BACKUP THẤT BẠI] " << err << "\n";
        return 1;
      }
    } else if (arg == "--verify-backup") {
      if (i + 1 >= argc) {
        std::cerr << "[LỖI] Thiếu đường dẫn bản sao lưu sau --verify-backup\n";
        return 1;
      }
      std::string backupDir = argv[++i];
      std::string err;
      std::cout << "[VERIFY] Đang kiểm tra tính toàn vẹn bản sao lưu: " << backupDir << " ...\n";
      if (BackupHelper::verifyBackup(backupDir, err)) {
        std::cout << "[VERIFY THÀNH CÔNG] Toàn bộ mã SHA-256 khớp 100%.\n";
        return 0;
      } else {
        std::cerr << "[VERIFY THẤT BẠI] " << err << "\n";
        return 1;
      }
    } else if (arg == "--migrate") {
      runMigrateCli = true;
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "PBL2 Phone Store Manager (GTK4 + C++17)\n"
                << "Cách sử dụng:\n"
                << "  pbl2_gtk4                    Khởi chạy giao diện người dùng GTK4\n"
                << "  pbl2_gtk4 --backup           Sao lưu data/ ra backups/<timestamp>/ kèm SHA-256\n"
                << "  pbl2_gtk4 --verify-backup <dir>  Kiểm tra tính toàn vẹn của bản sao lưu\n"
                << "  pbl2_gtk4 --migrate          Di trú chuẩn hóa dữ liệu lên v"
                << DataMigrator::CURRENT_SCHEMA_VERSION << " (idempotent)\n"
                << "  pbl2_gtk4 --help             Hiển thị thông tin trợ giúp\n";
      return 0;
    }
  }

  if (!runMigrateCli) {
    setupEmojiFont();
  }

  // Khóa ứng dụng đơn phiên bản (Single-Instance Lock)
  AppLock appLock(PathHelper::resolveDataPath(".app.lock"));
  std::string lockErr;
  if (!appLock.acquire(lockErr)) {
    std::cerr << "\n[PBL2 LỖI KHỞI ĐỘNG] " << lockErr << "\n"
              << "Vui lòng đóng tiến trình PBL2 đang chạy trước khi khởi động lại.\n\n";
    return 1;
  }

  if (runMigrateCli) {
    const std::string dataDir = PathHelper::resolveDataPath("data");
    FileTransaction::recoverPendingTransactions(dataDir);
    DataMigrator::MigrationResult mres = DataMigrator::migrateToLatest(dataDir);
    DataMigrator::printReport(mres, std::cout);
    return mres.success ? 0 : 1;
  }

  // BUG-CRITICAL-02: Để GTK4 tự động sử dụng native Win32 IME (GtkIMContextIME)
  // mặc định. Loại bỏ việc tự động ép GTK_IM_MODULE=simple để hỗ trợ gõ phím
  // trực tiếp và bộ gõ tiếng Việt. Chỉ kích hoạt simple IM khi được yêu cầu
  // tường minh qua biến PBL2_FORCE_SIMPLE_IM=1 (debugging).
  const char *forceSimple = g_getenv("PBL2_FORCE_SIMPLE_IM");
  if (forceSimple &&
      (std::string(forceSimple) == "1" || std::string(forceSimple) == "true")) {
    g_setenv("GTK_IM_MODULE", "simple", TRUE);
    std::cout << "[PBL2 Thông báo] Đang kích hoạt GTK_IM_MODULE=simple theo cờ "
                 "PBL2_FORCE_SIMPLE_IM.\n";
  }

  std::cout << "========================================================\n";
  std::cout << "[PBL2] Khởi tạo Phone Store Management System (GTK4)...\n";
  std::cout << "========================================================\n";

  // ─────────────────────────────────────────────────────────────────────────
  // Bước 0: Quét & khôi phục các giao dịch file dở dang nếu ứng dụng crash
  // ─────────────────────────────────────────────────────────────────────────
  const std::string dataRootDir = PathHelper::resolveDataPath("data");
  FileTransaction::recoverPendingTransactions(dataRootDir);
  const int detectedSchemaVersion = DataMigrator::readSchemaVersion(dataRootDir);
  if (detectedSchemaVersion < DataMigrator::CURRENT_SCHEMA_VERSION) {
    std::cout << "[PBL2 Schema] Phát hiện dữ liệu v" << detectedSchemaVersion
              << "; đang đọc tương thích ngược. Chạy 'pbl2_gtk4 --migrate' để chuẩn hóa lên v"
              << DataMigrator::CURRENT_SCHEMA_VERSION << ".\n";
  }

  // ─────────────────────────────────────────────────────────────────────────
  // Bước 1: Khởi tạo Repositories và nạp dữ liệu từ data/
  // ─────────────────────────────────────────────────────────────────────────
  PhoneRepository phoneRepo;
  OrderRepository orderRepo;
  InvoiceRepository invoiceRepo;
  StockReceiptRepository stockRepo;
  CustomerRepository customerRepo;
  EmployeeRepository employeeRepo;
  SupplierRepository supplierRepo;
  DiscountRepository discountRepo;
  ProductImageRepository productImageRepo;
  WarrantyRequestRepository warrantyRepo;

  if (!phoneRepo.loadFromFile()) {
    std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu sản phẩm từ file!\n";
  }
  if (!productImageRepo.loadFromFile()) {
    std::cerr << "[PBL2 Cảnh báo] Không thể nạp ánh xạ ảnh sản phẩm từ file!\n";
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
    std::cerr
        << "[PBL2 Cảnh báo] Không thể nạp dữ liệu nhà cung cấp từ file!\n";
  }
  if (!discountRepo.loadFromFile()) {
    std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu mã giảm giá từ file!\n";
  }
  if (!warrantyRepo.loadFromFile()) {
    std::cerr << "[PBL2 Cảnh báo] Không thể nạp dữ liệu bảo hành từ file!\n";
  }
  std::cout << "[PBL2] Nạp dữ liệu các Repositories hoàn tất.\n";

  // ─────────────────────────────────────────────────────────────────────────
  // Bước 2: Khởi tạo tầng Services với ReferenceChecker
  // ─────────────────────────────────────────────────────────────────────────
  ReferenceChecker refChecker(&orderRepo, &invoiceRepo, &stockRepo, &warrantyRepo);
  PhoneService phoneService(phoneRepo, &refChecker, &productImageRepo);
  OrderService orderService(orderRepo, &phoneService);
  CustomerService customerService(customerRepo, &refChecker);
  EmployeeService employeeService(employeeRepo, &refChecker);
  InvoiceService invoiceService(invoiceRepo, orderRepo);
  SupplierService supplierService(supplierRepo, stockRepo, &phoneService);
  DiscountService discountService(discountRepo, &refChecker);
  orderService.setDiscountService(&discountService);
  ReportService reportService(orderRepo, invoiceRepo, phoneRepo);
  AuthService authService(employeeService, customerService);
  WarrantyService warrantyService(warrantyRepo, orderRepo, invoiceRepo,
                                  phoneRepo, customerRepo);
  std::cout << "[PBL2] Khởi tạo các Core Services thành công.\n";

  // ─────────────────────────────────────────────────────────────────────────
  // Bước 3: Khởi tạo Controller điều phối và AppRunner UI
  // ─────────────────────────────────────────────────────────────────────────
  AppController controller(&phoneService, &orderService, &customerService,
                           &employeeService, &invoiceService, &supplierService,
                           &discountService, &reportService,
                           &authService, &warrantyService);

  AppRunner appRunner(&controller);
  std::cout << "[PBL2] Giao diện GTK4 đang khởi động...\n";

  // ─────────────────────────────────────────────────────────────────────────
  // Bước 4: Vòng lặp sự kiện GTK4
  // ─────────────────────────────────────────────────────────────────────────
  int status = appRunner.run(argc, argv);

  std::cout << "[PBL2] Ứng dụng kết thúc an toàn. Mã thoát: " << status << "\n";
  return status;
}
