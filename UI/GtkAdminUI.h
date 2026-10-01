#ifndef GTK_ADMIN_UI_H
#define GTK_ADMIN_UI_H

#include <gtk/gtk.h>
#include "../source/services/PhoneService.h"
#include "../source/services/OrderService.h"

// Lớp giao diện Admin sử dụng GTK3
class GtkAdminUI {
private:
    PhoneService& phoneService;
    OrderService& orderService;
    
    GtkWidget *window;
    GtkWidget *treeView;
    GtkListStore *listStore;
    
    // Callback khi nhấn nút Test tạo đơn
    static void on_btn_order_clicked(GtkWidget *widget, gpointer data);
    
    // Tải danh sách điện thoại lên TreeView
    void loadProductsToStore();
    
    // Luồng nghiệp vụ tạo đơn và duyệt đơn
    void createSampleOrder();

public:
    GtkAdminUI(PhoneService& pService, OrderService& oService);
    
    // Khởi tạo và hiển thị cửa sổ
    void show(int argc, char *argv[]);
};

#endif // GTK_ADMIN_UI_H
