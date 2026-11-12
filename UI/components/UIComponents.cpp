/**
 * @file UIComponents.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI HỆ THỐNG GIAO DIỆN CHUẨN GTK4
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - formatVND & formatNumber: Chuyển đổi định dạng số sang chuẩn tiền tệ Việt
 * Nam.
 * - showErrorDialog / showSuccessDialog / showWarningDialog / showInfoDialog:
 *   Tạo GtkWindow dạng Modal, liên kết transient_for với cửa sổ cha, tự giải
 * phóng khi đóng.
 * - createEmptyState / createLoadingState / createErrorState:
 *   Tạo khung giao diện phản hồi trạng thái dữ liệu rỗng hoặc lỗi kết nối.
 * - createStatCard & createStatusBadge & createRankBadge:
 *   Tạo thẻ chỉ số Dashboard và huy hiệu phân loại theo đúng lớp CSS trong
 * AppCss.h.
 * =========================================================================================
 */

#include "UIComponents.h"
#include "../../source/controllers/AppController.h"
#include "../screens/LoginScreen.h"
#include "../screens/MainShellWindow.h"
#include "../styles/AppCss.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <gdk/gdkkeysyms.h>
#include <iomanip>
#include <sstream>

namespace UIComponents {

static MainShellWindow *g_activeShell = nullptr;

void registerShellWindow(MainShellWindow *shell) { g_activeShell = shell; }

MainShellWindow *getActiveShell() { return g_activeShell; }

void pushPage(const std::string &id, GtkWidget *content,
              const std::string &title) {
  if (g_activeShell) {
    g_activeShell->pushPage(id, content, title);
  }
}

void popPage() {
  if (g_activeShell) {
    g_activeShell->popPage();
  }
}

void clearPageStack() {
  if (g_activeShell) {
    g_activeShell->clearPageStack();
  }
}

void showOverlayDialog(GtkWidget *card) {
  if (g_activeShell) {
    g_activeShell->showOverlayDialog(card);
  } else if (LoginScreen::getActiveInstance()) {
    LoginScreen::getActiveInstance()->showOverlayDialog(card);
  }
}

void closeOverlayDialog() {
  if (g_activeShell) {
    g_activeShell->closeOverlayDialog();
  } else if (LoginScreen::getActiveInstance()) {
    LoginScreen::getActiveInstance()->closeOverlayDialog();
  }
}

bool isOverlayDialogOpen() {
  if (g_activeShell) {
    return g_activeShell->isOverlayDialogOpen();
  }
  if (LoginScreen::getActiveInstance()) {
    return LoginScreen::getActiveInstance()->isOverlayDialogOpen();
  }
  return false;
}

// === ĐỊNH DẠNG DỮ LIỆU ===

std::string formatVND(double amount) {
  long long value = static_cast<long long>(amount);
  std::string s = std::to_string(value);
  std::string result = "";
  int count = 0;
  for (int i = static_cast<int>(s.length()) - 1; i >= 0; --i) {
    result.insert(result.begin(), s[i]);
    count++;
    if (count % 3 == 0 && i > 0) {
      result.insert(result.begin(), '.');
    }
  }
  return result + " ₫";
}

std::string formatNumber(long long number) {
  std::string s = std::to_string(number);
  std::string result = "";
  int count = 0;
  for (int i = static_cast<int>(s.length()) - 1; i >= 0; --i) {
    result.insert(result.begin(), s[i]);
    count++;
    if (count % 3 == 0 && i > 0) {
      result.insert(result.begin(), '.');
    }
  }
  return result;
}

// === HELPER OVERLAY & MODAL DIALOG DÙNG CHUNG ===

/**
 * @brief Lấy MainShellWindow hiện hành để hiển thị hộp thoại dạng In-Window
 * Overlay.
 * - Trả về con trỏ MainShellWindow đang hoạt động (từ g_activeShell hoặc
 * singleton).
 * - Lựa chọn fallback: Nếu hộp thoại phát sinh trước khi có MainShellWindow (ví
 * dụ: ở màn hình đăng nhập LoginScreen) hoặc phát sinh từ bên trong một cửa sổ
 * GtkWindow modal độc lập chưa chuyển đổi, hàm trả về nullptr để dùng GtkWindow
 * fallback nhằm tránh bị GTK modal grab khóa tương tác chuột.
 */
static MainShellWindow *resolveShellFromParent(GtkWindow *parent) {
  (void)parent;
  MainShellWindow *shell =
      g_activeShell ? g_activeShell : MainShellWindow::getActiveInstance();
  return shell;
}

struct ConfirmContext {
  MainShellWindow *shell;
  ConfirmCallback callback;
  gpointer userData;
};

static void on_confirm_click(GtkButton *btn, gpointer data) {
  (void)btn;
  ConfirmContext *ctx = static_cast<ConfirmContext *>(data);
  ConfirmCallback cb = ctx->callback;
  gpointer ud = ctx->userData;
  MainShellWindow *shell = ctx->shell;

  // Đóng overlay dialog TRƯỚC rồi mới chạy callback (để callback có thể mở
  // overlay/trang mới)
  if (shell) {
    shell->closeOverlayDialog();
  } else {
    closeOverlayDialog();
  }

  if (cb) {
    cb(ud);
  }
}

static void on_cancel_click(GtkButton *btn, gpointer data) {
  (void)btn;
  ConfirmContext *ctx = static_cast<ConfirmContext *>(data);
  if (ctx->shell) {
    ctx->shell->closeOverlayDialog();
  } else {
    closeOverlayDialog();
  }
}

static void showGeneralDialog(GtkWindow *parent, const std::string &icon,
                              const std::string &title,
                              const std::string &message,
                              const std::string &btnText = "Đóng",
                              const std::string &btnClass = "btn-primary") {
  MainShellWindow *shell = resolveShellFromParent(parent);

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "dialog-content");

  // Icon + Title Box
  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(headerBox, "overlay-dialog-header");

  GtkWidget *lblIcon = gtk_label_new(icon.c_str());
  gtk_widget_add_css_class(lblIcon, "overlay-dialog-icon");

  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "dialog-title");
  gtk_widget_add_css_class(lblTitle, "overlay-dialog-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_widget_set_hexpand(lblTitle, TRUE);

  gtk_box_append(GTK_BOX(headerBox), lblIcon);
  gtk_box_append(GTK_BOX(headerBox), lblTitle);
  gtk_box_append(GTK_BOX(card), headerBox);

  // Message
  GtkWidget *lblMsg = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(lblMsg, "dialog-message");
  gtk_widget_add_css_class(lblMsg, "overlay-dialog-message");
  gtk_label_set_wrap(GTK_LABEL(lblMsg), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(lblMsg), 48);
  gtk_label_set_xalign(GTK_LABEL(lblMsg), 0.0f);
  gtk_box_append(GTK_BOX(card), lblMsg);

  // Button Box
  GtkWidget *btnBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(btnBox, "overlay-dialog-actions");
  gtk_widget_set_halign(btnBox, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnBox, 8);

  GtkWidget *btn = gtk_button_new_with_label(btnText.c_str());
  gtk_widget_add_css_class(btn, btnClass.c_str());
  gtk_widget_add_css_class(btn, "overlay-dialog-btn");
  setButtonCursor(btn);
  gtk_box_append(GTK_BOX(btnBox), btn);
  gtk_box_append(GTK_BOX(card), btnBox);

  if (shell) {
    g_signal_connect_swapped(
        btn, "clicked", G_CALLBACK(+[](gpointer d) {
          static_cast<MainShellWindow *>(d)->closeOverlayDialog();
        }),
        shell);
    setupDialogShortcuts(card, btn, btn);
    shell->showOverlayDialog(card);
    return;
  }

  // Khi ở màn hình đăng nhập LoginScreen: hiển thị trên overlay của LoginScreen
  LoginScreen *login = LoginScreen::getActiveInstance();
  if (login) {
    g_signal_connect_swapped(
        btn, "clicked", G_CALLBACK(+[](gpointer d) {
          static_cast<LoginScreen *>(d)->closeOverlayDialog();
        }),
        login);
    setupDialogShortcuts(card, btn, btn);
    login->showOverlayDialog(card);
  }
}

void logError(const std::string &module, const std::string &technicalDetail) {
  try {
    g_mkdir_with_parents("data/system", 0755);
    std::ofstream logFile("data/system/error.log", std::ios::app);
    if (!logFile.is_open()) {
      logFile.open("error.log", std::ios::app);
    }
    if (logFile.is_open()) {
      auto now = std::chrono::system_clock::now();
      std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
      char timeBuf[64];
      std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S",
                    std::localtime(&nowTime));
      logFile << "[" << timeBuf << "] [" << module << "] " << technicalDetail
              << "\n";
      logFile.close();
    }
  } catch (...) {
  }
}

void setBusyCursor(GtkWidget *widget, bool busy) {
  if (!widget)
    return;
  if (busy) {
    gtk_widget_set_cursor_from_name(widget, "progress");
  } else {
    gtk_widget_set_cursor_from_name(widget, nullptr);
  }
}

void showToast(GtkWindow *parent, const std::string &message,
               const std::string &type) {
  (void)parent;
  MainShellWindow *shell =
      g_activeShell ? g_activeShell : MainShellWindow::getActiveInstance();
  if (shell) {
    shell->showToast(message, type);
    return;
  }
  LoginScreen *login = LoginScreen::getActiveInstance();
  if (login) {
    login->showToast(message, type);
  }
}

struct ChangePasswordContext {
  AppController *controller;
  MainShellWindow *shell;
  GtkWidget *eOld;
  GtkWidget *eNew;
  GtkWidget *eConfirm;
  GtkWidget *lblMsg;
};

void showChangePasswordDialog(GtkWindow *parent, AppController *controller) {
  MainShellWindow *shell = resolveShellFromParent(parent);
  if (!shell) {
    shell =
        g_activeShell ? g_activeShell : MainShellWindow::getActiveInstance();
  }
  if (!shell)
    return;

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_size_request(card, 480, -1);

  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(headerBox, "overlay-dialog-header");

  GtkWidget *lblIcon = gtk_label_new("🔑");
  gtk_widget_add_css_class(lblIcon, "overlay-dialog-icon");

  GtkWidget *lblTitle = gtk_label_new("Đổi mật khẩu");
  gtk_widget_add_css_class(lblTitle, "dialog-title");
  gtk_widget_add_css_class(lblTitle, "overlay-dialog-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_widget_set_hexpand(lblTitle, TRUE);

  gtk_box_append(GTK_BOX(headerBox), lblIcon);
  gtk_box_append(GTK_BOX(headerBox), lblTitle);
  gtk_box_append(GTK_BOX(card), headerBox);

  GtkWidget *eOld = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eOld), "Nhập mật khẩu hiện tại...");
  gtk_box_append(GTK_BOX(card), createFormField("Mật khẩu hiện tại *",
                                                createPasswordField(eOld)));

  GtkWidget *eNew = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eNew),
                                 "Tối thiểu 4 ký tự, khác mật khẩu cũ...");
  gtk_box_append(GTK_BOX(card),
                 createFormField("Mật khẩu mới *", createPasswordField(eNew)));

  GtkWidget *eConfirm = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eConfirm),
                                 "Nhập lại mật khẩu mới...");
  gtk_box_append(GTK_BOX(card), createFormField("Xác nhận mật khẩu mới *",
                                                createPasswordField(eConfirm)));

  GtkWidget *lblMsg = gtk_label_new("");
  gtk_widget_add_css_class(lblMsg, "badge-cancelled");
  gtk_label_set_wrap(GTK_LABEL(lblMsg), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(lblMsg), 45);
  gtk_widget_set_visible(lblMsg, FALSE);
  gtk_box_append(GTK_BOX(card), lblMsg);

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(btnRow, "overlay-dialog-actions");
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 8);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  gtk_widget_add_css_class(btnCancel, "overlay-dialog-btn");
  setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked", G_CALLBACK(+[](gpointer d) {
        static_cast<MainShellWindow *>(d)->closeOverlayDialog();
      }),
      shell);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  GtkWidget *btnSave = gtk_button_new_with_label("Đổi mật khẩu");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  gtk_widget_add_css_class(btnSave, "overlay-dialog-btn");
  setButtonCursor(btnSave);
  gtk_box_append(GTK_BOX(btnRow), btnSave);
  gtk_box_append(GTK_BOX(card), btnRow);

  ChangePasswordContext *ctx = new ChangePasswordContext{
      controller, shell, eOld, eNew, eConfirm, lblMsg};
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
        ChangePasswordContext *c = static_cast<ChangePasswordContext *>(d);
        std::string oldPw = gtk_editable_get_text(GTK_EDITABLE(c->eOld));
        std::string newPw = gtk_editable_get_text(GTK_EDITABLE(c->eNew));
        std::string confirmPw =
            gtk_editable_get_text(GTK_EDITABLE(c->eConfirm));

        auto showError = [c](const std::string &msg) {
          gtk_label_set_text(GTK_LABEL(c->lblMsg), msg.c_str());
          gtk_widget_set_visible(c->lblMsg, TRUE);
        };

        if (oldPw.empty() || newPw.empty() || confirmPw.empty()) {
          showError("Vui lòng nhập đầy đủ các trường bắt buộc (*)");
          return;
        }
        if (newPw != confirmPw) {
          showError("Mật khẩu xác nhận không khớp!");
          return;
        }
        OperationResult res = c->controller->changePassword(oldPw, newPw);
        if (!res.success) {
          showError(res.message);
          return;
        }
        MainShellWindow *targetShell = c->shell;
        GtkWindow *shellWin = targetShell ? targetShell->getWindow() : nullptr;
        std::string successMsg = res.message;
        if (targetShell) {
          targetShell->closeOverlayDialog();
        } else {
          closeOverlayDialog();
        }
        showSuccessDialog(shellWin, "Đổi mật khẩu", successMsg);
      }),
      ctx,
      [](gpointer d, GClosure *) {
        delete static_cast<ChangePasswordContext *>(d);
      },
      (GConnectFlags)0);

  setupDialogShortcuts(card, btnSave, btnCancel);
  shell->showOverlayDialog(card);
}

void showErrorDialog(GtkWindow *parent, const std::string &title,
                     const std::string &message) {
  std::string lower = message;
  for (auto &c : lower)
    c = std::tolower(c);

  bool isTechnical = (lower.find("exception") != std::string::npos ||
                      lower.find("std::") != std::string::npos ||
                      lower.find("failed to open") != std::string::npos ||
                      lower.find("cannot open") != std::string::npos ||
                      lower.find("errno") != std::string::npos ||
                      lower.find("bad_alloc") != std::string::npos ||
                      lower.find("ifstream") != std::string::npos ||
                      lower.find("ofstream") != std::string::npos ||
                      lower.find("database") != std::string::npos ||
                      lower.find(".txt") != std::string::npos);

  if (isTechnical) {
    logError(title.empty() ? "Database" : title, message);
    std::string friendlyMsg =
        "Không thể kết nối hoặc truy xuất tệp dữ liệu hệ thống.\n"
        "Chi tiết lỗi kỹ thuật đã được ghi lại trong tệp log hệ thống.\n"
        "Vui lòng kiểm tra lại quyền truy cập hoặc liên hệ quản trị viên.";
    showGeneralDialog(parent, "❌", "Lỗi dữ liệu hệ thống", friendlyMsg,
                      "Đã hiểu", "btn-secondary");
  } else {
    showGeneralDialog(parent, "❌", title.empty() ? "Có lỗi xảy ra" : title,
                      message, "Đã hiểu", "btn-secondary");
  }
}

void showSuccessDialog(GtkWindow *parent, const std::string &title,
                       const std::string &message) {
  (void)title;
  showToast(parent, message.empty() ? "Thao tác thành công!" : message,
            "success");
}

void showWarningDialog(GtkWindow *parent, const std::string &title,
                       const std::string &message) {
  showGeneralDialog(parent, "⚠️", title.empty() ? "Cảnh báo" : title, message,
                    "Tôi hiểu", "btn-secondary");
}

void showInfoDialog(GtkWindow *parent, const std::string &title,
                    const std::string &message) {
  showGeneralDialog(parent, "ℹ️", title.empty() ? "Thông báo" : title, message,
                    "Đóng", "btn-secondary");
}

void showServiceNotConnectedDialog(GtkWindow *parent,
                                   const std::string &featureName) {
  std::string msg = "Chức năng '" + featureName +
                    "' hiện đang hoạt động ở chế độ Prototype.\n"
                    "Giao diện đã sẵn sàng để tích hợp trực tiếp với Service "
                    "Layer trong các bước tiếp theo.";
  showGeneralDialog(parent, "🔌", "Chưa kết nối Service", msg, "Đã rõ",
                    "btn-secondary");
}

void showConfirmDialog(GtkWindow *parent, const std::string &title,
                       const std::string &message, ConfirmCallback onConfirm,
                       gpointer userData) {
  MainShellWindow *shell = resolveShellFromParent(parent);
  if (!shell) {
    shell =
        g_activeShell ? g_activeShell : MainShellWindow::getActiveInstance();
  }
  if (!shell)
    return;

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "dialog-content");

  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(headerBox, "overlay-dialog-header");

  GtkWidget *lblIcon = gtk_label_new("❓");
  gtk_widget_add_css_class(lblIcon, "overlay-dialog-icon");

  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "dialog-title");
  gtk_widget_add_css_class(lblTitle, "overlay-dialog-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_widget_set_hexpand(lblTitle, TRUE);

  gtk_box_append(GTK_BOX(headerBox), lblIcon);
  gtk_box_append(GTK_BOX(headerBox), lblTitle);
  gtk_box_append(GTK_BOX(card), headerBox);

  GtkWidget *lblMsg = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(lblMsg, "dialog-message");
  gtk_widget_add_css_class(lblMsg, "overlay-dialog-message");
  gtk_label_set_wrap(GTK_LABEL(lblMsg), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(lblMsg), 48);
  gtk_label_set_xalign(GTK_LABEL(lblMsg), 0.0f);
  gtk_box_append(GTK_BOX(card), lblMsg);

  ConfirmContext *ctx = new ConfirmContext{shell, onConfirm, userData};

  GtkWidget *btnBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(btnBox, "overlay-dialog-actions");
  gtk_widget_set_halign(btnBox, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnBox, 8);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy bỏ");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  gtk_widget_add_css_class(btnCancel, "overlay-dialog-btn");
  setButtonCursor(btnCancel);
  g_signal_connect(btnCancel, "clicked", G_CALLBACK(on_cancel_click), ctx);
  gtk_box_append(GTK_BOX(btnBox), btnCancel);

  GtkWidget *btnConfirm = gtk_button_new_with_label("Xác nhận");
  gtk_widget_add_css_class(btnConfirm, "btn-danger");
  gtk_widget_add_css_class(btnConfirm, "overlay-dialog-btn");
  setButtonCursor(btnConfirm);
  g_signal_connect_data(
      btnConfirm, "clicked", G_CALLBACK(on_confirm_click), ctx,
      [](gpointer d, GClosure *) { delete static_cast<ConfirmContext *>(d); },
      (GConnectFlags)0);
  gtk_box_append(GTK_BOX(btnBox), btnConfirm);

  gtk_box_append(GTK_BOX(card), btnBox);

  setupDialogShortcuts(card, btnConfirm, btnCancel);
  shell->showOverlayDialog(card);
}

// === TRẠNG THÁI HIỂN THỊ (EMPTY, LOADING, ERROR, SUCCESS) ===

GtkWidget *createEmptyState(const std::string &icon, const std::string &title,
                            const std::string &message,
                            const std::string &actionText, GCallback onAction,
                            gpointer actionData) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_add_css_class(box, "state-box");
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(box, TRUE);
  gtk_widget_set_vexpand(box, TRUE);

  GtkWidget *lblIcon = gtk_label_new(icon.empty() ? "📭" : icon.c_str());
  gtk_widget_set_margin_bottom(lblIcon, 4);
  gtk_box_append(GTK_BOX(box), lblIcon);

  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "state-title");
  gtk_label_set_justify(GTK_LABEL(lblTitle), GTK_JUSTIFY_CENTER);
  gtk_box_append(GTK_BOX(box), lblTitle);

  GtkWidget *lblMsg = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(lblMsg, "state-desc");
  gtk_label_set_justify(GTK_LABEL(lblMsg), GTK_JUSTIFY_CENTER);
  gtk_box_append(GTK_BOX(box), lblMsg);

  if (!actionText.empty() && onAction) {
    GtkWidget *btn = gtk_button_new_with_label(actionText.c_str());
    gtk_widget_add_css_class(btn, "btn-primary");
    setButtonCursor(btn);
    gtk_widget_set_halign(btn, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(btn, 10);
    g_signal_connect(btn, "clicked", onAction, actionData);
    gtk_box_append(GTK_BOX(box), btn);
  }

  return box;
}

GtkWidget *createLoadingState(const std::string &message) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(box, "state-box");
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(box, TRUE);
  gtk_widget_set_vexpand(box, TRUE);

  GtkWidget *spinner = gtk_spinner_new();
  gtk_spinner_start(GTK_SPINNER(spinner));
  gtk_widget_set_size_request(spinner, 40, 40);
  gtk_widget_set_halign(spinner, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), spinner);

  GtkWidget *lbl = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(lbl, "state-desc");
  gtk_label_set_justify(GTK_LABEL(lbl), GTK_JUSTIFY_CENTER);
  gtk_box_append(GTK_BOX(box), lbl);

  return box;
}

GtkWidget *createErrorState(const std::string &message, GCallback onRetry,
                            gpointer retryData) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_add_css_class(box, "state-box");
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

  GtkWidget *lblIcon = gtk_label_new("⚠️");
  gtk_box_append(GTK_BOX(box), lblIcon);

  GtkWidget *lblTitle = gtk_label_new("Không thể tải dữ liệu");
  gtk_widget_add_css_class(lblTitle, "state-title");
  gtk_box_append(GTK_BOX(box), lblTitle);

  GtkWidget *lblMsg = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(lblMsg, "state-desc");
  gtk_box_append(GTK_BOX(box), lblMsg);

  if (onRetry) {
    GtkWidget *btn = gtk_button_new_with_label("Thử lại");
    gtk_widget_add_css_class(btn, "btn-outline");
    setButtonCursor(btn);
    gtk_widget_set_halign(btn, GTK_ALIGN_CENTER);
    g_signal_connect(btn, "clicked", onRetry, retryData);
    gtk_box_append(GTK_BOX(box), btn);
  }

  return box;
}

GtkWidget *createSuccessBanner(const std::string &message) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_add_css_class(box, "card");
  gtk_widget_set_margin_bottom(box, 16);

  GtkWidget *icon = gtk_label_new("✅");
  GtkWidget *lbl = gtk_label_new(message.c_str());
  gtk_widget_set_hexpand(lbl, TRUE);
  gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);

  gtk_box_append(GTK_BOX(box), icon);
  gtk_box_append(GTK_BOX(box), lbl);
  return box;
}

// === TIỆN ÍCH GIAO DIỆN KHÁC ===

GtkWidget *createSvgIcon(const std::string &iconName, int pixelSize) {
  std::string fullPath = "UI/assets/icons/" + iconName;
  if (fullPath.size() < 4 || fullPath.substr(fullPath.size() - 4) != ".svg") {
    fullPath += ".svg";
  }
  if (g_file_test(fullPath.c_str(), G_FILE_TEST_EXISTS)) {
    GtkWidget *img = gtk_image_new_from_file(fullPath.c_str());
    gtk_image_set_pixel_size(GTK_IMAGE(img), pixelSize);
    return img;
  }
  return gtk_image_new_from_icon_name("image-missing");
}

GtkWidget *createClickableStatCard(const std::string &title, const std::string &value,
                                   const std::string &subtitle, const std::string &iconName,
                                   const std::string &colorClass, GCallback onClick,
                                   gpointer data) {
  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  gtk_widget_add_css_class(card, "stat-card");
  gtk_widget_add_css_class(card, "clickable");
  gtk_widget_set_hexpand(card, TRUE);
  setButtonCursor(card);
  gtk_widget_set_focusable(card, TRUE);

  std::string cardColorCss = "stat-card-blue";
  std::string iconColorCss = "stat-icon-blue";
  std::string trendColorCss = "";

  if (colorClass == "green" || colorClass == "success") {
    cardColorCss = "stat-card-green";
    iconColorCss = "stat-icon-green";
    trendColorCss = "stat-trend-positive";
  } else if (colorClass == "red" || colorClass == "danger") {
    cardColorCss = "stat-card-red";
    iconColorCss = "stat-icon-red";
    trendColorCss = "stat-trend-danger";
  } else if (colorClass == "amber" || colorClass == "warning" ||
             colorClass == "orange") {
    cardColorCss = "stat-card-amber";
    iconColorCss = "stat-icon-amber";
    trendColorCss = "stat-trend-warning";
  } else if (colorClass == "purple") {
    cardColorCss = "stat-card-purple";
    iconColorCss = "stat-icon-purple";
  } else if (colorClass == "cyan") {
    cardColorCss = "stat-card-cyan";
    iconColorCss = "stat-icon-cyan";
  }

  gtk_widget_add_css_class(card, cardColorCss.c_str());

  GtkWidget *topRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "stat-label");
  gtk_widget_set_hexpand(lblTitle, TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);

  GtkWidget *iconBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_add_css_class(iconBox, "stat-icon-box");
  gtk_widget_add_css_class(iconBox, iconColorCss.c_str());
  gtk_widget_set_valign(iconBox, GTK_ALIGN_CENTER);
  GtkWidget *iconImg = createSvgIcon(iconName, 20);
  gtk_box_append(GTK_BOX(iconBox), iconImg);

  gtk_box_append(GTK_BOX(topRow), lblTitle);
  gtk_box_append(GTK_BOX(topRow), iconBox);
  gtk_box_append(GTK_BOX(card), topRow);

  GtkWidget *lblVal = gtk_label_new(value.c_str());
  gtk_widget_add_css_class(lblVal, "stat-value");
  gtk_label_set_xalign(GTK_LABEL(lblVal), 0.0f);
  gtk_box_append(GTK_BOX(card), lblVal);

  if (!subtitle.empty()) {
    GtkWidget *lblSub = gtk_label_new(subtitle.c_str());
    gtk_widget_add_css_class(lblSub, "stat-trend");
    if (!trendColorCss.empty()) {
      gtk_widget_add_css_class(lblSub, trendColorCss.c_str());
    }
    gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
    gtk_box_append(GTK_BOX(card), lblSub);
  }

  if (onClick) {
    GtkGesture *click = gtk_gesture_click_new();
    g_signal_connect(click, "released", onClick, data);
    gtk_widget_add_controller(card, GTK_EVENT_CONTROLLER(click));
  }

  return card;
}

GtkWidget *createStatCard(const std::string &title, const std::string &value,
                          const std::string &subtitle, const std::string &icon,
                          const std::string &colorClass) {
  GtkWidget *card =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  gtk_widget_add_css_class(card, "stat-card");
  gtk_widget_set_hexpand(card, TRUE);

  std::string cardColorCss = "stat-card-blue";
  std::string iconColorCss = "stat-icon-blue";
  std::string trendColorCss = "";

  if (colorClass == "green" || colorClass == "success") {
    cardColorCss = "stat-card-green";
    iconColorCss = "stat-icon-green";
    trendColorCss = "stat-trend-positive";
  } else if (colorClass == "red" || colorClass == "danger") {
    cardColorCss = "stat-card-red";
    iconColorCss = "stat-icon-red";
    trendColorCss = "stat-trend-danger";
  } else if (colorClass == "amber" || colorClass == "warning" ||
             colorClass == "orange") {
    cardColorCss = "stat-card-amber";
    iconColorCss = "stat-icon-amber";
    trendColorCss = "stat-trend-warning";
  } else if (colorClass == "purple") {
    cardColorCss = "stat-card-purple";
    iconColorCss = "stat-icon-purple";
  } else if (colorClass == "cyan") {
    cardColorCss = "stat-card-cyan";
    iconColorCss = "stat-icon-cyan";
  }

  gtk_widget_add_css_class(card, cardColorCss.c_str());

  GtkWidget *topRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "stat-label");
  gtk_widget_set_hexpand(lblTitle, TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);

  GtkWidget *lblIcon = gtk_label_new(icon.c_str());
  gtk_widget_add_css_class(lblIcon, "stat-icon-box");
  gtk_widget_add_css_class(lblIcon, iconColorCss.c_str());
  gtk_widget_set_valign(lblIcon, GTK_ALIGN_CENTER);

  gtk_box_append(GTK_BOX(topRow), lblTitle);
  gtk_box_append(GTK_BOX(topRow), lblIcon);
  gtk_box_append(GTK_BOX(card), topRow);

  GtkWidget *lblVal = gtk_label_new(value.c_str());
  gtk_widget_add_css_class(lblVal, "stat-value");
  gtk_label_set_xalign(GTK_LABEL(lblVal), 0.0f);
  gtk_box_append(GTK_BOX(card), lblVal);

  if (!subtitle.empty()) {
    GtkWidget *lblSub = gtk_label_new(subtitle.c_str());
    gtk_widget_add_css_class(lblSub, "stat-trend");
    if (!trendColorCss.empty()) {
      gtk_widget_add_css_class(lblSub, trendColorCss.c_str());
    }
    gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
    gtk_box_append(GTK_BOX(card), lblSub);
  }

  return card;
}

GtkWidget *createStatusBadge(const std::string &text,
                             const std::string &badgeClass) {
  GtkWidget *badge = gtk_label_new(text.c_str());
  gtk_widget_add_css_class(badge, "badge");
  gtk_widget_add_css_class(badge, badgeClass.c_str());
  return badge;
}

GtkWidget *createOrderStatusBadge(OrderStatus status) {
  switch (status) {
  case OrderStatus::PENDING:
    return createStatusBadge("Chờ xử lý", "badge-pending");
  case OrderStatus::CONFIRMED:
    return createStatusBadge("Đã xác nhận", "badge-confirmed");
  case OrderStatus::PREPARING:
    return createStatusBadge("Đang đóng gói", "badge-preparing");
  case OrderStatus::READY_FOR_PICKUP:
    return createStatusBadge("Sẵn sàng nhận", "badge-ready");
  case OrderStatus::SHIPPING:
    return createStatusBadge("Đang giao hàng", "badge-shipping");
  case OrderStatus::COMPLETED:
    return createStatusBadge("Đã bán", "badge-sold");
  case OrderStatus::CANCELLED:
    return createStatusBadge("Đã trả / Hủy", "badge-returned");
  default:
    return createStatusBadge("Chưa rõ", "badge-secondary");
  }
}

GtkWidget *createPaymentStatusBadge(PaymentStatus status) {
  if (status == PaymentStatus::PAID) {
    return createStatusBadge("Đã thanh toán", "badge-paid");
  } else {
    return createStatusBadge("Chưa thanh toán", "badge-unpaid");
  }
}

GtkWidget *createStockStatusBadge(StockDisplayStatus status) {
  switch (status) {
  case StockDisplayStatus::IN_STOCK:
    return createStatusBadge("Còn hàng", "badge-in-stock");
  case StockDisplayStatus::LOW_STOCK:
    return createStatusBadge("Sắp hết hàng", "badge-low-stock");
  case StockDisplayStatus::OUT_OF_STOCK:
    return createStatusBadge("Hết hàng", "badge-out-of-stock");
  default:
    return createStatusBadge("Không rõ", "badge-secondary");
  }
}

GtkWidget *createWarrantyStatusBadge(WarrantyStatus status) {
  switch (status) {
  case WarrantyStatus::RECEIVED:
    return createStatusBadge("Mới tiếp nhận", "badge-pending");
  case WarrantyStatus::PROCESSING:
    return createStatusBadge("Đang bảo hành", "badge-under-warranty");
  case WarrantyStatus::RESOLVED:
    return createStatusBadge("Đã trả máy", "badge-returned");
  case WarrantyStatus::REJECTED:
    return createStatusBadge("Từ chối", "badge-out-of-stock");
  default:
    return createStatusBadge("Không rõ", "badge-secondary");
  }
}

GtkWidget *createTableHeaderRow(const std::vector<TableColumnSpec> &columns) {
  GtkWidget *row =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::TABLE_CELL_SPACING);
  gtk_widget_add_css_class(row, "table-header");
  for (const auto &col : columns) {
    GtkWidget *lbl = gtk_label_new(col.title.c_str());
    gtk_widget_add_css_class(lbl, "table-header-cell");
    if (!col.title.empty()) {
      gtk_widget_set_tooltip_text(lbl, col.title.c_str());
    }
    if (col.minWidth > 0) {
      gtk_widget_set_size_request(lbl, col.minWidth, -1);
    } else if (col.hexpand) {
      gtk_widget_set_size_request(lbl, 180, -1);
    }
    if (col.hexpand) {
      gtk_widget_set_hexpand(lbl, TRUE);
    }
    gtk_label_set_xalign(GTK_LABEL(lbl), col.alignment);
    gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(lbl), 1);
    gtk_box_append(GTK_BOX(row), lbl);
  }
  return row;
}

GtkWidget *
createScrollableTableContainer(const std::vector<TableColumnSpec> &columns,
                               GtkWidget **outBodyListBox, int minBodyHeight,
                               int maxBodyHeight) {
  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_widget_set_hexpand(tableContainer, TRUE);

  GtkWidget *hScroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(hScroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
  gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(hScroll),
                                                  FALSE);
  gtk_widget_set_hexpand(hScroll, TRUE);

  GtkWidget *innerTableBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(innerTableBox, TRUE);

  GtkWidget *headerRow = createTableHeaderRow(columns);
  gtk_box_append(GTK_BOX(innerTableBox), headerRow);

  GtkWidget *bodyScroll = gtk_scrolled_window_new();
  gtk_widget_add_css_class(bodyScroll, "table-body-scroll");
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(bodyScroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(bodyScroll),
                                            TRUE);
  if (minBodyHeight > 0) {
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(bodyScroll),
                                               minBodyHeight);
  }
  if (maxBodyHeight > 0) {
    gtk_scrolled_window_set_propagate_natural_height(
        GTK_SCROLLED_WINDOW(bodyScroll), TRUE);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(bodyScroll),
                                               maxBodyHeight);
  } else {
    gtk_widget_set_vexpand(tableContainer, TRUE);
    gtk_widget_set_vexpand(hScroll, TRUE);
    gtk_widget_set_vexpand(innerTableBox, TRUE);
    gtk_widget_set_vexpand(bodyScroll, TRUE);
    gtk_scrolled_window_set_propagate_natural_height(
        GTK_SCROLLED_WINDOW(bodyScroll), FALSE);
  }

  GtkWidget *bodyListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(bodyListBox, "table-body-list");
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(bodyScroll), bodyListBox);
  gtk_box_append(GTK_BOX(innerTableBox), bodyScroll);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(hScroll), innerTableBox);
  gtk_box_append(GTK_BOX(tableContainer), hScroll);

  if (outBodyListBox) {
    *outBodyListBox = bodyListBox;
  }
  return tableContainer;
}

GtkWidget *createTableDataRow(size_t rowIndex) {
  GtkWidget *row =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::TABLE_CELL_SPACING);
  gtk_widget_add_css_class(row, "table-row");
  gtk_widget_add_css_class(row, (rowIndex % 2 == 0) ? "row-even" : "row-odd");
  return row;
}

void applyColumnSpec(GtkWidget *cellWidget, const TableColumnSpec &spec) {
  if (!cellWidget)
    return;
  if (spec.minWidth > 0) {
    gtk_widget_set_size_request(cellWidget, spec.minWidth, -1);
  } else if (spec.hexpand) {
    gtk_widget_set_size_request(cellWidget, 180, -1);
  }
  gtk_widget_set_hexpand(cellWidget, spec.hexpand ? TRUE : FALSE);
  gtk_widget_set_valign(cellWidget, GTK_ALIGN_CENTER);
  if (GTK_IS_LABEL(cellWidget)) {
    gtk_label_set_xalign(GTK_LABEL(cellWidget), spec.alignment);
    gtk_label_set_ellipsize(GTK_LABEL(cellWidget), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(cellWidget), 1);
  }
}

GtkWidget *createTableTextCell(const std::string &text,
                               const TableColumnSpec &spec,
                               const std::string &cssClass) {
  GtkWidget *lbl = gtk_label_new(text.c_str());
  if (!text.empty()) {
    gtk_widget_set_tooltip_text(lbl, text.c_str());
  }
  gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
  applyColumnSpec(lbl, spec);
  if (!cssClass.empty()) {
    gtk_widget_add_css_class(lbl, cssClass.c_str());
  } else if (spec.alignment >= 0.9f) {
    gtk_widget_add_css_class(lbl, "cell-number");
  }
  return lbl;
}

GtkWidget *createTableTwoLineCell(const std::string &primaryText,
                                  const std::string &secondaryText,
                                  const TableColumnSpec &spec) {
  GtkWidget *box =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  applyColumnSpec(box, spec);
  if (!primaryText.empty() || !secondaryText.empty()) {
    std::string tip = secondaryText.empty()
                          ? primaryText
                          : (primaryText + "\n" + secondaryText);
    gtk_widget_set_tooltip_text(box, tip.c_str());
  }

  GtkWidget *lblPrimary = gtk_label_new(primaryText.c_str());
  gtk_widget_add_css_class(lblPrimary, "cell-title");
  gtk_label_set_ellipsize(GTK_LABEL(lblPrimary), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars(GTK_LABEL(lblPrimary), 1);
  gtk_label_set_xalign(GTK_LABEL(lblPrimary), spec.alignment);
  gtk_box_append(GTK_BOX(box), lblPrimary);

  if (!secondaryText.empty()) {
    GtkWidget *lblSecondary = gtk_label_new(secondaryText.c_str());
    gtk_widget_add_css_class(lblSecondary, "cell-subtitle");
    gtk_label_set_ellipsize(GTK_LABEL(lblSecondary), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(lblSecondary), 1);
    gtk_label_set_xalign(GTK_LABEL(lblSecondary), spec.alignment);
    gtk_box_append(GTK_BOX(box), lblSecondary);
  }

  return box;
}

GtkWidget *createTableWidgetCell(GtkWidget *innerWidget,
                                 const TableColumnSpec &spec) {
  GtkWidget *slot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  applyColumnSpec(slot, spec);

  if (innerWidget) {
    if (spec.alignment < 0.25f) {
      gtk_widget_set_halign(innerWidget, GTK_ALIGN_START);
    } else if (spec.alignment > 0.75f) {
      gtk_widget_set_halign(innerWidget, GTK_ALIGN_END);
    } else {
      gtk_widget_set_halign(innerWidget, GTK_ALIGN_CENTER);
    }
    gtk_widget_set_hexpand(innerWidget, TRUE);
    gtk_widget_set_valign(innerWidget, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(slot), innerWidget);
  }
  return slot;
}

GtkWidget *createTableActionButton(const std::string &label,
                                   const std::string &tooltip,
                                   const std::string &btnClass,
                                   GCallback onClick, gpointer data) {
  GtkWidget *btn = gtk_button_new_with_label(label.c_str());
  gtk_widget_add_css_class(btn, "btn-action-sm");
  if (!btnClass.empty()) {
    gtk_widget_add_css_class(btn, btnClass.c_str());
  }
  if (!tooltip.empty()) {
    gtk_widget_set_tooltip_text(btn, tooltip.c_str());
  }
  setButtonCursor(btn);
  gtk_widget_set_valign(btn, GTK_ALIGN_CENTER);
  if (onClick) {
    g_signal_connect(btn, "clicked", onClick, data);
  }
  return btn;
}

GtkWidget *createSearchBar(const std::string &placeholder,
                           GCallback onSearchChanged, gpointer data,
                           GtkWidget **outEntry) {
  GtkWidget *box =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), placeholder.c_str());
  gtk_widget_set_hexpand(entry, TRUE);

  if (onSearchChanged) {
    g_signal_connect(entry, "changed", onSearchChanged, data);
  }
  if (outEntry) {
    *outEntry = entry;
  }

  gtk_box_append(GTK_BOX(box), entry);
  return box;
}

GtkWidget *createFilterToolbar(const std::string &searchPlaceholder,
                               GCallback onSearchChanged, gpointer data,
                               GtkWidget **outEntry, GtkWidget **outCountBadge,
                               GtkWidget *extraControlsBox) {
  GtkWidget *toolbar =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(toolbar, "filter-toolbar");

  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), searchPlaceholder.c_str());
  gtk_widget_set_hexpand(entry, TRUE);

  if (onSearchChanged) {
    g_signal_connect(entry, "changed", onSearchChanged, data);
  }
  if (outEntry) {
    *outEntry = entry;
  }
  gtk_box_append(GTK_BOX(toolbar), entry);

  if (extraControlsBox) {
    gtk_widget_set_valign(extraControlsBox, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(toolbar), extraControlsBox);
  }

  if (outCountBadge) {
    GtkWidget *lblCount = gtk_label_new("Hiển thị: 0 bản ghi");
    gtk_widget_add_css_class(lblCount, "record-count-badge");
    gtk_widget_set_valign(lblCount, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(toolbar), lblCount);
    *outCountBadge = lblCount;
  }

  return toolbar;
}

void updateRecordCountBadge(GtkWidget *countBadgeLabel, size_t shownCount,
                            const std::string &unitName) {
  if (!countBadgeLabel || !GTK_IS_LABEL(countBadgeLabel))
    return;
  std::string text =
      "Hiển thị: " + formatNumber(static_cast<long long>(shownCount)) + " " +
      unitName;
  gtk_label_set_text(GTK_LABEL(countBadgeLabel), text.c_str());
}

GtkWidget *createFormField(const std::string &labelText, GtkWidget *inputWidget,
                           GtkWidget **outErrorLabel) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  GtkWidget *lbl = gtk_label_new(nullptr);
  gtk_widget_add_css_class(lbl, "form-label");
  gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);

  size_t starPos = labelText.find('*');
  if (starPos != std::string::npos) {
    std::string before = labelText.substr(0, starPos);
    std::string after =
        (starPos + 1 < labelText.length()) ? labelText.substr(starPos + 1) : "";
    std::string markup =
        before + "<span foreground='red' font_weight='bold'>*</span>" + after;
    gtk_label_set_markup(GTK_LABEL(lbl), markup.c_str());
  } else {
    gtk_label_set_text(GTK_LABEL(lbl), labelText.c_str());
  }

  gtk_box_append(GTK_BOX(box), lbl);
  gtk_box_append(GTK_BOX(box), inputWidget);

  GtkWidget *errLbl = gtk_label_new("");
  gtk_widget_add_css_class(errLbl, "form-error-label");
  gtk_label_set_xalign(GTK_LABEL(errLbl), 0.0f);
  gtk_widget_set_visible(errLbl, FALSE);
  gtk_box_append(GTK_BOX(box), errLbl);

  if (inputWidget) {
    g_object_set_data(G_OBJECT(inputWidget), "pbl-error-label", errLbl);
    g_object_set_data(G_OBJECT(inputWidget), "pbl-field-label", lbl);
  }
  if (outErrorLabel) {
    *outErrorLabel = errLbl;
  }
  return box;
}

GtkWidget *createPasswordField(GtkWidget *entryWidget) {
  g_return_val_if_fail(GTK_IS_ENTRY(entryWidget), nullptr);

  gtk_entry_set_visibility(GTK_ENTRY(entryWidget), FALSE);
  gtk_widget_set_hexpand(entryWidget, TRUE);

  GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_append(GTK_BOX(row), entryWidget);

  GtkWidget *button = gtk_button_new();
  gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
  gtk_widget_set_tooltip_text(button, "Rê chuột để hiện mật khẩu");
  setButtonCursor(button);

  GtkWidget *icon = gtk_image_new_from_icon_name("view-conceal-symbolic");
  gtk_image_set_pixel_size(GTK_IMAGE(icon), 22);
  gtk_button_set_child(GTK_BUTTON(button), icon);
  g_object_set_data(G_OBJECT(button), "password-entry", entryWidget);
  g_object_set_data(G_OBJECT(button), "password-icon", icon);

  GtkEventController *motionController = gtk_event_controller_motion_new();
  g_signal_connect(
      motionController, "enter",
      G_CALLBACK(+[](GtkEventControllerMotion *controller, double, double,
                     gpointer) {
        GtkWidget *button =
            gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
        GtkWidget *entry =
            GTK_WIDGET(g_object_get_data(G_OBJECT(button), "password-entry"));
        GtkWidget *icon =
            GTK_WIDGET(g_object_get_data(G_OBJECT(button), "password-icon"));
        gtk_entry_set_visibility(GTK_ENTRY(entry), TRUE);
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), "view-reveal-symbolic");
      }),
      nullptr);
  g_signal_connect(
      motionController, "leave",
      G_CALLBACK(+[](GtkEventControllerMotion *controller, gpointer) {
        GtkWidget *button =
            gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
        GtkWidget *entry =
            GTK_WIDGET(g_object_get_data(G_OBJECT(button), "password-entry"));
        GtkWidget *icon =
            GTK_WIDGET(g_object_get_data(G_OBJECT(button), "password-icon"));
        gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), "view-conceal-symbolic");
      }),
      nullptr);
  gtk_widget_add_controller(button, motionController);
  gtk_box_append(GTK_BOX(row), button);

  return row;
}

void setFieldError(GtkWidget *errorLabel, GtkWidget *inputWidget,
                   const std::string &errorMessage) {
  if (!errorLabel && inputWidget) {
    errorLabel =
        GTK_WIDGET(g_object_get_data(G_OBJECT(inputWidget), "pbl-error-label"));
  }
  if (errorLabel) {
    gtk_label_set_text(GTK_LABEL(errorLabel), errorMessage.c_str());
    gtk_widget_set_visible(errorLabel, TRUE);
  }
  if (inputWidget) {
    gtk_widget_add_css_class(inputWidget, "input-error");
  }
}

void clearFieldError(GtkWidget *errorLabel, GtkWidget *inputWidget) {
  if (!errorLabel && inputWidget) {
    errorLabel =
        GTK_WIDGET(g_object_get_data(G_OBJECT(inputWidget), "pbl-error-label"));
  }
  if (errorLabel) {
    gtk_label_set_text(GTK_LABEL(errorLabel), "");
    gtk_widget_set_visible(errorLabel, FALSE);
  }
  if (inputWidget) {
    gtk_widget_remove_css_class(inputWidget, "input-error");
  }
}

void setupCurrencyInput(GtkWidget *entryWidget) {
  if (!entryWidget || !GTK_IS_EDITABLE(entryWidget))
    return;

  // Định dạng giá trị ban đầu nếu có sẵn số
  const char *initText = gtk_editable_get_text(GTK_EDITABLE(entryWidget));
  std::string initDigits;
  for (const char *p = initText; *p; ++p) {
    if (g_ascii_isdigit(*p))
      initDigits += *p;
  }
  if (!initDigits.empty()) {
    try {
      long long val = std::stoll(initDigits);
      std::string formatted = UIComponents::formatNumber(val);
      gtk_editable_set_text(GTK_EDITABLE(entryWidget), formatted.c_str());
    } catch (...) {
    }
  }

  // Lắng nghe sự kiện rời ô (focus-leave) và vào ô (focus-enter)
  GtkEventController *focusCtrl = gtk_event_controller_focus_new();

  // Rời ô: Tự động gom chữ số và định dạng dấu chấm phân cách hàng nghìn
  g_signal_connect(
      focusCtrl, "leave",
      G_CALLBACK(+[](GtkEventControllerFocus *ctrl, gpointer) {
        GtkWidget *w =
            gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(ctrl));
        if (!w || !GTK_IS_EDITABLE(w))
          return;
        const char *text = gtk_editable_get_text(GTK_EDITABLE(w));
        std::string digits;
        for (const char *p = text; *p; ++p) {
          if (g_ascii_isdigit(*p))
            digits += *p;
        }
        if (!digits.empty()) {
          try {
            long long val = std::stoll(digits);
            std::string formatted = UIComponents::formatNumber(val);
            gtk_editable_set_text(GTK_EDITABLE(w), formatted.c_str());
          } catch (...) {
          }
        }
      }),
      nullptr);

  // Vào ô: Bỏ dấu chấm để tiện gõ/chỉnh sửa
  g_signal_connect(focusCtrl, "enter",
                   G_CALLBACK(+[](GtkEventControllerFocus *ctrl, gpointer) {
                     GtkWidget *w = gtk_event_controller_get_widget(
                         GTK_EVENT_CONTROLLER(ctrl));
                     if (!w || !GTK_IS_EDITABLE(w))
                       return;
                     const char *text = gtk_editable_get_text(GTK_EDITABLE(w));
                     std::string digits;
                     for (const char *p = text; *p; ++p) {
                       if (g_ascii_isdigit(*p))
                         digits += *p;
                     }
                     if (!digits.empty()) {
                       gtk_editable_set_text(GTK_EDITABLE(w), digits.c_str());
                       gtk_editable_set_position(GTK_EDITABLE(w), -1);
                     }
                   }),
                   nullptr);

  gtk_widget_add_controller(entryWidget, focusCtrl);

  // Khi gõ phím: Chỉ cho phép số và dấu chấm phân cách
  g_signal_connect(entryWidget, "changed",
                   G_CALLBACK(+[](GtkEditable *ed, gpointer) {
                     static bool in_change = false;
                     if (in_change)
                       return;
                     const char *text = gtk_editable_get_text(ed);
                     std::string clean;
                     bool has_invalid = false;
                     for (const char *p = text; *p; ++p) {
                       if (g_ascii_isdigit(*p) || *p == '.') {
                         clean += *p;
                       } else {
                         has_invalid = true;
                       }
                     }
                     if (has_invalid) {
                       in_change = true;
                       int pos = gtk_editable_get_position(ed);
                       gtk_editable_set_text(ed, clean.c_str());
                       gtk_editable_set_position(ed, pos > 0 ? pos - 1 : 0);
                       in_change = false;
                     }
                   }),
                   nullptr);
}

double parseCurrencyInput(GtkWidget *entryWidget) {
  if (!entryWidget || !GTK_IS_EDITABLE(entryWidget))
    return 0.0;
  const char *raw = gtk_editable_get_text(GTK_EDITABLE(entryWidget));
  std::string digits;
  for (const char *p = raw; *p; ++p) {
    if (g_ascii_isdigit(*p))
      digits += *p;
  }
  if (digits.empty())
    return 0.0;
  try {
    return std::stod(digits);
  } catch (...) {
    return 0.0;
  }
}

void setupPhoneInput(GtkWidget *entryWidget) {
  if (!entryWidget || !GTK_IS_EDITABLE(entryWidget))
    return;
  if (GTK_IS_ENTRY(entryWidget)) {
    gtk_entry_set_max_length(GTK_ENTRY(entryWidget), 10);
  }
  g_signal_connect(
      entryWidget, "changed", G_CALLBACK(+[](GtkEditable *ed, gpointer) {
        static bool in_change = false;
        if (in_change)
          return;
        const char *text = gtk_editable_get_text(ed);
        std::string digits;
        bool has_invalid = false;
        for (const char *p = text; *p; ++p) {
          if (g_ascii_isdigit(*p)) {
            if (digits.length() < 10) {
              digits += *p;
            }
          } else {
            has_invalid = true;
          }
        }
        if (has_invalid || strlen(text) > 10) {
          in_change = true;
          int pos = gtk_editable_get_position(ed);
          gtk_editable_set_text(ed, digits.c_str());
          gtk_editable_set_position(ed, std::min(pos, (int)digits.length()));
          in_change = false;
        }
      }),
      nullptr);
}

bool validatePhoneNumber(const std::string &phone, std::string &outErrorMsg) {
  if (phone.empty()) {
    outErrorMsg = "Số điện thoại không được để trống";
    return false;
  }
  if (phone[0] != '0') {
    outErrorMsg = "Số điện thoại phải bắt đầu bằng số 0";
    return false;
  }
  if (phone.length() != 10) {
    outErrorMsg = "Số điện thoại phải có đúng 10 chữ số (hiện có " +
                  std::to_string(phone.length()) + " số)";
    return false;
  }
  for (char c : phone) {
    if (!g_ascii_isdigit(c)) {
      outErrorMsg = "Số điện thoại chỉ được chứa chữ số";
      return false;
    }
  }
  return true;
}

void setupDialogShortcuts(GtkWidget *window, GtkWidget *btnSave,
                          GtkWidget *btnCancel) {
  if (!window)
    return;
  GtkEventController *keyCtrl = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(keyCtrl, GTK_PHASE_CAPTURE);
  struct ShortcutCtx {
    GtkWidget *saveBtn;
    GtkWidget *cancelBtn;
    GtkWidget *win;
  };
  ShortcutCtx *sctx = new ShortcutCtx{btnSave, btnCancel, window};
  g_signal_connect_data(
      keyCtrl, "key-pressed",
      G_CALLBACK(+[](GtkEventControllerKey *ctrl, guint keyval, guint,
                     GdkModifierType, gpointer d) -> gboolean {
        (void)ctrl;
        ShortcutCtx *sc = static_cast<ShortcutCtx *>(d);
        if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
          GtkWidget *focus = nullptr;
          if (GTK_IS_WINDOW(sc->win)) {
            focus = gtk_window_get_focus(GTK_WINDOW(sc->win));
          } else if (sc->win) {
            GtkRoot *root = gtk_widget_get_root(sc->win);
            if (root && GTK_IS_WINDOW(root)) {
              focus = gtk_window_get_focus(GTK_WINDOW(root));
            }
          }
          if (focus && (GTK_IS_TEXT_VIEW(focus) ||
                        (GTK_IS_BUTTON(focus) && focus != sc->saveBtn))) {
            return FALSE;
          }
          if (sc->saveBtn && gtk_widget_get_sensitive(sc->saveBtn)) {
            g_signal_emit_by_name(sc->saveBtn, "clicked");
            return TRUE;
          }
        } else if (keyval == GDK_KEY_Escape) {
          if (sc->cancelBtn && gtk_widget_get_sensitive(sc->cancelBtn)) {
            g_signal_emit_by_name(sc->cancelBtn, "clicked");
            return TRUE;
          } else if (sc->win && GTK_IS_WINDOW(sc->win)) {
            gtk_window_destroy(GTK_WINDOW(sc->win));
            return TRUE;
          } else if (isOverlayDialogOpen()) {
            closeOverlayDialog();
            return TRUE;
          }
        }
        return FALSE;
      }),
      sctx,
      [](gpointer d, GClosure *) { delete static_cast<ShortcutCtx *>(d); },
      (GConnectFlags)0);

  gtk_widget_add_controller(window, keyCtrl);
}

GtkWidget *createSectionHeader(const std::string &title,
                               const std::string &subtitle,
                               GtkWidget *actionWidget) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(box, "page-header-box");

  GtkWidget *titlesBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_set_hexpand(titlesBox, TRUE);

  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "page-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(titlesBox), lblTitle);

  if (!subtitle.empty()) {
    GtkWidget *lblSub = gtk_label_new(subtitle.c_str());
    gtk_widget_add_css_class(lblSub, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
    gtk_box_append(GTK_BOX(titlesBox), lblSub);
  }

  gtk_box_append(GTK_BOX(box), titlesBox);

  if (actionWidget) {
    gtk_widget_set_valign(actionWidget, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), actionWidget);
  }

  return box;
}

GtkWidget *createRankBadge(const std::string &membershipTier) {
  if (membershipTier.find("Kim Cương") != std::string::npos ||
      membershipTier.find("VIP") != std::string::npos) {
    return createStatusBadge("💎 " + membershipTier, "rank-diamond");
  } else if (membershipTier.find("Vàng") != std::string::npos) {
    return createStatusBadge("🥇 " + membershipTier, "rank-gold");
  } else if (membershipTier.find("Bạc") != std::string::npos) {
    return createStatusBadge("🥈 " + membershipTier, "rank-silver");
  } else {
    return createStatusBadge("🥉 " + membershipTier, "rank-bronze");
  }
}

GtkWidget *createBackHeader(const std::string &title,
                            const std::string &subtitle, GCallback onBack,
                            gpointer data, GtkWidget *actionWidget) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
  gtk_widget_add_css_class(box, "page-header-box");

  // Nút quay lại với icon mũi tên rõ ràng
  GtkWidget *btnBack = gtk_button_new_with_label("← Quay lại");
  gtk_widget_add_css_class(btnBack, "btn-back");
  setButtonCursor(btnBack);
  gtk_widget_set_valign(btnBack, GTK_ALIGN_CENTER);
  if (onBack) {
    g_signal_connect(btnBack, "clicked", onBack, data);
  }
  gtk_box_append(GTK_BOX(box), btnBack);

  GtkWidget *titlesBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_set_hexpand(titlesBox, TRUE);

  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "page-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(titlesBox), lblTitle);

  if (!subtitle.empty()) {
    GtkWidget *lblSub = gtk_label_new(subtitle.c_str());
    gtk_widget_add_css_class(lblSub, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
    gtk_box_append(GTK_BOX(titlesBox), lblSub);
  }

  gtk_box_append(GTK_BOX(box), titlesBox);

  if (actionWidget) {
    gtk_widget_set_valign(actionWidget, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), actionWidget);
  }

  return box;
}

void setButtonCursor(GtkWidget *btn) {
  if (btn) {
    gtk_widget_set_cursor_from_name(btn, "pointer");
  }
}

// === TIỆN ÍCH BIỂU ĐỒ & THỐNG KÊ (DESIGN.MD SECTION 8) ===

std::string formatCompactVND(double amount) {
  if (amount <= 0.0) return "0 đ";
  if (amount >= 1e9) {
    char buf[32];
    double b = amount / 1e9;
    if (std::abs(b - std::round(b)) < 0.05) {
      snprintf(buf, sizeof(buf), "%lld tỷ", static_cast<long long>(std::round(b)));
    } else {
      snprintf(buf, sizeof(buf), "%.1f tỷ", b);
    }
    return buf;
  }
  if (amount >= 1e6) {
    char buf[32];
    double m = amount / 1e6;
    if (std::abs(m - std::round(m)) < 0.05) {
      snprintf(buf, sizeof(buf), "%lld tr", static_cast<long long>(std::round(m)));
    } else {
      snprintf(buf, sizeof(buf), "%.1f tr", m);
    }
    return buf;
  }
  if (amount >= 1e3) {
    char buf[32];
    double k = amount / 1e3;
    if (std::abs(k - std::round(k)) < 0.05) {
      snprintf(buf, sizeof(buf), "%lld k", static_cast<long long>(std::round(k)));
    } else {
      snprintf(buf, sizeof(buf), "%.0f k", k);
    }
    return buf;
  }
  return std::to_string(static_cast<long long>(amount)) + " đ";
}

GtkWidget* createRevenueBarChart(const std::vector<TimelineStatPoint>& points) {
  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "card");
  gtk_widget_add_css_class(card, "chart-card");

  // Header Box: Tiêu đề + Chú thích (Legend)
  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  GtkWidget *titleCol = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);

  GtkWidget *lblTitle = gtk_label_new("📊 Biểu đồ Doanh thu & Lợi nhuận");
  gtk_widget_add_css_class(lblTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(titleCol), lblTitle);

  GtkWidget *lblSub = gtk_label_new("Theo dõi xu hướng dòng tiền và lợi nhuận gộp theo mốc thời gian.");
  gtk_widget_add_css_class(lblSub, "chart-subtitle");
  gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
  gtk_box_append(GTK_BOX(titleCol), lblSub);
  gtk_box_append(GTK_BOX(headerBox), titleCol);

  // Spacer
  GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_hexpand(spacer, TRUE);
  gtk_box_append(GTK_BOX(headerBox), spacer);

  // Legend
  GtkWidget *legendBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  gtk_widget_set_valign(legendBox, GTK_ALIGN_CENTER);

  auto addLegend = [&](const std::string& name, const std::string& dotClass) {
    GtkWidget *item = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_XS);
    GtkWidget *dot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(dot, "chart-legend-dot");
    gtk_widget_add_css_class(dot, dotClass.c_str());
    gtk_widget_set_size_request(dot, 10, 10);
    gtk_widget_set_valign(dot, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(item), dot);

    GtkWidget *lbl = gtk_label_new(name.c_str());
    gtk_widget_add_css_class(lbl, "chart-legend-item");
    gtk_box_append(GTK_BOX(item), lbl);
    gtk_box_append(GTK_BOX(legendBox), item);
  };

  addLegend("Doanh thu", "chart-legend-dot--revenue");
  addLegend("Lợi nhuận gộp", "chart-legend-dot--profit");
  gtk_box_append(GTK_BOX(headerBox), legendBox);

  gtk_box_append(GTK_BOX(card), headerBox);

  // Drawing Area
  GtkWidget *drawingArea = gtk_drawing_area_new();
  gtk_widget_set_size_request(drawingArea, -1, 260);
  gtk_widget_set_hexpand(drawingArea, TRUE);

  auto *pointsCopy = new std::vector<TimelineStatPoint>(points);
  gtk_drawing_area_set_draw_func(
      GTK_DRAWING_AREA(drawingArea),
      +[](GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer userData) {
        (void)area;
        auto *pts = static_cast<const std::vector<TimelineStatPoint> *>(userData);

        const double padLeft = 65.0;
        const double padRight = 25.0;
        const double padTop = 35.0;
        const double padBottom = 35.0;

        const double plotW = width - padLeft - padRight;
        const double plotH = height - padTop - padBottom;

        if (plotW <= 10.0 || plotH <= 10.0) return;

        PangoLayout *layout = pango_cairo_create_layout(cr);
        PangoFontDescription *desc = pango_font_description_from_string("Inter, Segoe UI, sans-serif 9");
        pango_layout_set_font_description(layout, desc);
        pango_font_description_free(desc);

        auto drawPangoText = [&](const std::string &text, double x, double y, double r, double g, double b, float alignX) {
          pango_layout_set_text(layout, text.c_str(), -1);
          int tw = 0, th = 0;
          pango_layout_get_pixel_size(layout, &tw, &th);
          cairo_set_source_rgb(cr, r, g, b);
          cairo_move_to(cr, x - tw * alignX, y - th * 0.5);
          pango_cairo_show_layout(cr, layout);
        };

        double maxVal = 0.0;
        if (pts) {
          for (const auto &p : *pts) {
            if (p.revenue > maxVal) maxVal = p.revenue;
            if (p.profit > maxVal) maxVal = p.profit;
          }
        }

        if (!pts || pts->empty() || maxVal <= 0.0) {
          // Lưới tọa độ rỗng
          for (int i = 0; i <= 4; ++i) {
            double level = i / 4.0;
            double y = padTop + plotH * (1.0 - level);
            cairo_set_source_rgb(cr, 0.886, 0.910, 0.941); // slate-200
            cairo_set_line_width(cr, 1.0);
            cairo_move_to(cr, padLeft, y);
            cairo_line_to(cr, width - padRight, y);
            cairo_stroke(cr);
          }
          drawPangoText("Chưa có phát sinh doanh thu trong kỳ này", padLeft + plotW * 0.5, padTop + plotH * 0.5, 0.4, 0.45, 0.5, 0.5f);
          g_object_unref(layout);
          return;
        }

        double roundedMax = maxVal * 1.20;

        // 1. Gridlines 25%, 50%, 75%, 100%
        for (int i = 1; i <= 4; ++i) {
          double level = i / 4.0;
          double y = padTop + plotH * (1.0 - level);

          cairo_set_source_rgb(cr, 0.886, 0.910, 0.941); // #E2E8F0
          cairo_set_line_width(cr, 1.0);
          cairo_move_to(cr, padLeft, y);
          cairo_line_to(cr, width - padRight, y);
          cairo_stroke(cr);

          std::string yLabel = formatCompactVND(roundedMax * level);
          drawPangoText(yLabel, padLeft - 10.0, y, 0.392, 0.455, 0.545, 1.0f); // #64748B
        }

        // 2. Baseline
        double baselineY = padTop + plotH;
        cairo_set_source_rgb(cr, 0.796, 0.835, 0.882); // slate-300
        cairo_set_line_width(cr, 1.5);
        cairo_move_to(cr, padLeft, baselineY);
        cairo_line_to(cr, width - padRight, baselineY);
        cairo_stroke(cr);
        drawPangoText("0 đ", padLeft - 10.0, baselineY, 0.392, 0.455, 0.545, 1.0f);

        // 3. Helper bo góc đỉnh cột
        auto drawTopRoundedRect = [](cairo_t *cr, double x, double y, double w, double h, double r) {
          if (w <= 0.0 || h <= 0.0) return;
          if (r > w * 0.5) r = w * 0.5;
          if (r > h) r = h;
          cairo_new_sub_path(cr);
          cairo_arc(cr, x + w - r, y + r, r, -G_PI * 0.5, 0);
          cairo_line_to(cr, x + w, y + h);
          cairo_line_to(cr, x, y + h);
          cairo_arc(cr, x + r, y + r, r, G_PI, 1.5 * G_PI);
          cairo_close_path(cr);
          cairo_fill(cr);
        };

        // 4. Vẽ cột đôi
        size_t n = pts->size();
        double slotW = plotW / static_cast<double>(n);

        for (size_t i = 0; i < n; ++i) {
          const auto &pt = (*pts)[i];
          double centerX = padLeft + (i + 0.5) * slotW;

          double maxGroupW = std::min(56.0, slotW * 0.70);
          double colSpacing = 3.0;
          double barW = (maxGroupW - colSpacing) * 0.5;
          if (barW < 4.0) barW = 4.0;

          double groupX = centerX - (barW * 2.0 + colSpacing) * 0.5;

          // Cột Doanh thu (#2563EB)
          double revH = (pt.revenue / roundedMax) * plotH;
          if (revH > plotH) revH = plotH;
          if (revH < 2.0 && pt.revenue > 0) revH = 2.0;
          double revX = groupX;
          double revY = baselineY - revH;

          cairo_set_source_rgb(cr, 0.145, 0.388, 0.922); // #2563EB
          drawTopRoundedRect(cr, revX, revY, barW, revH, 3.0);

          // Cột Lợi nhuận (#15803D)
          double profH = (std::max(0.0, pt.profit) / roundedMax) * plotH;
          if (profH > plotH) profH = plotH;
          if (profH < 2.0 && pt.profit > 0) profH = 2.0;
          double profX = revX + barW + colSpacing;
          double profY = baselineY - profH;

          cairo_set_source_rgb(cr, 0.082, 0.502, 0.239); // #15803D
          drawTopRoundedRect(cr, profX, profY, barW, profH, 3.0);

          // Nhãn tiền rút gọn trên đầu cột Doanh thu
          if (pt.revenue > 0 && slotW >= 42.0) {
            std::string revText = formatCompactVND(pt.revenue);
            drawPangoText(revText, revX + barW * 0.5, revY - 7.0, 0.145, 0.388, 0.922, 0.5f);
          }

          // Nhãn ngày (X-axis)
          drawPangoText(pt.label, centerX, baselineY + 16.0, 0.278, 0.333, 0.412, 0.5f); // #475569
        }

        g_object_unref(layout);
      },
      pointsCopy,
      [](gpointer d) { delete static_cast<std::vector<TimelineStatPoint> *>(d); });

  gtk_box_append(GTK_BOX(card), drawingArea);
  return card;
}

static void resolveBrandColor(const std::string &brand, double &r, double &g, double &b, std::string &hex) {
  std::string lower;
  for (char c : brand) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  if (lower.find("apple") != std::string::npos || lower.find("iphone") != std::string::npos) {
    r = 0.145; g = 0.388; b = 0.922; hex = "#2563EB"; // Blue
  } else if (lower.find("samsung") != std::string::npos) {
    r = 0.051; g = 0.580; b = 0.533; hex = "#0D9488"; // Teal
  } else if (lower.find("oppo") != std::string::npos) {
    r = 0.851; g = 0.467; b = 0.024; hex = "#D97706"; // Amber
  } else if (lower.find("xiaomi") != std::string::npos) {
    r = 0.545; g = 0.361; b = 0.965; hex = "#8B5CF6"; // Purple
  } else {
    r = 0.580; g = 0.643; b = 0.722; hex = "#94A3B8"; // Slate
  }
}

GtkWidget* createBrandDistributionCard(const std::vector<BrandShareStatItem>& brands) {
  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(card, "card");
  gtk_widget_add_css_class(card, "brand-dist-card");

  GtkWidget *lblTitle = gtk_label_new("🏢 Cơ cấu Doanh số theo Thương hiệu");
  gtk_widget_add_css_class(lblTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(card), lblTitle);

  GtkWidget *lblSub = gtk_label_new("Tỷ trọng % doanh thu giữa các thương hiệu trong kỳ lọc.");
  gtk_widget_add_css_class(lblSub, "chart-subtitle");
  gtk_label_set_xalign(GTK_LABEL(lblSub), 0.0f);
  gtk_box_append(GTK_BOX(card), lblSub);

  if (brands.empty()) {
    GtkWidget *emptyBox = createEmptyState("📊", "Chưa có dữ liệu hãng", "Không có đơn hàng bán ra theo thương hiệu trong kỳ này.");
    gtk_box_append(GTK_BOX(card), emptyBox);
    return card;
  }

  // 1. Segmented Bar
  GtkWidget *segArea = gtk_drawing_area_new();
  gtk_widget_set_size_request(segArea, -1, 14);
  gtk_widget_set_hexpand(segArea, TRUE);

  auto *brandCopy = new std::vector<BrandShareStatItem>(brands);
  gtk_drawing_area_set_draw_func(
      GTK_DRAWING_AREA(segArea),
      +[](GtkDrawingArea *, cairo_t *cr, int width, int height, gpointer data) {
        auto *bList = static_cast<const std::vector<BrandShareStatItem> *>(data);
        if (!bList || bList->empty() || width <= 4) return;

        double r = 7.0;
        if (r > height * 0.5) r = height * 0.5;
        cairo_new_sub_path(cr);
        cairo_arc(cr, width - r, r, r, -G_PI * 0.5, 0);
        cairo_arc(cr, width - r, height - r, r, 0, G_PI * 0.5);
        cairo_arc(cr, r, height - r, r, G_PI * 0.5, G_PI);
        cairo_arc(cr, r, r, r, G_PI, 1.5 * G_PI);
        cairo_close_path(cr);
        cairo_clip(cr);

        cairo_set_source_rgb(cr, 0.886, 0.910, 0.941);
        cairo_paint(cr);

        double currentX = 0.0;
        for (const auto &item : *bList) {
          if (item.percentage <= 0.0) continue;
          double segW = width * (item.percentage / 100.0);
          double red = 0.5, green = 0.5, blue = 0.5;
          std::string dummyHex;
          resolveBrandColor(item.brand, red, green, blue, dummyHex);

          cairo_set_source_rgb(cr, red, green, blue);
          cairo_rectangle(cr, currentX, 0, segW, height);
          cairo_fill(cr);
          currentX += segW;
        }
      },
      brandCopy,
      [](gpointer d) { delete static_cast<std::vector<BrandShareStatItem> *>(d); });
  gtk_box_append(GTK_BOX(card), segArea);

  // 2. Brand Rows
  GtkWidget *listContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XS);
  for (const auto &item : brands) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
    gtk_widget_add_css_class(row, "brand-row");

    double red = 0.5, green = 0.5, blue = 0.5;
    std::string hex;
    resolveBrandColor(item.brand, red, green, blue, hex);

    // Chấm màu hình tròn vẽ bằng Cairo
    GtkWidget *dot = gtk_drawing_area_new();
    gtk_widget_set_size_request(dot, 10, 10);
    gtk_widget_set_valign(dot, GTK_ALIGN_CENTER);
    struct DotColor { double r, g, b; };
    auto *dcol = new DotColor{red, green, blue};
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(dot),
        +[](GtkDrawingArea *, cairo_t *cr, int w, int h, gpointer d) {
          auto *c = static_cast<const DotColor *>(d);
          cairo_arc(cr, w * 0.5, h * 0.5, std::min(w, h) * 0.45, 0, 2 * G_PI);
          cairo_set_source_rgb(cr, c->r, c->g, c->b);
          cairo_fill(cr);
        },
        dcol,
        [](gpointer d) { delete static_cast<DotColor *>(d); });
    gtk_box_append(GTK_BOX(row), dot);

    // Tên hãng
    GtkWidget *lblName = gtk_label_new(item.brand.c_str());
    gtk_widget_add_css_class(lblName, "cell-title");
    gtk_widget_set_halign(lblName, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row), lblName);

    // Số máy bán
    std::string qtyStr = "(" + formatNumber(item.soldQuantity) + " máy)";
    GtkWidget *lblQty = gtk_label_new(qtyStr.c_str());
    gtk_widget_add_css_class(lblQty, "cell-subtitle");
    gtk_box_append(GTK_BOX(row), lblQty);

    GtkWidget *rowSpacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(rowSpacer, TRUE);
    gtk_box_append(GTK_BOX(row), rowSpacer);

    // Doanh thu VND
    GtkWidget *lblRev = gtk_label_new(formatVND(item.revenue).c_str());
    gtk_widget_add_css_class(lblRev, "product-price");
    gtk_box_append(GTK_BOX(row), lblRev);

    // Tỷ lệ %
    char pctBuf[32];
    snprintf(pctBuf, sizeof(pctBuf), "%.1f%%", item.percentage);
    GtkWidget *lblPct = gtk_label_new(pctBuf);
    gtk_widget_add_css_class(lblPct, "brand-pct-badge");
    gtk_widget_set_size_request(lblPct, 52, -1);
    gtk_box_append(GTK_BOX(row), lblPct);

    gtk_box_append(GTK_BOX(listContainer), row);
  }
  gtk_box_append(GTK_BOX(card), listContainer);

  return card;
}

GtkWidget* createOrderStatusPipelineBar(int completed, int shipping, int pending, int cancelled) {
  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_SM);
  gtk_widget_add_css_class(card, "card");
  gtk_widget_add_css_class(card, "order-pipeline-card");

  int total = completed + shipping + pending + cancelled;

  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  GtkWidget *lblTitle = gtk_label_new("🔄 Luồng Trạng thái Đơn hàng (Order Pipeline)");
  gtk_widget_add_css_class(lblTitle, "order-pipeline-title");
  gtk_widget_set_halign(lblTitle, GTK_ALIGN_START);
  gtk_widget_set_hexpand(lblTitle, TRUE);
  gtk_box_append(GTK_BOX(headerBox), lblTitle);

  std::string totalText = "Tổng: " + formatNumber(total) + " đơn";
  GtkWidget *lblTotal = gtk_label_new(totalText.c_str());
  gtk_widget_add_css_class(lblTotal, "cell-subtitle");
  gtk_box_append(GTK_BOX(headerBox), lblTotal);
  gtk_box_append(GTK_BOX(card), headerBox);

  // Segmented Bar
  GtkWidget *barArea = gtk_drawing_area_new();
  gtk_widget_set_size_request(barArea, -1, 12);
  gtk_widget_set_hexpand(barArea, TRUE);

  struct PipelineData {
    int comp;
    int ship;
    int pend;
    int canc;
    int tot;
  };
  auto *pdata = new PipelineData{completed, shipping, pending, cancelled, total};

  gtk_drawing_area_set_draw_func(
      GTK_DRAWING_AREA(barArea),
      +[](GtkDrawingArea *, cairo_t *cr, int width, int height, gpointer data) {
        auto *p = static_cast<const PipelineData *>(data);
        if (!p || width <= 4) return;

        double r = 6.0;
        if (r > height * 0.5) r = height * 0.5;
        cairo_new_sub_path(cr);
        cairo_arc(cr, width - r, r, r, -G_PI * 0.5, 0);
        cairo_arc(cr, width - r, height - r, r, 0, G_PI * 0.5);
        cairo_arc(cr, r, height - r, r, G_PI * 0.5, G_PI);
        cairo_arc(cr, r, r, r, G_PI, 1.5 * G_PI);
        cairo_close_path(cr);
        cairo_clip(cr);

        if (p->tot <= 0) {
          cairo_set_source_rgb(cr, 0.886, 0.910, 0.941);
          cairo_paint(cr);
          return;
        }

        double currentX = 0.0;
        auto drawSegment = [&](int count, double red, double green, double blue) {
          if (count <= 0) return;
          double segW = (static_cast<double>(count) / p->tot) * width;
          cairo_set_source_rgb(cr, red, green, blue);
          cairo_rectangle(cr, currentX, 0, segW, height);
          cairo_fill(cr);
          currentX += segW;
        };

        // Hoàn tất (#15803D)
        drawSegment(p->comp, 0.082, 0.502, 0.239);
        // Đang giao (#2563EB)
        drawSegment(p->ship, 0.145, 0.388, 0.922);
        // Chờ xử lý (#D97706)
        drawSegment(p->pend, 0.851, 0.467, 0.024);
        // Đã hủy (#DC2626)
        drawSegment(p->canc, 0.863, 0.149, 0.149);
      },
      pdata,
      [](gpointer d) { delete static_cast<PipelineData *>(d); });
  gtk_box_append(GTK_BOX(card), barArea);

  // Legends
  GtkWidget *legendBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  gtk_widget_set_margin_top(legendBox, 2);

  auto addLegendPill = [&](const std::string &label, int count, const std::string &pillClass) {
    int pct = total > 0 ? static_cast<int>(std::round(count * 100.0 / total)) : 0;
    std::string txt = label + ": " + std::to_string(count) + " (" + std::to_string(pct) + "%)";
    GtkWidget *pill = gtk_label_new(txt.c_str());
    gtk_widget_add_css_class(pill, "pipeline-legend-pill");
    gtk_widget_add_css_class(pill, pillClass.c_str());
    gtk_box_append(GTK_BOX(legendBox), pill);
  };

  addLegendPill("Hoàn tất", completed, "pipeline-legend-pill--completed");
  addLegendPill("Đang giao", shipping, "pipeline-legend-pill--shipping");
  addLegendPill("Chờ xử lý", pending, "pipeline-legend-pill--pending");
  addLegendPill("Đã hủy", cancelled, "pipeline-legend-pill--cancelled");

  gtk_box_append(GTK_BOX(card), legendBox);
  return card;
}

} // namespace UIComponents
