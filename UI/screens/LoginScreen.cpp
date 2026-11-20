/**
 * @file LoginScreen.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI MÀN HÌNH ĐĂNG NHẬP VÀ XỬ LÝ LỖI AN TOÀN
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - triggerLogin: Đọc thông tin từ entryUsername và entryPassword, gọi
 * controller->login().
 * - Hiển thị thông báo lỗi bằng nhãn (lblError) thay vì hộp thoại modal làm đứt
 * gãy luồng người dùng.
 * - Hỗ trợ nạp nhanh tài khoản demo (Quick Fill Pills) cho các vai trò hệ
 * thống.
 * - Hỗ trợ hộp thoại mô phỏng Đăng ký và Quên mật khẩu đặt gọn gàng ở góc dưới
 * form.
 * =========================================================================================
 */

#include "LoginScreen.h"
#include "../components/UIComponents.h"
#include <cstring>
#include <gdk/gdkkeysyms.h>

static LoginScreen *s_activeLoginScreen = nullptr;

LoginScreen *LoginScreen::getActiveInstance() { return s_activeLoginScreen; }

struct QuickFillData {
  LoginScreen *screen;
  const char *user;
  const char *pass;
};

LoginScreen::LoginScreen(AppController *ctrl,
                         LoginSuccessCallback onLoginSuccess, gpointer userData)
    : controller(ctrl), entryUsername(nullptr), entryPassword(nullptr),
      lblError(nullptr), btnLogin(nullptr), successCallback(onLoginSuccess),
      callbackData(userData) {
  s_activeLoginScreen = this;

  // Root Overlay Container (hỗ trợ cả overlay dialog và toast ngay trên màn
  // đăng nhập)
  rootWidget = gtk_overlay_new();
  gtk_widget_add_css_class(rootWidget, "login-overlay");
  gtk_widget_set_hexpand(rootWidget, TRUE);
  gtk_widget_set_vexpand(rootWidget, TRUE);

  // Background Picture Widget: đảm bảo ảnh hình nền luôn sắc nét và scale chuẩn
  // full màn hình
  const char *bgPath1 =
      "data/Device/productImages/D&N Store Phone Sales Screen.png";
  const char *bgPath2 =
      "C:/DUY/PBL2/data/Device/productImages/D&N Store Phone Sales Screen.png";
  GFile *bgFile = g_file_new_for_path(bgPath1);
  if (!g_file_query_exists(bgFile, nullptr)) {
    g_object_unref(bgFile);
    bgFile = g_file_new_for_path(bgPath2);
  }
  bgPicture = gtk_picture_new_for_file(bgFile);
  g_object_unref(bgFile);
  gtk_picture_set_content_fit(GTK_PICTURE(bgPicture), GTK_CONTENT_FIT_COVER);
  gtk_widget_set_hexpand(bgPicture, TRUE);
  gtk_widget_set_vexpand(bgPicture, TRUE);
  gtk_overlay_set_child(GTK_OVERLAY(rootWidget), bgPicture);

  // Dark Gradient Scrim Overlay: Tăng cường độ tương phản, đậm hơn ở phía đặt
  // form
  GtkWidget *scrim = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(scrim, "login-scrim");
  gtk_widget_set_hexpand(scrim, TRUE);
  gtk_widget_set_vexpand(scrim, TRUE);
  gtk_overlay_add_overlay(GTK_OVERLAY(rootWidget), scrim);

  // Scrolled Container with background
  bgScroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(bgScroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_add_css_class(bgScroll, "login-scroll");
  gtk_widget_set_hexpand(bgScroll, TRUE);
  gtk_widget_set_vexpand(bgScroll, TRUE);

  // Right Alignment Box: Đưa bảng login sang bên phải đối xứng với dòng chữ D&N
  // store
  centerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(centerBox, TRUE);
  gtk_widget_set_vexpand(centerBox, TRUE);
  gtk_widget_set_halign(centerBox, GTK_ALIGN_END);
  gtk_widget_set_valign(centerBox, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_end(centerBox, 80);
  gtk_widget_set_margin_top(centerBox, 40);
  gtk_widget_set_margin_bottom(centerBox, 40);

  // GtkStack chuyển cảnh mượt mà giữa Đăng nhập / Đăng ký / Quên mật khẩu
  authStack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(authStack),
                                GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  gtk_stack_set_transition_duration(GTK_STACK(authStack), 260);
  gtk_stack_set_hhomogeneous(GTK_STACK(authStack), FALSE);
  gtk_stack_set_vhomogeneous(GTK_STACK(authStack), FALSE);
  gtk_widget_set_halign(authStack, GTK_ALIGN_END);
  gtk_widget_set_hexpand(authStack, FALSE);

  loginCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_add_css_class(loginCard, "login-card");
  gtk_widget_set_size_request(loginCard, 560, -1);
  gtk_widget_set_halign(loginCard, GTK_ALIGN_END);
  gtk_widget_set_hexpand(loginCard, FALSE);

  registerCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
  gtk_widget_add_css_class(registerCard, "login-card");
  gtk_widget_set_size_request(registerCard, 560, -1);
  gtk_widget_set_halign(registerCard, GTK_ALIGN_END);
  gtk_widget_set_hexpand(registerCard, FALSE);

  forgotCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
  gtk_widget_add_css_class(forgotCard, "login-card");
  gtk_widget_set_size_request(forgotCard, 560, -1);
  gtk_widget_set_halign(forgotCard, GTK_ALIGN_END);
  gtk_widget_set_hexpand(forgotCard, FALSE);

  gtk_stack_add_named(GTK_STACK(authStack), loginCard, "login");
  gtk_stack_add_named(GTK_STACK(authStack), registerCard, "register");
  gtk_stack_add_named(GTK_STACK(authStack), forgotCard, "forgot");

  gtk_box_append(GTK_BOX(centerBox), authStack);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(bgScroll), centerBox);
  gtk_overlay_add_overlay(GTK_OVERLAY(rootWidget), bgScroll);

  // Phím Esc: đóng overlay dialog nếu đang mở, hoặc quay lại trang Đăng nhập
  GtkEventController *keyCtrl = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(keyCtrl, GTK_PHASE_CAPTURE);
  g_signal_connect(keyCtrl, "key-pressed",
                   G_CALLBACK(+[](GtkEventControllerKey *, guint keyval, guint,
                                  GdkModifierType, gpointer d) -> gboolean {
                     LoginScreen *self = static_cast<LoginScreen *>(d);
                     if (!self)
                       return GDK_EVENT_PROPAGATE;
                     if (keyval == GDK_KEY_Escape) {
                       if (self->isOverlayDialogOpen()) {
                         self->closeOverlayDialog();
                         return GDK_EVENT_STOP;
                       }
                       const char *vis = gtk_stack_get_visible_child_name(
                           GTK_STACK(self->authStack));
                       if (vis && std::strcmp(vis, "login") != 0) {
                         self->showLoginForm();
                         return GDK_EVENT_STOP;
                       }
                     }
                     return GDK_EVENT_PROPAGATE;
                   }),
                   this);
  gtk_widget_add_controller(rootWidget, keyCtrl);

  showLoginForm();
}

LoginScreen::~LoginScreen() {
  if (activeToastTimerId > 0) {
    g_source_remove(activeToastTimerId);
    activeToastTimerId = 0;
  }
  if (s_activeLoginScreen == this) {
    s_activeLoginScreen = nullptr;
  }
}

void LoginScreen::showLoginForm() {
  // Clear loginCard
  GtkWidget *child = gtk_widget_get_first_child(loginCard);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(loginCard), child);
    child = next;
  }

  // Card Header: Tiêu đề trang đăng nhập
  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  GtkWidget *lblTitle = gtk_label_new("ĐĂNG NHẬP");
  gtk_widget_add_css_class(lblTitle, "login-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);

  GtkWidget *lblSub =
      gtk_label_new("Hệ thống Quản lý Cửa hàng Điện thoại");
  gtk_widget_add_css_class(lblSub, "login-subtitle");
  gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblSub), TRUE);
  gtk_label_set_wrap_mode(GTK_LABEL(lblSub), PANGO_WRAP_WORD_CHAR);
  gtk_label_set_max_width_chars(GTK_LABEL(lblSub), 42);

  gtk_box_append(GTK_BOX(headerBox), lblTitle);
  gtk_box_append(GTK_BOX(headerBox), lblSub);
  gtk_widget_set_margin_bottom(headerBox, 6);
  gtk_box_append(GTK_BOX(loginCard), headerBox);

  // Form: Username hoặc Số điện thoại (khóa tài khoản)
  entryUsername = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entryUsername),
                                 "Nhập tên đăng nhập hoặc số điện thoại...");
  gtk_widget_set_name(entryUsername, "login_username_entry");
  gtk_box_append(GTK_BOX(loginCard),
                 UIComponents::createFormField("Tên đăng nhập / Số điện thoại", entryUsername));

  // Form: Password
  entryPassword = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entryPassword), "Nhập mật khẩu...");
  gtk_widget_set_name(entryPassword, "login_password_entry");
  gtk_box_append(
      GTK_BOX(loginCard),
      UIComponents::createFormField(
          "Mật khẩu", UIComponents::createPasswordField(entryPassword)));

  // Xóa cảnh báo lỗi khi người dùng bắt đầu nhập lại
  g_signal_connect_swapped(entryUsername, "changed",
                           G_CALLBACK(+[](LoginScreen *s) {
                             if (s->lblError)
                               gtk_widget_set_visible(s->lblError, FALSE);
                           }),
                           this);
  g_signal_connect_swapped(entryPassword, "changed",
                           G_CALLBACK(+[](LoginScreen *s) {
                             if (s->lblError)
                               gtk_widget_set_visible(s->lblError, FALSE);
                           }),
                           this);
  g_signal_connect_swapped(entryUsername, "activate",
                           G_CALLBACK(+[](LoginScreen *s) {
                             if (s->entryPassword)
                               gtk_widget_grab_focus(s->entryPassword);
                           }),
                           this);
  g_signal_connect_swapped(
      entryPassword, "activate",
      G_CALLBACK(+[](LoginScreen *s) { s->triggerLogin(); }), this);

  // Corner Action 1: Quên mật khẩu (gọn ở góc phải bên dưới ô mật khẩu)
  GtkWidget *forgotBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_halign(forgotBox, GTK_ALIGN_END);
  GtkWidget *btnForgot = gtk_button_new_with_label("Quên mật khẩu?");
  gtk_widget_add_css_class(btnForgot, "login-corner-btn");
  UIComponents::setButtonCursor(btnForgot);
  g_signal_connect_swapped(
      btnForgot, "clicked",
      G_CALLBACK(+[](LoginScreen *s) { s->showForgotPasswordDialog(); }), this);
  gtk_box_append(GTK_BOX(forgotBox), btnForgot);
  gtk_box_append(GTK_BOX(loginCard), forgotBox);

  // Inline Alert message box
  lblError = gtk_label_new("");
  gtk_widget_add_css_class(lblError, "login-alert");
  gtk_label_set_xalign(GTK_LABEL(lblError), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblError), TRUE);
  gtk_label_set_wrap_mode(GTK_LABEL(lblError), PANGO_WRAP_WORD_CHAR);
  gtk_label_set_max_width_chars(GTK_LABEL(lblError), 42);
  gtk_widget_set_visible(lblError, FALSE);
  gtk_box_append(GTK_BOX(loginCard), lblError);

  // Login Button
  btnLogin = gtk_button_new_with_label("Đăng nhập hệ thống");
  gtk_widget_add_css_class(btnLogin, "btn-primary");
  gtk_widget_set_name(btnLogin, "login_button");
  UIComponents::setButtonCursor(btnLogin);
  gtk_widget_set_margin_top(btnLogin, 6);
  g_signal_connect(btnLogin, "clicked", G_CALLBACK(on_login_btn_clicked), this);
  gtk_box_append(GTK_BOX(loginCard), btnLogin);

  // Corner Action 2: Đăng ký tài khoản (gọn ở góc phải bên dưới nút đăng nhập)
  GtkWidget *regBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_halign(regBox, GTK_ALIGN_END);
  gtk_widget_set_margin_top(regBox, 4);

  GtkWidget *lblNoAcc = gtk_label_new("Chưa có tài khoản?");
  gtk_widget_add_css_class(lblNoAcc, "page-subtitle");
  GtkWidget *btnToRegister = gtk_button_new_with_label("Đăng ký ngay →");
  gtk_widget_add_css_class(btnToRegister, "login-corner-btn");
  UIComponents::setButtonCursor(btnToRegister);
  g_signal_connect_swapped(
      btnToRegister, "clicked",
      G_CALLBACK(+[](LoginScreen *s) { s->showRegisterForm(); }), this);

  gtk_box_append(GTK_BOX(regBox), lblNoAcc);
  gtk_box_append(GTK_BOX(regBox), btnToRegister);
  gtk_box_append(GTK_BOX(loginCard), regBox);

  // Quick Login Demo Helpers
  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_set_margin_top(sep, 14);
  gtk_widget_set_margin_bottom(sep, 6);
  gtk_box_append(GTK_BOX(loginCard), sep);

  GtkWidget *lblQuick = gtk_label_new("Tài khoản mẫu dùng thử hệ thống:");
  gtk_widget_add_css_class(lblQuick, "stat-label");
  gtk_widget_add_css_class(lblQuick, "login-quick-label");
  gtk_label_set_xalign(GTK_LABEL(lblQuick), 0.0f);
  gtk_box_append(GTK_BOX(loginCard), lblQuick);

  GtkWidget *quickGrid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(quickGrid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(quickGrid), 10);

  // 4 Quick Buttons
  auto makeQuickBtn = [this](const char *label, const char *user,
                             const char *pass) -> GtkWidget * {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_add_css_class(btn, "login-quick-pill");
    UIComponents::setButtonCursor(btn);
    gtk_widget_set_hexpand(btn, TRUE);
    QuickFillData *data = new QuickFillData{this, user, pass};
    g_signal_connect_data(
        btn, "clicked", G_CALLBACK(on_quick_fill_clicked), data,
        [](gpointer d, GClosure *) { delete static_cast<QuickFillData *>(d); },
        (GConnectFlags)0);
    return btn;
  };

  GtkWidget *bAdmin =
      makeQuickBtn("👑 Quản trị (Admin)", "admin.demo", "DemoOnly-Admin-2026");
  GtkWidget *bSales =
      makeQuickBtn("💼 Bán hàng (Sales)", "sales.demo", "DemoOnly-Sales-2026");
  GtkWidget *bWarehouse = makeQuickBtn(
      "📦 Thủ kho (Warehouse)", "warehouse.demo", "DemoOnly-Warehouse-2026");
  GtkWidget *bCust = makeQuickBtn("🛍️ Khách hàng (Customer)", "customer001.demo",
                                  "DemoOnly-Customer-001");

  gtk_grid_attach(GTK_GRID(quickGrid), bAdmin, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(quickGrid), bSales, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(quickGrid), bWarehouse, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(quickGrid), bCust, 1, 1, 1, 1);

  gtk_box_append(GTK_BOX(loginCard), quickGrid);

  gtk_stack_set_transition_type(GTK_STACK(authStack),
                                GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
  gtk_stack_set_visible_child_name(GTK_STACK(authStack), "login");
}

void LoginScreen::showRegisterForm() {
  // Clear registerCard
  GtkWidget *child = gtk_widget_get_first_child(registerCard);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(registerCard), child);
    child = next;
  }

  // Top Navigation: Back to login
  GtkWidget *topBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *btnBack = gtk_button_new_with_label("← Quay lại Đăng nhập");
  gtk_widget_add_css_class(btnBack, "btn-back");
  UIComponents::setButtonCursor(btnBack);
  g_signal_connect_swapped(
      btnBack, "clicked",
      G_CALLBACK(+[](LoginScreen *s) { s->showLoginForm(); }), this);
  gtk_box_append(GTK_BOX(topBar), btnBack);
  gtk_box_append(GTK_BOX(registerCard), topBar);

  GtkWidget *titleBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  GtkWidget *lblTitle = gtk_label_new("ĐĂNG KÝ TÀI KHOẢN MỚI");
  gtk_widget_add_css_class(lblTitle, "login-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  GtkWidget *lblSub =
      gtk_label_new("Tạo tài khoản khách hàng để mua sắm và tích điểm ưu đãi.");
  gtk_widget_add_css_class(lblSub, "login-subtitle");
  gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblSub), TRUE);
  gtk_label_set_wrap_mode(GTK_LABEL(lblSub), PANGO_WRAP_WORD_CHAR);
  gtk_label_set_max_width_chars(GTK_LABEL(lblSub), 42);
  gtk_box_append(GTK_BOX(titleBox), lblTitle);
  gtk_box_append(GTK_BOX(titleBox), lblSub);
  gtk_box_append(GTK_BOX(registerCard), titleBox);

  // Fields
  GtkWidget *eFullName = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eFullName), "Ví dụ: Nguyễn Văn An");
  gtk_box_append(GTK_BOX(registerCard),
                 UIComponents::createFormField("Họ và tên *", eFullName));

  GtkWidget *ePhone = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePhone), "Ví dụ: 0912345678");
  gtk_box_append(GTK_BOX(registerCard),
                 UIComponents::createFormField("Số điện thoại *", ePhone));

  GtkWidget *eEmail = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eEmail),
                                 "Ví dụ: an.nguyen@example.com");
  gtk_box_append(GTK_BOX(registerCard),
                 UIComponents::createFormField("Email liên hệ", eEmail));

  GtkWidget *eUser = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eUser),
                                 "Tên đăng nhập (chữ cái & số)...");
  gtk_box_append(GTK_BOX(registerCard),
                 UIComponents::createFormField("Tên đăng nhập *", eUser));

  GtkWidget *ePass = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePass),
                                 "Nhập mật khẩu (tối thiểu 4 ký tự)...");
  gtk_box_append(GTK_BOX(registerCard),
                 UIComponents::createFormField(
                     "Mật khẩu *", UIComponents::createPasswordField(ePass)));

  GtkWidget *ePassConfirm = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePassConfirm),
                                 "Nhập lại mật khẩu...");
  gtk_box_append(GTK_BOX(registerCard),
                 UIComponents::createFormField(
                     "Xác nhận mật khẩu *",
                     UIComponents::createPasswordField(ePassConfirm)));

  GtkWidget *lblRegMsg = gtk_label_new("");
  gtk_widget_add_css_class(lblRegMsg, "badge-cancelled");
  gtk_label_set_xalign(GTK_LABEL(lblRegMsg), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(lblRegMsg), TRUE);
  gtk_label_set_wrap_mode(GTK_LABEL(lblRegMsg), PANGO_WRAP_WORD_CHAR);
  gtk_label_set_max_width_chars(GTK_LABEL(lblRegMsg), 42);
  gtk_widget_set_visible(lblRegMsg, FALSE);
  gtk_box_append(GTK_BOX(registerCard), lblRegMsg);

  struct RegCtx {
    LoginScreen *s;
    GtkWidget *eName;
    GtkWidget *ePhone;
    GtkWidget *eEmail;
    GtkWidget *eUser;
    GtkWidget *ePass;
    GtkWidget *eConfirm;
    GtkWidget *lblMsg;
  };
  RegCtx *ctx = new RegCtx{this,  eFullName, ePhone,       eEmail,
                           eUser, ePass,     ePassConfirm, lblRegMsg};

  GtkWidget *btnSubmitReg = gtk_button_new_with_label("Hoàn tất Đăng ký");
  gtk_widget_add_css_class(btnSubmitReg, "btn-primary");
  UIComponents::setButtonCursor(btnSubmitReg);
  gtk_widget_set_margin_top(btnSubmitReg, 10);
  g_signal_connect_data(
      btnSubmitReg, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        RegCtx *c = static_cast<RegCtx *>(d);
        const char *name = gtk_editable_get_text(GTK_EDITABLE(c->eName));
        const char *phone = gtk_editable_get_text(GTK_EDITABLE(c->ePhone));
        const char *email = gtk_editable_get_text(GTK_EDITABLE(c->eEmail));
        const char *user = gtk_editable_get_text(GTK_EDITABLE(c->eUser));
        const char *pass = gtk_editable_get_text(GTK_EDITABLE(c->ePass));
        const char *confirm = gtk_editable_get_text(GTK_EDITABLE(c->eConfirm));

        if (!name || strlen(name) == 0 || !phone || strlen(phone) == 0 ||
            !user || strlen(user) == 0 || !pass || strlen(pass) == 0) {
          gtk_label_set_text(GTK_LABEL(c->lblMsg),
                             "Vui lòng nhập đầy đủ các thông tin bắt buộc (*)");
          gtk_widget_set_visible(c->lblMsg, TRUE);
          return;
        }

        if (std::string(pass) != std::string(confirm)) {
          gtk_label_set_text(GTK_LABEL(c->lblMsg),
                             "Mật khẩu xác nhận không khớp!");
          gtk_widget_set_visible(c->lblMsg, TRUE);
          return;
        }

        OperationResult res = c->s->controller->registerCustomer(
            user, pass, name, phone, email ? email : "");
        if (res.success) {
          std::string savedUser = user;
          std::string savedPass = pass;
          c->s->showLoginForm();
          c->s->setQuickCredentials(savedUser, savedPass);
          UIComponents::showSuccessDialog(nullptr, "Đăng ký thành công",
                                          res.message);
        } else {
          gtk_label_set_text(GTK_LABEL(c->lblMsg), res.message.c_str());
          gtk_widget_set_visible(c->lblMsg, TRUE);
        }
      }),
      ctx, [](gpointer d, GClosure *) { delete static_cast<RegCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(registerCard), btnSubmitReg);

  gtk_stack_set_transition_type(GTK_STACK(authStack),
                                GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
  gtk_stack_set_visible_child_name(GTK_STACK(authStack), "register");
  gtk_widget_grab_focus(eFullName);
}

void LoginScreen::showForgotPasswordDialog() {
  // Clear forgotCard
  GtkWidget *child = gtk_widget_get_first_child(forgotCard);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(forgotCard), child);
    child = next;
  }

  // Top Navigation: Back to login
  GtkWidget *topBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *btnBack = gtk_button_new_with_label("← Quay lại Đăng nhập");
  gtk_widget_add_css_class(btnBack, "btn-back");
  UIComponents::setButtonCursor(btnBack);
  g_signal_connect_swapped(
      btnBack, "clicked",
      G_CALLBACK(+[](LoginScreen *s) { s->showLoginForm(); }), this);
  gtk_box_append(GTK_BOX(topBar), btnBack);
  gtk_box_append(GTK_BOX(forgotCard), topBar);

  GtkWidget *titleBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  GtkWidget *title = gtk_label_new("🔑 KHÔI PHỤC MẬT KHẨU");
  gtk_widget_add_css_class(title, "login-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  GtkWidget *desc =
      gtk_label_new("Tính năng mô phỏng cho đồ án PBL2: Nhập tên "
                    "tài khoản, SĐT hoặc Email đã đăng ký để xác minh "
                    "và mật khẩu mới (tối thiểu 4 ký tự):");
  gtk_widget_add_css_class(desc, "login-subtitle");
  gtk_label_set_xalign(GTK_LABEL(desc), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(desc), TRUE);
  gtk_label_set_wrap_mode(GTK_LABEL(desc), PANGO_WRAP_WORD_CHAR);
  gtk_label_set_max_width_chars(GTK_LABEL(desc), 42);
  gtk_box_append(GTK_BOX(titleBox), title);
  gtk_box_append(GTK_BOX(titleBox), desc);
  gtk_box_append(GTK_BOX(forgotCard), titleBox);

  GtkWidget *eUser = gtk_entry_new();
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(eUser),
      "Tên đăng nhập (ví dụ: customer001.demo, admin.demo)...");
  gtk_box_append(GTK_BOX(forgotCard), UIComponents::createFormField(
                                          "Tên đăng nhập tài khoản", eUser));

  GtkWidget *eContact = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eContact),
                                 "SĐT hoặc Email đã đăng ký với tài khoản...");
  gtk_box_append(GTK_BOX(forgotCard),
                 UIComponents::createFormField(
                     "Xác minh danh tính (SĐT / Email)", eContact));

  GtkWidget *ePass = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePass), "Nhập mật khẩu mới...");
  gtk_box_append(GTK_BOX(forgotCard),
                 UIComponents::createFormField(
                     "Mật khẩu mới", UIComponents::createPasswordField(ePass)));

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 10);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked",
      G_CALLBACK(+[](LoginScreen *s) { s->showLoginForm(); }), this);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct ForgotCtx {
    LoginScreen *s;
    GtkWidget *u;
    GtkWidget *p;
    GtkWidget *contact;
  };
  ForgotCtx *ctx = new ForgotCtx{this, eUser, ePass, eContact};

  GtkWidget *btnConfirm = gtk_button_new_with_label("Đặt lại mật khẩu");
  gtk_widget_add_css_class(btnConfirm, "btn-primary");
  UIComponents::setButtonCursor(btnConfirm);
  g_signal_connect_data(
      btnConfirm, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        ForgotCtx *c = static_cast<ForgotCtx *>(d);
        const char *u = gtk_editable_get_text(GTK_EDITABLE(c->u));
        const char *p = gtk_editable_get_text(GTK_EDITABLE(c->p));
        const char *contact = gtk_editable_get_text(GTK_EDITABLE(c->contact));

        if (!u || strlen(u) == 0 || !p || strlen(p) == 0 || !contact ||
            strlen(contact) == 0) {
          UIComponents::showWarningDialog(
              nullptr, "Thiếu thông tin",
              "Vui lòng nhập tên đăng nhập, SĐT/Email xác minh và mật khẩu "
              "mới!");
          return;
        }

        OperationResult res =
            c->s->controller->resetPasswordSimulation(u, p, contact);
        if (res.success) {
          std::string savedUser = u;
          std::string savedPass = p;
          c->s->showLoginForm();
          c->s->setQuickCredentials(savedUser, savedPass);
          UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
        } else {
          UIComponents::showErrorDialog(nullptr, "Lỗi", res.message);
        }
      }),
      ctx, [](gpointer d, GClosure *) { delete static_cast<ForgotCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnConfirm);
  gtk_box_append(GTK_BOX(forgotCard), btnRow);

  UIComponents::setupDialogShortcuts(forgotCard, btnConfirm, btnCancel);

  gtk_stack_set_transition_type(GTK_STACK(authStack),
                                GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
  gtk_stack_set_visible_child_name(GTK_STACK(authStack), "forgot");
  gtk_widget_grab_focus(eUser);
}

void LoginScreen::showOverlayDialog(GtkWidget *dialogCard) {
  if (!rootWidget || !dialogCard)
    return;

  closeOverlayDialog();

  currentOverlayWrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(currentOverlayWrapper, "overlay-scrim");
  gtk_widget_add_css_class(currentOverlayWrapper, "overlay-entering");
  gtk_widget_set_hexpand(currentOverlayWrapper, TRUE);
  gtk_widget_set_vexpand(currentOverlayWrapper, TRUE);
  gtk_widget_set_halign(currentOverlayWrapper, GTK_ALIGN_FILL);
  gtk_widget_set_valign(currentOverlayWrapper, GTK_ALIGN_FILL);

  currentOverlayCard = dialogCard;
  gtk_widget_add_css_class(dialogCard, "overlay-card");
  gtk_widget_set_halign(dialogCard, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(dialogCard, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(dialogCard, 800, -1);
  gtk_widget_set_hexpand(dialogCard, TRUE);
  gtk_widget_set_vexpand(dialogCard, TRUE);
  gtk_box_append(GTK_BOX(currentOverlayWrapper), dialogCard);

  GtkGesture *click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);
  g_signal_connect(
      click, "pressed",
      G_CALLBACK(+[](GtkGestureClick *, int, double x, double y, gpointer d) {
        LoginScreen *self = static_cast<LoginScreen *>(d);
        if (!self || !self->currentOverlayWrapper)
          return;
        GtkWidget *picked = gtk_widget_pick(self->currentOverlayWrapper, x, y,
                                            GTK_PICK_DEFAULT);
        if (picked == self->currentOverlayWrapper) {
          self->closeOverlayDialog();
        }
      }),
      this);
  gtk_widget_add_controller(currentOverlayWrapper, GTK_EVENT_CONTROLLER(click));

  gtk_overlay_add_overlay(GTK_OVERLAY(rootWidget), currentOverlayWrapper);

  GtkWidget *scrimRef = GTK_WIDGET(g_object_ref(currentOverlayWrapper));
  g_idle_add(
      +[](gpointer d) -> gboolean {
        GtkWidget *w = GTK_WIDGET(d);
        if (w && GTK_IS_WIDGET(w)) {
          gtk_widget_remove_css_class(w, "overlay-entering");
          g_object_unref(w);
        }
        return G_SOURCE_REMOVE;
      },
      scrimRef);

  gtk_widget_child_focus(dialogCard, GTK_DIR_TAB_FORWARD);
}

void LoginScreen::closeOverlayDialog() {
  if (!currentOverlayWrapper)
    return;

  GtkWidget *wrapper = currentOverlayWrapper;
  currentOverlayWrapper = nullptr;
  currentOverlayCard = nullptr;

  if (rootWidget && GTK_IS_OVERLAY(rootWidget) &&
      gtk_widget_get_parent(wrapper) == rootWidget) {
    gtk_overlay_remove_overlay(GTK_OVERLAY(rootWidget), wrapper);
  }
}

void LoginScreen::showToast(const std::string &message,
                            const std::string &type) {
  if (!rootWidget)
    return;

  if (activeToastTimerId > 0) {
    g_source_remove(activeToastTimerId);
    activeToastTimerId = 0;
  }
  if (activeToastBox) {
    if (gtk_widget_get_parent(activeToastBox) == rootWidget) {
      gtk_overlay_remove_overlay(GTK_OVERLAY(rootWidget), activeToastBox);
    }
    activeToastBox = nullptr;
  }

  GtkWidget *toastBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_add_css_class(toastBox, "in-app-toast");
  if (type == "error") {
    gtk_widget_add_css_class(toastBox, "in-app-toast-error");
  } else if (type == "info") {
    gtk_widget_add_css_class(toastBox, "in-app-toast-info");
  } else {
    gtk_widget_add_css_class(toastBox, "in-app-toast-success");
  }

  gtk_widget_set_halign(toastBox, GTK_ALIGN_END);
  gtk_widget_set_valign(toastBox, GTK_ALIGN_START);
  gtk_widget_set_margin_top(toastBox, 24);
  gtk_widget_set_margin_end(toastBox, 24);

  const char *iconStr = "✅";
  if (type == "error")
    iconStr = "❌";
  else if (type == "info")
    iconStr = "ℹ️";

  GtkWidget *iconLbl = gtk_label_new(iconStr);
  gtk_box_append(GTK_BOX(toastBox), iconLbl);

  GtkWidget *msgLbl = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(msgLbl, "in-app-toast-text");
  gtk_label_set_wrap(GTK_LABEL(msgLbl), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(msgLbl), 42);
  gtk_label_set_xalign(GTK_LABEL(msgLbl), 0.0f);
  gtk_box_append(GTK_BOX(toastBox), msgLbl);

  gtk_overlay_add_overlay(GTK_OVERLAY(rootWidget), toastBox);
  activeToastBox = toastBox;

  activeToastTimerId = g_timeout_add(
      2400,
      +[](gpointer d) -> gboolean {
        LoginScreen *self = static_cast<LoginScreen *>(d);
        if (s_activeLoginScreen == self && self->activeToastBox &&
            self->rootWidget) {
          if (gtk_widget_get_parent(self->activeToastBox) == self->rootWidget) {
            gtk_overlay_remove_overlay(GTK_OVERLAY(self->rootWidget),
                                       self->activeToastBox);
          }
          self->activeToastBox = nullptr;
        }
        if (s_activeLoginScreen == self) {
          self->activeToastTimerId = 0;
        }
        return G_SOURCE_REMOVE;
      },
      this);
}

void LoginScreen::setQuickCredentials(const std::string &username,
                                      const std::string &password) {
  if (entryUsername)
    gtk_editable_set_text(GTK_EDITABLE(entryUsername), username.c_str());
  if (entryPassword)
    gtk_editable_set_text(GTK_EDITABLE(entryPassword), password.c_str());
  if (lblError)
    gtk_widget_set_visible(lblError, FALSE);
}

void LoginScreen::resetForm() {
  closeOverlayDialog();
  if (authStack && GTK_IS_STACK(authStack)) {
    gtk_stack_set_visible_child_name(GTK_STACK(authStack), "login");
  }
  if (entryPassword && GTK_IS_EDITABLE(entryPassword)) {
    gtk_editable_set_text(GTK_EDITABLE(entryPassword), "");
  }
  if (lblError && GTK_IS_WIDGET(lblError)) {
    gtk_widget_set_visible(lblError, FALSE);
  }
  if (entryUsername && GTK_IS_WIDGET(entryUsername)) {
    gtk_widget_grab_focus(entryUsername);
    g_idle_add(
        +[](gpointer d) -> gboolean {
          LoginScreen *self = static_cast<LoginScreen *>(d);
          if (s_activeLoginScreen == self && self->entryUsername &&
              GTK_IS_WIDGET(self->entryUsername)) {
            gtk_widget_grab_focus(self->entryUsername);
          }
          return G_SOURCE_REMOVE;
        },
        this);
  }
}

void LoginScreen::triggerLogin() {
  const char *user = gtk_editable_get_text(GTK_EDITABLE(entryUsername));
  const char *pass = gtk_editable_get_text(GTK_EDITABLE(entryPassword));

  LoginRequest req;
  req.username = user ? user : "";
  req.password = pass ? pass : "";

  LoginResponse resp = controller->login(req);

  if (resp.result.success) {
    if (lblError)
      gtk_widget_set_visible(lblError, FALSE);
    if (successCallback) {
      successCallback(controller, callbackData);
    }
  } else {
    if (lblError) {
      std::string msg = resp.result.message;
      if (msg.find("Mật khẩu không chính xác") != std::string::npos) {
        msg = "⚠️ Sai mật khẩu đăng nhập. Vui lòng kiểm tra lại.";
      } else if (msg.find("vô hiệu hóa") != std::string::npos) {
        msg = "⚠️ Tài khoản đã bị vô hiệu hóa hoặc đã bị khóa trong hệ thống.";
      } else if (msg.find("không tồn tại") != std::string::npos ||
                 msg.find("bị khóa") != std::string::npos) {
        msg = "⚠️ Tài khoản không tồn tại hoặc đã bị khóa trong hệ thống.";
      } else if (msg.find("không được để trống") != std::string::npos) {
        msg = "⚠️ Vui lòng nhập đầy đủ tên đăng nhập/SĐT và mật khẩu.";
      } else if (!msg.empty() && msg.find("⚠️") == std::string::npos) {
        msg = "⚠️ " + msg;
      }
      gtk_label_set_text(GTK_LABEL(lblError), msg.c_str());
      gtk_widget_set_visible(lblError, TRUE);
    }
  }
}

void LoginScreen::on_login_btn_clicked(GtkButton *btn, gpointer data) {
  (void)btn;
  LoginScreen *self = static_cast<LoginScreen *>(data);
  self->triggerLogin();
}

void LoginScreen::on_quick_fill_clicked(GtkButton *btn, gpointer data) {
  (void)btn;
  QuickFillData *qd = static_cast<QuickFillData *>(data);
  qd->screen->setQuickCredentials(qd->user, qd->pass);
}
