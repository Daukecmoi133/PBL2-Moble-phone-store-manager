# ANTY.md — Việc cần làm cho PhoneStore (giỏ hàng, giữ chỗ, nhập kho)

Tài liệu giao việc cho Antigravity. Phạm vi: `c:\DUY\PBL2\PhoneStore\src`. Bản cũ ở `c:\DUY\PBL2\source` và `src\_unplaced\legacy` **không sửa**.

## 0. Quy tắc chung

- Giữ nguyên mọi comment/docstring không liên quan đến thay đổi.
- Đa số `Service` hiện là stub (`NotImplemented`). **Chỉ làm đúng các mục dưới đây.** Không tự cài các hàm không được nêu.
- Dòng stub nào có `TODO(logic)` mà bạn cài thì xóa TODO đó. Dòng bạn không cài thì giữ nguyên.
- Tuân thủ kiến trúc hiện có:
  - `CartService` gọi `OrderService` / `PhoneService` / `DiscountService`. `OrderService` không biết `Cart`.
  - Hàm tồn kho của `PhoneService` (`reserve`, `releaseReserved`, `commitReserved`, `restock`, `receiveStock`) chỉ sửa RAM và enlist vào `UnitOfWork`. **Người gọi chịu trách nhiệm commit.**
  - Kiểm tra quyền bằng `Permission::isAllowed(actor.role, Action::...)`.
  - Trả lỗi bằng `Result<T>::fail(ErrorCode::..., "thông báo tiếng Việt")`. Thêm `ErrorCode` mới nếu cần (cập nhật cả hàm chuyển chuỗi trong `ErrorCode.h`).
  - Không đưa ID nội bộ ra DTO dành cho khách.
- Mọi thay đổi định dạng file phải tương thích ngược (dòng cũ vẫn đọc được) và được ghi vào `docs/MIGRATION.md`.
- Sau mỗi phần, build thử bằng cách sẵn có trong dự án (xem `tools\` và `tests\`), sửa lỗi biên dịch. Nếu không có hệ build thì rà soát include/chữ ký thủ công.

## 1. Quyết định thiết kế đã chốt

Các quyết định này lấy theo đề xuất đã thảo luận. Chủ dự án chưa xác nhận từng mục. Ghi chúng vào `docs/OPEN_QUESTIONS.md` mục "Đã chốt tạm".

| # | Chủ đề | Quyết định |
|---|---|---|
| D1 | Giỏ hàng | **Không giữ chỗ.** `addItem` / `updateQuantity` chỉ `checkAvailable`, không đổi `reservedQuantity`. Giỏ chưa phải đơn nên không có trạng thái PENDING. |
| D2 | Chọn món để đặt | Mỗi dòng giỏ có cờ `selected`. Chỉ dòng đã chọn vào đơn khi checkout. |
| D3 | Giữ chỗ | Chỉ xảy ra trong `OrderService::createOrder` (đơn `PENDING`, `InventoryState::RESERVED`). Ai đặt trước thì được giữ hàng trước. Đặt sau mà hết hàng thì bị từ chối. Không có hàng chờ. |
| D4 | Thiếu hàng khi đặt | **All-or-nothing:** một dòng thiếu hàng thì từ chối cả đơn, rollback, giữ nguyên giỏ, báo rõ sản phẩm và số còn lại. |
| D5 | PENDING giữ hàng lâu | Tự hủy đơn `PENDING` quá hạn (kiểu lười, không cần thread nền) và giới hạn số đơn PENDING mỗi khách. Hằng số cấu hình ở Phần B mục 5. |
| D6 | Nhập hàng | Dòng phiếu nhập có thể là **sản phẩm có sẵn** hoặc **sản phẩm mới**. Sản phẩm mới được tạo **trong cùng một `UnitOfWork`** với phiếu nhập. |
| D7 | Hồ sơ tối thiểu khi nhập | Bắt buộc: tên, hãng, số lượng, giá nhập. ID tự sinh. Các trường còn lại để trống. Giá bán mặc định `0`. |
| D8 | Đăng bán | Sản phẩm tạo từ phiếu nhập có `listed = false` (ẩn với khách). Admin hoàn thiện thông tin, đặt giá bán rồi "đăng bán". |
| D9 | Phân quyền | Kho có `CreateImportReceipt`. Chỉ Admin có `ManagePhone` (kể cả đặt giá bán và đăng bán). Sale không sửa danh mục. |

## 2. Phần A — Cờ `listed` cho `Phone` (làm trước, các phần sau phụ thuộc)

Files: `src/model/catalog/Phone.h`, `src/dto/PhoneStaffView.h`, `src/service/PhoneService.h/.cpp`, `src/repository/PhoneRepository.cpp`.

1. Thêm `bool listed` vào `Phone`, mặc định `true` trong hàm dựng. Thêm `isListed()` và `setListed(bool)`.
2. Định dạng file `Products.txt`:
   - Thêm trường thứ 10 vào cuối dòng: `listed` (`1` hoặc `0`), sau `specifications`.
   - `toFileString` luôn ghi đủ 10 trường, kể cả khi `manufacturer` hoặc `specifications` rỗng. Mục đích là giữ đúng vị trí trường.
   - `fromFileLine` giữ hỗ trợ 7, 8, 9 trường. Thiếu trường thứ 10 thì `listed = true`.
   - Nếu `specifications` có thể chứa ký tự `|`, xử lý hoặc ghi chú giới hạn này.
3. `PhoneStaffView` thêm `bool listed` và `std::string manufacturer`. `PhoneView` (khách) không đổi.
4. `PhoneService`:
   - `listForCustomer`, `search`, `filterByPrice`: **chỉ trả sản phẩm `listed == true`**. Vẫn hiển thị hàng hết (để khách thấy "hết hàng"); ghi điểm này vào OPEN_QUESTIONS.
   - `listForStaff`: trả tất cả, kể cả `listed == false`.
   - Thêm `Result<Vector<PhoneStaffView>> listDrafts(const Actor&) const` trả các sản phẩm `listed == false`, quyền `ManagePhone`.
   - Thêm `Result<void> publishPhone(const Actor&, const std::string& id)`:
     - Quyền `ManagePhone`.
     - Điều kiện: `sellingPrice > 0`, tên và hãng không rỗng. Thiếu thì trả lỗi dữ liệu không hợp lệ (dùng `ErrorCode` đang có phù hợp nhất, hoặc thêm mới) kèm lý do.
     - Đặt `listed = true`, lưu, cập nhật index giá.
   - `checkAvailable` (khi khách đặt) phải từ chối sản phẩm `listed == false`.
   - `addPhone` (do Admin gọi trực tiếp): giữ hành vi cũ, `listed = true`.
5. Chỉ mục giá (`PhoneRepository::priceIndex`): không đưa sản phẩm `listed == false` vào kết quả `findByPriceRange`, hoặc lọc ở service. Chọn một cách và ghi chú vì sao.

## 3. Phần B — Giỏ hàng có tick chọn + checkout an toàn

Files: `src/model/sales/Cart.h`, `src/dto/CartLineView.h`, `src/service/CartService.h/.cpp`.

1. `CartItem` thêm `bool selected` (mặc định `true` như Shopee khi mới thêm). Thêm getter/setter.
   - `Cart::addItem` khi cộng dồn vào dòng đã có thì không đổi `selected`.
   - Thêm `Cart::getSelectedItems()` trả các dòng đã chọn, và `Cart::removeSelected()`.
   - `calculateTotal()` của `Cart` giữ nguyên (tổng cả giỏ). Thêm `calculateSelectedTotal()`.
2. `CartLineView` thêm `bool selected`.
3. `CartService.h` thêm `Result<void> setSelected(const Actor&, const std::string& phoneId, bool selected)` và `Result<void> setAllSelected(const Actor&, bool selected)`.
4. Cài các hàm của `CartService`:
   - Giỏ lưu trong `Vector<Cart> carts` theo `actor.userId`, tạo giỏ khi chưa có. Giỏ không lưu file.
   - Chỉ `Role::CUSTOMER` được dùng giỏ. Vai trò khác thì `PermissionDenied`.
   - `getCart`: đổi sang `CartLineView` (không ID), ghi `phoneId` vào `outIds` đúng thứ tự dòng. Dùng **giá hiện tại** của sản phẩm để hiển thị. Dòng trỏ tới sản phẩm đã bị ẩn (`listed == false`) hoặc đã xóa vẫn hiển thị nhưng gắn nhãn không đặt được. Nếu cần, thêm `bool available` vào `CartLineView`.
   - `addItem`: `quantity > 0`, `checkAvailable(phoneId, quantity + số lượng đang có trong giỏ)`, rồi cộng dồn. Không giữ chỗ.
   - `updateQuantity`: `<= 0` thì xóa dòng; ngược lại `checkAvailable` rồi cập nhật.
   - `removeItem`, `clear`: như TODO hiện có.
   - `calculateTotal`: **chỉ tính dòng đã chọn**; trừ `discountService.validateAndCalculate` nếu có mã. Không có dòng nào được chọn thì trả lỗi dữ liệu không hợp lệ ("Chưa chọn sản phẩm nào").
   - `checkout`:
     1. Lấy các dòng đã chọn; rỗng thì lỗi.
     2. Dựng `OrderDraft` từ dòng đã chọn và `CheckoutInfo` (xem `OrderDraft.h`, `CheckoutInfo.h`).
     3. Gọi `orderService.createOrder`.
     4. **Chỉ khi thành công**, xóa **các dòng đã chọn** khỏi giỏ (dòng chưa chọn ở lại).
     5. Thất bại thì giữ nguyên giỏ và trả lỗi gốc của `createOrder` (nêu rõ sản phẩm thiếu và số còn lại).
5. Hằng số cấu hình đặt tại một nơi dùng chung (ví dụ `src/common/Config.h`, tạo mới nếu chưa có):
   - `PENDING_ORDER_TTL_HOURS = 24`
   - `MAX_PENDING_ORDERS_PER_CUSTOMER = 3`
   - `MAX_QUANTITY_PER_LINE = 5`
   - `MAX_QUANTITY_PER_LINE` áp dụng ở `addItem` và `updateQuantity`.

## 4. Phần C — `OrderService`: giữ chỗ nguyên tử và hết hạn PENDING

Files: `src/service/OrderService.h/.cpp`, `src/model/sales/Order.h`, `src/repository/OrderRepository.*`.

1. `createOrder` (theo TODO đã có trong code):
   - Quyền `CreateOrder` (khách).
   - Đơn không rỗng, số lượng hợp lệ, mỗi dòng `<= MAX_QUANTITY_PER_LINE`.
   - Gọi hàm dọn đơn quá hạn (mục 2 bên dưới) **trước** khi kiểm tra tồn kho, để hàng của đơn hết hạn được nhả.
   - Đếm đơn `PENDING` hiện có của khách. Đạt `MAX_PENDING_ORDERS_PER_CUSTOMER` thì từ chối.
   - **Trong một `UnitOfWork`:** với từng dòng `checkAvailable` rồi `reserve`; áp dụng mã giảm giá (`consumeUsage`) nếu có; lưu đơn `PENDING` với `inventoryState = RESERVED`; commit. Bước kiểm tra và giữ chỗ không được tách rời (tránh hai khách cùng thấy "còn 1" rồi cùng đặt).
   - Bất kỳ dòng nào thất bại thì rollback toàn bộ, không để lại `reservedQuantity` lệch (quyết định D4).
   - Lưu `createdAt` của đơn (nếu `Order` chưa có thì thêm; cần tương thích file cũ, thiếu thì coi là không hết hạn).
2. Hết hạn đơn PENDING:
   - Thêm hàm riêng, ví dụ `int OrderService::cancelExpiredPending()`, **không cần Actor**. Chỉ dùng nội bộ hoặc do hệ thống gọi.
   - Duyệt đơn `PENDING` có `createdAt + TTL < now`. Với mỗi đơn: `releaseReserved` từng dòng, `releaseUsage` nếu có mã, đặt `inventoryState = RELEASED`, trạng thái đơn `CANCELLED`, commit. Mỗi lượng hàng chỉ nhả đúng một lần theo `inventoryState`.
   - Gọi hàm này ở các điểm: đầu `createOrder`, `listByStatus`, và khi khởi động ứng dụng. Không tạo thread nền.
3. `cancelOrder`, `confirmOrder` và các hàm khác: **không đổi** trong phạm vi tài liệu này, trừ khi cần để biên dịch.

## 5. Phần D — Nhập kho với sản phẩm mới

Files: `src/dto/` (mới), `src/service/ImportReceiptService.h/.cpp`, `src/service/PhoneService.h/.cpp`, `src/common/Permission.cpp`.

1. Quyền trong `Permission.cpp`:
   - `CreateImportReceipt`: `Role::WAREHOUSE_EMPLOYEE` (Admin đã qua dòng đầu).
   - `ManagePhone`: giữ chỉ Admin. Các hành động khác giữ nguyên `false`.
   - Sửa comment tương ứng và ghi vào `docs/OPEN_QUESTIONS.md`.
2. Tạo DTO `src/dto/ImportLine.h`:
   ```cpp
   struct NewPhoneInfo {
       std::string name;            // bắt buộc
       std::string brand;           // bắt buộc
       std::string operatingSystem; // tùy chọn
       std::string manufacturer;    // tùy chọn
       std::string specifications;  // tùy chọn
   };
   struct ImportLine {
       std::string phoneId;   // có giá trị: sản phẩm có sẵn. Rỗng: sản phẩm mới
       NewPhoneInfo newPhone; // chỉ dùng khi phoneId rỗng
       int quantity = 0;      // > 0
       Money unitPrice = 0;   // giá nhập, > 0
   };
   ```
3. `PhoneService` thêm `Result<std::string> createDraftForImport(UnitOfWork& uow, const NewPhoneInfo& info, Money importPrice)`:
   - Không nhận `Actor` (quyền đã kiểm ở `createReceipt`). Chỉ gọi từ `ImportReceiptService`.
   - Validate tên và hãng không rỗng. Chống trùng (cùng tên + hãng, không phân biệt hoa thường) thì lỗi, vì nên nhập vào sản phẩm có sẵn.
   - Sinh ID mới theo quy ước ID sẵn có của dự án (xem cách `Repository` đang sinh ID; không đoán. Nếu chưa có thì viết hàm sinh ID không trùng, nhất quán với định dạng ID trong `data\`).
   - Tạo `Phone` với `quantity = 0` (tồn kho sẽ do `receiveStock` cộng), `listed = false`, `sellingPrice = 0`, `importPrice = importPrice`. Enlist vào `uow`, chỉ sửa RAM.
   - Trả về ID mới.
4. `ImportReceiptService::createReceipt` đổi chữ ký để nhận `Vector<ImportLine>` (giữ `ImportReceipt` và định dạng file phiếu như cũ). Ví dụ:
   `Result<std::string> createReceipt(const Actor&, const std::string& supplierId, const Vector<ImportLine>& lines);`
   - Quyền `CreateImportReceipt`.
   - Xác minh nhà cung cấp qua `supplierService`.
   - Danh sách dòng không rỗng; `quantity > 0`; `unitPrice > 0`.
   - **Một `UnitOfWork`**, theo thứ tự:
     1. Dòng sản phẩm mới: `createDraftForImport` lấy ID.
     2. Dòng sản phẩm có sẵn: kiểm tra tồn tại.
     3. `receiveStock(uow, id, quantity)` cho mọi dòng.
     4. Cập nhật `Phone.importPrice` bằng giá của lần nhập này (giá gần nhất).
     5. Dựng `ImportReceipt` bằng `OrderDetail(phoneId, phoneName, quantity, unitPrice)`. `phoneName` chụp lại tên tại thời điểm nhập.
     6. Lưu phiếu và commit. Lỗi ở đâu thì rollback hết, không để lại sản phẩm mồ côi.
   - Cùng một sản phẩm xuất hiện ở nhiều dòng thì **từ chối** để phiếu rõ ràng.
5. `getById` và `list`: theo TODO có sẵn (kiểm tra quyền rồi tra repo).
6. Sửa các nơi gọi `createReceipt` cũ (controller, test) cho khớp chữ ký mới.
7. `PhoneService::removePhone`: không cho xóa nếu đã có phiếu nhập, đơn hoặc hóa đơn tham chiếu. Nếu chưa có cơ chế tra tham chiếu thì để nguyên TODO và ghi vào OPEN_QUESTIONS.

## 6. Phần E — Docs và test

1. Tạo (thư mục `PhoneStore\docs` hiện đang trống, dù code tham chiếu tới các file này) hoặc cập nhật:
   - `docs/OPEN_QUESTIONS.md`: mục "Đã chốt tạm" (bảng ở mục 1) và các điểm còn mở: thời hạn PENDING, có hiển thị hàng hết cho khách không, ai đặt giá bán ngoài Admin, phiếu điều chỉnh hoặc hủy phiếu nhập.
   - `docs/MIGRATION.md`: trường `listed` mới ở `Products.txt` (10 trường, tương thích 7/8/9), và `createdAt` của `Order` nếu thêm.
2. Test trong `tests\` (theo cách dự án đang chạy test; nếu chưa có framework thì viết file test console đơn giản biên dịch được):
   - Giỏ: thêm, cộng dồn, tick và bỏ tick, tổng chỉ tính dòng đã chọn, không chọn gì thì checkout lỗi, checkout thành công chỉ xóa dòng đã chọn.
   - Giữ chỗ: hai khách cùng đặt món chỉ còn 1, người đầu thành công, người sau bị `OutOfStock`; `reservedQuantity` đúng và giỏ người sau còn nguyên.
   - All-or-nothing: đơn 2 dòng, dòng thứ 2 thiếu hàng thì dòng thứ 1 không bị giữ chỗ.
   - Hết hạn: đơn PENDING quá TTL thì `cancelExpiredPending` nhả `reservedQuantity` đúng một lần, gọi lại không nhả lần hai.
   - Giới hạn: đơn PENDING thứ 4 của một khách bị từ chối.
   - Nhập kho: phiếu có dòng sản phẩm mới thì tạo `Phone` `listed = false`, `quantity` đúng, ẩn khỏi `listForCustomer`, hiện ở `listDrafts`; `publishPhone` thất bại khi `sellingPrice == 0`, thành công sau khi đặt giá; phiếu lỗi giữa chừng thì không có sản phẩm mới nào được lưu.
   - Quyền: kho nhập hàng được, kho không gọi được `publishPhone`, khách không gọi được `createReceipt`.
   - File: `Products.txt` dòng 7, 8, 9 trường vẫn đọc được với `listed = true`; ghi rồi đọc lại giữ nguyên `listed`.

## 7. Thứ tự thực hiện

1. Phần A (nền `listed` và `Products.txt`)
2. Phần D (phụ thuộc A)
3. Phần C (độc lập A, D)
4. Phần B (checkout chạy thật cần C; test checkout sau C)
5. Phần E (docs, test)

## 8. Tiêu chí hoàn thành

- Dự án biên dịch được. Test ở Phần E chạy qua.
- Không có `NotImplemented` ở các hàm được yêu cầu cài.
- Khách không bao giờ thấy sản phẩm `listed == false` (danh sách, tìm kiếm, lọc giá, đặt hàng).
- `reservedQuantity` không bao giờ âm hoặc vượt `quantity`; mỗi lượng hàng chỉ giải phóng hoặc hoàn đúng một lần.
- Cuối cùng báo lại: file đã đổi, quyết định D1-D9 nào bị thay đổi (nếu có) và việc còn dở.


# ANTY.md - Việc cần làm xử lí đơn hàng => thiết kế lại khả năng khách hàng xác nhận trả tiền

- Hiện khách hàng chỉ xem được trạng thái, khi đơn hàng ở trạng thái đang giao hàng(hoặc tương tự khi đơn hàng nhận trực tiếp tại cửa hàng) thì chỉ khi khách hàng tự xác nhận nhận hàng(và status thanh toán đã xong) thì nhân viên mới xác nhận đơn hàng hoàn thành được