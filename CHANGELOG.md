# CHANGELOG - HỆ THỐNG QUẢN LÝ CỬA HÀNG ĐIỆN THOẠI (PBL2)

---

## [v2.1.0] - 2026-10-15 — Khắc phục toàn vẹn dữ liệu & nghiệp vụ (A–G)

### 🐛 Sửa lỗi toàn vẹn dữ liệu & nghiệp vụ theo PBL2_PLAN.md
- **Đầu việc A (Thống nhất Role nhân viên):** Cập nhật `Employee::toFileString()` ghi mã 1-based (`1=Admin`, `2=Sale`, `3=Warehouse`) khớp `DATA_FORMAT_GUIDE.md` và `Employee::fromFileLine()`; bổ sung xử lý ngoại lệ an toàn `try-catch` cho `std::stoi`/`std::stod`.
- **Đầu việc B (Toàn vẹn tham chiếu & Dữ liệu mẫu):** Bổ sung các setter còn thiếu (`deliveryMethod`, `paymentMethod`, `shippingInfo`) và gán đầy đủ trong `Order::fromFileLine()`. Kiểm chứng toàn bộ dữ liệu mẫu trong `data/` không trùng khóa chính và khớp khóa ngoại.
- **Đầu việc C (Giao dịch đơn hàng & Tồn kho):** 
  - `OrderService::processNextPendingOrder()`: Bỏ qua đơn cũ không còn `PENDING` trong hàng đợi FIFO; kiểm tra tồn kho toàn bộ giỏ hàng trước khi xuất; trừ kho nguyên tử với cơ chế rollback tự động hoàn kho nếu có sản phẩm thất bại giữa chừng hoặc ghi file lỗi.
  - `OrderService::cancelOrder()`: Ngăn chặn hủy đơn `COMPLETED`; chỉ hoàn kho cho đơn `CONFIRMED` và trừ lại nếu lưu thất bại; chống hủy lặp lại nhiều lần.
- **Đầu việc D (Nhất quán RAM & File):** `PhoneRepository` và `OrderRepository` lưu snapshot dữ liệu RAM trước khi ghi đĩa; tự động rollback dữ liệu RAM về nguyên trạng nếu `saveToFile()` trả về thất bại.
- **Đầu việc E (BST và sản phẩm trùng giá):** `PhoneService` triển khai `PhonePriceKey` (Composite Key: `price + id`) cho cây nhị phân tìm kiếm `BST<PhonePriceKey, Phone>`; khắc phục hoàn toàn hiện tượng sản phẩm cùng mức giá bị đè mất khi lọc khoảng giá `filterByPriceRange()`.
- **Đầu việc F (Bảo vệ toàn vẹn tham chiếu):** Kết nối `ReferenceChecker` vào `PhoneService::deletePhone()`; từ chối xóa sản phẩm nếu đã phát sinh giao dịch trong Đơn hàng, Hóa đơn hoặc Phiếu nhập kho.
- **Đầu việc G (Validation & Dọn dẹp):** Bổ sung hàm kiểm tra tính hợp lệ của ngày tháng năm và năm nhuận trong `DateTime::fromDateString()`; dọn dẹp enum rỗng trong `ErrorMessage.h`; thêm header guard cho `CardService.h` và `InventoryService.h`.
- **Giao diện & Nạp dữ liệu:** Cập nhật `main.cpp` gọi `loadFromFile()` trên toàn bộ Repositories và kết nối `ReferenceChecker` vào `PhoneService` khi khởi động ứng dụng GTK4.

---

## [v2.0.0] - 2026-10-03 — GTK 4 Admin UI (Production)

### ✨ Giao diện quản trị GTK 4 hoàn chỉnh
- **`UI/AdminApp.h` & `UI/AdminApp.cpp`**: Lớp `AdminApp` dùng `GtkApplication` + `GtkApplicationWindow`.
- **`GtkHeaderBar`** với `GtkStackSwitcher` (tab Sản phẩm / Đơn hàng), nút Refresh, Toggle Theme (sáng/tối), About.
- **`GtkColumnView` + `GListStore` + `GtkSignalListItemFactory`**: bảng điện thoại (ID, Tên, Hãng, Đơn giá, Số lượng); tiền tệ có dấu phân cách hàng nghìn.
- **Tìm kiếm** theo tên, thống kê số sản phẩm / tổng tồn kho.
- **Action Panel** (`GtkGrid` + `GtkLabel` + `GtkEntry`): Thêm / Sửa / Xóa sản phẩm và Tạo đơn hàng mới qua `PhoneService` / `OrderService`.
- **Validate đầu vào** (bỏ trống tên, nhập chữ vào ô số, số âm...) và thông báo bằng `GtkAlertDialog` cho cả lỗi lẫn thành công.
- **Data binding**: bảng tự refresh ngay sau mọi thao tác thành công.
- **CSS (`GtkCssProvider`)**: tiêu đề in đậm, màu cảnh báo, viền form bo góc.

### 🔧 Build & Hệ thống
- **`main.cpp`**: Entry point GTK4 — khởi tạo Repositories → Services → `AdminApp` → `g_application_run`.
- **`Makefile`**: `g++ -std=c++17`, `pkg-config gtk4`, auto-dependency (`-MMD -MP`), targets `all`, `run`, `clean`, `info`.

### 🐛 Sửa lỗi
- Bỏ pseudo-class `:invalid` không tồn tại trong GTK4 CSS (gây `Theme parser error`).
- Sửa compound-literal / cast `gpointer → GObject*` khi biên dịch C++17.

### 🗑️ Loại bỏ
- `UI/GtkAdminUI.*` (GTK3) và `main_gtk4.cpp` — thay thế hoàn toàn bởi `AdminApp` và `main.cpp`.

---

## [v1.0.0] - 2026-10-02 — GTK UI Layer & Final Integration

### ✨ GTK UI Mới
- **`GtkAdminUI.h` & `GtkAdminUI.cpp`**: Tạo cửa sổ giao diện đồ họa sử dụng GTK3.
- **TreeView**: Hiển thị danh sách sản phẩm thực tế lấy từ `PhoneService`.
- **Button Test**: Nút bấm giả lập luồng tạo đơn và duyệt đơn qua `OrderService`, tự động cập nhật kho hàng trên UI (có popup thông báo).
- **`main.cpp`**: Entry point mới khởi tạo toàn bộ Repositories, Services và GTK UI.
- **`Makefile`**: Cấu hình biên dịch chuẩn C++ kết hợp `pkg-config gtk+-3.0`.

---


## [v0.3.0] - 2026-10-02 — Service Layer Complete

### ✨ Service mới triển khai

| Service | Trạng thái | Chức năng chính |
|---------|-----------|-----------------|
| `DiscountService` | **MỚI** | validateCode, calculateDiscountAmount, consumeUsage, CRUD voucher |
| `AuthService` | **MỚI** | login() điều phối Employee+Customer, LoginResult struct |
| `InvoiceService` | **MỚI** | createInvoiceFromOrder(), one-invoice-per-order rule, auto ID |
| `SupplierService` | **MỚI** | CRUD nhà cung cấp + receiveStock() giao dịch nguyên tử |
| `ReportService` | **MỚI** | Tổng hợp báo cáo: doanh thu, đơn hàng, bán chạy, tồn kho thấp |

### 🔧 Service cải thiện

| Service | Thay đổi |
|---------|---------|
| `CustomerService` | Thêm `addPoints()`, `deductPoints()` (nghiệp vụ tích điểm), `validatePhone()` (10-11 số, bắt đầu 0) |
| `Check.h` | Sửa include `stockReceiptRepo.h` → `StockReceiptRepository.h` |

### 📐 Business Rules đã triển khai

#### 1. Nghiệp vụ Bán hàng / Đặt hàng (OrderService)
- **Tạo đơn:** Kiểm tra tồn kho qua `PhoneService::hasStock()` trước khi lưu đơn
- **Duyệt đơn (processNextPendingOrder):** FIFO Queue, kiểm tra lại tồn kho trước khi trừ, **Rollback nguyên tử** nếu trừ kho thất bại hoặc lưu đơn thất bại
- **Hủy đơn (cancelOrder):** Không cho hủy đơn COMPLETED; hoàn trả tồn kho nếu đơn đã CONFIRMED; đơn PENDING trong Queue sẽ bị skip khi processNext

#### 2. Nghiệp vụ Khuyến mãi (DiscountService + CustomerService)
- **Áp dụng voucher:** `validateCode()` kiểm tra: mã tồn tại → còn hiệu lực (ngày) → còn lượt → đạt giá trị đơn tối thiểu
- **Tính tiền giảm:** `calculateDiscountAmount()` — không làm thay đổi `usedCount`
- **Commit voucher:** `consumeUsage()` — chỉ tăng `usedCount` sau khi giao dịch lưu thành công; rollback nếu save file thất bại
- **Tích điểm:** `CustomerService::addPoints()` — cộng điểm sau thanh toán thành công, rollback in-memory nếu lưu file lỗi
- **Trừ điểm:** `CustomerService::deductPoints()` — kiểm tra đủ điểm trước khi trừ; không cho trừ xuống âm; rollback nếu lỗi

#### 3. Validation đầu vào (toàn Service Layer)
- **Số điện thoại:** 10-11 chữ số, bắt đầu bằng `0` (`CustomerService::validatePhone()`)
- **Giá bán:** phải > 0; Giá nhập: không được âm (`PhoneService::addPhone/updatePhone`)
- **Lương nhân viên:** không được âm (`EmployeeService::addEmployee/updateEmployee`)
- **ID:** không rỗng, chỉ chứa chữ cái/số/dấu gạch ngang (`isValidID()`)
- **Voucher:** giá trị > 0; tỷ lệ % ≤ 100; ngày kết thúc ≥ ngày bắt đầu; maxUsage > 0

#### 4. Tính toàn vẹn tham chiếu
- **Không xóa Phone** đã có trong Order/Invoice/StockReceipt (ReferenceChecker)
- **Không xóa Supplier** đang có phiếu nhập kho tham chiếu
- **Không xóa Admin** nếu là Admin duy nhất trong hệ thống

#### 5. Nghiệp vụ Hóa đơn (InvoiceService)
- **One invoice per order:** Mỗi orderId chỉ được xuất đúng 1 hóa đơn
- **Chỉ từ đơn CONFIRMED/COMPLETED:** Đơn PENDING/CANCELLED không được xuất hóa đơn
- **discountAmount hợp lệ:** Không âm và không vượt subtotal đơn hàng
- **Auto ID:** Tự sinh ID theo format `INV-XXX` tăng dần

#### 6. Nghiệp vụ Nhập kho (SupplierService::receiveStock)
- **Giao dịch nguyên tử:** Validate TẤT CẢ detail trước → lưu phiếu → tăng kho từng sản phẩm
- **Rollback:** Nếu tăng kho thất bại → hoàn tồn kho đã tăng + xóa phiếu đã lưu
- **Validate detail:** quantity > 0; importPrice >= 0; productId tồn tại trong PhoneService
- **Auto ID:** Tự sinh ID theo format `REC-XXX` tăng dần

#### 7. Báo cáo (ReportService)
- **Doanh thu** chỉ tính từ đơn `COMPLETED` (không tính CONFIRMED chưa giao)
- **Bán chạy:** Tổng hợp từ detail các đơn COMPLETED, sắp xếp giảm dần
- **Tồn kho thấp:** Sản phẩm với quantity ≤ threshold (mặc định 5)

---

## [v0.2.0] - 2026-10-01 — Repository Layer Complete

### ✨ Thêm mới
- **`StockReceiptRepository.h`** — File mới, thay thế `stockReceiptRepo.h` (chuẩn hóa tên PascalCase). Dùng `ImportReceipt::fromFileLine()` và `toFileString()`.
- **`AccountRepository.h`** — Implement đầy đủ từ file rỗng. Hoạt động như facade tổng hợp tìm kiếm từ Customer & Employee. Có `authenticate()`, `findEmployeeAccount()`, `findCustomerAccount()`, `usernameExists()`.

### 🔧 Cải thiện / Sửa lỗi Repository

| File | Vấn đề cũ | Thay đổi |
|------|-----------|----------|
| `PhoneRepository.h` | Parse thủ công, không dùng `fromFileLine()`; header thiếu `manufacturer|specifications`; không tự tạo file | Dùng `Phone::fromFileLine()`; header đủ 9 cột; thêm `createEmptyFile()`; thêm `searchByName()`, `findByBrand()`, `existsById()` |
| `CustomerRepository.h` | Parse thủ công; không đọc `points`; header ghi `role` thay vì `points`; không tự tạo file | Dùng `Customer::fromFileLine()`; đọc đúng `points`; header chuẩn; thêm `createEmptyFile()`, `findByUsername()`, `searchByName()`, `existsById()` |
| `EmployeeRepository.h` | Parse thủ công; không đọc `salary`; header thiếu cột `salary`; không tự tạo file | Dùng `Employee::fromFileLine()`; đọc đúng `salary`; header 11 cột; thêm `createEmptyFile()`, `findByUsername()`, `findByRole()`, `existsById()` |
| `OrderRepository.h` | `loadFromFile()` parse thủ công; `saveToFile()` tự build chuỗi thủ công thay vì dùng `toFileString()`; không tự tạo file | Dùng `Order::fromFileLine()` và `Order::toFileString()`; thêm `createEmptyFile()`, `findByCustomerId()`, `findByStatus()`, `existsById()` |
| `DiscountRepository.h` | Parse thủ công; không tự tạo file | Dùng `DiscountCode::fromFileLine()`; thêm `createEmptyFile()`, `findValidCodes()`, `existsByCode()` |
| `InvoiceRepository.h` | Parse thủ công; không tự tạo file | Dùng `Invoice::fromFileLine()` và `toFileString()`; thêm `createEmptyFile()`, `findByCustomerId()`, `findByEmployeeId()`, `existsById()` |
| `SupplierRepository.h` | Parse thủ công; không tự tạo file | Dùng `Supplier::fromFileLine()` và `toFileString()`; thêm `createEmptyFile()`, `searchByCompanyName()`, `existsById()` |

### 📐 Cơ chế xử lý ngoại lệ I/O (áp dụng cho tất cả Repository)
- **File không tồn tại:** Tự động gọi `createEmptyFile()` để tạo file mới với header đúng định dạng
- **Dòng dữ liệu bị lỗi:** Bắt `std::exception` và `...`, in cảnh báo kèm số dòng, tiếp tục đọc
- **Không thể ghi file:** In thông báo lỗi và trả về `false`
- **Thêm ID trùng:** In cảnh báo và trả về `false` mà không thêm

### 🔗 Cập nhật tích hợp
- **`PBL2.h`** — Thêm include cho 4 repository mới: `AccountRepository`, `SupplierRepository`, `InvoiceRepository`, `StockReceiptRepository`

---

## [v0.1.0] - 2026-10-01 — Entity Layer Complete

### ✨ Thêm mới / Sửa lỗi Models

| File | Thay đổi |
|------|---------|
| `Phone.h` | Thêm `manufacturer`/`specifications` vào `toFileString()` (9 cột); thêm `fromFileLine()` |
| `Customer.h` | Sửa bug hardcode `|4|` → dùng `points` thực tế; thêm `fromFileLine()` |
| `Employee.h` | Thêm `salary` vào `toFileString()`; thêm `fromFileLine()` factory (tạo đúng subclass) |
| `Supplier.h` | Thêm `fromFileLine()` parser |
| `Order.h` | Thêm `toFileString()` và `fromFileLine()` (trước đó chưa có) |
| `Invoice.h` | Thêm `fromFileLine()` parser |
| `ImportReceipt.h` | Thêm `#include StringHelper.h`; thêm `fromFileLine()` |
| `DiscountCode.h` | Thêm `fromFileLine()` parser |

### 📁 Data Files
- `Products.txt` — Thêm 2 cột `manufacturer`, `specifications` (từ 7 → 9 cột)
- `Customers.txt` — Đổi header cột 9: `role` → `points`; cập nhật điểm tích lũy
- `Employee.txt` — Thêm cột `salary` với giá trị mẫu
- `DATA_FORMAT_GUIDE.md` — Cập nhật đầy đủ thay đổi + bảng mapping Enum→Int
