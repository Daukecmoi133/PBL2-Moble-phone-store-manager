// AdminApp.cpp — Implementation đầy đủ GTK4 Admin UI
// GTK4 GObject type registration, widget construction, callbacks và data binding

#include "AdminApp.h"
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

// ─────────────────────────────────────────────────────────────────────────────
// GObject PhoneItem — Implementation
// ─────────────────────────────────────────────────────────────────────────────
G_DEFINE_TYPE(PhoneItem, phone_item, G_TYPE_OBJECT)

static void phone_item_finalize(GObject *obj) {
    PhoneItem *self = PHONE_ITEM(obj);
    g_free(self->id);
    g_free(self->name);
    g_free(self->brand);
    g_free(self->os);
    g_free(self->specs);
    G_OBJECT_CLASS(phone_item_parent_class)->finalize(obj);
}

static void phone_item_class_init(PhoneItemClass *klass) {
    G_OBJECT_CLASS(klass)->finalize = phone_item_finalize;
}

static void phone_item_init(PhoneItem *self) {
    self->id = g_strdup("");
    self->name = g_strdup("");
    self->brand = g_strdup("");
    self->os = g_strdup("");
    self->specs = g_strdup("");
    self->selling_price = 0.0;
    self->import_price = 0.0;
    self->quantity = 0;
}

static PhoneItem* phone_item_new(const Phone& p) {
    PhoneItem *item = PHONE_ITEM(g_object_new(PHONE_ITEM_TYPE, nullptr));
    g_free(item->id);   item->id   = g_strdup(p.getID().c_str());
    g_free(item->name); item->name = g_strdup(p.getName().c_str());
    g_free(item->brand);item->brand= g_strdup(p.getBrand().c_str());
    g_free(item->os);   item->os   = g_strdup(p.getOperatingSystem().c_str());
    g_free(item->specs);item->specs= g_strdup(p.getSpecifications().c_str());
    item->selling_price = p.getSellingPrice();
    item->import_price  = p.getImportPrice();
    item->quantity      = p.getQuantity();
    return item;
}

// ─────────────────────────────────────────────────────────────────────────────
// GObject OrderItem — Implementation
// ─────────────────────────────────────────────────────────────────────────────
G_DEFINE_TYPE(OrderItem, order_item, G_TYPE_OBJECT)

static void order_item_finalize(GObject *obj) {
    OrderItem *self = ORDER_ITEM(obj);
    g_free(self->id);
    g_free(self->customer_id);
    g_free(self->status);
    g_free(self->order_time);
    G_OBJECT_CLASS(order_item_parent_class)->finalize(obj);
}

static void order_item_class_init(OrderItemClass *klass) {
    G_OBJECT_CLASS(klass)->finalize = order_item_finalize;
}

static void order_item_init(OrderItem *self) {
    self->id          = g_strdup("");
    self->customer_id = g_strdup("");
    self->status      = g_strdup("");
    self->order_time  = g_strdup("");
    self->total       = 0.0;
}

static OrderItem* order_item_new(const Order& o) {
    OrderItem *item = ORDER_ITEM(g_object_new(ORDER_ITEM_TYPE, nullptr));
    g_free(item->id);           item->id          = g_strdup(o.getID().c_str());
    g_free(item->customer_id);  item->customer_id = g_strdup(o.getCustomerId().c_str());
    g_free(item->status);       item->status      = g_strdup(orderStatusToString(o.getStatus()).c_str());
    // Tạo chuỗi ngày giờ đơn giản
    char buf[32];
    const DateTime& dt = o.getOrderTime();
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d %02d:%02d",
             dt.day, dt.month, dt.year, dt.hour, dt.minute);
    g_free(item->order_time); item->order_time = g_strdup(buf);
    item->total = o.calculateTotal();
    return item;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: Factory setup cho từng cột GtkColumnView
// ─────────────────────────────────────────────────────────────────────────────

// Cấu trúc truyền data cho factory callback
struct ColSetupData {
    const char* field; // tên trường để phân biệt cột
};

// Factory bind callback cho Phone table
static void phone_col_bind(GtkSignalListItemFactory *factory,
                           GtkListItem *list_item, gpointer field_ptr) {
    const char* field = (const char*)field_ptr;
    GtkWidget *label = gtk_list_item_get_child(list_item);
    PhoneItem *item  = PHONE_ITEM(gtk_list_item_get_item(list_item));
    if (!item) return;

    char buf[128];
    if (strcmp(field, "id") == 0)
        gtk_label_set_text(GTK_LABEL(label), item->id);
    else if (strcmp(field, "name") == 0)
        gtk_label_set_text(GTK_LABEL(label), item->name);
    else if (strcmp(field, "brand") == 0)
        gtk_label_set_text(GTK_LABEL(label), item->brand);
    else if (strcmp(field, "price") == 0) {
        // Format tiền tệ với dấu phẩy phân cách hàng nghìn
        snprintf(buf, sizeof(buf), "%.0f đ", item->selling_price);
        // Thêm dấu phẩy thủ công
        std::string s(buf);
        std::string num = "";
        std::string suffix = " đ";
        size_t suf = s.find(' ');
        if (suf != std::string::npos) {
            num = s.substr(0, suf);
            suffix = s.substr(suf);
        } else num = s;
        // Insert commas
        int n = (int)num.size();
        std::string formatted = "";
        for (int i = 0; i < n; i++) {
            if (i > 0 && (n - i) % 3 == 0) formatted += ",";
            formatted += num[i];
        }
        gtk_label_set_text(GTK_LABEL(label), (formatted + suffix).c_str());
    }
    else if (strcmp(field, "qty") == 0) {
        snprintf(buf, sizeof(buf), "%d", item->quantity);
        // Màu đỏ nếu hết hàng
        if (item->quantity == 0) {
            std::string markup = "<span foreground='red'><b>HẾT</b></span>";
            gtk_label_set_markup(GTK_LABEL(label), markup.c_str());
        } else if (item->quantity <= 5) {
            std::string markup = "<span foreground='orange'>" + std::string(buf) + "</span>";
            gtk_label_set_markup(GTK_LABEL(label), markup.c_str());
        } else {
            gtk_label_set_text(GTK_LABEL(label), buf);
        }
    }
}

static void phone_col_setup(GtkSignalListItemFactory *factory,
                            GtkListItem *list_item, gpointer data) {
    GtkWidget *label = gtk_label_new(nullptr);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_margin_end(label, 8);
    gtk_list_item_set_child(list_item, label);
}

// Factory bind callback cho Order table
static void order_col_bind(GtkSignalListItemFactory *factory,
                           GtkListItem *list_item, gpointer field_ptr) {
    const char* field = (const char*)field_ptr;
    GtkWidget *label = gtk_list_item_get_child(list_item);
    OrderItem *item  = ORDER_ITEM(gtk_list_item_get_item(list_item));
    if (!item) return;

    char buf[64];
    if (strcmp(field, "id") == 0)
        gtk_label_set_text(GTK_LABEL(label), item->id);
    else if (strcmp(field, "customer") == 0)
        gtk_label_set_text(GTK_LABEL(label), item->customer_id);
    else if (strcmp(field, "status") == 0) {
        std::string s(item->status);
        std::string markup;
        if (s == "Chờ xử lý")
            markup = "<span foreground='orange'>" + s + "</span>";
        else if (s == "Đã xác nhận" || s == "Hoàn thành")
            markup = "<span foreground='#2ecc71'><b>" + s + "</b></span>";
        else if (s == "Đã hủy")
            markup = "<span foreground='red'>" + s + "</span>";
        else
            markup = s;
        gtk_label_set_markup(GTK_LABEL(label), markup.c_str());
    }
    else if (strcmp(field, "time") == 0)
        gtk_label_set_text(GTK_LABEL(label), item->order_time);
    else if (strcmp(field, "total") == 0) {
        snprintf(buf, sizeof(buf), "%.0f d", item->total);
        std::string s(buf);
        gtk_label_set_text(GTK_LABEL(label), s.c_str());
    }
}

static void order_col_setup(GtkSignalListItemFactory *factory,
                            GtkListItem *list_item, gpointer data) {
    GtkWidget *label = gtk_label_new(nullptr);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_margin_end(label, 8);
    gtk_list_item_set_child(list_item, label);
}

// Tạo một GtkColumnViewColumn với factory
static GtkColumnViewColumn* make_phone_column(const char* title, const char* field) {
    GtkSignalListItemFactory *factory = GTK_SIGNAL_LIST_ITEM_FACTORY(
        gtk_signal_list_item_factory_new());
    g_signal_connect(factory, "setup", G_CALLBACK(phone_col_setup), nullptr);
    g_signal_connect(factory, "bind",  G_CALLBACK(phone_col_bind),  (gpointer)field);
    GtkColumnViewColumn *col = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));
    gtk_column_view_column_set_resizable(col, TRUE);
    return col;
}

static GtkColumnViewColumn* make_order_column(const char* title, const char* field) {
    GtkSignalListItemFactory *factory = GTK_SIGNAL_LIST_ITEM_FACTORY(
        gtk_signal_list_item_factory_new());
    g_signal_connect(factory, "setup", G_CALLBACK(order_col_setup), nullptr);
    g_signal_connect(factory, "bind",  G_CALLBACK(order_col_bind),  (gpointer)field);
    GtkColumnViewColumn *col = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));
    gtk_column_view_column_set_resizable(col, TRUE);
    return col;
}

// ─────────────────────────────────────────────────────────────────────────────
// AdminApp — Constructor & activate
// ─────────────────────────────────────────────────────────────────────────────
AdminApp::AdminApp(PhoneService* ps, OrderService* os)
    : phoneService(ps), orderService(os),
      phone_store(nullptr), order_store(nullptr) {}

void AdminApp::activate(GtkApplication *gapp, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);
    self->build_main_window(gapp);
    self->refresh_phone_list();
    self->refresh_order_list();
    self->refresh_stats();
    gtk_window_present(GTK_WINDOW(self->window));
}

// ─────────────────────────────────────────────────────────────────────────────
// Build Main Window
// ─────────────────────────────────────────────────────────────────────────────
void AdminApp::build_main_window(GtkApplication *gapp) {
    window = gtk_application_window_new(gapp);
    gtk_window_set_title(GTK_WINDOW(window), "Phone Store Manager — Admin Panel");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 750);

    // ── Header Bar ──────────────────────────────────────────────────────────
    header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

    // Nút Refresh trên header
    GtkWidget *btn_refresh = gtk_button_new_from_icon_name("view-refresh-symbolic");
    gtk_widget_set_tooltip_text(btn_refresh, "Làm mới dữ liệu");
    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(on_btn_refresh_clicked), this);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), btn_refresh);

    // Stack switcher trên header (chuyển tab Sản phẩm / Đơn hàng)
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack),
                                  GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 220);

    switcher_bar = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher_bar), GTK_STACK(stack));
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header_bar), switcher_bar);

    // ── CSS styling ─────────────────────────────────────────────────────────
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css,
        "columnview { font-size: 13px; }"
        "columnview row:selected { background: #3584e4; color: white; }"
        "columnview row:hover { background: alpha(#3584e4, 0.15); }"
        ".form-section { border: 1px solid alpha(gray, 0.3); border-radius: 8px; padding: 12px; }"
        ".stat-value { font-size: 20px; font-weight: bold; color: #3584e4; }"
        ".stat-label { font-size: 11px; color: alpha(black, 0.5); }"
        ".section-title { font-size: 14px; font-weight: bold; margin-bottom: 6px; }"
        ".warning-label { color: #e67e22; font-weight: bold; }"
        "entry.error { border-color: #e01b24; }"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    // ── Pages ────────────────────────────────────────────────────────────────
    GtkWidget *phone_page = build_phone_page();
    GtkWidget *order_page = build_order_page();

    gtk_stack_add_titled(GTK_STACK(stack), phone_page, "phones",  "📦 Sản Phẩm");
    gtk_stack_add_titled(GTK_STACK(stack), order_page, "orders",  "🛒 Đơn Hàng");

    gtk_window_set_child(GTK_WINDOW(window), stack);
}

// ─────────────────────────────────────────────────────────────────────────────
// Build trang Quản lý Sản phẩm
// ─────────────────────────────────────────────────────────────────────────────
GtkWidget* AdminApp::build_phone_page() {
    // Paned: bảng trái | form phải
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_start(paned, 12);
    gtk_widget_set_margin_end(paned, 12);
    gtk_widget_set_margin_top(paned, 8);
    gtk_widget_set_margin_bottom(paned, 12);

    // ── Bên TRÁI: Bảng danh sách sản phẩm ──────────────────────────────────
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    // Thanh tìm kiếm
    search_entry = gtk_search_entry_new();
    gtk_search_entry_set_placeholder_text(GTK_SEARCH_ENTRY(search_entry),
                                          "🔍  Tìm theo tên sản phẩm...");
    g_signal_connect(search_entry, "search-changed",
                     G_CALLBACK(on_search_changed), this);
    gtk_box_append(GTK_BOX(left_box), search_entry);

    // Stats bar
    GtkWidget *stats_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_margin_top(stats_bar, 4);

    lbl_total_phones = gtk_label_new("0");
    gtk_widget_add_css_class(lbl_total_phones, "stat-value");
    GtkWidget *lbl_p = gtk_label_new("Sản phẩm");
    gtk_widget_add_css_class(lbl_p, "stat-label");
    GtkWidget *vb1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_append(GTK_BOX(vb1), lbl_total_phones);
    gtk_box_append(GTK_BOX(vb1), lbl_p);

    lbl_total_stock = gtk_label_new("0");
    gtk_widget_add_css_class(lbl_total_stock, "stat-value");
    GtkWidget *lbl_s = gtk_label_new("Tổng tồn kho");
    gtk_widget_add_css_class(lbl_s, "stat-label");
    GtkWidget *vb2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_append(GTK_BOX(vb2), lbl_total_stock);
    gtk_box_append(GTK_BOX(vb2), lbl_s);

    gtk_box_append(GTK_BOX(stats_bar), vb1);
    gtk_box_append(GTK_BOX(stats_bar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
    gtk_box_append(GTK_BOX(stats_bar), vb2);
    gtk_box_append(GTK_BOX(left_box), stats_bar);

    // GtkColumnView
    phone_store = g_list_store_new(PHONE_ITEM_TYPE);
    GtkSingleSelection *selection = gtk_single_selection_new(G_LIST_MODEL(phone_store));
    g_signal_connect(selection, "selection-changed",
                     G_CALLBACK(on_phone_selected), this);

    phone_column_view = gtk_column_view_new(GTK_SELECTION_MODEL(selection));
    gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(phone_column_view), TRUE);
    gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(phone_column_view), TRUE);

    gtk_column_view_append_column(GTK_COLUMN_VIEW(phone_column_view),
                                  make_phone_column("Mã SP",     "id"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(phone_column_view),
                                  make_phone_column("Tên",       "name"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(phone_column_view),
                                  make_phone_column("Hãng",      "brand"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(phone_column_view),
                                  make_phone_column("Giá Bán",   "price"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(phone_column_view),
                                  make_phone_column("Tồn Kho",   "qty"));

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), phone_column_view);
    gtk_box_append(GTK_BOX(left_box), scroll);

    // ── Bên PHẢI: Form Thêm/Sửa ─────────────────────────────────────────────
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(right_box, 12);

    GtkWidget *form_title = gtk_label_new("THÔNG TIN SẢN PHẨM");
    gtk_widget_add_css_class(form_title, "section-title");
    gtk_label_set_xalign(GTK_LABEL(form_title), 0.0f);
    gtk_box_append(GTK_BOX(right_box), form_title);

    // Tạo form grid
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_add_css_class(grid, "form-section");

    auto add_form_row = [&](int row, const char* label_text, GtkWidget*& entry_ref,
                             const char* placeholder, bool mandatory = false) {
        std::string lbl_str = mandatory ? std::string(label_text) + " *" : label_text;
        GtkWidget *lbl = gtk_label_new(lbl_str.c_str());
        gtk_label_set_xalign(GTK_LABEL(lbl), 1.0f);
        gtk_widget_set_hexpand(lbl, FALSE);
        entry_ref = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry_ref), placeholder);
        gtk_widget_set_hexpand(entry_ref, TRUE);
        gtk_grid_attach(GTK_GRID(grid), lbl,       0, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), entry_ref, 1, row, 1, 1);
    };

    add_form_row(0, "Mã SP",       entry_id,           "VD: SP-001",       true);
    add_form_row(1, "Tên SP",      entry_name,         "VD: iPhone 15 Pro",true);
    add_form_row(2, "Hãng",        entry_brand,        "VD: Apple",        true);
    add_form_row(3, "Hệ điều hành",entry_os,           "VD: iOS 17");
    add_form_row(4, "Giá nhập (đ)",entry_import_price, "VD: 20000000",     true);
    add_form_row(5, "Giá bán (đ)", entry_sell_price,   "VD: 25000000",     true);
    add_form_row(6, "Tồn kho",     entry_quantity,     "VD: 10",           true);
    add_form_row(7, "Thông số",    entry_specs,        "VD: RAM 8GB...");

    GtkWidget *note = gtk_label_new("(*) Trường bắt buộc");
    gtk_label_set_xalign(GTK_LABEL(note), 0.0f);
    gtk_widget_add_css_class(note, "stat-label");
    gtk_grid_attach(GTK_GRID(grid), note, 0, 8, 2, 1);

    gtk_box_append(GTK_BOX(right_box), grid);

    // Nút hành động
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_top(btn_box, 6);

    btn_add_phone    = gtk_button_new_with_label("➕  Thêm Sản Phẩm");
    btn_update_phone = gtk_button_new_with_label("✏️  Cập Nhật Sản Phẩm");
    btn_delete_phone = gtk_button_new_with_label("🗑️  Xóa Sản Phẩm");
    btn_clear_form   = gtk_button_new_with_label("🔄  Làm Sạch Form");

    gtk_widget_add_css_class(btn_add_phone,    "suggested-action");
    gtk_widget_add_css_class(btn_delete_phone, "destructive-action");

    g_signal_connect(btn_add_phone,    "clicked", G_CALLBACK(on_btn_add_clicked),    this);
    g_signal_connect(btn_update_phone, "clicked", G_CALLBACK(on_btn_update_clicked), this);
    g_signal_connect(btn_delete_phone, "clicked", G_CALLBACK(on_btn_delete_clicked), this);
    g_signal_connect(btn_clear_form,   "clicked", G_CALLBACK(on_btn_clear_clicked),  this);

    gtk_box_append(GTK_BOX(btn_box), btn_add_phone);
    gtk_box_append(GTK_BOX(btn_box), btn_update_phone);
    gtk_box_append(GTK_BOX(btn_box), btn_delete_phone);
    gtk_box_append(GTK_BOX(btn_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    gtk_box_append(GTK_BOX(btn_box), btn_clear_form);
    gtk_box_append(GTK_BOX(right_box), btn_box);

    // Gắn vào paned
    gtk_paned_set_start_child(GTK_PANED(paned), left_box);
    gtk_paned_set_end_child(GTK_PANED(paned), right_box);
    gtk_paned_set_position(GTK_PANED(paned), 640);
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);

    return paned;
}

// ─────────────────────────────────────────────────────────────────────────────
// Build trang Quản lý Đơn hàng
// ─────────────────────────────────────────────────────────────────────────────
GtkWidget* AdminApp::build_order_page() {
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_start(paned, 12);
    gtk_widget_set_margin_end(paned, 12);
    gtk_widget_set_margin_top(paned, 8);
    gtk_widget_set_margin_bottom(paned, 12);

    // ── Bên TRÁI: Bảng đơn hàng ─────────────────────────────────────────────
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    // Stats
    GtkWidget *stats = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);

    lbl_revenue = gtk_label_new("0 đ");
    gtk_widget_add_css_class(lbl_revenue, "stat-value");
    GtkWidget *lbl_r = gtk_label_new("Doanh Thu");
    gtk_widget_add_css_class(lbl_r, "stat-label");
    GtkWidget *vb_rev = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_append(GTK_BOX(vb_rev), lbl_revenue);
    gtk_box_append(GTK_BOX(vb_rev), lbl_r);

    lbl_pending_count = gtk_label_new("0");
    gtk_widget_add_css_class(lbl_pending_count, "stat-value");
    GtkWidget *lbl_q = gtk_label_new("Đơn Chờ Duyệt");
    gtk_widget_add_css_class(lbl_q, "warning-label");
    GtkWidget *vb_pend = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_append(GTK_BOX(vb_pend), lbl_pending_count);
    gtk_box_append(GTK_BOX(vb_pend), lbl_q);

    gtk_box_append(GTK_BOX(stats), vb_rev);
    gtk_box_append(GTK_BOX(stats), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
    gtk_box_append(GTK_BOX(stats), vb_pend);
    gtk_box_append(GTK_BOX(left_box), stats);

    // GtkColumnView cho đơn hàng
    order_store = g_list_store_new(ORDER_ITEM_TYPE);
    GtkNoSelection *no_sel = gtk_no_selection_new(G_LIST_MODEL(order_store));
    order_column_view = gtk_column_view_new(GTK_SELECTION_MODEL(no_sel));
    gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(order_column_view), TRUE);
    gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(order_column_view), TRUE);

    gtk_column_view_append_column(GTK_COLUMN_VIEW(order_column_view),
                                  make_order_column("Mã ĐH",       "id"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(order_column_view),
                                  make_order_column("Khách Hàng",  "customer"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(order_column_view),
                                  make_order_column("Trạng Thái",  "status"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(order_column_view),
                                  make_order_column("Thời Gian",   "time"));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(order_column_view),
                                  make_order_column("Tổng Tiền",   "total"));

    GtkWidget *scroll2 = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll2, TRUE);
    gtk_widget_set_hexpand(scroll2, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll2), order_column_view);
    gtk_box_append(GTK_BOX(left_box), scroll2);

    // ── Bên PHẢI: Form Tạo Đơn hàng + Duyệt ────────────────────────────────
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(right_box, 12);

    GtkWidget *title2 = gtk_label_new("TẠO ĐƠN HÀNG NHANH");
    gtk_widget_add_css_class(title2, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title2), 0.0f);
    gtk_box_append(GTK_BOX(right_box), title2);

    GtkWidget *grid2 = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid2), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid2), 10);
    gtk_widget_add_css_class(grid2, "form-section");

    auto add_order_row = [&](int row, const char* lbl_text, GtkWidget*& entry_ref,
                              const char* placeholder) {
        GtkWidget *lbl = gtk_label_new(lbl_text);
        gtk_label_set_xalign(GTK_LABEL(lbl), 1.0f);
        entry_ref = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry_ref), placeholder);
        gtk_widget_set_hexpand(entry_ref, TRUE);
        gtk_grid_attach(GTK_GRID(grid2), lbl,       0, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid2), entry_ref, 1, row, 1, 1);
    };

    add_order_row(0, "Mã KH *",    entry_order_customer, "VD: KH-001");
    add_order_row(1, "Mã SP *",    entry_order_phone_id, "VD: SP-001");
    add_order_row(2, "Số lượng *", entry_order_qty,      "VD: 1");

    gtk_box_append(GTK_BOX(right_box), grid2);

    btn_create_order  = gtk_button_new_with_label("📝  Tạo Đơn Hàng Mới");
    btn_process_order = gtk_button_new_with_label("✅  Duyệt Đơn Tiếp Theo (FIFO)");

    gtk_widget_add_css_class(btn_create_order,  "suggested-action");
    gtk_widget_add_css_class(btn_process_order, "suggested-action");

    g_signal_connect(btn_create_order,  "clicked", G_CALLBACK(on_btn_create_order_clicked),  this);
    g_signal_connect(btn_process_order, "clicked", G_CALLBACK(on_btn_process_order_clicked), this);

    GtkWidget *info = gtk_label_new(
        "ℹ️ Duyệt đơn sẽ tự động kiểm tra\n"
        "tồn kho và trừ số lượng tương ứng.\n"
        "Đơn hàng được xử lý theo FIFO Queue.");
    gtk_label_set_wrap(GTK_LABEL(info), TRUE);
    gtk_widget_add_css_class(info, "stat-label");
    gtk_label_set_xalign(GTK_LABEL(info), 0.0f);
    gtk_widget_set_margin_top(info, 8);

    gtk_box_append(GTK_BOX(right_box), btn_create_order);
    gtk_box_append(GTK_BOX(right_box), btn_process_order);
    gtk_box_append(GTK_BOX(right_box), info);

    gtk_paned_set_start_child(GTK_PANED(paned), left_box);
    gtk_paned_set_end_child(GTK_PANED(paned), right_box);
    gtk_paned_set_position(GTK_PANED(paned), 650);
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);

    return paned;
}

// ─────────────────────────────────────────────────────────────────────────────
// Data Refresh Methods
// ─────────────────────────────────────────────────────────────────────────────
void AdminApp::refresh_phone_list() {
    g_list_store_remove_all(phone_store);
    const Vector<Phone>& phones = phoneService->getAllPhones();
    for (int i = 0; i < phones.getSize(); ++i) {
        PhoneItem *item = phone_item_new(phones[i]);
        g_list_store_append(phone_store, item);
        g_object_unref(item);
    }
    refresh_stats();
}

void AdminApp::refresh_phone_list_filtered(const std::string& keyword) {
    g_list_store_remove_all(phone_store);
    Vector<Phone> results = phoneService->searchByName(keyword);
    for (int i = 0; i < results.getSize(); ++i) {
        PhoneItem *item = phone_item_new(results[i]);
        g_list_store_append(phone_store, item);
        g_object_unref(item);
    }
}

void AdminApp::refresh_order_list() {
    g_list_store_remove_all(order_store);
    const Vector<Order>& orders = orderService->getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
        OrderItem *item = order_item_new(orders[i]);
        g_list_store_append(order_store, item);
        g_object_unref(item);
    }
    refresh_stats();
}

void AdminApp::refresh_stats() {
    // Cập nhật stats trang sản phẩm
    if (lbl_total_phones) {
        int cnt = phoneService->getAllPhones().getSize();
        gtk_label_set_text(GTK_LABEL(lbl_total_phones), std::to_string(cnt).c_str());
    }
    if (lbl_total_stock) {
        int total = 0;
        const Vector<Phone>& ps = phoneService->getAllPhones();
        for (int i = 0; i < ps.getSize(); ++i) total += ps[i].getQuantity();
        gtk_label_set_text(GTK_LABEL(lbl_total_stock), std::to_string(total).c_str());
    }
    // Cập nhật stats trang đơn hàng
    if (lbl_revenue) {
        double rev = orderService->calculateTotalRevenue();
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << rev << " d";
        gtk_label_set_text(GTK_LABEL(lbl_revenue), ss.str().c_str());
    }
    if (lbl_pending_count) {
        // Đếm đơn PENDING
        int cnt = 0;
        const Vector<Order>& orders = orderService->getAllOrders();
        for (int i = 0; i < orders.getSize(); ++i)
            if (orders[i].getStatus() == OrderStatus::PENDING) ++cnt;
        gtk_label_set_text(GTK_LABEL(lbl_pending_count), std::to_string(cnt).c_str());
    }
}

void AdminApp::populate_form_from_selected(PhoneItem *item) {
    gtk_editable_set_text(GTK_EDITABLE(entry_id),           item->id);
    gtk_editable_set_text(GTK_EDITABLE(entry_name),         item->name);
    gtk_editable_set_text(GTK_EDITABLE(entry_brand),        item->brand);
    gtk_editable_set_text(GTK_EDITABLE(entry_os),           item->os);
    gtk_editable_set_text(GTK_EDITABLE(entry_specs),        item->specs);

    char buf[64];
    snprintf(buf, sizeof(buf), "%.0f", item->import_price);
    gtk_editable_set_text(GTK_EDITABLE(entry_import_price), buf);
    snprintf(buf, sizeof(buf), "%.0f", item->selling_price);
    gtk_editable_set_text(GTK_EDITABLE(entry_sell_price), buf);
    snprintf(buf, sizeof(buf), "%d", item->quantity);
    gtk_editable_set_text(GTK_EDITABLE(entry_quantity), buf);

    selected_phone_id = item->id;
}

void AdminApp::clear_phone_form() {
    gtk_editable_set_text(GTK_EDITABLE(entry_id),           "");
    gtk_editable_set_text(GTK_EDITABLE(entry_name),         "");
    gtk_editable_set_text(GTK_EDITABLE(entry_brand),        "");
    gtk_editable_set_text(GTK_EDITABLE(entry_os),           "");
    gtk_editable_set_text(GTK_EDITABLE(entry_import_price), "");
    gtk_editable_set_text(GTK_EDITABLE(entry_sell_price),   "");
    gtk_editable_set_text(GTK_EDITABLE(entry_quantity),     "");
    gtk_editable_set_text(GTK_EDITABLE(entry_specs),        "");
    selected_phone_id = "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: Format currency
// ─────────────────────────────────────────────────────────────────────────────
std::string AdminApp::format_currency(double amount) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(0) << amount;
    return ss.str() + " d";
}

bool AdminApp::parse_double(const char* text, double& out) {
    if (!text || text[0] == '\0') return false;
    try { out = std::stod(text); return true; }
    catch (...) { return false; }
}

bool AdminApp::parse_int(const char* text, int& out) {
    if (!text || text[0] == '\0') return false;
    try { out = std::stoi(text); return true; }
    catch (...) { return false; }
}

// ─────────────────────────────────────────────────────────────────────────────
// Dialog thông báo dùng GtkAlertDialog (GTK 4.10+) hoặc GtkMessageDialog
// ─────────────────────────────────────────────────────────────────────────────
void AdminApp::show_dialog(const std::string& title, const std::string& message,
                           GtkMessageType type) {
    // GTK 4.10+ dùng GtkAlertDialog, fallback về GtkMessageDialog cho 4.x cũ
#if GTK_CHECK_VERSION(4, 10, 0)
    GtkAlertDialog *dlg = gtk_alert_dialog_new("%s", title.c_str());
    gtk_alert_dialog_set_detail(dlg, message.c_str());
    static const char* ok_btns[] = {"OK", nullptr};
    gtk_alert_dialog_set_buttons(dlg, ok_btns);
    gtk_alert_dialog_set_default_button(dlg, 0);
    gtk_alert_dialog_show(dlg, GTK_WINDOW(window));
    g_object_unref(dlg);
#else
    GtkWidget *dlg = gtk_message_dialog_new(
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        type,
        GTK_BUTTONS_OK,
        "%s", message.c_str());
    gtk_window_set_title(GTK_WINDOW(dlg), title.c_str());
    g_signal_connect(dlg, "response", G_CALLBACK(gtk_window_destroy), nullptr);
    gtk_widget_show(dlg);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// CALLBACKS
// ─────────────────────────────────────────────────────────────────────────────

// Khi chọn dòng trong bảng sản phẩm → điền form
void AdminApp::on_phone_selected(GtkSelectionModel *model, guint pos, guint n, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);
    gpointer raw = g_list_model_get_item(G_LIST_MODEL(model), pos);
    if (!raw) return;
    GObject *obj = G_OBJECT(raw);
    if (G_TYPE_CHECK_INSTANCE_TYPE(obj, PHONE_ITEM_TYPE)) {
        self->populate_form_from_selected(PHONE_ITEM(obj));
    }
    g_object_unref(obj);
}

// Tìm kiếm realtime
void AdminApp::on_search_changed(GtkSearchEntry *entry, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);
    const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (!text || text[0] == '\0') {
        self->refresh_phone_list();
    } else {
        self->refresh_phone_list_filtered(std::string(text));
    }
}

// ── Nút THÊM sản phẩm ───────────────────────────────────────────────────────
void AdminApp::on_btn_add_clicked(GtkButton *btn, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);

    const char *id    = gtk_editable_get_text(GTK_EDITABLE(self->entry_id));
    const char *name  = gtk_editable_get_text(GTK_EDITABLE(self->entry_name));
    const char *brand = gtk_editable_get_text(GTK_EDITABLE(self->entry_brand));
    const char *os    = gtk_editable_get_text(GTK_EDITABLE(self->entry_os));
    const char *specs = gtk_editable_get_text(GTK_EDITABLE(self->entry_specs));
    const char *ip_s  = gtk_editable_get_text(GTK_EDITABLE(self->entry_import_price));
    const char *sp_s  = gtk_editable_get_text(GTK_EDITABLE(self->entry_sell_price));
    const char *qty_s = gtk_editable_get_text(GTK_EDITABLE(self->entry_quantity));

    // Validate UI trước khi gọi Service
    if (!id || id[0] == '\0' || !name || name[0] == '\0' ||
        !brand || brand[0] == '\0') {
        self->show_dialog("Lỗi nhập liệu",
                          "Các trường bắt buộc (*) không được để trống!",
                          GTK_MESSAGE_ERROR);
        return;
    }

    double importPrice = 0.0, sellPrice = 0.0;
    int    qty = 0;
    if (!parse_double(ip_s, importPrice)) {
        self->show_dialog("Lỗi nhập liệu", "Giá nhập không hợp lệ (phải là số)!", GTK_MESSAGE_ERROR);
        return;
    }
    if (!parse_double(sp_s, sellPrice)) {
        self->show_dialog("Lỗi nhập liệu", "Giá bán không hợp lệ (phải là số)!", GTK_MESSAGE_ERROR);
        return;
    }
    if (!parse_int(qty_s, qty)) {
        self->show_dialog("Lỗi nhập liệu", "Số lượng không hợp lệ (phải là số nguyên)!", GTK_MESSAGE_ERROR);
        return;
    }

    Phone p(id, name, brand, os, importPrice, sellPrice, qty, "", specs);

    std::string errMsg;
    if (self->phoneService->addPhone(p, errMsg)) {
        self->refresh_phone_list();
        self->clear_phone_form();
        self->show_dialog("Thành công",
                          "Đã thêm sản phẩm [" + std::string(name) + "] vào hệ thống!");
    } else {
        self->show_dialog("Thất bại", errMsg, GTK_MESSAGE_ERROR);
    }
}

// ── Nút CẬP NHẬT sản phẩm ───────────────────────────────────────────────────
void AdminApp::on_btn_update_clicked(GtkButton *btn, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);

    if (self->selected_phone_id.empty()) {
        self->show_dialog("Chưa chọn sản phẩm",
                          "Vui lòng chọn sản phẩm trong bảng trước khi cập nhật.",
                          GTK_MESSAGE_WARNING);
        return;
    }

    const char *name  = gtk_editable_get_text(GTK_EDITABLE(self->entry_name));
    const char *brand = gtk_editable_get_text(GTK_EDITABLE(self->entry_brand));
    const char *os    = gtk_editable_get_text(GTK_EDITABLE(self->entry_os));
    const char *specs = gtk_editable_get_text(GTK_EDITABLE(self->entry_specs));
    const char *ip_s  = gtk_editable_get_text(GTK_EDITABLE(self->entry_import_price));
    const char *sp_s  = gtk_editable_get_text(GTK_EDITABLE(self->entry_sell_price));
    const char *qty_s = gtk_editable_get_text(GTK_EDITABLE(self->entry_quantity));

    double importPrice = 0.0, sellPrice = 0.0;
    int    qty = 0;
    if (!parse_double(ip_s, importPrice) || !parse_double(sp_s, sellPrice) ||
        !parse_int(qty_s, qty)) {
        self->show_dialog("Lỗi nhập liệu",
                          "Giá nhập, giá bán hoặc số lượng không hợp lệ!",
                          GTK_MESSAGE_ERROR);
        return;
    }

    Phone p(self->selected_phone_id, name, brand, os,
            importPrice, sellPrice, qty, "", specs);

    std::string errMsg;
    if (self->phoneService->updatePhone(p, errMsg)) {
        self->refresh_phone_list();
        self->clear_phone_form();
        self->show_dialog("Thành công",
                          "Đã cập nhật sản phẩm [" + self->selected_phone_id + "] thành công!");
    } else {
        self->show_dialog("Thất bại", errMsg, GTK_MESSAGE_ERROR);
    }
}

// ── Nút XÓA sản phẩm ────────────────────────────────────────────────────────
void AdminApp::on_btn_delete_clicked(GtkButton *btn, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);

    if (self->selected_phone_id.empty()) {
        self->show_dialog("Chưa chọn sản phẩm",
                          "Vui lòng chọn sản phẩm trong bảng trước khi xóa.",
                          GTK_MESSAGE_WARNING);
        return;
    }

    std::string errMsg;
    std::string target_id = self->selected_phone_id;
    if (self->phoneService->deletePhone(target_id, errMsg)) {
        self->refresh_phone_list();
        self->clear_phone_form();
        self->show_dialog("Thành công",
                          "Đã xóa sản phẩm [" + target_id + "] khỏi hệ thống.");
    } else {
        self->show_dialog("Không thể xóa", errMsg, GTK_MESSAGE_ERROR);
    }
}

// ── Nút LÀM SẠCH form ───────────────────────────────────────────────────────
void AdminApp::on_btn_clear_clicked(GtkButton *btn, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);
    self->clear_phone_form();
}

// ── Nút REFRESH ─────────────────────────────────────────────────────────────
void AdminApp::on_btn_refresh_clicked(GtkButton *btn, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);
    self->refresh_phone_list();
    self->refresh_order_list();
}

// ── Nút TẠO ĐƠN HÀNG ────────────────────────────────────────────────────────
void AdminApp::on_btn_create_order_clicked(GtkButton *btn, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);

    const char *cust_id  = gtk_editable_get_text(GTK_EDITABLE(self->entry_order_customer));
    const char *phone_id = gtk_editable_get_text(GTK_EDITABLE(self->entry_order_phone_id));
    const char *qty_s    = gtk_editable_get_text(GTK_EDITABLE(self->entry_order_qty));

    // Validate
    if (!cust_id || cust_id[0] == '\0') {
        self->show_dialog("Lỗi", "Mã khách hàng không được để trống!", GTK_MESSAGE_ERROR);
        return;
    }
    if (!phone_id || phone_id[0] == '\0') {
        self->show_dialog("Lỗi", "Mã sản phẩm không được để trống!", GTK_MESSAGE_ERROR);
        return;
    }
    int qty = 0;
    if (!parse_int(qty_s, qty) || qty <= 0) {
        self->show_dialog("Lỗi", "Số lượng phải là số nguyên dương!", GTK_MESSAGE_ERROR);
        return;
    }

    // Kiểm tra sản phẩm tồn tại
    Phone* p = self->phoneService->getPhoneById(std::string(phone_id));
    if (p == nullptr) {
        self->show_dialog("Lỗi", "Không tìm thấy sản phẩm có mã: " +
                          std::string(phone_id), GTK_MESSAGE_ERROR);
        return;
    }

    // Tạo ID đơn hàng tự động theo timestamp
    time_t now = time(nullptr);
    std::string orderId = "ORD-" + std::to_string(now);

    Order order;
    order.setID(orderId);
    order.setCustomerId(std::string(cust_id));
    order.setStatus(OrderStatus::PENDING);

    OrderDetail detail(p->getID(), p->getName(), qty, p->getSellingPrice());
    order.addDetail(detail);

    std::string errMsg;
    if (self->orderService->createOrder(order, errMsg)) {
        self->refresh_order_list();
        self->refresh_phone_list(); // Refresh để stats chính xác
        // Xóa form
        gtk_editable_set_text(GTK_EDITABLE(self->entry_order_customer), "");
        gtk_editable_set_text(GTK_EDITABLE(self->entry_order_phone_id), "");
        gtk_editable_set_text(GTK_EDITABLE(self->entry_order_qty), "");

        std::string msg = "Tạo đơn hàng [" + orderId + "] thành công!\n"
                          "Sản phẩm: " + p->getName() + " x" + std::to_string(qty) + "\n"
                          "Đơn đã vào hàng đợi PENDING.";
        self->show_dialog("Tạo Đơn Thành Công", msg);
    } else {
        self->show_dialog("Tạo Đơn Thất Bại", errMsg, GTK_MESSAGE_ERROR);
    }
}

// ── Nút DUYỆT ĐƠN TIẾP THEO (FIFO) ─────────────────────────────────────────
void AdminApp::on_btn_process_order_clicked(GtkButton *btn, gpointer data) {
    AdminApp *self = static_cast<AdminApp*>(data);

    Order processedOrder;
    std::string errMsg;
    if (self->orderService->processNextPendingOrder(processedOrder, errMsg)) {
        self->refresh_order_list();
        self->refresh_phone_list(); // Cập nhật tồn kho sau khi trừ
        std::string msg =
            "Đã duyệt đơn hàng: [" + processedOrder.getID() + "]\n"
            "Trạng thái → ĐÃ XÁC NHẬN\n"
            "Số lượng tồn kho đã được trừ tự động.";
        self->show_dialog("Duyệt Đơn Thành Công", msg);
    } else {
        self->show_dialog("Duyệt Đơn Thất Bại", errMsg, GTK_MESSAGE_WARNING);
    }
}
