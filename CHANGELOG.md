# CHANGELOG - HỆ THỐNG QUẢN LÝ CỬA HÀNG ĐIỆN THOẠI (PBL2)

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
