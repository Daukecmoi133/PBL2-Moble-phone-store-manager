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
#include "../screens/MainShellWindow.h"
#include "../styles/AppCss.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <gdk/gdkkeysyms.h>
#include <iomanip>
#include <sstream>

namespace UIComponents {

static MainShellWindow *g_activeShell = nullptr;

void registerShellWindow(MainShellWindow *shell) { g_activeShell = shell; }

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

// === HELPER MODAL DIALOG DÙNG CHUNG ===

struct ConfirmContext {
  GtkWidget *dialog;
  ConfirmCallback callback;
  gpointer userData;
};

static void on_confirm_click(GtkButton *btn, gpointer data) {
  (void)btn;
  ConfirmContext *ctx = static_cast<ConfirmContext *>(data);
  if (ctx->callback) {
    ctx->callback(ctx->userData);
  }
  gtk_window_destroy(GTK_WINDOW(ctx->dialog));
  delete ctx;
}

static void on_cancel_click(GtkButton *btn, gpointer data) {
  (void)btn;
  ConfirmContext *ctx = static_cast<ConfirmContext *>(data);
  gtk_window_destroy(GTK_WINDOW(ctx->dialog));
  delete ctx;
}

static void on_dialog_close_click(GtkButton *btn, gpointer win) {
  (void)btn;
  gtk_window_destroy(GTK_WINDOW(win));
}

static void showGeneralDialog(GtkWindow *parent, const std::string &icon,
                              const std::string &title,
                              const std::string &message,
                              const std::string &btnText = "Đóng",
                              const std::string &btnClass = "btn-primary") {
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win), title.c_str());
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  if (parent) {
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
  }
  gtk_window_set_default_size(GTK_WINDOW(win), 480, 250);
  gtk_window_set_resizable(GTK_WINDOW(win), FALSE);

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_margin_top(card, 16);
  gtk_widget_set_margin_bottom(card, 16);
  gtk_widget_set_margin_start(card, 16);
  gtk_widget_set_margin_end(card, 16);

  // Icon + Title Box
  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  GtkWidget *lblIcon = gtk_label_new(icon.c_str());
  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "dialog-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(headerBox), lblIcon);
  gtk_box_append(GTK_BOX(headerBox), lblTitle);
  gtk_box_append(GTK_BOX(card), headerBox);

  // Message
  GtkWidget *lblMsg = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(lblMsg, "dialog-message");
  gtk_label_set_wrap(GTK_LABEL(lblMsg), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(lblMsg), 45);
  gtk_label_set_xalign(GTK_LABEL(lblMsg), 0.0f);
  gtk_box_append(GTK_BOX(card), lblMsg);

  // Button Box
  GtkWidget *btnBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_halign(btnBox, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnBox, 10);

  GtkWidget *btn = gtk_button_new_with_label(btnText.c_str());
  gtk_widget_add_css_class(btn, btnClass.c_str());
  setButtonCursor(btn);
  g_signal_connect(btn, "clicked", G_CALLBACK(on_dialog_close_click), win);
  gtk_box_append(GTK_BOX(btnBox), btn);

  gtk_box_append(GTK_BOX(card), btnBox);

  gtk_window_set_child(GTK_WINDOW(win), card);
  gtk_window_present(GTK_WINDOW(win));
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
  if (g_activeShell) {
    g_activeShell->showToast(message, type);
    return;
  }

  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win), "Thông báo");
  gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
  if (parent) {
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
  }

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(box, "toast-box");
  if (type == "success") {
    gtk_widget_add_css_class(box, "toast-success");
  } else if (type == "error") {
    gtk_widget_add_css_class(box, "toast-error");
  } else {
    gtk_widget_add_css_class(box, "toast-info");
  }

  GtkWidget *iconLbl =
      gtk_label_new(type == "success" ? "✅" : (type == "error" ? "❌" : "ℹ️"));
  gtk_box_append(GTK_BOX(box), iconLbl);

  GtkWidget *msgLbl = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(msgLbl, "toast-text");
  gtk_label_set_wrap(GTK_LABEL(msgLbl), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(msgLbl), 40);
  gtk_box_append(GTK_BOX(box), msgLbl);

  GtkWidget *btnClose = gtk_button_new_with_label("✕");
  gtk_widget_add_css_class(btnClose, "toast-close-btn");
  setButtonCursor(btnClose);
  g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy),
                           win);
  gtk_box_append(GTK_BOX(box), btnClose);

  gtk_window_set_child(GTK_WINDOW(win), box);
  gtk_window_present(GTK_WINDOW(win));

  g_timeout_add(
      2500,
      +[](gpointer d) -> gboolean {
        GtkWidget *w = GTK_WIDGET(d);
        if (GTK_IS_WINDOW(w)) {
          gtk_window_destroy(GTK_WINDOW(w));
        }
        return G_SOURCE_REMOVE;
      },
      win);
}

struct ChangePasswordContext {
  AppController *controller;
  GtkWidget *window;
  GtkWidget *eOld;
  GtkWidget *eNew;
  GtkWidget *eConfirm;
  GtkWidget *lblMsg;
};

void showChangePasswordDialog(GtkWindow *parent, AppController *controller) {
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win), "Đổi mật khẩu");
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  if (parent) {
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
  }
  gtk_window_set_default_size(GTK_WINDOW(win), 500, 450);
  gtk_window_set_resizable(GTK_WINDOW(win), FALSE);

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(card, "dialog-content");

  GtkWidget *lblTitle = gtk_label_new("🔑 Đổi mật khẩu");
  gtk_widget_add_css_class(lblTitle, "dialog-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(card), lblTitle);

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

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 8);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  setButtonCursor(btnCancel);
  g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy),
                           win);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  GtkWidget *btnSave = gtk_button_new_with_label("Đổi mật khẩu");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  setButtonCursor(btnSave);
  gtk_box_append(GTK_BOX(btnRow), btnSave);
  gtk_box_append(GTK_BOX(card), btnRow);

  ChangePasswordContext *ctx =
      new ChangePasswordContext{controller, win, eOld, eNew, eConfirm, lblMsg};
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
        GtkWindow *transientParent =
            gtk_window_get_transient_for(GTK_WINDOW(c->window));
        gtk_window_destroy(GTK_WINDOW(c->window));
        showSuccessDialog(transientParent, "Đổi mật khẩu", res.message);
      }),
      ctx,
      [](gpointer d, GClosure *) {
        delete static_cast<ChangePasswordContext *>(d);
      },
      (GConnectFlags)0);

  gtk_window_set_child(GTK_WINDOW(win), card);
  gtk_window_present(GTK_WINDOW(win));
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
  GtkWidget *win = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(win), title.c_str());
  gtk_window_set_modal(GTK_WINDOW(win), TRUE);
  if (parent) {
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
  }
  gtk_window_set_default_size(GTK_WINDOW(win), 500, 250);
  gtk_window_set_resizable(GTK_WINDOW(win), FALSE);

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "dialog-content");
  gtk_widget_set_margin_top(card, 16);
  gtk_widget_set_margin_bottom(card, 16);
  gtk_widget_set_margin_start(card, 16);
  gtk_widget_set_margin_end(card, 16);

  GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  GtkWidget *lblIcon = gtk_label_new("❓");
  GtkWidget *lblTitle = gtk_label_new(title.c_str());
  gtk_widget_add_css_class(lblTitle, "dialog-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_box_append(GTK_BOX(headerBox), lblIcon);
  gtk_box_append(GTK_BOX(headerBox), lblTitle);
  gtk_box_append(GTK_BOX(card), headerBox);

  GtkWidget *lblMsg = gtk_label_new(message.c_str());
  gtk_widget_add_css_class(lblMsg, "dialog-message");
  gtk_label_set_wrap(GTK_LABEL(lblMsg), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(lblMsg), 45);
  gtk_label_set_xalign(GTK_LABEL(lblMsg), 0.0f);
  gtk_box_append(GTK_BOX(card), lblMsg);

  ConfirmContext *ctx = new ConfirmContext();
  ctx->dialog = win;
  ctx->callback = onConfirm;
  ctx->userData = userData;

  GtkWidget *btnBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign(btnBox, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnBox, 10);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy bỏ");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  setButtonCursor(btnCancel);
  g_signal_connect(btnCancel, "clicked", G_CALLBACK(on_cancel_click), ctx);
  gtk_box_append(GTK_BOX(btnBox), btnCancel);

  GtkWidget *btnConfirm = gtk_button_new_with_label("Xác nhận");
  gtk_widget_add_css_class(btnConfirm, "btn-danger");
  setButtonCursor(btnConfirm);
  g_signal_connect(btnConfirm, "clicked", G_CALLBACK(on_confirm_click), ctx);
  gtk_box_append(GTK_BOX(btnBox), btnConfirm);

  gtk_box_append(GTK_BOX(card), btnBox);

  gtk_window_set_child(GTK_WINDOW(win), card);
  gtk_window_present(GTK_WINDOW(win));
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
    if (col.minWidth > 0) {
      gtk_widget_set_size_request(lbl, col.minWidth, -1);
    }
    if (col.hexpand) {
      gtk_widget_set_hexpand(lbl, TRUE);
    }
    gtk_label_set_xalign(GTK_LABEL(lbl), col.alignment);
    gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(row), lbl);
  }
  return row;
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
  }
  gtk_widget_set_hexpand(cellWidget, spec.hexpand ? TRUE : FALSE);
  gtk_widget_set_valign(cellWidget, GTK_ALIGN_CENTER);
  if (GTK_IS_LABEL(cellWidget)) {
    gtk_label_set_xalign(GTK_LABEL(cellWidget), spec.alignment);
    gtk_label_set_ellipsize(GTK_LABEL(cellWidget), PANGO_ELLIPSIZE_END);
  }
}

GtkWidget *createTableTextCell(const std::string &text,
                               const TableColumnSpec &spec,
                               const std::string &cssClass) {
  GtkWidget *lbl = gtk_label_new(text.c_str());
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

  GtkWidget *lblPrimary = gtk_label_new(primaryText.c_str());
  gtk_widget_add_css_class(lblPrimary, "cell-title");
  gtk_label_set_ellipsize(GTK_LABEL(lblPrimary), PANGO_ELLIPSIZE_END);
  gtk_label_set_xalign(GTK_LABEL(lblPrimary), spec.alignment);
  gtk_box_append(GTK_BOX(box), lblPrimary);

  if (!secondaryText.empty()) {
    GtkWidget *lblSecondary = gtk_label_new(secondaryText.c_str());
    gtk_widget_add_css_class(lblSecondary, "cell-subtitle");
    gtk_label_set_ellipsize(GTK_LABEL(lblSecondary), PANGO_ELLIPSIZE_END);
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
    if (GTK_IS_LABEL(innerWidget)) {
      gtk_label_set_ellipsize(GTK_LABEL(innerWidget), PANGO_ELLIPSIZE_END);
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
          if (GTK_IS_WINDOW(sc->win)) {
            GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(sc->win));
            if (focus && GTK_IS_TEXT_VIEW(focus)) {
              return FALSE;
            }
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

} // namespace UIComponents
