KE HOACH KHAC PHUC VA HOAN THIEN PBL2
He thong quan ly cua hang dien thoai - tai lieu giao viec cho AI coding agent
Cap nhat: 2026-10-15

===============================================================================
0. MUC DICH VA PHAM VI
===============================================================================

Tai lieu nay chuyen ket qua ra soat thanh cac dau viec co the thuc hien trong
repository hien tai. Muc tieu la lam cho luong du lieu, nghiep vu va demo co the
tin cay o quy mo do an PBL2 sinh vien nam hai, khong bien du an thanh he thong
thuong mai lon.

Phien ban code trong workspace co the thay doi. Truoc moi sua doi, AI thuc hien
phai doc code dang co, git status, cac file lien quan va doi chieu dong code voi
cac nhan dinh ben duoi. Neu loi da duoc sua, khong tao ban sua trung lap; hay
ghi nhan da khac phuc va kiem chung. Cac duong dan trong tai lieu la dau moi
tham chieu, khong thay cho viec kiem tra code thuc te.

Pham vi chinh: source/, data/, cac UI va entry point dang duoc build, Makefile
va test lien quan. Khong tu y di chuyen/ghi de PhoneStore/ hoac kien truc moi
neu viec do khong can thiet. Khong xoa thay doi dang co cua nguoi dung.

Muc tieu ky thuat:
- Duy tri C++ va phong cach OOP phu hop sinh vien nam hai.
- Tiep tuc dung cac cau truc du lieu tu cai dat cua do an khi phu hop.
- Giu format file text hien tai de tranh pha huy du lieu.
- UI chi goi Service, khong truy cap Repository/file truc tiep.
- Loi phai duoc bao ro; khong tra ve thanh cong gia, khong bo qua ket qua ghi.
- Sua gon, co test cho tung loi va cap nhat CHANGELOG.md.

===============================================================================
1. QUY TRINH BAT BUOC TRUOC KHI SUA
===============================================================================

1. Kiem tra git status, Makefile, main.cpp, cac UI dang duoc build va header
   chinh. Workspace co the co thay doi chua commit; khong revert hoac ghi de
   chung. Neu thay doi cua nguoi dung xung dot truc tiep voi dau viec, dung va
   hoi truoc khi ghi de.
2. Doc cac file model, repository, service va data tuong ung. Dac biet xac minh
   format serialize/deserialize, quy uoc enum, thu tu cot va cach repository
   xu ly ID trung.
3. Xac dinh lenh build/test san co. Chay baseline nho truoc khi sua neu co the.
   Khong xem file .o/.exe cu la bang chung rang ma nguon hien tai build duoc.
4. Thuc hien tung nhom sua theo thu tu muc 9. Them test regression cho bug.
5. Sau moi nhom: build, chay test phu hop, xem git diff; khong sua ngoai pham vi.
6. Bao cao cuoi: cac dau viec da lam, file da doi, test da chay va viec con lai.

===============================================================================
2. DAU VIEC A - THONG NHAT ROLE NHAN VIEN KHI LUU/TAI
===============================================================================

Hien trang can xac minh:
- Employee::toFileString() trong source/models/Employee.h da tung ghi enum
  truc tiep (gia tri 0-based).
- Employee::fromFileLine() va data/DATA_FORMAT_GUIDE.md tung dung quy uoc
  role 1-based. Neu con lech, Admin co the bi bo qua va Sales/Warehouse bi
  doc thanh role khac sau khi restart.

Solution:
1. Chon mot quy uoc duy nhat dua tren format du lieu cu va giu tuong thich.
   Khuyen nghi duy tri quy uoc file da duoc DATA_FORMAT_GUIDE.md cong bo
   (neu guide la 1=Admin, 2=Sales, 3=Warehouse), va viet hai ham chuyen doi
   rieng, ro rang:
     RoleType -> ma file
     ma file -> RoleType hop le
   Khong cast enum truc tiep neu quy uoc file khac gia tri enum.
2. Parser phai tu choi ma role ngoai mien hop le; khong tu dong gan role mac
   dinh va khong lam mat dong du lieu ma khong thong bao. Theo pattern repo hien
   tai, bao dong sai kem so dong/ID va tiep tuc nap dong khac.
3. Kiem tra status active, username va cac truong can thiet co duoc round-trip
   day du hay khong; khong tu thay doi schema neu khong can.
4. Cap nhat DATA_FORMAT_GUIDE.md neu format chinh thuc duoc sua.

Test chap nhan:
- Tao tung Employee/Admin, Sales, Warehouse voi username duy nhat.
- Serialize -> parse -> so sanh role, ID, username, salary va active neu schema
  co truong nay.
- Test ca data legacy hop le va role code khong hop le.
- Repository load-save-load van giu nguyen role; dong loi khong lam crash.

===============================================================================
3. DAU VIEC B - LAM DUNG DU LIEU MAU VA QUAN HE THAM CHIEU
===============================================================================

Hien trang can xac minh:
- data/PAY/Order/Orders.txt da tung co nhieu ban ghi ORD-001.
- Mot so order detail co gia/so luong khong ro rang hoac khong hop voi san
  pham, va data/PAY/Invoices.txt co invoice tham chieu order khong ton tai.
- data/PAY/Discount.txt da tung chi co mot dong mo ta, khong co voucher parser
  doc duoc.
- Data folder co the co cac file moi chua commit va format co the thay doi.

Solution:
1. Tao bang doi chieu ID (phone, customer, employee, supplier, order, invoice,
   receipt, discount) va kiem tra:
   - ID duy nhat trong tung loai thuc the.
   - Moi FK/tham chieu den thuc the ton tai.
   - So luong > 0; gia khong am; tong tien = tong chi tiet tru discount.
   - Ngay ket thuc voucher khong truoc ngay bat dau.
2. Sua data mau theo entity va serializer hien tai, khong sua code de "hop thuc
   hoa" mot dong data sai. Truoc khi thay doi data co gia tri, luu lai noi dung
   cu trong diff de reviewer thay ro; khong xoa toan bo du lieu mau.
3. Duy tri ID cu neu co the. Neu phai doi ID, cap nhat tat ca tham chieu lien
   quan (invoice, order, receipt) trong cung thay doi.
4. Chuan hoa Discount.txt theo dung so truong va enum encoding ma
   DiscountCode::fromFileLine() su dung. Them it nhat 2 voucher mau: percentage
   co gioi han giam va fixed amount; ngay hieu luc hop ly.
5. Neu file su dung dong header, xac minh repository co nhan biet header. Khong
   bo dong dau mot cach vo dieu kien neu file co the khong co header. Bao loi
   neu file rong/sai format; tranh tao file mau vo tinh xoa du lieu.
6. Dung script/test doc file theo parser that de kiem tra du lieu; khong chi
   kiem tra bang mat. Kiem tra du lieu co the ghi lai sau mot lan serialize.

Test chap nhan:
- Moi ID duy nhat; tat ca tham chieu tim thay duoc.
- Tat ca dong du lieu co the parse va so luong ban ghi sau load dung nhu ky vong.
- Invoice/order total va cac detail hop le.
- Voucher mau nap duoc va kiem tra duoc ca truong hop hieu luc/het han/het luot/
  khong dat don toi thieu.
- Thu muc data van giu cac file nguon ngoai pham vi (vi du anh, thong so ky
  thuat); khong them duong dan phu thuoc may ca nhan.

===============================================================================
4. DAU VIEC C - GIAO DICH DON HANG, TON KHO VA HUY DON
===============================================================================

Hien trang can xac minh:
- source/services/OrderService.h tung co luong processNextPendingOrder lay don
  khoi queue, cap nhat stock va luu ma bo qua mot so ket qua.
- PhoneService::updateStock tung bo qua ket qua repository update.
- cancelOrder tung cho huy qua nhieu trang thai va bo qua loi cap nhat kho/lon.
- Chua thay reserved stock ro rang trong model Phone cu. Khong tu dong them
  reservation phuc tap neu quy mo PBL2 co the giai quyet bang validate lai.

Solution toi thieu, de giai thich duoc:
1. Tao order:
   - Validate order co ID duy nhat, detail khong rong, quantity > 0, product ton
     tai, gia snapshot hop le.
   - Kiem tra ton kho du cho tung detail truoc khi ghi order.
   - De tranh hai don PENDING cung chiem cung mot hang, chon mot trong hai
     phuong an va ghi ro trong docs:
       A. Don PENDING giu cho stock (reservedQuantity), sau do confirm chuyen
          reserved -> sold.
       B. Neu khong them reservation vao pham vi, chi tru kho khi confirm va
          bat buoc recheck stock tai confirm; don thieu hang khong duoc confirm.
     Uu tien A neu model/service da co ho tro; neu chon B, ghi ro UI cho biet
     don chua duoc dam bao hang den luc confirm.
2. Confirm/process:
   - Xac dinh don con PENDING va hop le truoc khi thay doi queue/trang thai.
   - Kiem tra lai tat ca products/stock; khong bo qua bool/result cua update.
   - Neu bat ky detail nao loi, don van PENDING, stock khong doi va loi duoc
     hien cho caller.
   - Chi danh dau CONFIRMED sau khi cac thay doi can thiet thanh cong.
3. Cancel:
   - Dinh nghia bang chuyen trang thai ro rang: khong huy don CANCELLED hoac
     COMPLETED; xac dinh cac trang thai duoc huy theo busRule.txt.
   - Huy PENDING phai giai phong reservation neu co.
   - Huy don da tru kho chi hoan kho dung mot lan; ngan hoan kho lap lai.
   - Neu thao tac kho/repository that bai, khong bao thanh cong va rollback hoac
     bao loi ro rang; khong de trang thai va stock khac nhau.
4. Loi luu file: kiem tra tat ca ket qua save. Toi thieu phai rollback state
   trong RAM khi save that bai. Neu nhieu file thay doi, dung helper snapshot /
   restore don gian neu co san; neu chua co transaction framework, khong dung
   UnitOfWork phuc tap chi de giai quyet mot bug. Nhan manh gioi han atomic cua
   nhieu file trong tai lieu.
5. Stock API:
   - updateStock/reduceStock phai tu choi quantity <= 0, so luong khong du,
     so luong am va overflow neu co the.
   - Neu repository update tra false, service phai tra false/error tuong ung.
   - Khong tra con tro den Phone bi vo hieu sau khi repository sua/xoa; neu API
     cu dang dung pointer, giu tuong thich UI truoc roi moi thay doi co kiem soat.

Test chap nhan:
- 2 don PENDING canh tranh so luong ton kho; khong the confirm ca hai neu tong
  vuot ton.
- Don nhieu detail: detail cuoi thieu hang thi khong detail nao bi tru kho.
- Loi save mock/thu muc read-only: khong bao thanh cong, state RAM/stock/order
  rollback hoac duoc bao loi ro rang theo chinh sach da chot.
- Huy PENDING, CONFIRMED (neu busRule cho phep), COMPLETED va CANCELLED.
- Huy lap lai khong cong kho hai lan.
- Queue FIFO khong lam mat don neu confirm that bai; don huy khong duoc xu ly.

===============================================================================
5. DAU VIEC D - REPOSITORY PHAI NHAT QUAN GIUA RAM VA FILE
===============================================================================

Solution:
1. Kiem tra CRUD cua tung repository, uu tien OrderRepository, CustomerRepository,
   PhoneRepository, InvoiceRepository, StockReceiptRepository va SupplierRepository.
2. Moi thao tac add/update/remove phai:
   - Validate input/ID/duplicate theo pattern hien co.
   - Chuan bi thay doi; neu save that bai, undo thay doi in-memory va index.
   - Chi tra thanh cong neu ca thay doi RAM va persistence thanh cong.
3. Chon cach don gian:
   - Snapshot Vector truoc khi mutate va restore neu save fail; hoac
   - Validate va serialize danh sach moi vao buffer, ghi file thanh cong roi
     publish state moi.
   Khong tao generic Repository framework moi neu cac repo la header-only nho.
4. Bao dam duplicate ID khong silently overwrite. Khi load file cu co duplicate,
   bao dong/ID bi bo qua; chon chinh sach ro rang (giu dong dau tien) va test.
5. Kiem tra file tam atomic neu co san; khong xoa file goc truoc khi ghi moi
   thanh cong. Neu Windows rename thay file co han che, xac minh hanh vi tren
   platform thuc te.
6. Error output phai di qua return/status/stream theo convention du an; khong
   bat exception rong roi bo qua.

Test chap nhan:
- Add/update/remove thanh cong va doc lai sau repository moi.
- Save fail khong thay doi getAll(), index, count, pending queue.
- Duplicate ID khong tao ban ghi duplicate.
- File hong: dong hong co bao loi va dong hop le khac van duoc nap neu pattern
  hien tai yeu cau tolerant loading.

===============================================================================
6. DAU VIEC E - BST VA SAN PHAM TRUNG GIA
===============================================================================

Hien trang: PhoneService co the dung BST voi sellingPrice lam key; BST.h dang
update value khi key trung. Ket qua la san pham cung gia bi mat khoi index.

Solution, chon mot phuong an nhe:
A. Sua Phone price index de key la composite (sellingPrice, phoneId), voi comparator
   ho tro thu tu tu nhien, de moi phone co key duy nhat.
B. Neu comparator composite khong phu hop code hien tai, dung BST theo gia ma
   value la danh sach ID/san pham cho moi gia; cap nhat Vector value khi insert.

Khong sua BST generic thanh multimap neu cac nguoi dung khac dang ky vong key
trung cap nhat value. Them test BST cu de giu behavior insert key trung.
filterByPriceRange phai lay day du san pham co gia o ca hai bien min/max.
Validate min <= max; neu min > max tra danh sach rong hoac loi theo API hien co,
nhung hanh vi phai ro rang va duoc test.

Test chap nhan:
- Tao >=3 phone co cung sellingPrice, va phone o hai gia bien.
- Filter range tra tat ca, khong duplicate, dung thu tu neu API quy dinh.
- Sua gia/xoa phone cap nhat price index dung.
- BST search/update/remove key trung giu nguyen semantics dang co.

===============================================================================
7. DAU VIEC F - KHONG XOA SAN PHAM CON DUOC THAM CHIEU
===============================================================================

Solution:
1. Tim helper ReferenceChecker trong source/services/Check.h va cac truy cap
   OrderRepository, InvoiceRepository, StockReceiptRepository.
2. Noi helper vao PhoneService::deletePhone truoc repository xoa.
3. Phan biet khong tim thay, dang duoc tham chieu va loi repository neu API hien
   tai co errorMessage/enum; tra thong diep co ID/type tham chieu neu co the.
4. Khong phu thuoc duy nhat vao viec xoa khoi UI; Service la noi enforcement.
5. Neu thuc the khong the xoa do lich su, co the de nghi "ngung kinh doanh"
   (active/archived) nhu feature rieng; khong them field/schema neu task chi can
   ngan xoa va no lam phat sinh migration khong can thiet.

Test chap nhan:
- Xoa san pham chua duoc tham chieu thanh cong.
- Xoa san pham co trong order/invoice/receipt bi tu choi, file va RAM khong doi.
- UI hien error ro rang va danh sach refresh khong bao thanh cong gia.

===============================================================================
8. DAU VIEC G - VALIDATION PARSER, SERVICE CON THIEU VA SECURITY CO BAN
===============================================================================

DateTime:
- Xem source/utils/DateTime.h: parse phai kiem tra du truong, ky tu du, gioi han
  ngay/thang/nam/gio/phut va ngay hop le (nam nhuan).
- Tra loi parse that bai theo convention hien tai; khong de ngay sai thanh
  01/01/0000 hoac mot DateTime default hop le gia.
- Giu nguyen cac format cu duoc ghi trong data guide; them test round-trip.

Parser cac thuc the:
- Kiem tra stoi/stod nem exception, enum cast ngoai pham vi, delimiter trong
  field, line empty/header. Repository phai bao loi co vi tri; khong lam crash
  toan bo app do mot dong data loi.
- Serializer/parser phai la cap doi xung; neu field co the chua '|', ',', ';',
  chon validate cam delimiter hoac escape co tuong thich. Khong tu y doi format
  ton tai ma khong cap nhat guide va data.

Service con thieu:
- CardService.h (ten file co the la loi chinh ta cua CartService), InventoryService.h
  va ErrorMessage.h: xac dinh duoc UI/build co dung khong truoc khi xoa/doi ten.
- Neu chuc nang can cho pham vi demo thi implement qua entity/repository/service
  theo pattern; neu khong duoc yeu cau thi ghi TODO/no-op ro rang, khong tao
  success-shaped stub.
- Chua co test tu dong: them test runner nhe theo toolchain hien co; khong them
  dependency ngoai neu khong can.

Mat khau:
- Account hien co the luu plaintext. Voi do an offline, khong gui mat khau that
  len repository hay log; dung tai khoan demo. Ghi ro day la gioi han hoc tap.
- Neu nang cap hash, phai chon ham hash mat khau phu hop va migration; khong tu
  viet crypto. Day khong phai blocker cua danh sach sua loi nghiep vu neu chua
  co thu vien/yeu cau, nhung phai bao cao ro trong ket qua.

===============================================================================
9. THU TU THUC HIEN VA PHAM VI UI
===============================================================================

Dot 1 - Tinh toan ven du lieu:
  A role round-trip; B data samples; parser validation.
Dot 2 - Tinh toan ven nghiep vu:
  C order/stock/cancel; D repository rollback; F referential integrity.
Dot 3 - DSA:
  E duplicate price; test regression.
Dot 4 - UI va demo:
  Build dung GTK4 entry point hien tai, xac minh cac callback dung Service API,
  xu ly loi va refresh. Neu UI goi API legacy khac voi service moi, sua adapter
  nho; khong dua repository vao UI.
  Thu tu giao dien demo co the la: danh sach/tim san pham; them/sua/xoa; tao va
  duyet/huy don; xem ton kho; nhap kho; voucher neu service da co.
  Khong tu dong tao Qt UI: workspace hien tai co GTK4 va Makefile GTK4; tai lieu
  UI Qt cu trong Summary khong phai bang chung Qt duoc su dung.
Dot 5 - Bao cao:
  Cap nhat CHANGELOG.md va file nay neu ket qua/hanh vi duoc chot thay doi.

Khong mo rong thanh authentication/role dashboard day du, backup system moi,
database SQL, networking, cloud, hoac kien truc 5 tang moi neu user khong yeu
cau. Khong coi PhoneStore/ skeleton chua build la production code; xac minh
Makefile va source target dang chay.

===============================================================================
10. TEST MATRIX VA DIEU KIEN NGHIEM THU
===============================================================================

Can co unit/regression tests cho:
[ ] Employee role serialize/parse cho moi role, invalid code.
[ ] Tat ca file format co round-trip; duplicate IDs; dong sai format.
[ ] Order create/confirm/cancel, FIFO, hai don tranh chap ton kho.
[ ] Multi-item stock update atomicity; loi ghi file; rollback.
[ ] Repository add/update/remove va RAM/du lieu tren dia sau save fail.
[ ] Price range voi nhieu phone trung gia, sua gia va xoa.
[ ] Phone referential delete voi order/invoice/receipt.
[ ] DateTime invalid dates, leap years, boundary hours and round-trip.
[ ] Voucher loading/calculate/expiry/usage limits.

Build/test:
- Chay build target chinh bang Makefile (khong chi chay executable cu).
- Chay tat ca test moi va test lien quan hien co.
- Bat warning san co (-Wall -Wextra); khong tat warning moi de che bug.
- Neu GTK dependency khong co trong moi truong, ghi ro build core tests rieng va
  loi dependency; khong tu sua Makefile lam mat target GTK.
- Kiem tra demo voi data mau tren ban sao thu muc tam neu test co ghi file; tranh
  lam thay doi data that khi chay test.

Nghiem thu san sang demo khi:
1. Clean build thanh cong tren toolchain du an.
2. Du lieu mau nap khong co duplicate FK/ID, role va voucher dung.
3. Mot luong tao -> confirm -> cancel/completion co ket qua kho hop ly.
4. Save error khong bi bao thanh cong va regression test pass.
5. UI khong crash khi data file loi; nguoi dung thay thong diep that.
6. README/CHANGELOG huong dan build/run, format file va han che con lai.

===============================================================================
11. DINH DANG BAO CAO CUA AI THUC HIEN
===============================================================================

Sau khi lam viec, tra loi bang tieng Viet, gom:
1. Tom tat cac van de da khac phuc va file lien quan.
2. Cac quyet dinh nghiep vu (reservation, trang thai duoc huy, duplicate data).
3. Danh sach test/build da chay kem ket qua that.
4. Viec chua lam va ly do; moi gioi han atomic cua text files.
5. Xac nhan khong ghi de thay doi co san ngoai pham vi.

Khong danh dau hoan tat neu chua build va chay test. Khong tu nhan "production
ready"; muc tieu la mot do an PBL2 nho, co nghiep vu chinh dung, du lieu nhat
quan, va sinh vien co the giai thich duoc thiet ke.

===============================================================================
12. DAU VIEC H - HOAN THIEN BACKEND PHONESTORE VA SAN SANG GAN UI
===============================================================================

Phan nay duoc bo sung tu dot audit backend/UI-readiness ngay 2026-10-15.
Day la danh sach viec can giao AI coder; cac nhan dinh phai duoc xac minh lai
tren workspace hien tai truoc khi sua, vi code co the da thay doi.

QUYET DINH PHAM VI BAT BUOC:
- Repository hien co hai huong code: ung dung legacy o root (source/, UI/,
  Makefile/main.cpp) va scaffold 5 tang PhoneStore/. Khong gop hai huong mot
  cach tu dong, khong xoa/di chuyen code. Kiem tra git status va build target
  thuc te truoc; hoi chu repo neu khong the xac dinh PhoneStore/ la target
  chinh hay chi la scaffold.
- Audit PhoneStore truoc do thay src/controller khong co controller, src/view
  chi co gtk/.gitkeep, tests/ chua co test va khong tim thay CMakeLists.txt
  hay build manifest trong PhoneStore/. UI legacy trong src/_unplaced/ khong
  duoc tinh la da wire. Xac minh lai truoc khi ket luan.
- Cac .cpp PhoneStore nam ngoai _unplaced da qua kiem tra cu phap rieng le voi
  g++ -std=c++17 -Wall -Wextra -Wpedantic -fsyntax-only; viec nay KHONG chung
  minh da link/build/run duoc ung dung.
- Muc tieu la hoan thien backend PhoneStore theo kien truc 5 tang neu day la
  huong chu repo chon; khong tu y viet lai ung dung legacy hay tao UI moi truoc
  khi xac dinh ro target.

H.1 - TAO TARGET BUILD VA KHA NANG KIEM THU
1. Them build config nhe phu hop voi toolchain repo (CMake neu du an chon CMake,
   hoac tich hop vao Makefile hien co neu PhoneStore la target cua repo).
   Khai bao ro source files; khong dua src/_unplaced vao target mot cach vo y.
2. Tao entry point / AppContext khoi tao FileManager, repository, service,
   session provider va dependency theo dung thu tu; load data, hien thi loi
   load ro rang, khong bo qua LoadReport.
3. Build ca target chinh va test voi -Wall -Wextra (them -Wpedantic neu tuong
   thich); khong chi syntax-check tung file. Ghi lai lenh build/test chinh xac.
4. Tao test runner headless va data directory tam; test khong duoc ghi vao data
   that, khong phu thuoc GTK/display/network.
5. Khong bao cao UI-ready cho den khi clean build, test headless va wiring app
   deu duoc xac nhan.

H.2 - COMMON, DTO, PERMISSION VA SECURITY
1. Chot ma tran role/action cho Customer, Sales, Warehouse, Admin/Manager voi
   chu repo. Sua Permission::isAllowed va test moi cap role/action; khong tu
   choi mac dinh tat ca quyen nhan vien neu nghiep vu can cho phep.
2. Bao dam moi thao tac ghi trong Service kiem tra Actor/Permission tai service,
   khong tin vao viec UI an nut. Customer chi xem/huy don cua chinh minh.
3. Chot mot kieu tien duy nhat (khuyen nghi so nguyen VND); chuyen model, DTO,
   tinh tong/discount, serialization va parser cho nhat quan. Tranh ep double
   thanh so nguyen lam mat phan le; them round-trip va tinh tien boundary tests.
4. Thiet ke schema khong luu mat khau plaintext. Neu day la demo offline, van
   khong dung credential that; neu co authentication, dung password hash phu
   hop va co test migration/parse format. Khong tu them package neu chua can.
5. Giam phu thuoc tang: Actor khong nen include model neu co the tach Role vao
   common; DTO khong include repository/storage hay entity model truc tiep.
   Chon mot kieu collection thong nhat theo yeu cau do an, tranh lam DTO phu
   thuoc storage/ds neu khong can thiet. Service/Controller/View chi trao DTO.

H.3 - STORAGE VA CAC CAU TRUC DU LIEU TU CAI
1. Sua FileManager::writeAllLinesAtomic: khong xoa file dich truoc khi thay the
   thanh cong; ten file tam phai tranh xung dot; khi that bai phai giu file cu
   va tra loi loi that. Test loi ghi/rename bang thu muc tam.
2. Nang cap backup/restore: restore phai khop toan bo tap file cua backup
   (xem xet file tao sau backup), co rollback neu ghi mot file that bai; bao
   cao ro gioi han atomic cua nhieu file neu khong the dam bao transaction
   nguyen khoi tren filesystem. Kiem tra ten backup va path traversal.
3. Sua UnitOfWork de commit that bai co ket qua rollback/flush duoc kiem tra;
   khong bo qua loi khi khoi phuc. Test loi o repository flush thu N va xac
   nhan RAM, index va file tro ve trang thai truoc giao dich.
4. Ra soat Vector: clear/pop_back phai huy dung vong doi cua phan tu bi loai,
   dac biet voi T quan ly tai nguyen; giu dung copy/move/destructor va
   exception safety. Them test voi kieu dem constructor/destructor va Result
   chua Vector.
5. Ra soat HashTable, BST, Queue, LinkedList: cam shallow copy voi kieu so huu;
   khai bao hoac xoa ro rang copy/move operations theo Rule of 3/5. Test copy,
   move, remove, clear, self-assignment va destruct. Khong ket luan double-free
   neu chua co repro.
6. LineCodec phai round-trip duoc |, ;, backslash, newline, empty field va
   escape sai; parser bao dong/field loi ro rang.

H.4 - REPOSITORY, INDEX VA UNIT OF WORK
Hoan tat CRUD, serialize/deserialize, load/save, duplicate detection,
rebuildIndexes va snapshot/restore cho tung repository dang duoc dung. Uu tien:
- PhoneRepository: findByName, findByBrand, findByPriceRange va index gia.
  BST key gia phai ho tro nhieu san pham cung gia; test trung gia, sua gia, xoa.
- CustomerRepository: username/phone index va findByUsername/findByPhone.
- EmployeeRepository: username index va findByUsername; clone/restore phai giu
  dung dynamic type Manager/SalesEmployee/WarehouseEmployee, khong slicing.
- DiscountRepository: findByCode/index code.
- OrderRepository: index theo customer, loc theo customer/status, FIFO pending;
  peekNextPending phai doc lai trang thai that va bo qua don khong con PENDING.
- InvoiceRepository: index/find theo order ID.
- ImportReceiptRepository, SupplierRepository, DataRepository: xac minh CRUD,
  FK, load/save, backup/restore va rebuild indexes theo cung hop dong.
- EmployeeRepository dung raw pointer: giu ownership ro rang, clone that bai
  khong ro ri; uu tien smart pointer neu khong lam thay doi API qua lon.

UnitOfWork phai duoc su dung cho moi nghiep vu cap nhat nhieu repository.
Enlist snapshot mot lan; commit flush cac repository; neu mot flush loi thi tra
loi goc va phuc hoi ca RAM/index/file theo kha nang cam ket. Neu text files
khong the giao dich nguyen khoi, phai ghi ro gioi han va thiet ke staging/
manifest hoac backup tam de tranh trang thai nua chung.

H.5 - HOAN THIEN TOAN BO 77 METHOD SERVICE CON STUB
Audit 2026-10-15 ghi nhan 77 method co TODO/NotImplemented. Doi chieu lai tung
method; neu van con, trien khai va test cac method sau (khong de stub tra loi
thanh cong gia):

- AuthService: login.
- CartService: getCart, addItem, removeItem, updateQuantity, clear,
  calculateTotal, checkout.
- CustomerService: registerCustomer, authenticate, getProfile, updateProfile,
  getCustomerById, listAll, deleteCustomer.
- DataService: backup, listBackups, restore, importPhones, importCustomers,
  exportInvoice.
- DiscountService: addDiscount, updateDiscount, removeDiscount, listAll,
  validateAndCalculate, consumeUsage, releaseUsage.
- EmployeeService: addEmployee, updateEmployee, removeEmployee, listAll,
  getEmployeeById, authenticate, assignRole, setActive.
- ImportReceiptService: createReceipt, getById, list.
- InvoiceService: createInvoiceFromOrder, getByOrderId, list.
- OrderService: createOrder, confirmOrder, startPreparing, markReadyForPickup,
  markShipping, takePayment, completeOrder, cancelOrder, createCounterSale,
  listForCustomer, listByStatus, getOrder.
- PhoneService: listForCustomer, listForStaff, search, filterByPrice, addPhone,
  updatePhone, removePhone, checkAvailable, reserve, releaseReserved,
  commitReserved, restock, receiveStock.
- ReportService: revenueByPeriod, orderCountByStatus, bestSellingProducts,
  stockSummary.
- SupplierService: addSupplier, updateSupplier, removeSupplier, listAll, search,
  getSupplierById.

Moi method can validate input, phan quyen, ownership, not-found/duplicate,
trang thai, FK va loi persistence; tra Result loi ro rang. Khong de TODO logic
trong duong chay nghiep vu chinh. Neu chuc nang ngoai pham vi, ghi ro ly do,
khong de UI goi mot API co ve hoat dong nhung thuc te NotImplemented.

H.6 - QUY TAC NGHIEP VU BAT BUOC
1. OrderStateMachine: bao phu PENDING, CONFIRMED, PREPARING, READY_FOR_PICKUP,
   SHIPPING, COMPLETED, CANCELLED; test moi transition hop le/khong hop le va
   phan biet PICKUP voi DELIVERY.
2. Ton kho: available = quantity - reservedQuantity; PENDING reserve; confirm
   tru reserved va quantity; cancel PENDING release; cancel CONFIRMED restock.
   Bao dam inventoryState chong release/hoan kho hai lan va UoW rollback
   quantity/reservedQuantity/index/file cung luc.
3. Khong cho huy don tu PREPARING tro di neu business rule da chot nhu prompt;
   khong complete neu chua PAID. Khach chi xem/huy don cua minh.
4. Checkout: kiem tra ton va discount, tao don, tang/ghi usage, clear cart trong
   mot giao dich; neu loi thi voucher, gio hang va kho phai duoc giu nguyen.
5. Counter sale: tao order, thu tien, tru kho va tao invoice trong mot UoW.
6. Import receipt: luu phieu va cong kho trong cung UoW; verify supplier,
   product, so luong va gia nhap.
7. FIFO: thu tu theo thoi diem dat; queue stale ID phai duoc bo qua neu order
   da huy/xac nhan.
8. Report doanh thu chi tinh order COMPLETED va PAID; test bien ngay, timezone
   va don bi huy/chua thanh toan.

H.7 - DTO, CONTROLLER, VIEW VA APP WIRING
1. Tao/hoan thien controllers cho Customer, Sales, Warehouse, Admin/Manager;
   controller nhan event tu handler, goi Service, chuyen Result thanh thong
   diep hien thi, va khong include Repository/FileManager.
2. Them session provider implementation va AppContext wiring; khong de view tu
   tao Repository/Service hoac truy cap file.
3. Kiem tra DTO theo tung man hinh: PhoneView khong lo internal ID; controller
   giu mapping index hien thi -> phoneId; PhoneStaffView co stock/reserved;
   OrderSummary/OrderDraft co ten khach, dia chi/loai giao, trang thai,
   payment, tong truoc/sau discount va chi tiet dong hang; Invoice/Import
   receipt co chi tiet line; CheckoutInfo, ReportRow, ImportReport du truong
   de render form/bang va thong diep loi.
4. Tao IXxxView/IXxxHandler cho event va hien thi can thiet, bao dam service
   feature co API/UI event tuong ung; khong them ham chi de du cho du neu
   nghiep vu khong co that.
5. Chay compile-time checks de dam bao view/controller khong include
   repository/storage; Service khong tra model mutable/reference cho tang tren.

H.8 - TEST HEADLESS BAT BUOC: tests/ui_readiness_test.cpp
Sau khi co build target, AppContext va cac service khong con stub, tao va tich
hop tests/ui_readiness_test.cpp (hoac giu ten file neu test runner san co).
Test phai dung thu muc data tam moi lan, co cleanup an toan dung target; khong
dua credential that vao test. Bao gom it nhat:

Kich ban 1 - Admin/Warehouse:
- Dang nhap Admin, them Supplier va Phone; dang nhap Warehouse, tao Import
  Receipt; xac nhan quantity tang dung; tao Discount.

Kich ban 2 - Customer/Sales, checkout va huy:
- Dang ky/dang nhap Customer, tim Phone, them vao Cart, checkout co Discount.
- Xac nhan reservedQuantity tang, usage voucher tang theo nghiep vu da chot.
- Customer huy don PENDING cua minh; reservedQuantity duoc release, voucher
  usage duoc hoan; thu huy don cua khach khac va xac nhan bi tu choi.

Kich ban 3 - FIFO, giao hang, hoan tat va bao cao:
- Customer dat don; Sales lay don PENDING FIFO va confirm; Warehouse chuan bi
  va mark shipping; Sales take payment/complete; tao Invoice.
- Kiem tra quantity/reservedQuantity, order/payment state va revenueByPeriod;
  xac nhan don chua PAID/CANCELLED khong vao doanh thu.

Kich ban 4 - Persistence va backup/restore:
- Tao lai AppContext tu data dir tam va xac nhan file text nap dung entity,
  index, role, stock va order state.
- Backup; thay doi/xoa du lieu; restore; reload repository va so sanh snapshot.
- Co test loi write/restore nua chung va xac nhan loi duoc bao ro, khong bao
  thanh cong gia hoac lam mat du lieu cu.

Neu mot trong bon scenario chua the chay, khong bo scenario khoi test va khong
bao cao backend dat 100%; ghi ro blocker va tiep tuc hoan thien. Bo sung unit
tests cho Repository/Service/UoW/LineCodec/DSA de khi integration test fail co
the khoanh vung nhanh.

H.9 - THU TU THUC HIEN / GATE HOAN TAT
Thuc hien theo thu tu phu thuoc:
1. Xac nhan target chinh, git status, build baseline, scope PhoneStore vs
   legacy; chot role, money, credential/persistence contract.
2. Build config, entry point, AppContext va data temp test harness.
3. Sua storage/DSA, backup/restore, UnitOfWork va test failure paths.
4. Hoan tat repository, indexes, duplicate/FK validation va test round-trip.
5. Trien khai auth/customer/employee/supplier/discount/catalog va permission.
6. Trien khai cart/checkout, order state machine, inventory reservation,
   payment, counter sale; test atomicity/FIFO/ownership.
7. Trien khai receipt, invoice, report, import/export/backup services.
8. Hoan thien DTO contract, controllers, session va view wiring.
9. Viet/chay day du tests/ui_readiness_test.cpp tren data directory tam.
10. Clean build target chinh, chay toan bo test, xem diff va cap nhat README/
    DATA_FORMAT_GUIDE.md/CHANGELOG.md; bao cao chinh xac cac han che con lai.

Dieu kien hoan tat H:
- Khong con TODO/NotImplemented tren API nghiep vu duoc UI su dung.
- Build/link target chinh thanh cong voi warnings duoc xem xet.
- Ca 4 scenario headless pass, co test loi va rollback.
- Khong ghi data that khi test; khong mat thay doi co san; moi thay doi schema
  duoc tai lieu hoa va co migration/tuong thich mau du lieu neu can.
- Chi ket luan "san sang gan UI" khi controller/service/view da duoc wire va
  test; neu chua co UI thi phan biet ro "backend san sang tich hop" voi "UI da
  hoat dong". Khong tu nhan production-ready.
