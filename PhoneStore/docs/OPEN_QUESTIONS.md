# OPEN_QUESTIONS.md — Các vấn đề thiết kế & quyết định trong PhoneStore

Tài liệu này ghi nhận các quyết định thiết kế đã chốt tạm thời và các điểm cần làm rõ thêm với chủ dự án.

---

## 1. Đã chốt tạm

Các quyết định dưới đây được thống nhất và áp dụng trong đợt refactor (theo tài liệu `ANTY.md`):

| # | Chủ đề | Quyết định |
|---|---|---|
| **D1** | Giỏ hàng | **Không giữ chỗ.** `addItem` / `updateQuantity` chỉ kiểm tra tồn kho khả dụng (`checkAvailable`), không thay đổi `reservedQuantity`. Giỏ hàng chưa phải đơn nên không có trạng thái `PENDING`. |
| **D2** | Chọn món để đặt | Mỗi dòng giỏ hàng (`CartItem`, `CartLineView`) có cờ `selected` (mặc định `true`). Khi checkout, chỉ các dòng đã chọn được đưa vào đơn nháp (`OrderDraft`). Dòng không chọn vẫn ở lại giỏ. |
| **D3** | Giữ chỗ | Chỉ diễn ra trong `OrderService::createOrder` (đơn `PENDING`, `InventoryState::RESERVED`). Ai đặt trước thì được giữ chỗ trước. Đặt sau mà hết hàng thì bị từ chối với lỗi `OutOfStock`. Không có hàng đợi chờ tồn kho. |
| **D4** | Thiếu hàng khi đặt | **All-or-nothing:** Một dòng bất kỳ thiếu hàng hoặc lỗi thì toàn bộ đơn hàng bị từ chối, `UnitOfWork` tự động rollback, giữ nguyên giỏ hàng của khách và báo rõ sản phẩm cùng số lượng còn lại. |
| **D5** | PENDING giữ hàng lâu | Tự hủy đơn `PENDING` quá hạn (kiểu lười - lazy cleanup, không cần thread nền) thông qua `OrderService::cancelExpiredPending()`, được gọi ở đầu `createOrder`, `listByStatus` và khi khởi động ứng dụng. Giới hạn mỗi khách tối đa 3 đơn `PENDING`. |
| **D6** | Nhập hàng | Dòng phiếu nhập kho (`ImportLine`) có thể là **sản phẩm có sẵn** hoặc **sản phẩm mới** (`NewPhoneInfo`). Sản phẩm mới được tạo **trong cùng một `UnitOfWork`** với phiếu nhập kho, đảm bảo tính nguyên tử (atomic). |
| **D7** | Hồ sơ tối thiểu khi nhập | Bắt buộc: Tên, Hãng, Số lượng, Giá nhập. Mã sản phẩm sinh tự động dạng `PHN-xxxxxx`. Giá bán mặc định `0`. Cờ đăng bán mặc định `listed = false`. |
| **D8** | Đăng bán | Sản phẩm tạo từ phiếu nhập có `listed = false` (ẩn với khách, không xuất hiện trong tìm kiếm, danh mục, lọc giá). Admin hoàn thiện thông tin, đặt giá bán (`sellingPrice > 0`) rồi gọi `publishPhone`. |
| **D9** | Phân quyền | Nhân viên kho (`WAREHOUSE_EMPLOYEE`) được cấp quyền `CreateImportReceipt` để lập phiếu nhập. Chỉ Admin có quyền `ManagePhone` (thêm, sửa, đăng bán, đặt giá). Nhân viên bán hàng (`SALE_EMPLOYEE`) không sửa danh mục sản phẩm. |

---

## 2. Các điểm mở cần quyết định tiếp

1. **Thời hạn PENDING (`PENDING_ORDER_TTL_HOURS`):**
   - Hiện tại: Đặt mặc định là 24 giờ trong `src/common/Config.h`.
   - Câu hỏi: Thời gian này có cần rút ngắn xuống 15-30 phút đối với các mùa flash sale hoặc đợt giảm giá cao điểm không? Có cần cấu hình động theo loại mặt hàng không?

2. **Hiển thị sản phẩm hết hàng cho khách hàng (`PhoneView.inStock`):**
   - Hiện tại: `PhoneService::listForCustomer`, `search`, `filterByPrice` vẫn trả về các sản phẩm đã đăng bán (`listed == true`) dù `availableQuantity() <= 0`, với cờ `inStock = false` để khách nhận biết cửa hàng có kinh doanh mặt hàng này nhưng tạm thời hết hàng.
   - Câu hỏi: Có cần tùy chọn ẩn hoàn toàn các sản phẩm hết hàng lâu ngày không?

3. **Ai được đặt giá bán và đăng bán ngoài Admin:**
   - Hiện tại: Theo D9, chỉ `Role::ADMIN` có quyền `ManagePhone` (bao gồm `publishPhone`, `updatePhone`).
   - Câu hỏi: Quản lý kho hoặc quản lý cửa hàng (Manager) có được phân quyền đặt giá bán và duyệt đăng bán không, hay phân định vai trò Manager riêng?

4. **Phiếu điều chỉnh kho / Hủy phiếu nhập:**
   - Hiện tại: Phiếu nhập khi đã lưu và commit thì không thể xóa hoặc sửa trực tiếp nhằm bảo toàn nhật ký kho.
   - Câu hỏi: Khi nhập sai số lượng hoặc trả hàng nhà cung cấp thì cần làm phiếu xuất kho trả hàng (Return Receipt) hay phiếu kiểm kê điều chỉnh tồn kho (Stock Adjustment)?

5. **Ràng buộc tham chiếu khi xóa sản phẩm (`PhoneService::removePhone`):**
   - Hiện tại: Tạm thời giữ `TODO(logic)` về kiểm tra ràng buộc khóa ngoại (foreign key references) với phiếu nhập `StockReceipts`, đơn hàng `Orders`, và hóa đơn `Invoices`. Nếu sản phẩm đã phát sinh giao dịch thì không được xóa vật lý mà chỉ nên hủy đăng bán (`listed = false`).
   - Câu hỏi: Có nên chuyển sang cơ chế xóa mềm (soft delete) cho mọi thực thể danh mục không?

6. **Chỉ mục giá (`PhoneRepository::priceIndex`) và cờ `listed`:**
   - Hiện tại: Lọc theo cờ `listed == true` được xử lý tại tầng nghiệp vụ `PhoneService` (`listForCustomer`, `filterByPrice`, `search`), trong khi `PhoneRepository` là tầng truy xuất dữ liệu thuần túy lưu trữ toàn bộ các sản phẩm.
   - Lý do: Giữ trách nhiệm phân tách rõ ràng giữa lưu trữ dữ liệu (Repository) và quy tắc nghiệp vụ/quyền truy cập (Service). `priceIndex` trong repository không cần biết logic nghiệp vụ về cờ hiển thị.

7. **Xác nhận nhận hàng và thanh toán:**
   - Theo đặc tả bổ sung: Khi đơn hàng đang giao (`SHIPPING`) hoặc nhận tại quầy (`READY_FOR_PICKUP`), khách hàng cần tự xác nhận đã nhận hàng (`OrderService::confirmReceived`), nhân viên mới có thể hoàn tất đơn hàng (`completeOrder`).

