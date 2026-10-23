#ifndef INVOICESERVICE_H
#define INVOICESERVICE_H

#include <string>
#include <vector>
#include <map>
#include "../ui/viewmodels/OperationResult.h"
#include "../ui/viewmodels/InvoiceViewModel.h"
#include "../models/Common.h"

class InventoryService; // Forward declaration

/**
 * @struct InvoiceDetailInput
 * @brief Đầu vào chi tiết từng mặt hàng cần xuất hóa đơn
 */
struct InvoiceDetailInput {
    std::string productId;
    int quantity;
    long long unitPrice;

    InvoiceDetailInput() : quantity(0), unitPrice(0) {}
    InvoiceDetailInput(std::string pId, int q, long long pr)
        : productId(pId), quantity(q), unitPrice(pr) {}
};

/**
 * @struct CreateInvoiceRequest
 * @brief Yêu cầu tạo hóa đơn bán hàng
 */
struct CreateInvoiceRequest {
    std::string orderId;
    std::string customerId;
    std::string employeeId;
    PaymentMethod paymentMethod;
    std::string createTime; // Để trống = thời gian hiện tại yyyy-MM-dd HH:mm:ss
    long long discountAmount;
    std::vector<InvoiceDetailInput> items;

    CreateInvoiceRequest() : paymentMethod(PaymentMethod::CASH), discountAmount(0) {}
};

/**
 * @class InvoiceService
 * @brief Tầng nghiệp vụ xử lý hóa đơn bán hàng tuân thủ quy tắc dữ liệu Invoices.txt và tài chính số nguyên.
 * 
 * QUY TẮC TÍNH TIỀN (Số nguyên long long, đơn vị đồng):
 * 1. tamTinh = tổng (soLuong × donGia) của các dòng D.
 * 2. Tiền sau giảm = tamTinh − giamGia. Giảm giá không âm và không vượt quá tamTinh.
 * 3. tienThue = làm tròn (tiền sau giảm × 8 / 100) = (tienSauGiam * 8 + 50) / 100.
 * 4. tongThanhToan = tiền sau giảm + tienThue.
 * 5. giaVon dòng D lấy từ InventoryService::giaVonHienTai(productId) tại thời điểm lập hóa đơn và lưu cố định.
 * 
 * QUY TẮC GHI & HỦY:
 * 6. Toàn bộ hóa đơn (dòng H và tất cả dòng D) ghi trong MỘT lần gọi FileStore::ghiFileAnToan.
 * 7. Kiểm tra tồn kho trước khi ghi bằng InventoryService::kiemTraDuTonKho.
 * 8. Mã hóa đơn theo định dạng INV-ddMMyyyy-STT, STT tăng theo ngày.
 * 9. Kiểm tra trường văn bản qua FileStore::kiemTraTruong.
 * 10-12. Hủy hóa đơn: đổi status thành "DaHuy", không xóa dòng, không cho hủy hóa đơn đã hủy.
 */
class InvoiceRepository;
class OrderRepository;

class InvoiceService {
private:
    std::string invoicesPath;
    std::string productsPath;
    std::string customersPath;
    std::string employeesPath;
    InventoryService* inventoryService;
    bool ownInventoryService;

    static std::string resolvePath(const std::string& preferred, const std::string& fallback);

public:
    InvoiceService(
        InventoryService* invService = nullptr,
        std::string invPath = "",
        std::string pPath = "",
        std::string cPath = "",
        std::string ePath = ""
    );
    // Tương thích với main.cpp
    InvoiceService(InvoiceRepository& iRepo, OrderRepository& oRepo);
    ~InvoiceService();

    void setInventoryService(InventoryService* invService);

    // 1. Tạo hóa đơn mới (Quy tắc 1 - 9)
    OperationResult taoHoaDon(const CreateInvoiceRequest& req, std::string& outInvoiceId);

    // 2. Hủy hóa đơn (Quy tắc 10 - 12)
    OperationResult huyHoaDon(const std::string& maHD);

    // 3. Lấy thông tin chi tiết một hóa đơn theo mã
    InvoiceItemViewModel layHoaDon(const std::string& maHD) const;

    // 4. Danh sách hóa đơn có bộ lọc theo ngày và từ khóa
    std::vector<InvoiceItemViewModel> danhSachHoaDon(
        const std::string& fromDate = "",
        const std::string& toDate = "",
        const std::string& keyword = ""
    ) const;

    // 5. Thống kê: Tổng doanh thu (chỉ tính hóa đơn HoatDong)
    long long tongDoanhThu(const std::string& fromDate = "", const std::string& toDate = "") const;

    // 6. Thống kê: Lợi nhuận = tiền sau giảm − tổng (soLuong × giaVon) (chỉ tính hóa đơn HoatDong)
    long long tongLoiNhuan(const std::string& fromDate = "", const std::string& toDate = "") const;

    // Helper: Sinh mã hóa đơn kế tiếp INV-ddMMyyyy-STT theo ngày
    std::string sinhMaHoaDon(const std::string& dateKey = "") const;
};

#endif // INVOICESERVICE_H
