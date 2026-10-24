# HỆ THỐNG QUẢN LÝ CỬA HÀNG ĐIỆN THOẠI (PHONE STORE MANAGER)

## Giới thiệu
Hệ thống Quản lý Cửa hàng Điện thoại là ứng dụng phần mềm máy tính xây dựng bằng C++17 kết hợp thư viện đồ họa GTK4. Ứng dụng phục vụ công tác điều hành bán lẻ, quản lý kho hàng, xử lý hóa đơn và quản trị nhân sự cho cửa hàng kinh doanh điện thoại. Toàn bộ dữ liệu giao dịch và danh mục được lưu trữ trực tiếp dưới dạng các tệp tin văn bản thuần (.txt) được mã hóa UTF-8.

## Tính năng theo vai trò

### Quản trị viên (Admin)
- Xem bảng điều khiển thống kê tổng quan doanh thu, lợi nhuận, số lượng đơn hàng và tình trạng tồn kho.
- Quản lý danh mục sản phẩm: thêm sản phẩm mới, cập nhật thông tin cấu hình, ngừng kinh doanh sản phẩm.
- Quản lý nhân viên: xem danh sách, cập nhật thông tin, phân công vai trò và quản lý lương cơ bản.
- Quản lý khách hàng: theo dõi thông tin liên lạc, điểm tích lũy và hạng thành viên.
- Quản lý chương trình khuyến mãi: tạo mã giảm giá, thiết lập hạn sử dụng và kiểm tra đối soát thời gian thực.
- Phân quyền tài khoản: khóa hoặc kích hoạt tài khoản đăng nhập của người dùng.

### Nhân viên bán hàng (Sales)
- Lập hóa đơn bán hàng tại quầy (POS): chọn sản phẩm, áp mã giảm giá, kiểm tra số tiền khách đưa và tính tiền thừa.
- Quản lý đơn hàng: tìm kiếm đơn hàng, duyệt đơn, cập nhật trạng thái đơn (chờ xử lý, đang chuẩn bị, hoàn tất).
- Tra cứu lịch sử hóa đơn bán lẻ và in thông tin chi tiết hóa đơn.
- Tra cứu danh mục sản phẩm và kiểm tra số lượng tồn kho khả dụng trước khi tư vấn.

### Nhân viên kho (Warehouse)
- Theo dõi tồn kho thực tế và tồn kho khả dụng của từng dòng máy.
- Lập phiếu nhập kho từ nhà cung cấp: nhập số lượng, đơn giá nhập, tự động tính tổng tiền nhập.
- Hủy phiếu nhập kho khi phát hiện sai sót (hệ thống tự động hoàn lại số lượng tồn và cập nhật giá vốn).
- Quản lý danh sách nhà cung cấp: thêm nhà cung cấp mới, cập nhật thông tin liên hệ và mã số thuế.

### Khách hàng (Customer)
- Duyệt danh mục điện thoại theo thương hiệu, hệ điều hành và khoảng giá bán.
- Xem chi tiết thông số kỹ thuật, hình ảnh và trạng thái còn hàng.
- Thêm sản phẩm vào giỏ hàng, điều chỉnh số lượng mua trong giới hạn tồn kho khả dụng.
- Đặt hàng trực tuyến: chọn hình thức nhận hàng (tại shop hoặc giao tận nơi), chọn phương thức thanh toán.
- Tra cứu lịch sử mua hàng cá nhân và gửi yêu cầu bảo hành trực tuyến cho máy đã mua.

## Yêu cầu môi trường
- Hệ điều hành đã kiểm chứng: Windows 11 64-bit.
- Hệ điều hành chưa kiểm chứng: Windows 10, Linux (Ubuntu/Debian), macOS.
- Trình biên dịch đã kiểm chứng: GCC/G++ 16.2.0 (bản phân phối MSYS2 UCRT64).
- Tiêu chuẩn ngôn ngữ: C++17 (`-std=c++17`).
- Công cụ Make: GNU Make 4.4.1 (chạy qua lệnh `mingw32-make` trên Windows).
- Thư viện giao diện đồ họa: GTK4 phiên bản 4.24.0 (cần cài đặt qua MSYS2 và cấu hình biến môi trường `PATH`).
- Công cụ dò tìm thư viện: `pkg-config` (đã kiểm chứng hoạt động cùng gói `gtk4`).

Ghi chú cho môi trường Windows:
- Cài đặt MSYS2 từ trang chủ msys2.org.
- Chạy lệnh trong MSYS2 UCRT64 shell để cài đặt gói cần thiết:
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-gtk4 mingw-w64-ucrt-x86_64-pkgconf
  ```
- Thêm đường dẫn `C:\msys64\ucrt64\bin` vào biến môi trường `PATH` của hệ thống để Windows nhận diện `g++`, `mingw32-make`, `pkg-config` và các tệp DLL runtime của GTK4.

## Tải mã nguồn
Để lấy toàn bộ mã nguồn về máy cục bộ, mở terminal và thực hiện:
```bash
git clone https://github.com/Daukecmoi133/PBL2-Moble-phone-store-manager.git
cd PBL2-Moble-phone-store-manager
```
Hoặc tải tệp nén ZIP từ giao diện web, giải nén và di chuyển con trỏ dòng lệnh vào thư mục chứa mã nguồn:
```bash
cd PBL2
```

## Biên dịch

### 1. Biên dịch ứng dụng giao diện chính
Chạy lệnh make từ thư mục gốc dự án:
```bash
mingw32-make all
```
Kết quả mong đợi:
- Trình biên dịch lần lượt tạo các tệp đối tượng `.o` và tệp phụ thuộc `.d` tương ứng với từng tệp mã nguồn.
- Liên kết sinh ra tệp thực thi `pbl2_gtk4.exe` (hoặc `pbl2_gtk4`) tại thư mục gốc của dự án.
- Thông báo kết thúc:
  ```text
  === Build thanh cong! Chay: mingw32-make run ===
  ```

### 2. Biên dịch tiện ích chuyển đổi dữ liệu độc lập
```bash
mingw32-make MigrateData.exe
```
Kết quả mong đợi: Tệp thực thi `MigrateData.exe` được sinh ra tại thư mục gốc dự án.

### 3. Biên dịch và chạy bộ kiểm thử tự động
```bash
mingw32-make test
```
Kết quả mong đợi: Biên dịch `test_runner.exe`, chạy toàn bộ 51 ca kiểm thử đơn vị & tích hợp, sau đó tự động xóa tệp nhị phân `test_runner.exe`.

### 4. Dọn dẹp tệp build
```bash
mingw32-make clean
```
Kết quả mong đợi: Xóa toàn bộ các tệp `.o`, `.d`, `pbl2_gtk4.exe` và `test_runner.exe`.

## Chuẩn bị dữ liệu
Hệ thống sử dụng các tệp tin văn bản lưu trong thư mục `data/`. Thư mục này bắt buộc phải có cấu trúc như sau:
```text
data/
├── Device/
│   ├── Products.txt          # Danh mục sản phẩm (chuẩn 9 cột, không lưu tồn kho)
│   ├── StockReceipts.txt     # Nhật ký phiếu nhập kho (chuẩn 6 cột)
│   ├── Thongsokythuat.txt    # Thông số chi tiết cấu hình máy
│   └── productImages/        # Tệp hình ảnh sản phẩm định dạng PNG
├── PAY/
│   ├── Invoices.txt          # Hóa đơn thanh toán (dòng H và dòng D)
│   ├── Discount.txt          # Danh mục mã giảm giá
│   └── Order/
│       └── Orders.txt        # Danh sách đơn hàng
├── people/
│   ├── Customers.txt         # Danh sách khách hàng
│   ├── Employee.txt          # Danh sách nhân viên
│   └── Suppliers.txt         # Danh mục nhà cung cấp
└── system/
    └── id_counters.txt       # Bộ đếm sinh mã tự tăng cho các thực thể
```

Cảnh báo an toàn dữ liệu:
- Chương trình đọc và ghi trực tiếp vào các tệp `.txt` này qua cơ chế ghi tệp tạm `.tmp` rồi đổi tên an toàn.
- Trước khi chạy thử nghiệm hoặc thao tác dữ liệu, luôn thực hiện sao lưu toàn bộ thư mục `data/`:
  ```powershell
  Copy-Item -Recurse data data_backup_truoc_khi_chay
  ```

Khởi tạo dữ liệu mẫu:
- Kho mã nguồn đã chứa sẵn bộ dữ liệu mẫu ban đầu trong thư mục `data/`. Nếu dữ liệu bị xóa hoặc chưa chuẩn hóa, chạy tiện ích chuyển đổi:
  ```bash
  .\MigrateData.exe data
  ```

## Chạy chương trình
Thực hiện lệnh make để khởi chạy ứng dụng:
```bash
mingw32-make run
```
Hoặc chạy trực tiếp tệp nhị phân:
```bash
.\pbl2_gtk4.exe
```

Cảnh báo: Dưới đây là các tài khoản thử nghiệm tích hợp sẵn trong mã nguồn nhằm mục đích đánh giá và chấm điểm đồ án. Tuyệt đối không sử dụng các mật khẩu mặc định này trong môi trường triển khai thực tế.

Bảng tài khoản thử nghiệm:
| Vai trò | Tên đăng nhập | Mật khẩu mặc định | Ghi chú quyền hạn |
|:---|:---|:---|:---|
| Quản trị viên | `admin.demo` | `123456` hoặc `admin` | Toàn quyền quản trị hệ thống và đối soát |
| Nhân viên Bán hàng | `sales.demo` | `123456` hoặc `sales` | Quầy thu ngân POS, quản lý đơn hàng |
| Nhân viên Kho | `warehouse.demo` | `123456` hoặc `warehouse` | Nhập kho, xuất kho, quản lý nhà cung cấp |
| Khách hàng | `customer001.demo` | `123456` hoặc `customer` | Mua hàng, theo dõi đơn, gửi bảo hành |

## Kịch bản thử nghiệm

### Kịch bản 1: Đăng nhập hệ thống theo phân quyền
1. Thao tác: Khởi động chương trình, tại màn hình đăng nhập, nhập Tên đăng nhập `admin.demo`, Mật khẩu `123456`, nhấn nút Đăng nhập.
2. Tài khoản: `admin.demo`.
3. Kết quả mong đợi: Đăng nhập thành công, xuất hiện thông báo chào mừng "Admin Hệ Thống", hiển thị giao diện thanh bên đầy đủ chức năng quản trị.

### Kịch bản 2: Bán hàng tại quầy và xuất hóa đơn POS
1. Thao tác:
   - Đăng nhập tài khoản `sales.demo`.
   - Chọn mục Bán hàng tại quầy (POS).
   - Thêm sản phẩm `PHN-000101` với số lượng 1 vào đơn bán.
   - Nhập số tiền khách đưa: `16000000`.
   - Nhấn nút Thanh toán tại quầy.
2. Tài khoản: `sales.demo`.
3. Kết quả mong đợi:
   - Giao diện báo thành công và hiển thị mã hóa đơn mới (dạng `INV-ddMMyyyy-xxxxxx`).
   - Mở tệp `data/PAY/Invoices.txt`, quan sát thấy 1 dòng `H` và 1 dòng `D` mới được ghi nhận an toàn ở cuối tệp. Dòng `H` có trạng thái `HoatDong`.
   - Tồn kho khả dụng của sản phẩm `PHN-000101` tự động giảm đi 1 máy.

### Kịch bản 3: Nhập kho sản phẩm từ nhà cung cấp
1. Thao tác:
   - Đăng nhập tài khoản `warehouse.demo`.
   - Chọn mục Quản lý nhập kho -> Tạo phiếu nhập mới.
   - Chọn nhà cung cấp `SUP-000001` (Apple Vietnam).
   - Chọn sản phẩm `PHN-000102`, nhập số lượng 5 máy, đơn giá nhập `27500000`.
   - Nhấn Xác nhận nhập kho.
2. Tài khoản: `warehouse.demo`.
3. Kết quả mong đợi:
   - Xuất hiện thông báo tạo phiếu nhập thành công (mã dạng `IMP-ddMMyyyy-xxxxxx`).
   - Mở tệp `data/Device/StockReceipts.txt`, xuất hiện dòng phiếu nhập mới với trạng thái `HoatDong`.
   - Tồn kho thực tế của `PHN-000102` tự động tăng thêm 5 máy trên giao diện và trong tính toán của `InventoryService`.

### Kịch bản 4: Hủy phiếu nhập kho và đối soát tồn kho
1. Thao tác:
   - Tại màn hình Quản lý nhập kho của tài khoản `warehouse.demo`.
   - Tìm phiếu nhập vừa tạo ở Kịch bản 3, nhấn nút Hủy phiếu nhập.
   - Xác nhận lý do hủy.
2. Tài khoản: `warehouse.demo`.
3. Kết quả mong đợi:
   - Phiếu nhập chuyển trạng thái sang `DaHuy`.
   - Tệp `data/Device/StockReceipts.txt` không bị xóa dòng, cột thứ 6 chuyển thành `DaHuy`.
   - Số lượng tồn kho của máy tự động giảm đi 5 máy, trở về đúng số lượng trước khi nhập.

### Kịch bản 5: Khách hàng đặt mua trực tuyến
1. Thao tác:
   - Đăng nhập tài khoản `customer001.demo`.
   - Chọn mục Cửa hàng điện thoại.
   - Chọn một máy còn hàng, nhấn nút Thêm vào giỏ.
   - Vào Giỏ hàng, kiểm tra số lượng, nhấn Tiếp tục đặt hàng.
   - Nhập họ tên người nhận, số điện thoại, địa chỉ nhận hàng, chọn phương thức COD, nhấn Xác nhận đặt hàng.
2. Tài khoản: `customer001.demo`.
3. Kết quả mong đợi:
   - Thông báo đặt đơn thành công kèm mã đơn hàng `ORD-ddMMyyyy-xxxxxx`.
   - Giỏ hàng được làm trống, sản phẩm chuyển sang trạng thái giữ chỗ (`reservedStock`).

## Chạy kiểm thử
Dự án được tích hợp sẵn bộ kiểm thử đơn vị và tích hợp độc lập tại `tests/unit_tests.cpp`. Toàn bộ quá trình kiểm thử không phụ thuộc thư viện bên ngoài và tự động cô lập trong các thư mục sandbox riêng để không làm ảnh hưởng đến dữ liệu thực.

Lệnh chạy kiểm thử:
```bash
mingw32-make test
```

Ý nghĩa các nhóm kiểm thử và kết quả kiểm chứng thực tế:
1. Nhóm 1 - Kiểm thử tiện ích FileStore (8 bài test):
   - Đọc và ghi tệp tin tiếng Việt có dấu mã hóa UTF-8.
   - Kiểm tra chống chèn ký tự phân tách trường `|` và ký tự xuống dòng `\n`.
   - Đảm bảo tính toàn vẹn của tệp gốc khi thao tác ghi gặp sự cố bất ngờ.
   - Tự động dọn dẹp và ghi đè tệp `.tmp` tồn đọng từ các phiên chạy trước.
2. Nhóm 2 - Kiểm thử InventoryService (14 bài test):
   - Chuỗi biến động tồn kho: tồn ban đầu 0 -> nhập 10 -> bán 3 -> nhập 5 -> hủy phiếu nhập 5 -> tồn kho chốt đúng 7 máy.
   - Tính toán dòng thời gian giá vốn đích danh: nhập ngày 1 giá 100, nhập ngày 2 giá 120 -> giá vốn 120; hủy phiếu ngày 2 -> giá vốn quay về đúng 100.
   - Chặn tuyệt đối hành vi bán hàng vượt quá tồn kho khả dụng và chặn số lượng không dương.
3. Nhóm 3 - Kiểm thử InvoiceService (17 bài test):
   - Tính toán tài chính chuẩn số nguyên: tạm tính 2.500.000 đ, chiết khấu 100.000 đ, thuế suất VAT 8% (192.000 đ), tổng thanh toán 2.592.000 đ.
   - Từ chối chiết khấu âm hoặc chiết khấu vượt quá tạm tính.
   - Hủy hóa đơn bán lẻ: hoàn trả tồn kho tự động, loại trừ doanh thu của hóa đơn đã hủy, chặn hủy lặp lại.
   - Tính nguyên tử khi lập hóa đơn nhiều mặt hàng: nếu mặt hàng thứ 2 trong số 3 mặt hàng bị thiếu kho, hủy bỏ toàn bộ giao dịch và không có dòng nào được ghi vào tệp.
4. Nhóm 4 - Kiểm thử tích hợp MigrateData (12 bài test):
   - Chạy chuyển đổi dữ liệu mẫu thực tế.
   - Kiểm tra cấu trúc 9 cột của `Product.txt`, chuẩn hóa thời gian và định dạng chi tiết của `stockReceipts.txt`.
   - Chuẩn hóa tách dòng `H` và dòng `D` trong `Invoices.txt`, đối soát số tiền khớp tính toán thủ công 100%.

Tổng kết kiểm chứng: **51/51 bài test ĐẠT (PASS 100%)**.

## Cấu trúc mã nguồn

| Tệp tin / Thư mục | Trách nhiệm và vai trò |
|:---|:---|
| `main.cpp` | Điểm khởi đầu của ứng dụng, khởi tạo các Repository, Service, Controller và khởi chạy luồng đồ họa |
| `Makefile` | Kịch bản điều khiển quá trình biên dịch GNU Make, cấu hình cờ biên dịch và liên kết thư viện GTK4 |
| `MigrateData.cpp` | Chương trình độc lập thực hiện chuyển đổi dữ liệu từ định dạng cũ sang định dạng mới có sao lưu |
| `source/core/` | Chứa các cấu trúc dữ liệu tự định nghĩa: `Vector.h`, `HashTable.h`, `Queue.h`, `Stack.h` |
| `source/models/` | Khai báo các thực thể miền nghiệp vụ: `Phone.h`, `ImportReceipt.h`, `Invoice.h`, `Order.h`, `Customer.h`, `Employee.h`, `Supplier.h`, `Account.h`, `Cart.h` |
| `source/repositories/` | Tầng truy xuất tệp dữ liệu cấp thấp: `PhoneRepository.h`, `StockReceiptRepository.h`, `InvoiceRepository.h`, `CustomerRepository.h`, `EmployeeRepository.h`, `SupplierRepository.h`, `OrderRepository.h`, `DiscountRepository.h` |
| `source/services/` | Tầng xử lý nghiệp vụ chính: `InventoryService.h` (tồn kho động & giá vốn), `InvoiceService.h` / `InvoiceService.cpp` (hóa đơn số nguyên), `AuthService.h`, `ReportService.h`, `CustomerService.h`, `EmployeeService.h`, `OrderService.h`, `SupplierService.h`, `DiscountService.h` |
| `source/controllers/` | `AppController.h` và `AppController.cpp` điều phối luồng dữ liệu giữa giao diện người dùng và tầng nghiệp vụ, quản lý phiên đăng nhập hiện hành |
| `source/ui/` | Tầng giao diện người dùng GTK4: `AppRunner.cpp` (khởi tạo GTK Application), `components/UIComponents.cpp` (widget dùng chung), `mock/MockDataStore.h` (dữ liệu mẫu phục vụ trình diễn), `mock/RealTimeManager.h` (giả lập đồng hồ hệ thống) |
| `source/ui/screens/` | Mã nguồn từng màn hình chức năng: `LoginScreen.cpp` (đăng nhập), `MainShellWindow.cpp` (khung giao diện), `AdminScreens.cpp` (quản trị), `SalesScreens.cpp` (bán hàng), `WarehouseScreens.cpp` (kho), `CustomerScreens.cpp` (khách mua hàng) |
| `source/ui/viewmodels/` | Các cấu trúc đóng gói dữ liệu phục vụ hiển thị trên giao diện (ViewModel và FormRequest) |
| `source/utils/` | Các module tiện ích nền tảng: `FileStore.h` / `FileStore.cpp` (ghi tệp an toàn nguyên tử), `DateTime.h` (xử lý ngày giờ), `StringHelper.h` (chuỗi), `IdGenerator.h` (sinh khóa chính tự tăng), `Utf8Helper.h` (tìm kiếm tiếng Việt) |
| `tests/` | `unit_tests.cpp` chứa bộ 51 ca kiểm thử đơn vị và kiểm thử tích hợp không dùng thư viện ngoài |

## Định dạng dữ liệu

### 1. `data/Device/Products.txt`
Dòng mẫu:
```text
PHN-000101|Oppo Reno 16F 5G|Oppo|Android - ColorOS 16|10600000|14000000|Oppo Electronics|6.57 inch AMOLED, 8GB RAM, 256GB, Dimensity 7300-Energy, 7000mAh|DangBan
```
Bảng các trường:
| STT | Tên trường | Ý nghĩa | Quy tắc |
|:---|:---|:---|:---|
| 1 | `id` | Mã định danh sản phẩm | Định dạng `PHN-xxxxxx`, duy nhất trong tệp |
| 2 | `phoneName` | Tên thương mại của máy | Chuỗi ký tự UTF-8, không chứa ký tự `\|` |
| 3 | `brand` | Thương hiệu sản phẩm | Ví dụ: Apple, Samsung, Oppo, Xiaomi |
| 4 | `operatingSystem` | Hệ điều hành cài sẵn | Ví dụ: iOS 17, Android 14 |
| 5 | `importPrice` | Giá nhập mặc định ban đầu | Số nguyên dương đơn vị đồng |
| 6 | `sellingPrice` | Giá bán lẻ niêm yết | Số nguyên dương đơn vị đồng, lớn hơn giá nhập |
| 7 | `manufacturer` | Nhà sản xuất phần cứng | Chuỗi ký tự văn bản |
| 8 | `specifications` | Tóm tắt cấu hình kỹ thuật | Chuỗi mô tả màn hình, RAM, bộ nhớ, pin |
| 9 | `status` | Trạng thái kinh doanh | `DangBan` (Đang kinh doanh) hoặc `NgungBan` (Ngừng bán) |

*Ghi chú: Cột số lượng tồn kho (`quantity`) cũ đã được loại bỏ hoàn toàn khỏi tệp này. Tồn kho được tính toán động qua nhật ký xuất nhập.*

### 2. `data/Device/StockReceipts.txt`
Dòng mẫu:
```text
IMP-10092026-000001|SUP-000001|EMP-000003|2026-09-10 08:30:00|PHN-000102,20,27500000;PHN-000105,35,11200000|HoatDong
```
Bảng các trường:
| STT | Tên trường | Ý nghĩa | Quy tắc |
|:---|:---|:---|:---|
| 1 | `id` | Mã phiếu nhập kho | Định dạng `IMP-ddMMyyyy-xxxxxx`, duy nhất |
| 2 | `supplierId` | Mã nhà cung cấp | Tham chiếu đến mã trong `Suppliers.txt` |
| 3 | `employeeId` | Mã nhân viên kho lập phiếu | Tham chiếu đến mã trong `Employee.txt` |
| 4 | `importTime` | Thời điểm lập phiếu nhập | Định dạng chuẩn `yyyy-MM-dd HH:mm:ss` |
| 5 | `details` | Danh sách mặt hàng nhập | Các sản phẩm phân tách bằng `;`. Mỗi sản phẩm gồm 3 thành phần: `maSP,soLuong,donGiaNhap` |
| 6 | `status` | Trạng thái phiếu nhập | `HoatDong` (Hợp lệ) hoặc `DaHuy` (Đã hủy phiếu) |

### 3. `data/PAY/Invoices.txt`
Tệp hóa đơn gồm hai loại dòng phân biệt bằng ký tự đầu dòng:
- Dòng H (Header - Thông tin chung của hóa đơn):
```text
H|INV-15092026-000001|ORD-15092026-000001|CUS-000001|EMP-000002|0|2026-09-15 10:20:00|42000000|0|8|3360000|45360000|DaThanhToan
```
Bảng các trường dòng H:
| STT | Tên trường | Ý nghĩa | Quy tắc |
|:---|:---|:---|:---|
| 1 | `recordType` | Loại bản ghi | Cố định là ký tự `H` |
| 2 | `id` | Mã hóa đơn | Định dạng `INV-ddMMyyyy-xxxxxx`, duy nhất |
| 3 | `orderId` | Mã đơn hàng liên kết | Mã đơn tương ứng hoặc để trống nếu bán lẻ POS |
| 4 | `customerId` | Mã khách hàng | Tham chiếu `Customers.txt` hoặc để trống |
| 5 | `employeeId` | Mã nhân viên lập hóa đơn | Tham chiếu `Employee.txt` |
| 6 | `paymentMethod` | Phương thức thanh toán | `0` (Tiền mặt), `1` (Chuyển khoản), `2` (Thẻ tín dụng) |
| 7 | `createTime` | Thời điểm xuất hóa đơn | Định dạng `yyyy-MM-dd HH:mm:ss` |
| 8 | `tamTinh` | Tổng tiền hàng trước thuế | Số nguyên, tổng `soLuong * donGia` các dòng D |
| 9 | `giamGia` | Số tiền chiết khấu | Số nguyên, không âm, không vượt quá tạm tính |
| 10 | `thueSuat` | Thuế suất giá trị gia tăng | Cố định là `8` (%) |
| 11 | `tienThue` | Tiền thuế giá trị gia tăng | Làm tròn số nguyên: `(tienSauGiam * 8 + 50) / 100` |
| 12 | `tongThanhToan` | Tổng số tiền phải trả | Số nguyên: `tienSauGiam + tienThue` |
| 13 | `status` | Trạng thái hóa đơn | `HoatDong`, `DaThanhToan` hoặc `DaHuy` |

- Dòng D (Detail - Chi tiết mặt hàng trong hóa đơn):
```text
D|INV-15092026-000001|PHN-000101|3|14000000|10600000
```
Bảng các trường dòng D:
| STT | Tên trường | Ý nghĩa | Quy tắc |
|:---|:---|:---|:---|
| 1 | `recordType` | Loại bản ghi | Cố định là ký tự `D` |
| 2 | `invoiceId` | Mã hóa đơn cha | Khớp chính xác với mã `id` của dòng `H` tương ứng |
| 3 | `productId` | Mã sản phẩm điện thoại | Tham chiếu đến mã trong `Products.txt` |
| 4 | `soLuong` | Số lượng mua | Số nguyên dương lớn hơn 0 |
| 5 | `donGia` | Đơn giá bán tại thời điểm lập | Số nguyên dương đơn vị đồng |
| 6 | `giaVon` | Giá vốn tại thời điểm lập | Lấy từ `InventoryService::giaVonHienTai` và lưu vết cố định |

### 4. `data/people/Suppliers.txt`
Dòng mẫu:
```text
SUP-000001|Apple Vietnam|0312345678|Nguyen Van Dai|02838222333|apple.supplier@example.com|Quan 1, TP Ho Chi Minh
```
Bảng các trường: `id|companyName|taxCode|contactPerson|phoneNumber|email|address`.

### 5. `data/people/Employee.txt`
Dòng mẫu:
```text
EMP-000001|Admin He Thong|0900000001|admin@example.com|Hai Chau, Da Nang|0|admin.demo|15000000
```
Bảng các trường: `id|fullName|phone|email|address|role|username|salary`. Trong đó role: `0` (Admin), `1` (Sale), `2` (Warehouse).

### 6. `data/people/Customers.txt`
Dòng mẫu:
```text
CUS-000001|Nguyen Van B|0912345678|customer001@example.com|Hai Chau, Da Nang|150|customer001.demo
```
Bảng các trường: `id|fullName|phone|email|address|points|username`.

## Hạn chế đã biết
1. Cảnh báo phân tích cú pháp hóa đơn cũ khi khởi động:
   - Vị trí: `source/repositories/InvoiceRepository.h:112`
   - Mô tả: Khi chạy chương trình, màn hình terminal ghi nhận các dòng cảnh báo:
     ```text
     [Canh bao] Loi doc dong 3 trong data/PAY/Invoices.txt: stoi
     [Canh bao] Loi doc dong 5 trong data/PAY/Invoices.txt: stoi
     [Canh bao] Loi doc dong 7 trong data/PAY/Invoices.txt: stoi
     ```
   - Nguyên nhân: Tệp `Invoices.txt` đã được nâng cấp lên định dạng chuẩn gồm 2 loại dòng `H` (Header) và `D` (Detail), trong khi lớp cũ `InvoiceRepository.h` trong tầng nạp dữ liệu ban đầu vẫn phân tích theo định dạng 1 dòng cũ. Cảnh báo này không làm sập chương trình nhưng cần được cập nhật nhất quán sang `InvoiceService`.
2. Đơn hàng đặt trực tuyến chưa tự động đồng bộ xuống tệp `Orders.txt`:
   - Vị trí: `source/controllers/AppController.cpp:791`
   - Mô tả: Khi khách hàng thực hiện checkout giỏ hàng trực tuyến, đơn hàng mới được lưu vào danh sách RAM trong `MockDataStore` để phục vụ giao diện và tính toán tồn kho giữ chỗ, nhưng chưa gọi lệnh ghi đĩa xuống tệp `data/PAY/Order/Orders.txt`. Khởi động lại ứng dụng sẽ làm mất các đơn hàng trực tuyến vừa tạo.
3. Các tệp Repository phụ trợ cũ vẫn sử dụng `std::ofstream` mở tệp trực tiếp:
   - Vị trí: `source/repositories/CustomerRepository.h:119`, `SupplierRepository.h:115`, `EmployeeRepository.h:149`, `OrderRepository.h:121`, `DiscountRepository.h:116`.
   - Mô tả: Các lớp trên chưa được chuyển đổi toàn diện sang `FileStore::ghiFileAnToan`, tiềm ẩn rủi ro hỏng tệp nếu mất điện đột ngột trong lúc ghi.
4. Xóa vật lý dòng trong một số Repository cũ:
   - Vị trí: `source/repositories/StockReceiptRepository.h:197` và `source/repositories/InvoiceRepository.h:230`.
   - Mô tả: Hai hàm `deleteReceipt` và `deleteInvoice` trong các repository cũ vẫn thực hiện xóa vật lý dòng dữ liệu khỏi danh sách thay vì chỉ chuyển trạng thái thành `DaHuy`. Người dùng cần tránh gọi trực tiếp hai hàm này mà sử dụng các phương thức `huyPhieuNhap` và `huyHoaDon` trong Service tương ứng.

## Xử lý sự cố
- Lỗi không tìm thấy thư viện GTK4 khi chạy lệnh `mingw32-make`:
  + Triệu chứng: `pkg-config: command not found` hoặc lỗi không tìm thấy các file header `gtk/gtk.h`.
  + Cách xử lý đã kiểm chứng: Mở trình cài đặt MSYS2, chạy `pacman -S mingw-w64-ucrt-x86_64-gtk4 mingw-w64-ucrt-x86_64-pkgconf` và kiểm tra đã thêm `C:\msys64\ucrt64\bin` vào biến môi trường `PATH`.
- Lỗi tệp dữ liệu rỗng hoặc sai thứ tự cột:
  + Triệu chứng: Ứng dụng báo lỗi đọc tệp hoặc hiển thị danh sách sản phẩm trống.
  + Cách xử lý đã kiểm chứng: Khôi phục lại thư mục từ bản sao lưu `data_backup_...` hoặc chạy tiện ích `.\MigrateData.exe data` để tự động tái tạo định dạng dữ liệu chuẩn.
- Lỗi tệp khóa tạm `.tmp` còn sót lại do tắt máy đột ngột:
  + Triệu chứng: Tệp dữ liệu có thêm tệp đuôi `.tmp` bên cạnh.
  + Cách xử lý đã kiểm chứng: Tiện ích `FileStore` tích hợp sẵn trong mã nguồn đã tự động xử lý ghi đè và dọn dẹp tệp `.tmp` cũ ngay trong lần ghi tiếp theo mà không cần xóa thủ công.

## Đóng góp và giấy phép
[Cần tác giả bổ sung]
