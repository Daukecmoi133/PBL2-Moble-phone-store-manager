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

  // Root Container with dark gradient background
  rootWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(rootWidget, "login-bg");
  gtk_widget_set_hexpand(rootWidget, TRUE);
  gtk_widget_set_vexpand(rootWidget, TRUE);

  // Center Alignment Box
  centerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(centerBox, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(centerBox, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(centerBox, 40);
  gtk_widget_set_margin_bottom(centerBox, 40);

  card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
  gtk_widget_add_css_class(card, "login-card");
  gtk_widget_set_size_request(card, 480, -1);

  gtk_box_append(GTK_BOX(centerBox), card);
  gtk_box_append(GTK_BOX(rootWidget), centerBox);

  showLoginForm();
}

void LoginScreen::showLoginForm() {
  // Clear card
  GtkWidget *child = gtk_widget_get_first_child(card);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(card), child);
    child = next;
  }

  // Brand Logo & Title
  GtkWidget *brandBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  GtkWidget *lblLogo = gtk_label_new("📱");
  gtk_widget_set_margin_bottom(lblLogo, 4);

  GtkWidget *lblTitle = gtk_label_new("PHONE STORE MANAGER");
  gtk_widget_add_css_class(lblTitle, "login-title");

  GtkWidget *lblSubtitle =
      gtk_label_new("Hệ thống quản lý cửa hàng điện thoại — Đồ án PBL2");
  gtk_widget_add_css_class(lblSubtitle, "login-subtitle");

  gtk_box_append(GTK_BOX(brandBox), lblLogo);
  gtk_box_append(GTK_BOX(brandBox), lblTitle);
  gtk_box_append(GTK_BOX(brandBox), lblSubtitle);
  gtk_box_append(GTK_BOX(card), brandBox);

  // Form: Username
  entryUsername = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entryUsername),
                                 "Nhập tên đăng nhập...");
  gtk_widget_set_name(entryUsername, "login_username_entry");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Tên đăng nhập", entryUsername));

  // Form: Password
  entryPassword = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entryPassword), "Nhập mật khẩu...");
  gtk_entry_set_visibility(GTK_ENTRY(entryPassword), FALSE);
  gtk_widget_set_name(entryPassword, "login_password_entry");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Mật khẩu", entryPassword));

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
  gtk_box_append(GTK_BOX(card), forgotBox);

  // Error message label
  lblError = gtk_label_new("");
  gtk_widget_add_css_class(lblError, "badge-cancelled");
  gtk_widget_set_visible(lblError, FALSE);
  gtk_box_append(GTK_BOX(card), lblError);

  // Login Button
  btnLogin = gtk_button_new_with_label("Đăng nhập hệ thống");
  gtk_widget_add_css_class(btnLogin, "btn-primary");
  gtk_widget_set_name(btnLogin, "login_button");
  UIComponents::setButtonCursor(btnLogin);
  gtk_widget_set_margin_top(btnLogin, 6);
  g_signal_connect(btnLogin, "clicked", G_CALLBACK(on_login_btn_clicked), this);
  gtk_box_append(GTK_BOX(card), btnLogin);

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
  gtk_box_append(GTK_BOX(card), regBox);

  // Quick Login Demo Helpers
  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_set_margin_top(sep, 14);
  gtk_widget_set_margin_bottom(sep, 6);
  gtk_box_append(GTK_BOX(card), sep);

  GtkWidget *lblQuick =
      gtk_label_new("Chọn nhanh tài khoản Demo để kiểm thử đồ án:");
  gtk_widget_add_css_class(lblQuick, "stat-label");
  gtk_label_set_xalign(GTK_LABEL(lblQuick), 0.0f);
  gtk_box_append(GTK_BOX(card), lblQuick);

  GtkWidget *quickGrid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(quickGrid), 8);
  gtk_grid_set_column_spacing(GTK_GRID(quickGrid), 8);

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

  gtk_box_append(GTK_BOX(card), quickGrid);
}

void LoginScreen::showRegisterForm() {
  // Clear card
  GtkWidget *child = gtk_widget_get_first_child(card);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(card), child);
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
  gtk_box_append(GTK_BOX(card), topBar);

  GtkWidget *titleBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  GtkWidget *lblTitle = gtk_label_new("ĐĂNG KÝ TÀI KHOẢN MỚI");
  gtk_widget_add_css_class(lblTitle, "login-title");
  GtkWidget *lblSub =
      gtk_label_new("Tạo tài khoản khách hàng để mua sắm và tích điểm ưu đãi.");
  gtk_widget_add_css_class(lblSub, "login-subtitle");
  gtk_box_append(GTK_BOX(titleBox), lblTitle);
  gtk_box_append(GTK_BOX(titleBox), lblSub);
  gtk_box_append(GTK_BOX(card), titleBox);

  // Fields
  GtkWidget *eFullName = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eFullName), "Ví dụ: Nguyễn Văn An");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Họ và tên *", eFullName));

  GtkWidget *ePhone = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePhone), "Ví dụ: 0912345678");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Số điện thoại *", ePhone));

  GtkWidget *eEmail = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eEmail),
                                 "Ví dụ: an.nguyen@example.com");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Email liên hệ", eEmail));

  GtkWidget *eUser = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eUser),
                                 "Tên đăng nhập (chữ cái & số)...");
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Tên đăng nhập *", eUser));

  GtkWidget *ePass = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePass),
                                 "Nhập mật khẩu (tối thiểu 4 ký tự)...");
  gtk_entry_set_visibility(GTK_ENTRY(ePass), FALSE);
  gtk_box_append(GTK_BOX(card),
                 UIComponents::createFormField("Mật khẩu *", ePass));

  GtkWidget *ePassConfirm = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePassConfirm),
                                 "Nhập lại mật khẩu...");
  gtk_entry_set_visibility(GTK_ENTRY(ePassConfirm), FALSE);
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Xác nhận mật khẩu *", ePassConfirm));

  GtkWidget *lblRegMsg = gtk_label_new("");
  gtk_widget_add_css_class(lblRegMsg, "badge-cancelled");
  gtk_widget_set_visible(lblRegMsg, FALSE);
  gtk_box_append(GTK_BOX(card), lblRegMsg);

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

  gtk_box_append(GTK_BOX(card), btnSubmitReg);
}

void LoginScreen::showForgotPasswordDialog() {
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win), "Khôi phục mật khẩu");
  gtk_window_set_default_size(GTK_WINDOW(win), 450, 430);
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);

  GtkWidget *cardBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(cardBox, "dialog-content");

  GtkWidget *title = gtk_label_new("🔑 Khôi phục Mật khẩu");
  gtk_widget_add_css_class(title, "dialog-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(cardBox), title);

  GtkWidget *desc = gtk_label_new("Tính năng mô phỏng cho đồ án PBL2: Nhập tên "
                                  "tài khoản, SĐT hoặc Email đã đăng ký để xác minh "
                                  "và mật khẩu mới (tối thiểu 4 ký tự):");
  gtk_widget_add_css_class(desc, "page-subtitle");
  gtk_label_set_wrap(GTK_LABEL(desc), TRUE);
  gtk_label_set_xalign(GTK_LABEL(desc), 0.0f);
  gtk_box_append(GTK_BOX(cardBox), desc);

  GtkWidget *eUser = gtk_entry_new();
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(eUser),
      "Tên đăng nhập (ví dụ: customer001.demo, admin.demo)...");
  gtk_box_append(GTK_BOX(cardBox), UIComponents::createFormField(
                                       "Tên đăng nhập tài khoản", eUser));

  GtkWidget *eContact = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eContact),
                                 "SĐT hoặc Email đã đăng ký với tài khoản...");
  gtk_box_append(GTK_BOX(cardBox),
                 UIComponents::createFormField("Xác minh danh tính (SĐT / Email)", eContact));

  GtkWidget *ePass = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(ePass), "Nhập mật khẩu mới...");
  gtk_entry_set_visibility(GTK_ENTRY(ePass), FALSE);
  gtk_box_append(GTK_BOX(cardBox),
                 UIComponents::createFormField("Mật khẩu mới", ePass));

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 10);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy),
                           win);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct ForgotCtx {
    LoginScreen *s;
    GtkWidget *w;
    GtkWidget *u;
    GtkWidget *p;
    GtkWidget *contact;
  };
  ForgotCtx *ctx = new ForgotCtx{this, win, eUser, ePass, eContact};

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
              GTK_WINDOW(c->w), "Thiếu thông tin",
              "Vui lòng nhập tên đăng nhập, SĐT/Email xác minh và mật khẩu mới!");
          return;
        }

        OperationResult res =
            c->s->controller->resetPasswordSimulation(u, p, contact);
        if (res.success) {
          c->s->setQuickCredentials(u, p);
          UIComponents::showSuccessDialog(nullptr, "Thành công", res.message);
          gtk_window_destroy(GTK_WINDOW(c->w));
        } else {
          UIComponents::showErrorDialog(GTK_WINDOW(c->w), "Lỗi", res.message);
        }
      }),
      ctx, [](gpointer d, GClosure *) { delete static_cast<ForgotCtx *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnConfirm);
  gtk_box_append(GTK_BOX(cardBox), btnRow);

  gtk_window_set_child(GTK_WINDOW(win), cardBox);
  gtk_window_present(GTK_WINDOW(win));
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
      gtk_label_set_text(GTK_LABEL(lblError), resp.result.message.c_str());
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
