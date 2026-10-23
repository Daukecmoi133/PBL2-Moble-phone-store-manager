#ifndef PRODUCT_VIEWMODEL_H
#define PRODUCT_VIEWMODEL_H

#include <string>
#include <vector>

/**
 * @file ProductViewModel.h
 * @brief ĐẶC TẢ DỮ LIỆU HIỂN THỊ SẢN PHẨM & TỒN KHO (PRODUCT & INVENTORY VIEWMODELS)
 * 
 * =========================================================================================
 * - StockDisplayStatus: Phân loại trạng thái còn hàng, sắp hết (<=5) và hết hàng.
 * - ProductItemViewModel: Đóng gói đầy đủ thông số cấu hình, giá nhập, giá bán, tồn kho thực,
 *   tồn giữ chỗ (reserved), tồn khả dụng (available) và đường dẫn ảnh sản phẩm.
 * =========================================================================================
 */

// Trạng thái tồn kho phục vụ hiển thị
enum class StockDisplayStatus {
    IN_STOCK,       // Còn hàng
    LOW_STOCK,      // Sắp hết hàng (<= 5)
    OUT_OF_STOCK    // Hết hàng
};

// ViewModel hiển thị một dòng / thẻ sản phẩm điện thoại
struct ProductItemViewModel {
    std::string id;
    std::string name;
    std::string brand;
    std::string os;
    double importPrice;
    double sellingPrice;
    int actualStock;        // Tồn kho thực tế
    int reservedStock;      // Số lượng đã giữ chỗ cho đơn đang xử lý
    int availableStock;     // Số lượng có thể bán = actual - reserved
    std::string manufacturer;
    std::string specs;
    std::string imageUrl;
    StockDisplayStatus status;

    ProductItemViewModel()
        : importPrice(0.0), sellingPrice(0.0), actualStock(0),
          reservedStock(0), availableStock(0), status(StockDisplayStatus::OUT_OF_STOCK) {}

    void calculateStockStatus() {
        availableStock = actualStock - reservedStock;
        if (availableStock <= 0) {
            status = StockDisplayStatus::OUT_OF_STOCK;
            if (availableStock < 0) availableStock = 0;
        } else if (availableStock <= 5) {
            status = StockDisplayStatus::LOW_STOCK;
        } else {
            status = StockDisplayStatus::IN_STOCK;
        }
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
