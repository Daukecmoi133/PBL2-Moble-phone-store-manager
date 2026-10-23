/**
 * @file MigrateData.cpp
 * @brief Chương trình độc lập chuyển đổi dữ liệu từ định dạng cũ sang định dạng mới.
 *        Sử dụng lớp tiện ích FileStore đã xây dựng.
 * 
 * Quy tắc chuyển đổi:
 * 1. Tự động sao lưu toàn bộ thư mục data sang data_backup_[ngày_giờ].
 * 2. Chuyển đổi định dạng ngày dd/MM/yyyy HH:mm:ss sang yyyy-MM-dd HH:mm:ss.
 * 3. Chuẩn hóa Product.txt: bỏ quantity, thêm status ("DangBan" / "NgungBan").
 * 4. Chuẩn hóa stockReceipts.txt: chuẩn hóa importTime, details bỏ tên SP, status = "HoatDong".
 * 5. Chuẩn hóa Invoices.txt: tách thành 2 loại dòng H (Header) và D (Detail).
 *    Tính tamTinh, thueSuat = 8, tienThue = round((tamTinh - giamGia)*8/100), tongThanhToan.
 *    giaVon lấy từ importPrice hiện tại của sản phẩm trong Product (giá vốn ước tính).
 * 6. Ghi các dòng lỗi vào migration_loi.txt kèm số dòng và lý do, không dừng chương trình.
 * 7. In bảng thống kê chi tiết sau khi hoàn thành.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include "source/utils/FileStore.h"

namespace fs = std::filesystem;

// Cấu trúc lưu thông tin sản phẩm để tra cứu giá vốn và kiểm tra khóa ngoại
struct ProductInfo {
    std::string id;
    std::string name;
    double importPrice = 0.0;
    double sellingPrice = 0.0;
    int quantity = 0;
};

// Cấu trúc bản ghi lỗi
struct ErrorRecord {
    std::string fileName;
    int lineNumber;
    std::string reason;
    std::string rawData;
};

// Thống kê từng file
struct FileStats {
    std::string name;
    int linesRead = 0;
    int linesWritten = 0;
    int errorCount = 0;
};

// ================= HÀM TIỆN ÍCH BỔ TRỢ =================

// Lấy chuỗi ngày giờ hiện tại định dạng YYYYMMDD_HHMMSS để đặt tên thư mục backup
std::string layThoiGianHienTai() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
    return std::string(buf);
}

// Cắt khoảng trắng đầu và cuối chuỗi
std::string trimStr(const std::string& str) {
    const std::string ws = " \t\r\n";
    size_t first = str.find_first_not_of(ws);
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(ws);
    return str.substr(first, last - first + 1);
}

// Chuyển đổi định dạng số sang chuỗi nguyên (không có đuôi .000000 cho VND)
std::string formatVND(double val) {
    long long ll = static_cast<long long>(std::round(val));
    if (std::abs(val - ll) < 1e-4) {
        return std::to_string(ll);
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << val;
    return oss.str();
}

// Tách chuỗi theo ký tự phân cách
std::vector<std::string> splitString(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream ss(str);
    while (std::getline(ss, token, delim)) {
        tokens.push_back(trimStr(token));
    }
    if (!str.empty() && str.back() == delim) {
        tokens.push_back("");
    }
    return tokens;
}

// Chuyển đổi ngày dd/MM/yyyy HH:mm:ss sang yyyy-MM-dd HH:mm:ss
bool chuyenDoiNgay(const std::string& input, std::string& output, std::string& errMsg) {
    std::string s = trimStr(input);
    if (s.empty()) {
        errMsg = "Chuỗi ngày giờ trống";
        return false;
    }

    // Nếu đã ở dạng chuẩn yyyy-MM-dd...
    if (s.length() >= 10 && s[4] == '-' && s[7] == '-') {
        output = s;
        return true;
    }

    size_t spacePos = s.find(' ');
    std::string datePart = (spacePos != std::string::npos) ? s.substr(0, spacePos) : s;
    std::string timePart = (spacePos != std::string::npos) ? s.substr(spacePos + 1) : "00:00:00";

    std::vector<std::string> dmy = splitString(datePart, '/');
    if (dmy.size() != 3) {
        errMsg = "Định dạng ngày không hợp lệ (cần dd/MM/yyyy): '" + input + "'";
        return false;
    }

    int day = 0, month = 0, year = 0;
    try {
        day = std::stoi(dmy[0]);
        month = std::stoi(dmy[1]);
        year = std::stoi(dmy[2]);
    } catch (...) {
        errMsg = "Ngày tháng năm không phải là số: '" + input + "'";
        return false;
    }

    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
        errMsg = "Giá trị ngày tháng năm nằm ngoài phạm vi cho phép: '" + input + "'";
        return false;
    }

    // Nếu thiếu giây (HH:mm) thì thêm :00
    if (timePart.length() == 5 && timePart[2] == ':') {
        timePart += ":00";
    }

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %s", year, month, day, timePart.c_str());
    output = std::string(buf);
    return true;
}

// Tìm đường dẫn thực tế của file từ danh sách các ứng viên
std::string timDuongDan(const std::vector<std::string>& candidates) {
    for (const auto& path : candidates) {
        std::error_code ec;
        if (fs::exists(path, ec)) {
            return path;
        }
    }
    return candidates.empty() ? "" : candidates.front();
}

// Lấy số dòng vật lý trong file để đối soát chính xác
std::vector<std::pair<int, std::string>> docFileKemSoDong(const std::string& path) {
    std::vector<std::pair<int, std::string>> res;
    std::ifstream f(path);
    if (!f.is_open()) return res;

    std::string line;
    int lineNum = 0;
    while (std::getline(f, line)) {
        ++lineNum;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string trimmed = trimStr(line);
        if (trimmed.empty()) continue; // Bỏ qua dòng trống
        res.push_back({lineNum, line});
    }
    return res;
}

// ================= HÀM CHÍNH THỰC HIỆN MIGRATION =================

int main(int argc, char* argv[]) {
    std::cout << "============================================================\n";
    std::cout << "  PBL2 PHONE STORE MANAGER - DATA MIGRATION UTILITY (C++17) \n";
    std::cout << "============================================================\n";

    std::string baseDataDir = (argc > 1) ? argv[1] : "data";
    std::cout << "Thư mục dữ liệu nguồn: " << baseDataDir << "\n\n";

    // Danh sách lỗi thu thập được trong toàn bộ quá trình
    std::vector<ErrorRecord> danhSachLoi;

    // Thống kê từng file
    FileStats statProduct{"Product.txt"};
    FileStats statStock{"stockReceipts.txt"};
    FileStats statInvoice{"Invoices.txt"};

    // -------------------------------------------------------------
    // BƯỚC 1: TỰ ĐỘNG SAO LƯU THƯ MỤC DATA SANG data_backup_[ngày giờ]
    // -------------------------------------------------------------
    std::string timeStamp = layThoiGianHienTai();
    std::string backupDir = "data_backup_" + timeStamp;
    std::error_code ecBackup;

    if (!fs::exists(baseDataDir, ecBackup)) {
        std::cerr << "[LỖI NGHIÊM TRỌNG] Không tìm thấy thư mục: " << baseDataDir << "\n";
        return 1;
    }

    fs::create_directories(backupDir, ecBackup);
    fs::copy(baseDataDir, backupDir, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ecBackup);
    if (ecBackup) {
        std::cerr << "[Cảnh báo] Sao lưu thư mục data gặp lỗi: " << ecBackup.message() << "\n";
    } else {
        std::cout << "[BƯỚC 1/4] Sao lưu toàn bộ thư mục dữ liệu thành công!\n";
        std::cout << "          -> Thư mục lưu trữ: " << backupDir << "\n\n";
    }

    // -------------------------------------------------------------
    // BƯỚC 2: CHUYỂN ĐỔI PRODUCT.TXT
    // Định dạng mới: id|phoneName|brand|operatingSystem|importPrice|sellingPrice|manufacturer|specifications|status
    // (Bỏ quantity; status là "DangBan" hoặc "NgungBan")
    // -------------------------------------------------------------
    std::cout << "[BƯỚC 2/4] Bắt đầu chuyển đổi danh mục sản phẩm (Product)...\n";
    std::string pathProduct = timDuongDan({
        baseDataDir + "/Device/Products.txt",
        baseDataDir + "/Device/Product.txt",
        baseDataDir + "/Products.txt",
        baseDataDir + "/Product.txt"
    });

    std::map<std::string, ProductInfo> catalogSanPham;
    std::vector<std::string> dongMoiProduct;
    // Header mới
    dongMoiProduct.push_back("id|phoneName|brand|operatingSystem|importPrice|sellingPrice|manufacturer|specifications|status");

    auto rawProductLines = docFileKemSoDong(pathProduct);
    statProduct.linesRead = static_cast<int>(rawProductLines.size());

    bool isHeaderProd = true;
    for (const auto& item : rawProductLines) {
        int lineNum = item.first;
        const std::string& rawLine = item.second;
        std::vector<std::string> parts = splitString(rawLine, '|');

        if (isHeaderProd) {
            isHeaderProd = false;
            // Bỏ qua dòng header cũ nếu có
            if (!parts.empty() && trimStr(parts[0]) == "id") {
                continue;
            }
        }

        if (parts.size() < 9) {
            danhSachLoi.push_back({pathProduct, lineNum, "Thiếu trường dữ liệu (yêu cầu ít nhất 9 trường, thực tế: " + std::to_string(parts.size()) + ")", rawLine});
            statProduct.errorCount++;
            continue;
        }

        std::string id = parts[0];
        std::string name = parts[1];
        std::string brand = parts[2];
        std::string os = parts[3];
        std::string importPriceStr = parts[4];
        std::string sellingPriceStr = parts[5];
        std::string quantityStr = parts[6];
        std::string manufacturer = parts[7];
        std::string specs = parts[8];

        if (id.empty()) {
            danhSachLoi.push_back({pathProduct, lineNum, "Mã sản phẩm (id) bị rỗng", rawLine});
            statProduct.errorCount++;
            continue;
        }

        double importPrice = 0.0;
        double sellingPrice = 0.0;
        int qty = 0;
        try {
            importPrice = std::stod(importPriceStr);
            sellingPrice = std::stod(sellingPriceStr);
            qty = std::stoi(quantityStr);
        } catch (...) {
            danhSachLoi.push_back({pathProduct, lineNum, "Không thể đọc số tiền hoặc số lượng tồn kho", rawLine});
            statProduct.errorCount++;
            continue;
        }

        if (importPrice < 0 || sellingPrice <= 0) {
            danhSachLoi.push_back({pathProduct, lineNum, "Giá nhập (< 0) hoặc giá bán (<= 0) không hợp lệ", rawLine});
            statProduct.errorCount++;
            continue;
        }

        if (catalogSanPham.find(id) != catalogSanPham.end()) {
            danhSachLoi.push_back({pathProduct, lineNum, "Trùng lặp mã sản phẩm: " + id, rawLine});
            statProduct.errorCount++;
            continue;
        }

        // Lưu vào catalog phục vụ tra cứu giá vốn ở hóa đơn
        ProductInfo p;
        p.id = id;
        p.name = name;
        p.importPrice = importPrice;
        p.sellingPrice = sellingPrice;
        p.quantity = qty;
        catalogSanPham[id] = p;

        // Quy tắc xác định status: còn hàng (> 0) thì DangBan, hết hàng (<= 0) thì NgungBan
        std::string status = (qty > 0) ? "DangBan" : "NgungBan";

        std::vector<std::string> newParts = {
            id, name, brand, os,
            formatVND(importPrice),
            formatVND(sellingPrice),
            manufacturer, specs, status
        };
        dongMoiProduct.push_back(FileStore::ghepTruong(newParts));
    }

    statProduct.linesWritten = static_cast<int>(dongMoiProduct.size());
    if (!FileStore::ghiFileAnToan(pathProduct, dongMoiProduct)) {
        std::cerr << "[LỖI] Không thể ghi file sản phẩm an toàn: " << pathProduct << "\n";
    } else {
        std::cout << "          -> Đã ghi thành công " << statProduct.linesWritten << " dòng vào " << pathProduct << "\n\n";
    }

    // -------------------------------------------------------------
    // BƯỚC 3: CHUYỂN ĐỔI STOCKRECEIPTS.TXT
    // Định dạng mới: id|supplierId|employeeId|importTime|details|status
    // (importTime dạng yyyy-MM-dd HH:mm:ss, details bỏ tên SP, status mặc định "HoatDong")
    // -------------------------------------------------------------
    std::cout << "[BƯỚC 3/4] Bắt đầu chuyển đổi phiếu nhập kho (stockReceipts)...\n";
    std::string pathStock = timDuongDan({
        baseDataDir + "/Device/StockReceipts.txt",
        baseDataDir + "/Device/stockReceipts.txt",
        baseDataDir + "/StockReceipts.txt",
        baseDataDir + "/stockReceipts.txt"
    });

    std::vector<std::string> dongMoiStock;
    dongMoiStock.push_back("id|supplierId|employeeId|importTime|details|status");

    auto rawStockLines = docFileKemSoDong(pathStock);
    statStock.linesRead = static_cast<int>(rawStockLines.size());

    bool isHeaderStock = true;
    for (const auto& item : rawStockLines) {
        int lineNum = item.first;
        const std::string& rawLine = item.second;
        std::vector<std::string> parts = splitString(rawLine, '|');

        if (isHeaderStock) {
            isHeaderStock = false;
            if (!parts.empty() && trimStr(parts[0]) == "id") {
                continue;
            }
        }

        if (parts.size() < 5) {
            danhSachLoi.push_back({pathStock, lineNum, "Thiếu trường dữ liệu phiếu nhập (yêu cầu ít nhất 5 trường, thực tế: " + std::to_string(parts.size()) + ")", rawLine});
            statStock.errorCount++;
            continue;
        }

        std::string id = parts[0];
        std::string supplierId = parts[1];
        std::string employeeId = parts[2];
        std::string rawTime = parts[3];
        std::string rawDetails = parts[4];

        std::string newTime, errTime;
        if (!chuyenDoiNgay(rawTime, newTime, errTime)) {
            danhSachLoi.push_back({pathStock, lineNum, "Thời gian nhập kho lỗi: " + errTime, rawLine});
            statStock.errorCount++;
            continue;
        }

        // Chuyển đổi details: các sản phẩm cách nhau bằng ';', mỗi item gồm productId,soLuong,donGia (bỏ tên sản phẩm)
        std::vector<std::string> rawItems = splitString(rawDetails, ';');
        std::vector<std::string> newItems;
        bool detailError = false;

        for (const auto& rawIt : rawItems) {
            if (rawIt.empty()) continue;
            std::vector<std::string> d = splitString(rawIt, ',');
            // Format cũ: productId,phoneName,quantity,importPrice (4 trường)
            // Hoặc nếu đã có 3 trường: productId,quantity,importPrice
            if (d.size() < 3) {
                danhSachLoi.push_back({pathStock, lineNum, "Chi tiết sản phẩm nhập kho sai định dạng: '" + rawIt + "'", rawLine});
                detailError = true;
                break;
            }

            std::string pId = d[0];
            std::string qtyStr = (d.size() >= 4) ? d[d.size() - 2] : d[1];
            std::string priceStr = (d.size() >= 4) ? d[d.size() - 1] : d[2];

            // Kiểm tra mã sản phẩm tồn tại
            if (catalogSanPham.find(pId) == catalogSanPham.end()) {
                danhSachLoi.push_back({pathStock, lineNum, "Mã sản phẩm '" + pId + "' không tồn tại trong danh mục Product", rawLine});
                detailError = true;
                break;
            }

            try {
                int q = std::stoi(qtyStr);
                double pr = std::stod(priceStr);
                if (q <= 0 || pr < 0) {
                    danhSachLoi.push_back({pathStock, lineNum, "Số lượng nhập (<=0) hoặc giá nhập (<0) không hợp lệ trong chi tiết", rawLine});
                    detailError = true;
                    break;
                }
                newItems.push_back(pId + "," + std::to_string(q) + "," + formatVND(pr));
            } catch (...) {
                danhSachLoi.push_back({pathStock, lineNum, "Số lượng hoặc đơn giá trong chi tiết phiếu nhập không đọc được", rawLine});
                detailError = true;
                break;
            }
        }

        if (detailError) {
            statStock.errorCount++;
            continue;
        }

        std::string newDetailsJoined;
        for (size_t i = 0; i < newItems.size(); ++i) {
            if (i > 0) newDetailsJoined += ";";
            newDetailsJoined += newItems[i];
        }

        std::vector<std::string> newRow = {
            id, supplierId, employeeId, newTime, newDetailsJoined, "HoatDong"
        };
        dongMoiStock.push_back(FileStore::ghepTruong(newRow));
    }

    statStock.linesWritten = static_cast<int>(dongMoiStock.size());
    if (!FileStore::ghiFileAnToan(pathStock, dongMoiStock)) {
        std::cerr << "[LỖI] Không thể ghi file phiếu nhập an toàn: " << pathStock << "\n";
    } else {
        std::cout << "          -> Đã ghi thành công " << statStock.linesWritten << " dòng vào " << pathStock << "\n\n";
    }

    // -------------------------------------------------------------
    // BƯỚC 4: CHUYỂN ĐỔI INVOICES.TXT
    // Định dạng mới gồm 2 loại dòng:
    // H|id|orderId|customerId|employeeId|paymentMethod|createTime|tamTinh|giamGia|thueSuat|tienThue|tongThanhToan|status
    // D|invoiceId|productId|soLuong|donGia|giaVon
    // Quy tắc:
    // tamTinh = tổng (soLuong × donGia)
    // thueSuat = 8
    // tienThue = round((tamTinh - giamGia) * 8 / 100)
    // tongThanhToan = tamTinh - giamGia + tienThue
    // giaVon = importPrice hiện tại của sản phẩm (giá vốn ước tính)
    // -------------------------------------------------------------
    std::cout << "[BƯỚC 4/4] Bắt đầu chuyển đổi hóa đơn (Invoices)...\n";
    std::string pathInvoice = timDuongDan({
        baseDataDir + "/PAY/Invoices.txt",
        baseDataDir + "/PAY/Invoice.txt",
        baseDataDir + "/Invoices.txt",
        baseDataDir + "/Invoice.txt"
    });

    std::vector<std::string> dongMoiInvoice;
    auto rawInvoiceLines = docFileKemSoDong(pathInvoice);
    statInvoice.linesRead = static_cast<int>(rawInvoiceLines.size());

    bool isHeaderInv = true;
    for (const auto& item : rawInvoiceLines) {
        int lineNum = item.first;
        const std::string& rawLine = item.second;
        std::vector<std::string> parts = splitString(rawLine, '|');

        if (isHeaderInv) {
            isHeaderInv = false;
            if (!parts.empty() && trimStr(parts[0]) == "id") {
                continue;
            }
        }

        if (parts.size() < 8) {
            danhSachLoi.push_back({pathInvoice, lineNum, "Thiếu trường dữ liệu hóa đơn (yêu cầu ít nhất 8 trường, thực tế: " + std::to_string(parts.size()) + ")", rawLine});
            statInvoice.errorCount++;
            continue;
        }

        std::string id = parts[0];
        std::string orderId = parts[1];
        std::string customerId = parts[2];
        std::string employeeId = parts[3];
        std::string paymentMethod = parts[4];
        std::string rawTime = parts[5];
        std::string discountStr = parts[6];
        std::string rawDetails = parts[7];

        std::string newTime, errTime;
        if (!chuyenDoiNgay(rawTime, newTime, errTime)) {
            danhSachLoi.push_back({pathInvoice, lineNum, "Thời gian lập hóa đơn lỗi: " + errTime, rawLine});
            statInvoice.errorCount++;
            continue;
        }

        double giamGia = 0.0;
        try {
            giamGia = std::stod(discountStr);
            if (giamGia < 0) giamGia = 0;
        } catch (...) {
            danhSachLoi.push_back({pathInvoice, lineNum, "Giá trị giảm giá không đọc được số: '" + discountStr + "'", rawLine});
            statInvoice.errorCount++;
            continue;
        }

        // Parse chi tiết hóa đơn và tính toán số liệu
        std::vector<std::string> rawItems = splitString(rawDetails, ';');
        struct DetailRow {
            std::string pId;
            int qty;
            double unitPrice;
            double costPrice;
        };
        std::vector<DetailRow> detailRows;
        double tamTinh = 0.0;
        bool invDetailError = false;

        for (const auto& rawIt : rawItems) {
            if (rawIt.empty()) continue;
            std::vector<std::string> d = splitString(rawIt, ',');
            // Format cũ: phoneId,phoneName,quantity,unitPrice (4 trường)
            if (d.size() < 4) {
                danhSachLoi.push_back({pathInvoice, lineNum, "Chi tiết hóa đơn thiếu trường (cần ít nhất 4 trường): '" + rawIt + "'", rawLine});
                invDetailError = true;
                break;
            }

            std::string pId = d[0];
            std::string qtyStr = d[d.size() - 2];
            std::string priceStr = d[d.size() - 1];

            auto itProd = catalogSanPham.find(pId);
            if (itProd == catalogSanPham.end()) {
                danhSachLoi.push_back({pathInvoice, lineNum, "Mã sản phẩm '" + pId + "' trong hóa đơn không tồn tại trong danh mục Product", rawLine});
                invDetailError = true;
                break;
            }

            try {
                int q = std::stoi(qtyStr);
                double pr = std::stod(priceStr);
                if (q <= 0 || pr < 0) {
                    danhSachLoi.push_back({pathInvoice, lineNum, "Số lượng (<=0) hoặc đơn giá (<0) không hợp lệ trong chi tiết hóa đơn", rawLine});
                    invDetailError = true;
                    break;
                }
                double cost = itProd->second.importPrice; // Quy tắc 4: lấy importPrice hiện tại làm giá vốn ước tính
                detailRows.push_back({pId, q, pr, cost});
                tamTinh += q * pr;
            } catch (...) {
                danhSachLoi.push_back({pathInvoice, lineNum, "Số lượng hoặc đơn giá trong chi tiết hóa đơn không thể chuyển sang số", rawLine});
                invDetailError = true;
                break;
            }
        }

        if (invDetailError) {
            statInvoice.errorCount++;
            continue;
        }

        // Quy tắc 3: Tính toán thuế và tổng tiền
        // thueSuat = 8%
        const int thueSuat = 8;
        double taxableAmount = (tamTinh > giamGia) ? (tamTinh - giamGia) : 0.0;
        long long tienThue = static_cast<long long>(std::round(taxableAmount * thueSuat / 100.0));
        long long tamTinhLL = static_cast<long long>(std::round(tamTinh));
        long long giamGiaLL = static_cast<long long>(std::round(giamGia));
        long long tongThanhToan = (tamTinhLL - giamGiaLL) + tienThue;
        if (tongThanhToan < 0) tongThanhToan = 0;

        // Trạng thái mặc định: "DaThanhToan"
        std::string status = "DaThanhToan";

        // Tạo dòng H
        std::vector<std::string> hRow = {
            "H", id, orderId, customerId, employeeId, paymentMethod, newTime,
            std::to_string(tamTinhLL),
            std::to_string(giamGiaLL),
            std::to_string(thueSuat),
            std::to_string(tienThue),
            std::to_string(tongThanhToan),
            status
        };
        dongMoiInvoice.push_back(FileStore::ghepTruong(hRow));

        // Tạo các dòng D tương ứng
        for (const auto& det : detailRows) {
            std::vector<std::string> dRow = {
                "D", id, det.pId,
                std::to_string(det.qty),
                formatVND(det.unitPrice),
                formatVND(det.costPrice)
            };
            dongMoiInvoice.push_back(FileStore::ghepTruong(dRow));
        }
    }

    statInvoice.linesWritten = static_cast<int>(dongMoiInvoice.size());
    if (!FileStore::ghiFileAnToan(pathInvoice, dongMoiInvoice)) {
        std::cerr << "[LỖI] Không thể ghi file hóa đơn an toàn: " << pathInvoice << "\n";
    } else {
        std::cout << "          -> Đã ghi thành công " << statInvoice.linesWritten << " dòng vào " << pathInvoice << "\n\n";
    }

    // -------------------------------------------------------------
    // BƯỚC 5: GHI FILE NHẬT KÝ LỖI MIGRATION_LOI.TXT
    // -------------------------------------------------------------
    std::string pathLoi = "migration_loi.txt";
    std::vector<std::string> dongLoiLog;
    dongLoiLog.push_back("================================================================================");
    dongLoiLog.push_back("                     NHẬT KÝ LỖI MIGRATION DỮ LIỆU");
    dongLoiLog.push_back("Thời gian thực thi: " + layThoiGianHienTai());
    dongLoiLog.push_back("Tổng số lỗi phát hiện: " + std::to_string(danhSachLoi.size()));
    dongLoiLog.push_back("================================================================================");

    if (danhSachLoi.empty()) {
        dongLoiLog.push_back("Không có lỗi nào xảy ra. Toàn bộ dữ liệu hợp lệ và đã được chuyển đổi thành công!");
    } else {
        for (size_t i = 0; i < danhSachLoi.size(); ++i) {
            const auto& err = danhSachLoi[i];
            std::string logLine = "[" + std::to_string(i + 1) + "] File: " + err.fileName
                                + " | Dòng: " + std::to_string(err.lineNumber)
                                + " | Lý do: " + err.reason
                                + "\n     -> Dữ liệu gốc: " + err.rawData;
            dongLoiLog.push_back(logLine);
        }
    }
    dongLoiLog.push_back("================================================================================");

    FileStore::ghiFileAnToan(pathLoi, dongLoiLog);

    // -------------------------------------------------------------
    // BƯỚC 6: IN BÁO CÁO THỐNG KÊ CHI TIẾT
    // -------------------------------------------------------------
    int tongDoc = statProduct.linesRead + statStock.linesRead + statInvoice.linesRead;
    int tongGhi = statProduct.linesWritten + statStock.linesWritten + statInvoice.linesWritten;
    int tongLoi = statProduct.errorCount + statStock.errorCount + statInvoice.errorCount;

    std::cout << "================================================================================\n";
    std::cout << "                       BÁO CÁO THỐNG KÊ MIGRATION DỮ LIỆU                      \n";
    std::cout << "================================================================================\n";
    std::cout << "1. Thư mục sao lưu: " << backupDir << "\n";
    std::cout << "2. Chi tiết từng bảng dữ liệu:\n";
    std::cout << "   ┌──────────────────────┬─────────────┬─────────────┬─────────────┐\n";
    std::cout << "   │ Tên file dữ liệu     │ Số dòng đọc │ Số dòng ghi │ Số dòng lỗi │\n";
    std::cout << "   ├──────────────────────┼─────────────┼─────────────┼─────────────┤\n";
    
    auto inDongBang = [](const std::string& name, int read, int written, int err) {
        std::cout << "   │ " << std::left << std::setw(20) << name << " │ "
                  << std::right << std::setw(11) << read << " │ "
                  << std::setw(11) << written << " │ "
                  << std::setw(11) << err << " │\n";
    };

    inDongBang("Product.txt", statProduct.linesRead, statProduct.linesWritten, statProduct.errorCount);
    inDongBang("stockReceipts.txt", statStock.linesRead, statStock.linesWritten, statStock.errorCount);
    inDongBang("Invoices.txt", statInvoice.linesRead, statInvoice.linesWritten, statInvoice.errorCount);
    std::cout << "   ├──────────────────────┼─────────────┼─────────────┼─────────────┤\n";
    inDongBang("TỔNG CỘNG", tongDoc, tongGhi, tongLoi);
    std::cout << "   └──────────────────────┴─────────────┴─────────────┴─────────────┘\n\n";

    std::cout << "3. Báo cáo nhật ký lỗi: Đã lưu tại '" << pathLoi << "' (" << tongLoi << " lỗi).\n";
    std::cout << "4. Ghi chú nghiệp vụ:\n";
    std::cout << "   - Giá vốn (giaVon) của các hóa đơn cũ trong dòng D được lấy từ importPrice\n";
    std::cout << "     hiện tại của sản phẩm trong Products.txt (đây là giá vốn ước tính do hóa đơn\n";
    std::cout << "     cũ không lưu vết snapshot giá vốn tại thời điểm mua).\n";
    std::cout << "   - Thuế suất áp dụng cố định 8%, tiền thuế làm tròn về đơn vị đồng.\n";
    std::cout << "================================================================================\n";
    std::cout << "[HOÀN THÀNH] Quá trình chuyển đổi dữ liệu kết thúc thành công!\n";

    return 0;
}
