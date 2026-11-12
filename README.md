# Phone Store Manager

Ứng dụng desktop quản lý cửa hàng điện thoại, viết bằng C++17 với giao diện
GTK4. Dữ liệu demo được lưu trong các tệp văn bản tại `data/`.

## Chức năng chính

- Quản lý sản phẩm, tồn kho, phiếu nhập và nhà cung cấp.
- Quản lý nhân viên, khách hàng, tài khoản và mã giảm giá.
- Hỗ trợ nghiệp vụ bán hàng, đơn hàng, hóa đơn và báo cáo.
- Giao diện có các vai trò quản trị, bán hàng, kho và khách hàng.

## Yêu cầu môi trường

Các lệnh bên dưới dành cho Windows với MSYS2 UCRT64. Makefile hiện dùng
`g++`, `pkg-config`, GTK4 và fontconfig.

1. Cài MSYS2 từ <https://www.msys2.org/> và mở **MSYS2 UCRT64**.
2. Cài các công cụ và thư viện cần thiết:

   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-gtk4 mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-fontconfig mingw-w64-ucrt-x86_64-adwaita-icon-theme
   ```

3. Nếu chạy ứng dụng từ Windows Terminal hoặc PowerShell, thêm
   `C:\msys64\ucrt64\bin` vào `PATH` để tìm thấy các DLL của GTK4 và
   fontconfig. Trong MSYS2 UCRT64, đường dẫn này đã được thiết lập sẵn.

## Tải mã nguồn

Trong MSYS2 UCRT64:

```bash
git clone https://github.com/Daukecmoi133/PBL2-Moble-phone-store-manager.git
cd PBL2-Moble-phone-store-manager
```

Nếu đã tải ZIP, mở MSYS2 UCRT64 tại thư mục dự án đã giải nén thay vì chạy hai
lệnh trên.

## Biên dịch và chạy

Chạy các lệnh từ thư mục gốc dự án:

```bash
mingw32-make all
```

Sau khi build thành công, tệp thực thi `pbl2_gtk4.exe` được tạo tại thư mục gốc.
Để build và khởi chạy ứng dụng:

```bash
mingw32-make run
```

Hoặc chạy trực tiếp trong MSYS2 UCRT64:

```bash
./pbl2_gtk4.exe
```

Một số lệnh Makefile khác:

```bash
mingw32-make info
mingw32-make clean
```

`info` hiển thị phiên bản thư viện và cấu hình build. `clean` xóa các tệp build
`.o`, `.d` và executable của ứng dụng.

## Tài khoản và dữ liệu demo

Màn hình đăng nhập có các nút điền nhanh tài khoản demo cho Admin, Sales,
Warehouse và Customer. Dùng các nút này để đăng nhập; không sử dụng dữ liệu
hoặc mật khẩu demo trong môi trường thực tế.

Ứng dụng đọc và ghi dữ liệu trong thư mục `data/`. Trước khi thử các thao tác
làm thay đổi dữ liệu, có thể sao lưu từ MSYS2 UCRT64 bằng lệnh:

```bash
cp -a data "data-backup-$(date +%Y%m%d-%H%M%S)"
```

Schema, định dạng ngày giờ, quy tắc nhập dữ liệu và các giới hạn của parser được
mô tả trong [data/DATA_FORMAT_GUIDE.md](data/DATA_FORMAT_GUIDE.md). Không sửa
định dạng hoặc xóa các tệp dữ liệu nếu chưa sao lưu.

## Kiểm thử

Makefile hiện không định nghĩa target `test` và repository không có
`tests/unit_tests.cpp`. Vì vậy, `mingw32-make test` không phải lệnh kiểm thử hợp
lệ của phiên bản hiện tại. Các tệp trong `tests/` là mã kiểm thử/diagnostic
riêng lẻ, không được Makefile tích hợp thành một bộ test chung.

## Cấu trúc dự án

| Đường dẫn | Nội dung |
|---|---|
| `main.cpp` | Khởi tạo repository, service, controller và giao diện ứng dụng |
| `Makefile` | Các target build, run, clean và info |
| `source/core/` | Các cấu trúc dữ liệu dùng trong dự án |
| `source/models/` | Các mô hình sản phẩm, nhân viên, khách hàng, đơn hàng và hóa đơn |
| `source/repositories/` | Đọc và ghi dữ liệu |
| `source/services/` | Nghiệp vụ ứng dụng |
| `source/controllers/` | Điều phối giữa service và giao diện; phần triển khai được chia theo nghiệp vụ (sản phẩm, giỏ hàng, đơn hàng, hóa đơn, khách hàng, nhân viên, kho, giảm giá, tài khoản và báo cáo) |
| `UI/` | Ứng dụng GTK4, màn hình, widget và view model |
| `data/` | Dữ liệu demo và ảnh sản phẩm |
| `tests/` | Mã kiểm thử và chẩn đoán riêng lẻ |
