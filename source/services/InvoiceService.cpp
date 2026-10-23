#include "InvoiceService.h"
#include "InventoryService.h"
#include "../utils/FileStore.h"
#include "../utils/StringHelper.h"
#include "../utils/DateTime.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <cmath>

std::string InvoiceService::resolvePath(const std::string& preferred, const std::string& fallback) {
    std::error_code ec;
    if (std::filesystem::exists(preferred, ec)) return preferred;
    if (std::filesystem::exists(fallback, ec)) return fallback;
    return preferred;
}

InvoiceService::InvoiceService(
    InventoryService* invService,
    std::string invPath,
    std::string pPath,
    std::string cPath,
    std::string ePath
) {
    invoicesPath  = !invPath.empty() ? invPath : resolvePath("data/PAY/Invoices.txt", "data/Invoices.txt");
    productsPath  = !pPath.empty()   ? pPath   : resolvePath("data/Device/Products.txt", "data/Product.txt");
    customersPath = !cPath.empty()   ? cPath   : resolvePath("data/people/Customers.txt", "data/Customers.txt");
    employeesPath = !ePath.empty()   ? ePath   : resolvePath("data/people/Employee.txt", "data/Employee.txt");

    if (invService != nullptr) {
        inventoryService = invService;
        ownInventoryService = false;
    } else {
        inventoryService = new InventoryService("", "", invoicesPath, "");
        ownInventoryService = true;
    }
}

InvoiceService::InvoiceService(InvoiceRepository&, OrderRepository&) {
    invoicesPath  = resolvePath("data/PAY/Invoices.txt", "data/Invoices.txt");
    productsPath  = resolvePath("data/Device/Products.txt", "data/Product.txt");
    customersPath = resolvePath("data/people/Customers.txt", "data/Customers.txt");
    employeesPath = resolvePath("data/people/Employee.txt", "data/Employee.txt");

    inventoryService = new InventoryService("", "", invoicesPath, "");
    ownInventoryService = true;
}

InvoiceService::~InvoiceService() {
    if (ownInventoryService && inventoryService != nullptr) {
        delete inventoryService;
        inventoryService = nullptr;
    }
}

void InvoiceService::setInventoryService(InventoryService* invService) {
    if (ownInventoryService && inventoryService != nullptr) {
        delete inventoryService;
    }
    inventoryService = invService;
    ownInventoryService = false;
}

// Helper chuẩn hóa ngày dd/MM/yyyy thành yyyy-MM-dd
static std::string normalizeDate(const std::string& d) {
    std::string s = StringHelper::trim(d);
    if (s.size() == 10 && s[2] == '/' && s[5] == '/') {
        return s.substr(6, 4) + "-" + s.substr(3, 2) + "-" + s.substr(0, 2);
    }
    return s;
}

std::string InvoiceService::sinhMaHoaDon(const std::string& dateKeyInput) const {
    std::string dateKey = dateKeyInput;
    if (dateKey.empty()) {
        DateTime now = DateTime::now();
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d%02d%04d", now.day, now.month, now.year);
        dateKey = buf;
    }

    std::string prefix = "INV-" + dateKey + "-";
    unsigned int maxSeq = 0;

    auto rows = FileStore::docFile(invoicesPath);
    for (const auto& row : rows) {
        if (!row.empty() && row[0] == "H" && row.size() >= 2) {
            const std::string& id = row[1];
            if (id.rfind(prefix, 0) == 0) { // Bắt đầu bằng prefix
                std::string numPart = id.substr(prefix.size());
                try {
                    unsigned int n = static_cast<unsigned int>(std::stoul(numPart));
                    if (n > maxSeq) maxSeq = n;
                } catch (...) {}
            }
        }
    }

    unsigned int nextSeq = maxSeq + 1;
    char seqBuf[16];
    snprintf(seqBuf, sizeof(seqBuf), "%06u", nextSeq);
    return prefix + seqBuf;
}

OperationResult InvoiceService::taoHoaDon(const CreateInvoiceRequest& req, std::string& outInvoiceId) {
    outInvoiceId.clear();

    if (req.items.empty()) {
        return OperationResult::Fail("Danh sách sản phẩm xuất hóa đơn không được để trống!");
    }

    // 1. Kiểm tra ký tự '|' trong các trường văn bản (Quy tắc 9)
    if (!FileStore::kiemTraTruong(req.orderId) ||
        !FileStore::kiemTraTruong(req.customerId) ||
        !FileStore::kiemTraTruong(req.employeeId)) {
        return OperationResult::Fail("Thông tin đơn hàng, khách hàng hoặc nhân viên chứa ký tự không hợp lệ '|'!");
    }

    for (const auto& it : req.items) {
        if (!FileStore::kiemTraTruong(it.productId)) {
            return OperationResult::Fail("Mã sản phẩm [" + it.productId + "] chứa ký tự không hợp lệ '|'!");
        }
        if (it.quantity <= 0) {
            return OperationResult::Fail("Số lượng của sản phẩm [" + it.productId + "] phải lớn hơn 0!");
        }
        if (it.unitPrice <= 0) {
            return OperationResult::Fail("Đơn giá của sản phẩm [" + it.productId + "] phải lớn hơn 0!");
        }
    }

    // 2. Kiểm tra tồn kho trước khi ghi (Quy tắc 7)
    // Nếu bất kỳ dòng nào không đủ, hủy toàn bộ thao tác và nêu rõ sản phẩm nào không đủ
    if (inventoryService != nullptr) {
        for (const auto& it : req.items) {
            if (!inventoryService->kiemTraDuTonKho(it.productId, it.quantity)) {
                int curStock = inventoryService->tinhTonKho(it.productId);
                return OperationResult::Fail(
                    "Sản phẩm [" + it.productId + "] không đủ tồn kho để xuất hóa đơn! " +
                    "(Yêu cầu: " + std::to_string(it.quantity) + ", tồn kho khả dụng hiện tại: " +
                    std::to_string(curStock) + ")"
                );
            }
        }
    }

    // 3. Tính toán tài chính số nguyên (Quy tắc 1, 2, 3, 4, 5)
    // - tamTinh = tổng (soLuong * donGia)
    // - giaVon của mỗi dòng D là giá vốn hiện tại tại thời điểm lập hóa đơn, lưu cố định
    long long tamTinh = 0;
    std::vector<long long> costPrices;
    costPrices.reserve(req.items.size());

    for (const auto& it : req.items) {
        long long lineSubtotal = static_cast<long long>(it.quantity) * it.unitPrice;
        tamTinh += lineSubtotal;

        long long cost = 0;
        if (inventoryService != nullptr) {
            cost = static_cast<long long>(std::round(inventoryService->giaVonHienTai(it.productId)));
        }
        costPrices.push_back(cost);
    }

    // - giamGia: không được âm và không vượt quá tamTinh
    if (req.discountAmount < 0) {
        return OperationResult::Fail("Số tiền giảm giá không được âm!");
    }
    if (req.discountAmount > tamTinh) {
        return OperationResult::Fail(
            "Số tiền giảm giá (" + std::to_string(req.discountAmount) +
            " đ) không được vượt quá tạm tính (" + std::to_string(tamTinh) + " đ)!"
        );
    }

    // - Tiền sau giảm
    long long tienSauGiam = tamTinh - req.discountAmount;

    // - tienThue = làm tròn (tiền sau giảm * 8 / 100) dùng số nguyên long long
    long long thueSuat = 8;
    long long tienThue = (tienSauGiam * 8 + 50) / 100;

    // - tongThanhToan = tiền sau giảm + tienThue
    long long tongThanhToan = tienSauGiam + tienThue;

    // 4. Sinh mã hóa đơn mới (Quy tắc 8)
    std::string invoiceId = sinhMaHoaDon();

    // Thời gian lập hóa đơn yyyy-MM-dd HH:mm:ss
    std::string createTime = req.createTime;
    if (createTime.empty()) {
        DateTime now = DateTime::now();
        createTime = now.toString();
    }

    // 5. Chuẩn bị dữ liệu ghi: dòng H và tất cả dòng D (Quy tắc 6)
    // Format dòng H: H|id|orderId|customerId|employeeId|paymentMethod|createTime|tamTinh|giamGia|thueSuat|tienThue|tongThanhToan|status
    std::vector<std::string> rowH = {
        "H",
        invoiceId,
        req.orderId,
        req.customerId,
        req.employeeId,
        std::to_string(static_cast<int>(req.paymentMethod)),
        createTime,
        std::to_string(tamTinh),
        std::to_string(req.discountAmount),
        std::to_string(thueSuat),
        std::to_string(tienThue),
        std::to_string(tongThanhToan),
        "HoatDong"
    };

    // Format các dòng D: D|invoiceId|productId|soLuong|donGia|giaVon
    std::vector<std::vector<std::string>> rowsD;
    for (size_t i = 0; i < req.items.size(); ++i) {
        rowsD.push_back({
            "D",
            invoiceId,
            req.items[i].productId,
            std::to_string(req.items[i].quantity),
            std::to_string(req.items[i].unitPrice),
            std::to_string(costPrices[i])
        });
    }

    // 6. Ghi MỘT LẦN DUY NHẤT qua FileStore::ghiFileAnToan (Quy tắc 6)
    auto currentRows = FileStore::docFile(invoicesPath);
    std::vector<std::string> allLines;
    allLines.reserve(currentRows.size() + 1 + rowsD.size());

    for (const auto& r : currentRows) {
        if (!r.empty()) {
            allLines.push_back(FileStore::ghepTruong(r));
        }
    }

    allLines.push_back(FileStore::ghepTruong(rowH));
    for (const auto& d : rowsD) {
        allLines.push_back(FileStore::ghepTruong(d));
    }

    bool ok = FileStore::ghiFileAnToan(invoicesPath, allLines);
    if (!ok) {
        return OperationResult::Fail("Lỗi khi ghi dữ liệu hóa đơn vào cơ sở dữ liệu!");
    }

    outInvoiceId = invoiceId;
    return OperationResult::Ok("Lập hóa đơn bán hàng [" + invoiceId + "] thành công!");
}

OperationResult InvoiceService::huyHoaDon(const std::string& maHD) {
    if (maHD.empty()) return OperationResult::Fail("Mã hóa đơn không hợp lệ!");

    auto rows = FileStore::docFile(invoicesPath);
    if (rows.empty()) return OperationResult::Fail("Dữ liệu hóa đơn rỗng!");

    int targetHIdx = -1;
    for (size_t i = 0; i < rows.size(); ++i) {
        if (!rows[i].empty() && rows[i][0] == "H" && rows[i].size() >= 2 && rows[i][1] == maHD) {
            targetHIdx = static_cast<int>(i);
            std::string st = (rows[i].size() >= 13) ? rows[i][12] : "HoatDong";
            // Quy tắc 12: Không cho hủy hóa đơn đã hủy
            if (st == "DaHuy") {
                return OperationResult::Fail("Hóa đơn [" + maHD + "] đã được hủy trước đó!");
            }
            break;
        }
    }

    if (targetHIdx < 0) {
        return OperationResult::Fail("Không tìm thấy hóa đơn [" + maHD + "] để hủy!");
    }

    // Quy tắc 10: Không xóa dòng. Đổi status thành "DaHuy"
    while (rows[targetHIdx].size() < 13) rows[targetHIdx].push_back("");
    rows[targetHIdx][12] = "DaHuy";

    std::vector<std::string> lines;
    lines.reserve(rows.size());
    for (const auto& r : rows) {
        if (!r.empty()) {
            lines.push_back(FileStore::ghepTruong(r));
        }
    }

    bool ok = FileStore::ghiFileAnToan(invoicesPath, lines);
    if (!ok) {
        return OperationResult::Fail("Lỗi khi cập nhật trạng thái hủy hóa đơn vào file!");
    }

    // Quy tắc 11: Tồn kho tự động tính lại vì InventoryService chỉ tính các hóa đơn HoatDong
    return OperationResult::Ok("Đã hủy hóa đơn [" + maHD + "] thành công! Số lượng tồn kho đã được tự động hoàn lại.");
}

InvoiceItemViewModel InvoiceService::layHoaDon(const std::string& maHD) const {
    auto list = danhSachHoaDon("", "", maHD);
    for (const auto& inv : list) {
        if (inv.id == maHD) return inv;
    }
    return InvoiceItemViewModel();
}

std::vector<InvoiceItemViewModel> InvoiceService::danhSachHoaDon(
    const std::string& fromDate,
    const std::string& toDate,
    const std::string& keyword
) const {
    std::vector<InvoiceItemViewModel> result;

    // 1. Đọc tên sản phẩm từ Product.txt (không lưu tên trong Invoices.txt)
    auto prodRows = FileStore::docFile(productsPath);
    std::map<std::string, std::string> prodNames;
    for (const auto& r : prodRows) {
        if (!r.empty() && r[0] != "id" && r.size() >= 2) {
            prodNames[r[0]] = r[1];
        }
    }

    // 2. Đọc tên khách hàng
    auto custRows = FileStore::docFile(customersPath);
    std::map<std::string, std::string> custNames;
    for (const auto& r : custRows) {
        if (!r.empty() && r[0] != "id" && r.size() >= 4) {
            // id|firstName|middleName|lastName|...
            std::string fullName = r[1];
            if (!r[2].empty()) fullName += " " + r[2];
            fullName += " " + r[3];
            custNames[r[0]] = fullName;
        }
    }

    // 3. Đọc tên nhân viên
    auto empRows = FileStore::docFile(employeesPath);
    std::map<std::string, std::string> empNames;
    for (const auto& r : empRows) {
        if (!r.empty() && r[0] != "id" && r.size() >= 4) {
            std::string fullName = r[1];
            if (!r[2].empty()) fullName += " " + r[2];
            fullName += " " + r[3];
            empNames[r[0]] = fullName;
        }
    }

    // 4. Đọc Invoices.txt
    auto invRows = FileStore::docFile(invoicesPath);
    std::map<std::string, InvoiceItemViewModel> invMap;
    std::vector<std::string> orderList;

    for (const auto& row : invRows) {
        if (row.empty()) continue;
        if (row[0] == "H" && row.size() >= 12) {
            InvoiceItemViewModel item;
            item.id = row[1];
            item.orderId = row[2];
            item.customerId = row[3];
            item.customerName = custNames.count(item.customerId) ? custNames[item.customerId] : (item.customerId.empty() ? "Khách vãng lai" : item.customerId);
            item.employeeId = row[4];
            item.employeeName = empNames.count(item.employeeId) ? empNames[item.employeeId] : item.employeeId;
            try { item.paymentMethod = static_cast<PaymentMethod>(std::stoi(row[5])); } catch (...) { item.paymentMethod = PaymentMethod::CASH; }
            item.paymentMethodText = paymentMethodToString(item.paymentMethod);
            item.createTime = row[6];
            try { item.subtotal = std::stod(row[7]); } catch (...) {}
            try { item.discountAmount = std::stod(row[8]); } catch (...) {}
            try { item.vatRate = std::stoll(row[9]); } catch (...) { item.vatRate = 8; }
            try { item.vatAmount = std::stod(row[10]); } catch (...) {}
            try { item.total = std::stod(row[11]); } catch (...) {}
            item.status = (row.size() >= 13) ? row[12] : "HoatDong";
            item.totalCost = 0.0;
            item.profit = 0.0;

            invMap[item.id] = item;
            orderList.push_back(item.id);
        } else if (row[0] == "D" && row.size() >= 6) {
            std::string invId = row[1];
            if (invMap.find(invId) != invMap.end()) {
                OrderDetailItemViewModel d;
                d.phoneId = row[2];
                d.phoneName = prodNames.count(d.phoneId) ? prodNames[d.phoneId] : d.phoneId;
                try { d.quantity = std::stoi(row[3]); } catch (...) { d.quantity = 0; }
                try { d.unitPrice = std::stod(row[4]); } catch (...) { d.unitPrice = 0.0; }
                try { d.costPrice = std::stod(row[5]); } catch (...) { d.costPrice = 0.0; }
                d.subtotal = d.quantity * d.unitPrice;

                invMap[invId].details.push_back(d);
                invMap[invId].totalCost += d.quantity * d.costPrice;
            }
        }
    }

    // Tính lợi nhuận cho từng hóa đơn
    for (auto& pair : invMap) {
        double tienSauGiam = pair.second.subtotal - pair.second.discountAmount;
        pair.second.profit = tienSauGiam - pair.second.totalCost;
    }

    // 5. Áp dụng bộ lọc
    std::string fromNorm = normalizeDate(fromDate);
    std::string toNorm   = normalizeDate(toDate);
    std::string kwLower  = StringHelper::toLower(StringHelper::trim(keyword));

    for (const auto& invId : orderList) {
        const auto& inv = invMap[invId];

        // Lọc ngày (so sánh 10 ký tự đầu yyyy-MM-dd)
        std::string invDate = (inv.createTime.size() >= 10) ? inv.createTime.substr(0, 10) : "";
        if (!fromNorm.empty() && invDate < fromNorm) continue;
        if (!toNorm.empty() && invDate > toNorm) continue;

        // Lọc từ khóa
        if (!kwLower.empty()) {
            std::string idLower = StringHelper::toLower(inv.id);
            std::string ordLower = StringHelper::toLower(inv.orderId);
            std::string custLower = StringHelper::toLower(inv.customerName);
            std::string empLower = StringHelper::toLower(inv.employeeName);
            if (idLower.find(kwLower) == std::string::npos &&
                ordLower.find(kwLower) == std::string::npos &&
                custLower.find(kwLower) == std::string::npos &&
                empLower.find(kwLower) == std::string::npos) {
                continue;
            }
        }

        result.push_back(inv);
    }

    // Sắp xếp mới nhất lên đầu
    std::reverse(result.begin(), result.end());
    return result;
}

long long InvoiceService::tongDoanhThu(const std::string& fromDate, const std::string& toDate) const {
    auto list = danhSachHoaDon(fromDate, toDate);
    long long total = 0;
    for (const auto& inv : list) {
        // Tổng doanh thu chỉ tính hóa đơn HoatDong (hoặc DaThanhToan cũ)
        if (inv.status != "DaHuy") {
            total += static_cast<long long>(std::round(inv.total));
        }
    }
    return total;
}

long long InvoiceService::tongLoiNhuan(const std::string& fromDate, const std::string& toDate) const {
    auto list = danhSachHoaDon(fromDate, toDate);
    long long totalProfit = 0;
    for (const auto& inv : list) {
        if (inv.status != "DaHuy") {
            long long sub = static_cast<long long>(std::round(inv.subtotal));
            long long disc = static_cast<long long>(std::round(inv.discountAmount));
            long long tienSauGiam = sub - disc;
            long long cost = static_cast<long long>(std::round(inv.totalCost));
            totalProfit += (tienSauGiam - cost);
        }
    }
    return totalProfit;
}
