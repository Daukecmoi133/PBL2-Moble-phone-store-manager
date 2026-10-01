# TÀI LIỆU HƯỚNG DẪN ĐỊNH DẠNG DỮ LIỆU (DATA FORMAT SPECIFICATION)
## DỰ ÁN PBL2: HỆ THỐNG QUẢN LÝ CỬA HÀNG ĐIỆN THOẠI (DUT)

Tài liệu này quy chuẩn cấu trúc lưu trữ và quy tắc định dạng cho toàn bộ các tệp tin dữ liệu text (`.txt`) trong thư mục `data/`.

> **Quy ước chung:** Mỗi file có dòng đầu là **Header** (tên cột), các dòng tiếp theo là dữ liệu. Dùng ký tự `|` làm phân cách cột chính. Dùng `;` để phân cách nhiều item trong cùng 1 cột. Dùng `,` để phân cách các trường bên trong mỗi item.

---

### 1. Dữ liệu Sản phẩm: `data/Device/Products.txt`
- **Dòng 1 (Header):**
  `id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications`
- **Quy cách các trường:**
  1. `id`: Mã sản phẩm (chuỗi chữ số hoặc mã duy nhất, ví dụ: `101`, `102`).
  2. `phoneName`: Tên sản phẩm đầy đủ (ví dụ: `iPhone 15 Pro Max 256GB`).
  3. `brand`: Hãng (ví dụ: `Apple`, `Samsung`, `Oppo`, `Xiaomi`).
  4. `operatingSystem`: Hệ điều hành (ví dụ: `iOS 17`, `Android 14 - One UI 6`).
  5. `importPrice`: Giá nhập (ví dụ: `28.000.000 đ` hoặc `28000000`).
  6. `sellingPrice`: Giá bán lẻ (ví dụ: `32.500.000 đ`).
  7. `quantity`: Số lượng tồn kho (số nguyên không âm).
  8. `manufacturer`: Nhà sản xuất đầy đủ (ví dụ: `Apple Inc.`).
  9. `specifications`: Thông số kỹ thuật tóm tắt (ví dụ: `6.7 inch, 8GB RAM, 256GB`).
- **Mẫu dữ liệu:**
  ```text
  id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications
  101|Oppo Reno 16f 5G|Oppo|Android - ColorOS16|10.600.000 đ|14.000.000 đ|42|Oppo Electronics|6.67 inch, 8GB RAM, 256GB, 5000mAh
  102|iPhone 15 Pro Max 256GB|Apple|iOS 17|27.500.000 đ|31.990.000 đ|18|Apple Inc.|6.7 inch OLED, 8GB RAM, 256GB, A17 Pro
  ```

---

### 2. Dữ liệu Khách hàng: `data/people/Customers.txt`
- **Dòng 1 (Header):**
  `id|firstName|middleName|lastName|phoneNumber|Email|username|password|points|address`
- **Quy cách các trường:**
  1. `id`: Mã khách hàng (ví dụ: `CUST-001`).
  2. `firstName`: Họ (ví dụ: `Nguyen`).
  3. `middleName`: Tên đệm (ví dụ: `Van`).
  4. `lastName`: Tên chính (ví dụ: `An`).
  5. `phoneNumber`: Số điện thoại (duy nhất, ví dụ: `0912345678`).
  6. `Email`: Email liên hệ.
  7. `username`: Tên tài khoản đăng nhập (duy nhất).
  8. `password`: Mật khẩu tài khoản.
  9. `points`: Điểm tích lũy thành viên (số nguyên, ví dụ: `150`). *(Trước đây là 'role', đã đổi)*
  10. `address`: Địa chỉ cư trú.
- **Mẫu dữ liệu:**
  ```text
  id|firstName|middleName|lastName|phoneNumber|Email|username|password|points|address
  CUST-001|Nguyen|Van|B|0912345678|nguyenvanb@gmail.com|vanb123|123456|150|Da Nang
  CUST-002|Tran|Thi|Mai|0905111222|maitran@gmail.com|thimai|mai123|80|Thanh Khe, Da Nang
  ```

---

### 3. Dữ liệu Nhân viên: `data/people/Employee.txt`
- **Dòng 1 (Header):**
  `id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address|salary`
- **Quy cách các trường:**
  1. `id`: Mã nhân viên (ví dụ: `EMP-001`).
  2. `firstName`, `middleName`, `lastName`: Họ, tên đệm và tên nhân viên.
  3. `phoneNumber`: Số điện thoại.
  4. `Email`: Email làm việc.
  5. `username`: Tên tài khoản đăng nhập.
  6. `password`: Mật khẩu tài khoản.
  7. `role`: Mã chức vụ:
     - `1`: `RoleType::ADMIN` (Quản trị viên)
     - `2`: `RoleType::SALE_EMPLOYEE` (Nhân viên bán hàng)
     - `3`: `RoleType::WAREHOUSE_EMPLOYEE` (Nhân viên kho)
  8. `address`: Địa chỉ nơi ở.
  9. `salary`: Mức lương cơ bản (số nguyên, đơn vị VNĐ, ví dụ: `8000000`). *(Cột mới)*
- **Mẫu dữ liệu:**
  ```text
  id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address|salary
  EMP-001|Admin|He|Thong|0900000001|admin@store.com|admin|123456|1|Hai Chau, Da Nang|15000000
  EMP-002|Tran|Khanh|Duy|0987654321|trankhanhduy@gmail.com|duykhanh|duy123|2|Hoa Khanh, Da Nang|8000000
  EMP-003|Le|Van|Kho|0935123456|vankho@store.com|kho_levan|kho123|3|Lien Chieu, Da Nang|7500000
  ```

---

### 4. Dữ liệu Mã giảm giá: `data/PAY/Discount.txt`
- **Dòng 1 (Header):**
  `code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount`
- **Quy cách các trường:**
  1. `code`: Mã voucher (ví dụ: `GIAM10`, `SALE500K`).
  2. `type`: Loại chiết khấu (`0` = Phần trăm `PERCENTAGE`, `1` = Số tiền cố định `FIXED_AMOUNT`).
  3. `value`: Giá trị giảm (Ví dụ: `10` nếu 10%, hoặc `500000` nếu 500.000 VNĐ).
  4. `startDate`: Ngày bắt đầu hiệu lực (`DD/MM/YYYY`).
  5. `endDate`: Ngày kết thúc hiệu lực (`DD/MM/YYYY`).
  6. `minOrder`: Giá trị đơn hàng tối thiểu.
  7. `maxDiscount`: Mức tiền tối đa được giảm (cho loại %; nếu là cố định thì bằng value).
  8. `maxUsage`: Tổng lượt sử dụng tối đa.
  9. `usedCount`: Số lượt đã dùng thực tế.
- **Mẫu dữ liệu:**
  ```text
  code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount
  GIAM10|0|10|01/01/2026|31/12/2026|10000000|1500000|100|2
  SALE500K|1|500000|01/01/2026|31/12/2026|5000000|500000|50|5
  FREESHIP|1|50000|01/01/2026|31/12/2026|2000000|50000|200|10
  VIP20|0|20|01/01/2026|31/12/2026|20000000|3000000|20|0
  ```

---

### 5. Dữ liệu Đơn hàng: `data/PAY/Order/Orders.txt`
- **Dòng 1 (Header):**
  `id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details`
- **Quy cách các trường:**
  1. `id`: Mã đơn hàng (ví dụ: `ORD-001`).
  2. `customerId`: Mã khách hàng (liên kết với `Customers.txt`).
  3. `deliveryMethod`: Phương thức nhận hàng (`0` = Nhận tại cửa hàng, `1` = Giao tận nơi).
  4. `paymentMethod`: Phương thức thanh toán (`0` = Tiền mặt, `1` = Chuyển khoản, `2` = Thẻ tín dụng).
  5. `status`: Trạng thái:
     - `0`: PENDING | `1`: CONFIRMED | `2`: PREPARING
     - `3`: READY_FOR_PICKUP | `4`: SHIPPING | `5`: COMPLETED | `6`: CANCELLED
  6. `orderTime`: Thời điểm tạo đơn (`DD/MM/YYYY HH:MM:SS`).
  7. `discountAmount`: Số tiền giảm giá.
  8. `receiverName`: Họ tên người nhận.
  9. `phone`: Số điện thoại người nhận.
  10. `address`: Địa chỉ giao hàng.
  11. `details`: Danh sách mặt hàng, ngăn cách `;`. Mỗi item: `phoneId,phoneName,quantity,unitPrice`.
- **Mẫu dữ liệu:**
  ```text
  id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details
  ORD-001|CUST-001|1|0|1|11/09/2026 22:22:00|0|Nguyen Van B|0912345678|Hai Chau, Da Nang|101,Oppo Reno 16f 5G,1,14000000
  ORD-002|CUST-002|1|1|0|15/09/2026 09:30:15|500000|Tran Thi Mai|0905111222|Thanh Khe, Da Nang|102,iPhone 15 Pro Max 256GB,1,31990000
  ```

---

### 6. Dữ liệu Hóa đơn thanh toán: `data/PAY/Invoices.txt`
- **Dòng 1 (Header):**
  `id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details`
- **Quy cách các trường:**
  1. `id`: Mã hóa đơn (ví dụ: `INV-001`).
  2. `orderId`: Mã đơn hàng liên kết.
  3. `customerId`: Mã khách hàng.
  4. `employeeId`: Mã nhân viên lập hóa đơn.
  5. `paymentMethod`: Phương thức (`0`: CASH, `1`: BANK_TRANSFER, `2`: CREDIT_CARD).
  6. `createTime`: Thời gian xuất hóa đơn (`DD/MM/YYYY HH:MM:SS`).
  7. `discountAmount`: Số tiền chiết khấu.
  8. `details`: Chi tiết sản phẩm (`phoneId,phoneName,quantity,unitPrice;...`).
- **Mẫu dữ liệu:**
  ```text
  id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details
  INV-001|ORD-001|CUST-001|EMP-002|0|11/09/2026 22:30:00|0|101,Oppo Reno 16f 5G,1,14000000
  INV-002|ORD-004|CUST-001|EMP-002|2|20/09/2026 10:15:00|100000|105,iPhone 13 128GB,1,13890000
  ```

---

### 7. Dữ liệu Nhà cung cấp: `data/people/Suppliers.txt`
- **Dòng 1 (Header):**
  `id|companyName|taxCode|contactPerson|phoneNumber|email|address`
- **Quy cách các trường:**
  1. `id`: Mã nhà cung cấp (ví dụ: `SUP-001`).
  2. `companyName`: Tên doanh nghiệp.
  3. `taxCode`: Mã số thuế.
  4. `contactPerson`: Người đại diện liên hệ (chuỗi đầy đủ, ví dụ: `Nguyen Van Dai`).
  5. `phoneNumber`: Số điện thoại.
  6. `email`: Email doanh nghiệp.
  7. `address`: Địa chỉ trụ sở.
- **Mẫu dữ liệu:**
  ```text
  id|companyName|taxCode|contactPerson|phoneNumber|email|address
  SUP-001|Cong ty TNHH Apple Viet Nam|0312345678|Nguyen Van Dai|02838222333|contact@apple.vn|Quan 1, TP Ho Chi Minh
  SUP-002|Cong ty Dien Tu Samsung Vina|0387654321|Park Jin Ho|02838111222|support@samsung.com|TP Thu Duc, TP HCM
  ```

---

### 8. Dữ liệu Phiếu nhập kho: `data/Device/StockReceipts.txt`
- **Dòng 1 (Header):**
  `id|supplierId|employeeId|importTime|details`
- **Quy cách các trường:**
  1. `id`: Mã phiếu nhập kho (ví dụ: `REC-001`).
  2. `supplierId`: Mã nhà cung cấp.
  3. `employeeId`: Mã nhân viên kho tiếp nhận.
  4. `importTime`: Thời điểm lập phiếu (`DD/MM/YYYY HH:MM:SS`).
  5. `details`: Chi tiết hàng nhập (`phoneId,phoneName,quantity,importPrice;...`).
- **Mẫu dữ liệu:**
  ```text
  id|supplierId|employeeId|importTime|details
  REC-001|SUP-001|EMP-003|10/09/2026 08:30:00|102,iPhone 15 Pro Max 256GB,20,27500000;105,iPhone 13 128GB,35,11200000
  REC-002|SUP-002|EMP-003|12/09/2026 09:15:00|103,Samsung Galaxy S24 Ultra,25,25000000;106,Samsung Galaxy A55 5G,50,7500000
  ```

---

## MAPPING BẢNG ENUM → SỐ NGUYÊN

| Enum | Giá trị | Ý nghĩa |
|------|---------|---------|
| `RoleType::ADMIN` | `1` | Quản trị viên |
| `RoleType::SALE_EMPLOYEE` | `2` | Nhân viên bán hàng |
| `RoleType::WAREHOUSE_EMPLOYEE` | `3` | Nhân viên kho |
| `RoleType::CUSTOMER` | `4` | Khách hàng (không lưu trong Employee.txt) |
| `DeliveryMethod::PICKUP` | `0` | Nhận tại cửa hàng |
| `DeliveryMethod::DELIVERY` | `1` | Giao tận nơi |
| `PaymentMethod::CASH` | `0` | Tiền mặt |
| `PaymentMethod::BANK_TRANSFER` | `1` | Chuyển khoản |
| `PaymentMethod::CREDIT_CARD` | `2` | Thẻ tín dụng |
| `DiscountType::PERCENTAGE` | `0` | Giảm theo % |
| `DiscountType::FIXED_AMOUNT` | `1` | Giảm số tiền cố định |

> **Lưu ý về Role trong Employee.txt:** Các giá trị role trong file là 1-based (1,2,3) khớp với `static_cast<int>(RoleType) + 1` vì enum bắt đầu từ 0. Hàm `fromFileLine()` trong Employee.h xử lý việc chuyển đổi này tự động.
