# Data dictionary - Phone Store PBL2

Tai lieu nay mo ta cac file mau trong `data/`, schema ma code hien tai dang doc,
cac quan he giua du lieu va nhung gioi han can biet khi nhap them du lieu.

> **Day la du lieu DEMO tong hop.** Ten, so dien thoai, ma so thue, dia chi va
> giao dich chi phuc vu hoc tap/kiem thu; khong dung lam du lieu khach hang hay
> giao dich that. Tai khoan va mat khau duoi day la credential demo plaintext,
> khong dung lai o bat ky he thong nao khac.

## 1. Quy tac chung khi nhap file

- Moi file repository doc co mot dong header o dong dau. Repository hien tai
  bo qua dong dau tien theo vi tri, khong tu xac nhan ten cot; khong bo header.
- Text chinh la UTF-8. Moi dong la mot ban ghi; khong chen dong mo ta vao file
  du lieu co repository doc.
- Cac cot cap mot tach bang `|`; cac detail tach bang `;`; cac truong detail
  tach bang `,`. Parser hien tai **khong ho tro escape** cac dau phan cach.
  Khong dat `|` trong bat ky cot nao, `;` trong detail list, hoac `,` trong
  phone name/detail text.
- Gia tien luu la so nguyen VND, khong co dau cham hang nghin, ky hieu `d`/`đ`,
  dau phay thap phan hay scientific notation. Vi du: `14000000`.
- So luong, enum code va diem la so nguyen. Quantity trong san pham khong am;
  quantity trong order/receipt detail phai lon hon 0.
- Ngay gio su dung `DD/MM/YYYY HH:MM:SS`; ngay voucher su dung `DD/MM/YYYY`.
  Ngay phai hop le theo lich. Parser DateTime hien tai chua validate day/month
  nghiem ngat, nen nguoi nhap phai tu kiem tra du lieu.
- ID phai duy nhat trong tung loai thuc the. Khong dung lai ID da ton tai.
- ID dung tien to viet hoa va so thu tu 6 chu so: `ACC`, `CUS`, `EMP`, `PHN`,
  `SUP`, `WAR` dung dang `PREFIX-000001`; `ORD`, `INV`, `IMP`, `WREQ`, `DUS`
  dung dang `PREFIX-ddmmyyyy-000001`. Counter duoc luu tai
  `system/id_counters.txt`, khong giam counter khi xoa ban ghi.
- Khoa ngoai phai tham chieu ID day du theo cung dinh dang. ID DiscountCode la
  `code` nghiep vu; khong dung ID so rieng de tra cuu ma giam gia.
- Repository co the bo qua dong parse loi, nhung khong tao bao cao loi co cau
  truc va co the khong phat hien FK sai. Phai chay checklist quan he o muc 11.
- Khong ghi credential that. Account hien duoc luu plaintext; day la gioi han
  cua project, khong phai cach luu mat khau cho san pham that.

## 2. Danh muc file va cach code su dung

| File | Entity | Repository/reader hien tai | Ghi chu |
|---|---|---|---|
| `Device/Products.txt` | Phone | `PhoneRepository` + `Phone::fromFileLine` | Du lieu chinh cua san pham. |
| `Device/Thongsokythuat.txt` | Phone detail | `PhoneRepository::loadDetailedSpecifications` | Noi dung nhieu dong, ghep vao Phone theo ID. Khong co header cot. |
| `people/Customers.txt` | Customer + Account | `CustomerRepository` | Account duoc nhung trong customer va co `accountId` rieng. |
| `people/Employee.txt` | Employee + Account | `EmployeeRepository` | Account duoc nhung trong employee va co `accountId` rieng; role code 1..3. |
| `people/Suppliers.txt` | Supplier | `SupplierRepository` | Ten nguoi lien he la mot chuoi day du. |
| `PAY/Discount.txt` | DiscountCode | `DiscountRepository` | Can dung 9 cot; type 0/1. |
| `PAY/Order/Orders.txt` | Order + OrderDetail | `OrderRepository` | Detail duoc luu nhung trong dong order. |
| `PAY/Invoices.txt` | Invoice + OrderDetail | `InvoiceRepository` | Mot invoice mau cho moi order da hoan tat. |
| `Device/StockReceipts.txt` | ImportReceipt + OrderDetail | `StockReceiptRepository` | Detail dung `OrderDetail` de tai su dung truong du lieu. |
| `Device/ProductImage.txt` | Anh san pham (metadata) | `ProductImageRepository` | Quản lý đường dẫn ảnh chính & phụ của từng sản phẩm. |
| `Device/ProductMarketing.txt` | Marketing & Khuyến mãi | `ProductMarketingRepository` | Giá gốc niêm yết, thời hạn sale, trả góp, màu sắc, bảo hành. |
| `Device/ProductDescriptions.txt` | Mô tả chi tiết sản phẩm | `ProductDescriptionRepository` | Điểm nổi bật và đoạn văn mô tả chi tiết của từng máy. |
| `system/StoreSettings.txt` | Cài đặt & Chính sách chung | `StoreSettingsRepository` | Chính sách vận chuyển, đổi trả, bảo hành toàn cửa hàng. |

Gio hang la trang thai phien lam viec trong RAM, khong co file persistence.
Bao cao doanh thu/ton kho duoc tinh tu entity, khong co file bao cao dau vao.
Repository khong tu dong nap khi constructor chay; entry point phai goi
`loadFromFile()` cho tung repository muon su dung.

## 3. San pham - `Device/Products.txt`

Schema:

```text
id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications
```

- `id`: khoa san pham, duy nhat; vi du `101`.
- `phoneName`: ten catalog; tranh dau phay vi ten cung duoc lap lai trong detail
  order/receipt phan cach bang dau phay.
- `brand`, `operatingSystem`, `manufacturer`: text khong rong.
- `importPrice`, `sellingPrice`: VND nguyen; gia nhap >= 0, gia ban > 0.
- `quantity`: ton kho hien tai (so dien thoai vat ly), >= 0.
- `specifications`: tom tat de hien thi; co the chua dau phay vi cot cap mot tach
  bang `|`.

Code chap nhan format cu 7 cot va format 9 cot. Chuan moi cho data demo la 9 cot.
`Phone::toFileString()` giu tom tat trong Products.txt; thong so chi tiet doc tu
file rieng. `Phone` hien chua co truong bao hanh, serial/IMEI, active/discontinued,
thue suat hoac vi tri ke; khong tu y them cot vao file nay vi serializer hien tai
se lam mat cac cot khong biet khi luu lai.

## 4. Thong so chi tiet - `Device/Thongsokythuat.txt`

Format ma `PhoneRepository::loadDetailedSpecifications()` dang doc:

```text
<productId>|
<dong mo ta 1>
<dong mo ta 2>

<productId-khac>|
<mo ta>
```

- Moi block bat dau bang ID ton tai trong `Products.txt`, tiep theo la `|`.
- Cac dong tiep theo thuoc block do cho den marker ID tiep theo.
- Khong chen header, khong dung marker ID trung, va dung ID san pham duy nhat.
- Noi dung hien duoc giu trong `Phone::detailedSpecifications`; hien khong duoc
  ghi nguoc vao Products.txt boi serializer.
- Data demo co tom tat catalog cho moi phone. Block cua phone 101 co mo ta chi
  tiet legacy; cac truong chua co nguon xac nhan khong nen duoc dien bang thong
  so phong doan.

## 5. Khach hang - `people/Customers.txt`

Schema:

```text
id|firstName|middleName|lastName|phoneNumber|Email|username|password|points|address|accountId
```

`id`, `username` va `phoneNumber` can duy nhat; `points` la so nguyen khong am.
So dien thoai demo theo quy tac service hien co: 10-11 chu so, bat dau bang 0.
Email va dia chi la thong tin demo. Mat khau la plaintext chi de test dang nhap.

Khong tach Accounts.txt: model Customer chua Account va serializer/parser doc
username/password/accountId trong cung dong. `accountId` dung tien to `ACC`.

## 6. Nhan vien - `people/Employee.txt`

Schema:

```text
id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address|salary|accountId
```

- Role code parser hien tai: `1=ADMIN`, `2=SALE_EMPLOYEE`,
  `3=WAREHOUSE_EMPLOYEE`.
- Salary la VND nguyen, khong am.
- Data mau gom mot Admin, mot Sales Employee va mot Warehouse Employee.
- Serializer va parser dung cung role code `1..3`.
- `accountId` dung tien to `ACC` va duoc luu trong cung ban ghi employee.
- `active` cua Account chua nam trong schema file, nen trang thai khoa tai khoan
  khong duoc persistence.

## 7. Nha cung cap - `people/Suppliers.txt`

Schema:

```text
id|companyName|taxCode|contactPerson|phoneNumber|email|address
```

Contact person la mot chuoi day du; taxCode va contact data trong bo mau la gia
lap. Moi supplier duoc dung boi receipt phai ton tai truoc trong file nay.

## 8. Ma giam gia - `PAY/Discount.txt`

Schema:

```text
code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount
```

- `type`: `0=PERCENTAGE`, `1=FIXED_AMOUNT`.
- Percentage `value` la phan tram 0..100; `maxDiscount` la tran giam VND.
- Fixed amount `value` la VND; `maxDiscount` trong data demo bang value.
- `minOrder`, `maxDiscount` la VND; `maxUsage > 0`, `0 <= usedCount <= maxUsage`.
- `endDate` khong nho hon `startDate`.
- Code mau co hieu luc nam 2026. Ma voucher khong duoc tham chieu truc tiep boi
  Order model hien tai; chi co `discountAmount`, nen khong the doi soat
  `usedCount` voi tung order.

## 9. Don hang - `PAY/Order/Orders.txt`

Schema:

```text
id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details
```

- `deliveryMethod`: `0=PICKUP`, `1=DELIVERY`.
- `paymentMethod`: `0=CASH`, `1=BANK_TRANSFER`, `2=CREDIT_CARD`.
- `status`: `0=PENDING`, `1=CONFIRMED`, `2=PREPARING`,
  `3=READY_FOR_PICKUP`, `4=SHIPPING`, `5=COMPLETED`, `6=CANCELLED`.
- `details`: danh sach `phoneId,phoneName,quantity,unitPrice`, ngan cach bang
  `;`. Unit price la gia ban snapshot tai thoi diem dat hang.
- Moi customer/phone ID phai co trong file entity tuong ung.
- Chi don da `COMPLETED` moi co invoice trong bo data demo. Don PENDING khong
  lam giam so luong ton kho trong phep doi soat demo.
- Order model hien khong co payment status, voucher code, employee processor
  hay audit history; khong them cac cot nay neu chua cap nhat model/parser/
  serializer cung luc.

## 10. Hoa don - `PAY/Invoices.txt`

Schema:

```text
id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details
```

Moi `orderId` chi co mot invoice; order, customer, employee phai ton tai. Bo
data demo tao invoice cho don da hoan tat, paymentMethod khop order, details va
discount khop order. Hien tai Invoice khong luu payment status/transaction ID,
thue, dia chi cua cua hang hay chu ky; pham vi do an khong gia lap cac thong tin
phap ly cua hoa don dien tu.

## 11. Phieu nhap kho - `Device/StockReceipts.txt`

Schema:

```text
id|supplierId|employeeId|importTime|details
```

`details` dung cung format detail 4 truong nhu order, nhung truong `unitPrice`
la gia nhap. Supplier, employee va phone phai ton tai. Bo data demo tinh ton
kho hien tai tu tong nhap tru don COMPLETED:

| Phone ID | Nhap | Da ban trong don hoan tat | Ton hien tai |
|---|---:|---:|---:|
| 101 | 45 | 3 | 42 |
| 102 | 20 | 2 | 18 |
| 103 | 25 | 0 | 25 |
| 104 | 15 | 0 | 15 |
| 105 | 35 | 5 | 30 |
| 106 | 50 | 0 | 50 |
| 107 | 15 | 3 | 12 |

Day la ledger demo don gian, khong co opening balance, adjustment, return,
damaged stock hay nhieu kho. Neu them cac nghiep vu do, can mo rong entity va
quy tac doi soat; khong tu sua quantity mau de "can" ma khong co giao dich giai
thich.

## 12. Anh san pham - `Device/ProductImage.txt`

Schema catalog de danh dau anh:

```text
productId|imagePath|isPrimary|displayOrder
```

`imagePath` la duong dan tuong doi voi thu muc `data/`, vi du
`Device/productImages/oppo_reno_16_f_5g_bac.png`; khong ghi duong dan tuyet doi
tren may ca nhan. `isPrimary` la 0/1, `displayOrder` bat dau tu 1.

Reader cua ProductImage.txt chua duoc implement trong source/UI. Bo data chi
tro den asset Oppo dang co; khong tao placeholder gia cho cac san pham khac.
Khi them anh, chi them duong dan sau khi asset ton tai va duoc phep su dung.

## 13. Du lieu khong tao thanh file rieng

- `Cart`/`CartItem`: tam thoi trong phien, chua co repository persistence.
- `Account`: nhung trong Customer/Employee.
- Reports: tinh tu Order/Invoice/Phone.
- Khong co file payment transaction, stock adjustment, return, warranty,
  promotion-to-order link, login audit hay backup snapshot trong bo data hien tai.

Khong can tao file rong cho cac entity tren neu chua co code doc/ghi; file khong
duoc su dung de tao cam giac tinh nang da duoc implement.

## 14. Enum codes

| Enum | Code file | Gia tri |
|---|---:|---|
| Employee role | 1 | ADMIN |
| Employee role | 2 | SALE_EMPLOYEE |
| Employee role | 3 | WAREHOUSE_EMPLOYEE |
| DeliveryMethod | 0 | PICKUP |
| DeliveryMethod | 1 | DELIVERY |
| PaymentMethod | 0 | CASH |
| PaymentMethod | 1 | BANK_TRANSFER |
| PaymentMethod | 2 | CREDIT_CARD |
| OrderStatus | 0 | PENDING |
| OrderStatus | 1 | CONFIRMED |
| OrderStatus | 2 | PREPARING |
| OrderStatus | 3 | READY_FOR_PICKUP |
| OrderStatus | 4 | SHIPPING |
| OrderStatus | 5 | COMPLETED |
| OrderStatus | 6 | CANCELLED |
| DiscountType | 0 | PERCENTAGE |
| DiscountType | 1 | FIXED_AMOUNT |

## 15. Checklist truoc khi commit data

1. Tat ca repository input co header dung chinh xac va dung so cot.
2. Moi primary ID duy nhat; usernames, email va phone khong bi dung lai khi
   nghiep vu yeu cau duy nhat.
3. Moi foreign key ton tai: customer/order, employee/invoice, supplier/receipt,
   phone trong order/invoice/receipt/spec/image.
4. Detail quantity > 0; gia khong am; enum nam trong mien; discount khong vuot
   subtotal; total tinh dung.
5. Moi invoice khop mot order; invoice detail, customer, method, discount va
   thoi gian hop ly.
6. Ton phone bang tong receipt + opening stock + adjustment - completed sales
   - write-off + returns. Neu chua co cac loai giao dich nay thi ghi ro cong
   thuc don gian dang ap dung.
7. Moi image path tro den file that, tuong doi voi data root. Nho rang code hien
   tai chua load ProductImage.txt.
8. Tao repository moi, goi loadFromFile(), kiem tra so ban ghi va round-trip
   serialize/parse tren ban sao thu muc data. Khong chay test ghi tren bo data
   goc.
9. Bao cao ro cac loi parser/round-trip con nam trong source; khong coi data
   sach la bang chung code da persistence dung.
