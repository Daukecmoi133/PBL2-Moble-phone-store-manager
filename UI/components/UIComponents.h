#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include <gtk/gtk.h>
#include <string>
#include <functional>
#include "../../source/models/Common.h"
#include "../viewmodels/OrderViewModel.h"
#include "../viewmodels/ProductViewModel.h"
#include "../viewmodels/WarrantyViewModel.h"
#include "../viewmodels/StatisticsViewModel.h"

/**
 * @file UIComponents.h
 * @brief ĐẶC TẢ HỆ THỐNG THÀNH PHẦN GIAO DIỆN DÙNG CHUNG (REUSABLE UI ATOMS & PATTERNS)
 * 
 * =========================================================================================
 * 1. MỤC ĐÍCH & NGUYÊN TẮC THIẾT KẾ:
 *    - Tập hợp các tiện ích tạo Widget chuẩn hóa, bảo đảm tính nhất quán giao diện toàn hệ thống:
 *      + Thẻ thống kê (Stat Cards) dùng cho Dashboard.
 *      + Huy hiệu trạng thái (Status Badges) cho Đơn hàng, Kho, Thanh toán, Bảo hành.
 *      + Huy hiệu xếp hạng khách hàng (Rank Badges) với màu sắc đặc tả riêng biệt (Bronze, Silver, Gold, Diamond).
 *      + Khung trạng thái rỗng / lỗi / tải dữ liệu (Empty/Error/Loading State) thân thiện.
 *      + Hộp thoại cảnh báo & xác nhận (Modal Dialogs) không gây crash hoặc rò rỉ cửa sổ.
 *      + Tiêu đề trang (Section Header & Back Header hỗ trợ màn hình chi tiết riêng biệt).
 * 
 * 2. ĐẶC TẢ THẨM MỸ & TRUY CẬP:
 *    - Phông chữ to, độ tương phản cao cho người mắt yếu.
 *    - Hỗ trợ định dạng tiền tệ VND có dấu chấm phân cách hàng nghìn.
 * =========================================================================================
 */
class MainShellWindow;
class AppController;

namespace UIComponents {

// === 1. ĐỊNH DẠNG DỮ LIỆU & TIỀN TỆ (FORMATTERS) ===
/// @brief Định dạng số thực thành chuỗi tiền tệ Việt Nam Đồng (vd: "15.990.000 ₫")
std::string formatVND(double amount);
/// @brief Định dạng số nguyên có dấu phân cách hàng nghìn (vd: "1.250")
std::string formatNumber(long long number);

// === DIALOGS DÙNG CHUNG (GTK4 MODAL WINDOWS) ===
void showErrorDialog(GtkWindow* parent, const std::string& title, const std::string& message);
void showSuccessDialog(GtkWindow* parent, const std::string& title, const std::string& message);
void showWarningDialog(GtkWindow* parent, const std::string& title, const std::string& message);
void showInfoDialog(GtkWindow* parent, const std::string& title, const std::string& message);
void showServiceNotConnectedDialog(GtkWindow* parent, const std::string& featureName);

/// @brief Hộp thoại đổi mật khẩu cho người dùng đang đăng nhập (mật khẩu hiện tại, mới, xác nhận)
void showChangePasswordDialog(GtkWindow* parent, AppController* controller);

void registerShellWindow(MainShellWindow* shell);
MainShellWindow* getActiveShell();

/// @brief Tiện ích chuyển trang dạng stack in-window (ủy quyền qua MainShellWindow)
void pushPage(const std::string& id, GtkWidget* content, const std::string& title);
void popPage();
void clearPageStack();

/// @brief Tiện ích hiển thị / đóng hộp thoại dạng in-window overlay modal
void showOverlayDialog(GtkWidget* card);
void closeOverlayDialog();
bool isOverlayDialogOpen();

/// @brief Hiển thị thông báo Toast ngắn ở góc, tự động ẩn sau 2 đến 3 giây
void showToast(GtkWindow* parent, const std::string& message, const std::string& type = "success");

/// @brief Thiết lập con trỏ chuột sang dạng chờ (progress) hoặc trả về mặc định
void setBusyCursor(GtkWidget* widget, bool busy);

/// @brief Ghi lỗi kỹ thuật / ngoại lệ vào tệp log hệ thống
void logError(const std::string& module, const std::string& technicalDetail);

// Dialog xác nhận thao tác (Confirm Dialog)
typedef void (*ConfirmCallback)(gpointer data);
void showConfirmDialog(GtkWindow* parent, const std::string& title, const std::string& message,
                       ConfirmCallback onConfirm, gpointer userData);

// === TRẠNG THÁI HIỂN THỊ (EMPTY, LOADING, ERROR, SUCCESS) ===
GtkWidget* createEmptyState(const std::string& icon, const std::string& title,
                            const std::string& message, const std::string& actionText = "",
                            GCallback onAction = nullptr, gpointer actionData = nullptr);

GtkWidget* createLoadingState(const std::string& message = "Đang tải dữ liệu...");
GtkWidget* createErrorState(const std::string& message, GCallback onRetry = nullptr, gpointer retryData = nullptr);
GtkWidget* createSuccessBanner(const std::string& message);

// === TIỆN ÍCH GIAO DIỆN KHÁC ===
/// @brief Tạo widget ảnh SVG đơn sắc chuẩn (Lucide) từ thư mục UI/assets/icons/
GtkWidget* createSvgIcon(const std::string& iconName, int pixelSize = 16);

GtkWidget* createStatCard(const std::string& title, const std::string& value,
                          const std::string& subtitle = "", const std::string& icon = "📊",
                          const std::string& colorClass = "blue");

/// @brief Tạo thẻ thống kê KPI hỗ trợ bấm điều hướng và hiệu ứng hover/focus chuẩn DESIGN.md
GtkWidget* createClickableStatCard(const std::string& title, const std::string& value,
                                   const std::string& subtitle, const std::string& iconName,
                                   const std::string& colorClass, GCallback onClick = nullptr,
                                   gpointer data = nullptr);

GtkWidget* createStatusBadge(const std::string& text, const std::string& badgeClass);
GtkWidget* createOrderStatusBadge(OrderStatus status);
GtkWidget* createPaymentStatusBadge(PaymentStatus status);
GtkWidget* createStockStatusBadge(StockDisplayStatus status);
GtkWidget* createWarrantyStatusBadge(WarrantyStatus status);

GtkWidget* createRankBadge(const std::string& membershipTier);

// === TIỆN ÍCH BẢNG DỮ LIỆU CHUẨN DOANH NGHIỆP ===
struct TableColumnSpec {
    std::string title;
    int minWidth;      // -1 nếu tự co giãn / hexpand
    float alignment;   // 0.0f: căn trái, 1.0f: căn phải, 0.5f: căn giữa
    bool hexpand;      // true nếu cột giãn nở chiếm phần còn lại
};

/// @brief Tạo hàng tiêu đề bảng chuẩn (in đậm, nền xám nhạt, cố định chiều rộng)
GtkWidget* createTableHeaderRow(const std::vector<TableColumnSpec>& columns);

/// @brief Tạo khung bảng (.table-container) với thanh tiêu đề cố định đứng yên ở trên và vùng dữ liệu cuộn dọc độc lập bên dưới
GtkWidget* createScrollableTableContainer(const std::vector<TableColumnSpec>& columns,
                                          GtkWidget** outBodyListBox,
                                          int minBodyHeight = 360,
                                          int maxBodyHeight = -1);

/// @brief Tạo hàng dữ liệu bảng chuẩn (kèm phân biệt màu dòng chẵn/lẻ .row-even / .row-odd)
GtkWidget* createTableDataRow(size_t rowIndex);

/// @brief Áp dụng thông số cột (minWidth, alignment, hexpand) từ TableColumnSpec lên widget ô dữ liệu
void applyColumnSpec(GtkWidget* cellWidget, const TableColumnSpec& spec);

/// @brief Tạo ô văn bản / số liệu đơn dòng khớp kích thước và căn lề của TableColumnSpec
GtkWidget* createTableTextCell(const std::string& text, const TableColumnSpec& spec,
                               const std::string& cssClass = "");

/// @brief Tạo ô dữ liệu 2 dòng (Tiêu đề chính + Phụ đề mã/thông số) khớp kích thước TableColumnSpec
GtkWidget* createTableTwoLineCell(const std::string& primaryText, const std::string& secondaryText,
                                  const TableColumnSpec& spec);

/// @brief Bọc một widget con (Badge, nhóm nút thao tác...) vào khung cột cố định chiều rộng theo TableColumnSpec
GtkWidget* createTableWidgetCell(GtkWidget* innerWidget, const TableColumnSpec& spec);

/// @brief Tạo nút thao tác nhỏ gọn ở cuối bảng kèm Tooltip giải thích hành động
GtkWidget* createTableActionButton(const std::string& label, const std::string& tooltip,
                                   const std::string& btnClass, GCallback onClick = nullptr, gpointer data = nullptr);

GtkWidget* createSearchBar(const std::string& placeholder, GCallback onSearchChanged,
                           gpointer data, GtkWidget** outEntry = nullptr);

/// @brief Tạo thanh công cụ lọc chuẩn gồm ô tìm kiếm, vùng điều khiển phụ (nếu có) và huy hiệu đếm số bản ghi
GtkWidget* createFilterToolbar(const std::string& searchPlaceholder, GCallback onSearchChanged,
                               gpointer data, GtkWidget** outEntry = nullptr,
                               GtkWidget** outCountBadge = nullptr, GtkWidget* extraControlsBox = nullptr);

/// @brief Cập nhật văn bản cho huy hiệu đếm số bản ghi trên thanh công cụ lọc
void updateRecordCountBadge(GtkWidget* countBadgeLabel, size_t shownCount,
                            const std::string& unitName = "bản ghi");

/// @brief Tạo trường nhập liệu chuẩn: Nhãn ở trên, căn trái, dấu * màu đỏ nếu bắt buộc, hỗ trợ nhãn lỗi bên dưới
GtkWidget* createFormField(const std::string& labelText, GtkWidget* inputWidget, GtkWidget** outErrorLabel = nullptr);

/// @brief Bọc ô nhập mật khẩu cùng nút biểu tượng hiện mật khẩu khi rê chuột
GtkWidget* createPasswordField(GtkWidget* entryWidget);

/// @brief Hiển thị thông báo lỗi màu đỏ ngay dưới ô nhập và thêm viền đỏ
void setFieldError(GtkWidget* errorLabel, GtkWidget* inputWidget, const std::string& errorMessage);

/// @brief Xóa thông báo lỗi và viền đỏ của ô nhập
void clearFieldError(GtkWidget* errorLabel, GtkWidget* inputWidget);

/// @brief Thiết lập ô nhập số tiền: chỉ nhận số và tự định dạng dấu chấm phân cách hàng nghìn khi rời ô
void setupCurrencyInput(GtkWidget* entryWidget);

/// @brief Đọc giá trị tiền từ ô nhập (tự động loại bỏ dấu chấm/phẩy/khoảng trắng)
double parseCurrencyInput(GtkWidget* entryWidget);

/// @brief Thiết lập ô nhập số điện thoại: chỉ nhận chữ số, giới hạn 10 ký tự
void setupPhoneInput(GtkWidget* entryWidget);

/// @brief Kiểm tra số điện thoại hợp lệ (10 chữ số, bắt đầu bằng 0)
bool validatePhoneNumber(const std::string& phone, std::string& outErrorMsg);

/// @brief Thiết lập phím tắt modal: Enter kích hoạt nút Lưu, Esc kích hoạt nút Hủy
void setupDialogShortcuts(GtkWidget* window, GtkWidget* btnSave, GtkWidget* btnCancel);

GtkWidget* createSectionHeader(const std::string& title, const std::string& subtitle = "",
                               GtkWidget* actionWidget = nullptr);

GtkWidget* createBackHeader(const std::string& title, const std::string& subtitle,
                            GCallback onBack, gpointer data, GtkWidget* actionWidget = nullptr);

/// @brief Thiết lập con trỏ chuột dạng bàn tay (pointer cursor) cho các nút bấm GTK4
void setButtonCursor(GtkWidget* btn);

// === TIỆN ÍCH BIỂU ĐỒ & THỐNG KÊ (DESIGN.MD SECTION 8) ===
/// @brief Định dạng số tiền rút gọn cho biểu đồ & nhãn gọn (vd: "15.2 tr", "500 k", "2.5 tỷ")
std::string formatCompactVND(double amount);

/// @brief Tạo widget Biểu đồ Cột đôi Doanh thu & Bảng số liệu chi tiết (kèm nút Trái/Phải đổi chu kỳ Ngày-Tháng-Năm)
GtkWidget* createRevenueBarChart(const std::vector<TimelineStatPoint>& points,
                                 int timelineMode = 0,
                                 std::function<void(int)> onModeChanged = nullptr);

/// @brief Tạo widget Thẻ Cơ cấu Doanh thu theo Thương hiệu / Nguyên liệu dạng Biểu đồ Tròn (Pie/Donut Chart)
GtkWidget* createBrandDistributionCard(const std::vector<BrandShareStatItem>& brands);

/// @brief Tạo thanh tỷ lệ trạng thái đơn hàng (Order Pipeline Segmented Bar) cho Admin Dashboard chuẩn DESIGN.md T17
GtkWidget* createOrderStatusPipelineBar(int completed, int shipping, int pending, int cancelled);

} // namespace UIComponents

#endif // UI_COMPONENTS_H
