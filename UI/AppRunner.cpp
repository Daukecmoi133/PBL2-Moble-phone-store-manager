/**
 * @file AppRunner.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI VÒNG ĐỜI VÀ ĐIỀU PHỐI CỬA SỔ GTK4
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - AppRunner: Khởi tạo GtkApplication và áp dụng bộ phong cách AppCss toàn
 * cục.
 * - showLogin & showMainShell: Triển khai chuyển dịch cửa sổ duy nhất (Single
 * Window Transition), tiêu hủy hoàn toàn cửa sổ cũ để ngăn chặn việc cửa sổ
 * đăng xuất không biến mất.
 * - Callback handlers: Xử lý kích hoạt ứng dụng, đóng cửa sổ an toàn, đăng nhập
 * và đăng xuất.
 * =========================================================================================
 */

#include "AppRunner.h"
#include "styles/AppCss.h"
#include <cstring>
#include <iostream>

static gboolean is_font_debug_enabled() {
  return g_getenv("PBL2_DEBUG_FONT") != nullptr;
}

static void custom_printerr(const gchar *string) {
  if (!is_font_debug_enabled() && string &&
      (strstr(string, "couldn't load font") ||
       strstr(string, "All font fallbacks failed"))) {
    return;
  }
  fputs(string, stderr);
}

static GLogWriterOutput custom_log_writer(GLogLevelFlags log_level,
                                          const GLogField *fields,
                                          gsize n_fields, gpointer user_data) {
  for (gsize i = 0; i < n_fields; i++) {
    if (fields[i].key && strcmp(fields[i].key, "MESSAGE") == 0) {
      const char *msg = (const char *)fields[i].value;
      if (!is_font_debug_enabled() && msg &&
          (strstr(msg, "font fallbacks failed") ||
           strstr(msg, "couldn't load font"))) {
        return G_LOG_WRITER_HANDLED;
      }
    }
  }
  return g_log_writer_default(log_level, fields, n_fields, user_data);
}

AppRunner::AppRunner(AppController *ctrl)
    : controller(ctrl), loginWindow(nullptr), loginScreen(nullptr),
      shellWindow(nullptr) {
  g_set_printerr_handler(custom_printerr);
  g_log_set_writer_func(custom_log_writer, nullptr, nullptr);
  app = gtk_application_new("vn.edu.dut.pbl2.phonestore.ui",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), this);
}

AppRunner::~AppRunner() {
  delete loginScreen;
  loginScreen = nullptr;
  if (loginWindow) {
    GtkWidget *win = loginWindow;
    loginWindow = nullptr;
    if (GTK_IS_WINDOW(win)) {
      gtk_window_destroy(GTK_WINDOW(win));
    }
  }
  if (shellWindow) {
    delete shellWindow;
    shellWindow = nullptr;
  }
  if (app) {
    g_object_unref(app);
  }
}

int AppRunner::run(int argc, char *argv[]) {
  return g_application_run(G_APPLICATION(app), argc, argv);
}

void AppRunner::showLogin() {
  if (shellWindow) {
    delete shellWindow;
    shellWindow = nullptr;
  }

  loginWindow = gtk_application_window_new(app);
  g_signal_connect(loginWindow, "close-request",
                   G_CALLBACK(on_login_window_close_request), this);
  gtk_window_set_title(GTK_WINDOW(loginWindow),
                       "PBL2 — Đăng nhập hệ thống Phone Store");
  gtk_window_set_default_size(GTK_WINDOW(loginWindow), 960, 760);
  gtk_window_set_resizable(GTK_WINDOW(loginWindow), TRUE);

  delete loginScreen;
  loginScreen = new LoginScreen(controller, on_login_success, this);
  gtk_window_set_child(GTK_WINDOW(loginWindow), loginScreen->getRootWidget());

  gtk_window_present(GTK_WINDOW(loginWindow));
}

void AppRunner::showMainShell() {
  if (loginWindow) {
    GtkWidget *win = loginWindow;
    loginWindow = nullptr;
    if (GTK_IS_WINDOW(win)) {
      gtk_window_destroy(GTK_WINDOW(win));
    }
  }

  delete shellWindow;
  shellWindow = new MainShellWindow(app, controller, on_logout_success, this);
  shellWindow->show();
}

void AppRunner::on_app_activate(GtkApplication *gapp, gpointer data) {
  (void)gapp;
  AppRunner *self = static_cast<AppRunner *>(data);

  // Nạp toàn bộ CSS phong cách giao diện hiện đại
  applyAppStyles();

  // Mở màn hình đăng nhập đầu tiên
  self->showLogin();
}

gboolean AppRunner::on_login_window_close_request(GtkWindow *window,
                                                  gpointer data) {
  AppRunner *self = static_cast<AppRunner *>(data);
  if (self->loginWindow == GTK_WIDGET(window)) {
    self->loginWindow = nullptr;
  }
  return FALSE;
}

void AppRunner::on_login_success(AppController *ctrl, gpointer data) {
  (void)ctrl;
  AppRunner *self = static_cast<AppRunner *>(data);
  self->showMainShell();
}

void AppRunner::on_logout_success(AppController *ctrl, gpointer data) {
  (void)ctrl;
  AppRunner *self = static_cast<AppRunner *>(data);
  self->showLogin();
}
