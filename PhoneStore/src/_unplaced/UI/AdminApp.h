// AdminApp.h — Ứng dụng GTK4 Admin UI Production-Ready
// Kiến trúc: GtkApplication + GtkApplicationWindow + GtkHeaderBar
// Sử dụng GtkColumnView + GListStore + GtkSignalListItemFactory cho bảng dữ liệu
// Mọi thao tác Thêm/Sửa/Xóa/Tạo đơn đều gọi qua Service Layer

#ifndef ADMIN_APP_H
#define ADMIN_APP_H

#include <gtk/gtk.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

#include "../source/services/PhoneService.h"
#include "../source/services/OrderService.h"
#include "../source/models/Phone.h"
#include "../source/models/Order.h"
#include "../source/models/Common.h"
#include "../source/core/Vector.h"

// ─────────────────────────────────────────────────────────────────────────────
// GObject wrapper cho Phone (dùng cho GListStore)
// GTK4 yêu cầu items trong GListStore phải là GObject
// ─────────────────────────────────────────────────────────────────────────────
G_BEGIN_DECLS

#define PHONE_ITEM_TYPE (phone_item_get_type())
G_DECLARE_FINAL_TYPE(PhoneItem, phone_item, PHONE, ITEM, GObject)

struct _PhoneItem {
    GObject parent_instance;
    char *id;
    char *name;
    char *brand;
    char *os;
    double selling_price;
    int quantity;
    double import_price;
    char *specs;
};

struct _PhoneItemClass { GObjectClass parent_class; };

G_END_DECLS

// ─────────────────────────────────────────────────────────────────────────────
// GObject wrapper cho Order (dùng cho GListStore bảng đơn hàng)
// ─────────────────────────────────────────────────────────────────────────────
G_BEGIN_DECLS

#define ORDER_ITEM_TYPE (order_item_get_type())
G_DECLARE_FINAL_TYPE(OrderItem, order_item, ORDER, ITEM, GObject)

struct _OrderItem {
    GObject parent_instance;
    char *id;
    char *customer_id;
    char *status;
    char *order_time;
    double total;
};

struct _OrderItemClass { GObjectClass parent_class; };

G_END_DECLS

// ─────────────────────────────────────────────────────────────────────────────
// Lớp AdminApp: Toàn bộ state + GTK widgets của ứng dụng
// ─────────────────────────────────────────────────────────────────────────────
class AdminApp {
public:
    // Services (Core Logic)
    PhoneService* phoneService;
    OrderService* orderService;

    // === Cửa sổ & Layout chính ===
    GtkWidget *window;
    GtkWidget *header_bar;
    GtkWidget *stack;          // GtkStack: chuyển đổi giữa trang Sản phẩm / Đơn hàng
    GtkWidget *switcher_bar;   // GtkStackSwitcher

    // === Trang 1: Quản lý Sản phẩm ===
    GtkWidget *phone_column_view;
    GListStore *phone_store;
    GtkWidget *search_entry;       // Ô tìm kiếm
    GtkWidget *lbl_total_phones;   // Hiển thị tổng số SP
    GtkWidget *lbl_total_stock;    // Tổng tồn kho

    // Form Thêm/Sửa Sản phẩm
    GtkWidget *entry_id;
    GtkWidget *entry_name;
    GtkWidget *entry_brand;
    GtkWidget *entry_os;
    GtkWidget *entry_import_price;
    GtkWidget *entry_sell_price;
    GtkWidget *entry_quantity;
    GtkWidget *entry_specs;
    GtkWidget *btn_add_phone;
    GtkWidget *btn_update_phone;
    GtkWidget *btn_delete_phone;
    GtkWidget *btn_clear_form;

    // === Trang 2: Quản lý Đơn hàng ===
    GtkWidget *order_column_view;
    GListStore *order_store;
    GtkWidget *lbl_revenue;
    GtkWidget *lbl_pending_count;

    // Form Tạo Đơn hàng nhanh
    GtkWidget *entry_order_phone_id;
    GtkWidget *entry_order_qty;
    GtkWidget *entry_order_customer;
    GtkWidget *btn_create_order;
    GtkWidget *btn_process_order;

    // Biến trạng thái
    std::string selected_phone_id;   // ID sản phẩm đang được chọn trong bảng

    // === Phương thức ===
    AdminApp(PhoneService* ps, OrderService* os);

    // Khởi tạo và hiển thị ứng dụng (hàm chính)
    static void activate(GtkApplication *gapp, gpointer data);

    // Build UI
    void build_main_window(GtkApplication *gapp);
    GtkWidget* build_phone_page();
    GtkWidget* build_order_page();

    // Dữ liệu
    void refresh_phone_list();
    void refresh_phone_list_filtered(const std::string& keyword);
    void refresh_order_list();
    void refresh_stats();
    void populate_form_from_selected(PhoneItem *item);
    void clear_phone_form();

    // Helpers
    static std::string format_currency(double amount);
    static std::string format_quantity(int qty);
    static bool parse_double(const char* text, double& out);
    static bool parse_int(const char* text, int& out);

    // Dialog thông báo (thay cho console print)
    void show_dialog(const std::string& title, const std::string& message,
                     GtkMessageType type = GTK_MESSAGE_INFO);

    // === Callbacks (static, nhận AdminApp* qua gpointer) ===
    static void on_btn_add_clicked(GtkButton *btn, gpointer data);
    static void on_btn_update_clicked(GtkButton *btn, gpointer data);
    static void on_btn_delete_clicked(GtkButton *btn, gpointer data);
    static void on_btn_clear_clicked(GtkButton *btn, gpointer data);
    static void on_search_changed(GtkSearchEntry *entry, gpointer data);
    static void on_phone_selected(GtkSelectionModel *model, guint pos, guint n, gpointer data);
    static void on_btn_create_order_clicked(GtkButton *btn, gpointer data);
    static void on_btn_process_order_clicked(GtkButton *btn, gpointer data);
    static void on_btn_refresh_clicked(GtkButton *btn, gpointer data);
};

#endif // ADMIN_APP_H
