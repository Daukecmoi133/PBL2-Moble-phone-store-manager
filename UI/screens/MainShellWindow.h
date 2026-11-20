#ifndef MAIN_SHELL_WINDOW_H
#define MAIN_SHELL_WINDOW_H

#include "../../source/controllers/AppController.h"
#include <gtk/gtk.h>
#include <map>
#include <string>
#include <vector>


class CustomerScreens;
class SalesScreens;
class WarehouseScreens;
class AdminScreens;

/**
 * @file MainShellWindow.h
 * @brief ĐẶC TẢ CỬA SỔ KHUNG CHÍNH (MAIN SHELL & DYNAMIC ROLE-BASED NAVIGATION
 * HOST)
 *
 * =========================================================================================
 * 1. KIẾN TRÚC & PHÂN BỐ GIAO DIỆN:
 *    - MainShellWindow là Container mẹ chứa toàn bộ không gian làm việc sau khi
 * đăng nhập.
 *    - Gồm 3 vùng cấu trúc chính:
 *      + HeaderBar phía trên: Brand title, Huy hiệu thời gian thực (Real-time
 * Simulation Clock), Huy hiệu người dùng hiện tại (Tên + Role badge), và Nút
 * Đăng xuất an toàn.
 *      + Sidebar bên trái: Menu điều hướng theo vai trò (Role-based
 * Navigation).
 *        * CUSTOMER: Tổng quan, Sản phẩm, Giỏ hàng (+ huy hiệu số lượng), Đơn
 * hàng, Hồ sơ, Bảo hành.
 *        * SALES: Bàn làm việc, Bán tại quầy (POS), Tra cứu sản phẩm, Khách
 * hàng, Hóa đơn, Đơn online, Tiếp nhận BH.
 *        * WAREHOUSE: Bàn làm việc, Danh mục tồn kho, Cảnh báo tồn thấp, Nhập
 * hàng, Đóng gói xuất kho.
 *        * ADMIN: 12 màn hình quản trị toàn diện, Thống kê báo cáo, Giả lập
 * thời gian thực.
 *      + Stack Container ở giữa: GtkStack hiển thị nội dung các màn hình chức
 * năng.
 *
 * 2. CƠ CHẾ ĐIỀU HƯỚNG & ĐỒNG BỘ TRẠNG THÁI:
 *    - navigateTo(): Chuyển đổi tab trên GtkStack và đồng bộ class CSS
 * 'sidebar-btn-active'.
 *    - updateCartBadge(): Cập nhật số lượng item trong giỏ hàng tức thời.
 *    - updateSimulatedTimeBadge(): Hiển thị thời gian thực giả lập hệ thống
 * (phục vụ đối soát voucher/bảo hành).
 * =========================================================================================
 */
class MainShellWindow {
private:
  AppController *controller;
  GtkApplication *app;
  GtkWidget *window;
  GtkWidget *headerBar;
  GtkWidget *lblHeaderTitle;
  GtkWidget *lblSimulatedTimeBadge;
  GtkWidget *lblUserBadge;
  GtkWidget *sidebarBox;
  GtkWidget *stack;
  GtkWidget *lblCartCountBadge;

  std::map<std::string, GtkWidget *> sidebarButtons;
  std::string currentPage;

  typedef void (*LogoutCallback)(AppController *ctrl, gpointer data);
  LogoutCallback logoutCallback;
  gpointer callbackData;

  // Sub-screen managers
  CustomerScreens *customerScreens;
  SalesScreens *salesScreens;
  WarehouseScreens *warehouseScreens;
  AdminScreens *adminScreens;

  GtkWidget *toastOverlay;

  // In-Window Page Stack Navigation
  struct PageEntry {
    std::string id;
    std::string title;
    std::string childName;
    GtkWidget *pageBox;
    GtkWidget *contentWidget;
  };
  GtkWidget *pageNavStack = nullptr;
  std::vector<PageEntry> pageStack;
  struct PageCleanupRecord {
    guint timerId;
    GtkWidget *stack;
    GtkWidget *page;
  };
  std::vector<PageCleanupRecord> pendingCleanups;

  // In-Window Overlay Dialog
  GtkWidget *currentOverlayWrapper = nullptr;
  GtkWidget *currentOverlayCard = nullptr;

  // Real-time Simulation Clock Timer (single global source)
  guint clockTimerId = 0;
  bool ownsWindow = false;
  GtkEventController *keyController = nullptr;

public:
  MainShellWindow(GtkWindow *parentWindow, AppController *ctrl,
                  LogoutCallback onLogout, gpointer userData);
  MainShellWindow(GtkApplication *gapp, AppController *ctrl,
                  LogoutCallback onLogout, gpointer userData);
  ~MainShellWindow();

  GtkWindow *getWindow() const { return window ? GTK_WINDOW(window) : nullptr; }
  GtkWidget *getRootWidget() const { return toastOverlay; }
  GtkWidget *getToastOverlay() const { return toastOverlay; }
  void show();
  void activateSession();
  void deactivateSession();
  void showToast(const std::string &message,
                 const std::string &type = "success");
  void navigateTo(const std::string &pageName);
  void navigateToCartWithOnlySelected(const std::string &phoneId);
  void updateCartBadge(int count);
  void updateSimulatedTimeBadge();
  void refreshCurrentRoleUI();

  // In-Window Page Stack Navigation API
  void pushPage(const std::string &id, GtkWidget *content,
                const std::string &title, bool showTopBack = true);
  void popPage();
  void clearPageStack();
  bool hasPages() const { return !pageStack.empty(); }
  size_t getPageStackDepth() const { return pageStack.size(); }

  // In-Window Overlay Dialog API
  void showOverlayDialog(GtkWidget *card);
  void closeOverlayDialog();
  bool isOverlayDialogOpen() const { return currentOverlayWrapper != nullptr; }

  static MainShellWindow *getActiveInstance();

private:
  void initShellContent();
  void flushPendingCleanups();
  void startClockTimer();
  void stopClockTimer();
  void handleClockTick();
  static gboolean on_clock_tick(gpointer data);

  void buildHeader();
  void buildSidebar();
  void buildStack();
  void addSidebarButton(const std::string &pageName, const std::string &label,
                        const std::string &icon);

  static void on_nav_btn_clicked(GtkButton *btn, gpointer data);
  static void on_logout_btn_clicked(GtkButton *btn, gpointer data);
  static void on_confirm_logout(gpointer data);
  static gboolean on_window_close_request(GtkWindow *window, gpointer data);
  static gboolean on_key_pressed(GtkEventControllerKey *controller,
                                 guint keyval, guint keycode,
                                 GdkModifierType state, gpointer data);
};

#endif // MAIN_SHELL_WINDOW_H
