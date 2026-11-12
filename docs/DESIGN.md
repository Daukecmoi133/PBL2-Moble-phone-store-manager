<!-- ---
name: PBL2 Phone Store Manager
version: 0.1.0
platform: Desktop C++17 + GTK (GTK CSS)
scope: Màn hình "Tổng quan" của Khách hàng (áp dụng dần cho Nhân viên / Admin)
colors:
  navy-900: "#0F172A"   # header, sidebar
  navy-800: "#1E293B"
  blue-600: "#2563EB"   # primary
  blue-700: "#1D4ED8"   # primary hover
  blue-100: "#DBEAFE"
  sky-400:  "#38BDF8"   # logo / accent trên nền tối
  red-600:  "#DC2626"   # destructive (Đăng xuất)
  red-100:  "#FEE2E2"
  green-700: "#15803D"  # tiền / trạng thái tốt
  green-100: "#DCFCE7"
  amber-600: "#D97706"  # cảnh báo / chờ xử lý
  slate-900: "#0F172A"  # text chính
  slate-600: "#475569"  # text phụ  (>= 4.5:1 trên nền trắng)
  slate-400: "#94A3B8"  # chỉ dùng cho placeholder / disabled
  slate-200: "#E2E8F0"  # border
  slate-100: "#F1F5F9"  # nền nút phụ
  slate-50:  "#F8FAFC"  # nền nội dung
  white:     "#FFFFFF"  # nền card
typography:
  font-family: "Inter, 'Segoe UI', 'Noto Sans', sans-serif"   # phải hỗ trợ tiếng Việt có dấu
  mono-family: "'JetBrains Mono', 'Cascadia Mono', monospace" # chỉ cho mã (voucher, ID)
  scale:
    caption:  "12px"  # nhỏ nhất được phép
    body:     "14px"
    body-lg:  "16px"
    title-sm: "18px"
    title:    "22px"
    metric:   "28px"  # số liệu lớn trên thẻ thống kê
  weight: { regular: 400, medium: 500, semibold: 600, bold: 700 }
spacing: { xs: 4, sm: 8, md: 12, lg: 16, xl: 24, xxl: 32 }   # px, bội số của 4
radius:  { sm: 6, md: 10, lg: 14, pill: 999 }
elevation:
  card: "0 1px 2px rgba(15,23,42,.06)"
  card-hover: "0 4px 12px rgba(15,23,42,.10)"
motion:
  fast: "120ms"
  base: "200ms"
  easing: "ease-out"
components:
  button-primary:   { bg: blue-600, text: white, radius: md, height: 36px, padding: "0 16px" }
  button-secondary: { bg: white, text: slate-900, border: slate-200, radius: md, height: 36px }
  button-danger:    { bg: red-600, text: white, radius: md, height: 36px }
  card:             { bg: white, border: slate-200, radius: lg, padding: 16px }
  stat-card:        { extends: card, accent: "viền trái 3px theo màu ngữ nghĩa" }
  badge:            { radius: pill, padding: "2px 10px", font: caption }
---

# DESIGN.md — PBL2 Phone Store Manager

Tài liệu này là **nguồn chuẩn duy nhất** về giao diện. Mọi màn hình phải dùng token ở phần front matter,
không dùng mã màu / cỡ chữ / khoảng cách tự đặt.

> [!IMPORTANT]
> **Hướng dẫn cho agent thực hiện (Antigravity):** file này CHỈ mô tả những gì cần sửa.
> Người viết file này chưa chạm vào mã nguồn. Hãy làm theo mục **4. Danh sách việc cần làm**
> theo thứ tự ưu tiên, và tick vào ô khi xong.

> [!NOTE]
> **Đã xác định vị trí giao diện (cập nhật):** mã giao diện là **GTK4 (C API)** nằm ở thư mục `UI/`:
> màn hình khách `UI/screens/CustomerScreens.cpp`, admin `UI/screens/AdminScreens.cpp`,
> CSS `UI/styles/AppCss.h`, thành phần dùng chung `UI/components/UIComponents.*`.
> Thư mục `PhoneStore/src/view/gtk` chỉ là chỗ trống, bỏ qua. Mục 2 bên dưới nói GTK 3 là chưa đúng: dự án dùng GTK4.

---

## 1. Nguyên tắc thiết kế

1. **Rõ ràng trước, đẹp sau**: chữ đọc được, tương phản đủ, thông tin quan trọng nổi bật.
2. **Nhất quán**: cùng một việc thì cùng một kiểu (nút, thẻ, badge, khoảng cách).
3. **Không rập khuôn**: không dùng emoji làm icon chức năng; dùng bộ icon thống nhất.
4. **Mọi phần tử bấm được phải trông giống bấm được** (hover, con trỏ tay, trạng thái focus).
5. **Không có dữ liệu "lạ"**: không hiển thị giá trị vô nghĩa (ví dụ "từ 0 đ").

## 2. Quy ước kỹ thuật GTK CSS

- Khai báo màu bằng `@define-color` (tên theo token, ví dụ `@define-color blue_600 #2563EB;`). Không dùng `var(--x)`
  trừ khi phiên bản GTK hỗ trợ (kiểm tra `gtk_get_minor_version()`; GTK 3 không hỗ trợ).
- GTK CSS **không có** flexbox / grid / `gap`. Khoảng cách làm bằng `Gtk::Box::set_spacing`,
  `margin-*`, `padding`, `Gtk::Grid::set_row/column_spacing`.
- Tách CSS ra file riêng (ví dụ `PhoneStore/src/view/gtk/theme.css`), nạp một lần ở khởi động qua
  `Gtk::CssProvider` với `GTK_STYLE_PROVIDER_PRIORITY_APPLICATION`. Không viết CSS inline rải rác trong code C++.
- Đặt tên class theo vai trò: `.btn-primary`, `.btn-secondary`, `.btn-danger`, `.card`, `.stat-card`,
  `.stat-card--purple`, `.badge`, `.badge--vip`, `.sidebar-item`, `.sidebar-item--active`, `.text-muted`...
- Font chữ phải hỗ trợ tiếng Việt có dấu; kiểm tra cả khi chạy trên Windows (MSYS2) và Linux.

---

## 3. Đánh giá hiện trạng (màn hình Tổng quan - Khách hàng)

Điểm ~6.5/10. Bố cục sạch, nhất quán, nhưng chữ nhỏ, còn khoảng trống lớn và có vài lỗi nội dung.

| # | Vấn đề | Mức | Vị trí |
|---|---|---|---|
| 1 | Chữ phụ / nhãn thẻ / mã giảm giá / HSD chỉ khoảng 9-10px | Cao | Thẻ thống kê, khối ưu đãi |
| 2 | Voucher hiển thị "Giảm 30% cho đơn hàng từ **0 đ**" | Cao | Khối "Ưu đãi & Mã giảm giá" |
| 3 | Hơn 40% phần dưới màn hình trống | Trung bình | Dưới các nút hành động |
| 4 | Thẻ thống kê nhìn như bấm được nhưng không điều hướng | Trung bình | 4 thẻ thống kê |
| 5 | Emoji dùng làm icon (menu, nút, tiêu đề, thẻ) | Trung bình | Toàn màn hình |
| 6 | Nút phụ "Xem giỏ hàng của tôi" tương phản thấp | Trung bình | Hàng nút hành động |
| 7 | Header chật: đồng hồ, chip người dùng, Đổi mật khẩu, Đăng xuất sát nhau | Thấp | Header |
| 8 | Nhãn thẻ khó hiểu: "Hạng TV", "1 phiếu", "Tổng lịch sử: 3 đơn" cạnh "0 đơn" | Thấp | 4 thẻ thống kê |
| 9 | Hạng VIP lặp ở badge góc phải và thẻ điểm | Thấp | Thẻ chào + thẻ điểm |
| 10 | Mã voucher và HSD dùng font mono lệch phong cách | Thấp | Khối ưu đãi |

---

## 4. Danh sách việc cần làm (theo thứ tự ưu tiên)

### P0 — Làm ngay

- [x] **T1. Tạo file theme chung.** Tạo file CSS riêng, khai báo toàn bộ `@define-color` theo token ở front matter,
  nạp một lần ở khởi động. Mọi bước dưới đây dùng file này.
- [x] **T2. Tăng cỡ chữ.** Quy tắc: không phần tử nào nhỏ hơn `12px` (caption).
  - Nhãn thẻ thống kê (chữ hoa nhỏ): 12px, weight 600, màu `slate-600`, letter-spacing nhẹ.
  - Dòng phụ dưới số liệu: 12px đến 13px.
  - Số liệu lớn: 28px, weight 700.
  - Nội dung thường: 14px. Tiêu đề khối: 18px, weight 700.
  - Mã voucher và HSD: tối thiểu 12px.
- [x] **T3. Sửa hiển thị voucher.**
  - Nếu giá trị đơn tối thiểu là 0 hoặc rỗng: hiển thị **"Áp dụng cho mọi đơn hàng"**, không in "từ 0 đ".
  - Nếu có mức tối thiểu: "Đơn từ 5.000.000 đ" (có dấu chấm ngăn cách nghìn).
  - Mã voucher đặt trong badge nền `blue-100`, chữ `blue-700`, 12px, weight 600.
  - HSD dùng font chung, màu `slate-600`, định dạng `dd/MM/yyyy`; nếu còn dưới 7 ngày thì đổi sang màu `amber-600`.
  - Thêm nút nhỏ **"Sao chép mã"** bên phải mỗi voucher.
  - Nếu không có voucher nào: hiện trạng thái rỗng "Hiện chưa có ưu đãi nào".

### P1 — Quan trọng

- [x] **T4. Làm thẻ thống kê bấm được.** Mỗi thẻ điều hướng tới mục tương ứng:
  Điểm tích lũy → Hồ sơ cá nhân; Giỏ hàng → Giỏ hàng; Đơn đang xử lý → Đơn hàng của tôi; Bảo hành → Trung tâm bảo hành.
  Thêm hover (đổ bóng `card-hover`, viền đậm hơn), con trỏ tay, trạng thái focus bàn phím, transition `200ms`.
- [x] **T5. Lấp khoảng trống phía dưới** bằng 1 đến 2 khối (chọn theo dữ liệu sẵn có):
  - **Đơn hàng gần đây** (3 đơn mới nhất: mã đơn, ngày, trạng thái bằng badge, tổng tiền) và liên kết "Xem tất cả".
  - **Sản phẩm nổi bật / mới về** (3 đến 4 thẻ có ảnh từ `data/Device/productImages`, tên, giá, nút "Xem").
  - **Tiến độ lên hạng**: thanh tiến độ "Còn X điểm để lên hạng tiếp theo" (thay cho dòng hạng bị lặp).
  - Nếu chưa có đơn nào: trạng thái rỗng kèm nút "Mua sắm ngay".
- [x] **T6. Thay emoji bằng icon thống nhất.** Dùng bộ SVG đơn sắc (ví dụ Lucide hoặc Phosphor, giấy phép MIT),
  nạp qua `Gtk::Image` / icon theme, kích thước 16px (menu, nút) và 20px (thẻ). Bỏ emoji ở: logo (📱),
  menu sidebar, tiêu đề khối (🎟️), nút (🛍️ 🛒 🔑 🚪), badge VIP (💎), lời chào (👋), icon thẻ (⭐ 🛒 📦 🛡️).
  Icon thẻ thống kê đặt trong ô tròn/vuông bo nền nhạt theo màu ngữ nghĩa.
- [x] **T7. Chỉnh nút.** Chuẩn hóa chiều cao 36px, radius `md`, padding ngang 16px.
  - Primary: nền `blue-600`, hover `blue-700`.
  - Secondary: nền trắng, viền `slate-200`, chữ `slate-900` weight 600, hover nền `slate-100`. (Thay nút xám nhạt hiện tại.)
  - Danger (Đăng xuất): giữ nền `red-600`, hover tối hơn; đặt tách biệt với "Đổi mật khẩu".
  - Tất cả có trạng thái `:hover`, `:active`, `:focus` (viền sáng 2px), `:disabled`.

### P2 — Hoàn thiện

- [x] **T8. Gọn header.** Gom **Đổi mật khẩu** và **Đăng xuất** vào menu thả xuống của chip người dùng (avatar + tên + hạng).
  Đồng hồ làm nhẹ hơn (không nền đậm, chữ 12px `slate-400` trên nền tối) hoặc chuyển xuống chân sidebar.
  Bỏ tiêu đề "Tổng quan" lặp ở header, hoặc đổi thành breadcrumb ("Khách hàng / Tổng quan").
- [x] **T9. Làm rõ nhãn thẻ.**
  - "ĐIỂM TÍCH LŨY & HẠNG TV" → "Điểm tích lũy"; dòng phụ "Hạng Kim Cương (VIP)".
  - "ĐƠN HÀNG ĐANG XỬ LÝ: 0 đơn" → dòng phụ "Đã đặt tất cả: 3 đơn".
  - "THIẾT BỊ & BẢO HÀNH: 1 phiếu" → "Phiếu bảo hành" với dòng phụ "1 phiếu đang hoạt động".
  - Số tiền tạm tính trong thẻ giỏ hàng dùng màu `green-700`, weight 600.
- [x] **T10. Bỏ lặp hạng VIP.** Giữ badge hạng ở thẻ chào (góc phải); thẻ điểm hiển thị thanh tiến độ lên hạng thay vì lặp tên hạng.
- [x] **T11. Sidebar.** Mục đang chọn: nền `blue-600`, chữ trắng, radius `md`. Hover: nền `navy-800`.
  Badge số lượng giỏ hàng: nền `red-600`, chữ trắng 12px, dạng pill, tối thiểu 18x18px.
  Nhóm nhãn "KHÁCH HÀNG" dùng 12px, màu `slate-400`.
- [x] **T12. Responsive cơ bản.** Cửa sổ tối thiểu khoảng 960x600. Nội dung nằm trong vùng cuộn
  (`Gtk::ScrolledWindow`) để không bị cắt khi thu nhỏ; 4 thẻ thống kê co đều theo chiều ngang (`homogeneous`).

---

## 5. Tiêu chí nghiệm thu

- [x] Không còn chữ nhỏ hơn 12px (kiểm tra bằng cách rà file CSS).
- [x] Tỷ lệ tương phản chữ thường / nền tối thiểu **4.5:1**, chữ lớn **3:1**.
- [x] Không còn emoji dùng làm icon chức năng.
- [x] Không còn chuỗi "từ 0 đ" trong giao diện.
- [x] Tất cả phần tử bấm được có hover, focus và con trỏ tay.
- [x] Toàn bộ màu, cỡ chữ, khoảng cách lấy từ token; không có mã màu hex lẻ trong code C++.
- [x] Hiển thị đúng tiếng Việt có dấu trên cả Windows và Linux.
- [x] Chụp lại màn hình Tổng quan sau khi sửa và so sánh với bản trước.

## 6. Ghi chú cho agent thực hiện

- Vị trí file giao diện / CSS thực tế: `UI/screens/CustomerScreens.cpp`, `UI/screens/MainShellWindow.cpp`, `UI/components/UIComponents.cpp`, `UI/styles/AppCss.h`, `UI/styles/theme.css` và `PhoneStore/src/view/gtk/theme.css`.
- Phiên bản GTK đang dùng (3 hay 4, gtkmm hay C API): GTK 4.14+ (C API qua C++17, trình biên dịch g++ trên MSYS2 ucrt64).
- Quyết định đã chọn ở T5 (khối nào lấp khoảng trống): Kết hợp cả 2 khối: Khối 1 - "Đơn hàng gần đây" (Recent Orders) hiển thị tối đa 3 đơn hàng mới nhất với mã đơn `cell-code-ord`, ngày `tabular-num`, tóm tắt sản phẩm, badge trạng thái, tổng tiền `green-700` và liên kết "Xem tất cả đơn hàng ›" (hoặc trạng thái rỗng thân thiện kèm nút "Mua sắm ngay"); Khối 2 - "Sản phẩm mới về & Nổi bật" (Featured Products) hiển thị 4 thẻ điện thoại kèm ảnh thumbnail, tên sản phẩm, giá bán và nút "Xem chi tiết"; đồng thời hiển thị thông tin tiến độ thăng hạng hội viên ở thẻ Điểm tích lũy.
- Bộ icon đã chọn ở T6: Bộ icon SVG Lucide đơn sắc (MIT license) chuẩn 16px/20px tại thư mục `UI/assets/icons/` (`smartphone.svg`, `layout-dashboard.svg`, `shopping-bag.svg`, `shopping-cart.svg`, `package.svg`, `ticket.svg`, `shield-check.svg`, `user.svg`, `key.svg`, `log-out.svg`, `copy.svg`, `star.svg`, `gem.svg`, `clock.svg`, `check.svg`, v.v.).

## 7. Áp dụng cho các màn hình khác

Sau khi hoàn tất màn Tổng quan - Khách hàng, dùng chung theme cho: Mua sắm điện thoại, Giỏ hàng,
Đơn hàng của tôi, Trung tâm bảo hành, Hồ sơ cá nhân, rồi tới giao diện Nhân viên / Admin.
Bảng dữ liệu (nếu có) dùng: header nền `slate-100`, dòng cao tối thiểu 40px, zebra nhẹ, số tiền căn phải.

--- -->

## 8. Đặc tả & Danh sách việc cần làm: Hệ thống Biểu đồ Thống kê (Admin Reports & Charts)

### 8.1. Nguyên tắc chọn lọc biểu đồ (Tránh "Wall of Numbers" & "Over-charting")

Biểu đồ chỉ có giá trị khi giúp Admin nhận diện **xu hướng (trend)** và **cơ cấu (composition)** trong 3 giây.
Tuyệt đối không vẽ biểu đồ tràn lan cho các chỉ số đơn lẻ hoặc dữ liệu tích lũy chậm.

| Danh mục | Vẽ biểu đồ? | Dạng biểu đồ | Dữ liệu & Ý nghĩa |
|---|:---:|---|---|
| **Doanh thu & Lợi nhuận theo thời gian** | **BẮT BUỘC** | **Cột đôi (Grouped Bar Chart)** hoặc **Đường (Line Chart)** | Trục X: ngày/tháng; Trục Y: tiền (VND). Thấy xu hướng tăng/giảm, chu kỳ bán chạy. |
| **Cơ cấu Doanh số theo Thương hiệu (Brand)** | **BẮT BUỘC** | **Thanh ngang phân bổ (Horizontal Distribution Bars)** hoặc **Donut** | Tỷ trọng % doanh thu giữa Apple, Samsung, Oppo, Xiaomi... Giúp định hướng vốn nhập hàng. |
| **Cơ cấu Trạng thái Đơn hàng** | **NÊN CÓ** | **Thanh tỷ lệ tích hợp (Segmented Progress Bar)** | Tỷ lệ Hoàn tất vs Chờ xử lý vs Đang giao vs Đã hủy. Đặt ngay trên Dashboard. |
| **Top sản phẩm bán chạy** | **KHÔNG VẼ BIỂU ĐỒ RIÊNG** | **Bảng dữ liệu hiện tại + Mini Progress Bar** | Giữ bảng `kReportTopProdCols` vì cần đối soát chính xác 5 cột số liệu, chỉ thêm thanh độ dài mini ở cột số lượng. |
| **Nhân sự & Khách hàng đăng ký** | **KHÔNG VẼ BIỂU ĐỒ** | **Thẻ KPI (Stat Cards)** | Dữ liệu biến động chậm, giữ nguyên thẻ chỉ số để tiết kiệm không gian. |

---

### 8.2. Quy cách kỹ thuật biểu đồ trong GTK4 (C++17)

1. **Bảng màu biểu đồ (Chart Tokens):**
   - Cột Doanh thu: `@define-color chart_revenue #2563EB;` (Xanh dương đậm)
   - Cột Lợi nhuận: `@define-color chart_profit #15803D;` (Xanh lá đậm)
   - Nhánh Hãng 1 (Apple): `#2563EB` | Hãng 2 (Samsung): `#0D9488` | Hãng 3 (Oppo): `#D97706` | Hãng 4 (Xiaomi): `#8B5CF6` | Hãng khác: `#94A3B8`
   - Nền lưới tọa độ: `#E2E8F0` (slate-200), nét mảnh 1px.
   - Text nhãn trục & chú thích: `12px`, font Inter/Segoe UI, màu `#64748B` (slate-500).

2. **Cách dựng trên GTK4:**
   - **Biểu đồ Cột theo thời gian:** Dùng `GtkDrawingArea` kết hợp hàm vẽ Cairo `gtk_drawing_area_set_draw_func()`. Tự động tính scale trục Y (Max value bo tròn lên 10%-20%), vẽ trục hoành, các cột đôi và nhãn ngày. Kích thước đề xuất: chiều cao tối thiểu 240px, co giãn theo chiều ngang.
   - **Biểu đồ Cơ cấu Hãng (Brand Share):** Xây dựng dưới dạng thẻ danh sách các thanh ngang phần trăm (`GtkBox` + widget con có độ rộng tỷ lệ kèm nhãn tên hãng, số máy bán và doanh thu VND). Cách này vừa trực quan, vừa chuẩn typography GTK CSS và không bị mờ chữ khi resize.
   - **Phản hồi tương tác (Reactivity):** Khi bấm nút `🔍 Lọc báo cáo` (chọn mốc từ ngày - đến ngày), biểu đồ phải tự động tính toán lại dữ liệu và gọi `gtk_widget_queue_draw()` để làm mới mượt mà.

---

### 8.3. Danh sách việc cần làm cho Anti (Thống kê & Biểu đồ)

- [x] **T13. Bổ sung ViewModel & Service phân tích dữ liệu biểu đồ:**
  - Mở rộng `UI/viewmodels/StatisticsViewModel.h`:
    - Struct `TimelineStatPoint`: `std::string label` (dd/MM), `double revenue`, `double profit`, `int orderCount`.
    - Struct `BrandShareStatItem`: `std::string brand`, `double revenue`, `int soldQuantity`, `double percentage`.
    - Thêm vào `DashboardStatsViewModel`: `std::vector<TimelineStatPoint> timelineStats` và `std::vector<BrandShareStatItem> brandStats`.
  - Cập nhật logic tính toán trong `AppController` / `StatisticsService` (`source/controllers/ReportController.cpp`): nhóm dữ liệu đơn hàng thành công theo mốc thời gian lọc và theo thương hiệu máy.

- [x] **T14. Xây dựng Custom Widget: Biểu đồ Cột Doanh thu & Lợi nhuận (Cairo DrawingArea):**
  - Tạo hàm helper trong `UIComponents`: `createRevenueBarChart(const std::vector<TimelineStatPoint>& points)`.
  - Cài đặt `draw_func` dùng Cairo:
    - Vẽ đường baseline và lưới ngang nhẹ (Gridlines) ở các mức 25%, 50%, 75%, 100%.
    - Vẽ cột Doanh thu (`#2563EB`) và cột Lợi nhuận (`#15803D`) bo góc nhẹ 3px ở đỉnh.
    - Vẽ nhãn ngày dưới chân cột (font 11-12px), định dạng tiền VND gọn (vd: "15.2 tr" / "2.4 tr") phía trên đầu cột hoặc khi hover.
    - Trạng thái rỗng: Nếu chưa có phát sinh doanh thu trong kỳ lọc, vẽ thông báo "Chưa có phát sinh doanh thu trong kỳ này".

- [x] **T15. Xây dựng Widget: Cơ cấu Doanh thu theo Hãng (Brand Revenue Distribution):**
  - Tạo hàm trong `UIComponents`: `createBrandDistributionCard(const std::vector<BrandShareStatItem>& brands)`.
  - Hiển thị danh sách các thương hiệu hàng đầu:
    - Mỗi dòng gồm: Icon/Logo Hãng + Tên Hãng + Thanh phần trăm độ dài tỉ lệ + Doanh thu VND + Tỷ lệ % (in đậm).
    - Thêm một thanh Segmented Bar tổng hợp ở đầu card thể hiện dải màu tương ứng của các hãng.

- [x] **T16. Tích hợp Biểu đồ vào màn hình Báo cáo (`AdminScreens::buildReportsView`):**
  - Chèn hàng biểu đồ (Grid 2 cột) nằm giữa **3 Thẻ KPI tổng hợp** và **Bảng Top sản phẩm bán chạy**:
    - Cột trái (chiếm 60-65% bề ngang): Biểu đồ Cột Doanh thu & Lợi nhuận theo ngày/tháng.
    - Cột phải (chiếm 35-40% bề ngang): Biểu đồ Cơ cấu doanh thu theo Thương hiệu.
  - Kết nối tín hiệu lọc: Khi người dùng bấm `🔍 Lọc báo cáo` hoặc `↺ Đặt lại`, biểu đồ được nạp lại dữ liệu tương ứng với `reportFromDate` và `reportToDate`.

- [x] **T17. Tối ưu Dashboard Admin (`AdminScreens::buildDashboardView`):**
  - Bổ sung thanh tỷ lệ trạng thái đơn hàng (Order Pipeline / Status Bar) ngay dưới thẻ KPI Đơn hàng:
    - Thanh màu chia đoạn: Hoàn tất (Xanh lá) | Đang giao (Xanh dương) | Chờ xử lý (Vàng cam) | Đã hủy (Xám/Đỏ).
    - Có chú thích số lượng rõ ràng để Admin biết ngay tình trạng nghẽn đơn.

- [x] **T18. Kiểm thử & Nghiệm thu:**
  - Kiểm tra co giãn responsive: Biểu đồ không bị tràn viền khi cửa sổ ứng dụng ở các kích thước khác nhau (960x600 đến Full HD).
  - Kiểm tra độ chính xác: Tổng % các hãng phải làm tròn đúng 100%; chiều cao cột tương quan đúng với số tiền thực trong `Invoices.txt` và `Orders.txt`.
  - Đảm bảo font chữ tiếng Việt hiển thị sắc nét, không bị lỗi font trên Windows (MSYS2) qua Cairo Pango Layout.
  - Test suite chuyên biệt `tests/test_admin_reports_charts.cpp` vượt qua 41/41 assertions.
