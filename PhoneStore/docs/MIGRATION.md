# MIGRATION.md — Hướng dẫn chuyển đổi dữ liệu và định dạng tệp tin

Tài liệu này ghi nhận sự thay đổi định dạng các file lưu trữ dữ liệu văn bản và quy tắc tương thích ngược (backward compatibility) trong `PhoneStore`.

---

## 1. Định dạng file sản phẩm (`Products.txt` / `phones.txt`)

### 1.1. Cấu trúc trường dữ liệu (10 trường)

Định dạng chuẩn mới bao gồm 10 trường ngăn cách bởi ký tự `|`:

```
id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications|listed
```

- **Trường 10 (`listed`):** Nhận giá trị `1` (đã đăng bán, hiển thị cho khách hàng) hoặc `0` (bản nháp, tạo khi nhập kho sản phẩm mới, chỉ hiển thị cho nhân viên/quản trị viên).
- Hàm `Phone::toFileString()` luôn ghi đầy đủ cả 10 trường, kể cả khi `manufacturer` hoặc `specifications` rỗng, nhằm bảo đảm đúng vị trí chỉ số cột.

### 1.2. Tương thích ngược

Hàm `Phone::fromFileLine(const std::string& line)` hỗ trợ đầy đủ các phiên bản dữ liệu cũ:
- **7 trường (bản sơ khai):** `id|name|brand|os|importPrice|sellingPrice|quantity`
- **8 trường:** thêm `manufacturer`
- **9 trường:** thêm `specifications`
- **10 trường (hiện tại):** thêm `listed`

**Quy tắc:** Nếu dòng dữ liệu có ít hơn 10 trường (thiếu trường `listed`), hệ thống tự động gán mặc định `listed = true`. Toàn bộ dữ liệu điện thoại đã có từ trước sẽ tiếp tục hiển thị bình thường cho khách hàng mà không bị ẩn nhầm.

### 1.3. Lưu ý về ký tự phân cách

- Do sử dụng ký tự `|` làm dấu phân cách cột, trường `specifications` (thông số kỹ thuật) không được chứa ký tự `|` chưa được chuẩn hóa, để tránh làm lệch số lượng trường khi phân tích chuỗi (`split`).

---

## 2. Định dạng file đơn hàng (`Orders.txt` / `orders.txt`)

### 1.1. Cấu trúc trường dữ liệu

Định dạng mở rộng bao gồm tối đa 14 trường:

```
id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details|inventoryState|customerConfirmed|discountCode
```

- **Trường 1-11 (kế thừa định dạng cũ):**
  - `id`: Mã đơn hàng (`ORD-DDMMYYYY-xxxxxx`)
  - `customerId`: Mã khách hàng
  - `deliveryMethod`: Phương thức giao hàng (`0`: PICKUP, `1`: DELIVERY)
  - `paymentMethod`: Phương thức thanh toán (`0`: CASH, `1`: BANK_TRANSFER, `2`: CREDIT_CARD)
  - `status`: Trạng thái đơn (`0`: PENDING, `1`: CONFIRMED, ...)
  - `orderTime`: Thời gian đặt hàng (`DD/MM/YYYY HH:MM:SS`)
  - `discountAmount`: Số tiền giảm giá
  - `receiverName`, `phone`, `address`: Thông tin nhận hàng
  - `details`: Danh sách sản phẩm dạng `phoneId,phoneName,quantity,unitPrice;...`
- **Trường 12 (`inventoryState`):** Trạng thái tồn kho (`0`: NONE, `1`: RESERVED, `2`: DEDUCTED, `3`: RELEASED, `4`: RESTOCKED).
- **Trường 13 (`customerConfirmed`):** Khách hàng đã xác nhận nhận hàng (`1` hoặc `0`).
- **Trường 14 (`discountCode`):** Mã giảm giá đã áp dụng.

### 2.2. Tương thích ngược

- Khi đọc file cũ có 11 trường:
  - `inventoryState` mặc định là `InventoryState::NONE`.
  - `customerConfirmed` mặc định là `false` (nếu đơn hàng đã ở trạng thái `COMPLETED` thì tự động coi là đã xác nhận `true`).
  - `discountCode` mặc định là chuỗi rỗng `""`.
- Đã khắc phục lỗi cũ: `deliveryMethod` và `paymentMethod` đã được nạp và gán chính xác vào đối tượng `Order` thay vì bị bỏ qua.

