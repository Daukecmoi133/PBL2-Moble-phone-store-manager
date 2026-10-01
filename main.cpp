#include "source/PBL2.h"
#include "UI/GtkAdminUI.h"
#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "Khởi tạo hệ thống..." << std::endl;

    // 1. Khởi tạo tầng Repositories
    PhoneRepository phoneRepo;
    OrderRepository orderRepo;
    // Khởi tạo các repository khác (nếu cần cho validation)
    CustomerRepository customerRepo;
    InvoiceRepository invoiceRepo;
    StockReceiptRepository stockReceiptRepo;

    // 2. Khởi tạo ReferenceChecker (Toàn vẹn dữ liệu)
    ReferenceChecker refChecker(&orderRepo, &invoiceRepo, &stockReceiptRepo);

    // 3. Khởi tạo tầng Services
    PhoneService phoneService(phoneRepo, &refChecker);
    OrderService orderService(orderRepo, &phoneService);

    std::cout << "Tải dữ liệu thành công. Bắt đầu GTK UI..." << std::endl;

    // 4. Khởi chạy Giao diện
    GtkAdminUI ui(phoneService, orderService);
    ui.show(argc, argv);

    return 0;
}
