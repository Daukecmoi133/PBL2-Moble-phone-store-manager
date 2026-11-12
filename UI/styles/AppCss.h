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
 *    - CỠ CHỮ (TỐI ƯU CHO NGƯỜI GIÀ & NGƯỜI MẮT YẾU): mặc định toàn cục 18px,
 * tiêu đề trang 28px, tiêu đề thẻ 22px, nội dung/nút/ô nhập 18px, nhãn/số liệu
 * 17px, tiêu đề bảng/huy hiệu/chú thích 16px, chỉ số KPI 32px, tổng tiền POS
 * 38px.
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
constexpr int SPACE_XS = 4; // 0.5x (4px)  - Khoảng cách nội bộ nhỏ, dòng bảng
constexpr int SPACE_SM = 8; // 1.0x (8px)  - Khoảng cách nút bấm, padding nhỏ
constexpr int SPACE_MD =
    12; // 1.5x (12px) - Khoảng cách giữa các trường form / cột
constexpr int SPACE_LG =
    16; // 2.0x (16px) - Khoảng cách lưới thẻ KPI, padding chuẩn
constexpr int SPACE_XL = 20; // 2.5x (20px) - Padding thẻ nội dung vừa
constexpr int SPACE_2XL =
    24; // 3.0x (24px) - Padding khung trang chính (.page-container)
constexpr int SPACE_3XL = 32; // 4.0x (32px) - Khoảng cách phân vùng lớn

// Kích thước khung điều hướng & bảng dữ liệu (bội số của 8px)
constexpr int SIDEBAR_WIDTH = 272;     // 34 * 8px — vừa vặn menu, tối ưu diện tích bảng
constexpr int TABLE_ROW_SPACING = 4;   // 1 * 4px
constexpr int TABLE_CELL_SPACING = 8;  // 2 * 4px — gọn gàng, tối ưu không gian ngang
constexpr int KPI_GRID_SPACING = 16;   // 2 * 8px
} // namespace DesignTokens

// Bộ CSS chuẩn giao diện hiện đại cho GTK4 Phone Store Manager
// Dùng font hệ thống để giao diện phù hợp với từng nền tảng
inline const char *APP_CSS_STYLES = R"CSS(
/* =========================================================================
   DESIGN TOKENS — SỬA MÀU TẠI ĐÂY, TOÀN ỨNG DỤNG TỰ ĐỔI THEO
   Chuẩn hóa theo docs/DESIGN.md
   ========================================================================= */
@define-color navy_900 #0F172A;
@define-color navy_800 #1E293B;
@define-color blue_600 #2563EB;
@define-color blue_700 #1D4ED8;
@define-color blue_100 #DBEAFE;
@define-color sky_400  #38BDF8;
@define-color red_600  #DC2626;
@define-color red_100  #FEE2E2;
@define-color green_700 #15803D;
@define-color green_100 #DCFCE7;
@define-color amber_600 #D97706;
@define-color slate_900 #0F172A;
@define-color slate_600 #475569;
@define-color slate_400 #94A3B8;
@define-color slate_200 #E2E8F0;
@define-color slate_100 #F1F5F9;
@define-color slate_50  #F8FAFC;
@define-color white     #FFFFFF;

/* Chart Tokens (DESIGN.md Sec 8.2) */
@define-color chart_revenue #2563EB;
@define-color chart_profit #15803D;
@define-color chart_apple #2563EB;
@define-color chart_samsung #0D9488;
@define-color chart_oppo #D97706;
@define-color chart_xiaomi #8B5CF6;
@define-color chart_other #94A3B8;
@define-color chart_grid #E2E8F0;
@define-color chart_text #64748B;

/* Bí danh (aliases) pbl_ tương thích toàn bộ hệ thống */
@define-color pbl_primary @blue_600;
@define-color pbl_primary_hover @blue_700;
@define-color pbl_primary_active #1e3a8a;
@define-color pbl_accent @sky_400;
@define-color pbl_focus_ring alpha(#2563eb, 0.45);

/* Nền & viền & chữ */
@define-color pbl_bg @slate_100;
@define-color pbl_surface @white;
@define-color pbl_surface_alt @slate_50;
@define-color pbl_surface_hover #eff6ff;
@define-color pbl_divider @slate_200;
@define-color pbl_border @slate_200;
@define-color pbl_border_strong @slate_400;
@define-color pbl_text @slate_900;
@define-color pbl_text_muted @slate_600;
@define-color pbl_text_subtle @slate_400;
@define-color pbl_text_inverse @slate_50;

/* Nền tối (header, sidebar, màn đăng nhập) */
@define-color pbl_navy @navy_900;
@define-color pbl_navy_2 @navy_800;
@define-color pbl_navy_3 #334155;
@define-color pbl_navy_text #cbd5e1;

/* Màu ngữ nghĩa: thành công / lỗi / cảnh báo */
@define-color pbl_success @green_700;
@define-color pbl_success_hover #166534;
@define-color pbl_success_active #14532d;
@define-color pbl_success_bg @green_100;
@define-color pbl_success_border #86efac;
@define-color pbl_danger @red_600;
@define-color pbl_danger_hover #b91c1c;
@define-color pbl_danger_active #991b1b;
@define-color pbl_danger_bg @red_100;
@define-color pbl_danger_border #fecaca;
@define-color pbl_warning @amber_600;
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
@define-color pbl_blue_bg @blue_100;
@define-color pbl_blue_border #bfdbfe;

/* Màu hạng khách hàng */
@define-color pbl_gold #854d0e;
@define-color pbl_gold_bg #fef9c3;
@define-color pbl_gold_border #ca8a04;
@define-color pbl_diamond #0e7490;
@define-color pbl_diamond_bg #ecfeff;
@define-color pbl_diamond_border #06b6d4;
@define-color pbl_silver #334155;
@define-color pbl_silver_bg @slate_100;
@define-color pbl_silver_border #64748b;

/* =========================================================================
   GLOBAL
   ========================================================================= */
* {
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 18px;
}

window,
dialog,
.background {
    background-color: @pbl_bg;
    color: @pbl_text;
}

/* === HEADER BAR === */
.main-header {
    background-color: @navy_900;
    padding: 12px 24px;
    border-bottom: 1px solid @navy_800;
    color: #ffffff;
}

.app-brand-title {
    font-family: "Inter", "Segoe UI", sans-serif;
    font-size: 18px;
    font-weight: 800;
    color: @sky_400;
    letter-spacing: 0.5px;
}

.header-screen-title {
    font-size: 16px;
    font-weight: 600;
    color: @slate_200;
    padding: 0 8px;
}

.user-profile-badge {
    font-family: "Inter", "Segoe UI", sans-serif;
    background-color: @navy_800;
    color: @slate_100;
    border-radius: 999px;
    padding: 6px 14px;
    font-weight: 600;
    font-size: 13px;
    border: 1px solid #334155;
}

.realtime-header-badge {
    background: transparent;
    color: @slate_400;
    border: none;
    border-radius: 6px;
    padding: 4px 8px;
    font-size: 12px;
    font-weight: 500;
}

.role-pill-admin {
    background-color: @pbl_danger;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 10px;
    font-size: 15px;
    font-weight: 700;
}

.role-pill-sales {
    background-color: @pbl_primary;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 10px;
    font-size: 15px;
    font-weight: 700;
}

.role-pill-warehouse {
    background-color: @pbl_warning;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 10px;
    font-size: 15px;
    font-weight: 700;
}

.role-pill-customer {
    background-color: @pbl_success;
    color: #ffffff;
    border-radius: 12px;
    padding: 4px 10px;
    font-size: 15px;
    font-weight: 700;
}

/* === SIDEBAR === */
.sidebar-panel {
    background-color: @navy_900;
    border-right: 1px solid @navy_800;
    padding: 16px 8px;
}

.sidebar-category-label {
    color: @slate_400;
    font-size: 12px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 1px;
    padding: 12px 8px 4px 8px;
}

.sidebar-divider {
    background-color: @navy_800;
    min-height: 1px;
    margin: 8px 8px;
}

.sidebar-icon-slot {
    min-width: 24px;
}

button.sidebar-btn, .sidebar-btn {
    background-color: transparent;
    background-image: none;
    box-shadow: none;
    color: @pbl_navy_text;
    border: none;
    border-radius: 10px;
    padding: 10px 14px;
    font-size: 15px;
    font-weight: 600;
    margin-bottom: 4px;
    transition: all 150ms ease;
}

button.sidebar-btn:hover, .sidebar-btn:hover {
    background-color: @navy_800;
    background-image: none;
    color: #ffffff;
}

button.sidebar-btn:focus, .sidebar-btn:focus {
    outline: 2px solid @sky_400;
    outline-offset: -2px;
}

button.sidebar-btn-active, .sidebar-btn-active {
    background-color: @blue_600;
    background-image: none;
    box-shadow: none;
    color: #ffffff;
    font-weight: 600;
    border-radius: 10px;
}

button.sidebar-btn-active:hover, .sidebar-btn-active:hover {
    background-color: @blue_700;
    background-image: none;
    color: #ffffff;
}

button.sidebar-btn-active:focus, .sidebar-btn-active:focus {
    outline: 2px solid @sky_400;
    outline-offset: -2px;
}

.cart-count-badge {
    background-color: @red_600;
    color: #ffffff;
    border-radius: 999px;
    padding: 2px 7px;
    font-size: 12px;
    font-weight: 700;
    min-width: 18px;
    min-height: 18px;
}

/* === CONTENT PAGES === */
.page-container {
    padding: 20px 24px;
}

.page-header-box {
    margin-bottom: 16px;
}

.page-title {
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 28px;
    font-weight: 800;
    color: @pbl_text;
}

.page-subtitle {
    font-size: 17px;
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
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 22px;
    font-weight: 700;
    color: @pbl_text;
    margin-bottom: 12px;
}

/* === STAT CARDS (ĐẶC TẢ MÀU NGỮ NGHĨA THEO VAI TRÒ & CHỈ SỐ) === */
.stat-card {
    background-color: @white;
    border: 1px solid @slate_200;
    border-left: 4px solid @blue_600;
    border-radius: 12px;
    padding: 16px 20px;
    box-shadow: 0 1px 2px rgba(15, 23, 42, 0.06);
    transition: all 200ms ease-out;
}

.stat-card.clickable {
    /* Clickable KPI card styling */
}

.stat-card.clickable:hover {
    box-shadow: 0 4px 12px rgba(15, 23, 42, 0.10);
    border-color: @slate_400;
    background-color: @pbl_surface_hover;
}

.stat-card.clickable:focus {
    outline: 2px solid @pbl_focus_ring;
    outline-offset: 2px;
}

.stat-card-blue {
    border-left: 4px solid @blue_600;
}

.stat-card-green {
    border-left: 4px solid @green_700;
}

.stat-card-amber, .stat-card-warning, .stat-card-orange {
    border-left: 4px solid @amber_600;
}

.stat-card-red, .stat-card-danger {
    border-left: 4px solid @red_600;
}

.stat-card-purple {
    border-left: 4px solid @pbl_purple;
}

.stat-card-cyan {
    border-left: 4px solid @pbl_diamond_border;
}

.stat-icon-box {
    border-radius: 10px;
    padding: 6px;
    min-width: 36px;
    min-height: 36px;
    background-color: @blue_100;
    color: @blue_600;
}

.stat-icon-blue {
    background-color: @blue_100;
    color: @blue_600;
}

.stat-icon-green {
    background-color: @green_100;
    color: @green_700;
}

.stat-icon-amber, .stat-icon-warning, .stat-icon-orange {
    background-color: #fef3c7;
    color: @amber_600;
}

.stat-icon-red, .stat-icon-danger {
    background-color: @red_100;
    color: @red_600;
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
    font-weight: 600;
    color: @slate_600;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.stat-value {
    font-size: 28px;
    font-weight: 700;
    color: @slate_900;
    margin: 6px 0;
    font-variant-numeric: tabular-nums;
}

.stat-trend {
    font-size: 13px;
    color: @slate_600;
    font-weight: 500;
}

.stat-trend-positive {
    color: @green_700;
    font-weight: 600;
}

.stat-trend-warning {
    color: @amber_600;
    font-weight: 600;
}

.stat-trend-danger {
    color: @red_600;
    font-weight: 600;
}

/* === BUTTONS === */
button {
    font-family: "Inter", "Segoe UI", sans-serif;
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
    background-color: @blue_600;
    background-image: none;
    color: #ffffff;
    border: none;
    border-radius: 10px;
    padding: 0 16px;
    min-height: 36px;
    font-size: 14px;
    font-weight: 600;
    transition: all 120ms ease;
}

button.btn-primary:hover, .btn-primary:hover {
    background-color: @blue_700;
    background-image: none;
}

button.btn-primary:active, .btn-primary:active {
    background-color: #1e3a8a;
    background-image: none;
}

button.btn-success, .btn-success {
    background-color: @green_700;
    background-image: none;
    color: #ffffff;
    border: none;
    border-radius: 10px;
    padding: 0 16px;
    min-height: 36px;
    font-size: 14px;
    font-weight: 600;
    transition: all 120ms ease;
}

button.btn-success:hover, .btn-success:hover {
    background-color: #166534;
    background-image: none;
}

button.btn-success:active, .btn-success:active {
    background-color: #14532d;
    background-image: none;
}

button.btn-danger, .btn-danger {
    background-color: @red_600;
    background-image: none;
    color: #ffffff;
    border: none;
    border-radius: 10px;
    padding: 0 16px;
    min-height: 36px;
    font-size: 14px;
    font-weight: 600;
    transition: all 120ms ease;
}

button.btn-danger:hover, .btn-danger:hover {
    background-color: #b91c1c;
    background-image: none;
}

button.btn-danger:active, .btn-danger:active {
    background-color: #991b1b;
    background-image: none;
}

button.btn-secondary, .btn-secondary {
    background-color: @white;
    background-image: none;
    color: @slate_900;
    border: 1px solid @slate_200;
    border-radius: 10px;
    padding: 0 16px;
    min-height: 36px;
    font-size: 14px;
    font-weight: 600;
    transition: all 120ms ease;
}

button.btn-secondary:hover, .btn-secondary:hover {
    background-color: @slate_100;
    border-color: @slate_400;
    background-image: none;
}

button.btn-secondary:active, .btn-secondary:active {
    background-color: @slate_200;
    background-image: none;
}

button.btn-outline, .btn-outline {
    background-color: transparent;
    background-image: none;
    color: @blue_600;
    border: 1px solid @slate_200;
    border-radius: 10px;
    padding: 0 16px;
    min-height: 36px;
    font-size: 14px;
    font-weight: 600;
    transition: all 120ms ease;
}

button.btn-outline:hover, .btn-outline:hover {
    background-color: @blue_100;
    background-image: none;
    border-color: @blue_600;
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
    padding: 10px 18px;
    min-height: 32px;
    font-size: 18px;
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
    padding: 16px 28px;
    min-height: 40px;
    font-size: 22px;
    font-weight: 800;
    border-radius: 8px;
}

/* === FORM INPUTS === */
entry {
    background-color: @pbl_surface;
    border: 2px solid @pbl_border_strong;
    border-radius: 8px;
    padding: 10px 16px;
    min-height: 32px;
    font-size: 18px;
    color: @pbl_text;
    transition: border-color 150ms ease;
}

entry:focus {
    border-color: @pbl_primary;
    outline: 2px solid alpha(@pbl_primary, 0.22);
    outline-offset: 0px;
    box-shadow: none;
}

dropdown button {
    padding: 8px 14px;
    min-height: 32px;
    font-size: 18px;
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
    padding: 6px 14px;
    font-size: 16px;
    font-weight: 700;
}

/* Ô tìm kiếm / quét mã của POS: to hơn, luôn nổi bật */
entry.pos-search {
    font-size: 22px;
    padding: 16px;
    min-height: 38px;
    border: 2px solid @pbl_primary;
}

.form-label {
    font-size: 17px;
    font-weight: 700;
    color: @pbl_navy_2;
    margin-bottom: 4px;
}

.form-error-label {
    font-size: 15px;
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
    padding: 6px 14px;
    font-size: 16px;
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
    padding: 6px 14px;
    font-size: 16px;
    font-weight: 800;
}

.rank-silver {
    background-color: @pbl_silver_bg;
    color: @pbl_silver;
    border: 2px solid @pbl_silver_border;
    border-radius: 12px;
    padding: 6px 14px;
    font-size: 16px;
    font-weight: 800;
}

.rank-gold {
    background-color: @pbl_gold_bg;
    color: @pbl_gold;
    border: 2px solid @pbl_gold_border;
    border-radius: 12px;
    padding: 6px 14px;
    font-size: 16px;
    font-weight: 800;
}

.rank-diamond {
    background-color: @pbl_diamond_bg;
    color: @pbl_diamond;
    border: 2px solid @pbl_diamond_border;
    border-radius: 12px;
    padding: 6px 14px;
    font-size: 16px;
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
    font-size: 21px;
    font-weight: 700;
    color: @pbl_text;
}

.product-brand {
    font-size: 16px;
    font-weight: 700;
    color: @pbl_text_muted;
}

.product-price {
    font-size: 23px;
    font-weight: 800;
    color: @pbl_danger;
}

/* === TABLES & ROWS === */
.table-container {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 10px;
}

.table-body-scroll {
    background-color: transparent;
    border-radius: 0 0 10px 10px;
}

.table-body-list {
    background-color: transparent;
}

.table-header {
    background-color: @pbl_bg;
    border-bottom: 2px solid @pbl_border;
    border-radius: 8px 8px 0 0;
    padding: 10px 14px;
    border-left: 3px solid transparent;
    font-weight: 700;
    font-size: 14px;
    color: @pbl_navy_3;
    text-transform: uppercase;
    letter-spacing: 0.3px;
}

.table-header-cell {
    font-weight: 700;
    font-size: 14px;
    color: @pbl_navy_3;
    text-transform: uppercase;
    letter-spacing: 0.3px;
}

.table-row {
    background-color: @pbl_surface;
    border-bottom: 1px solid @pbl_divider;
    padding: 10px 14px;
    border-left: 3px solid transparent;
    transition: background-color 100ms ease;
}

/* Đảm bảo huy hiệu, mã định danh, giá tiền và nút thao tác trong hàng dữ liệu bảng luôn hiển thị đầy đủ, không bị cắt chữ hay đẩy lệch cột */
.table-row .badge,
.table-row .rank-bronze,
.table-row .rank-silver,
.table-row .rank-gold,
.table-row .rank-diamond {
    font-size: 13px;
    padding: 4px 10px;
    border-radius: 10px;
}

.table-row .product-price {
    font-size: 16px;
    font-weight: 800;
    color: @pbl_danger;
    font-variant-numeric: tabular-nums;
}

.table-row .mono-num,
.table-row .tabular-num,
.table-row .cell-number {
    font-size: 15px;
}

.table-row .cell-code,
.table-row .cell-code-ord,
.table-row .cell-code-inv {
    font-size: 13.5px;
    padding: 3px 8px;
}

.table-row .cell-title {
    font-size: 16px;
}

.table-row .cell-subtitle {
    font-size: 14px;
}

.table-row button.btn-action-sm,
.table-row .btn-action-sm,
.table-row button.btn-table-action,
.table-row .btn-table-action {
    min-height: 30px;
    padding: 4px 10px;
    font-size: 13.5px;
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
    font-size: 17px;
    font-weight: 700;
    color: @pbl_text;
}

.cell-code {
    font-family: "Cascadia Mono", "Consolas", "Segoe UI", monospace;
    font-size: 16px;
    font-weight: 700;
    color: @pbl_navy_2;
    background-color: @pbl_surface_alt;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    padding: 4px 10px;
}

.cell-code-ord {
    font-family: "Cascadia Mono", "Consolas", "Segoe UI", monospace;
    font-size: 16px;
    font-weight: 700;
    color: @pbl_silver;
    background-color: @pbl_silver_bg;
    border: 1px solid @pbl_silver_border;
    border-radius: 8px;
    padding: 4px 10px;
}

.cell-code-inv {
    font-family: "Cascadia Mono", "Consolas", "Segoe UI", monospace;
    font-size: 16px;
    font-weight: 700;
    color: @pbl_primary_hover;
    background-color: @pbl_blue_bg;
    border: 1px solid @pbl_blue_border;
    border-radius: 8px;
    padding: 4px 10px;
}

.cell-title {
    font-size: 18px;
    font-weight: 700;
    color: @pbl_text;
}

.cell-subtitle {
    font-size: 16px;
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
    min-height: 10px;
}

progressbar progress {
    background-color: @pbl_primary;
    border-radius: 6px;
    min-height: 10px;
}

button.btn-action-sm, .btn-action-sm,
button.btn-table-action, .btn-table-action {
    min-height: 34px;
    padding: 6px 12px;
    font-size: 16px;
    font-weight: 600;
    border-radius: 6px;
}

button.btn-action-view, .btn-action-view {
    background-color: @pbl_surface_alt;
    color: @pbl_navy_2;
    border: 1px solid @pbl_border;
    border-radius: 6px;
    padding: 6px 12px;
    min-height: 34px;
    font-size: 16px;
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
    padding: 6px 12px;
    min-height: 34px;
    font-size: 16px;
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
    padding: 6px 12px;
    min-height: 34px;
    font-size: 16px;
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
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 24px;
    font-weight: 800;
    color: @pbl_navy_2;
    margin: 8px 0;
}

.state-desc {
    font-size: 18px;
    color: @pbl_text_muted;
    margin-bottom: 16px;
}

/* === TOAST NOTIFICATIONS === */
.toast-box {
    background: #0f172a;
    color: #ffffff;
    border-radius: 12px;
    padding: 16px 22px;
    box-shadow: none;
    border: 1px solid rgba(255, 255, 255, 0.12);
    min-width: 320px;
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
    font-size: 17px;
    font-weight: 600;
    color: #f8fafc;
}

.toast-close-btn {
    background: transparent;
    border: none;
    color: #94a3b8;
    font-size: 16px;
    font-weight: 700;
    padding: 4px 8px;
    border-radius: 6px;
    min-height: 28px;
    min-width: 28px;
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
    font-size: 16px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 1px;
}

.pos-total-value {
    color: @pbl_accent;
    font-size: 38px;
    font-weight: 800;
}

.shortcut-hint {
    font-size: 16px;
    color: @pbl_text_muted;
}

.kbd {
    background-color: @pbl_surface_alt;
    border: 1px solid @pbl_border;
    border-radius: 4px;
    padding: 2px 8px;
    font-size: 15px;
    font-weight: 700;
    color: @pbl_navy_2;
}

/* === LOGIN SCREEN === */
.login-bg {
    background-image: url("C:/DUY/PBL2/data/Device/productImages/D&N Store Phone Sales Screen.png");
}

.login-card {
    background-color: @pbl_surface;
    border-radius: 16px;
    padding: 32px 40px;
    border: 1px solid rgba(255, 255, 255, 0.15);
}

.login-title {
    font-size: 30px;
    font-weight: 800;
    color: @pbl_text;
}

.login-subtitle {
    font-size: 17px;
    color: @pbl_text_muted;
    margin-top: 4px;
    margin-bottom: 16px;
}

.login-alert {
    background-color: @pbl_danger_bg;
    color: @pbl_danger;
    border: 1px solid @pbl_danger_border;
    border-radius: 8px;
    padding: 12px 16px;
    font-size: 16px;
    font-weight: 600;
}

button.login-quick-pill, .login-quick-pill {
    background-color: @pbl_surface_alt;
    background-image: none;
    color: @pbl_navy_2;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    padding: 10px 16px;
    font-size: 16px;
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
    font-size: 17px;
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

.realtime-recon-box {
    background-color: #f0f7ff;
    border: 1px solid #bfdbfe;
    border-radius: 12px;
    padding: 16px 20px;
    color: @pbl_navy;
}

.realtime-recon-box .card-title {
    color: @pbl_primary_active;
    font-size: 18px;
    font-weight: 700;
    margin-bottom: 2px;
}

.realtime-recon-box .stat-label {
    color: @pbl_navy_3;
    font-size: 14px;
    font-weight: 800;
}

.realtime-recon-box .stat-trend {
    font-size: 16px;
    font-weight: 600;
}

.realtime-clock-display {
    font-size: 44px;
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
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 24px;
    font-weight: 800;
    color: @pbl_text;
    margin-bottom: 8px;
}

.dialog-message {
    font-size: 18px;
    color: @pbl_navy_3;
    margin-bottom: 24px;
}

/* === IN-WINDOW PAGE STACK NAVIGATION & OVERLAY DIALOGS === */
.page-container-view {
    background-color: @pbl_bg;
}

.page-header {
    background-color: @pbl_surface;
    border-bottom: 1px solid @pbl_border;
    padding: 12px 24px;
    min-height: 52px;
}

button.page-back-btn, .page-back-btn {
    background-color: @pbl_surface_alt;
    background-image: none;
    color: @pbl_primary;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    font-size: 16px;
    font-weight: 700;
    padding: 8px 16px;
    transition: all 120ms ease;
}

button.page-back-btn:hover, .page-back-btn:hover {
    background-color: @pbl_surface_hover;
    background-image: none;
    color: @pbl_primary_hover;
    border-color: @pbl_primary;
}

button.page-back-btn:active, .page-back-btn:active {
    background-color: @pbl_blue_bg;
    background-image: none;
    color: @pbl_primary_active;
    border-color: @pbl_primary_active;
}

.page-title {
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 22px;
    font-weight: 800;
    color: @pbl_text;
}

.overlay-scrim {
    background-color: rgba(15, 23, 42, 0.65);
    opacity: 1;
    transition: opacity 0.22s ease-out;
}

.overlay-scrim.overlay-entering {
    opacity: 0;
}

.overlay-card {
    background-color: @pbl_surface;
    color: @pbl_text;
    border: 1px solid @pbl_border;
    border-radius: 16px;
    padding: 28px 32px;
    box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.3), 0 8px 10px -6px rgba(0, 0, 0, 0.2);
    min-width: 440px;
}

.overlay-dialog-header {
    padding-bottom: 10px;
    border-bottom: 1px solid @pbl_divider;
    margin-bottom: 4px;
}

.overlay-dialog-icon {
    font-size: 24px;
}

.overlay-dialog-title {
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 22px;
    font-weight: 800;
    color: @pbl_text;
    margin-bottom: 0;
}

.overlay-dialog-message {
    font-size: 18px;
    color: @pbl_navy_3;
    margin-top: 4px;
    margin-bottom: 12px;
}

.overlay-dialog-actions {
    margin-top: 8px;
    padding-top: 12px;
    border-top: 1px solid @pbl_divider;
}

button.overlay-dialog-btn, .overlay-dialog-btn {
    min-width: 112px;
    padding: 10px 20px;
    border-radius: 8px;
    font-size: 17px;
    font-weight: 700;
}

/* === PRODUCT DETAIL PAGE (PDP — E-COMMERCE STYLE) === */
.pdp-breadcrumb-bar {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 10px;
    padding: 8px 16px;
}

button.pdp-breadcrumb-link, .pdp-breadcrumb-link {
    background: transparent;
    background-image: none;
    border: none;
    box-shadow: none;
    color: @pbl_primary;
    font-size: 16px;
    font-weight: 600;
    padding: 2px 6px;
}

button.pdp-breadcrumb-link:hover, .pdp-breadcrumb-link:hover {
    color: @pbl_primary_hover;
    background-color: @pbl_surface_hover;
    border-radius: 6px;
}

.pdp-breadcrumb-sep {
    color: @pbl_text_subtle;
    font-size: 16px;
    font-weight: 700;
}

.pdp-breadcrumb-current {
    color: @pbl_text;
    font-size: 16px;
    font-weight: 700;
}

.pdp-card {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 16px;
    padding: 24px;
    box-shadow: 0 4px 12px rgba(15, 23, 42, 0.05);
}

.pdp-image-stage {
    background: linear-gradient(145deg, @pbl_surface_alt 0%, #ffffff 100%);
    border: 1px solid @pbl_divider;
    border-radius: 14px;
    padding: 20px;
    box-shadow: inset 0 1px 4px rgba(15, 23, 42, 0.03);
}

button.pdp-thumb-btn, .pdp-thumb-btn {
    background-color: @pbl_surface_alt;
    background-image: none;
    border: 2px solid @pbl_border;
    border-radius: 10px;
    padding: 6px;
    min-width: 64px;
    min-height: 64px;
    transition: all 120ms ease;
}

button.pdp-thumb-btn:hover, .pdp-thumb-btn:hover {
    background-color: @pbl_surface_hover;
    border-color: @pbl_primary;
}

button.pdp-thumb-active, .pdp-thumb-active {
    background-color: @pbl_surface_hover;
    border-color: @pbl_primary;
    box-shadow: 0 0 0 2px @pbl_blue_border;
}

.pdp-title {
    font-family: "Segoe UI", "Noto Color Emoji", "Segoe UI Emoji", sans-serif;
    font-size: 28px;
    font-weight: 800;
    color: @pbl_text;
}

.pdp-price-box {
    background: linear-gradient(135deg, #fff1f2 0%, @pbl_surface_alt 100%);
    border: 1px solid @pbl_danger_border;
    border-radius: 12px;
    padding: 14px 20px;
}

.pdp-price {
    font-size: 32px;
    font-weight: 800;
    color: @pbl_danger;
}

.pdp-old-price {
    font-size: 18px;
    font-weight: 600;
    color: @pbl_text_subtle;
    text-decoration: line-through;
}

.pdp-badge {
    border-radius: 8px;
    padding: 4px 12px;
    font-size: 15px;
    font-weight: 700;
    background-color: @pbl_info_bg;
    color: @pbl_info;
    border: 1px solid @pbl_info_border;
}

.pdp-badge-discount {
    background-color: @pbl_danger;
    color: #ffffff;
    border-radius: 6px;
    padding: 3px 10px;
    font-size: 15px;
    font-weight: 800;
}

button.pdp-chip, .pdp-chip {
    background-color: @pbl_surface_alt;
    background-image: none;
    color: @pbl_text;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    padding: 8px 16px;
    font-size: 16px;
    font-weight: 600;
    transition: all 120ms ease;
}

button.pdp-chip:hover, .pdp-chip:hover {
    background-color: @pbl_surface_hover;
    border-color: @pbl_primary;
    color: @pbl_primary;
}

button.pdp-chip:active, .pdp-chip:active,
button.pdp-chip-active, .pdp-chip-active {
    background-color: @pbl_blue_bg;
    background-image: none;
    border: 2px solid @pbl_primary;
    color: @pbl_primary_active;
    font-weight: 700;
}

button.pdp-chip:disabled, .pdp-chip:disabled {
    background-color: @pbl_divider;
    color: @pbl_text_subtle;
    border-color: @pbl_border;
    opacity: 0.55;
}

button.pdp-qty-btn, .pdp-qty-btn {
    background-color: @pbl_surface_alt;
    background-image: none;
    color: @pbl_text;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    min-width: 40px;
    min-height: 40px;
    padding: 4px 10px;
    font-size: 20px;
    font-weight: 800;
}

button.pdp-qty-btn:hover, .pdp-qty-btn:hover {
    background-color: @pbl_surface_hover;
    border-color: @pbl_primary;
    color: @pbl_primary;
}

button.pdp-qty-btn:active, .pdp-qty-btn:active {
    background-color: @pbl_blue_bg;
}

button.pdp-qty-btn:disabled, .pdp-qty-btn:disabled {
    opacity: 0.45;
}

.pdp-qty-value {
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    border-radius: 8px;
    min-width: 56px;
    min-height: 40px;
    padding: 6px 12px;
    font-size: 18px;
    font-weight: 800;
    color: @pbl_text;
}

button.pdp-cart-btn, .pdp-cart-btn {
    background-color: @pbl_surface_hover;
    background-image: none;
    color: @pbl_primary;
    border: 2px solid @pbl_primary;
    border-radius: 10px;
    padding: 12px 24px;
    font-size: 18px;
    font-weight: 800;
    transition: all 120ms ease;
}

button.pdp-cart-btn:hover, .pdp-cart-btn:hover {
    background-color: @pbl_blue_bg;
    background-image: none;
    border-color: @pbl_primary_hover;
    color: @pbl_primary_hover;
}

button.pdp-cart-btn:active, .pdp-cart-btn:active {
    background-color: @pbl_blue_border;
    color: @pbl_primary_active;
}

button.pdp-cart-btn:disabled, .pdp-cart-btn:disabled {
    background-color: @pbl_divider;
    border-color: @pbl_border;
    color: @pbl_text_subtle;
    opacity: 0.6;
}

button.pdp-buy-btn, .pdp-buy-btn {
    background: linear-gradient(135deg, @pbl_danger 0%, @pbl_danger_hover 100%);
    color: #ffffff;
    border: none;
    border-radius: 10px;
    padding: 12px 28px;
    font-size: 18px;
    font-weight: 800;
    transition: all 120ms ease;
}

button.pdp-buy-btn:hover, .pdp-buy-btn:hover {
    background: linear-gradient(135deg, @pbl_danger_hover 0%, @pbl_danger_active 100%);
    color: #ffffff;
}

button.pdp-buy-btn:active, .pdp-buy-btn:active {
    background-color: @pbl_danger_active;
    background-image: none;
}

button.pdp-buy-btn:disabled, .pdp-buy-btn:disabled {
    background: @pbl_border;
    background-image: none;
    color: @pbl_text_subtle;
    opacity: 0.6;
}

.pdp-policy-box {
    background-color: @pbl_surface_alt;
    border: 1px solid @pbl_divider;
    border-radius: 12px;
    padding: 14px 18px;
}

.pdp-spec-row {
    padding: 10px 14px;
    border-radius: 8px;
    border-bottom: 1px solid @pbl_divider;
}

.pdp-related-card {
    background-color: @pbl_surface_alt;
    border: 1px solid @pbl_border;
    border-radius: 12px;
    padding: 16px;
    transition: all 120ms ease;
}

.pdp-related-card:hover {
    background-color: @pbl_surface_hover;
    border-color: @pbl_primary;
}

/* === VOUCHER & DISCOUNT CARDS === */
.voucher-card {
    border-radius: 12px;
    padding: 16px 20px;
    background-color: @pbl_surface;
    border: 1px solid @pbl_border;
    transition: all 120ms ease;
}

.voucher-card:hover {
    border-color: @pbl_primary;
    background-color: @pbl_surface_hover;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.05);
}

.voucher-badge-percent {
    background: linear-gradient(135deg, rgba(239, 68, 68, 0.12), rgba(249, 115, 22, 0.12));
    border: 1px solid rgba(239, 68, 68, 0.3);
    border-radius: 10px;
    padding: 10px 8px;
}

.voucher-badge-fixed {
    background: linear-gradient(135deg, rgba(16, 185, 129, 0.12), rgba(6, 182, 212, 0.12));
    border: 1px solid rgba(16, 185, 129, 0.3);
    border-radius: 10px;
    padding: 10px 8px;
}

.voucher-badge-expired {
    background: rgba(148, 163, 184, 0.12);
    border: 1px solid rgba(148, 163, 184, 0.3);
    border-radius: 10px;
    padding: 10px 8px;
    opacity: 0.7;
}

.voucher-code-badge {
    background-color: @blue_100;
    color: @blue_700;
    font-family: 'JetBrains Mono', 'Cascadia Mono', monospace;
    font-size: 12px;
    font-weight: 600;
    padding: 3px 10px;
    border-radius: 6px;
    border: 1px solid #bfdbfe;
}

.voucher-expiry-normal {
    font-family: "Inter", "Segoe UI", sans-serif;
    color: @slate_600;
    font-size: 13px;
}

.voucher-expiry-urgent {
    font-family: "Inter", "Segoe UI", sans-serif;
    color: @amber_600;
    font-size: 13px;
    font-weight: 600;
}

.voucher-copy-btn {
    background-color: @white;
    color: @slate_900;
    border: 1px solid @slate_200;
    border-radius: 6px;
    padding: 2px 10px;
    font-size: 12px;
    font-weight: 600;
    min-height: 28px;
    transition: all 120ms ease;
}

.voucher-copy-btn:hover {
    background-color: @slate_100;
    border-color: @slate_400;
}

/* === SMOOTH INTERACTIVE TRANSITIONS === */
.sidebar-btn, .btn-primary, .btn-secondary, .pdp-buy-btn, .pdp-cart-btn {
    transition: background-color 0.18s cubic-bezier(0.2, 0, 0, 1),
                color 0.18s cubic-bezier(0.2, 0, 0, 1),
                transform 0.12s ease-out,
                box-shadow 0.18s ease;
}

.overlay-scrim {
    transition: opacity 0.22s ease-out;
}

/* === PDP ADMIN EDIT & IN-PLACE STYLING (ANTI.MD) === */
.pdp-admin-panel {
    background-color: @navy_800;
    color: #ffffff;
    border: 1px solid @slate_600;
    border-radius: 14px;
    padding: 16px 20px;
}

.pdp-admin-panel .stat-label {
    color: @slate_400;
    font-size: 13px;
    font-weight: 600;
}

.pdp-admin-panel .stat-value {
    color: #ffffff;
    font-size: 20px;
    font-weight: 700;
}

.pdp-unsaved-badge {
    background-color: #FEF3C7;
    color: #92400E;
    border: 1px solid #FCD34D;
    border-radius: 9999px;
    padding: 4px 12px;
    font-size: 13px;
    font-weight: 700;
}

.pdp-editable {
    border: 1px dashed @slate_400;
    border-radius: 8px;
    padding: 6px 10px;
    background-color: #ffffff;
    transition: all 0.15s ease-in-out;
}

.pdp-editable:hover, .pdp-editable:focus {
    border-color: @blue_600;
    border-style: solid;
    box-shadow: 0 0 0 2px rgba(37, 99, 235, 0.2);
}

.pdp-edit-title {
    font-size: 22px;
    font-weight: 800;
    color: @navy_900;
}

.pdp-action-bar {
    background-color: @white;
    border-top: 1px solid @slate_200;
    padding: 16px 24px;
    border-radius: 12px;
}

/* =========================================================================
   CHART & REPORT COMPONENTS (DESIGN.MD SECTION 8)
   ========================================================================= */
.chart-card {
    background-color: @white;
    border: 1px solid @slate_200;
    border-radius: 12px;
    padding: 20px;
    box-shadow: 0 1px 3px rgba(15, 23, 42, 0.05);
}

.chart-title {
    font-size: 18px;
    font-weight: 700;
    color: @slate_900;
}

.chart-subtitle {
    font-size: 13px;
    color: @slate_600;
}

.chart-legend-box {
    margin-top: 4px;
}

.chart-legend-item {
    font-size: 12px;
    font-weight: 500;
    color: @slate_600;
}

.chart-legend-dot {
    min-width: 10px;
    min-height: 10px;
    border-radius: 3px;
}

.chart-legend-dot--revenue {
    background-color: @chart_revenue;
}

.chart-legend-dot--profit {
    background-color: @chart_profit;
}

/* Thẻ cơ cấu doanh thu theo hãng */
.brand-dist-card {
    background-color: @white;
    border: 1px solid @slate_200;
    border-radius: 12px;
    padding: 20px;
    box-shadow: 0 1px 3px rgba(15, 23, 42, 0.05);
}

.brand-row {
    padding: 8px 0;
    border-bottom: 1px solid @slate_100;
}

.brand-row:last-child {
    border-bottom: none;
}

.brand-badge {
    padding: 3px 8px;
    border-radius: 6px;
    font-size: 12px;
    font-weight: 600;
}

.brand-pct-badge {
    background-color: @slate_100;
    color: @slate_900;
    padding: 2px 8px;
    border-radius: 999px;
    font-size: 12px;
    font-weight: 700;
}

/* Order Pipeline Segmented Status Bar */
.order-pipeline-card {
    background-color: @white;
    border: 1px solid @slate_200;
    border-radius: 12px;
    padding: 16px 20px;
    box-shadow: 0 1px 3px rgba(15, 23, 42, 0.05);
}

.order-pipeline-title {
    font-size: 15px;
    font-weight: 700;
    color: @slate_900;
}

.pipeline-legend-pill {
    padding: 4px 10px;
    border-radius: 999px;
    font-size: 12px;
    font-weight: 600;
}

.pipeline-legend-pill--completed {
    background-color: @green_100;
    color: @green_700;
}

.pipeline-legend-pill--shipping {
    background-color: @blue_100;
    color: @blue_700;
}

.pipeline-legend-pill--pending {
    background-color: #FEF3C7;
    color: @amber_600;
}

.pipeline-legend-pill--cancelled {
    background-color: @red_100;
    color: @red_600;
}
)CSS";

inline void applyAppStyles() {
  GtkCssProvider *provider = gtk_css_provider_new();
  char *contents = nullptr;
  gsize length = 0;
  GError *err = nullptr;
  if (g_file_get_contents("UI/styles/theme.css", &contents, &length, &err)) {
    const char *css = contents;
    if (length >= 3 && (unsigned char)contents[0] == 0xEF &&
        (unsigned char)contents[1] == 0xBB && (unsigned char)contents[2] == 0xBF) {
      css += 3;
    }
    gtk_css_provider_load_from_string(provider, css);
    g_free(contents);
  } else {
    if (err) g_error_free(err);
    gtk_css_provider_load_from_string(provider, APP_CSS_STYLES);
  }

  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

#endif // APP_CSS_H
