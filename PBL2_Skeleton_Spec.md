# PBL2 – Quản lí cửa hàng điện thoại: Đặc tả dựng bộ khung kiến trúc 5 tầng

> Tài liệu giao cho AI coder. Đã được chủ dự án chốt kiến trúc.
> Tên class/file trong tài liệu lấy từ bản audit của Copilot nên **có thể khác code thật**: phải đối chiếu với code hiện có trước khi làm (xem mục 10).

---

## 1. Mục tiêu và phạm vi

### Việc cần làm (đợt này)
1. Dựng **bộ khung biên dịch được**: thư mục, file header/source, khai báo class và hàm, stub rỗng.
2. **Chuyển code hiện có** vào đúng vị trí trong cây thư mục (mục 10).
3. Dựng cơ chế dùng chung: `Result<T>`, `ErrorCode`, `Actor`, `Permission`, `UnitOfWork`, `Repository<T>`, `FileManager`.
4. Dựng `AppContext` nối mọi thành phần, `main.cpp` chạy được và thoát sạch.
5. Dựng script kiểm tra luật phụ thuộc giữa các tầng.

### Việc KHÔNG làm (đợt này)
- Không viết logic nghiệp vụ mới. Mọi hàm nghiệp vụ chưa có code cũ thì là stub trả `Result::fail(ErrorCode::NotImplemented, "TODO")`.
- Không sửa bug trong code cũ (danh sách bug sẽ làm đợt sau). Nếu bắt buộc phải chạm vào code cũ để biên dịch, xem quy tắc đánh dấu ở mục 10.
- Không viết giao diện GTK4/Qt (thư mục `view/gtk/` để trống, chỉ có `.gitkeep`).
- Không thêm thư viện ngoài. Không tự quyết điểm còn mở: ghi vào `docs/OPEN_QUESTIONS.md`.

---

## 2. Bối cảnh dự án (tóm tắt)
- Đồ án môn học PBL2: ứng dụng quản lí cửa hàng điện thoại, C++ hướng đối tượng, **hạn chế tối đa thư viện có sẵn**.
- Phải dùng cấu trúc dữ liệu tự cài và giải thích được lý do chọn; dữ liệu lưu bằng file `.txt`; cần backup/restore và import/export.
- Vai trò: **Admin/Manager**, **Sales Employee**, **Warehouse Employee**, **Customer**.
- Chức năng: sản phẩm, khách hàng, nhân viên, đơn hàng, giỏ hàng, mã giảm giá, hóa đơn, nhập hàng/nhà cung cấp, thống kê.
- Giao diện: làm bản console trước; GTK4 làm sau nếu còn thời gian. Vì vậy tầng View phải đặt sau **interface** để thay thế được.

---

## 3. Kiến trúc 5 tầng

```
View  →  Controller  →  Service  →  Repository  →  Data Structure + FileManager
```

| Tầng | Thư mục | Vai trò |
|---|---|---|
| 1. View | `src/view/` | Hiển thị form, bảng, nút bấm, thông báo |
| 2. Controller | `src/controller/` | Nhận sự kiện, lấy input, gọi Service, cập nhật View |
| 3. Service | `src/service/` | Xử lý nghiệp vụ, kiểm tra điều kiện, phối hợp các thành phần |
| 4. Repository | `src/repository/` | Tra cứu, thêm, sửa, xóa và quản lý dữ liệu |
| 5. Data Structure + FileManager | `src/storage/` | Tổ chức dữ liệu và đọc/ghi file `.txt` |

Phần dùng chung (không thuộc tầng nào): `src/model/`, `src/dto/`, `src/common/`. Composition root: `src/app/`.

### Ma trận được phép `#include`

| Thư mục | Được include |
|---|---|
| `common` | chỉ thư viện chuẩn |
| `storage/ds`, `storage/algo` | `common` |
| `storage/file` | `common`, `storage/ds` |
| `model` | `common`, `storage/ds` |
| `dto` | `common`, `model/enums`, `storage/ds` |
| `repository` | `storage` (ds, algo, file), `model`, `common` |
| `service` | `repository`, `model`, `dto`, `common`, `storage/ds`, `storage/algo` |
| `controller` | `view/interfaces`, `service`, `dto`, `common`, `model/enums` |
| `view` | `view/interfaces`, `dto`, `common`, `model/enums` |
| `app` | tất cả (chỉ nơi này được tạo và nối các đối tượng) |

Cấm tuyệt đối: View/Controller include `repository`, `storage/file`; Service include `view`, `controller`, `storage/file`; Repository include `service`; `storage` include `model`.

---

## 4. Cây thư mục

```
PhoneStore/
├─ CMakeLists.txt
├─ src/
│  ├─ view/
│  │  ├─ interfaces/        IView, ILoginView, ICatalogView, ICartView, IOrderView,
│  │  │                     IStaffDeskView, IWarehouseView, IAdminView, IViewFactory
│  │  ├─ console/           ConsoleViewFactory + bản console của từng interface
│  │  └─ gtk/               (trống, .gitkeep)
│  ├─ controller/
│  │  AuthController, CatalogController, CartController, OrderController,
│  │  CustomerController, EmployeeController, DiscountController,
│  │  InventoryController (kho, nhập hàng, nhà cung cấp), InvoiceController,
│  │  ReportController, DataController (backup, restore, import, export)
│  ├─ service/
│  │  AuthService, PhoneService, CustomerService, EmployeeService, DiscountService,
│  │  CartService, OrderService, OrderStateMachine, InvoiceService,
│  │  SupplierService, ImportReceiptService, ReportService, DataService
│  ├─ repository/
│  │  Repository<T> (lớp cơ sở), IRepositoryTx, UnitOfWork,
│  │  PhoneRepository, CustomerRepository, EmployeeRepository, OrderRepository,
│  │  DiscountRepository, InvoiceRepository, SupplierRepository,
│  │  ImportReceiptRepository, DataRepository (bọc backup/import/export)
│  ├─ storage/
│  │  ├─ ds/                Vector, HashTable, BST, Queue
│  │  ├─ algo/              Sort (merge/quick sort tự cài)
│  │  └─ file/              FileManager, BackupManager, LineCodec, ImportExport
│  ├─ model/
│  │  ├─ enums/             Role, OrderStatus, PaymentStatus, PaymentMethod,
│  │  │                     DeliveryType, InventoryState
│  │  ├─ people/            People (abstract) → Customer, Employee → Manager/SalesEmployee/WarehouseEmployee; Account, Name
│  │  ├─ catalog/           Phone (có reservedQuantity)
│  │  ├─ sales/             Cart, CartItem, Order, OrderDetail, ShippingInfo,
│  │  │                     Payment, DiscountCode, Invoice
│  │  └─ supply/            Supplier, ImportReceipt, ImportReceiptDetail
│  ├─ dto/                  PhoneView, PhoneStaffView, OrderSummary, OrderDraft,
│  │                        CheckoutInfo, ReportRow, ImportReport ...
│  ├─ common/               Result, ErrorCode, Money, DateTime, StringUtil,
│  │                        Validator, Actor, Permission
│  └─ app/                  AppContext, Session, main.cpp
├─ data/                    *.txt (tạo rỗng, mỗi file có dòng đầu "#v1");  data/backup/
├─ tests/                   smoke_test.cpp, README.md
├─ tools/                   check_layers.sh
└─ docs/                    MIGRATION.md, OPEN_QUESTIONS.md
```

Mỗi class: một cặp `.h`/`.cpp`. Template (`Vector`, `HashTable`, `BST`, `Queue`, `Repository<T>`, `Result<T>`) viết header-only.

---

## 5. Nhiệm vụ và điều cấm của từng tầng

| Tầng | Được làm | Cấm |
|---|---|---|
| **View** | Vẽ form/bảng/nút; nhận thao tác người dùng và báo cho handler; hiển thị thông báo | Gọi Service/Repository; chứa quy tắc nghiệp vụ; hiển thị ID nội bộ của sản phẩm cho khách |
| **Controller** | Nhận sự kiện từ View, lấy input, lấy `Actor` từ Session, gọi Service, đổi `Result` thành cập nhật View | Gọi Repository/FileManager; kiểm tra nghiệp vụ; biết View là console hay GTK (chỉ biết interface) |
| **Service** | Kiểm tra quyền và điều kiện, máy trạng thái đơn, tồn kho, thanh toán; điều phối nhiều Repository qua `UnitOfWork` | Include View/Controller; đọc/ghi file trực tiếp; trả con trỏ/reference sửa được ra ngoài; giữ bản sao dữ liệu thay cho Repository |
| **Repository** | Thêm, sửa, xóa, tra cứu; **giữ dữ liệu chính và các index**; nạp/lưu file qua FileManager; snapshot/rollback | Chứa quy tắc nghiệp vụ (đủ tồn kho không, ai có quyền) |
| **Data Structure + FileManager** | Container tổng quát; đọc/ghi file tạm rồi đổi tên; backup/restore; import/export dòng văn bản | Biết thực thể nào (`Phone`, `Order`...) hay quy tắc nghiệp vụ |

Quy tắc kèm theo:
- Phân quyền thật nằm ở Service; View chỉ ẩn nút cho đẹp.
- Mọi hàm ghi trong Service nhận `const Actor&` (trừ đăng nhập và đăng ký khách).
- Mỗi Repository tự cài `serialize/deserialize` thực thể của mình; FileManager chỉ làm việc với dòng văn bản.
- Đọc trả **bản sao hoặc const view**, không trả con trỏ, reference hay Queue sửa được. Ngoại lệ duy nhất: `EmployeeRepository` sở hữu các `Employee*` nội bộ (đa hình), nhưng bên ngoài chỉ nhận bản sao qua `clone()` bọc trong `std::unique_ptr`.
- Index (HashTable theo ID/username/phone/mã voucher, BST theo giá) **không lưu bản sao thực thể**, chỉ lưu vị trí hoặc ID, và được dựng lại sau mỗi thay đổi.
- Hàng đợi đơn PENDING nằm trong `OrderRepository`, chỉ lưu **ID đơn**. Khi lấy ra phải đọc lại trạng thái và bỏ qua đơn không còn PENDING.

---

## 6. Hợp đồng chung (chữ ký cần dựng)

### 6.1 `common`
```cpp
// ErrorCode.h
enum class ErrorCode {
    Ok, NotImplemented, InvalidInput, NotFound, Duplicate, PermissionDenied,
    InvalidState, OutOfStock, ReferenceExists, AuthFailed, StorageError, ParseError
};

// Result.h  (movable; có chuyên biệt hóa Result<void>)
template <typename T> class Result {
public:
    static Result ok(T value);
    static Result fail(ErrorCode code, std::string message);
    bool isOk() const;
    const T& value() const;          // chỉ gọi khi isOk()
    ErrorCode code() const;
    const std::string& message() const;
};

// Money.h      using Money = long long;   // VND, không dùng double
// DateTime.h   struct DateTime { static DateTime now(); std::string toString() const;
//                                 static Result<DateTime> parse(const std::string&); };

// Actor.h
struct Actor { std::string userId; Role role; bool isAuthenticated() const; };

// Permission.h
enum class Action { ViewCatalog, ManagePhone, ManageCustomer, ManageEmployee, ManageDiscount,
    CreateOrder, ConfirmOrder, PrepareOrder, ShipOrder, CancelOrder, TakePayment,
    CreateCounterSale, ManageSupplier, CreateImportReceipt, CreateInvoice, ViewReport,
    BackupRestore, ImportExport, AssignRole };
struct Permission { static bool isAllowed(Role role, Action action); };  // bảng quyền nằm tại đây
```
Thay toàn bộ `errorMessage` ra tham số và các overload "nuốt lỗi" bằng `Result`. `ErrorMessage.h` cũ được thay bằng `ErrorCode.h`.

### 6.2 `storage`
```cpp
class FileManager {
public:
    explicit FileManager(const std::string& dataDir);
    Result<Vector<std::string>> readAllLines(const std::string& fileName) const;
    Result<void> writeAllLinesAtomic(const std::string& fileName,
                                     const Vector<std::string>& lines) const; // ghi .tmp rồi đổi tên
    bool exists(const std::string& fileName) const;
};
class BackupManager {                       // dùng FileManager
    Result<std::string> createBackup() const;                 // trả tên thư mục backup (theo thời gian)
    Result<Vector<std::string>> listBackups() const;
    Result<void> restore(const std::string& backupName) const; // kiểm tra hợp lệ trước khi ghi đè
};
class LineCodec {                           // phân cách '|', danh sách ';', thoát bằng '\'
    static std::string encode(const Vector<std::string>& fields);
    static Result<Vector<std::string>> decode(const std::string& line);
};
```
`ImportExport`: đọc/ghi dòng văn bản theo đường dẫn người dùng đưa vào, trả báo cáo lỗi từng dòng. Không biết thực thể.

### 6.3 `repository`
```cpp
class IRepositoryTx {                       // để UnitOfWork điều phối
public:
    virtual void snapshot() = 0;            // chụp RAM
    virtual Result<void> flush() = 0;       // ghi file
    virtual void restore() = 0;             // trả RAM về snapshot (và dựng lại index)
    virtual ~IRepositoryTx() = default;
};

template <typename T>
class Repository : public IRepositoryTx {
public:
    Repository(FileManager& fm, const std::string& fileName);
    Result<void> load();                    // nạp file, dòng lỗi ghi vào loadReport()
    Result<void> save();
    Vector<T> getAll() const;               // bản sao
    Result<T> findById(const std::string& id) const;   // bản sao
    Result<void> add(const T& item);        // sửa RAM, KHÔNG tự save khi nằm trong UnitOfWork
    Result<void> update(const T& item);
    Result<void> remove(const std::string& id);
    size_t count() const;
protected:
    virtual std::string getId(const T&) const = 0;
    virtual std::string serialize(const T&) const = 0;
    virtual Result<T> deserialize(const std::string& line) const = 0;
    virtual void rebuildIndexes() = 0;
};

class UnitOfWork {
public:
    void enlist(IRepositoryTx& repo);       // snapshot ngay khi enlist (mỗi repo một lần)
    Result<void> commit();                  // flush từng repo; lỗi thì restore tất cả và ghi lại từ snapshot
    void rollback();                        // restore, không ghi file
};
```
Ghi chú: ghi nhiều file là *best effort* (không thật sự nguyên tử giữa các file); việc phục hồi dựa trên snapshot trong RAM.

Các Repository cụ thể (khai báo + stub, **chưa có logic**), phần tra cứu chính:
- `PhoneRepository`: `findByName(keyword)`, `findByBrand(keyword)`, `findByPriceRange(min, max)` (BST theo giá), tra theo ID (HashTable).
- `CustomerRepository`: `findByUsername`, `findByPhone` (HashTable).
- `EmployeeRepository`: `findByUsername`; lưu `Employee*` nội bộ; `findById` trả `Result<std::unique_ptr<Employee>>`.
- `OrderRepository`: `findByCustomer(customerId)`, `findByStatus(status)`, `Result<Order> peekNextPending()`, `rebuildPendingQueue()`.
- `DiscountRepository`: `findByCode`. `InvoiceRepository`: `findByOrderId`. `SupplierRepository`, `ImportReceiptRepository`: CRUD cơ bản.
- `DataRepository`: bọc `BackupManager` và `ImportExport` cho `DataService`.

### 6.4 `service` (chữ ký cần có; thân hàm là stub)
Mọi hàm trả `Result<...>`. Các hàm sửa dữ liệu nhận `const Actor&`.

```cpp
class AuthService      { Result<Actor> login(user, pass);  /* gọi CustomerService/EmployeeService */ };
class PhoneService {
    Result<Vector<PhoneView>>      listForCustomer() const;                  // không có ID nội bộ
    Result<Vector<PhoneStaffView>> listForStaff(const Actor&) const;
    Result<Vector<PhoneView>> search(keyword), filterByPrice(min, max);
    Result<void> addPhone(const Actor&, const Phone&); updatePhone(...); removePhone(const Actor&, id);
    // thao tác tồn kho: chỉ sửa RAM và enlist vào uow; NGƯỜI GỌI commit
    Result<void> checkAvailable(id, qty) const;                 // available = quantity - reservedQuantity
    Result<void> reserve(UnitOfWork&, id, qty);
    Result<void> releaseReserved(UnitOfWork&, id, qty);
    Result<void> commitReserved(UnitOfWork&, id, qty);          // giữ chỗ -> xuất kho thật
    Result<void> restock(UnitOfWork&, id, qty);                 // hoàn kho khi hủy đơn đã xác nhận
    Result<void> receiveStock(UnitOfWork&, id, qty);            // nhập kho
};
class CustomerService  { registerCustomer(...); authenticate(user, pass) -> Result<Actor>;
                         getProfile(const Actor&); updateProfile(...); listAll/deleteCustomer(const Actor&, ...); };
class EmployeeService  { addEmployee/updateEmployee/removeEmployee/listAll(const Actor&, ...);
                         authenticate(user, pass) -> Result<Actor>; assignRole(const Actor&, id, Role); setActive(...); };
class DiscountService  { add/update/remove/list(const Actor&, ...);
                         Result<Money> validateAndCalculate(code, orderTotal) const;
                         Result<void> consumeUsage(UnitOfWork&, code); releaseUsage(UnitOfWork&, code); };
class OrderService {
    Result<std::string> createOrder(const Actor&, const OrderDraft&);   // kiểm tra available, reserve, lưu PENDING
    Result<void> confirmOrder(const Actor&, orderId);                   // PENDING -> CONFIRMED, commitReserved
    Result<void> startPreparing(const Actor&, orderId);                 // CONFIRMED -> PREPARING
    Result<void> markReadyForPickup(const Actor&, orderId);             // PREPARING -> READY_FOR_PICKUP
    Result<void> markShipping(const Actor&, orderId);                   // PREPARING -> SHIPPING
    Result<void> takePayment(const Actor&, orderId, PaymentMethod);     // UNPAID -> PAID
    Result<void> completeOrder(const Actor&, orderId);                  // yêu cầu PAID
    Result<void> cancelOrder(const Actor&, orderId);
    Result<std::string> createCounterSale(const Actor&, const OrderDraft&, PaymentMethod); // bán tại quầy
    Result<Vector<OrderSummary>> listForCustomer(const Actor&) const;
    Result<Vector<OrderSummary>> listByStatus(const Actor&, OrderStatus) const;
    Result<OrderSummary> getOrder(const Actor&, orderId) const;
};
class OrderStateMachine {                   // thuần logic, không phụ thuộc Repository
    static bool canTransition(DeliveryType, OrderStatus from, OrderStatus to, Role actor);
};
class CartService      { getCart(const Actor&); addItem; removeItem; updateQuantity; clear;
                         calculateTotal(const Actor&, optionalCode);
                         Result<std::string> checkout(const Actor&, const CheckoutInfo&); /* gọi OrderService::createOrder */ };
class InvoiceService   { createInvoiceFromOrder(const Actor&, orderId); getByOrderId; list; };
class SupplierService  { add/update/remove/list/search(const Actor&, ...); };
class ImportReceiptService { createReceipt(const Actor&, ...); getById; list; };   // gọi PhoneService::receiveStock
class ReportService    { revenueByPeriod(const Actor&, from, to); orderCountByStatus(...);
                         bestSellingProducts(...); stockSummary(...); };           // chỉ đọc
class DataService      { backup(const Actor&); listBackups(...); restore(const Actor&, name);
                         importPhones/importCustomers(const Actor&, path) -> Result<ImportReport>;
                         exportInvoice(const Actor&, invoiceId, path); };
```

### 6.5 Phụ thuộc giữa các Service (không được có vòng)
| Tầng | Service | Được gọi |
|---|---|---|
| 1 | Phone, Customer, Employee, Discount, Supplier, Report, Data | chỉ Repository |
| 2 | Auth | Customer, Employee |
| 2 | ImportReceipt | Supplier, Phone |
| 2 | Order | Phone, Customer, Discount |
| 3 | Cart | Order, Phone, Discount |
| 3 | Invoice | Order, Customer, Employee |

`OrderService` không biết `Cart`. `ReportService` đọc thẳng Repository, không gọi `OrderService`.

### 6.6 View, Controller và DTO
- Controller cài `IXxxHandler` (ví dụ `ICatalogHandler { onSearch(keyword); onSelect(index); ... }`). View nhận con trỏ handler và gọi khi có sự kiện. Controller giữ tham chiếu tới `IXxxView` và gọi `show...()` để cập nhật.
- `IView` có sẵn: `showMessage`, `showError`, `confirm`. Mỗi view con thêm các hàm hiển thị riêng (`showPhones`, `showOrders`...).
- **ID sản phẩm nội bộ**: `PhoneView` (dành cho khách) **không có trường ID**. Controller giữ bảng ánh xạ "vị trí hiển thị → ID" cho danh sách đang hiển thị. `PhoneStaffView` (nhân viên) mới có ID, `quantity`, `reservedQuantity`, giá nhập.
- `IViewFactory`: tạo các view; `ConsoleViewFactory` là bản cài đặt hiện tại.

### 6.7 `app`
`AppContext` khởi tạo theo thứ tự: `FileManager` → Repository (gọi `load()`) → Service → View (qua `IViewFactory`) → Controller. `Session` giữ `Actor` đang đăng nhập. `main.cpp`: tạo `AppContext`, hiển thị màn hình đăng nhập console (stub), thoát bình thường.

---

## 7. Quy tắc nghiệp vụ cần mã hóa vào khung
Đợt này chỉ khai báo enum, bảng hằng và ghi chú `// TODO`, **chưa cài logic**.

**Trạng thái đơn:** `PENDING, CONFIRMED, PREPARING, READY_FOR_PICKUP, SHIPPING, COMPLETED, CANCELLED`
**Thanh toán:** `PaymentStatus { UNPAID, PAID }`, `PaymentMethod { CASH, TRANSFER }`
**Nhận hàng:** `DeliveryType { PICKUP, DELIVERY }`
**Tồn kho của đơn:** `InventoryState { NONE, RESERVED, DEDUCTED, RELEASED, RESTOCKED }`, lưu trong `Order`; chỉ `OrderService` được đổi trường này.

**Bảng chuyển trạng thái** (đưa vào `OrderStateMachine`):

| Từ | Đến | Luồng | Vai trò được phép |
|---|---|---|---|
| PENDING | CONFIRMED | cả hai | Sales, Admin |
| PENDING | CANCELLED | cả hai | Customer (đơn của mình), Sales, Admin |
| CONFIRMED | PREPARING | cả hai | Warehouse, Admin |
| CONFIRMED | CANCELLED | cả hai | Sales, Admin |
| PREPARING | READY_FOR_PICKUP | PICKUP | Warehouse, Admin |
| PREPARING | SHIPPING | DELIVERY | Warehouse, Admin |
| READY_FOR_PICKUP / SHIPPING | COMPLETED | theo luồng | Sales, Admin; **yêu cầu PAID** |

Từ PREPARING trở đi **không** cho hủy (V1).

**Tồn kho:** `availableQuantity = quantity - reservedQuantity`.
- Tạo đơn: kiểm tra đủ `available` rồi giữ chỗ (`reservedQuantity += n`), `inventoryState = RESERVED`.
- Xác nhận: `reservedQuantity -= n` và `quantity -= n`, `inventoryState = DEDUCTED`.
- Hủy PENDING: giải phóng giữ chỗ, `RELEASED`. Hủy CONFIRMED: `quantity += n`, `RESTOCKED`.
- Mỗi lượng hàng chỉ giải phóng/hoàn **đúng một lần** (dựa vào `inventoryState`).

**Khác:** doanh thu = tổng giá trị đơn `COMPLETED` và `PAID`. Thanh toán online chưa làm; nhân viên bán hàng đánh dấu `PAID` khi khách nhận hàng. Bán tại quầy: tạo đơn, thu tiền và lập hóa đơn trong một `UnitOfWork`.

---

## 8. Định dạng file `.txt`
- Một thực thể một dòng; dòng đầu của mỗi file là `#v1`.
- Trường cách nhau bằng `|`; danh sách con cách nhau bằng `;`; ký tự thoát `\` (`\|`, `\;`, `\\`, `\n`).
- Chi tiết đơn/phiếu nhập lưu nhúng trong dòng của đơn/phiếu (qua `LineCodec`).
- Tên file: `phones.txt, customers.txt, employees.txt, orders.txt, discounts.txt, invoices.txt, suppliers.txt, import_receipts.txt`. Backup nằm trong `data/backup/<thời gian>/`.
- Dòng lỗi khi nạp: bỏ qua và ghi vào `LoadReport`, không làm chương trình dừng.
- Nếu code cũ đã có định dạng file khác thì **giữ nguyên định dạng cũ**, chỉ ghi lại trong `docs/MIGRATION.md`.

---

## 9. Quy ước code
- C++17 (hoặc chuẩn mà dự án đang dùng). `#pragma once` trong header.
- Thư viện chuẩn được phép: `<string>`, `<iostream>`, `<fstream>`, `<sstream>`, `<cstdio>`, `<cstdlib>`, `<cctype>`, `<ctime>`, `<memory>` (chỉ `unique_ptr`), `<utility>`, `<filesystem>` (chỉ trong `storage/file`).
- **Không dùng** `std::vector, map, unordered_map, set, list, queue, stack, array` cho dữ liệu của ứng dụng; dùng `Vector, HashTable, BST, Queue` tự cài. Nếu code cũ đã có sẵn bản tự cài, giữ và chuyển vào `storage/ds`.
- Tên: class `PascalCase`, hàm và biến `camelCase`, hằng và enum value `UPPER_SNAKE`. Comment tiếng Việt cho mục đích class/hàm.
- Mỗi stub trả `Result::fail(ErrorCode::NotImplemented, "TODO: <tên hàm>")`, và có comment `// TODO(logic): <việc cần làm>`.

---

## 10. Di chuyển code hiện có (bắt buộc)
1. **Kiểm kê trước:** liệt kê mọi file `.h/.cpp` đang có và vị trí hiện tại. Đối chiếu với tên trong tài liệu này; lập bảng ánh xạ `file cũ → vị trí mới` trong `docs/MIGRATION.md`.
2. **Di chuyển, không viết lại.** Giữ nguyên tên class/hàm, logic và comment gốc. Chỉ đổi `#include` và namespace nếu cần.
3. Nếu bắt buộc phải sửa bên trong code cũ để biên dịch hoặc để đúng luật tầng: giữ nguyên phần còn lại của file, **xuất đầy đủ file đã sửa** và đánh dấu mỗi chỗ sửa bằng comment `// FIX LỖI: <lý do ngắn>`.
4. Code cũ vi phạm luật tầng mà chưa sửa ngay được (ví dụ Service giữ bản sao dữ liệu): để nguyên, ghi vào `docs/MIGRATION.md` mục "Vi phạm còn tồn đọng" kèm tên file và dòng. Không tự viết lại logic.
5. Code cũ không có chỗ đặt trong cây này: để ở `src/_unplaced/` và ghi vào `docs/MIGRATION.md`.

---

## 11. Thứ tự thực hiện
1. Kiểm kê code cũ, lập bảng ánh xạ (mục 10).
2. Tạo cây thư mục và `CMakeLists.txt` (một target chính + một target `smoke_test`, tùy chọn `BUILD_GTK=OFF`).
3. `common` → `storage/ds`, `storage/algo`, `storage/file` → `model` (enum trước) → `dto`.
4. `repository` (lớp cơ sở, `UnitOfWork`, các repository cụ thể).
5. `service` (kể cả `OrderStateMachine`, bảng quyền trong `Permission`).
6. `view/interfaces`, `controller`, `view/console`.
7. `app` (`AppContext`, `Session`, `main.cpp`).
8. `tools/check_layers.sh`, `tests/smoke_test.cpp`.
9. Biên dịch sạch, chạy, rồi điền `docs/MIGRATION.md` và `docs/OPEN_QUESTIONS.md`.

---

## 12. Tiêu chí nghiệm thu
- [ ] Cây thư mục đúng mục 4; mỗi class có cặp `.h/.cpp` (template thì header-only).
- [ ] Biên dịch sạch với `-Wall -Wextra`, không lỗi.
- [ ] `./phonestore` chạy, hiển thị màn hình đăng nhập console (stub), thoát bình thường.
- [ ] `smoke_test` tạo `AppContext` với thư mục dữ liệu tạm và thoát thành công.
- [ ] `tools/check_layers.sh` kiểm tra bằng `grep` đúng ma trận mục 3, trả mã thoát khác 0 nếu vi phạm; hiện tại chạy ra mã 0 (hoặc chỉ báo các vi phạm đã ghi trong `docs/MIGRATION.md`).
- [ ] Không còn `std::vector|map|unordered_map|set|list|queue|stack` trong `src/` (trừ chỗ được ghi nhận trong `docs/MIGRATION.md`).
- [ ] Không logic nghiệp vụ mới: mọi hàm nghiệp vụ chưa có code cũ là stub `NotImplemented`.
- [ ] `PhoneView` không có trường ID; `Phone` có `reservedQuantity` và `availableQuantity()`.
- [ ] `docs/MIGRATION.md` có bảng ánh xạ file cũ → mới, danh sách vi phạm còn tồn đọng, và mọi chỗ đã sửa trong code cũ (đánh dấu `// FIX LỖI`).
- [ ] Mọi chỗ không chắc đã ghi vào `docs/OPEN_QUESTIONS.md`, không tự quyết.

---

## 13. Điểm còn mở (ghi nhận, không ảnh hưởng bộ khung)
Chủ dự án chưa chốt các điểm sau; dùng mặc định tạm nếu cần, và **không** mã hóa cứng:
1. Mã giảm giá tính lượt dùng khi tạo đơn hay khi xác nhận đơn (mặc định tạm: khi tạo đơn, hoàn lượt khi hủy).
2. Bán tại quầy có giữ chỗ không (mặc định tạm: trừ kho thẳng, không giữ chỗ).
3. Nhân viên hủy đơn từ PREPARING trở đi (mặc định tạm: không cho phép trong V1).
4. Nhân viên kho có xem được tất cả đơn hay chỉ đơn từ CONFIRMED trở đi.

---

## 14. Công việc sau bộ khung (không thuộc đợt này)
Sửa lỗi P0/P1 theo bản audit Service (xử lý đơn FIFO, hủy đơn, tồn kho, lưu file có rollback), cài logic từng service theo thứ tự ưu tiên, giao diện console đầy đủ theo vai trò, rồi GTK4 nếu còn thời gian.
