#ifndef APP_RUNNER_H
#define APP_RUNNER_H

#include <gtk/gtk.h>
#include "../source/controllers/AppController.h"
#include "screens/LoginScreen.h"
#include "screens/MainShellWindow.h"

/**
 * @file AppRunner.h
 * @brief ĐẶC TẢ BỘ ĐIỀU PHỐI VÒNG ĐỜI VÀ CỬA SỔ ỨNG DỤNG (APPLICATION RUNNER & LIFECYCLE COORDINATOR)
 * 
 * =========================================================================================
 * 1. KIẾN TRÚC & NHIỆM VỤ:
 *    - Quản lý phiên chạy GtkApplication ("vn.edu.dut.pbl2.phonestore.ui").
 *    - Đóng vai trò Root Coordinator: Điều phối chuyển đổi trạng thái giữa màn hình Đăng nhập
 *      (Login Window) và Cửa sổ giao diện chính (MainShellWindow).
 *    - Áp dụng triệt để cơ chế chuyển đổi một cửa sổ (Single Active Window State):
 *      Khi đăng nhập thành công, cửa sổ đăng nhập bị tiêu hủy hoàn toàn (gtk_window_destroy);
 *      Khi đăng xuất, cửa sổ làm việc chính bị giải phóng và tái tạo cửa sổ đăng nhập mới.
 *      Điều này khắc phục triệt để lỗi tồn đọng cửa sổ cũ (zombie window/orphan popup).
 * 
 * 2. KẾT NỐI KIẾN TRÚC:
 *    - Tầng Điều khiển (Controller): Tương tác trực tiếp với AppController.
 *    - Giao diện phụ thuộc: LoginScreen, MainShellWindow, AppCss (nạp style toàn cục).
 * =========================================================================================
 */
class AppRunner {
private:
    GtkApplication* app;       ///< Đối tượng GtkApplication quản lý event loop chính
    AppController* controller; ///< Con trỏ tham chiếu tầng Controller nghiệp vụ

    GtkWidget* loginWindow;    ///< Cửa sổ top-level chứa màn hình đăng nhập
    LoginScreen* loginScreen;  ///< Thành phần giao diện đăng nhập (Login Screen component)
    MainShellWindow* shellWindow; ///< Cửa sổ vỏ bọc chính sau khi đăng nhập thành công

public:
    /**
     * @brief Khởi tạo AppRunner và đăng ký signal activate cho GtkApplication.
     * @param ctrl Con trỏ AppController đã được nạp dữ liệu và cấu hình Core Services.
     */
    AppRunner(AppController* ctrl);

    /**
     * @brief Hủy AppRunner, giải phóng toàn bộ cửa sổ con và unref GtkApplication.
     */
    ~AppRunner();

    /**
     * @brief Khởi động vòng lặp sự kiện giao diện GTK4 (g_application_run).
     * @param argc Số lượng tham số dòng lệnh.
     * @param argv Mảng chuỗi tham số dòng lệnh.
     * @return Mã thoát chương trình (exit status code).
     */
    int run(int argc, char* argv[]);

    /**
     * @brief Chuyển sang hiển thị màn hình Đăng nhập (hủy MainShellWindow nếu đang tồn tại).
     */
    void showLogin();

    /**
     * @brief Chuyển sang hiển thị giao diện làm việc chính theo vai trò người dùng (hủy loginWindow).
     */
    void showMainShell();

    // === TÍN HIỆU & HÀM GỌI LẠI (CALLBACK HANDLERS) ===
    static void on_app_activate(GtkApplication* gapp, gpointer data);
    static gboolean on_login_window_close_request(GtkWindow* window, gpointer data);
    static void on_login_success(AppController* ctrl, gpointer data);
    static void on_logout_success(AppController* ctrl, gpointer data);
};

#endif // APP_RUNNER_H
