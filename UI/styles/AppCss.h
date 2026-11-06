#ifndef APP_CSS_H
#define APP_CSS_H

#include <gtk/gtk.h>

/**
 * @file AppCss.h
 * @brief ĐẶC TẢ HỆ THỐNG GIAO DIỆN & BẢNG MÃ CSS CHUẨN CHO GTK4 (PBL2 PHONE
 * STORE MANAGER)
 *
 * =========================================================================================
 * 1. MỤC ĐÍCH & PHẠM VI ĐẶC TẢ:
 *    - Cung cấp toàn bộ Design Tokens (màu sắc, kích thước phông chữ, bo góc,
 * bóng đổ) phục vụ cho giao diện ứng dụng quản lý cửa hàng điện thoại PBL2.
 *    - Được áp dụng toàn cục (GTK_STYLE_PROVIDER_PRIORITY_APPLICATION) qua
 * GtkCssProvider.
 *    - Giải quyết yêu cầu phản hồi: nâng phông chữ lên tối thiểu 16px/15px cho
 * người mắt yếu, tăng cường độ tương phản, đặc tả mã màu phân hạng khách hàng
 * và trạng thái đơn hàng.
 *
 * 2. HỆ THIẾT KẾ THỐNG NHẤT (DESIGN TOKENS):
 *    - MÀU SẮC: toàn bộ màu được khai báo MỘT LẦN bằng @define-color ở đầu
 * chuỗi CSS (tiền tố pbl_), các quy tắc phía dưới chỉ tham chiếu @pbl_xxx. Đổi
 * 1 dòng @define-color là đổi cả ứng dụng.
 *    - CỠ CHỮ: tiêu đề trang 24px, tiêu đề thẻ 18px, nội dung 15px, chú thích
 * 13px, tổng tiền POS 32px.
 *    - KHOẢNG CÁCH: bội số của 8px (8/16/24/32); khoảng cách nhỏ dùng nửa bước
 * 4px.
 *    - BO GÓC: 8px cho nút/ô nhập, 12px cho thẻ/hộp thoại.
 *
 * 3. ĐÁNH GIÁ ĐỘ HỖ TRỢ VỚI ENGINE CSS CỦA GTK4:
 *    - ĐƯỢC HỖ TRỢ ĐẦY ĐỦ:
 *      + Selectors: Universal (*), Type Selector (window, entry, button), Class
 * (.card, .badge, ...)
 *      + Pseudo-classes: :hover, :active, :focus, :disabled. (Lưu ý: :invalid
 * không tồn tại trong GTK4).
 *      + Màu sắc: Mã Hex (#0f172a), rgba(r,g,b,a), alpha(color, a),
 * @define-color.
 *      + Nền & Gradient: linear-gradient(góc, màu1 điểm1, màu2 điểm2),
 * background-color.
 *      + Typography: font-family, font-size, font-weight, letter-spacing,
 * text-transform, text-decoration.
 *      + Hộp & Viền: padding, margin, min-height, border, border-radius,
 * box-shadow.
 *      + Chuyển động: transition (transition-property, duration,
 * timing-function).
 *    - KHÔNG ĐƯỢC HỖ TRỢ (ĐẶC THÙ GTK4 CSS KHÁC VỚI WEB CSS):
 *      + Layout Web: display: flex/grid, position: absolute/relative (GTK4 dùng
 * GtkBox, GtkGrid).
 *      + Kích thước tự do: width/height trong CSS không hỗ trợ (GTK4 chỉ nhận
 * min-width, min-height).
 *      + Con trỏ: cursor: pointer không có trong GTK4 CSS (phải đặt qua
 * gtk_widget_set_cursor_from_name).
 *      + Căn lề chữ: text-align không có (dùng gtk_label_set_xalign /
 * gtk_widget_set_halign).
 *      + Giả phần tử: ::before, ::after không tồn tại trong GTK4.
 *
 * 4. QUY TẮC BẢNG DỮ LIỆU:
 *    - Dòng xen kẽ: code gắn lớp .row-even / .row-odd cho từng dòng .table-row.
 *    - Cột số (số lượng, tiền) căn phải bằng gtk_label_set_xalign(label, 1.0).
 * =========================================================================================
 */

// =========================================================================
// HẰNG SỐ DESIGN TOKENS C++ CHO BỐ CỤC GTK4 (4PX BASE / 8PX RHYTHM)
// =========================================================================
namespace DesignTokens {
    // Quy tắc 4px base / 8px rhythm:
    // - Nhịp chính (8px rhythm): 8, 16, 24, 32, 40, 48
    // - Bước phụ (4px half-step): 4, 12, 20
    constexpr int SPACE_XS  = 4;   // 0.5x (4px)  - Khoảng cách nội bộ nhỏ, dòng bảng
    constexpr int SPACE_SM  = 8;   // 1.0x (8px)  - Khoảng cách nút bấm, padding nhỏ
    constexpr int SPACE_MD  = 12;  // 1.5x (12px) - Khoảng cách giữa các trường form / cột
    constexpr int SPACE_LG  = 16;  // 2.0x (16px) - Khoảng cách lưới thẻ KPI, padding chuẩn
    constexpr int SPACE_XL  = 20;  // 2.5x (20px) - Padding thẻ nội dung vừa
    constexpr int SPACE_2XL = 24;  // 3.0x (24px) - Padding khung trang chính (.page-container)
    constexpr int SPACE_3XL = 32;  // 4.0x (32px) - Khoảng cách phân vùng lớn

    // Kích thước khung điều hướng & bảng dữ liệu (bội số của 8px)
    constexpr int SIDEBAR_WIDTH      = 240; // 30 * 8px — thống nhất với MainShellWindow
    constexpr int TABLE_ROW_SPACING  = 4;   // 1 * 4px
    constexpr int TABLE_CELL_SPACING = 12;  // 3 * 4px
    constexpr int KPI_GRID_SPACING   = 16;  // 2 * 8px
} // namespace DesignTokens

// Bộ CSS chuẩn giao diện hiện đại cho GTK4 Phone Store Manager
// Dùng font hệ thống để giao diện phù hợp với từng nền tảng
inline const char *APP_CSS_STYLES = R"CSS(
/* =========================================================================
   DESIGN TOKENS — SỬA MÀU TẠI ĐÂY, TOÀN ỨNG DỤNG TỰ ĐỔI THEO
   ========================================================================= */
/* Màu chính (primary) */
@define-color pbl_primary #1d4ed8;
@define-color pbl_primary_hover #1e40af;
@define-color pbl_primary_active #1e3a8a;
@define-color pbl_accent #38bdf8;
@define-color pbl_focus_ring alpha(#2563eb, 0.45);

/* Nền & viền & chữ */
@define-color pbl_bg #f1f5f9;
@define-color pbl_surface #ffffff;
@define-color pbl_surface_alt #f8fafc;
@define-color pbl_surface_hover #eff6ff;
@define-color pbl_divider #e2e8f0;
@define-color pbl_border #cbd5e1;
@define-color pbl_border_strong #94a3b8;
@define-color pbl_text #0f172a;
@define-color pbl_text_muted #475569;
@define-color pbl_text_subtle #64748b;
@define-color pbl_text_inverse #f8fafc;

/* Nền tối (header, sidebar, màn đăng nhập) */
@define-color pbl_navy #0f172a;
@define-color pbl_navy_2 #1e293b;
@define-color pbl_navy_3 #334155;
@define-color pbl_navy_text #cbd5e1;

/* Màu ngữ nghĩa: thành công / lỗi / cảnh báo */
@define-color pbl_success #047857;
@define-color pbl_success_hover #065f46;
@define-color pbl_success_active #064e3b;
@define-color pbl_success_bg #d1fae5;
@define-color pbl_success_border #a7f3d0;
@define-color pbl_danger #b91c1c;
@define-color pbl_danger_hover #991b1b;
@define-color pbl_danger_active #7f1d1d;
@define-color pbl_danger_bg #fee2e2;
@define-color pbl_danger_border #fecaca;
@define-color pbl_warning #b45309;
@define-color pbl_warning_bg #fef3c7;
@define-color pbl_warning_border #fde68a;

/* Màu nhãn trạng thái phụ */
@define-color pbl_info #0369a1;
@define-color pbl_info_bg #e0f2fe;
@define-color pbl_info_border #bae6fd;
@define-color pbl_purple #6b21a8;
@define-color pbl_purple_bg #f3e8ff;
@define-color pbl_purple_border #e9d5ff;
@define-color pbl_orange #9a3412;
@define-color pbl_orange_bg #ffedd5;
@define-color pbl_orange_border #fed7aa;
@define-color pbl_blue_bg #dbeafe;
@define-color pbl_blue_border #bfdbfe;

/* Màu hạng khách hàng */
@define-color pbl_gold #854d0e;
@define-color pbl_gold_bg #fef9c3;
@define-color pbl_gold_border #ca8a04;
@define-color pbl_diamond #0e7490;
@define-color pbl_diamond_bg #ecfeff;
@define-color pbl_diamond_border #06b6d4;
@define-color pbl_silver #334155;
@define-color pbl_silver_bg #f1f5f9;
@define-color pbl_silver_border #64748b;

/* =========================================================================
   GLOBAL
   ========================================================================= */
* {
    font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, Roboto, sans-serif;
    font-size: 16px;
}

window {
    background-color: @pbl_bg;
    color: @pbl_text;
}

/* === HEADER BAR === */
.main-header {
    background: linear-gradient(135deg, @pbl_navy 0%, @pbl_navy_2 100%);
    padding: 16px 24px;
    border-bottom: 2px solid @pbl_navy_3;
    color: #ffffff;
}

.app-brand-title {
    font-size: 20px;
    font-weight: 800;
    color: @pbl_accent;
    letter-spacing: 0.5px;
}

.header-screen-title {
    font-size: 16px;
    font-weight: 700;
    color: @pbl_navy_text;
    padding: 0 8px;
}

.user-profile-badge {
    background-color: @pbl_navy_3;
    color: @pbl_text_inverse;
    border-radius: 20px;
    padding: 8px 16px;
    font-weight: 600;
    font-size: 14px;
    border: 1px solid @pbl_border_strong;
}

.realtime-header-badge {
    background-color: @pbl_primary_active;
    color: #dbeafe;
    border: 1px solid @pbl_primary;
    border-radius: 20px;
    padding: 4px 16px;
    font-size: 13px;
    font-weight: 700;
}

.role-pill-admin {
    background-color: @pbl_danger;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 8px;
    font-size: 12px;
    font-weight: 700;
}

.role-pill-sales {
    background-color: @pbl_primary;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 8px;
    font-size: 12px;
    font-weight: 700;
}

.role-pill-warehouse {
    background-color: @pbl_warning;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 8px;
    font-size: 12px;
    font-weight: 700;
}

.role-pill-customer {
    background-color: @pbl_success;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 8px;
    font-size: 12px;
    font-weight: 700;
}

/* === SIDEBAR === */
.sidebar-panel {
    background-color: @pbl_navy;
    border-right: 1px solid @pbl_navy_2;
    padding: 16px 8px;
}

.sidebar-category-label {
    color: @pbl_border_strong;
    font-size: 11px;
    font-weight: 800;
    text-transform: uppercase;
    letter-spacing: 1px;
    padding: 12px 8px 4px 8px;
}

.sidebar-divider {
    background-color: @pbl_navy_2;
    min-height: 1px;
    margin: 8px 8px;
}

.sidebar-icon-slot {
    min-width: 24px;
    font-size: 15px;
}

button.sidebar-btn, .sidebar-btn {
    background-color: transparent;
    background-image: none;
    box-shadow: none;
    color: @pbl_navy_text;
    border: none;
    border-radius: 8px;
    padding: 8px 16px;
    font-size: 14px;
    font-weight: 600;
    margin-bottom: 4px;
    transition: all 150ms ease;
}

button.sidebar-btn:hover, .sidebar-btn:hover {
    background-color: @pbl_navy_2;
    background-image: none;
    color: #ffffff;
}

button.sidebar-btn:focus, .sidebar-btn:focus {
    outline: 2px solid @pbl_accent;
    outline-offset: -2px;
}

button.sidebar-btn-active, .sidebar-btn-active {
    background-color: @pbl_primary;
    background-image: none;
    box-shadow: none;
    color: #ffffff;
    font-weight: 700;
}

button.sidebar-btn-active:hover, .sidebar-btn-active:hover {
    background-color: @pbl_primary_hover;
    background-image: none;
    color: #ffffff;
}

button.sidebar-btn-active:focus, .sidebar-btn-active:focus {
    outline: 2px solid @pbl_accent;
    outline-offset: -2px;
}

.cart-count-badge {
    background-color: @pbl_danger;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 8px;
    font-size: 12px;
    font-weight: 800;
}

/* === CONTENT PAGES === */
.page-container {
    padding: 20px 24px;
}

.page-header-box {
    margin-bottom: 16px;
}

.page-title {
    font-size: 22px;
    font-weight: 800;
    color: @pbl_text;
}

.page-subtitle {
    font-size: 14px;
    color: @pbl_text_muted;
    margin-top: 4px;
}

/* === CARDS & PANELS === */
.card {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 12px;
    padding: 20px 24px;
    box-shadow: none;
}

.card-title {
    font-size: 17px;
    font-weight: 700;
    color: @pbl_text;
    margin-bottom: 12px;
}

/* === STAT CARDS (ĐẶC TẢ MÀU NGỮ NGHĨA THEO VAI TRÒ & CHỈ SỐ) === */
.stat-card {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-left: 4px solid @pbl_primary;
    border-radius: 12px;
    padding: 16px 20px;
    box-shadow: none;
}

.stat-card-blue {
    border-left: 4px solid @pbl_primary;
}

.stat-card-green {
    border-left: 4px solid @pbl_success;
}

.stat-card-amber, .stat-card-warning, .stat-card-orange {
    border-left: 4px solid @pbl_warning;
}

.stat-card-red, .stat-card-danger {
    border-left: 4px solid @pbl_danger;
}

.stat-card-purple {
    border-left: 4px solid @pbl_purple;
}

.stat-card-cyan {
    border-left: 4px solid @pbl_diamond_border;
}

.stat-icon-box {
    border-radius: 8px;
    padding: 4px 8px;
    font-size: 15px;
    font-weight: 700;
    background-color: @pbl_info_bg;
    color: @pbl_primary;
}

.stat-icon-blue {
    background-color: @pbl_blue_bg;
    color: @pbl_primary;
}

.stat-icon-green {
    background-color: @pbl_success_bg;
    color: @pbl_success;
}

.stat-icon-amber, .stat-icon-warning, .stat-icon-orange {
    background-color: @pbl_warning_bg;
    color: @pbl_warning;
}

.stat-icon-red, .stat-icon-danger {
    background-color: @pbl_danger_bg;
    color: @pbl_danger;
}

.stat-icon-purple {
    background-color: @pbl_purple_bg;
    color: @pbl_purple;
}

.stat-icon-cyan {
    background-color: @pbl_diamond_bg;
    color: @pbl_diamond;
}

.stat-label {
    font-size: 12px;
    font-weight: 700;
    color: @pbl_text_muted;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.stat-value {
    font-size: 26px;
    font-weight: 800;
    color: @pbl_text;
    margin: 8px 0;
    font-variant-numeric: tabular-nums;
}

.stat-trend {
    font-size: 13px;
    color: @pbl_text_muted;
    font-weight: 600;
}

.stat-trend-positive {
    color: @pbl_success;
}

.stat-trend-warning {
    color: @pbl_warning;
}

.stat-trend-danger {
    color: @pbl_danger;
}

/* === BUTTONS === */
button {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Segoe UI Emoji", "Segoe UI Symbol", sans-serif;
    background-image: none;
    box-shadow: none;
    text-shadow: none;
    outline: none;
}

button:focus, button:focus-visible {
    outline: 2px solid @pbl_focus_ring;
    outline-offset: 2px;
}

button:disabled {
    opacity: 0.5;
}

button.btn-primary, .btn-primary {
    background-color: @pbl_primary;
    background-image: none;
    color: #ffffff;
    border: none;
    border-radius: 8px;
    padding: 8px 24px;
    min-height: 24px;
    font-size: 15px;
    font-weight: 700;
    transition: all 120ms ease;
}

button.btn-primary:hover, .btn-primary:hover {
    background-color: @pbl_primary_hover;
    background-image: none;
}

button.btn-primary:active, .btn-primary:active {
    background-color: @pbl_primary_active;
    background-image: none;
}

button.btn-success, .btn-success {
    background-color: @pbl_success;
    background-image: none;
    color: #ffffff;
    border: none;
    border-radius: 8px;
    padding: 8px 24px;
    min-height: 24px;
    font-size: 15px;
    font-weight: 700;
    transition: all 120ms ease;
}

button.btn-success:hover, .btn-success:hover {
    background-color: @pbl_success_hover;
    background-image: none;
}

button.btn-success:active, .btn-success:active {
    background-color: @pbl_success_active;
    background-image: none;
}

button.btn-danger, .btn-danger {
    background-color: @pbl_danger;
    background-image: none;
    color: #ffffff;
    border: none;
    border-radius: 8px;
    padding: 8px 24px;
    min-height: 24px;
    font-size: 15px;
    font-weight: 700;
    transition: all 120ms ease;
}

button.btn-danger:hover, .btn-danger:hover {
    background-color: @pbl_danger_hover;
    background-image: none;
}

button.btn-danger:active, .btn-danger:active {
    background-color: @pbl_danger_active;
    background-image: none;
}

button.btn-secondary, .btn-secondary {
    background-color: #e2e8f0;
    background-image: none;
    color: @pbl_text;
    border: none;
    border-radius: 8px;
    padding: 8px 24px;
    min-height: 24px;
    font-size: 15px;
    font-weight: 700;
    transition: all 120ms ease;
}

button.btn-secondary:hover, .btn-secondary:hover {
    background-color: @pbl_border;
    background-image: none;
}

button.btn-secondary:active, .btn-secondary:active {
    background-color: @pbl_border_strong;
    background-image: none;
}

button.btn-outline, .btn-outline {
    background-color: transparent;
    background-image: none;
    color: @pbl_navy_2;
    border: 2px solid @pbl_border_strong;
    border-radius: 8px;
    padding: 6px 24px;
    min-height: 24px;
    font-size: 15px;
    font-weight: 700;
    transition: all 120ms ease;
}

button.btn-outline:hover, .btn-outline:hover {
    background-color: @pbl_bg;
    background-image: none;
    border-color: @pbl_text_muted;
}

button.btn-outline:active, .btn-outline:active {
    background-color: #e2e8f0;
    background-image: none;
}

button.btn-back, .btn-back {
    background-color: @pbl_surface_alt;
    background-image: none;
    color: @pbl_navy_2;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    padding: 8px 16px;
    min-height: 24px;
    font-size: 15px;
    font-weight: 700;
    transition: all 120ms ease;
}

button.btn-back:hover, .btn-back:hover {
    background-color: #e2e8f0;
    background-image: none;
    color: @pbl_text;
}

/* Nút thanh toán lớn của màn POS */
button.btn-pay, .btn-pay {
    padding: 16px 24px;
    min-height: 32px;
    font-size: 18px;
    font-weight: 800;
    border-radius: 8px;
}

/* === FORM INPUTS === */
entry {
    background-color: @pbl_surface;
    border: 2px solid @pbl_border_strong;
    border-radius: 8px;
    padding: 8px 16px;
    min-height: 24px;
    font-size: 15px;
    color: @pbl_text;
    transition: border-color 150ms ease;
}

entry:focus {
    border-color: @pbl_primary;
    outline: 2px solid alpha(@pbl_primary, 0.22);
    outline-offset: 0px;
    box-shadow: none;
}

dropdown button:focus {
    border-color: @pbl_primary;
    outline: 2px solid alpha(@pbl_primary, 0.22);
    outline-offset: 0px;
}

/* Thanh công cụ lọc và đếm bản ghi chuẩn doanh nghiệp */
.filter-toolbar {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 10px;
    padding: 12px 16px;
}

.record-count-badge {
    background-color: @pbl_surface_alt;
    color: @pbl_text_muted;
    border: 1px solid @pbl_border;
    border-radius: 12px;
    padding: 4px 12px;
    font-size: 13px;
    font-weight: 700;
}

/* Ô tìm kiếm / quét mã của POS: to hơn, luôn nổi bật */
entry.pos-search {
    font-size: 18px;
    padding: 16px;
    min-height: 32px;
    border: 2px solid @pbl_primary;
}

.form-label {
    font-size: 14px;
    font-weight: 700;
    color: @pbl_navy_2;
    margin-bottom: 4px;
}

.form-error-label {
    font-size: 12px;
    font-weight: 600;
    color: @pbl_danger;
    margin-top: 3px;
    margin-bottom: 2px;
}

entry.input-error,
dropdown.input-error {
    border-color: @pbl_danger;
    background-color: alpha(@pbl_danger_bg, 0.4);
}

entry.input-error:focus,
dropdown.input-error:focus {
    border-color: @pbl_danger;
    box-shadow: none;
}

/* === STATUS BADGES === */
.badge {
    border-radius: 12px;
    padding: 4px 12px;
    font-size: 13px;
    font-weight: 800;
}

.badge-pending {
    background-color: @pbl_warning_bg;
    color: @pbl_warning;
    border: 1px solid @pbl_warning_border;
}

.badge-confirmed {
    background-color: @pbl_info_bg;
    color: @pbl_info;
    border: 1px solid @pbl_info_border;
}

.badge-preparing {
    background-color: @pbl_purple_bg;
    color: @pbl_purple;
    border: 1px solid @pbl_purple_border;
}

.badge-ready {
    background-color: @pbl_blue_bg;
    color: @pbl_primary_hover;
    border: 1px solid @pbl_blue_border;
}

.badge-shipping {
    background-color: @pbl_orange_bg;
    color: @pbl_orange;
    border: 1px solid @pbl_orange_border;
}

.badge-completed {
    background-color: @pbl_success_bg;
    color: @pbl_success_hover;
    border: 1px solid @pbl_success_border;
}

.badge-cancelled {
    background-color: @pbl_danger_bg;
    color: @pbl_danger_hover;
    border: 1px solid @pbl_danger_border;
}

.badge-paid {
    background-color: @pbl_success_bg;
    color: @pbl_success_hover;
    border: 1px solid @pbl_success_border;
}

.badge-unpaid {
    background-color: @pbl_danger_bg;
    color: @pbl_danger_hover;
    border: 1px solid @pbl_danger_border;
}

.badge-in-stock {
    background-color: @pbl_success_bg;
    color: @pbl_success_hover;
    border: 1px solid @pbl_success_border;
}

.badge-low-stock {
    background-color: @pbl_warning_bg;
    color: @pbl_warning;
    border: 1px solid @pbl_warning_border;
}

.badge-out-of-stock {
    background-color: @pbl_danger_bg;
    color: @pbl_danger_hover;
    border: 1px solid @pbl_danger_border;
}

.badge-sold {
    background-color: @pbl_bg;
    color: @pbl_text_muted;
    border: 1px solid @pbl_border;
}

.badge-under-warranty {
    background-color: @pbl_info_bg;
    color: @pbl_info;
    border: 1px solid @pbl_info_border;
}

.badge-returned {
    background-color: @pbl_orange_bg;
    color: @pbl_orange;
    border: 1px solid @pbl_orange_border;
}

/* === CUSTOMER RANK BADGES (ĐẶC TẢ MÀU PHÙ HỢP TỪNG HẠNG) === */
.rank-bronze {
    background-color: @pbl_orange_bg;
    color: @pbl_orange;
    border: 2px solid #ea580c;
    border-radius: 12px;
    padding: 4px 12px;
    font-size: 13px;
    font-weight: 800;
}

.rank-silver {
    background-color: @pbl_silver_bg;
    color: @pbl_silver;
    border: 2px solid @pbl_silver_border;
    border-radius: 12px;
    padding: 4px 12px;
    font-size: 13px;
    font-weight: 800;
}

.rank-gold {
    background-color: @pbl_gold_bg;
    color: @pbl_gold;
    border: 2px solid @pbl_gold_border;
    border-radius: 12px;
    padding: 4px 12px;
    font-size: 13px;
    font-weight: 800;
}

.rank-diamond {
    background-color: @pbl_diamond_bg;
    color: @pbl_diamond;
    border: 2px solid @pbl_diamond_border;
    border-radius: 12px;
    padding: 4px 12px;
    font-size: 13px;
    font-weight: 800;
}

/* === PRODUCT CARDS === */
.product-card {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 12px;
    padding: 16px;
    transition: transform 150ms ease, box-shadow 150ms ease;
}

.product-card:hover {
    border-color: #60a5fa;
}

.product-name {
    font-size: 17px;
    font-weight: 700;
    color: @pbl_text;
}

.product-brand {
    font-size: 13px;
    font-weight: 700;
    color: @pbl_text_muted;
}

.product-price {
    font-size: 19px;
    font-weight: 800;
    color: @pbl_danger;
}

/* === TABLES & ROWS === */
.table-container {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 10px;
}

.table-header {
    background-color: @pbl_bg;
    border-bottom: 2px solid @pbl_border;
    border-radius: 8px 8px 0 0;
    padding: 10px 16px;
    font-weight: 700;
    font-size: 13px;
    color: @pbl_navy_3;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.table-header-cell {
    font-weight: 700;
    font-size: 13px;
    color: @pbl_navy_3;
    text-transform: uppercase;
}

.table-row {
    background-color: @pbl_surface;
    border-bottom: 1px solid @pbl_divider;
    padding: 10px 16px;
    transition: background-color 100ms ease;
}

.table-row.row-even {
    background-color: @pbl_surface;
}

.table-row.row-odd {
    background-color: @pbl_surface_alt;
}

.table-row:hover, .table-row:focus, .table-row:focus-within, .table-row.selected {
    background-color: @pbl_surface_hover;
    border-left: 3px solid @pbl_primary;
}

/* Chữ số thẳng cột & nhãn ô dữ liệu bảng */
.mono-num, .tabular-num, .cell-number {
    font-family: "Cascadia Mono", "Consolas", "Segoe UI", monospace;
    font-variant-numeric: tabular-nums;
    font-size: 14px;
    font-weight: 700;
    color: @pbl_text;
}

.cell-code {
    font-family: "Cascadia Mono", "Consolas", "Segoe UI", monospace;
    font-size: 13px;
    font-weight: 700;
    color: @pbl_navy_2;
    background-color: @pbl_surface_alt;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    padding: 4px 8px;
}

.cell-title {
    font-size: 15px;
    font-weight: 700;
    color: @pbl_text;
}

.cell-subtitle {
    font-size: 13px;
    font-weight: 500;
    color: @pbl_text_muted;
}

.stock-critical {
    font-variant-numeric: tabular-nums;
    font-weight: 800;
    color: @pbl_danger;
}

.stock-low {
    font-variant-numeric: tabular-nums;
    font-weight: 800;
    color: @pbl_warning;
}

.stock-ok {
    font-variant-numeric: tabular-nums;
    font-weight: 700;
    color: @pbl_success;
}

/* Thanh tiến độ / tỷ trọng báo cáo */
progressbar trough {
    background-color: @pbl_bg;
    border-radius: 6px;
    min-height: 8px;
}

progressbar progress {
    background-color: @pbl_primary;
    border-radius: 6px;
    min-height: 8px;
}

button.btn-action-sm, .btn-action-sm {
    min-height: 28px;
    padding: 4px 10px;
    font-size: 13px;
    font-weight: 600;
    border-radius: 6px;
}

button.btn-action-view, .btn-action-view {
    background-color: @pbl_surface_alt;
    color: @pbl_navy_2;
    border: 1px solid @pbl_border;
    border-radius: 6px;
    padding: 4px 10px;
    min-height: 28px;
    font-size: 13px;
    font-weight: 600;
}
button.btn-action-view:hover, .btn-action-view:hover {
    background-color: #e2e8f0;
    border-color: @pbl_border_strong;
    color: @pbl_text;
}

button.btn-action-edit, .btn-action-edit {
    background-color: transparent;
    color: @pbl_primary;
    border: 1px solid @pbl_blue_border;
    border-radius: 6px;
    padding: 4px 10px;
    min-height: 28px;
    font-size: 13px;
    font-weight: 600;
}
button.btn-action-edit:hover, .btn-action-edit:hover {
    background-color: @pbl_blue_bg;
    border-color: @pbl_primary;
}

button.btn-action-delete, .btn-action-delete {
    background-color: transparent;
    color: @pbl_danger;
    border: 1px solid @pbl_danger_border;
    border-radius: 6px;
    padding: 4px 10px;
    min-height: 28px;
    font-size: 13px;
    font-weight: 600;
}
button.btn-action-delete:hover, .btn-action-delete:hover {
    background-color: @pbl_danger_bg;
    border-color: @pbl_danger;
}

/* === EMPTY & ERROR STATES === */
.state-box {
    padding: 48px 24px;
}

.state-title {
    font-size: 20px;
    font-weight: 800;
    color: @pbl_navy_2;
    margin: 8px 0;
}

.state-desc {
    font-size: 15px;
    color: @pbl_text_muted;
    margin-bottom: 16px;
}

/* === TOAST NOTIFICATIONS === */
.toast-box {
    background: #0f172a;
    color: #ffffff;
    border-radius: 12px;
    padding: 14px 20px;
    box-shadow: none;
    border: 1px solid rgba(255, 255, 255, 0.12);
    min-width: 280px;
    margin: 16px;
}

.toast-box.toast-success {
    border-left: 6px solid #22c55e;
}

.toast-box.toast-error {
    border-left: 6px solid #ef4444;
}

.toast-box.toast-info {
    border-left: 6px solid #3b82f6;
}

.toast-text {
    font-size: 14px;
    font-weight: 600;
    color: #f8fafc;
}

.toast-close-btn {
    background: transparent;
    border: none;
    color: #94a3b8;
    font-size: 13px;
    font-weight: 700;
    padding: 2px 6px;
    border-radius: 6px;
    min-height: 20px;
    min-width: 20px;
}

.toast-close-btn:hover {
    color: #ffffff;
    background: rgba(255, 255, 255, 0.15);
}

/* === POS (BÁN HÀNG TẠI QUẦY) === */
.pos-total-box {
    background: linear-gradient(135deg, @pbl_primary_active 0%, @pbl_navy 100%);
    border-radius: 12px;
    padding: 24px;
}

.pos-total-label {
    color: @pbl_navy_text;
    font-size: 13px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 1px;
}

.pos-total-value {
    color: @pbl_accent;
    font-size: 32px;
    font-weight: 800;
}

.shortcut-hint {
    font-size: 13px;
    color: @pbl_text_muted;
}

.kbd {
    background-color: @pbl_surface_alt;
    border: 1px solid @pbl_border;
    border-radius: 4px;
    padding: 0 8px;
    font-size: 12px;
    font-weight: 700;
    color: @pbl_navy_2;
}

/* === LOGIN SCREEN === */
.login-bg {
    background: linear-gradient(135deg, @pbl_navy 0%, @pbl_navy_2 50%, @pbl_navy 100%);
}

.login-card {
    background-color: @pbl_surface;
    border-radius: 16px;
    padding: 32px 40px;
    border: 1px solid rgba(255, 255, 255, 0.15);
}

.login-title {
    font-size: 26px;
    font-weight: 800;
    color: @pbl_text;
}

.login-subtitle {
    font-size: 14px;
    color: @pbl_text_muted;
    margin-top: 4px;
    margin-bottom: 16px;
}

button.login-quick-pill, .login-quick-pill {
    background-color: @pbl_surface_alt;
    background-image: none;
    color: @pbl_navy_2;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    padding: 8px 16px;
    font-size: 13px;
    font-weight: 700;
    transition: all 120ms ease;
}

button.login-quick-pill:hover, .login-quick-pill:hover {
    background-color: #e2e8f0;
    background-image: none;
    color: @pbl_text;
}

button.login-corner-btn, .login-corner-btn {
    background: transparent;
    background-image: none;
    border: none;
    color: @pbl_primary;
    font-size: 14px;
    font-weight: 700;
    padding: 4px 8px;
}

button.login-corner-btn:hover, .login-corner-btn:hover {
    color: @pbl_primary_hover;
    background-image: none;
    text-decoration: underline;
}

/* === REAL-TIME SIMULATION SCREEN === */
.realtime-clock-box {
    background: linear-gradient(135deg, @pbl_primary_active 0%, @pbl_navy 100%);
    color: #ffffff;
    border-radius: 16px;
    padding: 32px;
    border: 2px solid #3b82f6;
}

.realtime-clock-display {
    font-size: 38px;
    font-weight: 900;
    color: @pbl_accent;
    letter-spacing: 2px;
    font-family: monospace;
}

/* === DIALOGS === */
.dialog-content {
    background-color: @pbl_surface;
    border-radius: 12px;
    padding: 24px;
}

.dialog-title {
    font-size: 20px;
    font-weight: 800;
    color: @pbl_text;
    margin-bottom: 8px;
}

.dialog-message {
    font-size: 15px;
    color: @pbl_navy_3;
    margin-bottom: 24px;
}
)CSS";

inline void applyAppStyles() {
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(provider, APP_CSS_STYLES);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

#endif // APP_CSS_H
