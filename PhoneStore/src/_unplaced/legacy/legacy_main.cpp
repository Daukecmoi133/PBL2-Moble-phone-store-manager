// main.cpp — Entry point chính của ứng dụng GTK4 Admin (PBL2 Phone Store Manager)
// Thứ tự khởi tạo: Repository → Service → AdminApp → gtk_application_run

#include <gtk/gtk.h>
#include <iostream>
#include "source/repositories/PhoneRepository.h"
#include "source/repositories/OrderRepository.h"
#include "source/repositories/InvoiceRepository.h"
#include "source/repositories/StockReceiptRepository.h"
#include "source/services/PhoneService.h"
#include "source/services/OrderService.h"
#include "UI/AdminApp.h"

int main(int argc, char *argv[]) {
    std::cout << "[PBL2] Khởi tạo hệ thống..." << std::endl;

    // Bước 1: Repositories (tự nạp dữ liệu từ thư mục data/)
    PhoneRepository        phoneRepo;
    OrderRepository        orderRepo;
    InvoiceRepository      invoiceRepo;
    StockReceiptRepository stockRepo;
    std::cout << "[PBL2] Repositories khởi tạo xong." << std::endl;

    // Bước 2: Services
    PhoneService phoneService(phoneRepo);
    OrderService orderService(orderRepo, &phoneService);
    std::cout << "[PBL2] Services khởi tạo xong. Đang mở GTK4 UI..." << std::endl;

    // Bước 3: GtkApplication + AdminApp
    AdminApp admin(&phoneService, &orderService);

    GtkApplication *gapp = gtk_application_new(
        "vn.edu.dut.pbl2.phonestore", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gapp, "activate", G_CALLBACK(AdminApp::activate), &admin);

    // Bước 4: Vòng lặp sự kiện GTK
    int status = g_application_run(G_APPLICATION(gapp), argc, argv);

    // Bước 5: Dọn dẹp (đối tượng stack tự hủy theo thứ tự ngược lại)
    g_object_unref(gapp);
    std::cout << "[PBL2] Ứng dụng đóng sạch. Thoát." << std::endl;
    return status;
}
