/**
 * @file unit_tests.cpp
 * @brief Bộ kiểm thử đơn vị & tích hợp không dùng thư viện ngoài (dùng assert & hàm kiểm tra)
 * 
 * PHẠM VI KIỂM THỬ:
 * 1. FileStore:
 *    - Ghi file và đọc lại đúng nội dung tiếng Việt có dấu (UTF-8).
 *    - kiemTraTruong trả về false khi trường chứa ký tự '|' hoặc '\n'.
 *    - Ghi thất bại giữa chừng: file gốc được bảo toàn nguyên vẹn.
 *    - Tồn tại file .tmp cũ: lần ghi tiếp theo vẫn thành công, file gốc cập nhật đúng.
 * 2. InventoryService:
 *    - Chuỗi nghiệp vụ: tồn đầu 0, nhập 10, bán 3, nhập 5, hủy phiếu nhập 5 -> tồn còn 7.
 *    - Dòng thời gian giá vốn: nhập 10 cái giá 100 (ngày 1), nhập 5 cái giá 120 (ngày 2)
 *      -> giá vốn 120. Hủy phiếu ngày 2 -> giá vốn trở lại 100.
 *    - Kiểm tra chặn bán vượt tồn kho.
 * 3. InvoiceService:
 *    - Tính toán hóa đơn 2 sản phẩm: tamTinh, giamGia, tienSauGiam, tienThue (8%), tongThanhToan, giaVon, loiNhuan.
 *    - Chặn giảm giá âm hoặc giảm giá vượt quá tạm tính.
 *    - Hủy hóa đơn: tồn kho tự động hoàn lại, doanh thu loại bỏ hóa đơn đã hủy, chặn hủy lần 2.
 *    - Thiếu tồn kho ở dòng thứ 2/3: toàn bộ hóa đơn bị hủy, không có dòng nào được ghi (tính nguyên tử).
 * 4. Tích hợp MigrateData:
 *    - Chạy chuyển đổi dữ liệu trên bản sao dữ liệu mẫu cũ.
 *    - Kiểm tra số dòng, cấu trúc 9 cột Product, StockReceipts details, Invoices dòng H & D.
 *    - Đối soát tổng thanh toán, thuế, tạm tính khớp 100% với tính tay.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include "source/utils/FileStore.h"
#include "source/services/InventoryService.h"
#include "source/services/InvoiceService.h"

namespace fs = std::filesystem;

// ================= BỘ ĐẾM KẾT QUẢ TEST =================
static int g_totalTests = 0;
static int g_passedTests = 0;
static int g_failedTests = 0;

void checkTest(bool condition, const std::string& testName, const std::string& extraMsg = "") {
    g_totalTests++;
    if (condition) {
        g_passedTests++;
        std::cout << "  [PASS] " << testName << "\n";
    } else {
        g_failedTests++;
        std::cout << "  [FAIL] " << testName;
        if (!extraMsg.empty()) {
            std::cout << " -> " << extraMsg;
        }
        std::cout << "\n";
    }
}

// Xóa thư mục an toàn không ném ngoại lệ
void safeRemoveDir(const std::string& path) {
    std::error_code ec;
    fs::remove_all(path, ec);
}

// =========================================================================
// PHẦN 1: KIỂM THỬ FILESTORE
// =========================================================================
void test_filestore() {
    std::cout << "\n============================================================\n";
    std::cout << "1. KIỂM THỬ TIỆN ÍCH FILESTORE\n";
    std::cout << "============================================================\n";

    std::string testDir = "test_sandbox_filestore";
    safeRemoveDir(testDir);
    fs::create_directories(testDir);

    // 1.1 Ghi file rồi đọc lại đúng nội dung, bao gồm tiếng Việt có dấu
    {
        std::string filePath = testDir + "/tieng_viet.txt";
        std::vector<std::string> sampleLines = {
            "SP01|Điện thoại iPhone 15 Pro Max Xanh Titan|29990000|Bảo hành chính hãng 12 tháng",
            "SP02|Samsung Galaxy S24 Ultra Đen Titan|26990000|Khuyến mãi tặng ốp lưng & củ sạc nhanh",
            "SP03|Xiaomi 14 Pro Đen Bóng|16990000|Màn hình cong tràn viền, sạc siêu tốc 120W"
        };

        bool writeOk = FileStore::ghiFileAnToan(filePath, sampleLines);
        checkTest(writeOk, "FileStore::ghiFileAnToan ghi thành công nội dung tiếng Việt");

        auto readRows = FileStore::docFile(filePath);
        bool readMatch = (readRows.size() == 3);
        if (readMatch) {
            readMatch = (readRows[0][1] == "Điện thoại iPhone 15 Pro Max Xanh Titan" &&
                         readRows[1][3] == "Khuyến mãi tặng ốp lưng & củ sạc nhanh" &&
                         readRows[2][1] == "Xiaomi 14 Pro Đen Bóng");
        }
        checkTest(readMatch, "FileStore::docFile đọc lại chính xác 100% nội dung tiếng Việt có dấu");
    }

    // 1.2 kiemTraTruong trả về false khi có ký tự '|' hoặc xuống dòng
    {
        bool t1 = FileStore::kiemTraTruong("iPhone 15 Pro Max");
        bool t2 = FileStore::kiemTraTruong("Apple");
        bool f1 = FileStore::kiemTraTruong("iPhone|15");
        bool f2 = FileStore::kiemTraTruong("|");
        bool f3 = FileStore::kiemTraTruong("SP001|");
        bool f4 = FileStore::kiemTraTruong("Dòng 1\nDòng 2");

        checkTest(t1 && t2, "FileStore::kiemTraTruong trả về true với chuỗi hợp lệ");
        checkTest(!f1 && !f2 && !f3 && !f4, "FileStore::kiemTraTruong trả về false khi có '|' hoặc '\\n'");
    }

    // 1.3 Khi ghi thất bại giữa chừng, file gốc không bị thay đổi
    {
        std::string originPath = testDir + "/file_goc.txt";
        std::vector<std::string> originalLines = {
            "DONG_GOC_1|Du lieu khong duoc phep bi mat",
            "DONG_GOC_2|Bao toan toan ven du lieu"
        };
        FileStore::ghiFileAnToan(originPath, originalLines);

        // Mô phỏng thất bại giữa chừng:
        // Tạo một thư mục có tên trùng với file .tmp (originPath + ".tmp")
        // Khi ghiFileAnToan cố gắng mở std::ofstream trên đường dẫn thư mục, stream sẽ mở thất bại
        std::string blockedTmp = originPath + ".tmp";
        fs::create_directories(blockedTmp + "/sub_lock");

        std::vector<std::string> corruptLines = {
            "DONG_RAC_1|Du lieu rac muon ghi de",
            "DONG_RAC_2|Du lieu rac 2"
        };
        bool failResult = FileStore::ghiFileAnToan(originPath, corruptLines);
        checkTest(!failResult, "FileStore::ghiFileAnToan trả về false khi quá trình ghi gặp sự cố");

        // Dọn dẹp thư mục khóa
        safeRemoveDir(blockedTmp);

        // Kiểm tra file gốc: nội dung phải còn nguyên vẹn
        auto checkRows = FileStore::docFile(originPath);
        bool originIntact = (checkRows.size() == 2 &&
                             checkRows[0][0] == "DONG_GOC_1" &&
                             checkRows[1][0] == "DONG_GOC_2");
        checkTest(originIntact, "File gốc được bảo toàn nguyên vẹn sau khi lần ghi bị thất bại");
    }

    // 1.4 Còn file .tmp cũ từ lần trước: lần ghi tiếp theo vẫn thành công và file gốc đúng
    {
        std::string targetPath = testDir + "/target_tmp.txt";
        std::string leftoverTmp = targetPath + ".tmp";

        // Tạo sẵn file .tmp chứa dữ liệu rác tồn đọng
        std::ofstream oldTmp(leftoverTmp);
        oldTmp << "RAC_TMP_CU_TU_LAN_TRUOC\n";
        oldTmp.close();

        std::vector<std::string> newLines = {
            "DONG_MOI_1|Gia tri moi hop le",
            "DONG_MOI_2|Gia tri cap nhat chuan"
        };
        bool writeSuccess = FileStore::ghiFileAnToan(targetPath, newLines);
        checkTest(writeSuccess, "Ghi file thành công ngay cả khi có file .tmp rác tồn đọng");

        auto readBack = FileStore::docFile(targetPath);
        bool contentCorrect = (readBack.size() == 2 &&
                               readBack[0][0] == "DONG_MOI_1" &&
                               readBack[1][0] == "DONG_MOI_2");
        checkTest(contentCorrect, "File gốc chứa đúng dữ liệu mới, không bị ảnh hưởng bởi file .tmp cũ");
    }

    safeRemoveDir(testDir);
}

// =========================================================================
// PHẦN 2: KIỂM THỬ INVENTORYSERVICE
// =========================================================================
void test_inventory_service() {
    std::cout << "\n============================================================\n";
    std::cout << "2. KIỂM THỬ INVENTORYSERVICE (TỒN KHO & GIÁ VỐN)\n";
    std::cout << "============================================================\n";

    std::string testDir = "test_sandbox_inventory";
    safeRemoveDir(testDir);
    fs::create_directories(testDir);

    std::string pPath = testDir + "/Products.txt";
    std::string rPath = testDir + "/StockReceipts.txt";
    std::string iPath = testDir + "/Invoices.txt";
    std::string sPath = testDir + "/Suppliers.txt";

    // Khởi tạo sản phẩm: importPrice mặc định 100
    // id|phoneName|brand|os|importPrice|sellingPrice|manufacturer|specifications|status
    std::vector<std::string> pLines = {
        "SP01|iPhone Test|Apple|iOS|100|150|Foxconn|128GB|DangBan"
    };
    std::vector<std::string> sLines = {
        "id|companyName|taxCode|contactPerson|phoneNumber|email|address",
        "SUP01|Supplier Test|0123456789|Contact|0909|test@sup.com|HN"
    };
    FileStore::ghiFileAnToan(pPath, pLines);
    FileStore::ghiFileAnToan(rPath, {});
    FileStore::ghiFileAnToan(iPath, {});
    FileStore::ghiFileAnToan(sPath, sLines);

    InventoryService invService(pPath, rPath, iPath, sPath);

    // 2.1 Chuỗi tồn kho: tồn đầu 0, nhập 10, bán 3, nhập 5, hủy phiếu nhập 5 -> tồn còn 7
    {
        // Tồn đầu = 0
        int ton0 = invService.tinhTonKho("SP01");
        checkTest(ton0 == 0, "InventoryService: Tồn kho ban đầu bằng 0");

        // Nhập 10 (Phiếu REC01)
        std::vector<InventoryService::ItemInput> items1 = {{"SP01", "iPhone Test", 10, 100.0}};
        OperationResult resN1 = invService.taoPhieuNhap("REC01", "SUP01", "EMP01", "2026-10-01 09:00:00", items1);
        checkTest(resN1.success, "InventoryService: Nhập kho 10 cái (Phiếu REC01)");
        int ton1 = invService.tinhTonKho("SP01");
        checkTest(ton1 == 10, "InventoryService: Tồn kho sau nhập 10 là 10");

        // Bán 3 (Ghi trực tiếp hóa đơn HoatDong vào Invoices.txt)
        std::vector<std::string> invLines = {
            "H|INV01|ORD01|CUST01|EMP01|CASH|2026-10-01 10:00:00|450|0|8|36|486|HoatDong",
            "D|INV01|SP01|3|150|100"
        };
        FileStore::ghiFileAnToan(iPath, invLines);
        int ton2 = invService.tinhTonKho("SP01");
        checkTest(ton2 == 7, "InventoryService: Tồn kho sau khi bán 3 còn 7");

        // Nhập 5 (Phiếu REC02)
        std::vector<InventoryService::ItemInput> items2 = {{"SP01", "iPhone Test", 5, 120.0}};
        OperationResult resN2 = invService.taoPhieuNhap("REC02", "SUP01", "EMP01", "2026-10-02 09:00:00", items2);
        checkTest(resN2.success, "InventoryService: Nhập kho 5 cái (Phiếu REC02)");
        int ton3 = invService.tinhTonKho("SP01");
        checkTest(ton3 == 12, "InventoryService: Tồn kho sau nhập 5 là 12");

        // Hủy phiếu nhập 5 (REC02)
        OperationResult resHuyN2 = invService.huyPhieuNhap("REC02");
        checkTest(resHuyN2.success, "InventoryService: Hủy phiếu nhập 5 cái (REC02) thành công");
        int ton4 = invService.tinhTonKho("SP01");
        checkTest(ton4 == 7, "InventoryService: Tồn kho cuối cùng sau khi hủy phiếu nhập còn đúng 7");
    }

    // 2.2 Dòng thời gian giá vốn:
    // Nhập 10 cái giá 100 (ngày 1), nhập 5 cái giá 120 (ngày 2) -> giá vốn 120. Hủy phiếu ngày 2 -> giá vốn 100.
    {
        // Khởi tạo lại dữ liệu sạch cho test giá vốn
        FileStore::ghiFileAnToan(rPath, {});
        FileStore::ghiFileAnToan(iPath, {});

        // Nhập phiếu REC_D1 ngày 2026-10-01 giá 100
        std::vector<InventoryService::ItemInput> itm1 = {{"SP01", "iPhone Test", 10, 100.0}};
        invService.taoPhieuNhap("REC_D1", "SUP01", "EMP01", "2026-10-01 08:00:00", itm1);
        long long gv1 = invService.giaVonHienTai("SP01");
        checkTest(gv1 == 100, "InventoryService: Giá vốn sau phiếu ngày 1 là 100");

        // Nhập phiếu REC_D2 ngày 2026-10-02 giá 120
        std::vector<InventoryService::ItemInput> itm2 = {{"SP01", "iPhone Test", 5, 120.0}};
        invService.taoPhieuNhap("REC_D2", "SUP01", "EMP01", "2026-10-02 08:00:00", itm2);
        long long gv2 = invService.giaVonHienTai("SP01");
        checkTest(gv2 == 120, "InventoryService: Giá vốn sau phiếu ngày 2 cập nhật thành 120");

        // Hủy phiếu ngày 2 -> giá vốn tự động quay về phiếu ngày 1 (100)
        invService.huyPhieuNhap("REC_D2");
        long long gvSauHuy = invService.giaVonHienTai("SP01");
        checkTest(gvSauHuy == 100, "InventoryService: Hủy phiếu ngày 2 -> giá vốn quay về 100");
    }

    // 2.3 Không cho bán vượt tồn kho
    {
        // Hiện tại REC_D1 có 10 cái
        bool du10 = invService.kiemTraDuTonKho("SP01", 10);
        bool du11 = invService.kiemTraDuTonKho("SP01", 11);
        bool du0 = invService.kiemTraDuTonKho("SP01", 0);

        checkTest(du10, "InventoryService: Cho phép bán khi số lượng <= tồn kho khả dụng (yêu cầu 10 / tồn 10)");
        checkTest(!du11, "InventoryService: Chặn bán khi số lượng > tồn kho khả dụng (yêu cầu 11 / tồn 10)");
        checkTest(!du0, "InventoryService: Chặn bán khi số lượng <= 0");
    }

    safeRemoveDir(testDir);
}

// =========================================================================
// PHẦN 3: KIỂM THỬ INVOICESERVICE
// =========================================================================
void test_invoice_service() {
    std::cout << "\n============================================================\n";
    std::cout << "3. KIỂM THỬ INVOICESERVICE (HÓA ĐƠN BÁN HÀNG)\n";
    std::cout << "============================================================\n";

    std::string testDir = "test_sandbox_invoice";
    safeRemoveDir(testDir);
    fs::create_directories(testDir);

    std::string pPath = testDir + "/Products.txt";
    std::string rPath = testDir + "/StockReceipts.txt";
    std::string iPath = testDir + "/Invoices.txt";

    // Tạo danh mục sản phẩm:
    // SP_A giá bán 1.000.000
    // SP_B giá bán 500.000
    // SP_C giá bán 300.000
    std::vector<std::string> pLines = {
        "SP_A|Sản phẩm A|BrandA|OS|800000|1000000|NSX|SpecA|DangBan",
        "SP_B|Sản phẩm B|BrandB|OS|400000|500000|NSX|SpecB|DangBan",
        "SP_C|Sản phẩm C|BrandC|OS|200000|300000|NSX|SpecC|DangBan"
    };
    FileStore::ghiFileAnToan(pPath, pLines);

    // Nhập kho:
    // SP_A: 10 cái giá 800.000
    // SP_B: 10 cái giá 400.000
    // SP_C: 2 cái giá 200.000 (SP_C chỉ có 2 cái tồn kho)
    std::vector<std::string> rLines = {
        "REC01|SUP01|EMP01|2026-10-10 10:00:00|SP_A,10,800000;SP_B,10,400000;SP_C,2,200000|HoatDong"
    };
    FileStore::ghiFileAnToan(rPath, rLines);
    FileStore::ghiFileAnToan(iPath, {});

    InventoryService invService(pPath, rPath, iPath, "");
    InvoiceService invoiceService(&invService, iPath, pPath, "", "");

    // 3.1 Hóa đơn 2 sản phẩm:
    // tamTinh = 2 x 1.000.000 + 1 x 500.000 = 2.500.000; giảm 100.000; tiền sau giảm 2.400.000;
    // thuế 8% = 192.000; tổng = 2.592.000.
    std::string invId1;
    {
        CreateInvoiceRequest req;
        req.customerId = "CUST01";
        req.employeeId = "EMP01";
        req.paymentMethod = PaymentMethod::CASH;
        req.discountAmount = 100000;
        req.items.push_back(InvoiceDetailInput("SP_A", 2, 1000000));
        req.items.push_back(InvoiceDetailInput("SP_B", 1, 500000));

        OperationResult res = invoiceService.taoHoaDon(req, invId1);
        checkTest(res.success, "InvoiceService: Tạo hóa đơn 2 sản phẩm thành công", res.message);

        InvoiceItemViewModel vm = invoiceService.layHoaDon(invId1);
        bool matchTamTinh     = (vm.subtotal == 2500000.0);
        bool matchGiamGia     = (vm.discountAmount == 100000.0);
        bool matchTienSauGiam = ((vm.subtotal - vm.discountAmount) == 2400000.0);
        bool matchThue        = (vm.vatRate == 8 && vm.vatAmount == 192000.0);
        bool matchTong        = (vm.total == 2592000.0);
        bool matchStatus      = (vm.status == "HoatDong");
        bool matchCost        = (vm.details.size() == 2 && vm.details[0].costPrice == 800000.0 && vm.details[1].costPrice == 400000.0);
        bool matchProfit      = (vm.profit == 400000.0);

        checkTest(matchTamTinh, "tamTinh = 2 x 1.000.000 + 1 x 500.000 = 2.500.000");
        checkTest(matchGiamGia, "giamGia = 100.000");
        checkTest(matchTienSauGiam, "tienSauGiam = 2.400.000");
        checkTest(matchThue, "tienThue (8% của 2.400.000) = 192.000");
        checkTest(matchTong, "tongThanhToan = 2.400.000 + 192.000 = 2.592.000");
        checkTest(matchStatus, "Trạng thái hóa đơn mới tạo là 'HoatDong'");
        checkTest(matchCost && matchProfit, "giaVon dòng D lưu cố định và lợi nhuận gộp tính đúng (400.000 đ)");
    }

    // 3.2 Giảm giá âm hoặc lớn hơn tamTinh: bị từ chối
    {
        CreateInvoiceRequest reqAm;
        reqAm.items.push_back(InvoiceDetailInput("SP_A", 1, 1000000));
        reqAm.discountAmount = -50000;
        std::string dummyId;
        OperationResult resAm = invoiceService.taoHoaDon(reqAm, dummyId);
        checkTest(!resAm.success, "InvoiceService: Từ chối hóa đơn có giảm giá âm (-50.000 đ)");

        CreateInvoiceRequest reqVuot;
        reqVuot.items.push_back(InvoiceDetailInput("SP_A", 1, 1000000));
        reqVuot.discountAmount = 1500000; // Vượt quá tạm tính 1.000.000
        OperationResult resVuot = invoiceService.taoHoaDon(reqVuot, dummyId);
        checkTest(!resVuot.success, "InvoiceService: Từ chối hóa đơn có giảm giá vượt quá tạm tính (1.500.000 / 1.000.000 đ)");
    }

    // 3.3 Hủy hóa đơn: tồn kho tăng đúng, doanh thu không còn tính hóa đơn đó
    {
        // Trước khi hủy: tồn SP_A = 8, tồn SP_B = 9
        int tonA_truoc = invService.tinhTonKho("SP_A");
        int tonB_truoc = invService.tinhTonKho("SP_B");
        long long dt_truoc = invoiceService.tongDoanhThu();
        checkTest(tonA_truoc == 8 && tonB_truoc == 9, "Tồn kho sau bán: SP_A=8, SP_B=9");
        checkTest(dt_truoc == 2592000, "Doanh thu trước khi hủy: 2.592.000 đ");

        // Thực hiện hủy hóa đơn
        OperationResult resHuy = invoiceService.huyHoaDon(invId1);
        checkTest(resHuy.success, "Hủy hóa đơn hợp lệ thành công");

        // Sau khi hủy: tồn kho tự động hồi phục về 10, doanh thu trở về 0
        int tonA_sau = invService.tinhTonKho("SP_A");
        int tonB_sau = invService.tinhTonKho("SP_B");
        long long dt_sau = invoiceService.tongDoanhThu();
        checkTest(tonA_sau == 10 && tonB_sau == 10, "Tồn kho tự động tăng đúng trở lại sau khi hủy (SP_A=10, SP_B=10)");
        checkTest(dt_sau == 0, "Doanh thu không còn tính hóa đơn đã hủy (doanh thu = 0 đ)");

        // Chặn không cho hủy lại hóa đơn đã hủy
        OperationResult resHuyLai = invoiceService.huyHoaDon(invId1);
        checkTest(!resHuyLai.success, "Không cho phép hủy hóa đơn đã hủy trước đó");
    }

    // 3.4 Hóa đơn thiếu tồn ở dòng thứ 2 của 3: không có dòng nào được ghi vào file (nguyên tử)
    {
        // Hiện tại: SP_A có 10 cái, SP_C có 2 cái, SP_B có 10 cái
        auto rowsTruoc = FileStore::docFile(iPath);
        size_t soDongTruoc = rowsTruoc.size();

        CreateInvoiceRequest req3;
        req3.items.push_back(InvoiceDetailInput("SP_A", 2, 1000000)); // Dòng 1: cần 2 / tồn 10 (đủ)
        req3.items.push_back(InvoiceDetailInput("SP_C", 5, 300000));  // Dòng 2: cần 5 / tồn 2 (THIẾU TỒN KHO)
        req3.items.push_back(InvoiceDetailInput("SP_B", 1, 500000));  // Dòng 3: cần 1 / tồn 10 (đủ)

        std::string dummyId3;
        OperationResult res3 = invoiceService.taoHoaDon(req3, dummyId3);
        checkTest(!res3.success, "InvoiceService: Từ chối hóa đơn khi dòng thứ 2/3 không đủ tồn kho", res3.message);

        // Kiểm tra tính nguyên tử: file Invoices.txt không có thêm dòng nào
        auto rowsSau = FileStore::docFile(iPath);
        checkTest(rowsSau.size() == soDongTruoc, "Tính nguyên tử: Không có dòng nào được ghi vào file Invoices.txt khi phát hiện thiếu tồn kho");
    }

    safeRemoveDir(testDir);
}

// =========================================================================
// PHẦN 4: KIỂM THỬ TÍCH HỢP TỐI THIỂU (MIGRATEDATA)
// =========================================================================
void test_integration_migrate_data() {
    std::cout << "\n============================================================\n";
    std::cout << "4. KIỂM THỬ TÍCH HỢP TỐI THIỂU: MIGRATEDATA TRÊN DỮ LIỆU MẪU\n";
    std::cout << "============================================================\n";

    std::string testDir = "test_sandbox_migrate";
    safeRemoveDir(testDir);
    fs::create_directories(testDir);

    // Chuẩn bị dữ liệu mẫu định dạng CŨ:
    // 1. Product.txt cũ: id|phoneName|brand|os|importPrice|sellingPrice|quantity|manufacturer|specs
    std::vector<std::string> oldProductLines = {
        "id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications",
        "P01|iPhone 13|Apple|iOS|12000000|15000000|10|Foxconn|128GB",
        "P02|Galaxy A54|Samsung|Android|6000000|8000000|20|Samsung|256GB"
    };
    FileStore::ghiFileAnToan(testDir + "/Product.txt", oldProductLines);

    // 2. stockReceipts.txt cũ: id|supplierId|employeeId|importTime|details (details có tên SP, ngày dd/MM/yyyy HH:mm:ss)
    std::vector<std::string> oldReceiptLines = {
        "id|supplierId|employeeId|importTime|details",
        "REC01|SUP01|EMP01|15/10/2026 09:30:00|P01,iPhone 13,5,12000000;P02,Galaxy A54,10,6000000"
    };
    FileStore::ghiFileAnToan(testDir + "/stockReceipts.txt", oldReceiptLines);

    // 3. Invoices.txt cũ: id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details
    // Chi tiết: 2 x P01 @ 15.000.000 + 1 x P02 @ 8.000.000, giảm giá 500.000
    // Tính tay:
    // - tamTinh = 2 x 15.000.000 + 1 x 8.000.000 = 38.000.000
    // - giamGia = 500.000
    // - tienSauGiam = 37.500.000
    // - thueSuat = 8
    // - tienThue = round(37.500.000 x 8 / 100) = 3.000.000
    // - tongThanhToan = 37.500.000 + 3.000.000 = 40.500.000
    std::vector<std::string> oldInvoiceLines = {
        "id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details",
        "INV01|ORD01|CUST01|EMP01|CASH|16/10/2026 14:00:00|500000|P01,iPhone 13,2,15000000;P02,Galaxy A54,1,8000000"
    };
    FileStore::ghiFileAnToan(testDir + "/Invoices.txt", oldInvoiceLines);

    // Chạy MigrateData.exe trên thư mục test
    std::string cmd = "MigrateData.exe \"" + testDir + "\" > nul 2>&1";
#if defined(_WIN32)
    int exitCode = std::system(cmd.c_str());
#else
    cmd = "./MigrateData.exe \"" + testDir + "\" > /dev/null 2>&1";
    int exitCode = std::system(cmd.c_str());
#endif
    checkTest(exitCode == 0, "Chương trình MigrateData.exe thực thi thành công không lỗi");

    // Kiểm tra Product.txt sau migration
    auto newProdRows = FileStore::docFile(testDir + "/Product.txt");
    bool prodOk = (newProdRows.size() == 3); // 1 header + 2 dòng data
    if (prodOk) {
        prodOk = (newProdRows[1].size() == 9 && newProdRows[1][8] == "DangBan" &&
                  newProdRows[2].size() == 9 && newProdRows[2][8] == "DangBan");
    }
    checkTest(prodOk, "Product.txt sau migrate: 3 dòng, định dạng mới 9 cột, bỏ quantity, status='DangBan'");

    // Kiểm tra stockReceipts.txt sau migration
    auto newRecRows = FileStore::docFile(testDir + "/stockReceipts.txt");
    bool recOk = (newRecRows.size() == 2);
    if (recOk) {
        // details bỏ tên SP: P01,5,12000000;P02,10,6000000
        // importTime chuẩn: 2026-10-15 09:30:00, status: HoatDong
        recOk = (newRecRows[1][3] == "2026-10-15 09:30:00" &&
                 newRecRows[1][4] == "P01,5,12000000;P02,10,6000000" &&
                 newRecRows[1][5] == "HoatDong");
    }
    checkTest(recOk, "stockReceipts.txt sau migrate: chuẩn hóa importTime, details bỏ tên SP, status='HoatDong'");

    // Kiểm tra Invoices.txt sau migration:
    // 1 dòng H + 2 dòng D = 3 dòng
    auto newInvRows = FileStore::docFile(testDir + "/Invoices.txt");
    bool invCountOk = (newInvRows.size() == 3);
    checkTest(invCountOk, "Invoices.txt sau migrate: chuẩn hóa tách thành 1 dòng H và 2 dòng D (tổng 3 dòng)");

    if (invCountOk) {
        // Dòng H: H|id|orderId|customerId|employeeId|paymentMethod|createTime|tamTinh|giamGia|thueSuat|tienThue|tongThanhToan|status
        const auto& h = newInvRows[0];
        bool hValid = (h.size() >= 13 && h[0] == "H" && h[1] == "INV01");
        long long hTamTinh = hValid ? std::stoll(h[7]) : 0;
        long long hGiamGia = hValid ? std::stoll(h[8]) : 0;
        long long hThue    = hValid ? std::stoll(h[10]) : 0;
        long long hTong    = hValid ? std::stoll(h[11]) : 0;
        std::string hStatus= hValid ? h[12] : "";

        checkTest(hTamTinh == 38000000, "Invoices dòng H: tamTinh khớp tính tay = 38.000.000 đ");
        checkTest(hGiamGia == 500000,   "Invoices dòng H: giamGia khớp tính tay = 500.000 đ");
        checkTest(hThue == 3000000,     "Invoices dòng H: tienThue (8%) khớp tính tay = 3.000.000 đ");
        checkTest(hTong == 40500000,    "Invoices dòng H: tongThanhToan khớp tính tay = 40.500.000 đ");
        checkTest(hStatus == "DaThanhToan", "Invoices dòng H: status mặc định là 'DaThanhToan'");

        // Dòng D1: D|INV01|P01|2|15000000|12000000
        const auto& d1 = newInvRows[1];
        bool d1Ok = (d1.size() >= 6 && d1[0] == "D" && d1[2] == "P01" && d1[3] == "2" &&
                     d1[4] == "15000000" && d1[5] == "12000000");
        checkTest(d1Ok, "Invoices dòng D1: P01, số lượng 2, đơn giá 15.000.000, giá vốn lấy từ importPrice = 12.000.000");

        // Dòng D2: D|INV01|P02|1|8000000|6000000
        const auto& d2 = newInvRows[2];
        bool d2Ok = (d2.size() >= 6 && d2[0] == "D" && d2[2] == "P02" && d2[3] == "1" &&
                     d2[4] == "8000000" && d2[5] == "6000000");
        checkTest(d2Ok, "Invoices dòng D2: P02, số lượng 1, đơn giá 8.000.000, giá vốn lấy từ importPrice = 6.000.000");
    }

    // Dọn dẹp thư mục test và các thư mục backup do MigrateData tạo ra
    safeRemoveDir(testDir);
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name.rfind("data_backup_", 0) == 0 && name != "data_backup_20261023_200758") {
                safeRemoveDir(entry.path().string());
            }
        }
    }
}

// =========================================================================
// HÀM MAIN CHẠY TOÀN BỘ TEST SUITE
// =========================================================================
int main() {
    std::cout << "############################################################\n";
    std::cout << "#         PBL2 PHONE STORE MANAGER - UNIT TEST SUITE       #\n";
    std::cout << "############################################################\n";

    test_filestore();
    test_inventory_service();
    test_invoice_service();
    test_integration_migrate_data();

    std::cout << "\n============================================================\n";
    std::cout << "TỔNG KẾT BỘ KIỂM THỬ:\n";
    std::cout << "  - Tổng số bài test đã chạy: " << g_totalTests << "\n";
    std::cout << "  - Số bài test ĐẠT (PASS)  : " << g_passedTests << "\n";
    std::cout << "  - Số bài test HỎNG (FAIL) : " << g_failedTests << "\n";
    std::cout << "============================================================\n";

    if (g_failedTests == 0) {
        std::cout << "===> KẾT QUẢ: TẤT CẢ CÁC BÀI TEST ĐỀU ĐẠT CHUẨN (PASS)!\n\n";
        return 0;
    } else {
        std::cerr << "===> KẾT QUẢ: CÓ " << g_failedTests << " BÀI TEST BỊ THẤT BẠI (FAIL)!\n\n";
        return 1;
    }
}
