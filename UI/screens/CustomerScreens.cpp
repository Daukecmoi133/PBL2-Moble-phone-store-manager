/**
 * @file CustomerScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ KHÁCH HÀNG (CUSTOMER PORTAL)
 *
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildDashboardView: Hiển thị lời chào, cấp bậc thành viên (Rank badge), số
 * điểm và chi tiêu.
 * - buildProductsView & refreshProducts: Duyệt danh mục điện thoại, tìm kiếm từ
 * khóa, lọc theo hãng.
 * - buildCartView & refreshCart: Quản lý giỏ hàng, cập nhật số lượng, nhập
 * voucher, đặt hàng.
 * - buildOrdersView & refreshOrders: Lịch sử đơn hàng, xem chi tiết và hủy đơn
 * hàng khi đủ điều kiện.
 * - buildProfileView & refreshProfile: Thông tin cá nhân, quyền lợi thăng hạng
 * theo điểm tích lũy.
 * - buildWarrantyView & refreshWarranty: Tra cứu chính sách và đăng ký bảo hành
 * cho thiết bị đã mua.
 * =========================================================================================
 */

#include "CustomerScreens.h"
#include "../../source/utils/PathHelper.h"
#include "../../source/utils/Utf8Helper.h"
#include "../components/ProductDetailView.h"
#include "../components/UIComponents.h"
#include "../mock/RealTimeManager.h"
#include "../styles/AppCss.h"
#include "MainShellWindow.h"
#include <algorithm>
#include <sstream>


namespace {
static const std::vector<UIComponents::TableColumnSpec> kCustomerCartCols = {
    {"Chọn", 65, 0.5f, false},
    {"Tên sản phẩm & Đơn giá", 220, 0.0f, true},
    {"Số lượng", 150, 0.5f, false},
    {"Thành tiền", 160, 1.0f, false},
    {"Thao tác", 105, 0.5f, false}};

static const std::vector<UIComponents::TableColumnSpec> kCustomerOrderCols = {
    {"Mã đơn", 190, 0.0f, false},
    {"Trạng thái", 155, 0.5f, false},
    {"Thanh toán", 155, 0.5f, false},
    {"Thời gian & Hình thức", 200, 0.0f, false},
    {"Danh sách sản phẩm", 220, 0.0f, true},
    {"Tổng tiền", 150, 1.0f, false},
    {"Thao tác", 180, 0.5f, false}};

static const std::vector<UIComponents::TableColumnSpec> kCustomerWarrantyCols =
    {{"Mã phiếu", 195, 0.0f, false},
     {"Thiết bị", 210, 0.0f, false},
     {"Ngày gửi", 165, 0.0f, false},
     {"Trạng thái", 155, 0.5f, false},
     {"Hiện tượng lỗi & Phản hồi kỹ thuật", 220, 0.0f, true}};
} // namespace

struct AddToCartData {
  CustomerScreens *screens;
  std::string phoneId;
};

struct CartQtyData {
  CustomerScreens *screens;
  std::string phoneId;
  int newQty;
};

struct CancelOrderData {
  CustomerScreens *screens;
  std::string orderId;
};

struct VoucherActionData {
  CustomerScreens *screens;
  std::string code;
};

CustomerScreens::CustomerScreens(AppController *ctrl,
                                 MainShellWindow *parentShell)
    : controller(ctrl), shell(parentShell), currentSearchKeyword(""),
      currentBrandFilter("Tất cả"), currentSortBy("name_asc") {

  dashboardView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  productsView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productsView), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  cartView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(cartView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  ordersView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ordersView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  discountsView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(discountsView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  profileView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(profileView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  warrantyView = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyView),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  discountsFilterType = "Tất cả";
  discountsFilterStatus = "active";
  discountsSortBy = "expiry_asc"; // MẶC ĐỊNH: Tự động sắp xếp theo thời gian hết hạn gần nhất trước

  buildDashboardView();
  buildProductsView();
  buildCartView();
  buildOrdersView();
  buildDiscountsView();
  buildProfileView();
  buildWarrantyView();
}

void CustomerScreens::refreshAll() {
  refreshDashboard();
  refreshProducts();
  refreshCart();
  refreshOrders();
  refreshDiscounts();
  refreshProfile();
  refreshWarranty();
}

// ================= 1. CUSTOMER DASHBOARD =================

void CustomerScreens::buildDashboardView() {

  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  const UserSessionViewModel &user = controller->getCurrentUser();
  CustomerItemViewModel cust = controller->getCustomerById(user.userId);
  std::vector<OrderItemViewModel> myOrders =
      controller->getCustomerOrders(user.userId);
  std::vector<WarrantyItemViewModel> myWarranties =
      controller->getCustomerWarranties(user.userId);
  CartSummaryViewModel myCart = controller->getCart(user.userId);

  // Welcome Card with Membership Rank Badge
  GtkWidget *welcomeCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_SM);
  gtk_widget_add_css_class(welcomeCard, "card");
  gtk_widget_set_name(welcomeCard, "customer_welcome_card");

  GtkWidget *welcomeTopRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  std::string welcomeTitle =
      "👋 Chào mừng trở lại, " + (user.fullName.empty() ? "Khách hàng" : user.fullName);
  GtkWidget *lblTitle = gtk_label_new(welcomeTitle.c_str());
  gtk_widget_add_css_class(lblTitle, "page-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_widget_set_hexpand(lblTitle, TRUE);

  GtkWidget *rankBadge = UIComponents::createRankBadge(cust.membershipTier);
  gtk_widget_set_valign(rankBadge, GTK_ALIGN_CENTER);

  gtk_box_append(GTK_BOX(welcomeTopRow), lblTitle);
  gtk_box_append(GTK_BOX(welcomeTopRow), rankBadge);

  GtkWidget *lblSubtitle =
      gtk_label_new("Chào mừng bạn đến với cửa hàng điện thoại PBL2 Store. "
                    "Khám phá các mẫu điện thoại mới nhất hôm nay!");
  gtk_widget_add_css_class(lblSubtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(lblSubtitle), 0.0f);

  gtk_box_append(GTK_BOX(welcomeCard), welcomeTopRow);
  gtk_box_append(GTK_BOX(welcomeCard), lblSubtitle);
  gtk_box_append(GTK_BOX(content), welcomeCard);

  // 4 Role-Specific Customer KPI Cards (Clickable, SVG icons, proper labels)
  GtkWidget *statsGrid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(statsGrid), DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(statsGrid),
                              DesignTokens::KPI_GRID_SPACING);
  gtk_grid_set_column_homogeneous(GTK_GRID(statsGrid), TRUE);

  int pendingCount = 0;
  for (const auto &o : myOrders) {
    if (o.status == OrderStatus::PENDING ||
        o.status == OrderStatus::CONFIRMED ||
        o.status == OrderStatus::PREPARING ||
        o.status == OrderStatus::SHIPPING ||
        o.status == OrderStatus::READY_FOR_PICKUP) {
      pendingCount++;
    }
  }

  int activeWarranties = 0;
  for (const auto &w : myWarranties) {
    if (w.status == WarrantyStatus::RECEIVED || w.status == WarrantyStatus::PROCESSING) {
      activeWarranties++;
    }
  }

  std::string nextTierInfo;
  if (cust.points < 50) {
    nextTierInfo = "Còn " + std::to_string(50 - cust.points) + " điểm để lên hạng Bạc";
  } else if (cust.points < 200) {
    nextTierInfo = "Còn " + std::to_string(200 - cust.points) + " điểm để lên hạng Vàng";
  } else if (cust.points < 500) {
    nextTierInfo = "Còn " + std::to_string(500 - cust.points) + " điểm để lên hạng Kim Cương";
  } else {
    nextTierInfo = "Đã đạt thứ hạng cao nhất (Kim Cương VIP)";
  }

  std::string cartSub = myCart.totalQuantity > 0
      ? ("Tạm tính: " + UIComponents::formatVND(myCart.grandTotal))
      : "Chưa có sản phẩm trong giỏ";

  std::string warrantySub = (activeWarranties > 0)
      ? (std::to_string(activeWarranties) + " phiếu đang hoạt động")
      : "Bảo hành chính hãng";

  GtkWidget *cardPoints = UIComponents::createClickableStatCard(
      "Điểm tích lũy",
      UIComponents::formatNumber(cust.points) + " điểm",
      nextTierInfo, "star", "purple",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        CustomerScreens *s = static_cast<CustomerScreens *>(d);
        if (s && s->shell) s->shell->navigateTo("customer_profile");
      }),
      this);

  GtkWidget *cardCart = UIComponents::createClickableStatCard(
      "Giỏ hàng của tôi",
      UIComponents::formatNumber(myCart.totalQuantity) + " sản phẩm",
      cartSub, "shopping-cart", "green",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        CustomerScreens *s = static_cast<CustomerScreens *>(d);
        if (s && s->shell) s->shell->navigateTo("customer_cart");
      }),
      this);

  GtkWidget *cardOrders = UIComponents::createClickableStatCard(
      "Đơn hàng đang xử lý",
      UIComponents::formatNumber(pendingCount) + " đơn",
      "Đã đặt tất cả: " + UIComponents::formatNumber(static_cast<long long>(myOrders.size())) + " đơn",
      "package", pendingCount > 0 ? "amber" : "blue",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        CustomerScreens *s = static_cast<CustomerScreens *>(d);
        if (s && s->shell) s->shell->navigateTo("customer_orders");
      }),
      this);

  GtkWidget *cardWarranty = UIComponents::createClickableStatCard(
      "Phiếu bảo hành",
      UIComponents::formatNumber(static_cast<long long>(myWarranties.size())) + " phiếu",
      warrantySub, "shield-check", "cyan",
      G_CALLBACK(+[](GtkGestureClick *, int, double, double, gpointer d) {
        CustomerScreens *s = static_cast<CustomerScreens *>(d);
        if (s && s->shell) s->shell->navigateTo("customer_warranty");
      }),
      this);

  gtk_grid_attach(GTK_GRID(statsGrid), cardPoints, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(statsGrid), cardCart, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(statsGrid), cardOrders, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(statsGrid), cardWarranty, 3, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), statsGrid);

  // Promo Coupons Banner
  GtkWidget *promoCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(promoCard, "card");

  GtkWidget *promoHeaderRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  GtkWidget *lblPromoHeader =
      gtk_label_new("🎟️ Ưu đãi & Mã giảm giá đang diễn ra");
  gtk_widget_add_css_class(lblPromoHeader, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblPromoHeader), 0.0f);
  gtk_widget_set_hexpand(lblPromoHeader, TRUE);

  gtk_box_append(GTK_BOX(promoHeaderRow), lblPromoHeader);

  GtkWidget *btnAllVouchers = gtk_button_new_with_label("Xem ví voucher ›");
  gtk_widget_add_css_class(btnAllVouchers, "btn-outline");
  UIComponents::setButtonCursor(btnAllVouchers);
  g_signal_connect_swapped(
      btnAllVouchers, "clicked",
      G_CALLBACK(+[](CustomerScreens *s) {
        if (s && s->shell) s->shell->navigateTo("customer_discounts");
      }),
      this);
  gtk_box_append(GTK_BOX(promoHeaderRow), btnAllVouchers);
  gtk_box_append(GTK_BOX(promoCard), promoHeaderRow);

  RealTimeManager &rtm = RealTimeManager::getInstance();
  std::vector<DiscountItemViewModel> discounts = controller->getDiscounts();
  std::vector<DiscountItemViewModel> validDiscounts;
  for (const auto &d : discounts) {
    if (d.isValid && !rtm.isDateExpired(d.expiryDate) && (d.maxUsage <= 0 || d.usedCount < d.maxUsage)) {
      validDiscounts.push_back(d);
    }
  }

  if (validDiscounts.empty()) {
    GtkWidget *emptyBox = UIComponents::createEmptyState(
        "", "Hiện chưa có ưu đãi nào",
        "Các chương trình giảm giá và quà tặng sẽ sớm xuất hiện. Hãy quay lại sau nhé!");
    gtk_box_append(GTK_BOX(promoCard), emptyBox);
  } else {
    GtkWidget *promoTable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(promoTable, "table-container");

    size_t discIdx = 0;
    for (const auto &d : validDiscounts) {
      GtkWidget *row = UIComponents::createTableDataRow(discIdx++);

      GtkWidget *codeBadge = gtk_label_new(d.code.c_str());
      gtk_widget_add_css_class(codeBadge, "voucher-code-badge");

      std::string condStr = (d.minOrderValue <= 0)
          ? "Áp dụng cho mọi đơn hàng"
          : ("Đơn từ " + UIComponents::formatVND(d.minOrderValue));
      std::string descText = (d.description.empty() || d.description.find("từ 0") != std::string::npos)
          ? ("Giảm " + d.formattedValue + " (" + condStr + ")")
          : d.description;

      GtkWidget *desc = gtk_label_new(descText.c_str());
      gtk_widget_add_css_class(desc, "cell-title");
      gtk_widget_set_hexpand(desc, TRUE);
      gtk_label_set_xalign(GTK_LABEL(desc), 0.0f);

      int remDays = rtm.getRemainingDays(d.expiryDate);
      bool urgent = (remDays >= 0 && remDays <= 7);
      GtkWidget *exp = gtk_label_new(("HSD: " + d.expiryDate).c_str());
      gtk_widget_add_css_class(exp, urgent ? "voucher-expiry-urgent" : "voucher-expiry-normal");
      gtk_widget_add_css_class(exp, "tabular-num");

      GtkWidget *btnCopy = gtk_button_new_with_label("📋 Sao chép mã");
      gtk_widget_add_css_class(btnCopy, "voucher-copy-btn");
      UIComponents::setButtonCursor(btnCopy);

      struct CopyData {
        CustomerScreens *screens;
        std::string code;
      };
      CopyData *cd = new CopyData{this, d.code};
      g_signal_connect_data(
          btnCopy, "clicked",
          G_CALLBACK(+[](GtkButton *, gpointer data) {
            CopyData *c = static_cast<CopyData *>(data);
            GdkClipboard *cb = gdk_display_get_clipboard(gdk_display_get_default());
            gdk_clipboard_set_text(cb, c->code.c_str());
            if (c->screens && c->screens->shell) {
              c->screens->shell->showToast("Đã sao chép mã '" + c->code + "' vào bộ nhớ tạm!", "success");
            }
          }),
          cd, [](gpointer d, GClosure *) { delete static_cast<CopyData *>(d); },
          (GConnectFlags)0);

      gtk_box_append(GTK_BOX(row), codeBadge);
      gtk_box_append(GTK_BOX(row), desc);
      gtk_box_append(GTK_BOX(row), exp);
      gtk_box_append(GTK_BOX(row), btnCopy);
      gtk_box_append(GTK_BOX(promoTable), row);
    }
    gtk_box_append(GTK_BOX(promoCard), promoTable);
  }
  gtk_box_append(GTK_BOX(content), promoCard);

  // Quick Action Bar
  GtkWidget *actionsBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  
  GtkWidget *btnShop =
      gtk_button_new_with_label("🛍️ Xem danh mục điện thoại");
  gtk_widget_add_css_class(btnShop, "btn-primary");
  UIComponents::setButtonCursor(btnShop);
  g_signal_connect_swapped(btnShop, "clicked",
                           G_CALLBACK(+[](MainShellWindow *s) {
                             s->navigateTo("customer_products");
                           }),
                           shell);

  GtkWidget *btnCart = gtk_button_new_with_label("🛒 Xem giỏ hàng của tôi");
  gtk_widget_add_css_class(btnCart, "btn-secondary");
  UIComponents::setButtonCursor(btnCart);
  g_signal_connect_swapped(
      btnCart, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->navigateTo("customer_cart"); }),
      shell);

  gtk_box_append(GTK_BOX(actionsBox), btnShop);
  gtk_box_append(GTK_BOX(actionsBox), btnCart);
  gtk_box_append(GTK_BOX(content), actionsBox);

  // Lower Content Block (Đơn hàng gần đây + Sản phẩm nổi bật)
  GtkWidget *recentOrdersCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(recentOrdersCard, "card");

  GtkWidget *orderHeaderRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  GtkWidget *lblOrderHeader = gtk_label_new("📦 Đơn hàng gần đây");
  gtk_widget_add_css_class(lblOrderHeader, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblOrderHeader), 0.0f);
  gtk_widget_set_hexpand(lblOrderHeader, TRUE);

  gtk_box_append(GTK_BOX(orderHeaderRow), lblOrderHeader);

  GtkWidget *btnAllOrders = gtk_button_new_with_label("Xem tất cả đơn hàng ›");
  gtk_widget_add_css_class(btnAllOrders, "btn-outline");
  UIComponents::setButtonCursor(btnAllOrders);
  g_signal_connect_swapped(
      btnAllOrders, "clicked",
      G_CALLBACK(+[](CustomerScreens *s) {
        if (s && s->shell) s->shell->navigateTo("customer_orders");
      }),
      this);
  gtk_box_append(GTK_BOX(orderHeaderRow), btnAllOrders);
  gtk_box_append(GTK_BOX(recentOrdersCard), orderHeaderRow);

  if (myOrders.empty()) {
    GtkWidget *emptyOrders = UIComponents::createEmptyState(
        "", "Chưa có đơn hàng nào",
        "Bạn chưa đặt đơn hàng nào tại cửa hàng. Hãy khám phá và mua sắm điện thoại yêu thích ngay!",
        "Mua sắm ngay",
        G_CALLBACK(+[](GtkButton *, gpointer d) {
          CustomerScreens *s = static_cast<CustomerScreens *>(d);
          if (s && s->shell) s->shell->navigateTo("customer_products");
        }),
        this);
    gtk_box_append(GTK_BOX(recentOrdersCard), emptyOrders);
  } else {
    GtkWidget *orderTable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(orderTable, "table-container");

    size_t count = std::min<size_t>(3, myOrders.size());
    for (size_t i = 0; i < count; ++i) {
      const auto &o = myOrders[i];
      GtkWidget *row = UIComponents::createTableDataRow(i);

      GtkWidget *codeLbl = gtk_label_new(o.id.c_str());
      gtk_widget_add_css_class(codeLbl, "cell-code-ord");

      GtkWidget *timeLbl = gtk_label_new(o.orderTime.c_str());
      gtk_widget_add_css_class(timeLbl, "cell-subtitle");
      gtk_widget_add_css_class(timeLbl, "tabular-num");

      std::string summary = "";
      for (const auto &item : o.details) {
        if (!summary.empty()) summary += ", ";
        summary += item.phoneName + " x" + std::to_string(item.quantity);
      }
      if (summary.empty()) {
        summary = "Đơn hàng " + o.id;
      }
      GtkWidget *summLbl = gtk_label_new(summary.c_str());
      gtk_widget_add_css_class(summLbl, "cell-title");
      gtk_widget_set_hexpand(summLbl, TRUE);
      gtk_label_set_xalign(GTK_LABEL(summLbl), 0.0f);
      gtk_label_set_ellipsize(GTK_LABEL(summLbl), PANGO_ELLIPSIZE_END);

      GtkWidget *statusBadge = UIComponents::createOrderStatusBadge(o.status);

      GtkWidget *totalLbl = gtk_label_new(UIComponents::formatVND(o.total).c_str());
      gtk_widget_add_css_class(totalLbl, "cell-title");
      gtk_widget_add_css_class(totalLbl, "stat-trend-positive");

      gtk_box_append(GTK_BOX(row), codeLbl);
      gtk_box_append(GTK_BOX(row), timeLbl);
      gtk_box_append(GTK_BOX(row), summLbl);
      gtk_box_append(GTK_BOX(row), statusBadge);
      gtk_box_append(GTK_BOX(row), totalLbl);
      gtk_box_append(GTK_BOX(orderTable), row);
    }
    gtk_box_append(GTK_BOX(recentOrdersCard), orderTable);
  }
  gtk_box_append(GTK_BOX(content), recentOrdersCard);

  // Featured Products Card
  std::vector<ProductItemViewModel> products = controller->getProducts();
  if (!products.empty()) {
    GtkWidget *featCard =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
    gtk_widget_add_css_class(featCard, "card");

    GtkWidget *featHeaderRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *lblFeatHeader = gtk_label_new("📱 Sản phẩm mới về & Nổi bật");
    gtk_widget_add_css_class(lblFeatHeader, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblFeatHeader), 0.0f);
    gtk_widget_set_hexpand(lblFeatHeader, TRUE);

    gtk_box_append(GTK_BOX(featHeaderRow), lblFeatHeader);

    GtkWidget *btnAllProds = gtk_button_new_with_label("Xem tất cả sản phẩm ›");
    gtk_widget_add_css_class(btnAllProds, "btn-outline");
    UIComponents::setButtonCursor(btnAllProds);
    g_signal_connect_swapped(
        btnAllProds, "clicked",
        G_CALLBACK(+[](CustomerScreens *s) {
          if (s && s->shell) s->shell->navigateTo("customer_products");
        }),
        this);
    gtk_box_append(GTK_BOX(featHeaderRow), btnAllProds);
    gtk_box_append(GTK_BOX(featCard), featHeaderRow);

    GtkWidget *featGrid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(featGrid), DesignTokens::SPACE_MD);
    gtk_grid_set_row_spacing(GTK_GRID(featGrid), DesignTokens::SPACE_MD);
    gtk_grid_set_column_homogeneous(GTK_GRID(featGrid), TRUE);

    size_t prodLimit = std::min<size_t>(4, products.size());
    for (size_t i = 0; i < prodLimit; ++i) {
      const auto &p = products[i];
      GtkWidget *pCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
      gtk_widget_add_css_class(pCard, "product-card");

      std::string resolvedImg = PathHelper::resolveDataPath(p.imageUrl);
      GtkWidget *imgThumb = nullptr;
      if (g_file_test(resolvedImg.c_str(), G_FILE_TEST_EXISTS)) {
        imgThumb = gtk_image_new_from_file(resolvedImg.c_str());
        gtk_image_set_pixel_size(GTK_IMAGE(imgThumb), 90);
      } else {
        imgThumb = gtk_label_new("📱");
        gtk_widget_add_css_class(imgThumb, "page-title");
      }
      gtk_widget_set_halign(imgThumb, GTK_ALIGN_CENTER);
      gtk_box_append(GTK_BOX(pCard), imgThumb);

      GtkWidget *pName = gtk_label_new(p.name.c_str());
      gtk_widget_add_css_class(pName, "product-name");
      gtk_label_set_ellipsize(GTK_LABEL(pName), PANGO_ELLIPSIZE_END);
      gtk_label_set_max_width_chars(GTK_LABEL(pName), 18);
      gtk_label_set_xalign(GTK_LABEL(pName), 0.0f);
      gtk_box_append(GTK_BOX(pCard), pName);

      GtkWidget *pPrice = gtk_label_new(UIComponents::formatVND(p.sellingPrice).c_str());
      gtk_widget_add_css_class(pPrice, "product-price");
      gtk_label_set_xalign(GTK_LABEL(pPrice), 0.0f);
      gtk_box_append(GTK_BOX(pCard), pPrice);

      GtkWidget *btnView = gtk_button_new_with_label("Xem chi tiết");
      gtk_widget_add_css_class(btnView, "btn-outline");
      UIComponents::setButtonCursor(btnView);
      g_signal_connect_swapped(
          btnView, "clicked",
          G_CALLBACK(+[](CustomerScreens *s) {
            if (s && s->shell) s->shell->navigateTo("customer_products");
          }),
          this);
      gtk_box_append(GTK_BOX(pCard), btnView);

      gtk_grid_attach(GTK_GRID(featGrid), pCard, static_cast<int>(i), 0, 1, 1);
    }
    gtk_box_append(GTK_BOX(featCard), featGrid);
    gtk_box_append(GTK_BOX(content), featCard);
  }

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(dashboardView), content);
}

void CustomerScreens::refreshDashboard() { buildDashboardView(); }

// ================= 2. DANH SÁCH SẢN PHẨM =================

void CustomerScreens::buildProductsView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  // Header
  GtkWidget *header = UIComponents::createSectionHeader(
      "Danh mục sản phẩm",
      "Khám phá các dòng điện thoại thông minh chính hãng với giá tốt nhất.");
  gtk_box_append(GTK_BOX(content), header);

  // Bộ lọc bổ sung bên trong thanh tìm kiếm: Tình trạng & Sắp xếp
  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  const char *stockOptions[] = {"Tất cả tình trạng", "Còn hàng", "Sắp hết hàng",
                                "Hết hàng", nullptr};
  GtkWidget *dropStock = gtk_drop_down_new_from_strings(stockOptions);
  auto onStockChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Tất cả", "Còn hàng", "Sắp hết hàng", "Hết hàng"};
    if (idx < 4) {
      self->currentStockFilter = opts[idx];
      self->refreshProducts();
    }
  };
  g_signal_connect(dropStock, "notify::selected", G_CALLBACK(onStockChanged), this);
  gtk_box_append(GTK_BOX(extraControls), dropStock);

  const char *sortOptions[] = {"Mặc định (Mới nhất)", "Giá: Thấp đến cao",
                               "Giá: Cao đến thấp", "Tên: A-Z",
                               "Tồn kho: Nhiều nhất", nullptr};
  GtkWidget *dropSort = gtk_drop_down_new_from_strings(sortOptions);
  auto onSortChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Mặc định", "price_asc", "price_desc", "name_asc",
                          "stock_desc"};
    if (idx < 5) {
      self->currentSortBy = opts[idx];
      self->refreshProducts();
    }
  };
  g_signal_connect(dropSort, "notify::selected", G_CALLBACK(onSortChanged), this);
  gtk_box_append(GTK_BOX(extraControls), dropSort);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo tên điện thoại, thương hiệu hoặc mã...",
      G_CALLBACK(+[](GtkEditable *entry, gpointer data) {
        CustomerScreens *self = static_cast<CustomerScreens *>(data);
        self->currentSearchKeyword = gtk_editable_get_text(entry);
        self->refreshProducts();
      }),
      this, nullptr, &productsCountBadge, extraControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  // Thanh nhãn hãng ĐẶT DƯỚI thanh tìm kiếm
  brandButtonsBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  const char *brands[] = {"Tất cả", "Apple", "Samsung", "Oppo", "Xiaomi"};
  for (const char *b : brands) {
    GtkWidget *btnB = gtk_button_new_with_label(b);
    gtk_widget_add_css_class(btnB, currentBrandFilter == b ? "btn-primary"
                                                           : "btn-outline");
    UIComponents::setButtonCursor(btnB);
    g_signal_connect_data(
        btnB, "clicked", G_CALLBACK(+[](GtkButton *btn, gpointer data) {
          CustomerScreens *self = static_cast<CustomerScreens *>(data);
          self->currentBrandFilter = gtk_button_get_label(btn);
          self->refreshProducts();
        }),
        this, nullptr, (GConnectFlags)0);
    gtk_box_append(GTK_BOX(brandButtonsBox), btnB);
  }
  gtk_box_append(GTK_BOX(content), brandButtonsBox);

  // Dynamic Products List Container
  productsListBox =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_box_append(GTK_BOX(content), productsListBox);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(productsView), content);
  refreshProducts();
}

void CustomerScreens::refreshProducts() {
  // Cập nhật trạng thái nút thương hiệu đang chọn
  if (brandButtonsBox) {
    GtkWidget *btn = gtk_widget_get_first_child(brandButtonsBox);
    while (btn != nullptr) {
      if (GTK_IS_BUTTON(btn)) {
        const char *lbl = gtk_button_get_label(GTK_BUTTON(btn));
        bool isActive = (lbl && currentBrandFilter == lbl);
        gtk_widget_remove_css_class(btn,
                                    isActive ? "btn-outline" : "btn-primary");
        gtk_widget_add_css_class(btn, isActive ? "btn-primary" : "btn-outline");
      }
      btn = gtk_widget_get_next_sibling(btn);
    }
  }

  // Xóa danh sách cũ
  GtkWidget *child = gtk_widget_get_first_child(productsListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(productsListBox), child);
    child = next;
  }

  ProductFilterRequest req;
  req.keyword = currentSearchKeyword;
  req.brand = currentBrandFilter;
  req.sortBy = currentSortBy;

  std::vector<ProductItemViewModel> products = controller->getProducts(req);

  // Lọc theo tình trạng tồn kho nếu có chọn
  if (!currentStockFilter.empty() && currentStockFilter != "Tất cả") {
    std::vector<ProductItemViewModel> filtered;
    for (const auto &p : products) {
      if (currentStockFilter == "Còn hàng" && p.status == StockDisplayStatus::IN_STOCK) {
        filtered.push_back(p);
      } else if (currentStockFilter == "Sắp hết hàng" && p.status == StockDisplayStatus::LOW_STOCK) {
        filtered.push_back(p);
      } else if (currentStockFilter == "Hết hàng" && p.status == StockDisplayStatus::OUT_OF_STOCK) {
        filtered.push_back(p);
      }
    }
    products = std::move(filtered);
  }

  // Sắp xếp bổ sung
  if (currentSortBy == "price_asc") {
    std::sort(products.begin(), products.end(), [](const auto &a, const auto &b) {
      return a.sellingPrice < b.sellingPrice;
    });
  } else if (currentSortBy == "price_desc") {
    std::sort(products.begin(), products.end(), [](const auto &a, const auto &b) {
      return a.sellingPrice > b.sellingPrice;
    });
  } else if (currentSortBy == "name_asc") {
    std::sort(products.begin(), products.end(), [](const auto &a, const auto &b) {
      return a.name < b.name;
    });
  } else if (currentSortBy == "stock_desc") {
    std::sort(products.begin(), products.end(), [](const auto &a, const auto &b) {
      return a.availableStock > b.availableStock;
    });
  }

  UIComponents::updateRecordCountBadge(productsCountBadge, products.size(),
                                       "sản phẩm");

  if (products.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "🔍", "Không tìm thấy sản phẩm phù hợp",
        "Thử tìm kiếm với từ khóa khác hoặc xóa bộ lọc thương hiệu.",
        "Xóa bộ lọc", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CustomerScreens *self = static_cast<CustomerScreens *>(d);
          self->currentSearchKeyword = "";
          self->currentBrandFilter = "Tất cả";
          self->refreshProducts();
        }),
        this);
    gtk_box_append(GTK_BOX(productsListBox), empty);
    return;
  }

  for (const auto &p : products) {
    GtkWidget *card =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_LG);
    gtk_widget_add_css_class(card, "product-card");

    // Thumbnail or Icon Phone
    GtkWidget *imgThumb = nullptr;
    std::string resolvedImg = PathHelper::resolveDataPath(p.imageUrl);
    if (!p.imageUrl.empty() &&
        g_file_test(resolvedImg.c_str(), G_FILE_TEST_EXISTS)) {
      imgThumb = gtk_image_new_from_file(resolvedImg.c_str());
      gtk_image_set_pixel_size(GTK_IMAGE(imgThumb), 56);
    } else {
      imgThumb = gtk_label_new("📱");
      gtk_widget_set_size_request(imgThumb, 56, 56);
    }
    gtk_box_append(GTK_BOX(card), imgThumb);

    // Info box
    GtkWidget *infoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(infoBox, TRUE);

    GtkWidget *topRow =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
    GtkWidget *lblName = gtk_label_new(p.name.c_str());
    gtk_widget_add_css_class(lblName, "product-name");
    gtk_label_set_xalign(GTK_LABEL(lblName), 0.0f);

    GtkWidget *badgeBrand =
        UIComponents::createStatusBadge(p.brand, "role-pill-sales");
    GtkWidget *badgeStock = UIComponents::createStockStatusBadge(p.status);

    gtk_box_append(GTK_BOX(topRow), lblName);
    gtk_box_append(GTK_BOX(topRow), badgeBrand);
    gtk_box_append(GTK_BOX(topRow), badgeStock);
    gtk_box_append(GTK_BOX(infoBox), topRow);

    GtkWidget *lblSpecs = gtk_label_new(p.specs.c_str());
    gtk_widget_add_css_class(lblSpecs, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblSpecs), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(lblSpecs), TRUE);
    gtk_box_append(GTK_BOX(infoBox), lblSpecs);

    const char *availClass = (p.availableStock <= 0)   ? "stock-critical"
                             : (p.availableStock <= 5) ? "stock-low"
                                                       : "stock-ok";
    GtkWidget *lblAvail =
        gtk_label_new(("Số lượng có thể bán: " +
                       UIComponents::formatNumber(p.availableStock) + " máy")
                          .c_str());
    gtk_widget_add_css_class(lblAvail, availClass);
    gtk_widget_add_css_class(lblAvail, "tabular-num");
    gtk_label_set_xalign(GTK_LABEL(lblAvail), 0.0f);
    gtk_box_append(GTK_BOX(infoBox), lblAvail);

    gtk_box_append(GTK_BOX(card), infoBox);

    // Price & Actions box
    GtkWidget *actionBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_valign(actionBox, GTK_ALIGN_CENTER);

    GtkWidget *lblPrice =
        gtk_label_new(UIComponents::formatVND(p.sellingPrice).c_str());
    gtk_widget_add_css_class(lblPrice, "product-price");
    gtk_widget_add_css_class(lblPrice, "tabular-num");
    gtk_label_set_xalign(GTK_LABEL(lblPrice), 1.0f);
    gtk_box_append(GTK_BOX(actionBox), lblPrice);

    // Hành động: 1 nút duy nhất "🛍️ Mua hàng" (thay cho cả nút Thêm giỏ và Chi tiết cũ)
    GtkWidget *btnBuy = gtk_button_new_with_label("🛍️ Mua hàng");
    gtk_widget_add_css_class(btnBuy, "btn-primary");
    UIComponents::setButtonCursor(btnBuy);

    AddToCartData *dData = new AddToCartData{this, p.id};
    g_signal_connect_data(
        btnBuy, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          AddToCartData *ad = static_cast<AddToCartData *>(d);
          ProductItemViewModel prod =
              ad->screens->controller->getProductById(ad->phoneId);
          ad->screens->showProductDetailModal(prod);
        }),
        dData,
        [](gpointer d, GClosure *) { delete static_cast<AddToCartData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actionBox), btnBuy);
    gtk_box_append(GTK_BOX(card), actionBox);

    gtk_box_append(GTK_BOX(productsListBox), card);
  }
}

void CustomerScreens::showProductDetailModal(
    const ProductItemViewModel &product) {
  if (!shell)
    return;

  // Thành phần dùng chung ProductDetailView cung cấp các nút: 🛒 Thêm vào giỏ và ⚡ Mua ngay
  ProductDetailView::Callbacks callbacks;
  callbacks.onBrandSelected = [this](const std::string &brand) {
    currentBrandFilter = brand.empty() ? "Tất cả" : brand;
    refreshProducts();
    if (shell) shell->navigateTo("customer_products");
  };
  callbacks.onNavigateHome = [this]() {
    if (shell) shell->navigateTo("customer_dashboard");
  };
  callbacks.onProductSelected = [this](const std::string &productId) {
    if (controller) {
      ProductItemViewModel nextProd = controller->getProductById(productId);
      showProductDetailModal(nextProd);
    }
  };

  GtkWidget *detailView = ProductDetailView::create(
      controller, shell, product, ProductDetailMode::Customer, false, callbacks);

  shell->pushPage("product_detail", detailView, product.name);
}

// ================= 3. GIỎ HÀNG & CHECKOUT =================

void CustomerScreens::buildCartView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Giỏ hàng & Đặt hàng",
      "Kiểm tra sản phẩm, áp dụng mã giảm giá và hoàn tất đơn hàng.");
  gtk_box_append(GTK_BOX(content), header);

  // Grid layout: Left: Cart items table, Right: Checkout Form & Total
  GtkWidget *mainGrid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(mainGrid), DesignTokens::SPACE_XL);
  gtk_grid_set_column_spacing(GTK_GRID(mainGrid), DesignTokens::SPACE_XL);

  // Left Container: Cart Items Box
  cartListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_hexpand(cartListBox, TRUE);
  gtk_grid_attach(GTK_GRID(mainGrid), cartListBox, 0, 0, 1, 1);

  // Right Container: Checkout Card
  GtkWidget *checkoutCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(checkoutCard, "card");
  gtk_widget_set_size_request(checkoutCard, 380, -1);

  GtkWidget *lblCheckoutTitle =
      gtk_label_new("Thông tin thanh toán & Giao hàng");
  gtk_widget_add_css_class(lblCheckoutTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblCheckoutTitle), 0.0f);
  gtk_box_append(GTK_BOX(checkoutCard), lblCheckoutTitle);

  // Coupon box
  GtkWidget *couponBox =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);
  entryCoupon = gtk_entry_new();
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(entryCoupon),
      "Nhập mã voucher (vd: SALE10, Enter để áp dụng)...");
  gtk_widget_set_hexpand(entryCoupon, TRUE);

  GtkWidget *btnApplyCoupon = gtk_button_new_with_label("Áp dụng");
  gtk_widget_add_css_class(btnApplyCoupon, "btn-primary");
  UIComponents::setButtonCursor(btnApplyCoupon);
  auto onApplyCoupon = +[](GtkWidget *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    const char *code = gtk_editable_get_text(GTK_EDITABLE(self->entryCoupon));
    const UserSessionViewModel &cur = self->controller->getCurrentUser();
    OperationResult res =
        self->controller->applyCoupon(cur.userId, code ? code : "");
    if (res.success) {
      self->refreshCart();
      UIComponents::showSuccessDialog(self->shell->getWindow(), "Mã giảm giá",
                                      res.message);
    } else {
      UIComponents::showWarningDialog(self->shell->getWindow(), "Không hợp lệ",
                                      res.message);
    }
  };
  g_signal_connect_data(btnApplyCoupon, "clicked", G_CALLBACK(onApplyCoupon),
                        this, nullptr, (GConnectFlags)0);
  g_signal_connect_data(entryCoupon, "activate", G_CALLBACK(onApplyCoupon),
                        this, nullptr, (GConnectFlags)0);

  GtkWidget *btnBrowseVouchers = gtk_button_new_with_label("🎟️ Chọn mã");
  gtk_widget_add_css_class(btnBrowseVouchers, "btn-outline");
  UIComponents::setButtonCursor(btnBrowseVouchers);
  g_signal_connect_swapped(
      btnBrowseVouchers, "clicked",
      G_CALLBACK(+[](CustomerScreens *s) {
        if (s && s->shell) s->shell->navigateTo("customer_discounts");
      }),
      this);

  gtk_box_append(GTK_BOX(couponBox), entryCoupon);
  gtk_box_append(GTK_BOX(couponBox), btnApplyCoupon);
  gtk_box_append(GTK_BOX(couponBox), btnBrowseVouchers);
  gtk_box_append(GTK_BOX(checkoutCard),
                 UIComponents::createFormField("Mã giảm giá", couponBox));

  // Delivery Method Dropdown
  const char *deliveryOptions[] = {"Nhận tại cửa hàng (54 Nguyễn Lương Bằng)",
                                   "Giao hàng tận nơi", nullptr};
  comboDeliveryMethod = gtk_drop_down_new_from_strings(deliveryOptions);
  gtk_box_append(GTK_BOX(checkoutCard),
                 UIComponents::createFormField("Hình thức nhận hàng",
                                               comboDeliveryMethod));

  // Recipient Info
  const UserSessionViewModel &cur = controller->getCurrentUser();
  entryReceiverName = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryReceiverName), cur.fullName.c_str());
  gtk_box_append(
      GTK_BOX(checkoutCard),
      UIComponents::createFormField("Họ tên người nhận *", entryReceiverName));

  entryReceiverPhone = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryReceiverPhone), cur.phone.c_str());
  gtk_box_append(GTK_BOX(checkoutCard),
                 UIComponents::createFormField("Số điện thoại nhận hàng *",
                                               entryReceiverPhone));

  entryDeliveryAddress = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryDeliveryAddress),
                        cur.address.c_str());
  gtk_entry_set_placeholder_text(GTK_ENTRY(entryDeliveryAddress),
                                 "Địa chỉ giao hàng chi tiết...");
  gtk_box_append(
      GTK_BOX(checkoutCard),
      UIComponents::createFormField("Địa chỉ nhận hàng (nếu giao tận nơi)",
                                    entryDeliveryAddress));

  // Payment Method Dropdown
  const char *paymentOptions[] = {"Tiền mặt khi nhận hàng (COD)",
                                  "Chuyển khoản ngân hàng", nullptr};
  comboPaymentMethod = gtk_drop_down_new_from_strings(paymentOptions);
  gtk_box_append(GTK_BOX(checkoutCard),
                 UIComponents::createFormField("Phương thức thanh toán",
                                               comboPaymentMethod));

  // Dynamic Summary Container
  cartSummaryBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_box_append(GTK_BOX(checkoutCard), cartSummaryBox);

  // Place Order Button
  btnCheckoutAction = gtk_button_new_with_label("🛍️ Xác nhận đặt hàng");
  gtk_widget_add_css_class(btnCheckoutAction, "btn-success");
  UIComponents::setButtonCursor(btnCheckoutAction);
  gtk_widget_set_margin_top(btnCheckoutAction, 10);
  g_signal_connect_data(
      btnCheckoutAction, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        CustomerScreens *self = static_cast<CustomerScreens *>(d);
        const UserSessionViewModel &curUser =
            self->controller->getCurrentUser();

        if (self->selectedCartPhoneIds.empty()) {
          UIComponents::showWarningDialog(self->shell->getWindow(),
                                          "Chưa chọn sản phẩm",
                                          "Vui lòng tick chọn ít nhất một sản phẩm trong giỏ hàng để đặt hàng!");
          return;
        }

        CheckoutRequest req;
        req.customerId = curUser.userId;
        req.selectedPhoneIds.assign(self->selectedCartPhoneIds.begin(),
                                    self->selectedCartPhoneIds.end());
        req.receiverName =
            gtk_editable_get_text(GTK_EDITABLE(self->entryReceiverName));
        req.receiverPhone =
            gtk_editable_get_text(GTK_EDITABLE(self->entryReceiverPhone));
        req.deliveryAddress =
            gtk_editable_get_text(GTK_EDITABLE(self->entryDeliveryAddress));
        req.deliveryMethod = gtk_drop_down_get_selected(
            GTK_DROP_DOWN(self->comboDeliveryMethod));
        req.paymentMethod =
            gtk_drop_down_get_selected(GTK_DROP_DOWN(self->comboPaymentMethod));
        req.discountCode =
            gtk_editable_get_text(GTK_EDITABLE(self->entryCoupon));

        std::string orderId;
        OperationResult res = self->controller->checkout(req, orderId);
        if (res.success) {
          for (const auto &pid : req.selectedPhoneIds) {
            self->selectedCartPhoneIds.erase(pid);
          }
          CartSummaryViewModel updatedCart =
              self->controller->getCart(curUser.userId);
          self->shell->updateCartBadge(updatedCart.totalQuantity);
          self->refreshCart();
          self->refreshOrders();
          UIComponents::showSuccessDialog(self->shell->getWindow(),
                                          "Đặt hàng thành công", res.message);
          self->shell->navigateTo("customer_orders");
        } else {
          UIComponents::showErrorDialog(self->shell->getWindow(),
                                        "Lỗi đặt hàng", res.message);
        }
      }),
      this, nullptr, (GConnectFlags)0);

  gtk_box_append(GTK_BOX(checkoutCard), btnCheckoutAction);

  gtk_grid_attach(GTK_GRID(mainGrid), checkoutCard, 1, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), mainGrid);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(cartView), content);
  refreshCart();
}

void CustomerScreens::refreshCart() {
  // Xóa danh sách items
  GtkWidget *child = gtk_widget_get_first_child(cartListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(cartListBox), child);
    child = next;
  }

  // Xóa summary box
  child = gtk_widget_get_first_child(cartSummaryBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(cartSummaryBox), child);
    child = next;
  }

  const UserSessionViewModel &user = controller->getCurrentUser();
  CartSummaryViewModel cart = controller->getCart(user.userId);

  if (cart.items.empty()) {
    selectedCartPhoneIds.clear();
    GtkWidget *empty = UIComponents::createEmptyState(
        "🛒", "Giỏ hàng của bạn đang trống",
        "Bạn chưa chọn sản phẩm nào. Hãy khám phá và chọn sản phẩm yêu thích "
        "nhé!",
        "Mua sắm ngay", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CustomerScreens *self = static_cast<CustomerScreens *>(d);
          self->shell->navigateTo("customer_products");
        }),
        this);
    gtk_box_append(GTK_BOX(cartListBox), empty);

    // Hiển thị 0 ₫ trong summary
    GtkWidget *lblZero = gtk_label_new("Tổng thanh toán: 0 ₫");
    gtk_widget_add_css_class(lblZero, "stat-label");
    gtk_widget_add_css_class(lblZero, "tabular-num");
    gtk_box_append(GTK_BOX(cartSummaryBox), lblZero);

    if (btnCheckoutAction) {
      gtk_button_set_label(GTK_BUTTON(btnCheckoutAction), "🛍️ Xác nhận đặt hàng (0)");
      gtk_widget_set_sensitive(btnCheckoutAction, FALSE);
    }
    return;
  }

  // Quản lý trạng thái chọn: Mặc định lần đầu chọn tất cả các sản phẩm
  if (!cartSelectionInitialized) {
    selectedCartPhoneIds.clear();
    for (const auto &it : cart.items) {
      selectedCartPhoneIds.insert(it.phoneId);
    }
    cartSelectionInitialized = true;
  } else {
    // Dọn dẹp các ID sản phẩm không còn tồn tại trong giỏ
    std::set<std::string> curCartIds;
    for (const auto &it : cart.items) curCartIds.insert(it.phoneId);
    for (auto it = selectedCartPhoneIds.begin(); it != selectedCartPhoneIds.end(); ) {
      if (curCartIds.count(*it) == 0) it = selectedCartPhoneIds.erase(it);
      else ++it;
    }
  }

  // Card bao danh sách các mặt hàng
  GtkWidget *itemsCard =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_widget_add_css_class(itemsCard, "card");

  GtkWidget *cardHeaderRow =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_MD);
  GtkWidget *lblTableTitle = gtk_label_new("Danh sách sản phẩm trong giỏ");
  gtk_widget_add_css_class(lblTableTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblTableTitle), 0.0f);
  gtk_widget_set_hexpand(lblTableTitle, TRUE);

  // Checkbox "Chọn tất cả"
  GtkWidget *chkSelectAll = gtk_check_button_new_with_label("Chọn tất cả");
  bool allSelected = (!cart.items.empty() && selectedCartPhoneIds.size() == cart.items.size());
  gtk_check_button_set_active(GTK_CHECK_BUTTON(chkSelectAll), allSelected ? TRUE : FALSE);
  UIComponents::setButtonCursor(chkSelectAll);

  g_signal_connect(
      chkSelectAll, "toggled",
      G_CALLBACK(+[](GtkCheckButton *chk, gpointer d) {
        CustomerScreens *self = static_cast<CustomerScreens *>(d);
        gboolean act = gtk_check_button_get_active(chk);
        const UserSessionViewModel &u = self->controller->getCurrentUser();
        CartSummaryViewModel c = self->controller->getCart(u.userId);
        self->selectedCartPhoneIds.clear();
        if (act) {
          for (const auto &it : c.items) {
            self->selectedCartPhoneIds.insert(it.phoneId);
          }
        }
        self->refreshCart();
      }),
      this);

  GtkWidget *countBadge = gtk_label_new(
      (UIComponents::formatNumber(static_cast<long long>(cart.items.size())) +
       " mẫu (" + UIComponents::formatNumber(cart.totalQuantity) + " máy)")
          .c_str());
  gtk_widget_add_css_class(countBadge, "record-count-badge");
  gtk_widget_set_valign(countBadge, GTK_ALIGN_CENTER);

  gtk_box_append(GTK_BOX(cardHeaderRow), lblTableTitle);
  gtk_box_append(GTK_BOX(cardHeaderRow), chkSelectAll);
  gtk_box_append(GTK_BOX(cardHeaderRow), countBadge);
  gtk_box_append(GTK_BOX(itemsCard), cardHeaderRow);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kCustomerCartCols));

  for (size_t i = 0; i < cart.items.size(); ++i) {
    const auto &item = cart.items[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Checkbox chọn sản phẩm
    GtkWidget *chkBox = gtk_check_button_new();
    gtk_widget_set_halign(chkBox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(chkBox, GTK_ALIGN_CENTER);
    bool isChecked = (selectedCartPhoneIds.count(item.phoneId) > 0);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(chkBox), isChecked ? TRUE : FALSE);
    UIComponents::setButtonCursor(chkBox);

    struct CartItemToggleData {
      CustomerScreens *screens;
      std::string phoneId;
    };
    CartItemToggleData *toggleData = new CartItemToggleData{this, item.phoneId};
    g_signal_connect_data(
        chkBox, "toggled",
        G_CALLBACK(+[](GtkCheckButton *b, gpointer d) {
          CartItemToggleData *cd = static_cast<CartItemToggleData *>(d);
          gboolean act = gtk_check_button_get_active(b);
          if (act) {
            cd->screens->selectedCartPhoneIds.insert(cd->phoneId);
          } else {
            cd->screens->selectedCartPhoneIds.erase(cd->phoneId);
          }
          cd->screens->refreshCart();
        }),
        toggleData,
        [](gpointer d, GClosure *) { delete static_cast<CartItemToggleData *>(d); },
        (GConnectFlags)0);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     chkBox, kCustomerCartCols[0]));

    // 1: Tên sản phẩm & Đơn giá
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       item.phoneName,
                       "Đơn giá: " + UIComponents::formatVND(item.unitPrice),
                       kCustomerCartCols[1]));

    // 2: Quantity controls
    GtkWidget *qtyBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_valign(qtyBox, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(qtyBox, GTK_ALIGN_CENTER);

    GtkWidget *btnMinus = gtk_button_new_with_label("−");
    gtk_widget_add_css_class(btnMinus, "btn-outline");
    gtk_widget_add_css_class(btnMinus, "btn-table-action");
    UIComponents::setButtonCursor(btnMinus);
    CartQtyData *mData = new CartQtyData{this, item.phoneId, item.quantity - 1};
    g_signal_connect_data(
        btnMinus, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CartQtyData *cd = static_cast<CartQtyData *>(d);
          const UserSessionViewModel &cur =
              cd->screens->controller->getCurrentUser();
          cd->screens->controller->updateCartQuantity(cur.userId, cd->phoneId,
                                                      cd->newQty);
          CartSummaryViewModel c = cd->screens->controller->getCart(cur.userId);
          cd->screens->shell->updateCartBadge(c.totalQuantity);
          cd->screens->refreshCart();
        }),
        mData,
        [](gpointer d, GClosure *) { delete static_cast<CartQtyData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(qtyBox), btnMinus);

    GtkWidget *lblQty =
        gtk_label_new(UIComponents::formatNumber(item.quantity).c_str());
    gtk_widget_add_css_class(lblQty, "cell-number");
    gtk_widget_set_size_request(lblQty, 30, -1);
    gtk_box_append(GTK_BOX(qtyBox), lblQty);

    GtkWidget *btnPlus = gtk_button_new_with_label("+");
    gtk_widget_add_css_class(btnPlus, "btn-outline");
    gtk_widget_add_css_class(btnPlus, "btn-table-action");
    UIComponents::setButtonCursor(btnPlus);
    CartQtyData *pData = new CartQtyData{this, item.phoneId, item.quantity + 1};
    g_signal_connect_data(
        btnPlus, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CartQtyData *cd = static_cast<CartQtyData *>(d);
          const UserSessionViewModel &cur =
              cd->screens->controller->getCurrentUser();
          OperationResult res = cd->screens->controller->updateCartQuantity(
              cur.userId, cd->phoneId, cd->newQty);
          if (!res.success) {
            UIComponents::showWarningDialog(cd->screens->shell->getWindow(),
                                            "Tồn kho", res.message);
          }
          CartSummaryViewModel c = cd->screens->controller->getCart(cur.userId);
          cd->screens->shell->updateCartBadge(c.totalQuantity);
          cd->screens->refreshCart();
        }),
        pData,
        [](gpointer d, GClosure *) { delete static_cast<CartQtyData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(qtyBox), btnPlus);

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     qtyBox, kCustomerCartCols[2]));

    // 3: Subtotal (căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(item.subtotal),
                                     kCustomerCartCols[3], "product-price"));

    // 4: Remove Button
    GtkWidget *btnDel = UIComponents::createTableActionButton(
        "✕ Xóa", "Xóa sản phẩm khỏi giỏ hàng", "btn-action-delete");
    CartQtyData *delData = new CartQtyData{this, item.phoneId, 0};
    g_signal_connect_data(
        btnDel, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CartQtyData *cd = static_cast<CartQtyData *>(d);
          const UserSessionViewModel &cur =
              cd->screens->controller->getCurrentUser();
          cd->screens->controller->removeFromCart(cur.userId, cd->phoneId);
          cd->screens->selectedCartPhoneIds.erase(cd->phoneId);
          CartSummaryViewModel c = cd->screens->controller->getCart(cur.userId);
          cd->screens->shell->updateCartBadge(c.totalQuantity);
          cd->screens->refreshCart();
        }),
        delData,
        [](gpointer d, GClosure *) { delete static_cast<CartQtyData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     btnDel, kCustomerCartCols[4]));

    gtk_box_append(GTK_BOX(tableContainer), row);
  }
  gtk_box_append(GTK_BOX(itemsCard), tableContainer);
  gtk_box_append(GTK_BOX(cartListBox), itemsCard);

  // Tính toán tóm tắt cho CÁC SẢN PHẨM ĐÃ TICK CHỌN
  int selectedCount = 0;
  int selectedQty = 0;
  double selectedSubtotal = 0.0;
  for (const auto &it : cart.items) {
    if (selectedCartPhoneIds.count(it.phoneId) > 0) {
      selectedCount++;
      selectedQty += it.quantity;
      selectedSubtotal += it.subtotal;
    }
  }

  double applicableDiscount = 0.0;
  if (!cart.appliedDiscountCode.empty() && selectedSubtotal > 0.0) {
    controller->validateDiscount(cart.appliedDiscountCode, selectedSubtotal, applicableDiscount);
  }
  double selectedGrandTotal = std::max(0.0, selectedSubtotal - applicableDiscount);

  // Summary Breakdown
  auto addSummaryRow = [this](const char *label, const std::string &val,
                              bool isDiscount = false) {
    GtkWidget *r = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *l = gtk_label_new(label);
    gtk_widget_set_hexpand(l, TRUE);
    gtk_label_set_xalign(GTK_LABEL(l), 0.0f);
    gtk_widget_add_css_class(l, "stat-label");

    GtkWidget *v = gtk_label_new(val.c_str());
    gtk_label_set_xalign(GTK_LABEL(v), 1.0f);
    gtk_widget_add_css_class(v, "tabular-num");
    gtk_widget_add_css_class(v, isDiscount ? "stock-ok" : "stat-label");

    gtk_box_append(GTK_BOX(r), l);
    gtk_box_append(GTK_BOX(r), v);
    gtk_box_append(GTK_BOX(this->cartSummaryBox), r);
  };

  addSummaryRow("Sản phẩm đã chọn:",
                std::to_string(selectedCount) + " / " + std::to_string(cart.items.size()) +
                " mặt hàng (" + std::to_string(selectedQty) + " máy)");
  addSummaryRow("Tạm tính:", UIComponents::formatVND(selectedSubtotal));
  if (applicableDiscount > 0) {
    addSummaryRow("Giảm giá (Voucher):",
                  "-" + UIComponents::formatVND(applicableDiscount), true);
  }

  GtkWidget *totalBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_add_css_class(totalBox, "pos-total-box");
  GtkWidget *lblGrand = gtk_label_new("Tổng thanh toán:");
  gtk_widget_add_css_class(lblGrand, "pos-total-label");
  gtk_widget_set_hexpand(lblGrand, TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblGrand), 0.0f);

  GtkWidget *valGrand =
      gtk_label_new(UIComponents::formatVND(selectedGrandTotal).c_str());
  gtk_widget_add_css_class(valGrand, "pos-total-value");
  gtk_widget_add_css_class(valGrand, "tabular-num");
  gtk_label_set_xalign(GTK_LABEL(valGrand), 1.0f);

  gtk_box_append(GTK_BOX(totalBox), lblGrand);
  gtk_box_append(GTK_BOX(totalBox), valGrand);
  gtk_box_append(GTK_BOX(cartSummaryBox), totalBox);

  if (btnCheckoutAction) {
    std::string btnText = "🛍️ Xác nhận đặt hàng (" + std::to_string(selectedCount) + ")";
    gtk_button_set_label(GTK_BUTTON(btnCheckoutAction), btnText.c_str());
    gtk_widget_set_sensitive(btnCheckoutAction, selectedCount > 0);
  }
}

// ================= 4. ĐƠN HÀNG CỦA TÔI =================

void CustomerScreens::buildOrdersView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Đơn hàng của tôi",
      "Theo dõi tình trạng xử lý đơn hàng và lịch sử đặt hàng của bạn.");
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraOrderControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  const char *statusOptions[] = {"Tất cả trạng thái", "Chờ xử lý", "Đã xác nhận",
                                 "Đang giao", "Hoàn tất", "Đã hủy", nullptr};
  GtkWidget *dropStatus = gtk_drop_down_new_from_strings(statusOptions);
  auto onOrderStatusChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Tất cả", "Chờ xử lý", "Đã xác nhận", "Đang giao", "Hoàn tất", "Đã hủy"};
    if (idx < 6) {
      self->ordersFilterStatus = opts[idx];
      self->refreshOrders();
    }
  };
  g_signal_connect(dropStatus, "notify::selected", G_CALLBACK(onOrderStatusChanged), this);
  gtk_box_append(GTK_BOX(extraOrderControls), dropStatus);

  const char *paymentOptions[] = {"Tất cả thanh toán", "Đã thanh toán", "Chưa thanh toán", nullptr};
  GtkWidget *dropPayment = gtk_drop_down_new_from_strings(paymentOptions);
  auto onOrderPaymentChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Tất cả", "Đã thanh toán", "Chưa thanh toán"};
    if (idx < 3) {
      self->ordersFilterPayment = opts[idx];
      self->refreshOrders();
    }
  };
  g_signal_connect(dropPayment, "notify::selected", G_CALLBACK(onOrderPaymentChanged), this);
  gtk_box_append(GTK_BOX(extraOrderControls), dropPayment);

  const char *orderSortOptions[] = {"Thời gian: Mới nhất", "Thời gian: Cũ nhất",
                                    "Tổng tiền: Cao đến thấp", "Tổng tiền: Thấp đến cao", nullptr};
  GtkWidget *dropOrderSort = gtk_drop_down_new_from_strings(orderSortOptions);
  auto onOrderSortChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"time_desc", "time_asc", "total_desc", "total_asc"};
    if (idx < 4) {
      self->ordersSortBy = opts[idx];
      self->refreshOrders();
    }
  };
  g_signal_connect(dropOrderSort, "notify::selected", G_CALLBACK(onOrderSortChanged), this);
  gtk_box_append(GTK_BOX(extraOrderControls), dropOrderSort);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã đơn, tên sản phẩm hoặc trạng thái...",
      G_CALLBACK(+[](GtkEditable *entry, gpointer d) {
        CustomerScreens *self = static_cast<CustomerScreens *>(d);
        self->ordersKeyword = gtk_editable_get_text(entry);
        self->refreshOrders();
      }),
      this, nullptr, &ordersCountBadge, extraOrderControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kCustomerOrderCols));

  ordersListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), ordersListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(ordersView), content);
  refreshOrders();
}

void CustomerScreens::refreshOrders() {
  GtkWidget *child = gtk_widget_get_first_child(ordersListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(ordersListBox), child);
    child = next;
  }

  const UserSessionViewModel &user = controller->getCurrentUser();
  std::vector<OrderItemViewModel> allOrders =
      controller->getCustomerOrders(user.userId);
  std::vector<OrderItemViewModel> orders;

  for (const auto &ord : allOrders) {
    if (!ordersKeyword.empty()) {
      bool match =
          Utf8Helper::containsCaseInsensitive(ord.id, ordersKeyword) ||
          Utf8Helper::containsCaseInsensitive(ord.statusText, ordersKeyword) ||
          Utf8Helper::containsCaseInsensitive(ord.paymentStatusText,
                                              ordersKeyword) ||
          Utf8Helper::containsCaseInsensitive(ord.deliveryMethodText,
                                              ordersKeyword);
      if (!match) {
        for (const auto &d : ord.details) {
          if (Utf8Helper::containsCaseInsensitive(d.phoneName, ordersKeyword)) {
            match = true;
            break;
          }
        }
      }
      if (!match)
        continue;
    }

    // Lọc theo trạng thái đơn hàng
    if (!ordersFilterStatus.empty() && ordersFilterStatus != "Tất cả") {
      if (ord.statusText != ordersFilterStatus)
        continue;
    }

    // Lọc theo trạng thái thanh toán
    if (!ordersFilterPayment.empty() && ordersFilterPayment != "Tất cả") {
      if (ord.paymentStatusText != ordersFilterPayment)
        continue;
    }

    orders.push_back(ord);
  }

  // Sắp xếp đơn hàng
  if (ordersSortBy == "time_asc") {
    std::reverse(orders.begin(), orders.end());
  } else if (ordersSortBy == "total_desc") {
    std::sort(orders.begin(), orders.end(), [](const auto &a, const auto &b) {
      return a.total > b.total;
    });
  } else if (ordersSortBy == "total_asc") {
    std::sort(orders.begin(), orders.end(), [](const auto &a, const auto &b) {
      return a.total < b.total;
    });
  }

  UIComponents::updateRecordCountBadge(ordersCountBadge, orders.size(),
                                       "đơn hàng");

  if (orders.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "📦",
        ordersKeyword.empty() ? "Chưa có đơn hàng nào"
                              : "Không tìm thấy đơn hàng phù hợp",
        ordersKeyword.empty() ? "Bạn chưa đặt mua đơn hàng nào. Hãy chọn những "
                                "sản phẩm ưng ý tại cửa hàng nhé!"
                              : "Thử tìm kiếm với từ khóa hoặc mã đơn khác.",
        "Khám phá sản phẩm", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CustomerScreens *self = static_cast<CustomerScreens *>(d);
          self->shell->navigateTo("customer_products");
        }),
        this);
    gtk_box_append(GTK_BOX(ordersListBox), empty);
    return;
  }

  for (size_t i = 0; i < orders.size(); ++i) {
    const auto &ord = orders[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã đơn
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       ord.id, kCustomerOrderCols[0], "cell-code-ord"));

    // 1: Trạng thái đơn (Badge)
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableWidgetCell(
                       UIComponents::createOrderStatusBadge(ord.status),
                       kCustomerOrderCols[1]));

    // 2: Thanh toán (Badge)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     UIComponents::createPaymentStatusBadge(
                                         ord.paymentStatus),
                                     kCustomerOrderCols[2]));

    // 3: Thời gian & Hình thức
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTwoLineCell(
                                     ord.orderTime, ord.deliveryMethodText,
                                     kCustomerOrderCols[3]));

    // 4: Danh sách sản phẩm
    std::string itemsSummary = "";
    for (size_t j = 0; j < ord.details.size(); ++j) {
      if (j > 0)
        itemsSummary += ", ";
      itemsSummary += ord.details[j].phoneName + " (x" +
                      std::to_string(ord.details[j].quantity) + ")";
    }
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       itemsSummary, kCustomerOrderCols[4], "cell-title"));

    // 5: Tổng tiền (căn phải)
    gtk_box_append(GTK_BOX(row), UIComponents::createTableTextCell(
                                     UIComponents::formatVND(ord.total),
                                     kCustomerOrderCols[5], "product-price"));

    // 6: Actions
    GtkWidget *actionsRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_valign(actionsRow, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(actionsRow, GTK_ALIGN_CENTER);

    GtkWidget *btnDetail = UIComponents::createTableActionButton(
        "🔍 Chi tiết", "Xem thông tin chi tiết các sản phẩm trong đơn",
        "btn-action-view");
    CancelOrderData *dData = new CancelOrderData{this, ord.id};
    g_signal_connect_data(
        btnDetail, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CancelOrderData *cd = static_cast<CancelOrderData *>(d);
          OrderItemViewModel o =
              cd->screens->controller->getOrderById(cd->orderId);
          cd->screens->showOrderDetailModal(o);
        }),
        dData,
        [](gpointer d, GClosure *) {
          delete static_cast<CancelOrderData *>(d);
        },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actionsRow), btnDetail);

    if (ord.canCancel()) {
      GtkWidget *btnCancel = UIComponents::createTableActionButton(
          "✕ Hủy", "Hủy đơn hàng chưa xử lý", "btn-action-delete");
      CancelOrderData *cData = new CancelOrderData{this, ord.id};
      g_signal_connect_data(
          btnCancel, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
            (void)b;
            CancelOrderData *cd = static_cast<CancelOrderData *>(d);
            UIComponents::showConfirmDialog(
                cd->screens->shell->getWindow(), "Xác nhận hủy đơn hàng",
                "Bạn có chắc chắn muốn hủy đơn hàng " + cd->orderId + " không?",
                [](gpointer data) {
                  CancelOrderData *inner = static_cast<CancelOrderData *>(data);
                  OperationResult res =
                      inner->screens->controller->cancelOrder(inner->orderId);
                  if (res.success) {
                    inner->screens->refreshOrders();
                    UIComponents::showSuccessDialog(
                        inner->screens->shell->getWindow(), "Đã hủy đơn",
                        res.message);
                  } else {
                    UIComponents::showErrorDialog(
                        inner->screens->shell->getWindow(), "Lỗi hủy đơn",
                        res.message);
                  }
                },
                cd);
          }),
          cData,
          [](gpointer d, GClosure *) {
            delete static_cast<CancelOrderData *>(d);
          },
          (GConnectFlags)0);
      gtk_box_append(GTK_BOX(actionsRow), btnCancel);
    }

    gtk_box_append(GTK_BOX(row), UIComponents::createTableWidgetCell(
                                     actionsRow, kCustomerOrderCols[6]));
    gtk_box_append(GTK_BOX(ordersListBox), row);
  }
}

void CustomerScreens::showOrderDetailModal(const OrderItemViewModel &order) {
  if (!shell)
    return;

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *pageContent =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(pageContent, "page-container");

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "pdp-card");
  gtk_widget_set_hexpand(card, TRUE);

  GtkWidget *headerRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  GtkWidget *lblTitle =
      gtk_label_new(("📦 Chi tiết đơn hàng " + order.id).c_str());
  gtk_widget_add_css_class(lblTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
  gtk_widget_set_hexpand(lblTitle, TRUE);
  gtk_box_append(GTK_BOX(headerRow), lblTitle);

  gtk_box_append(GTK_BOX(headerRow),
                 UIComponents::createOrderStatusBadge(order.status));
  gtk_box_append(GTK_BOX(headerRow),
                 UIComponents::createPaymentStatusBadge(order.paymentStatus));
  gtk_box_append(GTK_BOX(card), headerRow);

  GtkWidget *sepTop = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(card), sepTop);

  int infoRowIdx = 0;
  auto addRow = [card, &infoRowIdx](const char *label, const std::string &val) {
    GtkWidget *r = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(r, "pdp-spec-row");
    gtk_widget_add_css_class(r, (infoRowIdx % 2 == 0) ? "row-even" : "row-odd");
    infoRowIdx++;

    GtkWidget *l = gtk_label_new(label);
    gtk_widget_add_css_class(l, "stat-label");
    gtk_widget_set_size_request(l, 160, -1);
    gtk_label_set_xalign(GTK_LABEL(l), 0.0f);

    GtkWidget *v = gtk_label_new(val.c_str());
    gtk_label_set_xalign(GTK_LABEL(v), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(v), TRUE);
    gtk_widget_set_hexpand(v, TRUE);

    gtk_box_append(GTK_BOX(r), l);
    gtk_box_append(GTK_BOX(r), v);
    gtk_box_append(GTK_BOX(card), r);
  };

  addRow("Thời gian đặt:", order.orderTime);
  addRow("Trạng thái đơn:", order.statusText);
  addRow("Trạng thái TT:", order.paymentStatusText);
  addRow("Phương thức TT:", order.paymentMethodText);
  addRow("Hình thức nhận:", order.deliveryMethodText);
  addRow("Người nhận:", order.receiverName + " (" + order.receiverPhone + ")");
  addRow("Địa chỉ nhận:", order.deliveryAddress);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(card), sep);

  GtkWidget *lblItemsHeader = gtk_label_new("🛍️ Danh sách mặt hàng:");
  gtk_widget_add_css_class(lblItemsHeader, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblItemsHeader), 0.0f);
  gtk_box_append(GTK_BOX(card), lblItemsHeader);

  for (size_t idx = 0; idx < order.details.size(); ++idx) {
    const auto &d = order.details[idx];
    GtkWidget *iRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(iRow, "pdp-spec-row");
    gtk_widget_add_css_class(iRow, (idx % 2 == 0) ? "row-even" : "row-odd");

    GtkWidget *n = gtk_label_new(d.phoneName.c_str());
    gtk_widget_add_css_class(n, "cell-title");
    gtk_widget_set_hexpand(n, TRUE);
    gtk_label_set_xalign(GTK_LABEL(n), 0.0f);

    GtkWidget *q = gtk_label_new(("x" + std::to_string(d.quantity)).c_str());
    gtk_widget_add_css_class(q, "tabular-num");
    GtkWidget *p = gtk_label_new(UIComponents::formatVND(d.subtotal).c_str());
    gtk_widget_add_css_class(p, "product-price");
    gtk_widget_add_css_class(p, "tabular-num");

    gtk_box_append(GTK_BOX(iRow), n);
    gtk_box_append(GTK_BOX(iRow), q);
    gtk_box_append(GTK_BOX(iRow), p);
    gtk_box_append(GTK_BOX(card), iRow);
  }

  if (order.discountAmount > 0) {
    addRow("Giảm giá:", "-" + UIComponents::formatVND(order.discountAmount));
  }
  addRow("Tổng cộng:", UIComponents::formatVND(order.total));

  GtkWidget *btnClose = gtk_button_new_with_label("← Quay lại danh sách");
  gtk_widget_add_css_class(btnClose, "btn-secondary");
  UIComponents::setButtonCursor(btnClose);
  gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
  g_signal_connect_swapped(
      btnClose, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->popPage(); }), shell);
  gtk_box_append(GTK_BOX(card), btnClose);

  gtk_box_append(GTK_BOX(pageContent), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("customer_order_detail", scroll,
                  "Chi tiết đơn hàng — " + order.id);
}

// ================= 4b. VÍ VOUCHER & MÃ GIẢM GIÁ =================

void CustomerScreens::buildDiscountsView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Ví Voucher & Mã giảm giá",
      "Kho ưu đãi và khuyến mãi hấp dẫn dành riêng cho bạn. Tự động sắp xếp theo thời gian hết hạn gần nhất.");
  gtk_box_append(GTK_BOX(content), header);

  // Vùng bộ lọc & sắp xếp mở rộng
  GtkWidget *extraControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  // 1. Lọc theo hình thức giảm giá
  const char *typeOptions[] = {"Tất cả hình thức", "Giảm theo phần trăm (%)",
                               "Giảm tiền mặt cố định", nullptr};
  GtkWidget *dropType = gtk_drop_down_new_from_strings(typeOptions);
  auto onDiscountTypeChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Tất cả", "percent", "fixed"};
    if (idx < 3) {
      self->discountsFilterType = opts[idx];
      self->refreshDiscounts();
    }
  };
  g_signal_connect(dropType, "notify::selected", G_CALLBACK(onDiscountTypeChanged), this);
  gtk_box_append(GTK_BOX(extraControls), dropType);

  // 2. Lọc theo trạng thái hiệu lực
  const char *statusOptions[] = {"Còn hiệu lực", "Tất cả trạng thái", "Đã hết hạn / Hết lượt", nullptr};
  GtkWidget *dropStatus = gtk_drop_down_new_from_strings(statusOptions);
  auto onDiscountStatusChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"active", "all", "expired"};
    if (idx < 3) {
      self->discountsFilterStatus = opts[idx];
      self->refreshDiscounts();
    }
  };
  g_signal_connect(dropStatus, "notify::selected", G_CALLBACK(onDiscountStatusChanged), this);
  gtk_box_append(GTK_BOX(extraControls), dropStatus);

  // 3. Sắp xếp danh mục (Mặc định: Hết hạn gần nhất trước)
  const char *sortOptions[] = {
      "Hạn dùng: Sắp hết hạn nhất (Mặc định)",
      "Hạn dùng: Còn hạn lâu nhất",
      "Mức giảm: Cao đến thấp",
      "Mức giảm: Thấp đến cao",
      "Mã voucher: A-Z",
      nullptr};
  GtkWidget *dropSort = gtk_drop_down_new_from_strings(sortOptions);
  auto onDiscountSortChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"expiry_asc", "expiry_desc", "value_desc", "value_asc", "code_asc"};
    if (idx < 5) {
      self->discountsSortBy = opts[idx];
      self->refreshDiscounts();
    }
  };
  g_signal_connect(dropSort, "notify::selected", G_CALLBACK(onDiscountSortChanged), this);
  gtk_box_append(GTK_BOX(extraControls), dropSort);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm kiếm theo mã voucher hoặc ưu đãi...",
      G_CALLBACK(+[](GtkEditable *entry, gpointer d) {
        CustomerScreens *self = static_cast<CustomerScreens *>(d);
        self->discountsKeyword = gtk_editable_get_text(entry);
        self->refreshDiscounts();
      }),
      this, nullptr, &discountsCountBadge, extraControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  // Vùng chứa các thẻ Voucher
  discountsListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_MD);
  gtk_box_append(GTK_BOX(content), discountsListBox);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(discountsView), content);
  refreshDiscounts();
}

void CustomerScreens::refreshDiscounts() {
  GtkWidget *child = gtk_widget_get_first_child(discountsListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(discountsListBox), child);
    child = next;
  }

  std::vector<DiscountItemViewModel> allDiscounts = controller->getDiscounts();
  std::vector<DiscountItemViewModel> discounts;
  RealTimeManager &rtm = RealTimeManager::getInstance();

  for (const auto &d : allDiscounts) {
    bool isExpired = rtm.isDateExpired(d.expiryDate) ||
                     (d.maxUsage > 0 && d.usedCount >= d.maxUsage) ||
                     !d.isValid;

    // Lọc theo từ khóa tìm kiếm
    if (!discountsKeyword.empty()) {
      bool match = Utf8Helper::containsCaseInsensitive(d.code, discountsKeyword) ||
                   Utf8Helper::containsCaseInsensitive(d.description, discountsKeyword) ||
                   Utf8Helper::containsCaseInsensitive(d.typeName, discountsKeyword) ||
                   Utf8Helper::containsCaseInsensitive(d.formattedValue, discountsKeyword);
      if (!match) continue;
    }

    // Lọc theo hình thức
    if (discountsFilterType == "percent") {
      if (d.typeName.find("%") == std::string::npos &&
          d.typeName.find("trăm") == std::string::npos)
        continue;
    } else if (discountsFilterType == "fixed") {
      if (d.typeName.find("%") != std::string::npos ||
          d.typeName.find("trăm") != std::string::npos)
        continue;
    }

    // Lọc theo trạng thái
    if (discountsFilterStatus == "active") {
      if (isExpired) continue;
    } else if (discountsFilterStatus == "expired") {
      if (!isExpired) continue;
    }

    discounts.push_back(d);
  }

  // TỰ ĐỘNG SẮP XẾP THEO THỜI GIAN HẾT HẠN (HOẶC TIÊU CHÍ ĐƯỢC CHỌN)
  if (discountsSortBy == "expiry_asc" || discountsSortBy.empty()) {
    // Ưu tiên: Voucher còn hạn xếp trước -> Voucher sắp hết hạn xếp trên cùng
    std::sort(discounts.begin(), discounts.end(), [&](const DiscountItemViewModel &a, const DiscountItemViewModel &b) {
      bool aExpired = rtm.isDateExpired(a.expiryDate) || (a.maxUsage > 0 && a.usedCount >= a.maxUsage) || !a.isValid;
      bool bExpired = rtm.isDateExpired(b.expiryDate) || (b.maxUsage > 0 && b.usedCount >= b.maxUsage) || !b.isValid;
      if (aExpired != bExpired) {
        return !aExpired; // Còn hạn xếp trước
      }
      long long aEpoch = RealTimeManager::dateToEpochDays(a.expiryDate);
      long long bEpoch = RealTimeManager::dateToEpochDays(b.expiryDate);
      if (aEpoch != bEpoch) {
        return aEpoch < bEpoch; // Hạn gần hơn xếp trước
      }
      return a.code < b.code;
    });
  } else if (discountsSortBy == "expiry_desc") {
    // Voucher còn hạn xếp trước -> Hạn xa nhất xếp trước
    std::sort(discounts.begin(), discounts.end(), [&](const DiscountItemViewModel &a, const DiscountItemViewModel &b) {
      bool aExpired = rtm.isDateExpired(a.expiryDate) || (a.maxUsage > 0 && a.usedCount >= a.maxUsage) || !a.isValid;
      bool bExpired = rtm.isDateExpired(b.expiryDate) || (b.maxUsage > 0 && b.usedCount >= b.maxUsage) || !b.isValid;
      if (aExpired != bExpired) {
        return !aExpired;
      }
      long long aEpoch = RealTimeManager::dateToEpochDays(a.expiryDate);
      long long bEpoch = RealTimeManager::dateToEpochDays(b.expiryDate);
      if (aEpoch != bEpoch) {
        return aEpoch > bEpoch; // Hạn xa hơn xếp trước
      }
      return a.code < b.code;
    });
  } else if (discountsSortBy == "value_desc") {
    std::sort(discounts.begin(), discounts.end(), [](const DiscountItemViewModel &a, const DiscountItemViewModel &b) {
      return a.value > b.value;
    });
  } else if (discountsSortBy == "value_asc") {
    std::sort(discounts.begin(), discounts.end(), [](const DiscountItemViewModel &a, const DiscountItemViewModel &b) {
      return a.value < b.value;
    });
  } else if (discountsSortBy == "code_asc") {
    std::sort(discounts.begin(), discounts.end(), [](const DiscountItemViewModel &a, const DiscountItemViewModel &b) {
      return a.code < b.code;
    });
  }

  UIComponents::updateRecordCountBadge(discountsCountBadge, discounts.size(), "voucher");

  if (discounts.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "🎟️",
        discountsKeyword.empty() ? "Hiện chưa có mã giảm giá nào"
                                 : "Không tìm thấy mã giảm giá phù hợp",
        discountsKeyword.empty() ? "Các chương trình khuyến mãi và voucher ưu đãi sẽ được cập nhật thường xuyên tại đây."
                                 : "Thử tìm kiếm với từ khóa khác hoặc chuyển bộ lọc về 'Tất cả trạng thái'.",
        "Tiếp tục mua sắm",
        G_CALLBACK(+[](GtkButton *, gpointer d) {
          CustomerScreens *self = static_cast<CustomerScreens *>(d);
          self->shell->navigateTo("customer_products");
        }),
        this);
    gtk_box_append(GTK_BOX(discountsListBox), empty);
    return;
  }

  for (const auto &disc : discounts) {
    bool isExpired = rtm.isDateExpired(disc.expiryDate) ||
                     (disc.maxUsage > 0 && disc.usedCount >= disc.maxUsage) ||
                     !disc.isValid;
    int remainingDays = rtm.getRemainingDays(disc.expiryDate);
    bool isPercent = (disc.typeName.find("%") != std::string::npos ||
                      disc.typeName.find("trăm") != std::string::npos);

    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(card, "voucher-card");

    // === 1. Visual badge bên trái ===
    GtkWidget *leftBadge = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_size_request(leftBadge, 110, -1);
    gtk_widget_set_valign(leftBadge, GTK_ALIGN_CENTER);

    GtkWidget *valLabel = gtk_label_new(disc.formattedValue.c_str());
    gtk_widget_add_css_class(valLabel, "card-title");
    gtk_widget_add_css_class(valLabel, "tabular-num");

    GtkWidget *typeLabel = gtk_label_new(isPercent ? "GIẢM %" : "GIẢM TIỀN");
    gtk_widget_add_css_class(typeLabel, "caption-text");

    if (isExpired) {
      gtk_widget_add_css_class(leftBadge, "voucher-badge-expired");
    } else if (isPercent) {
      gtk_widget_add_css_class(leftBadge, "voucher-badge-percent");
    } else {
      gtk_widget_add_css_class(leftBadge, "voucher-badge-fixed");
    }

    gtk_box_append(GTK_BOX(leftBadge), valLabel);
    gtk_box_append(GTK_BOX(leftBadge), typeLabel);
    gtk_box_append(GTK_BOX(card), leftBadge);

    // === 2. Thông tin chi tiết ở giữa ===
    GtkWidget *infoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand(infoBox, TRUE);
    gtk_widget_set_valign(infoBox, GTK_ALIGN_CENTER);

    // Dòng 1: Mã voucher + Huy hiệu hạn dùng
    GtkWidget *codeRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lblCode = gtk_label_new(disc.code.c_str());
    gtk_widget_add_css_class(lblCode, "cell-code-ord");
    gtk_box_append(GTK_BOX(codeRow), lblCode);

    GtkWidget *badgeExpiry = nullptr;
    if (isExpired) {
      badgeExpiry = gtk_label_new("❌ Đã hết hạn");
      gtk_widget_add_css_class(badgeExpiry, "status-badge-inactive");
    } else if (remainingDays <= 0) {
      badgeExpiry = gtk_label_new("⚠️ Hết hạn hôm nay");
      gtk_widget_add_css_class(badgeExpiry, "status-badge-warning");
    } else if (remainingDays <= 3) {
      std::string expText = "⚡ Sắp hết hạn (còn " + std::to_string(remainingDays) + " ngày)";
      badgeExpiry = gtk_label_new(expText.c_str());
      gtk_widget_add_css_class(badgeExpiry, "status-badge-warning");
    } else {
      std::string expText = "⏳ Còn " + std::to_string(remainingDays) + " ngày";
      badgeExpiry = gtk_label_new(expText.c_str());
      gtk_widget_add_css_class(badgeExpiry, "status-badge-active");
    }
    gtk_box_append(GTK_BOX(codeRow), badgeExpiry);
    gtk_box_append(GTK_BOX(infoBox), codeRow);

    // Dòng 2: Nội dung mô tả
    GtkWidget *lblDesc = gtk_label_new(disc.description.c_str());
    gtk_label_set_xalign(GTK_LABEL(lblDesc), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(lblDesc), TRUE);
    gtk_widget_add_css_class(lblDesc, "cell-title");
    gtk_box_append(GTK_BOX(infoBox), lblDesc);

    // Dòng 3: Điều kiện áp dụng (Đơn tối thiểu, Giảm tối đa, Hạn dùng, Lượt dùng)
    GtkWidget *metaRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);

    std::string minOrderStr = "Đơn tối thiểu: " + UIComponents::formatVND(disc.minOrderValue);
    GtkWidget *lblMin = gtk_label_new(minOrderStr.c_str());
    gtk_widget_add_css_class(lblMin, "caption-text");
    gtk_box_append(GTK_BOX(metaRow), lblMin);

    if (isPercent && disc.maxDiscountAmount > 0) {
      std::string maxStr = "Giảm tối đa: " + UIComponents::formatVND(disc.maxDiscountAmount);
      GtkWidget *lblMax = gtk_label_new(maxStr.c_str());
      gtk_widget_add_css_class(lblMax, "caption-text");
      gtk_box_append(GTK_BOX(metaRow), lblMax);
    }

    std::string expDateStr = "Hạn dùng: " + disc.expiryDate;
    GtkWidget *lblExp = gtk_label_new(expDateStr.c_str());
    gtk_widget_add_css_class(lblExp, "caption-text");
    gtk_box_append(GTK_BOX(metaRow), lblExp);

    if (disc.maxUsage > 0) {
      std::string usageStr = "Lượt dùng: " + std::to_string(disc.usedCount) + "/" + std::to_string(disc.maxUsage);
      GtkWidget *lblUsage = gtk_label_new(usageStr.c_str());
      gtk_widget_add_css_class(lblUsage, "caption-text");
      gtk_box_append(GTK_BOX(metaRow), lblUsage);
    }

    gtk_box_append(GTK_BOX(infoBox), metaRow);
    gtk_box_append(GTK_BOX(card), infoBox);

    // === 3. Nhóm nút thao tác bên phải ===
    GtkWidget *actionsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_valign(actionsBox, GTK_ALIGN_CENTER);

    GtkWidget *btnUse = gtk_button_new_with_label("🛍️ Dùng ngay");
    UIComponents::setButtonCursor(btnUse);
    if (isExpired) {
      gtk_widget_set_sensitive(btnUse, FALSE);
      gtk_widget_add_css_class(btnUse, "btn-secondary");
    } else {
      gtk_widget_add_css_class(btnUse, "btn-primary");
      VoucherActionData *useData = new VoucherActionData{this, disc.code};
      g_signal_connect_data(
          btnUse, "clicked",
          G_CALLBACK(+[](GtkButton *, gpointer d) {
            VoucherActionData *vd = static_cast<VoucherActionData *>(d);
            CustomerScreens *s = vd->screens;
            std::string c = vd->code;
            if (s->entryCoupon) {
              gtk_editable_set_text(GTK_EDITABLE(s->entryCoupon), c.c_str());
            }
            const UserSessionViewModel &cur = s->controller->getCurrentUser();
            CartSummaryViewModel cart = s->controller->getCart(cur.userId);
            if (!cart.items.empty()) {
              OperationResult res = s->controller->applyCoupon(cur.userId, c);
              s->refreshCart();
              s->shell->navigateTo("customer_cart");
              if (res.success) {
                UIComponents::showToast(s->shell->getWindow(), "Đã áp dụng mã " + c + " vào giỏ hàng!", "success");
              } else {
                UIComponents::showWarningDialog(s->shell->getWindow(), "Mã giảm giá", res.message);
              }
            } else {
              s->shell->navigateTo("customer_products");
              UIComponents::showToast(s->shell->getWindow(), "Đã lưu mã " + c + ". Hãy chọn điện thoại để mua sắm!", "info");
            }
          }),
          useData,
          [](gpointer d, GClosure *) { delete static_cast<VoucherActionData *>(d); },
          (GConnectFlags)0);
    }
    gtk_box_append(GTK_BOX(actionsBox), btnUse);

    GtkWidget *btnCopy = gtk_button_new_with_label("📋 Sao chép mã");
    gtk_widget_add_css_class(btnCopy, "btn-outline");
    UIComponents::setButtonCursor(btnCopy);
    VoucherActionData *copyData = new VoucherActionData{this, disc.code};
    g_signal_connect_data(
        btnCopy, "clicked",
        G_CALLBACK(+[](GtkButton *, gpointer d) {
          VoucherActionData *vd = static_cast<VoucherActionData *>(d);
          CustomerScreens *s = vd->screens;
          std::string c = vd->code;
          GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default());
          gdk_clipboard_set_text(clipboard, c.c_str());
          UIComponents::showToast(s->shell->getWindow(), "Đã sao chép mã '" + c + "' vào bộ nhớ tạm!", "success");
        }),
        copyData,
        [](gpointer d, GClosure *) { delete static_cast<VoucherActionData *>(d); },
        (GConnectFlags)0);
    gtk_box_append(GTK_BOX(actionsBox), btnCopy);

    gtk_box_append(GTK_BOX(card), actionsBox);
    gtk_box_append(GTK_BOX(discountsListBox), card);
  }
}

// ================= 5. HỒ SƠ CÁ NHÂN & ĐIỂM TÍCH LŨY =================

void CustomerScreens::buildProfileView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_XL);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *header = UIComponents::createSectionHeader(
      "Hồ sơ cá nhân",
      "Quản lý thông tin tài khoản, điểm thưởng và hạng thành viên của bạn.");
  gtk_box_append(GTK_BOX(content), header);

  const UserSessionViewModel &user = controller->getCurrentUser();
  CustomerItemViewModel cust = controller->getCustomerById(user.userId);

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), DesignTokens::SPACE_XL);
  gtk_grid_set_column_spacing(GTK_GRID(grid), DesignTokens::SPACE_XL);

  // Left Card: Form edit profile
  GtkWidget *formCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_add_css_class(formCard, "card");
  gtk_widget_set_hexpand(formCard, TRUE);

  GtkWidget *lblFormTitle = gtk_label_new("Chỉnh sửa thông tin liên hệ");
  gtk_widget_add_css_class(lblFormTitle, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblFormTitle), 0.0f);
  gtk_box_append(GTK_BOX(formCard), lblFormTitle);

  GtkWidget *entryName = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryName), user.fullName.c_str());
  gtk_box_append(GTK_BOX(formCard),
                 UIComponents::createFormField("Họ và tên", entryName));

  GtkWidget *entryPhone = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryPhone), user.phone.c_str());
  gtk_box_append(GTK_BOX(formCard),
                 UIComponents::createFormField("Số điện thoại", entryPhone));

  GtkWidget *entryEmail = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryEmail), user.email.c_str());
  gtk_box_append(GTK_BOX(formCard),
                 UIComponents::createFormField("Email", entryEmail));

  GtkWidget *entryAddr = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entryAddr), user.address.c_str());
  gtk_box_append(GTK_BOX(formCard),
                 UIComponents::createFormField("Địa chỉ mặc định", entryAddr));

  GtkWidget *btnSave = gtk_button_new_with_label("💾 Lưu thay đổi hồ sơ");
  gtk_widget_add_css_class(btnSave, "btn-primary");
  UIComponents::setButtonCursor(btnSave);
  gtk_widget_set_margin_top(btnSave, 8);

  struct ProfileSaveContext {
    CustomerScreens *screens;
    GtkWidget *eName;
    GtkWidget *ePhone;
    GtkWidget *eEmail;
    GtkWidget *eAddr;
  };
  ProfileSaveContext *pctx = new ProfileSaveContext{this, entryName, entryPhone,
                                                    entryEmail, entryAddr};
  g_signal_connect_data(
      btnSave, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        ProfileSaveContext *ctx = static_cast<ProfileSaveContext *>(d);
        const char *n = gtk_editable_get_text(GTK_EDITABLE(ctx->eName));
        const char *p = gtk_editable_get_text(GTK_EDITABLE(ctx->ePhone));
        const char *e = gtk_editable_get_text(GTK_EDITABLE(ctx->eEmail));
        const char *a = gtk_editable_get_text(GTK_EDITABLE(ctx->eAddr));

        OperationResult res = ctx->screens->controller->updateProfile(
            n ? n : "", p ? p : "", e ? e : "", a ? a : "");
        if (res.success) {
          UIComponents::showSuccessDialog(ctx->screens->shell->getWindow(),
                                          "Thành công", res.message);
          ctx->screens->refreshProfile();
        } else {
          UIComponents::showErrorDialog(ctx->screens->shell->getWindow(), "Lỗi",
                                        res.message);
        }
      }),
      pctx,
      [](gpointer d, GClosure *) {
        delete static_cast<ProfileSaveContext *>(d);
      },
      (GConnectFlags)0);
  gtk_box_append(GTK_BOX(formCard), btnSave);

  GtkWidget *btnChangePw = gtk_button_new_with_label("🔑 Đổi mật khẩu");
  gtk_widget_add_css_class(btnChangePw, "btn-secondary");
  UIComponents::setButtonCursor(btnChangePw);
  g_signal_connect(btnChangePw, "clicked",
                   G_CALLBACK(+[](GtkButton *, gpointer d) {
                     CustomerScreens *self = static_cast<CustomerScreens *>(d);
                     UIComponents::showChangePasswordDialog(
                         self->shell->getWindow(), self->controller);
                   }),
                   this);
  gtk_box_append(GTK_BOX(formCard), btnChangePw);

  gtk_grid_attach(GTK_GRID(grid), formCard, 0, 0, 1, 1);

  // Right Card: Loyalty Points & Rewards
  GtkWidget *loyaltyCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(loyaltyCard, "card");
  gtk_widget_set_size_request(loyaltyCard, 360, -1);

  GtkWidget *lblLoyalty = gtk_label_new("Chương trình Khách hàng thân thiết");
  gtk_widget_add_css_class(lblLoyalty, "card-title");
  gtk_label_set_xalign(GTK_LABEL(lblLoyalty), 0.0f);
  gtk_box_append(GTK_BOX(loyaltyCard), lblLoyalty);

  // Rank Badge đặc tả màu
  GtkWidget *rankBadge = UIComponents::createRankBadge(cust.membershipTier);
  gtk_box_append(GTK_BOX(loyaltyCard), rankBadge);

  GtkWidget *pointBox = UIComponents::createStatCard(
      "Điểm tích lũy hiện tại",
      UIComponents::formatNumber(cust.points) + " điểm",
      "Hạng hội viên: " + cust.membershipTier, "🌟", "purple");
  gtk_box_append(GTK_BOX(loyaltyCard), pointBox);

  GtkWidget *lblRules = gtk_label_new(
      "Quy tắc tích điểm:\n• Mỗi 100.000 đ thanh toán thành công = 1 điểm tích "
      "lũy\n• Từ 50 điểm: Thăng hạng Bạc\n• Từ 200 điểm: Thăng hạng Vàng\n• Từ "
      "500 điểm: Thăng hạng Kim Cương (VIP)\n• Đổi điểm để nhận voucher ưu đãi "
      "lên đến 15%!");
  gtk_widget_add_css_class(lblRules, "page-subtitle");
  gtk_label_set_wrap(GTK_LABEL(lblRules), TRUE);
  gtk_label_set_xalign(GTK_LABEL(lblRules), 0.0f);
  gtk_box_append(GTK_BOX(loyaltyCard), lblRules);

  gtk_grid_attach(GTK_GRID(grid), loyaltyCard, 1, 0, 1, 1);
  gtk_box_append(GTK_BOX(content), grid);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(profileView), content);
}

void CustomerScreens::refreshProfile() { buildProfileView(); }

// ================= 6. BẢO HÀNH =================

void CustomerScreens::buildWarrantyView() {
  GtkWidget *content =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(content, "page-container");

  GtkWidget *btnNew = gtk_button_new_with_label("➕ Gửi yêu cầu bảo hành mới");
  gtk_widget_add_css_class(btnNew, "btn-primary");
  UIComponents::setButtonCursor(btnNew);
  g_signal_connect_swapped(
      btnNew, "clicked",
      G_CALLBACK(+[](CustomerScreens *s) { s->showCreateWarrantyModal(); }),
      this);

  GtkWidget *header = UIComponents::createSectionHeader(
      "Trung tâm bảo hành",
      "Tra cứu và tạo yêu cầu sửa chữa, bảo hành thiết bị chính hãng.", btnNew);
  gtk_box_append(GTK_BOX(content), header);

  GtkWidget *extraWarrantyControls =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DesignTokens::SPACE_SM);

  const char *statusWOpts[] = {"Tất cả trạng thái", "Chờ tiếp nhận", "Đang xử lý",
                               "Hoàn tất", "Từ chối", nullptr};
  GtkWidget *dropStatusW = gtk_drop_down_new_from_strings(statusWOpts);
  auto onWarrantyStatusChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"Tất cả", "Chờ tiếp nhận", "Đang xử lý", "Hoàn tất", "Từ chối"};
    if (idx < 5) {
      self->warrantyFilterStatus = opts[idx];
      self->refreshWarranty();
    }
  };
  g_signal_connect(dropStatusW, "notify::selected", G_CALLBACK(onWarrantyStatusChanged), this);
  gtk_box_append(GTK_BOX(extraWarrantyControls), dropStatusW);

  const char *sortWOpts[] = {"Thời gian: Mới nhất", "Thời gian: Cũ nhất", "Mã phiếu: A-Z", nullptr};
  GtkWidget *dropSortW = gtk_drop_down_new_from_strings(sortWOpts);
  auto onWarrantySortChanged = +[](GObject *obj, GParamSpec *, gpointer d) {
    CustomerScreens *self = static_cast<CustomerScreens *>(d);
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(obj));
    const char *opts[] = {"date_desc", "date_asc", "id_asc"};
    if (idx < 3) {
      self->warrantySortBy = opts[idx];
      self->refreshWarranty();
    }
  };
  g_signal_connect(dropSortW, "notify::selected", G_CALLBACK(onWarrantySortChanged), this);
  gtk_box_append(GTK_BOX(extraWarrantyControls), dropSortW);

  GtkWidget *filterToolbar = UIComponents::createFilterToolbar(
      "Tìm theo mã phiếu, thiết bị hoặc mô tả lỗi...",
      G_CALLBACK(+[](GtkEditable *entry, gpointer d) {
        CustomerScreens *self = static_cast<CustomerScreens *>(d);
        self->warrantyKeyword = gtk_editable_get_text(entry);
        self->refreshWarranty();
      }),
      this, nullptr, &warrantyCountBadge, extraWarrantyControls);
  gtk_box_append(GTK_BOX(content), filterToolbar);

  GtkWidget *tableContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(tableContainer, "table-container");
  gtk_box_append(GTK_BOX(tableContainer),
                 UIComponents::createTableHeaderRow(kCustomerWarrantyCols));

  warrantyListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(tableContainer), warrantyListBox);
  gtk_box_append(GTK_BOX(content), tableContainer);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(warrantyView), content);
  refreshWarranty();
}

void CustomerScreens::refreshWarranty() {
  GtkWidget *child = gtk_widget_get_first_child(warrantyListBox);
  while (child != nullptr) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_box_remove(GTK_BOX(warrantyListBox), child);
    child = next;
  }

  const UserSessionViewModel &user = controller->getCurrentUser();
  std::vector<WarrantyItemViewModel> allWarranties =
      controller->getCustomerWarranties(user.userId);
  std::vector<WarrantyItemViewModel> warranties;

  for (const auto &w : allWarranties) {
    if (!warrantyKeyword.empty()) {
      bool match =
          Utf8Helper::containsCaseInsensitive(w.id, warrantyKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.phoneName, warrantyKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.issueDescription,
                                              warrantyKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.technicianNote,
                                              warrantyKeyword) ||
          Utf8Helper::containsCaseInsensitive(w.requestDate, warrantyKeyword);
      if (!match)
        continue;
    }

    // Lọc theo trạng thái bảo hành
    if (!warrantyFilterStatus.empty() && warrantyFilterStatus != "Tất cả") {
      if (w.statusText != warrantyFilterStatus)
        continue;
    }

    warranties.push_back(w);
  }

  // Sắp xếp
  if (warrantySortBy == "date_asc") {
    std::reverse(warranties.begin(), warranties.end());
  } else if (warrantySortBy == "id_asc") {
    std::sort(warranties.begin(), warranties.end(), [](const auto &a, const auto &b) {
      return a.id < b.id;
    });
  }

  UIComponents::updateRecordCountBadge(warrantyCountBadge, warranties.size(),
                                       "phiếu bảo hành");

  if (warranties.empty()) {
    GtkWidget *empty = UIComponents::createEmptyState(
        "🛡️",
        warrantyKeyword.empty() ? "Chưa có yêu cầu bảo hành nào"
                                : "Không tìm thấy phiếu bảo hành phù hợp",
        warrantyKeyword.empty()
            ? "Thiết bị của bạn hoạt động bình thường! Nếu gặp sự cố, bạn có "
              "thể tạo yêu cầu bảo hành bất cứ lúc nào."
            : "Thử tìm kiếm với từ khóa hoặc mã phiếu khác.",
        "Tạo yêu cầu bảo hành", G_CALLBACK(+[](GtkButton *b, gpointer d) {
          (void)b;
          CustomerScreens *self = static_cast<CustomerScreens *>(d);
          self->showCreateWarrantyModal();
        }),
        this);
    gtk_box_append(GTK_BOX(warrantyListBox), empty);
    return;
  }

  for (size_t i = 0; i < warranties.size(); ++i) {
    const auto &w = warranties[i];
    GtkWidget *row = UIComponents::createTableDataRow(i);

    // 0: Mã phiếu
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.id, kCustomerWarrantyCols[0], "cell-code"));

    // 1: Thiết bị
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.phoneName, kCustomerWarrantyCols[1], "cell-title"));

    // 2: Ngày gửi
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTextCell(
                       w.requestDate, kCustomerWarrantyCols[2], "tabular-num"));

    // 3: Trạng thái badge
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableWidgetCell(
                       UIComponents::createWarrantyStatusBadge(w.status),
                       kCustomerWarrantyCols[3]));

    // 4: Mô tả lỗi & phản hồi
    gtk_box_append(GTK_BOX(row),
                   UIComponents::createTableTwoLineCell(
                       "Hiện tượng lỗi: " + w.issueDescription,
                       w.technicianNote.empty()
                           ? "Đang chờ kỹ thuật viên kiểm tra"
                           : ("Kỹ thuật viên: " + w.technicianNote),
                       kCustomerWarrantyCols[4]));

    gtk_box_append(GTK_BOX(warrantyListBox), row);
  }
}

void CustomerScreens::showCreateWarrantyModal() {
  if (!shell)
    return;

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll, TRUE);
  gtk_widget_set_vexpand(scroll, TRUE);

  GtkWidget *pageContent =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, DesignTokens::SPACE_LG);
  gtk_widget_add_css_class(pageContent, "page-container");

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
  gtk_widget_add_css_class(card, "pdp-card");
  gtk_widget_set_hexpand(card, TRUE);

  GtkWidget *title = gtk_label_new("🛡️ Phiếu yêu cầu bảo hành thiết bị");
  gtk_widget_add_css_class(title, "card-title");
  gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
  gtk_box_append(GTK_BOX(card), title);

  GtkWidget *subtitle = gtk_label_new(
      "Chọn sản phẩm cần hỗ trợ và mô tả chi tiết tình trạng thiết bị để kỹ "
      "thuật viên tiếp nhận.");
  gtk_widget_add_css_class(subtitle, "page-subtitle");
  gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
  gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
  gtk_box_append(GTK_BOX(card), subtitle);

  std::vector<ProductItemViewModel> products = controller->getProducts();
  std::vector<const char *> prodNames;
  for (const auto &p : products) {
    prodNames.push_back(p.name.c_str());
  }
  prodNames.push_back(nullptr);

  GtkWidget *comboProd = gtk_drop_down_new_from_strings(prodNames.data());
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Chọn thiết bị cần bảo hành *", comboProd));

  GtkWidget *entryIssue = gtk_entry_new();
  gtk_entry_set_placeholder_text(
      GTK_ENTRY(entryIssue),
      "Mô tả chi tiết hiện tượng lỗi (vd: không lên nguồn, sọc màn hình)...");
  gtk_box_append(GTK_BOX(card), UIComponents::createFormField(
                                    "Mô tả lỗi gặp phải *", entryIssue));

  GtkWidget *btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btnRow, GTK_ALIGN_END);
  gtk_widget_set_margin_top(btnRow, 8);

  GtkWidget *btnCancel = gtk_button_new_with_label("Hủy");
  gtk_widget_add_css_class(btnCancel, "btn-secondary");
  UIComponents::setButtonCursor(btnCancel);
  g_signal_connect_swapped(
      btnCancel, "clicked",
      G_CALLBACK(+[](MainShellWindow *s) { s->popPage(); }), shell);
  gtk_box_append(GTK_BOX(btnRow), btnCancel);

  struct WarSubmitContext {
    CustomerScreens *screens;
    GtkWidget *combo;
    GtkWidget *entry;
    std::vector<ProductItemViewModel> prods;
  };
  WarSubmitContext *ctx =
      new WarSubmitContext{this, comboProd, entryIssue, products};

  GtkWidget *btnSubmit = gtk_button_new_with_label("Gửi yêu cầu");
  gtk_widget_add_css_class(btnSubmit, "btn-primary");
  UIComponents::setButtonCursor(btnSubmit);
  g_signal_connect_data(
      btnSubmit, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer d) {
        (void)b;
        WarSubmitContext *c = static_cast<WarSubmitContext *>(d);
        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->combo));
        const char *issue = gtk_editable_get_text(GTK_EDITABLE(c->entry));

        if (sel < c->prods.size() && issue && strlen(issue) > 0) {
          CreateWarrantyRequest req;
          req.phoneId = c->prods[sel].id;
          req.issueDescription = issue;
          OperationResult res = c->screens->controller->createWarranty(req);
          if (res.success) {
            c->screens->refreshWarranty();
            if (c->screens->shell) {
              c->screens->shell->popPage();
            }
            UIComponents::showSuccessDialog(
                c->screens->shell ? c->screens->shell->getWindow() : nullptr,
                "Đã tiếp nhận", res.message);
          } else {
            UIComponents::showErrorDialog(
                c->screens->shell ? c->screens->shell->getWindow() : nullptr,
                "Lỗi", res.message);
          }
        } else {
          UIComponents::showWarningDialog(
              c->screens->shell ? c->screens->shell->getWindow() : nullptr,
              "Thiếu thông tin", "Vui lòng nhập mô tả lỗi!");
        }
      }),
      ctx,
      [](gpointer d, GClosure *) { delete static_cast<WarSubmitContext *>(d); },
      (GConnectFlags)0);

  gtk_box_append(GTK_BOX(btnRow), btnSubmit);
  gtk_box_append(GTK_BOX(card), btnRow);

  UIComponents::setupDialogShortcuts(card, btnSubmit, btnCancel);

  gtk_box_append(GTK_BOX(pageContent), card);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), pageContent);
  shell->pushPage("customer_create_warranty", scroll,
                  "Tạo yêu cầu bảo hành mới");
  gtk_widget_grab_focus(entryIssue);
}
