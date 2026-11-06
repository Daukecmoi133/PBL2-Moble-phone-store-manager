#ifndef LOGIN_SCREEN_H
#define LOGIN_SCREEN_H

#include <gtk/gtk.h>
#include <string>
#include "../../source/controllers/AppController.h"

/**
 * @file LoginScreen.h
 * @brief ĐẶC TẢ MÀN HÌNH XÁC THỰC & ĐĂNG NHẬP HỆ THỐNG (AUTHENTICATION & ACCESS CONTROL)
 * 
 * =========================================================================================
 * 1. MỤC TIÊU & CHỨC NĂNG NGHIỆP VỤ:
 *    - Xác thực tài khoản người dùng thông qua AppController::login(username, password).
 *    - Hỗ trợ phân quyền tức thì theo vai trò (Role): Admin, Sales, Warehouse, Customer.
 *    - Bổ sung chức năng theo yêu cầu feedback:
 *      + Nút Đăng ký tài khoản (Register) & Quên mật khẩu (Forgot Password) đặt gọn gàng ở góc thẻ đăng nhập.
 *      + Thẻ chọn nhanh tài khoản mẫu (Quick Demo Pills: Admin, Saler, Kho, Khách hàng) phục vụ chấm đồ án.
 *      + Cơ chế hiển thị lỗi validation trực quan ngay trên giao diện mà không gây crash khi nhập tiếng Việt.
 * =========================================================================================
 */
class LoginScreen {
private:
    AppController* controller;
    GtkWidget* rootWidget;
    GtkWidget* centerBox;
    GtkWidget* card;
    
    // Login form widgets
    GtkWidget* entryUsername;
    GtkWidget* entryPassword;
    GtkWidget* lblError;
    GtkWidget* btnLogin;

    typedef void (*LoginSuccessCallback)(AppController* ctrl, gpointer data);
    LoginSuccessCallback successCallback;
    gpointer callbackData;

public:
    LoginScreen(AppController* ctrl, LoginSuccessCallback onLoginSuccess, gpointer userData);

    GtkWidget* getRootWidget() const { return rootWidget; }

    void triggerLogin();
    void setQuickCredentials(const std::string& username, const std::string& password);

    void showLoginForm();
    void showRegisterForm();
    void showForgotPasswordDialog();

    // Callbacks
    static void on_login_btn_clicked(GtkButton* btn, gpointer data);
    static void on_quick_fill_clicked(GtkButton* btn, gpointer data);
};

#endif // LOGIN_SCREEN_H
