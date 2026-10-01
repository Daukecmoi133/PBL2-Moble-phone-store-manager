#include "GtkAdminUI.h"
#include <string>
#include <ctime>

// Định nghĩa các cột cho TreeView
enum {
    COL_ID = 0,
    COL_NAME,
    COL_BRAND,
    COL_PRICE,
    COL_STOCK,
    NUM_COLS
};

GtkAdminUI::GtkAdminUI(PhoneService& pService, OrderService& oService)
    : phoneService(pService), orderService(oService), 
      window(nullptr), treeView(nullptr), listStore(nullptr) {
}

void GtkAdminUI::show(int argc, char *argv[]) {
    // 1. Khởi tạo GTK
    gtk_init(&argc, &argv);

    // 2. Tạo cửa sổ chính
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Admin UI - Phone Store Manager (PBL2)");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 3. Layout chính (Box dọc)
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Label tiêu đề
    GtkWidget *lblTitle = gtk_label_new("<b>DANH SÁCH SẢN PHẨM (KHO HÀNG)</b>");
    gtk_label_set_use_markup(GTK_LABEL(lblTitle), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), lblTitle, FALSE, FALSE, 0);

    // 4. Tạo ListStore (Model)
    listStore = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, 
                                   G_TYPE_STRING, G_TYPE_DOUBLE, G_TYPE_INT);
    
    // 5. Tạo TreeView (View)
    treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));
    g_object_unref(listStore); // TreeView tự quản lý reference

    // Thêm các cột vào TreeView
    const char* col_titles[] = {"Mã SP", "Tên Điện Thoại", "Hãng", "Giá Bán", "Tồn Kho"};
    for (int i = 0; i < NUM_COLS; ++i) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column;
        if (i == COL_PRICE) {
            column = gtk_tree_view_column_new_with_attributes(col_titles[i], renderer, "text", i, NULL);
        } else {
            column = gtk_tree_view_column_new_with_attributes(col_titles[i], renderer, "text", i, NULL);
        }
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);
    }

    // Scroll window cho TreeView
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE); // Mở rộng theo chiều dọc
    gtk_container_add(GTK_CONTAINER(scroll), treeView);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    // 6. Nút chức năng
    GtkWidget *hboxBtn = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hboxBtn, FALSE, FALSE, 0);

    GtkWidget *btnOrder = gtk_button_new_with_label("Test Luồng Giao Dịch (Tạo + Duyệt Đơn)");
    g_signal_connect(btnOrder, "clicked", G_CALLBACK(on_btn_order_clicked), this);
    gtk_box_pack_start(GTK_BOX(hboxBtn), btnOrder, TRUE, TRUE, 0);

    // 7. Load dữ liệu
    loadProductsToStore();

    // 8. Hiển thị
    gtk_widget_show_all(window);
    gtk_main();
}

void GtkAdminUI::loadProductsToStore() {
    gtk_list_store_clear(listStore);
    const Vector<Phone>& phones = phoneService.getAllPhones();
    
    for (int i = 0; i < phones.getSize(); ++i) {
        GtkTreeIter iter;
        gtk_list_store_append(listStore, &iter);
        gtk_list_store_set(listStore, &iter,
                           COL_ID, phones[i].getID().c_str(),
                           COL_NAME, phones[i].getName().c_str(),
                           COL_BRAND, phones[i].getBrand().c_str(),
                           COL_PRICE, phones[i].getSellingPrice(),
                           COL_STOCK, phones[i].getQuantity(),
                           -1);
    }
}

// Hàm Test tạo và duyệt đơn hàng (chứng minh Core Logic hoạt động)
void GtkAdminUI::createSampleOrder() {
    const Vector<Phone>& phones = phoneService.getAllPhones();
    if (phones.getSize() == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE, "Lỗi: Không có sản phẩm nào trong kho!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Tìm một sản phẩm còn hàng
    const Phone* targetPhone = nullptr;
    for (int i = 0; i < phones.getSize(); ++i) {
        if (phones[i].getQuantity() > 0) {
            targetPhone = &phones[i];
            break;
        }
    }

    if (targetPhone == nullptr) {
         GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE, "Lỗi: Toàn bộ sản phẩm đã hết hàng!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // 1. Tạo đơn hàng mới
    Order order;
    // ID sinh ngẫu nhiên theo time
    std::string orderId = "ORD-" + std::to_string(time(nullptr));
    order.setID(orderId);
    order.setCustomerId("CUS-TEST-001");
    order.setStatus(OrderStatus::PENDING);
    
    OrderDetail detail(targetPhone->getID(), targetPhone->getName(), 1, targetPhone->getSellingPrice());
    order.addDetail(detail);

    std::string err;
    // Gọi Service tạo đơn -> sẽ đẩy vào Queue
    if (!orderService.createOrder(order, err)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE, "Tạo đơn thất bại:\n%s", err.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // 2. Duyệt đơn hàng đầu tiên trong Queue
    Order processedOrder;
    if (orderService.processNextPendingOrder(processedOrder, err)) {
        // Trừ kho thành công, trạng thái CONFIRMED
        std::string msg = "Giao dịch THÀNH CÔNG!\n- Đơn hàng: " + processedOrder.getID() + 
                          "\n- Sản phẩm: " + targetPhone->getName() +
                          "\n- Đã trừ 1 sản phẩm khỏi kho.";
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_CLOSE, "%s", msg.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        // Cập nhật lại UI danh sách
        loadProductsToStore();
    } else {
        // Rollback nếu có lỗi
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE, "Duyệt đơn thất bại (Đã Rollback):\n%s", err.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void GtkAdminUI::on_btn_order_clicked(GtkWidget *widget, gpointer data) {
    GtkAdminUI *ui = static_cast<GtkAdminUI*>(data);
    ui->createSampleOrder();
}
