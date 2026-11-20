#ifndef PRODUCT_DETAIL_VIEW_H
#define PRODUCT_DETAIL_VIEW_H

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <functional>
#include "../viewmodels/ProductViewModel.h"

class AppController;
class MainShellWindow;

/**
 * @brief Chế độ hiển thị của màn hình chi tiết sản phẩm
 */
enum class ProductDetailMode {
  Customer,       // Khách xem thông tin, chọn mua, thêm giỏ, gửi đánh giá
  SalesPos,       // Nhân viên Sale xem chi tiết, chọn tổ hợp & 1 nút duy nhất "🛒 Thêm vào giỏ"
  AdminEdit,      // Admin chỉnh sửa trực quan tại chỗ mọi trường thông tin
  AdminPreview    // Admin xem trước với giao diện khách (nút mua bị vô hiệu hóa)
};

/**
 * @class ProductDetailView
 * @brief Thành phần dùng chung hiển thị và chỉnh sửa trực quan trang chi tiết sản phẩm (ANTI.md)
 */
class ProductDetailView {
public:
  struct Callbacks {
    std::function<void()> onSaved;
    std::function<void()> onDeleted;
    std::function<void()> onCancelled;
    std::function<void(const std::string &brand)> onBrandSelected;
    std::function<void(const std::string &productId)> onProductSelected;
    std::function<void()> onNavigateHome;
    std::function<void(const ProductItemViewModel &variant, int quantity)>
        onAddToPosCart;
  };

  /**
   * @brief Tạo widget trang chi tiết sản phẩm
   * @param controller Pointer tới AppController
   * @param shell Pointer tới MainShellWindow
   * @param product Dữ liệu sản phẩm (ProductItemViewModel)
   * @param mode Chế độ hiển thị ban đầu
   * @param isNew True nếu đang tạo mới sản phẩm
   * @param callbacks Tập callback cho các hành động Lưu, Hủy, Xóa, Điều hướng
   * @return GtkWidget* cuộn toàn trang (GtkScrolledWindow)
   */
  static GtkWidget *create(AppController *controller,
                           MainShellWindow *shell,
                           const ProductItemViewModel &product,
                           ProductDetailMode mode,
                           bool isNew = false,
                           Callbacks callbacks = {});
};

#endif // PRODUCT_DETAIL_VIEW_H

