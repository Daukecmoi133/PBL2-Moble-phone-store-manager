# ĐỀ CƯƠNG VÀ TƯ LIỆU BÁO CÁO ĐỒ ÁN MÔN HỌC PBL2
## ĐỀ TÀI: HỆ THỐNG QUẢN LÝ CỬA HÀNG ĐIỆN THOẠI (PHONE STORE MANAGER)

---

### THÔNG TIN CHUNG ĐỀ TÀI
- **Tên đề tài:** Hệ thống Quản lý Cửa hàng Điện thoại (Phone Store Management System)
- **Học phần:** Dự án Cơ sở Lập trình (PBL2)
- **Ngôn ngữ thực hiện:** C++17 (`-std=c++17`)
- **Giao diện đồ họa (GUI):** GTK4 (phiên bản 4.24.0 trên MSYS2 UCRT64)
- **Cơ chế lưu trữ:** Hệ thống tệp văn bản định dạng thuần UTF-8 (`.txt`) với cơ chế giao dịch ghi tệp tạm an toàn (Safe FileStore Atomic Write)
- **Công cụ biên dịch và xây dựng:** GNU Compiler Collection (GCC/G++ 16.2.0), GNU Make 4.4.1

---

## CHƯƠNG 1: TỔNG QUAN VỀ ĐỀ TÀI VÀ KHẢO SÁT BÀI TOÁN

### 1.1. Đặt vấn đề và tính cấp thiết
Trong bối cảnh thị trường thiết bị di động phát triển nhanh chóng, các chuỗi cửa hàng bán lẻ điện thoại thông minh phải đối mặt với bài toán quản trị phức tạp:
1. **Quản lý đa dạng danh mục hàng hóa và thông số kỹ thuật:** Mỗi dòng điện thoại có nhiều phiên bản bộ nhớ, màu sắc, cấu hình chi tiết (CPU, RAM, Màn hình, Camera, Dung lượng pin).
2. **Quản lý biến động giá vốn và tồn kho thời gian thực:** Đảm bảo số liệu tồn kho luôn đồng bộ với các nghiệp vụ nhập kho từ nhà cung cấp và bán hàng tại quầy (POS), tránh tình trạng bán vượt tồn kho (âm kho).
3. **Quản lý bán hàng và xuất hóa đơn minh bạch:** Yêu cầu tính toán chính xác tuyệt đối các khoản tiền (tạm tính, giảm giá khuyến mãi, thuế VAT 8%, tổng thanh toán) mà không được phép xuất hiện sai số dấu phẩy động.
4. **Phân quyền người dùng chặt chẽ:** Tách biệt rõ ràng các vai trò Quản trị viên (Admin), Nhân viên Bán hàng (Sales), Nhân viên Kho (Warehouse), và Khách hàng (Customer).

### 1.2. Mục tiêu của đồ án
- **Về mặt kỹ thuật lập trình:**
  - Vận dụng sâu sắc và thuần thục 4 nguyên lý lập trình hướng đối tượng (Kế thừa, Đóng gói, Đa hình, Trừu tượng) trong C++17.
  - Tự tay nghiên cứu, thiết kế và cài đặt toàn bộ các **Cấu trúc Dữ liệu và Giải thuật nền tảng** (`Vector`, `HashTable`, `BST`, `LinkedList`, `Queue`, `Stack`), tuyệt đối không phụ thuộc vào các container có sẵn của thư viện chuẩn STL (`std::vector`, `std::map`, `std::unordered_map`,...) cho các thực thể dữ liệu nghiệp vụ chính.
  - Xây dựng kiến trúc phân tầng rõ ràng (Multi-tier Architecture): Tách bạch tầng Giao diện (UI), Tầng Dịch vụ nghiệp vụ (Service), Tầng Truy cập dữ liệu (Repository), và Tầng Lưu trữ an toàn (FileStore).
- **Về mặt ứng dụng thực tiễn:**
  - Cung cấp giao diện trực quan, thân thiện bằng thư viện đồ họa hiện đại GTK4.
  - Xử lý các luồng nghiệp vụ thực tế: Bán hàng tại quầy POS, Quản lý đơn hàng trực tuyến, Lập phiếu nhập kho, Hủy phiếu nhập cập nhật giá vốn, Áp dụng mã giảm giá, Tra cứu bảo hành.

### 1.3. Yêu cầu phi chức năng
1. **Tính chính xác về tài chính (Financial Accuracy):** Toàn bộ các giá trị tiền tệ VND sử dụng số nguyên 64-bit (`long long`), tuyệt đối không dùng số thực (`float`, `double`) để triệt tiêu sai số làm tròn số học.
2. **Tính toàn vẹn dữ liệu (Data Integrity):** Không lưu trữ số lượng tồn kho tĩnh trong tệp sản phẩm `Products.txt`. Tồn kho được tính toán động từ tổng số lượng thực tế đã nhập qua các phiếu nhập hợp lệ trừ đi số lượng đã xuất theo các hóa đơn bán thành công (Nguyên lý Single Source of Truth).
3. **An toàn khi thao tác tệp tin (File Atomicity):** Cơ chế ghi dữ liệu luôn qua tệp tạm `.tmp`, thẩm định tính hợp lệ rồi mới thực hiện đổi tên thay thế tệp gốc (`Atomic Rename`), ngăn chặn việc hỏng dữ liệu khi gặp sự cố ngắt nguồn.
4. **Trải nghiệm người dùng (UX):** Giao diện tiếng Việt chuẩn UTF-8, định dạng tiền tệ phân tách hàng nghìn bằng dấu chấm (ví dụ: `25.990.000 đ`), cơ chế vô hiệu hóa nút bấm thanh toán chống bấm lặp (double-click prevention).

---

## CHƯƠNG 2: PHÂN TÍCH VÀ THIẾT KẾ KIẾN TRÚC HỆ THỐNG

### 2.1. Kiến trúc phân tầng (Multi-tier Architecture)
Hệ thống được thiết kế theo mô hình 4 tầng phân lập rõ ràng:

```text
+-------------------------------------------------------------------+
|                     TẦNG GIAO DIỆN (UI LAYER)                    |
|       GTK4 Screens: LoginScreen, AdminScreens, SalesScreens,      |
|             WarehouseScreens, CustomerScreens, UIComponents       |
+-------------------------------------------------------------------+
                                  │
                                  ▼
+-------------------------------------------------------------------+
|                   TẦNG DỊCH VỤ (SERVICE LAYER)                    |
|   AuthService, InventoryService, InvoiceService, PhoneService,     |
|   OrderService, DiscountService, EmployeeService, SupplierService |
+-------------------------------------------------------------------+
                                  │
                                  ▼
+-------------------------------------------------------------------+
|               TẦNG TRUY CẤP DỮ LIỆU (REPOSITORY LAYER)            |
|     AccountRepo, PhoneRepo, StockReceiptRepo, InvoiceRepo,        |
|               OrderRepo, DiscountRepo, CustomerRepo               |
+-------------------------------------------------------------------+
                                  │
                                  ▼
+-------------------------------------------------------------------+
|            TẦNG CẤU TRÚC DỮ LIỆU & LƯU TRỮ (CORE & STORAGE)       |
|    - Cấu trúc dữ liệu: Vector<T>, HashTable<K,V>, BST<T>,...      |
|    - Tiện ích an toàn: FileStore (Atomic Write, Validator)        |
+-------------------------------------------------------------------+
```

#### Ma trận phụ thuộc và quy tắc gọi giữa các tầng:
- **Tầng Giao diện (UI)**: Chỉ giao tiếp với tầng Service thông qua các ViewModel và kết quả trả về `OperationResult`. Giao diện không bao giờ được đọc/ghi trực tiếp vào tệp tin hoặc gọi tầng Repository.
- **Tầng Dịch vụ (Service)**: Chứa toàn bộ các ràng buộc nghiệp vụ (kiểm tra tồn kho, tính thuế, áp mã giảm giá, kiểm tra trạng thái phiếu). Tầng Service đóng gói logic tính toán và gọi Repository để lấy dữ liệu.
- **Tầng Repository**: Quản lý việc đọc, ánh xạ dữ liệu thô từ cấu trúc dữ liệu lưu trữ thành các đối tượng Model (`Phone`, `Customer`, `Employee`, `Invoice`, `StockReceipt`).
- **Tầng Tiện ích & Lưu trữ**: Cung cấp các thao tác đọc/ghi tệp, chuẩn hóa chuỗi Unicode, xử lý thời gian.

---

## CHƯƠNG 3: THIẾT KẾ VÀ CÀI ĐẶT CÁC CẤU TRÚC DỮ LIỆU TỰ XÂY DỰNG

Đồ án tự cài đặt đầy đủ các cấu trúc dữ liệu mẫu (Template) đặt tại thư mục `source/core/`:

### 3.1. Mảng động tự co giãn (`Vector<T>`) - `source/core/Vector.h`
- **Nguyên lý hoạt động:** Cấp phát bộ nhớ động liên tục. Khi số lượng phần tử đạt đến ngưỡng dung lượng (`capacity`), cấu trúc sẽ tự động cấp phát một vùng nhớ mới gấp đôi dung lượng cũ ($2 \times capacity$), sao chép các phần tử sang vùng nhớ mới và giải phóng vùng nhớ cũ.
- **Độ phức tạp thuật toán:**
  - Truy xuất theo chỉ số ngẫu nhiên: $O(1)$.
  - Thêm phần tử vào cuối (`push_back`): Chi phí phân bổ trung bình (Amortized) $O(1)$, trường hợp xấu nhất khi cấp phát lại là $O(n)$.
  - Xóa phần tử tại vị trí bất kỳ: $O(n)$ do phải dịch chuyển các phần tử phía sau.
- **Ứng dụng trong đồ án:** Lưu danh sách sản phẩm hiển thị trên bảng, danh sách các mục trong giỏ hàng (`Cart`), danh sách kết quả tìm kiếm và bộ lọc.

### 3.2. Bảng băm (`HashTable<K, V>`) - `source/core/HashTable.h`
- **Nguyên lý hoạt động:** Sử dụng hàm băm chuỗi để ánh xạ khóa kiểu chuỗi thành chỉ số trong mảng băm. Xử lý va chạm băm (Collision Resolution) bằng kỹ thuật nối kết trực tiếp (Separate Chaining) kết hợp danh sách liên kết tại mỗi bucket.
- **Độ phức tạp thuật toán:**
  - Tra cứu theo khóa (Lookup/Get): Trung bình $O(1)$, trường hợp xấu nhất khi va chạm toàn bộ là $O(n)$.
  - Thêm hoặc cập nhật phần tử (Insert/Set): Trung bình $O(1)$.
  - Xóa phần tử (Remove): Trung bình $O(1)$.
- **Ứng dụng trong đồ án:** 
  - Tra cứu nhanh thông tin cấu hình kỹ thuật theo mã `specsId`.
  - Kiểm tra tính tồn tại và quyền truy cập của tài khoản (`username`).
  - Đối soát và áp dụng mã giảm giá (`discountId`) trong thời gian thực.

### 3.3. Cây nhị phân tìm kiếm (`BST<T>`) - `source/core/BST.h`
- **Nguyên lý hoạt động:** Cây nhị phân có tính chất: với mỗi nút $X$, mọi nút ở cây con bên trái đều có khóa nhỏ hơn $X$, và mọi nút ở cây con bên phải đều có khóa lớn hơn $X$. Duyệt cây theo thứ tự giữa (In-Order Traversal) cho ra một dãy khóa được sắp xếp tăng dần.
- **Độ phức tạp thuật toán:**
  - Tìm kiếm, Thêm, Xóa: Trung bình $O(\log n)$, trường hợp xấu nhất (cây suy biến thành danh sách liên kết) là $O(n)$.
- **Ứng dụng trong đồ án:** 
  - Quản lý danh mục sản phẩm theo khoảng giá bán: Cho phép duyệt và lọc nhanh các điện thoại trong khoảng giá $[P_{min}, P_{max}]$.
  - Sắp xếp tự nhiên danh sách khách hàng theo điểm tích lũy hoặc doanh số.

### 3.4. Danh sách liên kết đơn/đôi (`LinkedList<T>`) - `source/core/LinkedList.h`
- **Nguyên lý hoạt động:** Tập hợp các nút (Node) lưu trữ dữ liệu và con trỏ trỏ đến nút kế tiếp (và nút trước đó).
- **Độ phức tạp thuật toán:** Thêm/Xóa tại đầu hoặc cuối danh sách đạt $O(1)$ khi có con trỏ `head` và `tail`.
- **Ứng dụng trong đồ án:** Làm nền tảng xây dựng hàng đợi, ngăn xếp và các bucket trong bảng băm.

### 3.5. Hàng đợi FIFO (`Queue<T>`) - `source/core/Queue.h`
- **Nguyên lý hoạt động:** Vào trước - Ra trước (First In First Out).
- **Độ phức tạp:** Thao tác thêm vào cuối (`enqueue`) và lấy ra ở đầu (`dequeue`) đều đạt $O(1)$.
- **Ứng dụng trong đồ án:** Quản lý hàng đợi xử lý đơn hàng trực tuyến của khách hàng theo đúng thứ tự thời gian đặt hàng.

### 3.6. Ngăn xếp LIFO (`Stack<T>`) - `source/core/Stack.h`
- **Nguyên lý hoạt động:** Vào sau - Ra trước (Last In First Out).
- **Độ phức tạp:** Đẩy vào đỉnh (`push`) và lấy ra khỏi đỉnh (`pop`) đều đạt $O(1)$.
- **Ứng dụng trong đồ án:** Quản lý lịch sử điều hướng các màn hình giao diện (Stack điều hướng), hỗ trợ quay lại (Back) màn hình trước đó mà không làm mất trạng thái đang nhập liệu dở dang.

---

## CHƯƠNG 4: THIẾT KẾ CƠ SỞ DỮ LIỆU VÀ CƠ CHẾ AN TOÀN TỆP TIN

### 4.1. Quy ước lưu trữ tệp tin văn bản thuần (.txt)
Hệ thống sử dụng các tệp tin đặt trong thư mục `data/`, mã hóa UTF-8, mỗi thực thể một dòng, các trường phân tách bởi ký tự gạch đứng `|`:

| Tên tệp | Số cột | Cấu trúc định dạng chi tiết | Ghi chú quan trọng |
|:---|:---:|:---|:---|
| `data/Device/Products.txt` | 9 | `id\|brand\|name\|category\|price\|costPrice\|description\|status\|specsId` | **Tuyệt đối không lưu cột tồn kho**, giá vốn cập nhật theo phiếu nhập gần nhất |
| `data/Device/StockReceipts.txt` | 6 | `id\|supplierId\|employeeId\|importTime\|details\|status` | `details` có định dạng `productId:qty:unitPrice,...`; `status` gồm `HoatDong` hoặc `DaHuy` |
| `data/PAY/Invoices.txt` | Biến thiên | - Dòng Header (H - 9 cột): `H\|invoiceId\|customerId\|employeeId\|invoiceDate\|subTotal\|discountAmount\|taxAmount\|totalAmount\|status`<br>- Dòng Detail (D - 5 cột): `D\|invoiceId\|productId\|quantity\|unitPrice\|costPrice` | Đảm bảo tính nguyên tử: Dòng H và các dòng D đi kèm được ghi liên tục cùng một lần |
| `data/PAY/Discount.txt` | 3 | `id\|discountPercent\|detail` | `detail` hỗ trợ chuỗi Unicode có dấu, giới hạn 255 ký tự Unicode |
| `data/Device/Thongsokythuat.txt` | 8 | `specsId\|screen\|os\|camera\|cpu\|ram\|rom\|battery` | Bảng đặc tả phần cứng chi tiết của máy |
| `data/people/Customers.txt` | 7 | `id\|fullName\|phone\|address\|email\|loyaltyPoints\|memberTier` | Hạng thành viên (Standard, Silver, Gold, Platinum) |
| `data/people/Employee.txt` | 7 | `id\|fullName\|role\|phone\|email\|salary\|status` | Phân quyền vai trò: 1-Admin, 2-Sales, 3-Warehouse |
| `data/people/Suppliers.txt` | 5 | `id\|name\|phone\|address\|taxCode` | Danh bạ đối tác cung ứng thiết bị |
| `data/system/id_counters.txt` | 2 | `entityType\|currentValue` | Đảm bảo tính duy nhất và tăng tự động của các mã định danh |

### 4.2. Cơ chế ghi tệp an toàn (Safe FileStore Atomic Write)
Để tránh rủi ro hỏng tệp khi mất điện hoặc lỗi giữa chừng, lớp `FileStore` (`source/utils/FileStore.cpp`) tuân thủ nghiêm ngặt quy trình:
1. **Kiểm tra đầu vào (Field Validation):** Mọi trường văn bản trước khi ghi đều được kiểm tra qua `kiemTraTruong()`. Nếu chứa ký tự cấm `|` hoặc ký tự xuống dòng `\n`, hệ thống sẽ từ chối để chống lỗi vỡ định dạng cấu trúc tệp.
2. **Ghi vào tệp tạm thời:** Toàn bộ nội dung mới được ghi ra tệp tạm có đuôi `.tmp` (ví dụ: `data/PAY/Invoices.txt.tmp`).
3. **Đồng bộ hóa dữ liệu xuống đĩa (Flush):** Đảm bảo buffer ghi tệp hoàn tất và đóng luồng tệp tạm an toàn.
4. **Đổi tên nguyên tử (Atomic Rename):** Sử dụng hàm hệ thống để thay thế tệp tạm vào tệp đích. Nếu có sự cố tại bất kỳ bước nào trước đó, tệp gốc vẫn được giữ nguyên vẹn 100%.

---

## CHƯƠNG 5: CÁC QUY TẮC NGHIỆP VỤ CỐT LÕI (BUSINESS RULES)

### 5.1. Quy tắc tính toán tài chính và tiền tệ VND
- Mọi biến lưu trữ tiền tệ đều dùng `long long` biểu diễn đơn vị Đồng Việt Nam.
- **Công thức tính hóa đơn bán lẻ POS:**
  $$\text{tamTinh} = \sum_{i=1}^{n} (\text{quantity}_i \times \text{unitPrice}_i)$$
  $$\text{tienSauGiam} = \max(0, \text{tamTinh} - \text{giamGia})$$
  $$\text{tienThue} = \text{round}(\text{tienSauGiam} \times 0.08) \quad (\text{Thuế VAT 8\% tính sau giảm giá})$$
  $$\text{tongThanhToan} = \text{tienSauGiam} + \text{tienThue}$$
  $$\text{loiNhuanGop} = \text{tienSauGiam} - \sum_{i=1}^{n} (\text{quantity}_i \times \text{costPrice}_i)$$

### 5.2. Quy tắc tồn kho động (Dynamic Inventory)
- Tồn kho khả dụng của sản phẩm $P$ tại bất kỳ thời điểm nào được tính theo công thức:
  $$\text{Tồn kho}(P) = \sum \text{Số lượng nhập từ phiếu HoatDong} - \sum \text{Số lượng bán từ hóa đơn DaThanhToan}$$
- **Quy tắc chặn bán âm:** Khi lập hóa đơn hoặc duyệt đơn hàng, hệ thống kiểm tra số lượng yêu cầu $Q_{req} \le \text{Tồn kho}(P)$. Nếu không đủ tồn kho, từ chối toàn bộ hóa đơn ngay lập tức (tính nguyên tử, không trừ dở dang).
- **Quy tắc hủy hóa đơn:** Khi một hóa đơn hợp lệ bị hủy (chuyển trạng thái sang `DaHuy`), số lượng các sản phẩm trong hóa đơn tự động được hoàn trả vào tồn kho khả dụng ngay lập tức. Không bao giờ xóa vật lý dòng dữ liệu trong tệp.

### 5.3. Quy tắc cập nhật và hoàn nguyên giá vốn
- Giá vốn của sản phẩm được ghi nhận theo giá nhập của phiếu nhập kho gần nhất có trạng thái `HoatDong`.
- Khi một phiếu nhập kho bị hủy (do sai sót nhà cung cấp):
  - Số lượng hàng của phiếu đó bị trừ khỏi tồn kho.
  - Giá vốn của sản phẩm tự động quay về (rollback) giá vốn của phiếu nhập hợp lệ liền trước. Nếu không còn phiếu nhập nào, giá vốn quay về 0.

---

## CHƯƠNG 6: THIẾT KẾ VÀ XÂY DỰNG GIAO DIỆN NGƯỜI DÙNG (GTK4 GUI)

### 6.1. Bố cục tổng thể (Window Layout)
Giao diện ứng dụng máy tính xây dựng trên chuẩn GTK4 với kiến trúc Modern Dashboard:
- **Thanh bên điều hướng (Sidebar Navigation):** Hiển thị danh mục các phân hệ chức năng tương ứng với vai trò của tài khoản đã đăng nhập. Có hiệu ứng kích hoạt trang hiện tại.
- **Thanh tiêu đề trên (Header Bar):** Hiển thị thông tin phiên làm việc: Họ tên người dùng, Vai trò, Nút Đổi mật khẩu và Nút Đăng xuất.
- **Vùng nội dung chính (Main Content Area):** Sử dụng `GtkStack` chuyển trang mượt mà, không giật lag.

### 6.2. Phân hệ các màn hình theo vai trò người dùng
1. **Màn hình Đăng nhập (`LoginScreen`):** Kiểm tra tên đăng nhập, mật khẩu, và trạng thái tài khoản (`HoatDong` hay `BiKhoa`).
2. **Màn hình Bán hàng POS (`SalesScreens`):**
   - Lưới sản phẩm trực quan, tìm kiếm nhanh theo tên hoặc quét mã.
   - Bảng giỏ hàng hiển thị tồn kho hiện tại theo thời gian thực.
   - Nút áp dụng mã giảm giá tự động cập nhật số tiền giảm và thuế VAT 8%.
   - Chống bấm nhiều lần: Nút "Thanh toán" tự động chuyển sang trạng thái Disable trong lúc đang xử lý ghi tệp an toàn.
3. **Màn hình Quản lý Kho (`WarehouseScreens`):**
   - Bảng theo dõi tồn kho và giá vốn thời gian thực.
   - Form lập phiếu nhập kho: Chọn nhà cung cấp, nhập đơn giá, số lượng.
   - Quản lý nhật ký nhập kho và chức năng hủy phiếu nhập an toàn.
4. **Màn hình Quản trị (`AdminScreens`):**
   - Báo cáo thống kê: Biểu đồ doanh thu, tổng chi phí giá vốn, lợi nhuận ròng.
   - Quản lý nhân sự: Cập nhật thông tin, thay đổi lương cơ bản, kích hoạt/khóa tài khoản.
   - Quản lý chương trình khuyến mãi: Tạo mã giảm giá có trường mô tả tiếng Việt chi tiết.
5. **Màn hình Khách hàng (`CustomerScreens`):**
   - Danh mục sản phẩm trực quan kèm hình ảnh và thông số kỹ thuật chi tiết.
   - Giỏ hàng cá nhân, đặt hàng trực tuyến chọn địa chỉ nhận hàng.
   - Quản lý hồ sơ cá nhân và gửi phiếu yêu cầu bảo hành trực tuyến.

---

## CHƯƠNG 7: KẾT QUẢ KIỂM THỬ VÀ ĐÁNH GIÁ HỆ THỐNG

### 7.1. Bộ kiểm thử tự động (51 Unit & Integration Tests)
Hệ thống tích hợp bộ kiểm thử độc lập tại `tests/unit_tests.cpp`, có thể kích hoạt qua lệnh `mingw32-make test`. Kết quả thực tế đạt tỷ lệ **100% PASS (51/51 ca kiểm thử)**:

| Phân hệ kiểm thử | Số ca test | Kết quả | Các trường hợp tiêu biểu đã chứng minh |
|:---|:---:|:---:|:---|
| **1. Tiện ích FileStore** | 8 | 8/8 PASS | Đọc/ghi tiếng Việt có dấu, chặn ký tự `\|` và `\n`, bảo toàn dữ liệu gốc khi ghi thất bại, dọn sạch tệp `.tmp` tồn đọng |
| **2. InventoryService** | 6 | 6/6 PASS | Tồn kho chuỗi (nhập 10 - bán 3 + nhập 5 - hủy phiếu 5 = còn 7), hoàn nguyên giá vốn khi hủy phiếu nhập, chặn bán âm khi vượt tồn kho |
| **3. InvoiceService** | 9 | 9/9 PASS | Tạm tính 2 sản phẩm, tính thuế 8% sau giảm giá, tính lợi nhuận gộp từ giá vốn cố định dòng D, hoàn tồn kho khi hủy hóa đơn, chặn tạo hóa đơn nếu thiếu hàng |
| **4. MigrateData tích hợp** | 11 | 11/11 PASS | Chuẩn hóa toàn bộ bộ dữ liệu mẫu ban đầu: tách dòng H và D hóa đơn, loại bỏ hoàn toàn cột quantity trong `Products.txt` |
| **5. Core Data Structures & Extended** | 17 | 17/17 PASS | Kiểm tra tính co giãn của `Vector`, xử lý xung đột băm trong `HashTable`, duyệt thứ tự cây `BST`, cơ chế FIFO của `Queue` |

---

## CHƯƠNG 8: KẾT LUẬN VÀ HƯỚNG PHÁT TRIỂN

### 8.1. Các kết quả đã đạt được
1. Đã xây dựng hoàn chỉnh một ứng dụng quản lý bán hàng máy tính để bàn (Desktop App) với giao diện GTK4 mượt mà, chuyên nghiệp.
2. Tự cài đặt thành công và làm chủ 100% các cấu trúc dữ liệu nền tảng (`Vector`, `HashTable`, `BST`, `Queue`, `Stack`), đáp ứng xuất sắc các tiêu chí học thuật của đồ án môn học PBL2.
3. Giải quyết triệt để bài toán đồng bộ tồn kho thời gian thực và quản lý tài chính an toàn (số nguyên VND, thuế sau giảm giá, giao dịch tệp nguyên tử).
4. Hệ thống mã nguồn sạch sẽ, có bộ kiểm thử tự động 51 ca test và tài liệu hướng dẫn kỹ thuật chi tiết theo chuẩn quốc tế.

### 8.2. Hạn chế và Hướng phát triển trong tương lai
- **Hạn chế:** Hệ thống hiện tại lưu trữ trên các tệp văn bản `.txt`. Khi dữ liệu đạt hàng triệu dòng, tốc độ quét tuần tự có thể bị suy giảm so với các hệ quản trị cơ sở dữ liệu chuyên dụng.
- **Hướng phát triển:**
  - Nâng cấp tầng lưu trữ sang cơ sở dữ liệu quan hệ SQLite hoặc PostgreSQL thông qua cơ chế trừu tượng hóa của tầng Repository hiện có.
  - Tích hợp cổng thanh toán trực tuyến quét mã QR (VietQR, VNPay API).
  - Kết nối với máy in nhiệt POS chuẩn ESC/POS để in hóa đơn bán lẻ trực tiếp tại quầy.
