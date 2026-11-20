#ifndef PRODUCT_VIEWMODEL_H
#define PRODUCT_VIEWMODEL_H

#include <string>
#include <vector>
#include "../../source/utils/AppClock.h"
#include "../../source/utils/DateTime.h"

/**
 * @file ProductViewModel.h
 * @brief ĐẶC TẢ DỮ LIỆU HIỂN THỊ SẢN PHẨM & TỒN KHO (PRODUCT & INVENTORY
 * VIEWMODELS)
 *
 * =========================================================================================
 * - StockDisplayStatus: Phân loại trạng thái còn hàng, sắp hết (<=5) và hết
 * hàng.
 * - ProductItemViewModel: Đóng gói đầy đủ thông số cấu hình, giá nhập, giá bán,
 * tồn kho thực, tồn giữ chỗ (reserved), tồn khả dụng (available) và đường dẫn
 * ảnh sản phẩm.
 * =========================================================================================
 */

// Trạng thái tồn kho phục vụ hiển thị
enum class StockDisplayStatus {
  IN_STOCK,    // Còn hàng
  LOW_STOCK,   // Sắp hết hàng (<= 5)
  OUT_OF_STOCK // Hết hàng
};

// Mục thông tin ảnh trong thư viện ảnh sản phẩm
struct ProductImageItem {
  std::string path;
  bool isPrimary;
  int displayOrder;

  ProductImageItem() : isPrimary(false), displayOrder(1) {}
  ProductImageItem(std::string p, bool prim = false, int ord = 1)
      : path(std::move(p)), isPrimary(prim), displayOrder(ord) {}
};

// ViewModel hiển thị một dòng / thẻ sản phẩm điện thoại
struct ProductItemViewModel {
  std::string id;
  std::string name;
  std::string brand;
  std::string os;
  double importPrice;
  double sellingPrice;
  int actualStock;    // Tồn kho thực tế
  int reservedStock;  // Số lượng đã giữ chỗ cho đơn đang xử lý
  int availableStock; // Số lượng có thể bán = actual - reserved
  std::string manufacturer;
  std::string specs;
  std::string imageUrl;
  StockDisplayStatus status;

  // Marketing & Sale (B2 & B3 - ANTI.md)
  double listPrice;                // Giá niêm yết gốc (0 nếu không có sale)
  std::string saleStart;           // Ngày bắt đầu sale (dd/MM/yyyy)
  std::string saleEnd;             // Ngày kết thúc sale (dd/MM/yyyy)
  bool installmentEnabled;         // Có hỗ trợ trả góp 0% không
  std::string installmentNote;     // Ghi chú trả góp
  std::vector<std::string> colors; // Danh sách màu sắc thực tế
  int warrantyMonths;              // Thời hạn bảo hành chính hãng (tháng)
  int returnDays;                  // Thời hạn đổi trả (ngày)

  // Phiên bản sản phẩm (ANTY.md mục 1)
  std::string baseModel; // Dòng máy gốc (vd: "iPhone 15 Pro Max")
  std::string color;     // Màu sắc cụ thể của Phone này
  std::string capacity;  // Dung lượng cụ thể của Phone này
  std::vector<std::string> availableCapacities; // Danh sách dung lượng hỗ trợ

  // Mô tả chi tiết & Điểm nổi bật
  std::string shortHighlights;
  std::string description;

  // Bảng thông số chi tiết (cặp Nhãn - Giá trị)
  std::vector<std::pair<std::string, std::string>> detailedSpecs;

  // Thư viện ảnh đầy đủ của sản phẩm
  std::vector<ProductImageItem> images;

  ProductItemViewModel()
      : importPrice(0.0), sellingPrice(0.0), actualStock(0), reservedStock(0),
        availableStock(0), status(StockDisplayStatus::OUT_OF_STOCK),
        listPrice(0.0), installmentEnabled(false), warrantyMonths(12),
        returnDays(30) {}

  void calculateStockStatus() {
    availableStock = actualStock - reservedStock;
    if (availableStock <= 0) {
      status = StockDisplayStatus::OUT_OF_STOCK;
      if (availableStock < 0)
        availableStock = 0;
    } else if (availableStock <= 5) {
      status = StockDisplayStatus::LOW_STOCK;
    } else {
      status = StockDisplayStatus::IN_STOCK;
    }
  }

  // Kiểm tra có đang áp dụng chương trình giảm giá hợp lệ theo đồng hồ hệ thống không
  bool hasActiveSale(const DateTime& now = AppClock::now()) const {
    if (!(listPrice > sellingPrice && sellingPrice > 0)) {
      return false;
    }
    bool hasStart = !saleStart.empty();
    bool hasEnd = !saleEnd.empty();
    DateTime startDt(0, 0, 0, 0, 0, 0);
    DateTime endDt(0, 0, 0, 0, 0, 0);
    if (hasStart) {
      if (!DateTime::tryParseDate(saleStart, startDt)) {
        return false;
      }
      startDt.hour = 0;
      startDt.minute = 0;
      startDt.second = 0;
      if (now < startDt) {
        return false;
      }
    }
    if (hasEnd) {
      if (!DateTime::tryParseDate(saleEnd, endDt)) {
        return false;
      }
      endDt.hour = 23;
      endDt.minute = 59;
      endDt.second = 59;
      if (now > endDt) {
        return false;
      }
    }
    if (hasStart && hasEnd && endDt < startDt) {
      return false;
    }
    return true;
  }

  // Tính % giảm giá thực tế = round((listPrice - sellingPrice) * 100 /
  // listPrice)
  int calculateDiscountPercent() const {
    if (listPrice > sellingPrice && listPrice > 0) {
      return static_cast<int>((listPrice - sellingPrice) * 100.0 / listPrice +
                              0.5);
    }
    return 0;
  }
};

// Request lọc/tìm kiếm sản phẩm
struct ProductFilterRequest {
  std::string keyword;
  std::string brand;
  double minPrice;
  double maxPrice;
  std::string sortBy; // "price_asc", "price_desc", "name_asc", "stock_desc"
  bool onlyInStock;

  ProductFilterRequest()
      : minPrice(0.0), maxPrice(0.0), sortBy("name_asc"), onlyInStock(false) {}
};

#endif // PRODUCT_VIEWMODEL_H
