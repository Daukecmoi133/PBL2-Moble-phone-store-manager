#include "ProductDetailView.h"
#include "../../source/controllers/AppController.h"
#include "../../source/utils/PathHelper.h"
#include "../../source/utils/StringHelper.h"
#include "../components/UIComponents.h"
#include "../screens/MainShellWindow.h"
#include "../styles/AppCss.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

namespace {

struct SpecRowWidgets {
  GtkWidget *entryKey;
  GtkWidget *entryVal;
  GtkWidget *rowBox;
};

struct ProductDetailState {
  AppController *controller;
  MainShellWindow *shell;
  ProductDetailMode mode;
  bool isNew;
  ProductDetailView::Callbacks callbacks;

  ProductItemViewModel originalProduct;
  ProductItemViewModel currentProduct;

  bool hasUnsavedChanges = false;
  size_t currentImageIndex = 0;
  int selectedQty = 1;

  // Container widgets
  GtkWidget *scrollWindow = nullptr;
  GtkWidget *contentBox = nullptr;

  // Widgets for live updating
  GtkWidget *imgStage = nullptr;
  GtkWidget *imgCenterBox = nullptr;
  GtkWidget *imgLarge = nullptr;
  GtkWidget *imgPlaceholderBox = nullptr;
  GtkWidget *lblStageCaption = nullptr;
  GtkWidget *thumbStrip = nullptr;
  std::vector<GtkWidget *> thumbButtons;

  GtkWidget *lblCrumbName = nullptr;
  GtkWidget *lblTitle = nullptr;
  GtkWidget *lblSku = nullptr;
  GtkWidget *lblStockBadge = nullptr;
  GtkWidget *lblSubRating = nullptr;
  GtkWidget *lblPrice = nullptr;
  GtkWidget *lblOldPrice = nullptr;
  GtkWidget *lblDiscBadge = nullptr;
  GtkWidget *lblInstallment = nullptr;
  GtkWidget *colorPreviewBox = nullptr;
  GtkWidget *specsListBox = nullptr;
  GtkWidget *unsavedIndicator = nullptr;

  // Variants & Options (ANTY.md mục 1)
  std::vector<ProductItemViewModel> variants;
  std::string selectedCapacity;
  std::string selectedColor;
  GtkWidget *lblStockHint = nullptr;
  std::vector<GtkWidget *> capacityChipButtons;
  std::vector<GtkWidget *> colorChipButtons;

  // Quantity widgets (Customer mode)
  GtkWidget *lblQty = nullptr;
  GtkWidget *btnMinus = nullptr;
  GtkWidget *btnPlus = nullptr;

  // Admin edit controls
  GtkWidget *entryName = nullptr;
  GtkWidget *entryBrand = nullptr;
  GtkWidget *entryOs = nullptr;
  GtkWidget *entryManufacturer = nullptr;
  GtkWidget *entrySellingPrice = nullptr;
  GtkWidget *entryListPrice = nullptr;
  GtkWidget *entrySaleStart = nullptr;
  GtkWidget *entrySaleEnd = nullptr;
  GtkWidget *chkInstallment = nullptr;
  GtkWidget *entryInstallmentNote = nullptr;
  GtkWidget *entryColors = nullptr;
  GtkWidget *entryImportPrice = nullptr;
  GtkWidget *entryActualStock = nullptr;
  GtkWidget *entryWarranty = nullptr;
  GtkWidget *entryReturnDays = nullptr;
  GtkWidget *entryShortHighlights = nullptr;
  GtkWidget *textDescription = nullptr;
  GtkWidget *lblCharCount = nullptr;
  GtkWidget *entryNewImgPath = nullptr;

  std::vector<SpecRowWidgets> specRows;

  // Admin KPI labels
  GtkWidget *lblAdminProfit = nullptr;
  GtkWidget *lblAdminMargin = nullptr;
  GtkWidget *lblAdminAvail = nullptr;

  // Error labels
  GtkWidget *errName = nullptr;
  GtkWidget *errBrand = nullptr;
  GtkWidget *errSellingPrice = nullptr;
  GtkWidget *errListPrice = nullptr;
  GtkWidget *errImportPrice = nullptr;
  GtkWidget *errStock = nullptr;
  GtkWidget *errWarranty = nullptr;
  GtkWidget *errReturnDays = nullptr;
};

// Forward declaration
void renderView(ProductDetailState *state);

// Đồng bộ dữ liệu từ các ô nhập liệu vào currentProduct
void syncFromInputs(ProductDetailState *state) {
  if (state->mode != ProductDetailMode::AdminEdit)
    return;

  if (state->entryName) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entryName));
    state->currentProduct.name = t ? StringHelper::trim(t) : "";
  }
  if (state->entryBrand) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entryBrand));
    state->currentProduct.brand = t ? StringHelper::trim(t) : "";
  }
  if (state->entryOs) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entryOs));
    state->currentProduct.os = t ? StringHelper::trim(t) : "";
  }
  if (state->entryManufacturer) {
    const char *t =
        gtk_editable_get_text(GTK_EDITABLE(state->entryManufacturer));
    state->currentProduct.manufacturer = t ? StringHelper::trim(t) : "";
  }
  if (state->entrySellingPrice) {
    state->currentProduct.sellingPrice =
        UIComponents::parseCurrencyInput(state->entrySellingPrice);
  }
  if (state->entryListPrice) {
    state->currentProduct.listPrice =
        UIComponents::parseCurrencyInput(state->entryListPrice);
  }
  if (state->entrySaleStart) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entrySaleStart));
    state->currentProduct.saleStart = t ? StringHelper::trim(t) : "";
  }
  if (state->entrySaleEnd) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entrySaleEnd));
    state->currentProduct.saleEnd = t ? StringHelper::trim(t) : "";
  }
  if (state->chkInstallment) {
    state->currentProduct.installmentEnabled =
        gtk_check_button_get_active(GTK_CHECK_BUTTON(state->chkInstallment));
  }
  if (state->entryInstallmentNote) {
    const char *t =
        gtk_editable_get_text(GTK_EDITABLE(state->entryInstallmentNote));
    state->currentProduct.installmentNote = t ? StringHelper::trim(t) : "";
  }
  if (state->entryColors) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entryColors));
    std::string colorsRaw = t ? t : "";
    state->currentProduct.colors.clear();
    std::stringstream ss(colorsRaw);
    std::string cToken;
    while (std::getline(ss, cToken, ',')) {
      std::string cTrim = StringHelper::trim(cToken);
      if (!cTrim.empty()) {
        state->currentProduct.colors.push_back(cTrim);
      }
    }
  }
  if (state->entryImportPrice) {
    state->currentProduct.importPrice =
        UIComponents::parseCurrencyInput(state->entryImportPrice);
  }
  if (state->entryActualStock) {
    const char *t =
        gtk_editable_get_text(GTK_EDITABLE(state->entryActualStock));
    try {
      state->currentProduct.actualStock = std::max(0, std::stoi(t ? t : "0"));
    } catch (...) {
      state->currentProduct.actualStock = 0;
    }
    state->currentProduct.calculateStockStatus();
  }
  if (state->entryWarranty) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entryWarranty));
    try {
      state->currentProduct.warrantyMonths =
          std::max(0, std::stoi(t ? t : "12"));
    } catch (...) {
      state->currentProduct.warrantyMonths = 12;
    }
  }
  if (state->entryReturnDays) {
    const char *t = gtk_editable_get_text(GTK_EDITABLE(state->entryReturnDays));
    try {
      state->currentProduct.returnDays = std::max(0, std::stoi(t ? t : "30"));
    } catch (...) {
      state->currentProduct.returnDays = 30;
    }
  }
  if (state->entryShortHighlights) {
    const char *t =
        gtk_editable_get_text(GTK_EDITABLE(state->entryShortHighlights));
    state->currentProduct.shortHighlights = t ? StringHelper::trim(t) : "";
  }
  if (state->textDescription) {
    GtkTextBuffer *buf =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->textDescription));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);
    char *text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
    state->currentProduct.description = text ? text : "";
    g_free(text);
  }

  // Thu thập danh sách thông số kỹ thuật chi tiết
  state->currentProduct.detailedSpecs.clear();
  std::string summarySpecs = "";
  for (const auto &row : state->specRows) {
    const char *k = gtk_editable_get_text(GTK_EDITABLE(row.entryKey));
    const char *v = gtk_editable_get_text(GTK_EDITABLE(row.entryVal));
    std::string keyTrim = k ? StringHelper::trim(k) : "";
    std::string valTrim = v ? StringHelper::trim(v) : "";
    if (!keyTrim.empty() && !valTrim.empty()) {
      state->currentProduct.detailedSpecs.push_back({keyTrim, valTrim});
      if (!summarySpecs.empty())
        summarySpecs += ", ";
      summarySpecs += valTrim;
    }
  }
  if (!summarySpecs.empty()) {
    state->currentProduct.specs = summarySpecs;
  }
}

// Cập nhật tính toán trực tiếp % giảm giá và các KPI Admin
void recalculateLiveBadgesAndKPIs(ProductDetailState *state) {
  if (!state || state->mode != ProductDetailMode::AdminEdit)
    return;

  double selling = UIComponents::parseCurrencyInput(state->entrySellingPrice);
  double list = UIComponents::parseCurrencyInput(state->entryListPrice);
  double importP = UIComponents::parseCurrencyInput(state->entryImportPrice);

  // Tính % giảm giá thực tế (chỉ tính từ hai giá thật)
  if (list > selling && selling > 0) {
    int pct = static_cast<int>(std::round((list - selling) * 100.0 / list));
    if (state->lblDiscBadge) {
      gtk_label_set_text(GTK_LABEL(state->lblDiscBadge),
                         ("-" + std::to_string(pct) + "%").c_str());
      gtk_widget_add_css_class(state->lblDiscBadge, "pdp-badge-discount");
      gtk_widget_set_visible(state->lblDiscBadge, TRUE);
    }
    if (state->lblOldPrice) {
      gtk_label_set_text(GTK_LABEL(state->lblOldPrice),
                         UIComponents::formatVND(list).c_str());
      gtk_widget_set_visible(state->lblOldPrice, TRUE);
    }
  } else {
    if (state->lblDiscBadge) {
      gtk_label_set_text(GTK_LABEL(state->lblDiscBadge), "(Không có giảm giá)");
      gtk_widget_remove_css_class(state->lblDiscBadge, "pdp-badge-discount");
      gtk_widget_set_visible(state->lblDiscBadge, TRUE);
    }
    if (state->lblOldPrice) {
      gtk_widget_set_visible(state->lblOldPrice, FALSE);
    }
  }

  // Tính lợi nhuận & biên lợi nhuận
  double profit = selling - importP;
  double margin = (selling > 0) ? (profit * 100.0 / selling) : 0.0;
  if (state->lblAdminProfit) {
    gtk_label_set_text(GTK_LABEL(state->lblAdminProfit),
                       UIComponents::formatVND(profit).c_str());
  }
  if (state->lblAdminMargin) {
    char mBuf[32];
    std::snprintf(mBuf, sizeof(mBuf), "%.1f%%", margin);
    gtk_label_set_text(GTK_LABEL(state->lblAdminMargin), mBuf);
  }

  // Tính tồn kho khả dụng
  if (state->lblAdminAvail && state->entryActualStock) {
    const char *stk =
        gtk_editable_get_text(GTK_EDITABLE(state->entryActualStock));
    int actual = 0;
    try {
      actual = std::stoi(stk ? stk : "0");
    } catch (...) {
    }
    int avail = std::max(0, actual - state->currentProduct.reservedStock);
    gtk_label_set_text(GTK_LABEL(state->lblAdminAvail),
                       (std::to_string(avail) + " máy").c_str());
  }

  // Đánh dấu có thay đổi chưa lưu
  state->hasUnsavedChanges = true;
  if (state->unsavedIndicator) {
    gtk_widget_set_visible(state->unsavedIndicator, TRUE);
  }
}

// Cập nhật vùng hiển thị ảnh lớn và dải thumbnail
void updateImageStage(ProductDetailState *state) {
  if (!state || !state->imgCenterBox)
    return;

  // Xóa ảnh / icon cũ trong imgCenterBox
  GtkWidget *child = gtk_widget_get_first_child(state->imgCenterBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(state->imgCenterBox), child);
    child = next;
  }

  std::string activePath = "";
  if (!state->currentProduct.images.empty()) {
    if (state->currentImageIndex >= state->currentProduct.images.size()) {
      state->currentImageIndex = 0;
    }
    activePath = state->currentProduct.images[state->currentImageIndex].path;
  } else if (!state->currentProduct.imageUrl.empty()) {
    activePath = state->currentProduct.imageUrl;
  }

  std::string resolved = PathHelper::resolveDataPath(activePath);
  bool valid = (!activePath.empty() &&
                g_file_test(resolved.c_str(), G_FILE_TEST_EXISTS));

  if (valid) {
    GtkWidget *img = gtk_image_new_from_file(resolved.c_str());
    gtk_image_set_pixel_size(GTK_IMAGE(img), 240);
    gtk_widget_set_halign(img, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(img, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(state->imgCenterBox), img);
  } else {
    GtkWidget *icon = gtk_label_new("📱");
    gtk_widget_add_css_class(icon, "stat-value");
    gtk_box_append(GTK_BOX(state->imgCenterBox), icon);

    GtkWidget *lblNoImg = gtk_label_new("Chưa có ảnh sản phẩm");
    gtk_widget_add_css_class(lblNoImg, "page-subtitle");
    gtk_box_append(GTK_BOX(state->imgCenterBox), lblNoImg);
  }

  // Cập nhật dải thumbnail
  if (state->thumbStrip) {
    GtkWidget *tChild = gtk_widget_get_first_child(state->thumbStrip);
    while (tChild != nullptr) {
      GtkWidget *next = gtk_widget_get_next_sibling(tChild);
      gtk_box_remove(GTK_BOX(state->thumbStrip), tChild);
      tChild = next;
    }
    state->thumbButtons.clear();

    size_t count = state->currentProduct.images.size();
    if (count > 1 || state->mode == ProductDetailMode::AdminEdit) {
      gtk_widget_set_visible(state->thumbStrip, TRUE);
      for (size_t i = 0; i < count; ++i) {
        const auto &imgInfo = state->currentProduct.images[i];
        GtkWidget *tBtn = gtk_button_new();
        gtk_widget_add_css_class(tBtn, "pdp-thumb-btn");
        if (i == state->currentImageIndex) {
          gtk_widget_add_css_class(tBtn, "pdp-thumb-active");
        }
        UIComponents::setButtonCursor(tBtn);

        GtkWidget *tBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_halign(tBox, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(tBox, GTK_ALIGN_CENTER);

        std::string resThumb = PathHelper::resolveDataPath(imgInfo.path);
        if (g_file_test(resThumb.c_str(), G_FILE_TEST_EXISTS)) {
          GtkWidget *mini = gtk_image_new_from_file(resThumb.c_str());
          gtk_image_set_pixel_size(GTK_IMAGE(mini), 36);
          gtk_box_append(GTK_BOX(tBox), mini);
        } else {
          GtkWidget *tIcon = gtk_label_new("📱");
          gtk_box_append(GTK_BOX(tBox), tIcon);
        }

        std::string labelText = "Ảnh " + std::to_string(i + 1);
        if (imgInfo.isPrimary)
          labelText = "⭐ Chính";
        GtkWidget *tLbl = gtk_label_new(labelText.c_str());
        gtk_widget_add_css_class(tLbl, "shortcut-hint");
        gtk_box_append(GTK_BOX(tBox), tLbl);
        gtk_button_set_child(GTK_BUTTON(tBtn), tBox);

        state->thumbButtons.push_back(tBtn);

        struct ThumbClickData {
          ProductDetailState *s;
          size_t idx;
        };
        ThumbClickData *tcd = new ThumbClickData{state, i};
        g_signal_connect_data(
            tBtn, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
              ThumbClickData *data = static_cast<ThumbClickData *>(d);
              data->s->currentImageIndex = data->idx;
              updateImageStage(data->s);
            }),
            tcd,
            [](gpointer d, GClosure *) {
              delete static_cast<ThumbClickData *>(d);
            },
            (GConnectFlags)0);

        gtk_box_append(GTK_BOX(state->thumbStrip), tBtn);
      }
    } else {
      gtk_widget_set_visible(state->thumbStrip, FALSE);
    }
  }
}

// Cập nhật nhãn tổng quan sao và đánh giá
void updateRatingSummary(ProductDetailState *state) {
  if (!state || !state->lblSubRating)
    return;

  std::vector<ProductReviewViewModel> revs =
      state->controller->getProductReviews(state->currentProduct.id);
  std::string ratingSummaryText;
  if (!revs.empty()) {
    double totalStars = 0.0;
    for (const auto &r : revs)
      totalStars += r.ratingStars;
    double avg = totalStars / revs.size();
    char avgBuf[32];
    std::snprintf(avgBuf, sizeof(avgBuf), "%.1f", avg);
    ratingSummaryText = "⭐ " + std::string(avgBuf) + "/5.0 (" +
                        std::to_string(revs.size()) + " đánh giá)";
  } else {
    ratingSummaryText = "⭐ Chưa có đánh giá";
  }

  if (!state->currentProduct.manufacturer.empty()) {
    ratingSummaryText +=
        "  •  Phân phối chính hãng bởi " + state->currentProduct.manufacturer;
  }
  gtk_label_set_text(GTK_LABEL(state->lblSubRating), ratingSummaryText.c_str());
}

// Cập nhật giao diện khi người dùng đổi Dung lượng hoặc Màu sắc (ANTY.md mục 1)
void updateVariantSelection(ProductDetailState *state) {
  if (!state)
    return;
  ProductItemViewModel chosen;
  bool foundExact = false;
  for (const auto &v : state->variants) {
    if (v.capacity == state->selectedCapacity &&
        v.color == state->selectedColor) {
      chosen = v;
      foundExact = true;
      break;
    }
  }
  if (!foundExact) {
    for (const auto &v : state->variants) {
      if (v.capacity == state->selectedCapacity) {
        chosen = v;
        state->selectedColor = v.color;
        break;
      }
    }
  }
  if (chosen.id.empty()) {
    for (const auto &v : state->variants) {
      if (v.color == state->selectedColor) {
        chosen = v;
        state->selectedCapacity = v.capacity;
        break;
      }
    }
  }
  if (!chosen.id.empty()) {
    state->currentProduct = chosen;
  }

  if (state->lblPrice) {
    gtk_label_set_text(
        GTK_LABEL(state->lblPrice),
        UIComponents::formatVND(state->currentProduct.sellingPrice).c_str());
  }

  if (state->lblOldPrice && state->lblDiscBadge) {
    if (state->currentProduct.hasActiveSale()) {
      int pct = state->currentProduct.calculateDiscountPercent();
      gtk_label_set_text(GTK_LABEL(state->lblDiscBadge),
                         ("-" + std::to_string(pct) + "%").c_str());
      gtk_widget_set_visible(state->lblDiscBadge, TRUE);
      gtk_label_set_text(
          GTK_LABEL(state->lblOldPrice),
          UIComponents::formatVND(state->currentProduct.listPrice).c_str());
      gtk_widget_set_visible(state->lblOldPrice, TRUE);
    } else {
      gtk_widget_set_visible(state->lblDiscBadge, FALSE);
      gtk_widget_set_visible(state->lblOldPrice, FALSE);
    }
  }

  if (state->lblStockHint) {
    std::string stockHint =
        (state->currentProduct.availableStock > 0)
            ? ("(" + std::to_string(state->currentProduct.availableStock) +
               " sản phẩm có sẵn)")
            : "(Hiện đang tạm hết hàng)";
    gtk_label_set_text(GTK_LABEL(state->lblStockHint), stockHint.c_str());
  }

  if (state->lblCrumbName) {
    gtk_label_set_text(GTK_LABEL(state->lblCrumbName),
                       state->currentProduct.name.c_str());
  }
  if (state->lblTitle) {
    gtk_label_set_text(GTK_LABEL(state->lblTitle),
                       state->currentProduct.name.c_str());
  }
  if (state->lblSku) {
    gtk_label_set_text(GTK_LABEL(state->lblSku),
                       ("Mã: " + state->currentProduct.id).c_str());
  }
  if (state->lblStockBadge) {
    gtk_widget_remove_css_class(state->lblStockBadge, "badge-completed");
    gtk_widget_remove_css_class(state->lblStockBadge, "badge-pending");
    gtk_widget_remove_css_class(state->lblStockBadge, "badge-cancelled");
    if (state->currentProduct.availableStock > 5) {
      gtk_label_set_text(
          GTK_LABEL(state->lblStockBadge),
          ("✅ Còn hàng (" +
           std::to_string(state->currentProduct.availableStock) + " máy)")
              .c_str());
      gtk_widget_add_css_class(state->lblStockBadge, "badge-completed");
    } else if (state->currentProduct.availableStock > 0) {
      gtk_label_set_text(
          GTK_LABEL(state->lblStockBadge),
          ("⚠️ Sắp hết (Còn " +
           std::to_string(state->currentProduct.availableStock) + " máy)")
              .c_str());
      gtk_widget_add_css_class(state->lblStockBadge, "badge-pending");
    } else {
      gtk_label_set_text(GTK_LABEL(state->lblStockBadge), "❌ Hết hàng");
      gtk_widget_add_css_class(state->lblStockBadge, "badge-cancelled");
    }
  }
  updateImageStage(state);

  for (GtkWidget *btn : state->capacityChipButtons) {
    const char *lbl = gtk_button_get_label(GTK_BUTTON(btn));
    if (lbl && state->selectedCapacity == lbl) {
      gtk_widget_add_css_class(btn, "pdp-chip-active");
    } else {
      gtk_widget_remove_css_class(btn, "pdp-chip-active");
    }
  }

  for (GtkWidget *btn : state->colorChipButtons) {
    const char *lbl = gtk_button_get_label(GTK_BUTTON(btn));
    if (lbl && state->selectedColor == lbl) {
      gtk_widget_add_css_class(btn, "pdp-chip-active");
    } else {
      gtk_widget_remove_css_class(btn, "pdp-chip-active");
    }
  }
}

// Xử lý Lưu sản phẩm
void onSaveProduct(ProductDetailState *state) {
  syncFromInputs(state);

  // Validate các trường bắt buộc
  UIComponents::clearFieldError(state->errName, state->entryName);
  UIComponents::clearFieldError(state->errBrand, state->entryBrand);
  UIComponents::clearFieldError(state->errSellingPrice,
                                state->entrySellingPrice);
  UIComponents::clearFieldError(state->errListPrice, state->entryListPrice);
  UIComponents::clearFieldError(state->errImportPrice, state->entryImportPrice);
  UIComponents::clearFieldError(state->errStock, state->entryActualStock);

  bool hasError = false;
  if (state->currentProduct.name.empty()) {
    UIComponents::setFieldError(state->errName, state->entryName,
                                "Tên sản phẩm không được để trống!");
    hasError = true;
  }
  if (state->currentProduct.brand.empty()) {
    UIComponents::setFieldError(state->errBrand, state->entryBrand,
                                "Vui lòng nhập thương hiệu sản phẩm!");
    hasError = true;
  }
  if (state->currentProduct.sellingPrice <= 0) {
    UIComponents::setFieldError(state->errSellingPrice,
                                state->entrySellingPrice,
                                "Giá bán phải lớn hơn 0 VND!");
    hasError = true;
  }
  if (state->currentProduct.listPrice > 0 &&
      state->currentProduct.listPrice <= state->currentProduct.sellingPrice) {
    UIComponents::setFieldError(
        state->errListPrice, state->entryListPrice,
        "Giá niêm yết (gốc) phải lớn hơn giá bán khuyến mãi!");
    hasError = true;
  }
  if (state->currentProduct.actualStock < 0) {
    UIComponents::setFieldError(state->errStock, state->entryActualStock,
                                "Số lượng tồn kho không được âm!");
    hasError = true;
  }
  if (state->currentProduct.warrantyMonths < 0) {
    UIComponents::setFieldError(state->errWarranty, state->entryWarranty,
                                "Thời gian bảo hành không hợp lệ!");
    hasError = true;
  }
  if (state->currentProduct.returnDays < 0) {
    UIComponents::setFieldError(state->errReturnDays, state->entryReturnDays,
                                "Thời gian đổi trả không hợp lệ!");
    hasError = true;
  }

  if (hasError) {
    UIComponents::showErrorDialog(
        state->shell ? state->shell->getWindow() : nullptr, "Lỗi xác thực",
        "Vui lòng kiểm tra lại các thông tin báo lỗi trên màn hình.");
    return;
  }

  auto performSave = [](ProductDetailState *s) {
    OperationResult res;
    if (s->isNew) {
      res = s->controller->addProduct(s->currentProduct);
    } else {
      res = s->controller->updateProduct(s->currentProduct);
    }

    if (res.success) {
      s->hasUnsavedChanges = false;
      s->originalProduct = s->currentProduct;
      UIComponents::showSuccessDialog(s->shell ? s->shell->getWindow()
                                               : nullptr,
                                      "Thành công", res.message);
      if (s->callbacks.onSaved) {
        s->callbacks.onSaved();
      }
    } else {
      UIComponents::showErrorDialog(s->shell ? s->shell->getWindow() : nullptr,
                                    "Lỗi lưu sản phẩm", res.message);
    }
  };

  // Cảnh báo bán lỗ (sellingPrice < importPrice)
  if (state->currentProduct.importPrice > 0 &&
      state->currentProduct.sellingPrice < state->currentProduct.importPrice) {
    UIComponents::showConfirmDialog(
        state->shell ? state->shell->getWindow() : nullptr,
        "Cảnh báo bán lỗ vốn",
        "Giá bán (" +
            UIComponents::formatVND(state->currentProduct.sellingPrice) +
            ") thấp hơn giá nhập (" +
            UIComponents::formatVND(state->currentProduct.importPrice) +
            "). Bạn có chắc chắn muốn lưu sản phẩm này không?",
        +[](gpointer data) {
          ProductDetailState *s = static_cast<ProductDetailState *>(data);
          OperationResult res;
          if (s->isNew) {
            res = s->controller->addProduct(s->currentProduct);
          } else {
            res = s->controller->updateProduct(s->currentProduct);
          }
          if (res.success) {
            s->hasUnsavedChanges = false;
            s->originalProduct = s->currentProduct;
            UIComponents::showSuccessDialog(s->shell ? s->shell->getWindow()
                                                     : nullptr,
                                            "Thành công", res.message);
            if (s->callbacks.onSaved) {
              s->callbacks.onSaved();
            }
          } else {
            UIComponents::showErrorDialog(s->shell ? s->shell->getWindow()
                                                   : nullptr,
                                          "Lỗi lưu sản phẩm", res.message);
          }
        },
        state);
    return;
  }

  performSave(state);
}

// Xử lý Xóa sản phẩm
void onDeleteProduct(ProductDetailState *state) {
  if (state->isNew)
    return;

  UIComponents::showConfirmDialog(
      state->shell ? state->shell->getWindow() : nullptr,
      "Xác nhận xóa sản phẩm",
      "Bạn có chắc chắn muốn xóa vĩnh viễn sản phẩm '" +
          state->currentProduct.name +
          "' khỏi danh mục không? Thao tác này không thể hoàn tác.",
      +[](gpointer data) {
        ProductDetailState *s = static_cast<ProductDetailState *>(data);
        OperationResult res =
            s->controller->deleteProduct(s->currentProduct.id);
        if (res.success) {
          UIComponents::showSuccessDialog(s->shell ? s->shell->getWindow()
                                                   : nullptr,
                                          "Đã xóa sản phẩm", res.message);
          if (s->callbacks.onDeleted) {
            s->callbacks.onDeleted();
          }
        } else {
          UIComponents::showErrorDialog(s->shell ? s->shell->getWindow()
                                                 : nullptr,
                                        "Lỗi xóa sản phẩm", res.message);
        }
      },
      state);
}

// Xử lý Hủy bỏ
void onCancelProduct(ProductDetailState *state) {
  syncFromInputs(state);
  if (state->hasUnsavedChanges) {
    UIComponents::showConfirmDialog(
        state->shell ? state->shell->getWindow() : nullptr, "Xác nhận hủy",
        "Bạn có các thay đổi chưa được lưu. Bạn có chắc muốn hủy bỏ không?",
        +[](gpointer data) {
          ProductDetailState *s = static_cast<ProductDetailState *>(data);
          if (s->callbacks.onCancelled) {
            s->callbacks.onCancelled();
          }
        },
        state);
  } else {
    if (state->callbacks.onCancelled) {
      state->callbacks.onCancelled();
    }
  }
}

// Hàm dựng toàn bộ giao diện dựa trên mode hiện tại
void renderView(ProductDetailState *state) {
  // Xóa toàn bộ nội dung cũ trong contentBox
  GtkWidget *child = gtk_widget_get_first_child(state->contentBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(state->contentBox), child);
    child = next;
  }

  state->specRows.clear();
  bool isAdminEdit = (state->mode == ProductDetailMode::AdminEdit);
  bool isAdminPreview = (state->mode == ProductDetailMode::AdminPreview);
  bool isSalesPos = (state->mode == ProductDetailMode::SalesPos);
  bool isCustomer = (state->mode == ProductDetailMode::Customer);
  bool isAdminMode = (isAdminEdit || isAdminPreview);

  // ================= 1. BREADCRUMB + CHẾ ĐỘ XEM TRƯỚC / EDIT TOGGLE
  // =================
  GtkWidget *breadcrumbBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_add_css_class(breadcrumbBar, "pdp-breadcrumb-bar");

  GtkWidget *btnHome = gtk_button_new_with_label("🏠 Trang chủ");
  gtk_widget_add_css_class(btnHome, "pdp-breadcrumb-link");
  UIComponents::setButtonCursor(btnHome);
  g_signal_connect_swapped(btnHome, "clicked",
                           G_CALLBACK(+[](ProductDetailState *s) {
                             if (s->callbacks.onNavigateHome)
                               s->callbacks.onNavigateHome();
                           }),
                           state);
  gtk_box_append(GTK_BOX(breadcrumbBar), btnHome);

  GtkWidget *sep1 = gtk_label_new("›");
  gtk_widget_add_css_class(sep1, "pdp-breadcrumb-sep");
  gtk_box_append(GTK_BOX(breadcrumbBar), sep1);

  std::string brandLabel = state->currentProduct.brand.empty()
                               ? "Điện thoại"
                               : state->currentProduct.brand;
  GtkWidget *btnBrand = gtk_button_new_with_label(brandLabel.c_str());
  gtk_widget_add_css_class(btnBrand, "pdp-breadcrumb-link");
  UIComponents::setButtonCursor(btnBrand);
  g_signal_connect_swapped(
      btnBrand, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
        if (s->callbacks.onBrandSelected)
          s->callbacks.onBrandSelected(s->currentProduct.brand);
      }),
      state);
  gtk_box_append(GTK_BOX(breadcrumbBar), btnBrand);

  GtkWidget *sep2 = gtk_label_new("›");
  gtk_widget_add_css_class(sep2, "pdp-breadcrumb-sep");
  gtk_box_append(GTK_BOX(breadcrumbBar), sep2);

  std::string prodTitle =
      state->isNew ? "Thêm sản phẩm mới" : state->currentProduct.name;
  state->lblCrumbName = gtk_label_new(prodTitle.c_str());
  gtk_widget_add_css_class(state->lblCrumbName, "pdp-breadcrumb-current");
  gtk_label_set_xalign(GTK_LABEL(state->lblCrumbName), 0.0f);
  gtk_label_set_ellipsize(GTK_LABEL(state->lblCrumbName), PANGO_ELLIPSIZE_END);
  gtk_widget_set_hexpand(state->lblCrumbName, TRUE);
  gtk_box_append(GTK_BOX(breadcrumbBar), state->lblCrumbName);

  // Badge thay đổi chưa lưu
  state->unsavedIndicator = gtk_label_new("⚠️ Có thay đổi chưa lưu");
  gtk_widget_add_css_class(state->unsavedIndicator, "pdp-unsaved-badge");
  gtk_widget_set_visible(state->unsavedIndicator, state->hasUnsavedChanges);
  gtk_box_append(GTK_BOX(breadcrumbBar), state->unsavedIndicator);

  // Nút công tắc chuyển đổi AdminEdit ⇄ AdminPreview (B4-4)
  if (isAdminMode) {
    GtkWidget *btnToggleMode = nullptr;
    if (isAdminEdit) {
      btnToggleMode = gtk_button_new_with_label("👁️ Xem như khách");
      gtk_widget_add_css_class(btnToggleMode, "btn-outline");
      g_signal_connect_swapped(btnToggleMode, "clicked",
                               G_CALLBACK(+[](ProductDetailState *s) {
                                 syncFromInputs(s);
                                 s->mode = ProductDetailMode::AdminPreview;
                                 renderView(s);
                               }),
                               state);
    } else {
      btnToggleMode = gtk_button_new_with_label("✏️ Quay lại chỉnh sửa");
      gtk_widget_add_css_class(btnToggleMode, "btn-primary");
      g_signal_connect_swapped(btnToggleMode, "clicked",
                               G_CALLBACK(+[](ProductDetailState *s) {
                                 s->mode = ProductDetailMode::AdminEdit;
                                 renderView(s);
                               }),
                               state);
    }
    UIComponents::setButtonCursor(btnToggleMode);
    gtk_box_append(GTK_BOX(breadcrumbBar), btnToggleMode);
  }

  gtk_box_append(GTK_BOX(state->contentBox), breadcrumbBar);

  // ================= 2. KHUNG CHỈ ADMIN (B4-3) =================
  if (isAdminMode) {
    GtkWidget *adminCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(adminCard, "pdp-admin-panel");

    GtkWidget *admHeaderRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *lblAdmTitle =
        gtk_label_new("🔒 BẢNG KIỂM SOÁT QUẢN TRỊ (ADMIN ONLY)");
    gtk_widget_add_css_class(lblAdmTitle, "stat-label");
    gtk_box_append(GTK_BOX(admHeaderRow), lblAdmTitle);

    if (isAdminPreview) {
      GtkWidget *lblPrevTag =
          gtk_label_new("🔍 Đang ở chế độ xem trước khách hàng");
      gtk_widget_add_css_class(lblPrevTag, "badge-pending");
      gtk_widget_set_hexpand(lblPrevTag, TRUE);
      gtk_label_set_xalign(GTK_LABEL(lblPrevTag), 1.0f);
      gtk_box_append(GTK_BOX(admHeaderRow), lblPrevTag);
    }
    gtk_box_append(GTK_BOX(adminCard), admHeaderRow);

    // Bộ chọn nhanh các tổ hợp (Dung lượng × Màu sắc) của dòng máy này ngay trong màn Sửa
    if (!state->isNew && state->variants.size() > 1) {
      GtkWidget *varBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
      GtkWidget *lblVarHeader = gtk_label_new(
          ("📦 Chọn tổ hợp phiên bản để chỉnh sửa (" +
           std::to_string(state->variants.size()) + " tổ hợp):")
              .c_str());
      gtk_widget_add_css_class(lblVarHeader, "stat-label");
      gtk_label_set_xalign(GTK_LABEL(lblVarHeader), 0.0f);
      gtk_box_append(GTK_BOX(varBox), lblVarHeader);

      GtkWidget *varFlow = gtk_flow_box_new();
      gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(varFlow),
                                      GTK_SELECTION_NONE);
      gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(varFlow), 4);
      gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(varFlow), 8);
      gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(varFlow), 6);

      struct SwitchVarCtx {
        ProductDetailState *s;
        ProductItemViewModel v;
      };

      for (const auto &v : state->variants) {
        bool isCurrent = (v.id == state->currentProduct.id);
        std::string chipTxt =
            (v.capacity.empty() ? "" : (v.capacity + " - ")) +
            (v.color.empty() ? v.name : v.color) + " (" +
            std::to_string(v.actualStock) + " máy • " +
            UIComponents::formatCompactVND(v.sellingPrice) + ")";
        GtkWidget *btnVar = gtk_button_new_with_label(chipTxt.c_str());
        gtk_widget_add_css_class(btnVar, isCurrent ? "btn-primary"
                                                   : "btn-outline");
        UIComponents::setButtonCursor(btnVar);

        SwitchVarCtx *vctx = new SwitchVarCtx{state, v};
        g_signal_connect_data(
            btnVar, "clicked",
            G_CALLBACK(+[](GtkButton *, gpointer d) {
              SwitchVarCtx *c = static_cast<SwitchVarCtx *>(d);
              if (c->s->currentProduct.id == c->v.id)
                return;
              // Lấy dữ liệu mới nhất từ controller nếu đã lưu trước đó
              ProductItemViewModel latest =
                  c->s->controller
                      ? c->s->controller->getProductById(c->v.id)
                      : c->v;
              if (latest.id.empty())
                latest = c->v;
              c->s->currentProduct = latest;
              c->s->originalProduct = latest;
              c->s->selectedCapacity = latest.capacity;
              c->s->selectedColor = latest.color;
              c->s->hasUnsavedChanges = false;
              renderView(c->s);
            }),
            vctx,
            [](gpointer d, GClosure *) {
              delete static_cast<SwitchVarCtx *>(d);
            },
            (GConnectFlags)0);
        gtk_flow_box_insert(GTK_FLOW_BOX(varFlow), btnVar, -1);
      }

      gtk_box_append(GTK_BOX(varBox), varFlow);
      gtk_box_append(GTK_BOX(adminCard), varBox);
    }

    GtkWidget *kpiRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);

    // SKU
    GtkWidget *boxSku = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lblL1 = gtk_label_new("Mã SP (SKU)");
    gtk_widget_add_css_class(lblL1, "stat-label");
    GtkWidget *lblV1 = gtk_label_new(
        state->isNew ? "(Tự sinh khi lưu)" : state->currentProduct.id.c_str());
    gtk_widget_add_css_class(lblV1, "stat-value");
    gtk_box_append(GTK_BOX(boxSku), lblL1);
    gtk_box_append(GTK_BOX(boxSku), lblV1);
    gtk_box_append(GTK_BOX(kpiRow), boxSku);

    // Giá nhập
    GtkWidget *boxImp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lblL2 = gtk_label_new("Giá nhập");
    gtk_widget_add_css_class(lblL2, "stat-label");
    if (isAdminEdit) {
      state->entryImportPrice = gtk_entry_new();
      std::string impStr = (state->currentProduct.importPrice > 0)
                               ? std::to_string(static_cast<long long>(
                                     state->currentProduct.importPrice))
                               : "";
      gtk_editable_set_text(GTK_EDITABLE(state->entryImportPrice),
                            impStr.c_str());
      UIComponents::setupCurrencyInput(state->entryImportPrice);
      gtk_widget_add_css_class(state->entryImportPrice, "pdp-editable");
      g_signal_connect_swapped(state->entryImportPrice, "changed",
                               G_CALLBACK(recalculateLiveBadgesAndKPIs), state);
      gtk_box_append(GTK_BOX(boxImp), lblL2);
      gtk_box_append(GTK_BOX(boxImp), state->entryImportPrice);
    } else {
      GtkWidget *lblV2 = gtk_label_new(
          UIComponents::formatVND(state->currentProduct.importPrice).c_str());
      gtk_widget_add_css_class(lblV2, "stat-value");
      gtk_box_append(GTK_BOX(boxImp), lblL2);
      gtk_box_append(GTK_BOX(boxImp), lblV2);
    }
    gtk_box_append(GTK_BOX(kpiRow), boxImp);

    // Lợi nhuận mỗi máy
    double profit =
        state->currentProduct.sellingPrice - state->currentProduct.importPrice;
    double margin = (state->currentProduct.sellingPrice > 0)
                        ? (profit * 100.0 / state->currentProduct.sellingPrice)
                        : 0.0;
    GtkWidget *boxProfit = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lblL3 = gtk_label_new("Lợi nhuận dự kiến");
    gtk_widget_add_css_class(lblL3, "stat-label");
    state->lblAdminProfit =
        gtk_label_new(UIComponents::formatVND(profit).c_str());
    gtk_widget_add_css_class(state->lblAdminProfit, "stat-value");
    gtk_box_append(GTK_BOX(boxProfit), lblL3);
    gtk_box_append(GTK_BOX(boxProfit), state->lblAdminProfit);
    gtk_box_append(GTK_BOX(kpiRow), boxProfit);

    // Biên lợi nhuận
    GtkWidget *boxMargin = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lblL4 = gtk_label_new("Biên lợi nhuận %");
    gtk_widget_add_css_class(lblL4, "stat-label");
    char mBuf[32];
    std::snprintf(mBuf, sizeof(mBuf), "%.1f%%", margin);
    state->lblAdminMargin = gtk_label_new(mBuf);
    gtk_widget_add_css_class(state->lblAdminMargin, "stat-value");
    gtk_box_append(GTK_BOX(boxMargin), lblL4);
    gtk_box_append(GTK_BOX(boxMargin), state->lblAdminMargin);
    gtk_box_append(GTK_BOX(kpiRow), boxMargin);

    // Tồn thực
    GtkWidget *boxAct = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lblL5 = gtk_label_new("Tồn thực");
    gtk_widget_add_css_class(lblL5, "stat-label");
    if (isAdminEdit) {
      state->entryActualStock = gtk_entry_new();
      gtk_editable_set_text(
          GTK_EDITABLE(state->entryActualStock),
          std::to_string(state->currentProduct.actualStock).c_str());
      gtk_widget_add_css_class(state->entryActualStock, "pdp-editable");
      g_signal_connect_swapped(state->entryActualStock, "changed",
                               G_CALLBACK(recalculateLiveBadgesAndKPIs), state);
      gtk_box_append(GTK_BOX(boxAct), lblL5);
      gtk_box_append(GTK_BOX(boxAct), state->entryActualStock);
    } else {
      GtkWidget *lblV5 = gtk_label_new(
          (std::to_string(state->currentProduct.actualStock) + " máy").c_str());
      gtk_widget_add_css_class(lblV5, "stat-value");
      gtk_box_append(GTK_BOX(boxAct), lblL5);
      gtk_box_append(GTK_BOX(boxAct), lblV5);
    }
    gtk_box_append(GTK_BOX(kpiRow), boxAct);

    // Giữ chỗ
    GtkWidget *boxRes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lblL6 = gtk_label_new("Giữ chỗ (Đơn chờ)");
    gtk_widget_add_css_class(lblL6, "stat-label");
    GtkWidget *lblV6 = gtk_label_new(
        (std::to_string(state->currentProduct.reservedStock) + " máy").c_str());
    gtk_widget_add_css_class(lblV6, "stat-value");
    gtk_box_append(GTK_BOX(boxRes), lblL6);
    gtk_box_append(GTK_BOX(boxRes), lblV6);
    gtk_box_append(GTK_BOX(kpiRow), boxRes);

    // Khả dụng
    GtkWidget *boxAvail = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lblL7 = gtk_label_new("Khả dụng bán");
    gtk_widget_add_css_class(lblL7, "stat-label");
    state->lblAdminAvail = gtk_label_new(
        (std::to_string(state->currentProduct.availableStock) + " máy")
            .c_str());
    gtk_widget_add_css_class(state->lblAdminAvail, "stat-value");
    gtk_box_append(GTK_BOX(boxAvail), lblL7);
    gtk_box_append(GTK_BOX(boxAvail), state->lblAdminAvail);
    gtk_box_append(GTK_BOX(kpiRow), boxAvail);

    gtk_box_append(GTK_BOX(adminCard), kpiRow);
    gtk_box_append(GTK_BOX(state->contentBox), adminCard);
  }

  // ================= 3. HERO CARD (ẢNH + THÔNG TIN BÁN HÀNG) =================
  GtkWidget *heroCard =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_2XL);
  gtk_widget_add_css_class(heroCard, "pdp-card");

  // ---------- CỘT TRÁI: Gallery Ảnh ----------
  GtkWidget *leftCol =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_set_size_request(leftCol, 380, -1);
  gtk_widget_set_hexpand(leftCol, FALSE);
  gtk_widget_set_valign(leftCol, GTK_ALIGN_START);

  state->imgStage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_add_css_class(state->imgStage, "pdp-image-stage");
  gtk_widget_set_size_request(state->imgStage, 360, 300);

  state->imgCenterBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_vexpand(state->imgCenterBox, TRUE);
  gtk_widget_set_valign(state->imgCenterBox, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(state->imgCenterBox, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(state->imgStage), state->imgCenterBox);
  gtk_box_append(GTK_BOX(leftCol), state->imgStage);

  // Dải Thumbnail
  state->thumbStrip = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_set_halign(state->thumbStrip, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(leftCol), state->thumbStrip);

  // Bộ điều khiển quản lý ảnh (AdminEdit)
  if (isAdminEdit) {
    GtkWidget *imgCtrlBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_add_css_class(imgCtrlBox, "card-nested");

    GtkWidget *imgBtnRow1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *btnPickFile = gtk_button_new_with_label("🖼️ Thêm ảnh từ máy...");
    gtk_widget_add_css_class(btnPickFile, "btn-outline");
    UIComponents::setButtonCursor(btnPickFile);

    g_signal_connect_swapped(
        btnPickFile, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
          GtkFileChooserNative *native = gtk_file_chooser_native_new(
              "Chọn ảnh sản phẩm", s->shell ? s->shell->getWindow() : nullptr,
              GTK_FILE_CHOOSER_ACTION_OPEN, "Chọn", "Hủy");
          GtkFileFilter *filter = gtk_file_filter_new();
          gtk_file_filter_set_name(filter,
                                   "Hình ảnh (*.png, *.jpg, *.jpeg, *.webp)");
          gtk_file_filter_add_mime_type(filter, "image/png");
          gtk_file_filter_add_mime_type(filter, "image/jpeg");
          gtk_file_filter_add_mime_type(filter, "image/webp");
          gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

          g_signal_connect_data(
              native, "response",
              G_CALLBACK(
                  +[](GtkNativeDialog *dialog, gint resId, gpointer data) {
                    if (resId == GTK_RESPONSE_ACCEPT) {
                      GFile *f =
                          gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
                      if (f) {
                        char *path = g_file_get_path(f);
                        if (path) {
                          ProductDetailState *st =
                              static_cast<ProductDetailState *>(data);
                          ProductImageItem newImg;
                          newImg.path = path;
                          newImg.isPrimary = st->currentProduct.images.empty();
                          newImg.displayOrder = static_cast<int>(
                              st->currentProduct.images.size() + 1);
                          st->currentProduct.images.push_back(newImg);
                          st->currentImageIndex =
                              st->currentProduct.images.size() - 1;
                          st->hasUnsavedChanges = true;
                          updateImageStage(st);
                          g_free(path);
                        }
                        g_object_unref(f);
                      }
                    }
                    g_object_unref(dialog);
                  }),
              s, nullptr, (GConnectFlags)0);

          gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
        }),
        state);

    GtkWidget *btnSetPrimary =
        gtk_button_new_with_label("⭐ Đặt làm ảnh chính");
    gtk_widget_add_css_class(btnSetPrimary, "btn-outline");
    UIComponents::setButtonCursor(btnSetPrimary);
    g_signal_connect_swapped(
        btnSetPrimary, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
          if (s->currentImageIndex < s->currentProduct.images.size()) {
            for (size_t i = 0; i < s->currentProduct.images.size(); ++i) {
              s->currentProduct.images[i].isPrimary =
                  (i == s->currentImageIndex);
            }
            s->currentProduct.imageUrl =
                s->currentProduct.images[s->currentImageIndex].path;
            s->hasUnsavedChanges = true;
            updateImageStage(s);
          }
        }),
        state);

    GtkWidget *btnDelImg = gtk_button_new_with_label("🗑️ Xóa");
    gtk_widget_add_css_class(btnDelImg, "btn-danger");
    UIComponents::setButtonCursor(btnDelImg);
    g_signal_connect_swapped(
        btnDelImg, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
          if (!s->currentProduct.images.empty() &&
              s->currentImageIndex < s->currentProduct.images.size()) {
            s->currentProduct.images.erase(s->currentProduct.images.begin() +
                                           s->currentImageIndex);
            if (s->currentImageIndex > 0 &&
                s->currentImageIndex >= s->currentProduct.images.size()) {
              s->currentImageIndex = s->currentProduct.images.size() - 1;
            }
            s->hasUnsavedChanges = true;
            updateImageStage(s);
          }
        }),
        state);

    gtk_box_append(GTK_BOX(imgBtnRow1), btnPickFile);
    gtk_box_append(GTK_BOX(imgBtnRow1), btnSetPrimary);
    gtk_box_append(GTK_BOX(imgBtnRow1), btnDelImg);
    gtk_box_append(GTK_BOX(imgCtrlBox), imgBtnRow1);

    gtk_box_append(GTK_BOX(leftCol), imgCtrlBox);
  }

  gtk_box_append(GTK_BOX(heroCard), leftCol);

  // ---------- CỘT PHẢI: Thông tin sản phẩm, Giá, Biến thể, Mua hàng ----------
  GtkWidget *rightCol =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_set_hexpand(rightCol, TRUE);

  // Hàng huy hiệu: Hãng + Hệ điều hành + Tình trạng tồn kho
  GtkWidget *badgeRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

  if (isAdminEdit) {
    state->entryBrand = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(state->entryBrand),
                          state->currentProduct.brand.c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(state->entryBrand),
                                   "Hãng / Thương hiệu *");
    gtk_widget_add_css_class(state->entryBrand, "pdp-editable");
    gtk_box_append(GTK_BOX(badgeRow), state->entryBrand);

    state->entryOs = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(state->entryOs),
                          state->currentProduct.os.c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(state->entryOs),
                                   "Hệ điều hành (Android, iOS...)");
    gtk_widget_add_css_class(state->entryOs, "pdp-editable");
    gtk_box_append(GTK_BOX(badgeRow), state->entryOs);
  } else {
    GtkWidget *lblBrand =
        gtk_label_new(("🏷️ " + state->currentProduct.brand).c_str());
    gtk_widget_add_css_class(lblBrand, "pdp-badge");
    gtk_box_append(GTK_BOX(badgeRow), lblBrand);

    if (!state->currentProduct.os.empty()) {
      GtkWidget *lblOs =
          gtk_label_new(("⚙️ " + state->currentProduct.os).c_str());
      gtk_widget_add_css_class(lblOs, "pdp-badge");
      gtk_box_append(GTK_BOX(badgeRow), lblOs);
    }
  }

  GtkWidget *lblStockBadge = nullptr;
  if (state->currentProduct.availableStock > 5) {
    lblStockBadge = gtk_label_new(
        ("✅ Còn hàng (" +
         std::to_string(state->currentProduct.availableStock) + " máy)")
            .c_str());
    gtk_widget_add_css_class(lblStockBadge, "pdp-badge");
    gtk_widget_add_css_class(lblStockBadge, "badge-completed");
  } else if (state->currentProduct.availableStock > 0) {
    lblStockBadge = gtk_label_new(
        ("⚠️ Sắp hết (Còn " +
         std::to_string(state->currentProduct.availableStock) + " máy)")
            .c_str());
    gtk_widget_add_css_class(lblStockBadge, "pdp-badge");
    gtk_widget_add_css_class(lblStockBadge, "badge-pending");
  } else {
    lblStockBadge = gtk_label_new("❌ Hết hàng");
    gtk_widget_add_css_class(lblStockBadge, "pdp-badge");
    gtk_widget_add_css_class(lblStockBadge, "badge-cancelled");
  }
  gtk_box_append(GTK_BOX(badgeRow), lblStockBadge);
  state->lblStockBadge = lblStockBadge;

  if (!isCustomer) {
    GtkWidget *lblSku = gtk_label_new(
        ("Mã: " + (state->isNew ? "PHN-NEW" : state->currentProduct.id))
            .c_str());
    gtk_widget_add_css_class(lblSku, "page-subtitle");
    gtk_widget_add_css_class(lblSku, "tabular-num");
    gtk_widget_set_hexpand(lblSku, TRUE);
    gtk_label_set_xalign(GTK_LABEL(lblSku), 1.0f);
    gtk_box_append(GTK_BOX(badgeRow), lblSku);
    state->lblSku = lblSku;
  } else {
    state->lblSku = nullptr;
  }

  gtk_box_append(GTK_BOX(rightCol), badgeRow);

  // Tiêu đề sản phẩm
  if (isAdminEdit) {
    state->entryName = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(state->entryName),
                          state->currentProduct.name.c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(state->entryName),
                                   "Nhập tên sản phẩm điện thoại... *");
    gtk_widget_add_css_class(state->entryName, "pdp-editable");
    gtk_widget_add_css_class(state->entryName, "pdp-edit-title");
    gtk_box_append(GTK_BOX(rightCol), state->entryName);
  } else {
    GtkWidget *lblTitle = gtk_label_new(state->currentProduct.name.c_str());
    gtk_widget_add_css_class(lblTitle, "pdp-title");
    gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(lblTitle), TRUE);
    gtk_box_append(GTK_BOX(rightCol), lblTitle);
    state->lblTitle = lblTitle;
  }

  // Nhà sản xuất & Đánh giá sao
  if (isAdminEdit) {
    GtkWidget *manuRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lblM = gtk_label_new("Nhà sản xuất:");
    gtk_widget_add_css_class(lblM, "stat-label");
    state->entryManufacturer = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(state->entryManufacturer),
                          state->currentProduct.manufacturer.c_str());
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(state->entryManufacturer),
        "Tên nhà sản xuất ủy quyền (Apple, Samsung Electronics...)");
    gtk_widget_add_css_class(state->entryManufacturer, "pdp-editable");
    gtk_widget_set_hexpand(state->entryManufacturer, TRUE);
    gtk_box_append(GTK_BOX(manuRow), lblM);
    gtk_box_append(GTK_BOX(manuRow), state->entryManufacturer);
    gtk_box_append(GTK_BOX(rightCol), manuRow);
  }

  state->lblSubRating = gtk_label_new("");
  gtk_widget_add_css_class(state->lblSubRating, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(state->lblSubRating), 0.0f);
  gtk_box_append(GTK_BOX(rightCol), state->lblSubRating);
  updateRatingSummary(state);

  // Khung Giá bán + Giá niêm yết gốc + % Giảm + Trả góp
  GtkWidget *priceBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
  gtk_widget_add_css_class(priceBox, "pdp-price-box");

  if (isAdminEdit) {
    GtkWidget *pEditCol1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lblSellPrompt = gtk_label_new("Giá bán (VND) *");
    gtk_widget_add_css_class(lblSellPrompt, "stat-label");
    state->entrySellingPrice = gtk_entry_new();
    std::string sSell = (state->currentProduct.sellingPrice > 0)
                            ? std::to_string(static_cast<long long>(
                                  state->currentProduct.sellingPrice))
                            : "";
    gtk_editable_set_text(GTK_EDITABLE(state->entrySellingPrice),
                          sSell.c_str());
    UIComponents::setupCurrencyInput(state->entrySellingPrice);
    gtk_widget_add_css_class(state->entrySellingPrice, "pdp-editable");
    g_signal_connect_swapped(state->entrySellingPrice, "changed",
                             G_CALLBACK(recalculateLiveBadgesAndKPIs), state);
    gtk_box_append(GTK_BOX(pEditCol1), lblSellPrompt);
    gtk_box_append(GTK_BOX(pEditCol1), state->entrySellingPrice);
    gtk_box_append(GTK_BOX(priceBox), pEditCol1);

    GtkWidget *pEditCol2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lblListPrompt = gtk_label_new("Giá gốc / Niêm yết (VND)");
    gtk_widget_add_css_class(lblListPrompt, "stat-label");
    state->entryListPrice = gtk_entry_new();
    std::string sList = (state->currentProduct.listPrice > 0)
                            ? std::to_string(static_cast<long long>(
                                  state->currentProduct.listPrice))
                            : "";
    gtk_editable_set_text(GTK_EDITABLE(state->entryListPrice), sList.c_str());
    UIComponents::setupCurrencyInput(state->entryListPrice);
    gtk_widget_add_css_class(state->entryListPrice, "pdp-editable");
    g_signal_connect_swapped(state->entryListPrice, "changed",
                             G_CALLBACK(recalculateLiveBadgesAndKPIs), state);
    gtk_box_append(GTK_BOX(pEditCol2), lblListPrompt);
    gtk_box_append(GTK_BOX(pEditCol2), state->entryListPrice);
    gtk_box_append(GTK_BOX(priceBox), pEditCol2);

    state->lblDiscBadge = gtk_label_new("");
    gtk_widget_set_valign(state->lblDiscBadge, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(priceBox), state->lblDiscBadge);
    recalculateLiveBadgesAndKPIs(state);
  } else {
    state->lblPrice = gtk_label_new(
        UIComponents::formatVND(state->currentProduct.sellingPrice).c_str());
    gtk_widget_add_css_class(state->lblPrice, "pdp-price");
    gtk_widget_add_css_class(state->lblPrice, "tabular-num");
    gtk_label_set_xalign(GTK_LABEL(state->lblPrice), 0.0f);
    gtk_box_append(GTK_BOX(priceBox), state->lblPrice);

    // Chỉ hiện giá gạch ngang và badge -X% khi chương trình khuyến mãi đang hiệu lực (D1)
    if (state->currentProduct.hasActiveSale()) {
      int discountPct = state->currentProduct.calculateDiscountPercent();
      state->lblOldPrice = gtk_label_new(
          UIComponents::formatVND(state->currentProduct.listPrice).c_str());
      gtk_widget_add_css_class(state->lblOldPrice, "pdp-old-price");
      gtk_widget_add_css_class(state->lblOldPrice, "tabular-num");
      gtk_widget_set_valign(state->lblOldPrice, GTK_ALIGN_CENTER);
      gtk_box_append(GTK_BOX(priceBox), state->lblOldPrice);

      state->lblDiscBadge =
          gtk_label_new(("-" + std::to_string(discountPct) + "%").c_str());
      gtk_widget_add_css_class(state->lblDiscBadge, "pdp-badge-discount");
      gtk_widget_set_valign(state->lblDiscBadge, GTK_ALIGN_CENTER);
      gtk_box_append(GTK_BOX(priceBox), state->lblDiscBadge);
    }

    // Chỉ hiện Trả góp 0% khi installmentEnabled == true trong dữ liệu (A5-2)
    if (state->currentProduct.installmentEnabled) {
      std::string instText = "💳 Trả góp 0%";
      if (!state->currentProduct.installmentNote.empty()) {
        instText += " (" + state->currentProduct.installmentNote + ")";
      }
      state->lblInstallment = gtk_label_new(instText.c_str());
      gtk_widget_add_css_class(state->lblInstallment, "pdp-badge");
      gtk_widget_set_valign(state->lblInstallment, GTK_ALIGN_CENTER);
      gtk_widget_set_hexpand(state->lblInstallment, TRUE);
      gtk_label_set_xalign(GTK_LABEL(state->lblInstallment), 1.0f);
      gtk_box_append(GTK_BOX(priceBox), state->lblInstallment);
    }
  }

  gtk_box_append(GTK_BOX(rightCol), priceBox);

  // Cấu hình khuyến mãi & Trả góp (AdminEdit)
  if (isAdminEdit) {
    GtkWidget *saleDateBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lblDates = gtk_label_new("Thời gian sale:");
    gtk_widget_add_css_class(lblDates, "stat-label");
    state->entrySaleStart = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(state->entrySaleStart),
                                   "Từ ngày (YYYY-MM-DD)");
    gtk_editable_set_text(GTK_EDITABLE(state->entrySaleStart),
                          state->currentProduct.saleStart.c_str());
    gtk_widget_add_css_class(state->entrySaleStart, "pdp-editable");

    state->entrySaleEnd = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(state->entrySaleEnd),
                                   "Đến ngày (YYYY-MM-DD)");
    gtk_editable_set_text(GTK_EDITABLE(state->entrySaleEnd),
                          state->currentProduct.saleEnd.c_str());
    gtk_widget_add_css_class(state->entrySaleEnd, "pdp-editable");

    gtk_box_append(GTK_BOX(saleDateBox), lblDates);
    gtk_box_append(GTK_BOX(saleDateBox), state->entrySaleStart);
    gtk_box_append(GTK_BOX(saleDateBox), state->entrySaleEnd);
    gtk_box_append(GTK_BOX(rightCol), saleDateBox);

    GtkWidget *instBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    state->chkInstallment =
        gtk_check_button_new_with_label("Hỗ trợ trả góp 0%");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(state->chkInstallment),
                                state->currentProduct.installmentEnabled);
    state->entryInstallmentNote = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(state->entryInstallmentNote),
                                   "Ghi chú trả góp (vd: Qua thẻ tín dụng)...");
    gtk_editable_set_text(GTK_EDITABLE(state->entryInstallmentNote),
                          state->currentProduct.installmentNote.c_str());
    gtk_widget_add_css_class(state->entryInstallmentNote, "pdp-editable");
    gtk_widget_set_hexpand(state->entryInstallmentNote, TRUE);

    gtk_box_append(GTK_BOX(instBox), state->chkInstallment);
    gtk_box_append(GTK_BOX(instBox), state->entryInstallmentNote);
    gtk_box_append(GTK_BOX(rightCol), instBox);
  }

  // Dung lượng sản phẩm (ANTY.md mục 1)
  if (!isAdminEdit) {
    std::vector<std::string> caps;
    for (const auto &v : state->variants) {
      if (!v.capacity.empty() &&
          std::find(caps.begin(), caps.end(), v.capacity) == caps.end()) {
        caps.push_back(v.capacity);
      }
    }
    if (caps.empty()) {
      caps = {"128GB", "256GB", "512GB", "1TB"};
    }
    state->capacityChipButtons.clear();
    GtkWidget *capRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lblCap = gtk_label_new("Dung lượng:");
    gtk_widget_add_css_class(lblCap, "stat-label");
    gtk_box_append(GTK_BOX(capRow), lblCap);

    for (const auto &capStr : caps) {
      GtkWidget *chip = gtk_button_new_with_label(capStr.c_str());
      gtk_widget_add_css_class(chip, "pdp-chip");
      if (capStr == state->selectedCapacity) {
        gtk_widget_add_css_class(chip, "pdp-chip-active");
      }
      UIComponents::setButtonCursor(chip);
      struct ChipCapCtx {
        ProductDetailState *s;
        std::string cap;
      };
      ChipCapCtx *ctx = new ChipCapCtx{state, capStr};
      g_signal_connect_data(
          chip, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
            ChipCapCtx *c = static_cast<ChipCapCtx *>(d);
            c->s->selectedCapacity = c->cap;
            updateVariantSelection(c->s);
          }),
          ctx,
          [](gpointer d, GClosure *) { delete static_cast<ChipCapCtx *>(d); },
          (GConnectFlags)0);
      state->capacityChipButtons.push_back(chip);
      gtk_box_append(GTK_BOX(capRow), chip);
    }
    gtk_box_append(GTK_BOX(rightCol), capRow);
  }

  // Màu sắc sản phẩm (A5-3, ANTY.md mục 1)
  if (isAdminEdit) {
    GtkWidget *colorEditBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lblC =
        gtk_label_new("Danh sách màu sắc (cách nhau bởi dấu phẩy):");
    gtk_widget_add_css_class(lblC, "stat-label");

    std::string joinedColors = "";
    for (size_t i = 0; i < state->currentProduct.colors.size(); ++i) {
      if (i > 0)
        joinedColors += ", ";
      joinedColors += state->currentProduct.colors[i];
    }
    state->entryColors = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(state->entryColors),
                          joinedColors.c_str());
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(state->entryColors),
        "Titan Tự Nhiên, Đen Không Gian, Xanh Sa Mạc...");
    gtk_widget_add_css_class(state->entryColors, "pdp-editable");

    gtk_box_append(GTK_BOX(colorEditBox), lblC);
    gtk_box_append(GTK_BOX(colorEditBox), state->entryColors);
    gtk_box_append(GTK_BOX(rightCol), colorEditBox);
  } else {
    std::vector<std::string> cols;
    for (const auto &v : state->variants) {
      if (!v.color.empty() &&
          std::find(cols.begin(), cols.end(), v.color) == cols.end()) {
        cols.push_back(v.color);
      }
    }
    if (cols.empty())
      cols = state->currentProduct.colors;
    if (cols.empty())
      cols = {"Mặc định"};

    state->colorChipButtons.clear();
    GtkWidget *colorRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lblColorLabel = gtk_label_new("Màu sắc:");
    gtk_widget_add_css_class(lblColorLabel, "stat-label");
    gtk_box_append(GTK_BOX(colorRow), lblColorLabel);

    for (const auto &colStr : cols) {
      GtkWidget *chip = gtk_button_new_with_label(colStr.c_str());
      gtk_widget_add_css_class(chip, "pdp-chip");
      if (colStr == state->selectedColor) {
        gtk_widget_add_css_class(chip, "pdp-chip-active");
      }
      UIComponents::setButtonCursor(chip);
      struct ChipColCtx {
        ProductDetailState *s;
        std::string col;
      };
      ChipColCtx *ctx = new ChipColCtx{state, colStr};
      g_signal_connect_data(
          chip, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
            ChipColCtx *c = static_cast<ChipColCtx *>(d);
            c->s->selectedColor = c->col;
            updateVariantSelection(c->s);
          }),
          ctx,
          [](gpointer d, GClosure *) { delete static_cast<ChipColCtx *>(d); },
          (GConnectFlags)0);
      state->colorChipButtons.push_back(chip);
      gtk_box_append(GTK_BOX(colorRow), chip);
    }
    gtk_box_append(GTK_BOX(rightCol), colorRow);
  }

  // Số lượng & Nút hành động Mua / Thêm giỏ hàng (Chỉ khách hoặc xem trước)
  if (!isAdminEdit) {
    GtkWidget *qtyRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lblQtyTitle = gtk_label_new("Số lượng:");
    gtk_widget_add_css_class(lblQtyTitle, "stat-label");
    gtk_widget_set_size_request(lblQtyTitle, 90, -1);
    gtk_box_append(GTK_BOX(qtyRow), lblQtyTitle);

    state->btnMinus = gtk_button_new_with_label("−");
    gtk_widget_add_css_class(state->btnMinus, "pdp-qty-btn");
    gtk_widget_set_sensitive(state->btnMinus, state->selectedQty > 1);
    UIComponents::setButtonCursor(state->btnMinus);

    state->lblQty = gtk_label_new(std::to_string(state->selectedQty).c_str());
    gtk_widget_add_css_class(state->lblQty, "pdp-qty-value");
    gtk_widget_add_css_class(state->lblQty, "tabular-num");

    state->btnPlus = gtk_button_new_with_label("+");
    gtk_widget_add_css_class(state->btnPlus, "pdp-qty-btn");
    gtk_widget_set_sensitive(state->btnPlus, TRUE);
    UIComponents::setButtonCursor(state->btnPlus);

    g_signal_connect_swapped(
        state->btnMinus, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
          if (s->selectedQty > 1) {
            s->selectedQty--;
            gtk_label_set_text(GTK_LABEL(s->lblQty),
                               std::to_string(s->selectedQty).c_str());
            gtk_widget_set_sensitive(s->btnMinus, s->selectedQty > 1);
          }
        }),
        state);

    g_signal_connect_swapped(
        state->btnPlus, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
          s->selectedQty++;
          gtk_label_set_text(GTK_LABEL(s->lblQty),
                             std::to_string(s->selectedQty).c_str());
          gtk_widget_set_sensitive(s->btnMinus, s->selectedQty > 1);
        }),
        state);

    gtk_box_append(GTK_BOX(qtyRow), state->btnMinus);
    gtk_box_append(GTK_BOX(qtyRow), state->lblQty);
    gtk_box_append(GTK_BOX(qtyRow), state->btnPlus);

    std::string stockHint =
        (state->currentProduct.availableStock > 0)
            ? ("(" + std::to_string(state->currentProduct.availableStock) +
               " sản phẩm có sẵn)")
            : "(Hiện đang tạm hết hàng)";
    state->lblStockHint = gtk_label_new(stockHint.c_str());
    gtk_widget_add_css_class(state->lblStockHint, "page-subtitle");
    gtk_box_append(GTK_BOX(qtyRow), state->lblStockHint);

    gtk_box_append(GTK_BOX(rightCol), qtyRow);

    // Nút hành động lớn:
    // - Màn hình Sale (SalesPos): thu gọn 2 nút mua và để giỏ thành 1 nút duy nhất "🛒 Thêm vào giỏ"
    // - Màn hình Customer / AdminPreview: 2 nút "🛒 Thêm vào giỏ hàng" và "⚡ Mua ngay"
    GtkWidget *ctaRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_margin_top(ctaRow, 6);

    if (isSalesPos) {
      GtkWidget *btnPosAdd = gtk_button_new_with_label("🛒 Thêm vào giỏ");
      gtk_widget_add_css_class(btnPosAdd, "pdp-buy-btn");
      gtk_widget_set_hexpand(btnPosAdd, TRUE);
      UIComponents::setButtonCursor(btnPosAdd);

      g_signal_connect_swapped(
          btnPosAdd, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
            if (s->callbacks.onAddToPosCart) {
              s->callbacks.onAddToPosCart(s->currentProduct, s->selectedQty);
            }
          }),
          state);
      gtk_box_append(GTK_BOX(ctaRow), btnPosAdd);
    } else {
      GtkWidget *btnAddToCart =
          gtk_button_new_with_label("🛒 Thêm vào giỏ hàng");
      gtk_widget_add_css_class(btnAddToCart, "pdp-cart-btn");
      gtk_widget_set_hexpand(btnAddToCart, TRUE);

      GtkWidget *btnBuyNow = gtk_button_new_with_label("⚡ Mua ngay");
      gtk_widget_add_css_class(btnBuyNow, "pdp-buy-btn");
      gtk_widget_set_hexpand(btnBuyNow, TRUE);

      if (isAdminPreview) {
        // Trong chế độ xem trước của Admin: nút bị vô hiệu hóa để không tạo đơn
        // thử
        gtk_widget_set_sensitive(btnAddToCart, FALSE);
        gtk_widget_set_sensitive(btnBuyNow, FALSE);
        gtk_widget_set_tooltip_text(btnAddToCart,
                                    "Chế độ xem trước - Nút mua bị vô hiệu hóa");
        gtk_widget_set_tooltip_text(btnBuyNow,
                                    "Chế độ xem trước - Nút mua bị vô hiệu hóa");
      } else {
        UIComponents::setButtonCursor(btnAddToCart);
        UIComponents::setButtonCursor(btnBuyNow);

        g_signal_connect_swapped(
            btnAddToCart, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
              if (!s->controller)
                return;
              const UserSessionViewModel &cur = s->controller->getCurrentUser();
              OperationResult res = s->controller->addToCart(
                  cur.userId, s->currentProduct.id, s->selectedQty);
              if (res.success) {
                if (s->shell) {
                  CartSummaryViewModel cart =
                      s->controller->getCart(cur.userId);
                  s->shell->updateCartBadge(cart.totalQuantity);
                }
                UIComponents::showSuccessDialog(
                    s->shell ? s->shell->getWindow() : nullptr, "Thành công",
                    res.message);
              } else {
                UIComponents::showWarningDialog(
                    s->shell ? s->shell->getWindow() : nullptr,
                    "Không thể thêm", res.message);
              }
            }),
            state);

        g_signal_connect_swapped(
            btnBuyNow, "clicked", G_CALLBACK(+[](ProductDetailState *s) {
              if (!s->controller)
                return;
              const UserSessionViewModel &cur = s->controller->getCurrentUser();
              OperationResult res = s->controller->addToCart(
                  cur.userId, s->currentProduct.id, s->selectedQty);
              if (res.success) {
                if (s->shell) {
                  CartSummaryViewModel cart =
                      s->controller->getCart(cur.userId);
                  s->shell->updateCartBadge(cart.totalQuantity);
                  s->shell->navigateToCartWithOnlySelected(
                      s->currentProduct.id);
                }
              } else {
                UIComponents::showWarningDialog(
                    s->shell ? s->shell->getWindow() : nullptr,
                    "Không thể mua ngay", res.message);
              }
            }),
            state);
      }

      gtk_box_append(GTK_BOX(ctaRow), btnAddToCart);
      gtk_box_append(GTK_BOX(ctaRow), btnBuyNow);
    }
    gtk_box_append(GTK_BOX(rightCol), ctaRow);
  }

  // Khung chính sách bảo hành & đổi trả (A5-5)
  GtkWidget *policyBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_add_css_class(policyBox, "pdp-policy-box");

  if (isAdminEdit) {
    GtkWidget *polRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *lblW = gtk_label_new("Bảo hành (tháng):");
    gtk_widget_add_css_class(lblW, "stat-label");
    state->entryWarranty = gtk_entry_new();
    gtk_editable_set_text(
        GTK_EDITABLE(state->entryWarranty),
        std::to_string(state->currentProduct.warrantyMonths).c_str());
    gtk_widget_add_css_class(state->entryWarranty, "pdp-editable");

    GtkWidget *lblR = gtk_label_new("Đổi trả (ngày):");
    gtk_widget_add_css_class(lblR, "stat-label");
    state->entryReturnDays = gtk_entry_new();
    gtk_editable_set_text(
        GTK_EDITABLE(state->entryReturnDays),
        std::to_string(state->currentProduct.returnDays).c_str());
    gtk_widget_add_css_class(state->entryReturnDays, "pdp-editable");

    gtk_box_append(GTK_BOX(polRow), lblW);
    gtk_box_append(GTK_BOX(polRow), state->entryWarranty);
    gtk_box_append(GTK_BOX(polRow), lblR);
    gtk_box_append(GTK_BOX(polRow), state->entryReturnDays);
    gtk_box_append(GTK_BOX(policyBox), polRow);

    GtkWidget *lblStoreNote =
        gtk_label_new("🚚 Chính sách vận chuyển & đồng kiểm áp dụng theo cài "
                      "đặt chung của cửa hàng.");
    gtk_widget_add_css_class(lblStoreNote, "shortcut-hint");
    gtk_box_append(GTK_BOX(policyBox), lblStoreNote);
  } else {
    std::string wText = "🛡️ Bảo hành chính hãng " +
                        std::to_string(state->currentProduct.warrantyMonths) +
                        " tháng tại trung tâm ủy quyền toàn quốc";
    std::string rText = "🔄 Hỗ trợ 1 đổi 1 trong " +
                        std::to_string(state->currentProduct.returnDays) +
                        " ngày đầu nếu có lỗi phần cứng từ nhà sản xuất";
    std::string sText = "🚚 Giao hàng nhanh toàn quốc • Miễn phí vận chuyển • "
                        "Đồng kiểm khi nhận hàng";
    if (state->controller) {
      sText = "🚚 " + state->controller->getStoreSettings().get(
                          "store_shipping_policy", sText);
    }

    for (const std::string &pol : {wText, rText, sText}) {
      GtkWidget *lblPol = gtk_label_new(pol.c_str());
      gtk_label_set_xalign(GTK_LABEL(lblPol), 0.0f);
      gtk_label_set_wrap(GTK_LABEL(lblPol), TRUE);
      gtk_box_append(GTK_BOX(policyBox), lblPol);
    }
  }

  gtk_box_append(GTK_BOX(rightCol), policyBox);
  gtk_box_append(GTK_BOX(heroCard), rightCol);
  gtk_box_append(GTK_BOX(state->contentBox), heroCard);

  // ================= 4. THÔNG SỐ KỸ THUẬT & MÔ TẢ SẢN PHẨM =================
  GtkWidget *detailsRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_XL);

  // Card Thông số kỹ thuật
  GtkWidget *specsCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_add_css_class(specsCard, "pdp-card");
  gtk_widget_set_hexpand(specsCard, TRUE);
  gtk_widget_set_valign(specsCard, GTK_ALIGN_START);

  GtkWidget *specsHeaderBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  GtkWidget *lblSpecsHeader = gtk_label_new("📋 Thông số kỹ thuật chi tiết");
  gtk_widget_add_css_class(lblSpecsHeader, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblSpecsHeader), 0.0f);
  gtk_widget_set_hexpand(lblSpecsHeader, TRUE);
  gtk_box_append(GTK_BOX(specsHeaderBox), lblSpecsHeader);

  if (isAdminEdit) {
    GtkWidget *btnAddSpec = gtk_button_new_with_label("➕ Thêm dòng");
    gtk_widget_add_css_class(btnAddSpec, "btn-outline");
    UIComponents::setButtonCursor(btnAddSpec);
    g_signal_connect_swapped(btnAddSpec, "clicked",
                             G_CALLBACK(+[](ProductDetailState *s) {
                               syncFromInputs(s);
                               s->currentProduct.detailedSpecs.push_back(
                                   {"Thông số mới", "Giá trị"});
                               s->hasUnsavedChanges = true;
                               renderView(s);
                             }),
                             state);
    gtk_box_append(GTK_BOX(specsHeaderBox), btnAddSpec);
  }
  gtk_box_append(GTK_BOX(specsCard), specsHeaderBox);

  // Bảng thông số chi tiết (A5-7)
  if (isAdminEdit) {
    GtkWidget *specsContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    for (size_t i = 0; i < state->currentProduct.detailedSpecs.size(); ++i) {
      const auto &spec = state->currentProduct.detailedSpecs[i];
      GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
      gtk_widget_add_css_class(row, "pdp-spec-row");

      GtkWidget *eKey = gtk_entry_new();
      gtk_editable_set_text(GTK_EDITABLE(eKey), spec.first.c_str());
      gtk_widget_add_css_class(eKey, "pdp-editable");
      gtk_widget_set_size_request(eKey, 150, -1);

      GtkWidget *eVal = gtk_entry_new();
      gtk_editable_set_text(GTK_EDITABLE(eVal), spec.second.c_str());
      gtk_widget_add_css_class(eVal, "pdp-editable");
      gtk_widget_set_hexpand(eVal, TRUE);

      GtkWidget *btnDel = gtk_button_new_with_label("❌");
      gtk_widget_add_css_class(btnDel, "btn-secondary");
      UIComponents::setButtonCursor(btnDel);

      struct DelSpecData {
        ProductDetailState *s;
        size_t idx;
      };
      DelSpecData *dsd = new DelSpecData{state, i};
      g_signal_connect_data(
          btnDel, "clicked", G_CALLBACK(+[](GtkButton *, gpointer d) {
            DelSpecData *data = static_cast<DelSpecData *>(d);
            syncFromInputs(data->s);
            if (data->idx < data->s->currentProduct.detailedSpecs.size()) {
              data->s->currentProduct.detailedSpecs.erase(
                  data->s->currentProduct.detailedSpecs.begin() + data->idx);
              data->s->hasUnsavedChanges = true;
              renderView(data->s);
            }
          }),
          dsd,
          [](gpointer d, GClosure *) { delete static_cast<DelSpecData *>(d); },
          (GConnectFlags)0);

      gtk_box_append(GTK_BOX(row), eKey);
      gtk_box_append(GTK_BOX(row), eVal);
      gtk_box_append(GTK_BOX(row), btnDel);

      state->specRows.push_back({eKey, eVal, row});
      gtk_box_append(GTK_BOX(specsContainer), row);
    }
    gtk_box_append(GTK_BOX(specsCard), specsContainer);
  } else {
    // Chế độ xem: Hiển thị bảng 2 cột sạch đẹp từ detailedSpecs thật
    size_t specRowIdx = 0;
    auto addRow = [&specRowIdx, specsCard](const std::string &k,
                                           const std::string &v) {
      if (k.empty() || v.empty())
        return;
      GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
      gtk_widget_add_css_class(row, "pdp-spec-row");
      gtk_widget_add_css_class(row,
                               (specRowIdx % 2 == 0) ? "row-even" : "row-odd");
      specRowIdx++;

      GtkWidget *lblK = gtk_label_new(k.c_str());
      gtk_widget_add_css_class(lblK, "stat-label");
      gtk_widget_set_size_request(lblK, 160, -1);
      gtk_label_set_xalign(GTK_LABEL(lblK), 0.0f);

      GtkWidget *lblV = gtk_label_new(v.c_str());
      gtk_label_set_xalign(GTK_LABEL(lblV), 0.0f);
      gtk_label_set_wrap(GTK_LABEL(lblV), TRUE);
      gtk_widget_set_hexpand(lblV, TRUE);

      gtk_box_append(GTK_BOX(row), lblK);
      gtk_box_append(GTK_BOX(row), lblV);
      gtk_box_append(GTK_BOX(specsCard), row);
    };

    addRow("Mã sản phẩm (SKU)", state->currentProduct.id);
    addRow("Thương hiệu", state->currentProduct.brand);
    if (!state->currentProduct.manufacturer.empty()) {
      addRow("Nhà sản xuất", state->currentProduct.manufacturer);
    }
    if (!state->currentProduct.os.empty()) {
      addRow("Hệ điều hành", state->currentProduct.os);
    }

    if (!state->currentProduct.detailedSpecs.empty()) {
      for (const auto &sp : state->currentProduct.detailedSpecs) {
        addRow(sp.first, sp.second);
      }
    } else if (!state->currentProduct.specs.empty()) {
      addRow("Cấu hình tóm tắt", state->currentProduct.specs);
    }
  }

  gtk_box_append(GTK_BOX(detailsRow), specsCard);

  // Card Mô tả sản phẩm (A5-6)
  GtkWidget *descCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(descCard, "pdp-card");
  gtk_widget_set_hexpand(descCard, TRUE);
  gtk_widget_set_valign(descCard, GTK_ALIGN_START);

  GtkWidget *lblDescHeader = gtk_label_new("📝 Mô tả sản phẩm");
  gtk_widget_add_css_class(lblDescHeader, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblDescHeader), 0.0f);
  gtk_box_append(GTK_BOX(descCard), lblDescHeader);

  if (isAdminEdit) {
    GtkWidget *lblH = gtk_label_new("Điểm nổi bật ngắn (Highlights):");
    gtk_widget_add_css_class(lblH, "stat-label");
    state->entryShortHighlights = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(state->entryShortHighlights),
                          state->currentProduct.shortHighlights.c_str());
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(state->entryShortHighlights),
        "Màn hình 120Hz mượt mà • Camera tiềm vọng 50MP...");
    gtk_widget_add_css_class(state->entryShortHighlights, "pdp-editable");

    GtkWidget *lblD = gtk_label_new("Nội dung mô tả chi tiết:");
    gtk_widget_add_css_class(lblD, "stat-label");

    GtkWidget *scrollDesc = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrollDesc, -1, 200);
    state->textDescription = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(state->textDescription),
                                GTK_WRAP_WORD_CHAR);
    gtk_widget_add_css_class(state->textDescription, "pdp-editable");
    GtkTextBuffer *buf =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->textDescription));
    gtk_text_buffer_set_text(buf, state->currentProduct.description.c_str(),
                             -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollDesc),
                                  state->textDescription);

    state->lblCharCount = gtk_label_new(
        ("Ký tự: " + std::to_string(state->currentProduct.description.size()))
            .c_str());
    gtk_widget_add_css_class(state->lblCharCount, "shortcut-hint");
    gtk_label_set_xalign(GTK_LABEL(state->lblCharCount), 1.0f);

    auto onBufChanged = +[](ProductDetailState *s) {
      if (!s->textDescription || !s->lblCharCount)
        return;
      GtkTextBuffer *b =
          gtk_text_view_get_buffer(GTK_TEXT_VIEW(s->textDescription));
      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter(b, &start);
      gtk_text_buffer_get_end_iter(b, &end);
      char *t = gtk_text_buffer_get_text(b, &start, &end, FALSE);
      if (t) {
        gtk_label_set_text(GTK_LABEL(s->lblCharCount),
                           ("Ký tự: " + std::to_string(strlen(t))).c_str());
        g_free(t);
      }
      s->hasUnsavedChanges = true;
      if (s->unsavedIndicator)
        gtk_widget_set_visible(s->unsavedIndicator, TRUE);
    };
    g_signal_connect_swapped(buf, "changed", G_CALLBACK(onBufChanged), state);

    gtk_box_append(GTK_BOX(descCard), lblH);
    gtk_box_append(GTK_BOX(descCard), state->entryShortHighlights);
    gtk_box_append(GTK_BOX(descCard), lblD);
    gtk_box_append(GTK_BOX(descCard), scrollDesc);
    gtk_box_append(GTK_BOX(descCard), state->lblCharCount);
  } else {
    if (!state->currentProduct.shortHighlights.empty()) {
      GtkWidget *lblHigh = gtk_label_new(
          ("✨ " + state->currentProduct.shortHighlights).c_str());
      gtk_widget_add_css_class(lblHigh, "stat-label");
      gtk_label_set_xalign(GTK_LABEL(lblHigh), 0.0f);
      gtk_label_set_wrap(GTK_LABEL(lblHigh), TRUE);
      gtk_box_append(GTK_BOX(descCard), lblHigh);
    }

    std::string descBody =
        state->currentProduct.description.empty()
            ? "Đang cập nhật thông tin mô tả chi tiết cho sản phẩm này."
            : state->currentProduct.description;
    GtkWidget *lblDesc = gtk_label_new(descBody.c_str());
    gtk_label_set_xalign(GTK_LABEL(lblDesc), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(lblDesc), TRUE);
    gtk_box_append(GTK_BOX(descCard), lblDesc);
  }

  gtk_box_append(GTK_BOX(detailsRow), descCard);
  gtk_box_append(GTK_BOX(state->contentBox), detailsRow);

  // ================= 5. ĐÁNH GIÁ TỪ KHÁCH HÀNG (A5-8) =================
  if (isCustomer || isSalesPos || isAdminPreview) {
    GtkWidget *reviewsCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(reviewsCard, "pdp-card");

    GtkWidget *revHeaderRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *lblRevTitle =
        gtk_label_new("⭐ Đánh giá & Nhận xét từ khách hàng");
    gtk_widget_add_css_class(lblRevTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblRevTitle), 0.0f);
    gtk_widget_set_hexpand(lblRevTitle, TRUE);
    gtk_box_append(GTK_BOX(revHeaderRow), lblRevTitle);
    gtk_box_append(GTK_BOX(reviewsCard), revHeaderRow);

    // Form đánh giá
    GtkWidget *revFormBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_add_css_class(revFormBox, "card");

    GtkWidget *revScoreBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_size_request(revScoreBox, 160, -1);
    gtk_widget_set_valign(revScoreBox, GTK_ALIGN_CENTER);

    GtkWidget *lblScoreNum = gtk_label_new("5.0");
    gtk_widget_add_css_class(lblScoreNum, "stat-value");
    gtk_label_set_xalign(GTK_LABEL(lblScoreNum), 0.5f);

    GtkWidget *lblScoreStars = gtk_label_new("⭐⭐⭐⭐⭐");
    gtk_label_set_xalign(GTK_LABEL(lblScoreStars), 0.5f);

    GtkWidget *lblScoreCount = gtk_label_new("0 đánh giá");
    gtk_widget_add_css_class(lblScoreCount, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblScoreCount), 0.5f);

    gtk_box_append(GTK_BOX(revScoreBox), lblScoreNum);
    gtk_box_append(GTK_BOX(revScoreBox), lblScoreStars);
    gtk_box_append(GTK_BOX(revScoreBox), lblScoreCount);
    gtk_box_append(GTK_BOX(revFormBox), revScoreBox);

    GtkWidget *revVsep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_append(GTK_BOX(revFormBox), revVsep);

    GtkWidget *revNoteCol = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand(revNoteCol, TRUE);
    gtk_widget_set_valign(revNoteCol, GTK_ALIGN_CENTER);

    GtkWidget *lblNoteTitle =
        gtk_label_new("💬 Nhận xét từ người mua hàng thực tế");
    gtk_widget_add_css_class(lblNoteTitle, "form-label");
    gtk_label_set_xalign(GTK_LABEL(lblNoteTitle), 0.0f);

    GtkWidget *lblNoteDesc =
        gtk_label_new("Chỉ những khách hàng đã mua và hoàn tất đơn hàng mới có "
                      "thể đánh giá sản phẩm.\n"
                      "Nếu bạn đã mua sản phẩm này, vui lòng truy cập mục "
                      "\"Đơn hàng của tôi\" để viết hoặc chỉnh sửa đánh giá.");
    gtk_widget_add_css_class(lblNoteDesc, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblNoteDesc), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(lblNoteDesc), TRUE);

    gtk_box_append(GTK_BOX(revNoteCol), lblNoteTitle);
    gtk_box_append(GTK_BOX(revNoteCol), lblNoteDesc);
    gtk_box_append(GTK_BOX(revFormBox), revNoteCol);
    gtk_box_append(GTK_BOX(reviewsCard), revFormBox);

    GtkWidget *reviewsListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(reviewsCard), reviewsListBox);

    struct RevUIData {
      ProductDetailState *state;
      GtkWidget *listBox;
      GtkWidget *lblNum;
      GtkWidget *lblStars;
      GtkWidget *lblCount;
    };

    auto loadReviewsUI = [](RevUIData *data) {
      GtkWidget *ch = gtk_widget_get_first_child(data->listBox);
      while (ch != nullptr) {
        GtkWidget *nx = gtk_widget_get_next_sibling(ch);
        gtk_box_remove(GTK_BOX(data->listBox), ch);
        ch = nx;
      }

      std::vector<ProductReviewViewModel> rList =
          data->state->controller->getProductReviews(
              data->state->currentProduct.id);
      if (rList.empty()) {
        gtk_label_set_text(GTK_LABEL(data->lblNum), "0.0");
        gtk_label_set_text(GTK_LABEL(data->lblStars), "☆☆☆☆☆");
        gtk_label_set_text(GTK_LABEL(data->lblCount), "Chưa có đánh giá");

        GtkWidget *empty =
            gtk_label_new("Chưa có đánh giá nào cho sản phẩm này.");
        gtk_widget_add_css_class(empty, "page-subtitle");
        gtk_box_append(GTK_BOX(data->listBox), empty);
        return;
      }

      double sum = 0.0;
      for (const auto &r : rList)
        sum += r.ratingStars;
      double avg = sum / rList.size();
      char avgBuf[16];
      std::snprintf(avgBuf, sizeof(avgBuf), "%.1f", avg);
      gtk_label_set_text(GTK_LABEL(data->lblNum), avgBuf);

      int rounded = static_cast<int>(avg + 0.5);
      std::string starStr = "";
      for (int s = 0; s < 5; ++s)
        starStr += (s < rounded) ? "⭐" : "☆";
      gtk_label_set_text(GTK_LABEL(data->lblStars), starStr.c_str());
      gtk_label_set_text(GTK_LABEL(data->lblCount),
                         (std::to_string(rList.size()) + " đánh giá").c_str());

      for (const auto &r : rList) {
        GtkWidget *rCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_widget_add_css_class(rCard, "card-nested");

        GtkWidget *rTop = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget *rUser = gtk_label_new(
            r.customerName.empty() ? "Khách hàng" : r.customerName.c_str());
        gtk_widget_add_css_class(rUser, "table-primary-text");
        gtk_box_append(GTK_BOX(rTop), rUser);

        std::string sText = "";
        for (int s = 0; s < r.ratingStars; ++s)
          sText += "⭐";
        GtkWidget *rStar = gtk_label_new(sText.c_str());
        gtk_box_append(GTK_BOX(rTop), rStar);

        GtkWidget *rTime = gtk_label_new(r.reviewTime.c_str());
        gtk_widget_add_css_class(rTime, "page-subtitle");
        gtk_widget_set_hexpand(rTime, TRUE);
        gtk_label_set_xalign(GTK_LABEL(rTime), 1.0f);
        gtk_box_append(GTK_BOX(rTop), rTime);
        gtk_box_append(GTK_BOX(rCard), rTop);

        GtkWidget *rComment = gtk_label_new(r.comment.c_str());
        gtk_label_set_xalign(GTK_LABEL(rComment), 0.0f);
        gtk_label_set_wrap(GTK_LABEL(rComment), TRUE);
        gtk_box_append(GTK_BOX(rCard), rComment);

        gtk_box_append(GTK_BOX(data->listBox), rCard);
      }
    };

    RevUIData *revData = new RevUIData{state, reviewsListBox, lblScoreNum,
                                       lblScoreStars, lblScoreCount};
    g_object_set_data_full(
        G_OBJECT(reviewsCard), "rev_ui_data", revData,
        [](gpointer d) { delete static_cast<RevUIData *>(d); });
    loadReviewsUI(revData);

    gtk_box_append(GTK_BOX(state->contentBox), reviewsCard);
  }

  // ================= 6. THANH HÀNH ĐỘNG CỐ ĐỊNH PHÍA DƯỚI (B4-5)
  // =================
  if (isAdminMode) {
    GtkWidget *actionBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(actionBar, "pdp-action-bar");

    GtkWidget *lblUnsavedNotice = gtk_label_new(
        state->hasUnsavedChanges ? "⚠️ Có thay đổi chưa lưu" : "Đã đồng bộ");
    gtk_widget_add_css_class(lblUnsavedNotice, state->hasUnsavedChanges
                                                   ? "pdp-unsaved-badge"
                                                   : "shortcut-hint");
    gtk_box_append(GTK_BOX(actionBar), lblUnsavedNotice);

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(actionBar), spacer);

    if (!state->isNew) {
      GtkWidget *btnDelete = gtk_button_new_with_label("🗑️ Xóa sản phẩm");
      gtk_widget_add_css_class(btnDelete, "btn-danger");
      UIComponents::setButtonCursor(btnDelete);
      g_signal_connect_swapped(btnDelete, "clicked",
                               G_CALLBACK(onDeleteProduct), state);
      gtk_box_append(GTK_BOX(actionBar), btnDelete);
    }

    GtkWidget *btnCancel = gtk_button_new_with_label("Hủy bỏ");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(onCancelProduct),
                             state);
    gtk_box_append(GTK_BOX(actionBar), btnCancel);

    GtkWidget *btnSave = gtk_button_new_with_label(
        state->isNew ? "💾 Tạo sản phẩm mới" : "💾 Lưu thay đổi");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    g_signal_connect_swapped(btnSave, "clicked", G_CALLBACK(onSaveProduct),
                             state);
    gtk_box_append(GTK_BOX(actionBar), btnSave);

    gtk_box_append(GTK_BOX(state->contentBox), actionBar);
  }

  // Khởi tạo hiển thị ảnh
  updateImageStage(state);
}

} // namespace

GtkWidget *ProductDetailView::create(AppController *controller,
                                     MainShellWindow *shell,
                                     const ProductItemViewModel &product,
                                     ProductDetailMode mode, bool isNew,
                                     Callbacks callbacks) {
  ProductDetailState *state = new ProductDetailState();
  state->controller = controller;
  state->shell = shell;
  state->mode = mode;
  state->isNew = isNew;
  state->callbacks = callbacks;
  state->originalProduct = product;
  state->currentProduct = product;
  state->hasUnsavedChanges = false;
  state->currentImageIndex = 0;
  state->selectedQty = 1;

  if (controller) {
    state->variants = controller->getProductVariants(product.id);
  }
  if (state->variants.empty()) {
    state->variants.push_back(product);
  }
  state->selectedCapacity = product.capacity;
  state->selectedColor =
      product.color.empty()
          ? (!product.colors.empty() ? product.colors[0] : "Mặc định")
          : product.color;

  state->scrollWindow = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(state->scrollWindow),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(state->scrollWindow, TRUE);
  gtk_widget_set_vexpand(state->scrollWindow, TRUE);

  state->contentBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(state->contentBox, "page-container");

  // Giải phóng state khi widget bị hủy
  g_object_set_data_full(
      G_OBJECT(state->scrollWindow), "pdp_view_state", state,
      [](gpointer d) { delete static_cast<ProductDetailState *>(d); });

  renderView(state);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(state->scrollWindow),
                                state->contentBox);
  return state->scrollWindow;
}
