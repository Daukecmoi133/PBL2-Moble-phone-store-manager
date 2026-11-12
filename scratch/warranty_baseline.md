# PBL2 - Warranty Baseline & Test Verification

**Thời điểm ghi nhận:** 2026-11-12 15:48 (GMT+7)  
**Nhánh Git:** `duy` (chuẩn bị rẽ nhánh `feature/warranty`)  
**Môi trường:** Windows x64, MSYS2 UCRT64, GCC 16.2.0, GTK 4.24.0, OpenSSL 3.x

---

## 1. Lệnh Build Ứng Dụng Chính (Makefile)

- **Lệnh build:**
  ```bash
  mingw32-make
  # hoặc: mingw32-make build
  ```
  - *Compiler:* `g++` (C++17)
  - *Flags:* `-std=c++17 -Wall -Wextra $(pkg-config --cflags gtk4 fontconfig) -I. -Wno-unused-parameter -Wno-deprecated-declarations -Wno-pedantic`
  - *Linker Flags:* `$(pkg-config --libs gtk4 fontconfig libcrypto)`
  - *Target đầu ra:* `pbl2_gtk4.exe`
  - *Lưu ý môi trường:* Trên Windows sandbox, GNU Make 4.4.1 có thể gặp lỗi tạo named pipe `0xc0000008`, chạy lệnh với quyền bypass sandbox hoặc biên dịch trực tiếp qua `g++`.

- **Lệnh dọn dẹp:**
  ```bash
  mingw32-make clean
  ```

- **Lệnh chạy ứng dụng:**
  ```bash
  mingw32-make run
  # hoặc: .\pbl2_gtk4.exe
  ```

---

## 2. Quy Cách Biên Dịch & Chạy Từng File Trong `tests/`

Các test trong dự án tuân thủ cơ chế cô lập dữ liệu (Sandbox). Khi chạy, các test tự tạo thư mục dữ liệu giả lập riêng (ví dụ `tests/sandbox_auth_test_data`, `tests/sandbox_runtime_data`, `tests/sandbox/c_data`,...) và tự động dọn dẹp sau khi kết thúc để không làm biến đổi dữ liệu thật trong `data/`.

### Nhóm 1: Test nghiệp vụ độc lập & ViewModel (Không phụ thuộc GTK / Crypto)
Biên dịch trực tiếp bằng `g++ -std=c++17 -I.`:

| Tên Test | Lệnh Biên Dịch | Lệnh Chạy | Sandbox Sử Dụng |
| :--- | :--- | :--- | :--- |
| `test_admin_reports_charts.cpp` | `g++ -std=c++17 -I. tests/test_admin_reports_charts.cpp -o tests/test_admin_reports_charts.exe` | `.\tests\test_admin_reports_charts.exe` | In-memory |
| `test_anti_product_detail.cpp` | `g++ -std=c++17 -I. tests/test_anti_product_detail.cpp -o tests/test_anti_product_detail.exe` | `.\tests\test_anti_product_detail.exe` | In-memory |
| `test_customer_discounts.cpp` | `g++ -std=c++17 -I. tests/test_customer_discounts.cpp -o tests/test_customer_discounts.exe` | `.\tests\test_customer_discounts.exe` | In-memory |
| `test_design_md_compliance.cpp` | `g++ -std=c++17 -I. tests/test_design_md_compliance.cpp -o tests/test_design_md_compliance.exe` | `.\tests\test_design_md_compliance.exe` | Đọc `docs/DESIGN.md`, `UI/styles/` |
| `test_sales_filter_sort.cpp` | `g++ -std=c++17 -I. tests/test_sales_filter_sort.cpp -o tests/test_sales_filter_sort.exe` | `.\tests\test_sales_filter_sort.exe` | Static code analysis |
| `test_phase_e2_ui.cpp` | `g++ -std=c++17 -I. tests/test_phase_e2_ui.cpp -o tests/test_phase_e2_ui.exe` | `.\tests\test_phase_e2_ui.exe` | Static UI audit |
| `test_warranty_system.cpp` | `g++ -std=c++17 -I. tests/test_warranty_system.cpp source/controllers/*.o UI/components/*.o UI/screens/*.o UI/AppRunner.o UI/FontSetup.o $(pkg-config --libs gtk4 fontconfig libcrypto) -o tests/test_warranty_system.exe` | `.\tests\test_warranty_system.exe` | `tests/sandbox_warranty` |

### Nhóm 2: Test tích hợp Controller, Persistence, RBAC & E2E (Cần libcrypto / GTK / Object files)
Biên dịch liên kết với các object file của ứng dụng:

```bash
g++ -std=c++17 -I. tests/<ten_test>.cpp source/controllers/*.o UI/components/*.o UI/screens/*.o UI/AppRunner.o UI/FontSetup.o $(pkg-config --libs gtk4 fontconfig libcrypto) -o tests/<ten_test>.exe
```

| Tên Test | Sandbox Dữ Liệu | Mục Tiêu Kiểm Thử |
| :--- | :--- | :--- |
| `test_auth_real_data.cpp` | `tests/sandbox_auth_test_data` | Xác thực đăng nhập, băm mật khẩu PBKDF2, phân quyền RBAC |
| `test_persistence_hardening.cpp` | `tests/sandbox_runtime_data` | Giao dịch đa file, atomic write, crash recovery |
| `test_persistence_phase_a.cpp` | `tests/sandbox_data` | Khôi phục file khi lỗi ghi đĩa |
| `test_phase_b1_pos.cpp` | `tests/sandbox_b1_pos_runtime` | Luồng bán hàng POS, giỏ hàng, trừ tồn kho |
| `test_phase_b2_warehouse.cpp` | `tests/sandbox_b2_warehouse_runtime` | Phiếu nhập kho, số lượng tùy chỉnh, liên kết NCC |
| `test_phase_b3_voucher.cpp` | `tests/sandbox_b3_voucher_runtime` | Quản lý voucher, gia hạn, định dạng tiền tệ |
| `test_phase_c_image.cpp` | `tests/sandbox/c_data` | Quản lý ảnh sản phẩm, lưu trữ bền vững |
| `test_phase_d_sync.cpp` | `tests/sandbox/d_data` | Đồng bộ thời gian thực giữa các tab/màn hình |
| `test_phase_e1_search.cpp` | `tests/sandbox/e1_data` | Tìm kiếm sản phẩm không dấu tiếng Việt |
| `test_phase_f_e2e.cpp` | `tests/sandbox_f_e2e` | Kịch bản bán hàng toàn diện đa vai trò |

### Nhóm 3: Test Giao Diện / Input Method / Win32 (Yêu cầu màn hình hiển thị tương tác)
- `diag_im_query.cpp` / `diag_ime_app.cpp` / `test_f1_hwnd.cpp` / `test_f1_windows_input.cpp` / `test_ime_final_verification.cpp` / `test_ime_modal_lifecycle.cpp` / `test_im_unit.cpp`.

---

## 3. Bảng Kết Quả Kiểm Thử Toàn Bộ Test Hiện Có (Baseline)

| STT | Tên Test Executable | Kết Quả | Chi Tiết / Lý Do Nếu Thất Bại |
| :---: | :--- | :---: | :--- |
| 1 | `diag_im_query.exe` | **PASS** | Kiểm tra môi trường GTK IM Module thành công. |
| 2 | `test_admin_reports_charts.exe` | **PASS** | 100% assertions đạt: Thống kê doanh thu, tỷ suất, dữ liệu biểu đồ. |
| 3 | `test_anti_product_detail.exe` | **PASS** | 100% assertions đạt: ViewModel chi tiết sản phẩm, mô tả & thông số. |
| 4 | `test_auth_real_data.exe` | **PASS** | **63/63 assertions PASS**: Đăng nhập, phân quyền, đổi pass, băm mật khẩu. |
| 5 | `test_customer_discounts.exe` | **PASS** | 100% assertions đạt: Lọc và nạp danh sách mã giảm giá khách hàng. |
| 6 | `test_design_md_compliance.exe` | **PASS** | 100% assertions đạt: Tuân thủ quy chuẩn DESIGN.md và bảng màu. |
| 7 | `test_f1_hwnd.exe` | **PASS** | Cửa sổ Win32 HWND khởi tạo thành công. |
| 8 | `test_f1_windows_input.exe` | **FAIL** *(từ trước)* | **Lỗi môi trường Sandbox/CI:** Test dùng Win32 API `SendInput` để gõ phím ảo; trong sandbox/headless không có tiêu điểm chuột/bàn phím desktop active nên `GtkEntry` nhận chuỗi rỗng `''`. |
| 9 | `test_im_unit.exe` | **PASS** | 100% assertions đạt: IM context switch. |
| 10 | `test_ime_final_verification.exe` | **PASS** | 100% assertions đạt: Gõ tiếng Việt bộ gõ tích hợp. |
| 11 | `test_ime_modal_lifecycle.exe` | **PASS** | 100% assertions đạt: Vòng đời dialog modal không crash IME. |
| 12 | `test_persistence_hardening.exe` | **FAIL** *(từ trước)* | **59 PASS, 3 FAIL:** Thất bại tại Test 8 do sản phẩm `Oppo Reno 16F 5G` trong test case không nằm trong danh mục của nhà cung cấp đã chọn (ràng buộc nghiệp vụ mới của BUG-HIGH-04). |
| 13 | `test_persistence_phase_a.exe` | **PASS** | 100% assertions đạt: Ghi file atomic và rollback an toàn. |
| 14 | `test_phase_b1_pos.exe` | **PASS** | 100% assertions đạt: Luồng bán hàng POS tại quầy. |
| 15 | `test_phase_b2_warehouse.exe` | **FAIL** *(từ trước)* | **28 PASS, 1 FAIL:** Lệch 1 assertion kiểm tra tồn kho ban đầu của mã `PHN-000105` sau khi restart ứng dụng (do dữ liệu khởi điểm). |
| 16 | `test_phase_b3_voucher.exe` | **PASS** | 100% assertions đạt: Gia hạn và áp dụng voucher. |
| 17 | `test_phase_c_image.exe` | **FAIL** *(từ trước)* | **43 PASS, 1 FAIL:** Lệch 1 assertion tại dòng 278 về snapshot sản phẩm `PHN-000105` trong thư mục dữ liệu sandbox. |
| 18 | `test_phase_d_sync.exe` | **PASS** | 100% assertions đạt: Đồng bộ dữ liệu đa màn hình. |
| 19 | `test_phase_e1_search.exe` | **PASS** | 100% assertions đạt: Tìm kiếm tiếng Việt chuẩn NFC & không dấu. |
| 20 | `test_phase_e2_ui.exe` | **PASS** | 100% assertions đạt: Căn chỉnh cột bảng và chuẩn kích thước text. |
| 21 | `test_phase_f_e2e.exe` | **FAIL** *(từ trước)* | **52 PASS, 5 FAIL:** Lệch do voucher `DIS00001` trong dữ liệu cũ hết hạn trước mốc test và công thức cộng dồn tồn kho cũ trong kịch bản E2E. |
| 22 | `test_sales_filter_sort.exe` | **PASS** | **42/42 assertions PASS**: Lọc & sắp xếp 5 danh mục SalesScreens. |
| 23 | `test_warranty_system.exe` | **PASS** | **41/41 assertions PASS**: Toàn bộ nghiệp vụ Bảo hành sản phẩm. |

---

## 4. Tình Trạng Hiện Tại Về Các Từ Khóa Warranty Trong `source/`

Kết quả chạy `git grep -E "WarrantyRequest|WarrantyService|WarrantyRequestRepository" source/`:

1. `source/PBL2.h`: Include các header bảo hành.
2. `source/controllers/AccountController.cpp`: Xử lý `getWarranties`, `getCustomerWarranties`, `createWarranty`, `updateWarrantyStatus`.
3. `source/controllers/AppController.h` & `source/controllers/AppController.cpp`: Con trỏ `WarrantyService*` và hàm tạo.
4. `source/controllers/AppControllerSupport.h`: Macro `REQUIRE_ADMIN_OR_STAFF_ROLE()`.
5. `source/models/Common.h`: Khai báo enum `WarrantyRequestStatus`.

> **Ghi chú an toàn:** Các thay đổi này hiện đang ở trạng thái chưa commit trên cây làm việc (`working tree`). Tùy theo lựa chọn của người dùng (`commit` vào commit hiện tại hoặc `stash` sang nhánh phụ), cây làm việc sẽ được đưa về trạng thái sạch trước khi tạo nhánh `feature/warranty`.

