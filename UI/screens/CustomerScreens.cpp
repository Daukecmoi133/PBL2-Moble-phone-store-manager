/**
 * @file CustomerScreens.cpp
 * @brief ĐẶC TẢ TRIỂN KHAI PHÂN HỆ KHÁCH HÀNG (CUSTOMER PORTAL)
 * 
 * =========================================================================================
 * Các chức năng chính được cài đặt:
 * - buildDashboardView: Hiển thị lời chào, cấp bậc thành viên (Rank badge), số điểm và chi tiêu.
 * - buildProductsView & refreshProducts: Duyệt danh mục điện thoại, tìm kiếm từ khóa, lọc theo hãng.
 * - buildCartView & refreshCart: Quản lý giỏ hàng, cập nhật số lượng, nhập voucher, đặt hàng.
 * - buildOrdersView & refreshOrders: Lịch sử đơn hàng, xem chi tiết và hủy đơn hàng khi đủ điều kiện.
 * - buildProfileView & refreshProfile: Thông tin cá nhân, quyền lợi thăng hạng theo điểm tích lũy.
 * - buildWarrantyView & refreshWarranty: Tra cứu chính sách và đăng ký bảo hành cho thiết bị đã mua.
 * =========================================================================================
 */

#include "CustomerScreens.h"
#include "MainShellWindow.h"
#include "../components/UIComponents.h"

struct AddToCartData {
    CustomerScreens* screens;
    std::string phoneId;
};

struct CartQtyData {
    CustomerScreens* screens;
    std::string phoneId;
    int newQty;
};

struct CancelOrderData {
    CustomerScreens* screens;
    std::string orderId;
};

CustomerScreens::CustomerScreens(AppController* ctrl, MainShellWindow* parentShell)
    : controller(ctrl), shell(parentShell),
      currentSearchKeyword(""), currentBrandFilter("Tất cả"), currentSortBy("name_asc") {

    dashboardView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dashboardView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    productsView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productsView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    cartView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(cartView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    ordersView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ordersView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    profileView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(profileView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    warrantyView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    buildDashboardView();
    buildProductsView();
    buildCartView();
    buildOrdersView();
    buildProfileView();
    buildWarrantyView();
}

void CustomerScreens::refreshAll() {
    refreshDashboard();
    refreshProducts();
    refreshCart();
    refreshOrders();
    refreshProfile();
    refreshWarranty();
}

// ================= 1. CUSTOMER DASHBOARD =================

void CustomerScreens::buildDashboardView() {

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_add_css_class(content, "page-container");

    // Welcome Card
    GtkWidget* welcomeCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_add_css_class(welcomeCard, "card");
    gtk_widget_set_name(welcomeCard, "customer_welcome_card");

    const UserSessionViewModel& user = controller->getCurrentUser();
    std::string welcomeTitle = "Xin chào, " + (user.fullName.empty() ? "Khách hàng" : user.fullName) + " 👋";
    GtkWidget* lblTitle = gtk_label_new(welcomeTitle.c_str());
    gtk_widget_add_css_class(lblTitle, "page-title");
    gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);

    GtkWidget* lblSubtitle = gtk_label_new("Chào mừng bạn đến với cửa hàng điện thoại PBL2 Store. Khám phá các mẫu điện thoại mới nhất hôm nay!");
    gtk_widget_add_css_class(lblSubtitle, "page-subtitle");
    gtk_label_set_xalign(GTK_LABEL(lblSubtitle), 0.0f);

    gtk_box_append(GTK_BOX(welcomeCard), lblTitle);
    gtk_box_append(GTK_BOX(welcomeCard), lblSubtitle);
    gtk_box_append(GTK_BOX(content), welcomeCard);

    // 3 Quick Metric Cards
    GtkWidget* statsGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(statsGrid), 14);
    gtk_grid_set_column_spacing(GTK_GRID(statsGrid), 14);
    gtk_grid_set_column_homogeneous(GTK_GRID(statsGrid), TRUE);

    CustomerItemViewModel cust = controller->getCustomerById(user.userId);
    std::vector<OrderItemViewModel> myOrders = controller->getCustomerOrders(user.userId);
    std::vector<WarrantyItemViewModel> myWarranties = controller->getCustomerWarranties(user.userId);

    int pendingCount = 0;
    for (const auto& o : myOrders) {
        if (o.status == OrderStatus::PENDING || o.status == OrderStatus::CONFIRMED || o.status == OrderStatus::PREPARING) {
            pendingCount++;
        }
    }

    GtkWidget* cardPoints = UIComponents::createStatCard("Điểm tích lũy", std::to_string(cust.points) + " điểm", "Hạng: " + cust.membershipTier, "⭐");
    GtkWidget* cardOrders = UIComponents::createStatCard("Đơn hàng đang xử lý", std::to_string(pendingCount) + " đơn", "Tổng số: " + std::to_string(myOrders.size()) + " đơn", "📦");
    GtkWidget* cardWarranty = UIComponents::createStatCard("Yêu cầu bảo hành", std::to_string(myWarranties.size()) + " phiếu", "Bảo hành chính hãng", "🛡️");

    gtk_grid_attach(GTK_GRID(statsGrid), cardPoints, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(statsGrid), cardOrders, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(statsGrid), cardWarranty, 2, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), statsGrid);

    // Promo Coupons Banner
    GtkWidget* promoCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(promoCard, "card");

    GtkWidget* lblPromoHeader = gtk_label_new("🎟️ Ưu đãi & Mã giảm giá đang diễn ra");
    gtk_widget_add_css_class(lblPromoHeader, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblPromoHeader), 0.0f);
    gtk_box_append(GTK_BOX(promoCard), lblPromoHeader);

    std::vector<DiscountItemViewModel> discounts = controller->getDiscounts();
    size_t discIdx = 0;
    for (const auto& d : discounts) {
        if (!d.isValid) continue;
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (discIdx % 2 == 0) ? "row-even" : "row-odd");
        discIdx++;

        GtkWidget* codeBadge = UIComponents::createStatusBadge(d.code, "badge-confirmed");
        GtkWidget* desc = gtk_label_new(d.description.c_str());
        gtk_widget_set_hexpand(desc, TRUE);
        gtk_label_set_xalign(GTK_LABEL(desc), 0.0f);

        GtkWidget* exp = gtk_label_new(("HSD: " + d.expiryDate).c_str());
        gtk_widget_add_css_class(exp, "stat-label");

        gtk_box_append(GTK_BOX(row), codeBadge);
        gtk_box_append(GTK_BOX(row), desc);
        gtk_box_append(GTK_BOX(row), exp);
        gtk_box_append(GTK_BOX(promoCard), row);
    }
    gtk_box_append(GTK_BOX(content), promoCard);

    // Quick Action Bar
    GtkWidget* actionsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget* btnShop = gtk_button_new_with_label("🛍️ Xem danh mục điện thoại");
    gtk_widget_add_css_class(btnShop, "btn-primary");
    UIComponents::setButtonCursor(btnShop);
    g_signal_connect_swapped(btnShop, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("customer_products");
    }), shell);

    GtkWidget* btnCart = gtk_button_new_with_label("🛒 Xem giỏ hàng của tôi");
    gtk_widget_add_css_class(btnCart, "btn-secondary");
    UIComponents::setButtonCursor(btnCart);
    g_signal_connect_swapped(btnCart, "clicked", G_CALLBACK(+[](MainShellWindow* s) {
        s->navigateTo("customer_cart");
    }), shell);

    gtk_box_append(GTK_BOX(actionsBox), btnShop);
    gtk_box_append(GTK_BOX(actionsBox), btnCart);
    gtk_box_append(GTK_BOX(content), actionsBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(dashboardView), content);
}

void CustomerScreens::refreshDashboard() {
    buildDashboardView();
}

// ================= 2. DANH SÁCH SẢN PHẨM =================

void CustomerScreens::buildProductsView() {
    productsView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(productsView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    // Header
    GtkWidget* header = UIComponents::createSectionHeader("Danh mục sản phẩm", "Khám phá các dòng điện thoại thông minh chính hãng với giá tốt nhất.");
    gtk_box_append(GTK_BOX(content), header);

    // Filter Bar Card
    GtkWidget* filterCard = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(filterCard, "card");
    gtk_widget_set_margin_bottom(filterCard, 6);

    // Search bar
    GtkWidget* searchBar = UIComponents::createSearchBar("Tìm theo tên điện thoại hoặc mã...", G_CALLBACK(+[](GtkEditable* entry, gpointer data) {
        CustomerScreens* self = static_cast<CustomerScreens*>(data);
        self->currentSearchKeyword = gtk_editable_get_text(entry);
        self->refreshProducts();
    }), this);
    gtk_widget_set_hexpand(searchBar, TRUE);
    gtk_box_append(GTK_BOX(filterCard), searchBar);

    // Brand Buttons
    const char* brands[] = {"Tất cả", "Apple", "Samsung", "Oppo", "Xiaomi"};
    for (const char* b : brands) {
        GtkWidget* btnB = gtk_button_new_with_label(b);
        gtk_widget_add_css_class(btnB, "btn-outline");
        UIComponents::setButtonCursor(btnB);
        g_signal_connect_data(btnB, "clicked", G_CALLBACK(+[](GtkButton* btn, gpointer data) {
            CustomerScreens* self = static_cast<CustomerScreens*>(data);
            self->currentBrandFilter = gtk_button_get_label(btn);
            self->refreshProducts();
        }), this, nullptr, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(filterCard), btnB);
    }

    gtk_box_append(GTK_BOX(content), filterCard);

    // Dynamic Products List Container
    productsListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_box_append(GTK_BOX(content), productsListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(productsView), content);
    refreshProducts();
}

void CustomerScreens::refreshProducts() {
    // Xóa danh sách cũ
    GtkWidget* child = gtk_widget_get_first_child(productsListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(productsListBox), child);
        child = next;
    }

    ProductFilterRequest req;
    req.keyword = currentSearchKeyword;
    req.brand = currentBrandFilter;
    req.sortBy = currentSortBy;

    std::vector<ProductItemViewModel> products = controller->getProducts(req);

    if (products.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState("🔍", "Không tìm thấy sản phẩm phù hợp",
            "Thử tìm kiếm với từ khóa khác hoặc xóa bộ lọc thương hiệu.", "Xóa bộ lọc",
            G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                CustomerScreens* self = static_cast<CustomerScreens*>(d);
                self->currentSearchKeyword = "";
                self->currentBrandFilter = "Tất cả";
                self->refreshProducts();
            }), this);
        gtk_box_append(GTK_BOX(productsListBox), empty);
        return;
    }

    for (const auto& p : products) {
        GtkWidget* card = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
        gtk_widget_add_css_class(card, "product-card");

        // Thumbnail or Icon Phone
        GtkWidget* imgThumb = nullptr;
        if (!p.imageUrl.empty() && g_file_test(p.imageUrl.c_str(), G_FILE_TEST_EXISTS)) {
            imgThumb = gtk_image_new_from_file(p.imageUrl.c_str());
            gtk_image_set_pixel_size(GTK_IMAGE(imgThumb), 56);
        } else {
            imgThumb = gtk_label_new("📱");
            gtk_widget_set_size_request(imgThumb, 56, 56);
        }
        gtk_box_append(GTK_BOX(card), imgThumb);

        // Info box
        GtkWidget* infoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_widget_set_hexpand(infoBox, TRUE);

        GtkWidget* topRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* lblName = gtk_label_new(p.name.c_str());
        gtk_widget_add_css_class(lblName, "product-name");
        gtk_label_set_xalign(GTK_LABEL(lblName), 0.0f);

        GtkWidget* badgeBrand = UIComponents::createStatusBadge(p.brand, "role-pill-sales");
        GtkWidget* badgeStock = UIComponents::createStockStatusBadge(p.status);

        gtk_box_append(GTK_BOX(topRow), lblName);
        gtk_box_append(GTK_BOX(topRow), badgeBrand);
        gtk_box_append(GTK_BOX(topRow), badgeStock);
        gtk_box_append(GTK_BOX(infoBox), topRow);

        GtkWidget* lblSpecs = gtk_label_new(p.specs.c_str());
        gtk_widget_add_css_class(lblSpecs, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(lblSpecs), 0.0f);
        gtk_label_set_wrap(GTK_LABEL(lblSpecs), TRUE);
        gtk_box_append(GTK_BOX(infoBox), lblSpecs);

        GtkWidget* lblAvail = gtk_label_new(("Số lượng có thể bán: " + std::to_string(p.availableStock) + " máy").c_str());
        gtk_widget_add_css_class(lblAvail, "stat-label");
        gtk_label_set_xalign(GTK_LABEL(lblAvail), 0.0f);
        gtk_box_append(GTK_BOX(infoBox), lblAvail);

        gtk_box_append(GTK_BOX(card), infoBox);

        // Price & Actions box
        GtkWidget* actionBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_set_valign(actionBox, GTK_ALIGN_CENTER);

        GtkWidget* lblPrice = gtk_label_new(UIComponents::formatVND(p.sellingPrice).c_str());
        gtk_widget_add_css_class(lblPrice, "product-price");
        gtk_label_set_xalign(GTK_LABEL(lblPrice), 1.0f);
        gtk_box_append(GTK_BOX(actionBox), lblPrice);

        GtkWidget* btnsRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

        GtkWidget* btnDetail = gtk_button_new_with_label("Chi tiết");
        gtk_widget_add_css_class(btnDetail, "btn-outline");
        UIComponents::setButtonCursor(btnDetail);
        AddToCartData* dData = new AddToCartData{this, p.id};
        g_signal_connect_data(btnDetail, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AddToCartData* ad = static_cast<AddToCartData*>(d);
            ProductItemViewModel prod = ad->screens->controller->getProductById(ad->phoneId);
            ad->screens->showProductDetailModal(prod);
        }), dData, [](gpointer d, GClosure*) { delete static_cast<AddToCartData*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(btnsRow), btnDetail);

        GtkWidget* btnAdd = gtk_button_new_with_label("🛒 Thêm vào giỏ");
        if (p.availableStock <= 0) {
            gtk_widget_set_sensitive(btnAdd, FALSE);
            gtk_widget_add_css_class(btnAdd, "btn-secondary");
        } else {
            gtk_widget_add_css_class(btnAdd, "btn-primary");
            UIComponents::setButtonCursor(btnAdd);
        }

        AddToCartData* aData = new AddToCartData{this, p.id};
        g_signal_connect_data(btnAdd, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            AddToCartData* ad = static_cast<AddToCartData*>(d);
            const UserSessionViewModel& cur = ad->screens->controller->getCurrentUser();
            OperationResult res = ad->screens->controller->addToCart(cur.userId, ad->phoneId, 1);
            if (res.success) {
                CartSummaryViewModel cart = ad->screens->controller->getCart(cur.userId);
                ad->screens->shell->updateCartBadge(cart.totalQuantity);
                ad->screens->refreshCart();
                UIComponents::showSuccessDialog(ad->screens->shell->getWindow(), "Thành công", res.message);
            } else {
                UIComponents::showWarningDialog(ad->screens->shell->getWindow(), "Không thể thêm", res.message);
            }
        }), aData, [](gpointer d, GClosure*) { delete static_cast<AddToCartData*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(btnsRow), btnAdd);

        gtk_box_append(GTK_BOX(actionBox), btnsRow);
        gtk_box_append(GTK_BOX(card), actionBox);

        gtk_box_append(GTK_BOX(productsListBox), card);
    }
}

void CustomerScreens::showProductDetailModal(const ProductItemViewModel& product) {
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), ("Chi tiết sản phẩm — " + product.name).c_str());
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 500, 420);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    GtkWidget* title = gtk_label_new(product.name.c_str());
    gtk_widget_add_css_class(title, "page-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(card), title);

    if (!product.imageUrl.empty() && g_file_test(product.imageUrl.c_str(), G_FILE_TEST_EXISTS)) {
        GtkWidget* img = gtk_image_new_from_file(product.imageUrl.c_str());
        gtk_image_set_pixel_size(GTK_IMAGE(img), 120);
        gtk_widget_set_halign(img, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(card), img);
    }

    auto addSpecRow = [card](const char* label, const std::string& val) {
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* l = gtk_label_new(label);
        gtk_widget_add_css_class(l, "stat-label");
        gtk_widget_set_size_request(l, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(l), 0.0f);

        GtkWidget* v = gtk_label_new(val.c_str());
        gtk_label_set_xalign(GTK_LABEL(v), 0.0f);
        gtk_label_set_wrap(GTK_LABEL(v), TRUE);
        gtk_widget_set_hexpand(v, TRUE);

        gtk_box_append(GTK_BOX(row), l);
        gtk_box_append(GTK_BOX(row), v);
        gtk_box_append(GTK_BOX(card), row);
    };

    addSpecRow("Mã sản phẩm:", product.id);
    addSpecRow("Thương hiệu:", product.brand);
    addSpecRow("Hệ điều hành:", product.os);
    addSpecRow("Nhà sản xuất:", product.manufacturer);
    addSpecRow("Thông số kỹ thuật:", product.specs);
    addSpecRow("Tồn kho thực tế:", std::to_string(product.actualStock) + " máy");
    addSpecRow("Đang giữ chỗ:", std::to_string(product.reservedStock) + " máy");
    addSpecRow("Có thể bán:", std::to_string(product.availableStock) + " máy");
    addSpecRow("Giá niêm yết:", UIComponents::formatVND(product.sellingPrice));

    GtkWidget* btnClose = gtk_button_new_with_label("Đóng");
    gtk_widget_add_css_class(btnClose, "btn-secondary");
    UIComponents::setButtonCursor(btnClose);
    gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
    g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(card), btnClose);

    gtk_window_set_child(GTK_WINDOW(win), card);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 3. GIỎ HÀNG & CHECKOUT =================

void CustomerScreens::buildCartView() {
    cartView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(cartView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Giỏ hàng & Đặt hàng", "Kiểm tra sản phẩm, áp dụng mã giảm giá và hoàn tất đơn hàng.");
    gtk_box_append(GTK_BOX(content), header);

    // Grid layout: Left: Cart items table, Right: Checkout Form & Total
    GtkWidget* mainGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(mainGrid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(mainGrid), 20);

    // Left Container: Cart Items Box
    cartListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(cartListBox, TRUE);
    gtk_grid_attach(GTK_GRID(mainGrid), cartListBox, 0, 0, 1, 1);

    // Right Container: Checkout Card
    GtkWidget* checkoutCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(checkoutCard, "card");
    gtk_widget_set_size_request(checkoutCard, 380, -1);

    GtkWidget* lblCheckoutTitle = gtk_label_new("Thông tin thanh toán & Giao hàng");
    gtk_widget_add_css_class(lblCheckoutTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblCheckoutTitle), 0.0f);
    gtk_box_append(GTK_BOX(checkoutCard), lblCheckoutTitle);

    // Coupon box
    GtkWidget* couponBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    entryCoupon = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entryCoupon), "Nhập mã voucher (vd: SALE10)...");
    gtk_widget_set_hexpand(entryCoupon, TRUE);

    GtkWidget* btnApplyCoupon = gtk_button_new_with_label("Áp dụng");
    gtk_widget_add_css_class(btnApplyCoupon, "btn-primary");
    UIComponents::setButtonCursor(btnApplyCoupon);
    g_signal_connect_data(btnApplyCoupon, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        CustomerScreens* self = static_cast<CustomerScreens*>(d);
        const char* code = gtk_editable_get_text(GTK_EDITABLE(self->entryCoupon));
        const UserSessionViewModel& cur = self->controller->getCurrentUser();
        OperationResult res = self->controller->applyCoupon(cur.userId, code ? code : "");
        if (res.success) {
            self->refreshCart();
            UIComponents::showSuccessDialog(self->shell->getWindow(), "Mã giảm giá", res.message);
        } else {
            UIComponents::showWarningDialog(self->shell->getWindow(), "Không hợp lệ", res.message);
        }
    }), this, nullptr, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(couponBox), entryCoupon);
    gtk_box_append(GTK_BOX(couponBox), btnApplyCoupon);
    gtk_box_append(GTK_BOX(checkoutCard), UIComponents::createFormField("Mã giảm giá", couponBox));

    // Delivery Method Dropdown
    const char* deliveryOptions[] = {"Nhận tại cửa hàng (54 Nguyễn Lương Bằng)", "Giao hàng tận nơi", nullptr};
    comboDeliveryMethod = gtk_drop_down_new_from_strings(deliveryOptions);
    gtk_box_append(GTK_BOX(checkoutCard), UIComponents::createFormField("Hình thức nhận hàng", comboDeliveryMethod));

    // Recipient Info
    const UserSessionViewModel& cur = controller->getCurrentUser();
    entryReceiverName = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryReceiverName), cur.fullName.c_str());
    gtk_box_append(GTK_BOX(checkoutCard), UIComponents::createFormField("Họ tên người nhận *", entryReceiverName));

    entryReceiverPhone = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryReceiverPhone), cur.phone.c_str());
    gtk_box_append(GTK_BOX(checkoutCard), UIComponents::createFormField("Số điện thoại nhận hàng *", entryReceiverPhone));

    entryDeliveryAddress = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryDeliveryAddress), cur.address.c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(entryDeliveryAddress), "Địa chỉ giao hàng chi tiết...");
    gtk_box_append(GTK_BOX(checkoutCard), UIComponents::createFormField("Địa chỉ nhận hàng (nếu giao tận nơi)", entryDeliveryAddress));

    // Payment Method Dropdown
    const char* paymentOptions[] = {"Tiền mặt khi nhận hàng (COD)", "Chuyển khoản ngân hàng", nullptr};
    comboPaymentMethod = gtk_drop_down_new_from_strings(paymentOptions);
    gtk_box_append(GTK_BOX(checkoutCard), UIComponents::createFormField("Phương thức thanh toán", comboPaymentMethod));

    // Dynamic Summary Container
    cartSummaryBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_append(GTK_BOX(checkoutCard), cartSummaryBox);

    // Place Order Button
    GtkWidget* btnCheckout = gtk_button_new_with_label("🛍️ Xác nhận đặt hàng");
    gtk_widget_add_css_class(btnCheckout, "btn-success");
    UIComponents::setButtonCursor(btnCheckout);
    gtk_widget_set_margin_top(btnCheckout, 10);
    g_signal_connect_data(btnCheckout, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        CustomerScreens* self = static_cast<CustomerScreens*>(d);
        const UserSessionViewModel& curUser = self->controller->getCurrentUser();

        CheckoutRequest req;
        req.customerId = curUser.userId;
        req.receiverName = gtk_editable_get_text(GTK_EDITABLE(self->entryReceiverName));
        req.receiverPhone = gtk_editable_get_text(GTK_EDITABLE(self->entryReceiverPhone));
        req.deliveryAddress = gtk_editable_get_text(GTK_EDITABLE(self->entryDeliveryAddress));
        req.deliveryMethod = gtk_drop_down_get_selected(GTK_DROP_DOWN(self->comboDeliveryMethod));
        req.paymentMethod = gtk_drop_down_get_selected(GTK_DROP_DOWN(self->comboPaymentMethod));
        req.discountCode = gtk_editable_get_text(GTK_EDITABLE(self->entryCoupon));

        std::string orderId;
        OperationResult res = self->controller->checkout(req, orderId);
        if (res.success) {
            self->shell->updateCartBadge(0);
            self->refreshCart();
            self->refreshOrders();
            UIComponents::showSuccessDialog(self->shell->getWindow(), "Đặt hàng thành công", res.message);
            self->shell->navigateTo("customer_orders");
        } else {
            UIComponents::showErrorDialog(self->shell->getWindow(), "Lỗi đặt hàng", res.message);
        }
    }), this, nullptr, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(checkoutCard), btnCheckout);

    gtk_grid_attach(GTK_GRID(mainGrid), checkoutCard, 1, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), mainGrid);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(cartView), content);
    refreshCart();
}

void CustomerScreens::refreshCart() {
    // Xóa danh sách items
    GtkWidget* child = gtk_widget_get_first_child(cartListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(cartListBox), child);
        child = next;
    }

    // Xóa summary box
    child = gtk_widget_get_first_child(cartSummaryBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(cartSummaryBox), child);
        child = next;
    }

    const UserSessionViewModel& user = controller->getCurrentUser();
    CartSummaryViewModel cart = controller->getCart(user.userId);

    if (cart.items.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState("🛒", "Giỏ hàng của bạn đang trống",
            "Bạn chưa chọn sản phẩm nào. Hãy khám phá và chọn sản phẩm yêu thích nhé!", "Mua sắm ngay",
            G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                CustomerScreens* self = static_cast<CustomerScreens*>(d);
                self->shell->navigateTo("customer_products");
            }), this);
        gtk_box_append(GTK_BOX(cartListBox), empty);

        // Hiển thị 0 ₫ trong summary
        GtkWidget* lblZero = gtk_label_new("Tổng thanh toán: 0 ₫");
        gtk_widget_add_css_class(lblZero, "stat-label");
        gtk_box_append(GTK_BOX(cartSummaryBox), lblZero);
        return;
    }

    // Card bao danh sách các mặt hàng
    GtkWidget* itemsCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(itemsCard, "card");

    GtkWidget* lblTableTitle = gtk_label_new(("Danh sách sản phẩm trong giỏ (" + std::to_string(cart.totalQuantity) + " máy)").c_str());
    gtk_widget_add_css_class(lblTableTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblTableTitle), 0.0f);
    gtk_box_append(GTK_BOX(itemsCard), lblTableTitle);

    std::vector<UIComponents::TableColumnSpec> cartCols = {
        {"Tên sản phẩm & Đơn giá", 0, 0.0f, true},
        {"Số lượng", 110, 0.5f, false},
        {"Thành tiền", 140, 1.0f, false},
        {"Thao tác", 70, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(itemsCard), UIComponents::createTableHeaderRow(cartCols));

    for (size_t i = 0; i < cart.items.size(); ++i) {
        const auto& item = cart.items[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        GtkWidget* infoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_hexpand(infoBox, TRUE);
        gtk_widget_set_valign(infoBox, GTK_ALIGN_CENTER);

        GtkWidget* nameLbl = gtk_label_new(item.phoneName.c_str());
        gtk_widget_add_css_class(nameLbl, "product-name");
        gtk_label_set_xalign(GTK_LABEL(nameLbl), 0.0f);

        GtkWidget* priceLbl = gtk_label_new(("Đơn giá: " + UIComponents::formatVND(item.unitPrice)).c_str());
        gtk_widget_add_css_class(priceLbl, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(priceLbl), 0.0f);

        gtk_box_append(GTK_BOX(infoBox), nameLbl);
        gtk_box_append(GTK_BOX(infoBox), priceLbl);
        gtk_box_append(GTK_BOX(row), infoBox);

        // Quantity controls
        GtkWidget* qtyBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_widget_set_size_request(qtyBox, 110, -1);
        gtk_widget_set_valign(qtyBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(qtyBox, GTK_ALIGN_CENTER);

        GtkWidget* btnMinus = gtk_button_new_with_label("-");
        gtk_widget_add_css_class(btnMinus, "btn-outline");
        UIComponents::setButtonCursor(btnMinus);
        CartQtyData* mData = new CartQtyData{this, item.phoneId, item.quantity - 1};
        g_signal_connect_data(btnMinus, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            CartQtyData* cd = static_cast<CartQtyData*>(d);
            const UserSessionViewModel& cur = cd->screens->controller->getCurrentUser();
            cd->screens->controller->updateCartQuantity(cur.userId, cd->phoneId, cd->newQty);
            CartSummaryViewModel c = cd->screens->controller->getCart(cur.userId);
            cd->screens->shell->updateCartBadge(c.totalQuantity);
            cd->screens->refreshCart();
        }), mData, [](gpointer d, GClosure*) { delete static_cast<CartQtyData*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(qtyBox), btnMinus);

        GtkWidget* lblQty = gtk_label_new(std::to_string(item.quantity).c_str());
        gtk_widget_set_size_request(lblQty, 30, -1);
        gtk_box_append(GTK_BOX(qtyBox), lblQty);

        GtkWidget* btnPlus = gtk_button_new_with_label("+");
        gtk_widget_add_css_class(btnPlus, "btn-outline");
        UIComponents::setButtonCursor(btnPlus);
        CartQtyData* pData = new CartQtyData{this, item.phoneId, item.quantity + 1};
        g_signal_connect_data(btnPlus, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            CartQtyData* cd = static_cast<CartQtyData*>(d);
            const UserSessionViewModel& cur = cd->screens->controller->getCurrentUser();
            OperationResult res = cd->screens->controller->updateCartQuantity(cur.userId, cd->phoneId, cd->newQty);
            if (!res.success) {
                UIComponents::showWarningDialog(cd->screens->shell->getWindow(), "Tồn kho", res.message);
            }
            CartSummaryViewModel c = cd->screens->controller->getCart(cur.userId);
            cd->screens->shell->updateCartBadge(c.totalQuantity);
            cd->screens->refreshCart();
        }), pData, [](gpointer d, GClosure*) { delete static_cast<CartQtyData*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(qtyBox), btnPlus);

        gtk_box_append(GTK_BOX(row), qtyBox);

        // Subtotal (căn phải)
        GtkWidget* subLbl = gtk_label_new(UIComponents::formatVND(item.subtotal).c_str());
        gtk_widget_add_css_class(subLbl, "product-price");
        gtk_widget_set_size_request(subLbl, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(subLbl), 1.0f);
        gtk_widget_set_valign(subLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), subLbl);

        // Remove Button
        GtkWidget* btnDel = UIComponents::createTableActionButton("✕", "Xóa sản phẩm khỏi giỏ hàng", "btn-action-delete");
        gtk_widget_set_size_request(btnDel, 70, -1);
        CartQtyData* delData = new CartQtyData{this, item.phoneId, 0};
        g_signal_connect_data(btnDel, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            CartQtyData* cd = static_cast<CartQtyData*>(d);
            const UserSessionViewModel& cur = cd->screens->controller->getCurrentUser();
            cd->screens->controller->removeFromCart(cur.userId, cd->phoneId);
            CartSummaryViewModel c = cd->screens->controller->getCart(cur.userId);
            cd->screens->shell->updateCartBadge(c.totalQuantity);
            cd->screens->refreshCart();
        }), delData, [](gpointer d, GClosure*) { delete static_cast<CartQtyData*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(row), btnDel);

        gtk_box_append(GTK_BOX(itemsCard), row);
    }
    gtk_box_append(GTK_BOX(cartListBox), itemsCard);

    // Summary Breakdown
    auto addSummaryRow = [this](const char* label, const std::string& val, bool isTotal = false) {
        GtkWidget* r = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* l = gtk_label_new(label);
        gtk_widget_set_hexpand(l, TRUE);
        gtk_label_set_xalign(GTK_LABEL(l), 0.0f);

        GtkWidget* v = gtk_label_new(val.c_str());
        gtk_label_set_xalign(GTK_LABEL(v), 1.0f);

        if (isTotal) {
            gtk_widget_add_css_class(l, "card-title");
            gtk_widget_add_css_class(v, "product-price");
        } else {
            gtk_widget_add_css_class(l, "stat-label");
            gtk_widget_add_css_class(v, "stat-label");
        }

        gtk_box_append(GTK_BOX(r), l);
        gtk_box_append(GTK_BOX(r), v);
        gtk_box_append(GTK_BOX(this->cartSummaryBox), r);
    };

    addSummaryRow("Tạm tính:", UIComponents::formatVND(cart.subtotal));
    if (cart.discountAmount > 0) {
        addSummaryRow("Giảm giá (Voucher):", "-" + UIComponents::formatVND(cart.discountAmount));
    }
    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(cartSummaryBox), sep);
    addSummaryRow("Tổng thanh toán:", UIComponents::formatVND(cart.grandTotal), true);
}

// ================= 4. ĐƠN HÀNG CỦA TÔI =================

void CustomerScreens::buildOrdersView() {
    ordersView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ordersView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Đơn hàng của tôi", "Theo dõi tình trạng xử lý đơn hàng và lịch sử đặt hàng của bạn.");
    gtk_box_append(GTK_BOX(content), header);

    std::vector<UIComponents::TableColumnSpec> ordCols = {
        {"Mã đơn", 120, 0.0f, false},
        {"Trạng thái", 130, 0.5f, false},
        {"Thanh toán", 130, 0.5f, false},
        {"Thời gian & Hình thức", 180, 0.0f, false},
        {"Danh sách sản phẩm", 0, 0.0f, true},
        {"Tổng tiền", 140, 1.0f, false},
        {"Thao tác", 160, 0.5f, false}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(ordCols));

    ordersListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), ordersListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(ordersView), content);
    refreshOrders();
}

void CustomerScreens::refreshOrders() {
    GtkWidget* child = gtk_widget_get_first_child(ordersListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(ordersListBox), child);
        child = next;
    }

    const UserSessionViewModel& user = controller->getCurrentUser();
    std::vector<OrderItemViewModel> orders = controller->getCustomerOrders(user.userId);

    if (orders.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState("📦", "Chưa có đơn hàng nào",
            "Bạn chưa đặt mua đơn hàng nào. Hãy chọn những sản phẩm ưng ý tại cửa hàng nhé!", "Khám phá sản phẩm",
            G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                CustomerScreens* self = static_cast<CustomerScreens*>(d);
                self->shell->navigateTo("customer_products");
            }), this);
        gtk_box_append(GTK_BOX(ordersListBox), empty);
        return;
    }

    for (size_t i = 0; i < orders.size(); ++i) {
        const auto& ord = orders[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Mã đơn
        GtkWidget* lblId = gtk_label_new(ord.id.c_str());
        gtk_widget_add_css_class(lblId, "product-name");
        gtk_widget_set_size_request(lblId, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(lblId), 0.0f);
        gtk_widget_set_valign(lblId, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), lblId);

        // Trạng thái đơn (Badge)
        GtkWidget* statusBadge = UIComponents::createOrderStatusBadge(ord.status);
        GtkWidget* stBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(stBox, 130, -1);
        gtk_widget_set_valign(stBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(stBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(stBox), statusBadge);
        gtk_box_append(GTK_BOX(row), stBox);

        // Thanh toán (Badge)
        GtkWidget* payBadge = UIComponents::createPaymentStatusBadge(ord.paymentStatus);
        GtkWidget* payBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(payBox, 130, -1);
        gtk_widget_set_valign(payBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(payBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(payBox), payBadge);
        gtk_box_append(GTK_BOX(row), payBox);

        // Thời gian & Hình thức
        GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_size_request(timeBox, 180, -1);
        gtk_widget_set_valign(timeBox, GTK_ALIGN_CENTER);
        GtkWidget* lblTime = gtk_label_new(ord.orderTime.c_str());
        gtk_label_set_xalign(GTK_LABEL(lblTime), 0.0f);
        GtkWidget* lblMethod = gtk_label_new(ord.deliveryMethodText.c_str());
        gtk_widget_add_css_class(lblMethod, "page-subtitle");
        gtk_label_set_xalign(GTK_LABEL(lblMethod), 0.0f);
        gtk_box_append(GTK_BOX(timeBox), lblTime);
        gtk_box_append(GTK_BOX(timeBox), lblMethod);
        gtk_box_append(GTK_BOX(row), timeBox);

        // Danh sách sản phẩm
        std::string itemsSummary = "";
        for (size_t j = 0; j < ord.details.size(); ++j) {
            if (j > 0) itemsSummary += ", ";
            itemsSummary += ord.details[j].phoneName + " (x" + std::to_string(ord.details[j].quantity) + ")";
        }
        GtkWidget* lblItems = gtk_label_new(itemsSummary.c_str());
        gtk_widget_set_hexpand(lblItems, TRUE);
        gtk_label_set_xalign(GTK_LABEL(lblItems), 0.0f);
        gtk_widget_set_valign(lblItems, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), lblItems);

        // Tổng tiền (căn phải)
        GtkWidget* lblTotal = gtk_label_new(UIComponents::formatVND(ord.total).c_str());
        gtk_widget_add_css_class(lblTotal, "product-price");
        gtk_widget_set_size_request(lblTotal, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(lblTotal), 1.0f);
        gtk_widget_set_valign(lblTotal, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), lblTotal);

        // Actions
        GtkWidget* actionsRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_widget_set_size_request(actionsRow, 160, -1);
        gtk_widget_set_valign(actionsRow, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(actionsRow, GTK_ALIGN_CENTER);

        GtkWidget* btnDetail = UIComponents::createTableActionButton("🔍 Chi tiết", "Xem thông tin chi tiết các sản phẩm trong đơn", "btn-action-edit");
        CancelOrderData* dData = new CancelOrderData{this, ord.id};
        g_signal_connect_data(btnDetail, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
            (void)b;
            CancelOrderData* cd = static_cast<CancelOrderData*>(d);
            OrderItemViewModel o = cd->screens->controller->getOrderById(cd->orderId);
            cd->screens->showOrderDetailModal(o);
        }), dData, [](gpointer d, GClosure*) { delete static_cast<CancelOrderData*>(d); }, (GConnectFlags)0);
        gtk_box_append(GTK_BOX(actionsRow), btnDetail);

        if (ord.canCancel()) {
            GtkWidget* btnCancel = UIComponents::createTableActionButton("✕ Hủy", "Hủy đơn hàng chưa xử lý", "btn-action-delete");
            CancelOrderData* cData = new CancelOrderData{this, ord.id};
            g_signal_connect_data(btnCancel, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                CancelOrderData* cd = static_cast<CancelOrderData*>(d);
                UIComponents::showConfirmDialog(cd->screens->shell->getWindow(),
                    "Xác nhận hủy đơn hàng",
                    "Bạn có chắc chắn muốn hủy đơn hàng " + cd->orderId + " không?",
                    [](gpointer data) {
                        CancelOrderData* inner = static_cast<CancelOrderData*>(data);
                        OperationResult res = inner->screens->controller->cancelOrder(inner->orderId);
                        if (res.success) {
                            inner->screens->refreshOrders();
                            UIComponents::showSuccessDialog(inner->screens->shell->getWindow(), "Đã hủy đơn", res.message);
                        } else {
                            UIComponents::showErrorDialog(inner->screens->shell->getWindow(), "Lỗi hủy đơn", res.message);
                        }
                    }, cd);
            }), cData, nullptr, (GConnectFlags)0);
            gtk_box_append(GTK_BOX(actionsRow), btnCancel);
        }

        gtk_box_append(GTK_BOX(row), actionsRow);
        gtk_box_append(GTK_BOX(ordersListBox), row);
    }
}

void CustomerScreens::showOrderDetailModal(const OrderItemViewModel& order) {
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), ("Chi tiết đơn hàng — " + order.id).c_str());
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 520, 480);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    GtkWidget* lblTitle = gtk_label_new(("Chi tiết đơn hàng " + order.id).c_str());
    gtk_widget_add_css_class(lblTitle, "page-title");
    gtk_label_set_xalign(GTK_LABEL(lblTitle), 0.0f);
    gtk_box_append(GTK_BOX(card), lblTitle);

    auto addRow = [card](const char* label, const std::string& val) {
        GtkWidget* r = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* l = gtk_label_new(label);
        gtk_widget_add_css_class(l, "stat-label");
        gtk_widget_set_size_request(l, 140, -1);
        gtk_label_set_xalign(GTK_LABEL(l), 0.0f);

        GtkWidget* v = gtk_label_new(val.c_str());
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

    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(card), sep);

    GtkWidget* lblItemsHeader = gtk_label_new("Danh sách mặt hàng:");
    gtk_widget_add_css_class(lblItemsHeader, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblItemsHeader), 0.0f);
    gtk_box_append(GTK_BOX(card), lblItemsHeader);

    for (const auto& d : order.details) {
        GtkWidget* iRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* n = gtk_label_new(d.phoneName.c_str());
        gtk_widget_set_hexpand(n, TRUE);
        gtk_label_set_xalign(GTK_LABEL(n), 0.0f);

        GtkWidget* q = gtk_label_new(("x" + std::to_string(d.quantity)).c_str());
        GtkWidget* p = gtk_label_new(UIComponents::formatVND(d.subtotal).c_str());
        gtk_widget_add_css_class(p, "stat-label");

        gtk_box_append(GTK_BOX(iRow), n);
        gtk_box_append(GTK_BOX(iRow), q);
        gtk_box_append(GTK_BOX(iRow), p);
        gtk_box_append(GTK_BOX(card), iRow);
    }

    if (order.discountAmount > 0) {
        addRow("Giảm giá:", "-" + UIComponents::formatVND(order.discountAmount));
    }
    addRow("Tổng cộng:", UIComponents::formatVND(order.total));

    GtkWidget* btnClose = gtk_button_new_with_label("Đóng");
    gtk_widget_add_css_class(btnClose, "btn-secondary");
    UIComponents::setButtonCursor(btnClose);
    gtk_widget_set_halign(btnClose, GTK_ALIGN_END);
    g_signal_connect_swapped(btnClose, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(card), btnClose);

    gtk_window_set_child(GTK_WINDOW(win), card);
    gtk_window_present(GTK_WINDOW(win));
}

// ================= 5. HỒ SƠ CÁ NHÂN & ĐIỂM TÍCH LŨY =================

void CustomerScreens::buildProfileView() {
    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* header = UIComponents::createSectionHeader("Hồ sơ cá nhân", "Quản lý thông tin tài khoản, điểm thưởng và hạng thành viên của bạn.");
    gtk_box_append(GTK_BOX(content), header);

    const UserSessionViewModel& user = controller->getCurrentUser();
    CustomerItemViewModel cust = controller->getCustomerById(user.userId);

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    // Left Card: Form edit profile
    GtkWidget* formCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(formCard, "card");
    gtk_widget_set_hexpand(formCard, TRUE);

    GtkWidget* lblFormTitle = gtk_label_new("Chỉnh sửa thông tin liên hệ");
    gtk_widget_add_css_class(lblFormTitle, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblFormTitle), 0.0f);
    gtk_box_append(GTK_BOX(formCard), lblFormTitle);

    GtkWidget* entryName = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryName), user.fullName.c_str());
    gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField("Họ và tên", entryName));

    GtkWidget* entryPhone = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryPhone), user.phone.c_str());
    gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField("Số điện thoại", entryPhone));

    GtkWidget* entryEmail = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryEmail), user.email.c_str());
    gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField("Email", entryEmail));

    GtkWidget* entryAddr = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entryAddr), user.address.c_str());
    gtk_box_append(GTK_BOX(formCard), UIComponents::createFormField("Địa chỉ mặc định", entryAddr));

    GtkWidget* btnSave = gtk_button_new_with_label("💾 Lưu thay đổi hồ sơ");
    gtk_widget_add_css_class(btnSave, "btn-primary");
    UIComponents::setButtonCursor(btnSave);
    gtk_widget_set_margin_top(btnSave, 8);

    struct ProfileSaveContext {
        CustomerScreens* screens;
        GtkWidget* eName;
        GtkWidget* ePhone;
        GtkWidget* eEmail;
        GtkWidget* eAddr;
    };
    ProfileSaveContext* pctx = new ProfileSaveContext{this, entryName, entryPhone, entryEmail, entryAddr};
    g_signal_connect_data(btnSave, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        ProfileSaveContext* ctx = static_cast<ProfileSaveContext*>(d);
        const char* n = gtk_editable_get_text(GTK_EDITABLE(ctx->eName));
        const char* p = gtk_editable_get_text(GTK_EDITABLE(ctx->ePhone));
        const char* e = gtk_editable_get_text(GTK_EDITABLE(ctx->eEmail));
        const char* a = gtk_editable_get_text(GTK_EDITABLE(ctx->eAddr));

        OperationResult res = ctx->screens->controller->updateProfile(n ? n : "", p ? p : "", e ? e : "", a ? a : "");
        if (res.success) {
            UIComponents::showSuccessDialog(ctx->screens->shell->getWindow(), "Thành công", res.message);
            ctx->screens->refreshProfile();
        } else {
            UIComponents::showErrorDialog(ctx->screens->shell->getWindow(), "Lỗi", res.message);
        }
    }), pctx, [](gpointer d, GClosure*) { delete static_cast<ProfileSaveContext*>(d); }, (GConnectFlags)0);
    gtk_box_append(GTK_BOX(formCard), btnSave);

    GtkWidget* btnChangePw = gtk_button_new_with_label("🔑 Đổi mật khẩu");
    gtk_widget_add_css_class(btnChangePw, "btn-secondary");
    UIComponents::setButtonCursor(btnChangePw);
    g_signal_connect(btnChangePw, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
        CustomerScreens* self = static_cast<CustomerScreens*>(d);
        UIComponents::showChangePasswordDialog(self->shell->getWindow(), self->controller);
    }), this);
    gtk_box_append(GTK_BOX(formCard), btnChangePw);

    gtk_grid_attach(GTK_GRID(grid), formCard, 0, 0, 1, 1);

    // Right Card: Loyalty Points & Rewards
    GtkWidget* loyaltyCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(loyaltyCard, "card");
    gtk_widget_set_size_request(loyaltyCard, 360, -1);

    GtkWidget* lblLoyalty = gtk_label_new("Chương trình Khách hàng thân thiết");
    gtk_widget_add_css_class(lblLoyalty, "card-title");
    gtk_label_set_xalign(GTK_LABEL(lblLoyalty), 0.0f);
    gtk_box_append(GTK_BOX(loyaltyCard), lblLoyalty);

    // Rank Badge đặc tả màu
    GtkWidget* rankBadge = UIComponents::createRankBadge(cust.membershipTier);
    gtk_box_append(GTK_BOX(loyaltyCard), rankBadge);

    GtkWidget* pointBox = UIComponents::createStatCard("Điểm tích lũy hiện tại", std::to_string(cust.points) + " điểm", "Hạng hội viên: " + cust.membershipTier, "🌟");
    gtk_box_append(GTK_BOX(loyaltyCard), pointBox);

    GtkWidget* lblRules = gtk_label_new("Quy tắc tích điểm:\n• Mỗi 100.000 đ thanh toán thành công = 1 điểm tích lũy\n• Từ 50 điểm: Thăng hạng Bạc\n• Từ 200 điểm: Thăng hạng Vàng\n• Từ 500 điểm: Thăng hạng Kim Cương (VIP)\n• Đổi điểm để nhận voucher ưu đãi lên đến 15%!");
    gtk_widget_add_css_class(lblRules, "page-subtitle");
    gtk_label_set_wrap(GTK_LABEL(lblRules), TRUE);
    gtk_label_set_xalign(GTK_LABEL(lblRules), 0.0f);
    gtk_box_append(GTK_BOX(loyaltyCard), lblRules);

    gtk_grid_attach(GTK_GRID(grid), loyaltyCard, 1, 0, 1, 1);
    gtk_box_append(GTK_BOX(content), grid);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(profileView), content);
}

void CustomerScreens::refreshProfile() {
    buildProfileView();
}

// ================= 6. BẢO HÀNH =================

void CustomerScreens::buildWarrantyView() {
    warrantyView = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(warrantyView), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(content, "page-container");

    GtkWidget* btnNew = gtk_button_new_with_label("➕ Gửi yêu cầu bảo hành mới");
    gtk_widget_add_css_class(btnNew, "btn-primary");
    UIComponents::setButtonCursor(btnNew);
    g_signal_connect_swapped(btnNew, "clicked", G_CALLBACK(+[](CustomerScreens* s) {
        s->showCreateWarrantyModal();
    }), this);

    GtkWidget* header = UIComponents::createSectionHeader("Trung tâm bảo hành", "Tra cứu và tạo yêu cầu sửa chữa, bảo hành thiết bị chính hãng.", btnNew);
    gtk_box_append(GTK_BOX(content), header);

    std::vector<UIComponents::TableColumnSpec> warCols = {
        {"Mã phiếu", 120, 0.0f, false},
        {"Thiết bị", 180, 0.0f, false},
        {"Ngày gửi", 130, 0.0f, false},
        {"Trạng thái", 140, 0.5f, false},
        {"Hiện tượng lỗi & Phản hồi kỹ thuật", 0, 0.0f, true}
    };
    gtk_box_append(GTK_BOX(content), UIComponents::createTableHeaderRow(warCols));

    warrantyListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(content), warrantyListBox);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(warrantyView), content);
    refreshWarranty();
}

void CustomerScreens::refreshWarranty() {
    GtkWidget* child = gtk_widget_get_first_child(warrantyListBox);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(warrantyListBox), child);
        child = next;
    }

    const UserSessionViewModel& user = controller->getCurrentUser();
    std::vector<WarrantyItemViewModel> warranties = controller->getCustomerWarranties(user.userId);

    if (warranties.empty()) {
        GtkWidget* empty = UIComponents::createEmptyState("🛡️", "Chưa có yêu cầu bảo hành nào",
            "Thiết bị của bạn hoạt động bình thường! Nếu gặp sự cố, bạn có thể tạo yêu cầu bảo hành bất cứ lúc nào.",
            "Tạo yêu cầu bảo hành", G_CALLBACK(+[](GtkButton* b, gpointer d) {
                (void)b;
                CustomerScreens* self = static_cast<CustomerScreens*>(d);
                self->showCreateWarrantyModal();
            }), this);
        gtk_box_append(GTK_BOX(warrantyListBox), empty);
        return;
    }

    for (size_t i = 0; i < warranties.size(); ++i) {
        const auto& w = warranties[i];
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "table-row");
        gtk_widget_add_css_class(row, (i % 2 == 0) ? "row-even" : "row-odd");

        // Mã phiếu
        GtkWidget* lblId = gtk_label_new(w.id.c_str());
        gtk_widget_add_css_class(lblId, "product-name");
        gtk_widget_set_size_request(lblId, 120, -1);
        gtk_label_set_xalign(GTK_LABEL(lblId), 0.0f);
        gtk_widget_set_valign(lblId, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), lblId);

        // Thiết bị
        GtkWidget* prodLbl = gtk_label_new(w.phoneName.c_str());
        gtk_widget_set_size_request(prodLbl, 180, -1);
        gtk_label_set_xalign(GTK_LABEL(prodLbl), 0.0f);
        gtk_widget_set_valign(prodLbl, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(row), prodLbl);

        // Ngày gửi
        GtkWidget* timeLbl = gtk_label_new(w.requestDate.c_str());
        gtk_widget_set_size_request(timeLbl, 130, -1);
        gtk_label_set_xalign(GTK_LABEL(timeLbl), 0.0f);
        gtk_widget_set_valign(timeLbl, GTK_ALIGN_CENTER);
        gtk_widget_add_css_class(timeLbl, "page-subtitle");
        gtk_box_append(GTK_BOX(row), timeLbl);

        // Trạng thái badge
        GtkWidget* badge = UIComponents::createWarrantyStatusBadge(w.status);
        GtkWidget* badgeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(badgeBox, 140, -1);
        gtk_widget_set_valign(badgeBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(badgeBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(badgeBox), badge);
        gtk_box_append(GTK_BOX(row), badgeBox);

        // Mô tả lỗi & phản hồi
        GtkWidget* descBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_hexpand(descBox, TRUE);
        gtk_widget_set_valign(descBox, GTK_ALIGN_CENTER);
        GtkWidget* descLbl = gtk_label_new(("Hiện tượng lỗi: " + w.issueDescription).c_str());
        gtk_label_set_xalign(GTK_LABEL(descLbl), 0.0f);
        gtk_box_append(GTK_BOX(descBox), descLbl);

        if (!w.technicianNote.empty()) {
            GtkWidget* techLbl = gtk_label_new(("Kỹ thuật viên: " + w.technicianNote).c_str());
            gtk_widget_add_css_class(techLbl, "stat-trend");
            gtk_label_set_xalign(GTK_LABEL(techLbl), 0.0f);
            gtk_box_append(GTK_BOX(descBox), techLbl);
        }
        gtk_box_append(GTK_BOX(row), descBox);

        gtk_box_append(GTK_BOX(warrantyListBox), row);
    }
}

void CustomerScreens::showCreateWarrantyModal() {
    GtkWidget* win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), "Tạo yêu cầu bảo hành mới");
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), shell->getWindow());
    gtk_window_set_default_size(GTK_WINDOW(win), 460, 360);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(card, "dialog-content");
    gtk_widget_set_margin_top(card, 16);
    gtk_widget_set_margin_bottom(card, 16);
    gtk_widget_set_margin_start(card, 16);
    gtk_widget_set_margin_end(card, 16);

    GtkWidget* title = gtk_label_new("Phiếu yêu cầu bảo hành");
    gtk_widget_add_css_class(title, "page-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(card), title);

    std::vector<ProductItemViewModel> products = controller->getProducts();
    std::vector<const char*> prodNames;
    for (const auto& p : products) {
        prodNames.push_back(p.name.c_str());
    }
    prodNames.push_back(nullptr);

    GtkWidget* comboProd = gtk_drop_down_new_from_strings(prodNames.data());
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Chọn thiết bị cần bảo hành *", comboProd));

    GtkWidget* entryIssue = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entryIssue), "Mô tả chi tiết hiện tượng lỗi (vd: không lên nguồn, sọc màn hình)...");
    gtk_box_append(GTK_BOX(card), UIComponents::createFormField("Mô tả lỗi gặp phải *", entryIssue));

    GtkWidget* btnRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btnRow, GTK_ALIGN_END);

    GtkWidget* btnCancel = gtk_button_new_with_label("Hủy");
    gtk_widget_add_css_class(btnCancel, "btn-secondary");
    UIComponents::setButtonCursor(btnCancel);
    g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy), win);
    gtk_box_append(GTK_BOX(btnRow), btnCancel);

    struct WarSubmitContext {
        CustomerScreens* screens;
        GtkWidget* win;
        GtkWidget* combo;
        GtkWidget* entry;
        std::vector<ProductItemViewModel> prods;
    };
    WarSubmitContext* ctx = new WarSubmitContext{this, win, comboProd, entryIssue, products};

    GtkWidget* btnSubmit = gtk_button_new_with_label("Gửi yêu cầu");
    gtk_widget_add_css_class(btnSubmit, "btn-primary");
    UIComponents::setButtonCursor(btnSubmit);
    g_signal_connect_data(btnSubmit, "clicked", G_CALLBACK(+[](GtkButton* b, gpointer d) {
        (void)b;
        WarSubmitContext* c = static_cast<WarSubmitContext*>(d);
        guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(c->combo));
        const char* issue = gtk_editable_get_text(GTK_EDITABLE(c->entry));

        if (sel < c->prods.size() && issue && strlen(issue) > 0) {
            CreateWarrantyRequest req;
            req.phoneId = c->prods[sel].id;
            req.issueDescription = issue;
            OperationResult res = c->screens->controller->createWarranty(req);
            if (res.success) {
                c->screens->refreshWarranty();
                UIComponents::showSuccessDialog(c->screens->shell->getWindow(), "Đã tiếp nhận", res.message);
                gtk_window_destroy(GTK_WINDOW(c->win));
            } else {
                UIComponents::showErrorDialog(c->screens->shell->getWindow(), "Lỗi", res.message);
            }
        } else {
            UIComponents::showWarningDialog(c->screens->shell->getWindow(), "Thiếu thông tin", "Vui lòng nhập mô tả lỗi!");
        }
    }), ctx, [](gpointer d, GClosure*) { delete static_cast<WarSubmitContext*>(d); }, (GConnectFlags)0);

    gtk_box_append(GTK_BOX(btnRow), btnSubmit);
    gtk_box_append(GTK_BOX(card), btnRow);

    gtk_window_set_child(GTK_WINDOW(win), card);
    gtk_window_present(GTK_WINDOW(win));
}
