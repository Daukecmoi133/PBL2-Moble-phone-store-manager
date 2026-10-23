#ifndef INVENTORYSERVICE_H
#define INVENTORYSERVICE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include "../utils/FileStore.h"
#include "../utils/StringHelper.h"
#include "../ui/viewmodels/OperationResult.h"
#include "../ui/viewmodels/GoodsReceiptViewModel.h"
#include "../ui/viewmodels/ProductViewModel.h"
#include "../models/Phone.h"
#include "../models/ImportReceipt.h"

/**
 * @class InventoryService
 * @brief Dịch vụ quản lý tồn kho, phiếu nhập và nghiệp vụ sản phẩm theo mô hình FileStore.
 * 
 * QUY TẮC NGHIỆP VỤ:
 * 1. Tồn kho không lưu trong Product.txt. Tồn kho = Tổng nhập (HoatDong) - Tổng bán (HoatDong/DaThanhToan).
 * 2. Giá vốn hiện tại: Lấy donGia của phiếu nhập HoatDong có importTime mới nhất.
 *    So sánh chuỗi thời gian yyyy-MM-dd HH:mm:ss. Nếu không có phiếu nào, lấy importPrice trong Product.txt.
 * 3. Tạo phiếu nhập: Kiểm tra soLuong > 0, donGia > 0, NCC và sản phẩm tồn tại.
 *    Ghi file an toàn qua ghiFileAnToan, sau đó cập nhật cache giá vốn.
 * 4. Hủy phiếu nhập: Đổi status thành "DaHuy", không xóa dòng. Tự động tính lại tồn kho.
 *    Không cho hủy nếu làm tồn kho sản phẩm bị âm. Giá vốn quay về phiếu HoatDong trước đó.
 * 5. Xóa sản phẩm: Nếu đã có phiếu nhập hoặc hóa đơn (kể cả đã hủy) -> đổi status thành "NgungBan".
 *    Nếu chưa có giao dịch nào -> xóa dòng khỏi Product.txt.
 * 6. Thêm/sửa sản phẩm: Không cho sửa mã sản phẩm. Kiểm tra các trường không chứa '|'.
 */
class InventoryService {
private:
    std::string productsPath;
    std::string receiptsPath;
    std::string invoicesPath;
    std::string suppliersPath;

    // Cache giá vốn hiện tại trong bộ nhớ: maSP -> donGia
    std::map<std::string, double> giaVonCache;

    static std::string resolvePath(const std::string& preferred, const std::string& fallback) {
        std::error_code ec;
        if (std::filesystem::exists(preferred, ec)) return preferred;
        if (std::filesystem::exists(fallback, ec)) return fallback;
        return preferred;
    }

public:
    InventoryService(
        std::string pPath = "",
        std::string rPath = "",
        std::string iPath = "",
        std::string sPath = ""
    ) {
        productsPath  = !pPath.empty() ? pPath : resolvePath("data/Device/Products.txt", "data/Product.txt");
        receiptsPath  = !rPath.empty() ? rPath : resolvePath("data/Device/StockReceipts.txt", "data/stockReceipts.txt");
        invoicesPath  = !iPath.empty() ? iPath : resolvePath("data/PAY/Invoices.txt", "data/Invoices.txt");
        suppliersPath = !sPath.empty() ? sPath : resolvePath("data/people/Suppliers.txt", "data/Suppliers.txt");

        khoiTaoCache();
    }

    // =========================================================================
    // 1. TÍNH TOÁN TỒN KHO & GIÁ VỐN
    // =========================================================================

    /**
     * @brief Tính số lượng tồn kho của một sản phẩm.
     * Tồn kho = Tổng số lượng trong các phiếu nhập HoatDong - Tổng số lượng trong chi tiết hóa đơn HoatDong.
     */
    int tinhTonKho(const std::string& maSP) const {
        if (maSP.empty()) return 0;
        int tongNhap = 0;
        int tongXuat = 0;

        // 1. Tính tổng nhập từ stockReceipts.txt (chỉ phiếu có status HoatDong)
        auto receiptRows = FileStore::docFile(receiptsPath);
        for (size_t i = 0; i < receiptRows.size(); ++i) {
            const auto& row = receiptRows[i];
            if (row.empty() || row[0] == "id") continue; // Bỏ qua header
            if (row.size() < 5) continue;

            // Kiểm tra status (mặc định HoatDong nếu không có cột 5)
            std::string status = (row.size() >= 6) ? row[5] : "HoatDong";
            if (status != "HoatDong") continue;

            // Phân tích details: productId,quantity,donGia;...
            std::string detailStr = row[4];
            auto details = StringHelper::split(detailStr, ';');
            for (int j = 0; j < details.getSize(); ++j) {
                auto tokens = StringHelper::split(details[j], ',');
                if (tokens.getSize() == 3) {
                    if (tokens[0] == maSP) {
                        try { tongNhap += std::stoi(tokens[1]); } catch (...) {}
                    }
                } else if (tokens.getSize() >= 4) { // Dạng cũ: productId,phoneName,quantity,donGia
                    if (tokens[0] == maSP) {
                        try { tongNhap += std::stoi(tokens[2]); } catch (...) {}
                    }
                }
            }
        }

        // 2. Tính tổng xuất từ Invoices.txt (chỉ hóa đơn không bị hủy)
        auto invoiceRows = FileStore::docFile(invoicesPath);
        std::map<std::string, bool> validInvoices;
        for (const auto& row : invoiceRows) {
            if (row.empty()) continue;
            if (row[0] == "H" && row.size() >= 2) {
                std::string invId = row[1];
                std::string status = (row.size() >= 13) ? row[12] : "HoatDong";
                validInvoices[invId] = (status != "DaHuy");
            }
        }

        for (const auto& row : invoiceRows) {
            if (row.empty()) continue;
            if (row[0] == "D" && row.size() >= 4) {
                std::string invId = row[1];
                std::string pId = row[2];
                if (pId == maSP && validInvoices[invId]) {
                    try { tongXuat += std::stoi(row[3]); } catch (...) {}
                }
            }
        }

        return tongNhap - tongXuat;
    }

    /**
     * @brief Kiểm tra xem kho có đủ số lượng sản phẩm yêu cầu không.
     */
    bool kiemTraDuTonKho(const std::string& maSP, int soLuong) const {
        if (soLuong <= 0) return false;
        return tinhTonKho(maSP) >= soLuong;
    }

    /**
     * @brief Lấy giá vốn hiện tại (giá nhập gần nhất) của một sản phẩm.
     * Đơn giá của phiếu nhập HoatDong có importTime mới nhất chứa sản phẩm đó.
     * So sánh theo thời gian (chuỗi yyyy-MM-dd HH:mm:ss). Nếu không có, lấy importPrice trong Product.txt.
     */
    double giaVonHienTai(const std::string& maSP) {
        if (maSP.empty()) return 0.0;

        auto it = giaVonCache.find(maSP);
        if (it != giaVonCache.end()) {
            return it->second;
        }

        double val = tinhGiaVonTuFile(maSP);
        giaVonCache[maSP] = val;
        return val;
    }

    /**
     * @brief Nạp lại hoặc khởi tạo cache giá vốn cho tất cả sản phẩm.
     */
    void khoiTaoCache() {
        giaVonCache.clear();
        auto prodRows = FileStore::docFile(productsPath);
        for (const auto& row : prodRows) {
            if (row.empty() || row[0] == "id") continue;
            std::string id = row[0];
            giaVonCache[id] = tinhGiaVonTuFile(id);
        }
    }

private:
    double tinhGiaVonTuFile(const std::string& maSP) const {
        std::string latestTime = "";
        double latestPrice = -1.0;

        auto receiptRows = FileStore::docFile(receiptsPath);
        for (const auto& row : receiptRows) {
            if (row.empty() || row[0] == "id") continue;
            if (row.size() < 5) continue;

            std::string status = (row.size() >= 6) ? row[5] : "HoatDong";
            if (status != "HoatDong") continue;

            std::string impTime = row[3];
            std::string detailStr = row[4];
            auto details = StringHelper::split(detailStr, ';');
            for (int j = 0; j < details.getSize(); ++j) {
                auto tokens = StringHelper::split(details[j], ',');
                if (tokens.getSize() == 3 && tokens[0] == maSP) {
                    double pr = StringHelper::parsePrice(tokens[2]);
                    if (impTime > latestTime) {
                        latestTime = impTime;
                        latestPrice = pr;
                    }
                } else if (tokens.getSize() >= 4 && tokens[0] == maSP) {
                    double pr = StringHelper::parsePrice(tokens[3]);
                    if (impTime > latestTime) {
                        latestTime = impTime;
                        latestPrice = pr;
                    }
                }
            }
        }

        if (latestPrice >= 0.0) {
            return latestPrice;
        }

        // Nếu không có phiếu nhập nào: lấy importPrice từ Products.txt
        auto prodRows = FileStore::docFile(productsPath);
        for (const auto& row : prodRows) {
            if (row.empty() || row[0] == "id") continue;
            if (row[0] == maSP && row.size() >= 5) {
                return StringHelper::parsePrice(row[4]);
            }
        }

        return 0.0;
    }

public:
    // =========================================================================
    // 2. NGHIỆP VỤ PHIẾU NHẬP (GOODS RECEIPTS)
    // =========================================================================

    struct ItemInput {
        std::string productId;
        std::string productName;
        int quantity;
        double unitPrice;
    };

    /**
     * @brief Tạo phiếu nhập mới theo Quy tắc 3.
     */
    OperationResult taoPhieuNhap(
        const std::string& receiptId,
        const std::string& supplierId,
        const std::string& employeeId,
        const std::string& importTime,
        const std::vector<ItemInput>& items
    ) {
        if (receiptId.empty()) return OperationResult::Fail("Mã phiếu nhập không được để trống!");
        if (supplierId.empty()) return OperationResult::Fail("Chưa chọn nhà cung cấp!");
        if (items.empty()) return OperationResult::Fail("Danh sách mặt hàng nhập không được để trống!");

        // 1. Kiểm tra nhà cung cấp tồn tại
        bool supExists = false;
        auto supRows = FileStore::docFile(suppliersPath);
        for (const auto& row : supRows) {
            if (!row.empty() && row[0] == supplierId) {
                supExists = true;
                break;
            }
        }
        if (!supExists && !supRows.empty()) {
            return OperationResult::Fail("Nhà cung cấp [" + supplierId + "] không tồn tại trong hệ thống!");
        }

        // 2. Kiểm tra từng mặt hàng
        auto prodRows = FileStore::docFile(productsPath);
        for (const auto& item : items) {
            if (item.quantity <= 0) {
                return OperationResult::Fail("Số lượng nhập của sản phẩm [" + item.productId + "] phải lớn hơn 0!");
            }
            if (item.unitPrice <= 0) {
                return OperationResult::Fail("Đơn giá nhập của sản phẩm [" + item.productId + "] phải lớn hơn 0!");
            }
            if (!FileStore::kiemTraTruong(item.productId)) {
                return OperationResult::Fail("Mã sản phẩm không được chứa ký tự '|'!");
            }

            bool pExists = false;
            for (const auto& prow : prodRows) {
                if (!prow.empty() && prow[0] == item.productId) {
                    pExists = true;
                    break;
                }
            }
            if (!pExists) {
                return OperationResult::Fail("Sản phẩm [" + item.productId + "] không tồn tại trong cơ sở dữ liệu!");
            }
        }

        // 3. Chuẩn bị chuỗi chi tiết: productId,quantity,donGia;...
        std::string detailStr;
        for (size_t i = 0; i < items.size(); ++i) {
            long long prLL = static_cast<long long>(std::round(items[i].unitPrice));
            detailStr += items[i].productId + "," + std::to_string(items[i].quantity) + "," + std::to_string(prLL);
            if (i < items.size() - 1) detailStr += ";";
        }

        // Chuẩn bị dòng phiếu nhập mới: id|supplierId|employeeId|importTime|details|status
        std::vector<std::string> rowTokens = {
            receiptId,
            supplierId,
            employeeId,
            importTime,
            detailStr,
            "HoatDong"
        };
        std::string newLine = FileStore::ghepTruong(rowTokens);

        // 4. Đọc file hiện tại, thêm dòng mới và ghi file an toàn
        auto allRows = FileStore::docFile(receiptsPath);
        std::vector<std::string> lines;
        lines.push_back("id|supplierId|employeeId|importTime|details|status");

        bool exists = false;
        for (size_t i = 0; i < allRows.size(); ++i) {
            if (i == 0 && !allRows[i].empty() && allRows[i][0] == "id") continue;
            if (!allRows[i].empty() && allRows[i][0] == receiptId) {
                exists = true;
                break;
            }
            lines.push_back(FileStore::ghepTruong(allRows[i]));
        }

        if (exists) {
            return OperationResult::Fail("Mã phiếu nhập [" + receiptId + "] đã tồn tại!");
        }

        lines.push_back(newLine);

        bool saveOk = FileStore::ghiFileAnToan(receiptsPath, lines);
        if (!saveOk) {
            return OperationResult::Fail("Lỗi khi ghi dữ liệu phiếu nhập vào file!");
        }

        // 5. Ngay sau khi ghi thành công, cập nhật cache giá vốn trong bộ nhớ
        for (const auto& item : items) {
            giaVonCache[item.productId] = item.unitPrice;
        }

        return OperationResult::Ok("Lập phiếu nhập kho [" + receiptId + "] thành công!");
    }

    /**
     * @brief Hủy phiếu nhập theo Quy tắc 4.
     * Đổi status thành "DaHuy", không xóa dòng.
     * Không được hủy nếu làm tồn kho của sản phẩm thành âm.
     * Giá vốn tự động quay về phiếu HoatDong trước đó.
     */
    OperationResult huyPhieuNhap(const std::string& maPN) {
        if (maPN.empty()) return OperationResult::Fail("Mã phiếu nhập không hợp lệ!");

        auto receiptRows = FileStore::docFile(receiptsPath);
        if (receiptRows.empty()) return OperationResult::Fail("Dữ liệu phiếu nhập rỗng!");

        int targetIndex = -1;
        std::vector<std::pair<std::string, int>> itemsInReceipt;

        for (size_t i = 0; i < receiptRows.size(); ++i) {
            if (i == 0 && !receiptRows[i].empty() && receiptRows[i][0] == "id") continue;
            if (!receiptRows[i].empty() && receiptRows[i][0] == maPN) {
                targetIndex = static_cast<int>(i);
                std::string status = (receiptRows[i].size() >= 6) ? receiptRows[i][5] : "HoatDong";
                if (status == "DaHuy") {
                    return OperationResult::Fail("Phiếu nhập [" + maPN + "] đã được hủy trước đó!");
                }

                // Parse details để lấy các sản phẩm và số lượng
                if (receiptRows[i].size() >= 5) {
                    auto details = StringHelper::split(receiptRows[i][4], ';');
                    for (int j = 0; j < details.getSize(); ++j) {
                        auto tokens = StringHelper::split(details[j], ',');
                        if (tokens.getSize() == 3) {
                            try { itemsInReceipt.push_back({tokens[0], std::stoi(tokens[1])}); } catch (...) {}
                        } else if (tokens.getSize() >= 4) {
                            try { itemsInReceipt.push_back({tokens[0], std::stoi(tokens[2])}); } catch (...) {}
                        }
                    }
                }
                break;
            }
        }

        if (targetIndex < 0) {
            return OperationResult::Fail("Không tìm thấy phiếu nhập [" + maPN + "]!");
        }

        // KIỂM TRA TỒN KHO: Không được hủy nếu làm tồn kho của sản phẩm thành âm
        for (const auto& item : itemsInReceipt) {
            int currentStock = tinhTonKho(item.first);
            if (currentStock - item.second < 0) {
                return OperationResult::Fail(
                    "Không thể hủy phiếu nhập vì sẽ làm tồn kho của sản phẩm [" + item.first +
                    "] bị âm (Hiện có: " + std::to_string(currentStock) + ", trong phiếu: " +
                    std::to_string(item.second) + ", còn lại: " +
                    std::to_string(currentStock - item.second) + ")!"
                );
            }
        }

        // Cập nhật status thành "DaHuy"
        if (receiptRows[targetIndex].size() >= 6) {
            receiptRows[targetIndex][5] = "DaHuy";
        } else {
            while (receiptRows[targetIndex].size() < 5) receiptRows[targetIndex].push_back("");
            receiptRows[targetIndex].push_back("DaHuy");
        }

        // Ghi lại toàn bộ file bằng ghiFileAnToan
        std::vector<std::string> lines;
        lines.push_back("id|supplierId|employeeId|importTime|details|status");
        for (size_t i = 0; i < receiptRows.size(); ++i) {
            if (i == 0 && !receiptRows[i].empty() && receiptRows[i][0] == "id") continue;
            lines.push_back(FileStore::ghepTruong(receiptRows[i]));
        }

        bool saveOk = FileStore::ghiFileAnToan(receiptsPath, lines);
        if (!saveOk) {
            return OperationResult::Fail("Lỗi khi cập nhật trạng thái phiếu nhập vào file!");
        }

        // Cập nhật lại cache giá vốn trong bộ nhớ: quay về phiếu HoatDong trước đó
        for (const auto& item : itemsInReceipt) {
            giaVonCache.erase(item.first);
            giaVonHienTai(item.first);
        }

        return OperationResult::Ok("Đã hủy phiếu nhập [" + maPN + "] thành công! Tồn kho và giá vốn đã được cập nhật.");
    }

    /**
     * @brief Đọc danh sách tất cả phiếu nhập kho dưới dạng ViewModel
     */
    std::vector<GoodsReceiptSummaryViewModel> layDanhSachPhieuNhap() const {
        std::vector<GoodsReceiptSummaryViewModel> result;
        auto receiptRows = FileStore::docFile(receiptsPath);
        auto supRows = FileStore::docFile(suppliersPath);

        std::map<std::string, std::string> supMap;
        for (const auto& srow : supRows) {
            if (!srow.empty() && srow.size() >= 2) {
                supMap[srow[0]] = srow[1];
            }
        }

        // Đọc tên sản phẩm từ Products.txt để hiển thị trong detail
        auto prodRows = FileStore::docFile(productsPath);
        std::map<std::string, std::string> prodNames;
        for (const auto& prow : prodRows) {
            if (!prow.empty() && prow.size() >= 2) {
                prodNames[prow[0]] = prow[1];
            }
        }

        for (size_t i = 0; i < receiptRows.size(); ++i) {
            const auto& row = receiptRows[i];
            if (row.empty() || row[0] == "id") continue;
            if (row.size() < 5) continue;

            GoodsReceiptSummaryViewModel r;
            r.id = row[0];
            r.supplierId = row[1];
            r.supplierName = supMap.count(r.supplierId) ? supMap[r.supplierId] : r.supplierId;
            r.employeeId = row[2];
            r.employeeName = r.employeeId;
            r.importTime = row[3];
            r.status = (row.size() >= 6) ? row[5] : "HoatDong";
            r.totalAmount = 0.0;

            auto details = StringHelper::split(row[4], ';');
            for (int j = 0; j < details.getSize(); ++j) {
                auto tokens = StringHelper::split(details[j], ',');
                GoodsReceiptDetailViewModel d;
                if (tokens.getSize() == 3) {
                    d.phoneId = tokens[0];
                    d.phoneName = prodNames.count(d.phoneId) ? prodNames[d.phoneId] : d.phoneId;
                    try { d.quantity = std::stoi(tokens[1]); } catch (...) {}
                    d.importPrice = StringHelper::parsePrice(tokens[2]);
                    d.subtotal = d.quantity * d.importPrice;
                    r.totalAmount += d.subtotal;
                    r.details.push_back(d);
                } else if (tokens.getSize() >= 4) {
                    d.phoneId = tokens[0];
                    d.phoneName = tokens[1];
                    try { d.quantity = std::stoi(tokens[2]); } catch (...) {}
                    d.importPrice = StringHelper::parsePrice(tokens[3]);
                    d.subtotal = d.quantity * d.importPrice;
                    r.totalAmount += d.subtotal;
                    r.details.push_back(d);
                }
            }

            result.push_back(r);
        }

        // Đảo ngược để phiếu nhập mới nhất ở trên đầu
        std::reverse(result.begin(), result.end());
        return result;
    }

    // =========================================================================
    // 3. NGHIỆP VỤ SẢN PHẨM (PRODUCTS) THEO QUY TẮC 5 & 6
    // =========================================================================

    /**
     * @brief Kiểm tra xem sản phẩm đã từng có giao dịch nào chưa (phiếu nhập hoặc hóa đơn, kể cả đã hủy).
     */
    bool daTungCoGiaoDich(const std::string& maSP) const {
        if (maSP.empty()) return false;

        // 1. Kiểm tra trong StockReceipts.txt
        auto receiptRows = FileStore::docFile(receiptsPath);
        for (const auto& row : receiptRows) {
            if (row.empty() || row[0] == "id" || row.size() < 5) continue;
            auto details = StringHelper::split(row[4], ';');
            for (int j = 0; j < details.getSize(); ++j) {
                auto tokens = StringHelper::split(details[j], ',');
                if (tokens.getSize() >= 1 && tokens[0] == maSP) {
                    return true;
                }
            }
        }

        // 2. Kiểm tra trong Invoices.txt (dòng D)
        auto invoiceRows = FileStore::docFile(invoicesPath);
        for (const auto& row : invoiceRows) {
            if (row.empty()) continue;
            if (row[0] == "D" && row.size() >= 3 && row[2] == maSP) {
                return true;
            }
        }

        return false;
    }

    /**
     * @brief Xóa sản phẩm theo Quy tắc 5.
     * - Nếu sản phẩm đã có phiếu nhập hoặc hóa đơn (kể cả đã hủy): Đổi status thành "NgungBan".
     * - Nếu chưa có giao dịch nào: Xóa dòng khỏi Product.txt.
     */
    OperationResult xoaSanPham(const std::string& maSP) {
        if (maSP.empty()) return OperationResult::Fail("Mã sản phẩm không hợp lệ!");

        auto prodRows = FileStore::docFile(productsPath);
        int targetIdx = -1;
        for (size_t i = 0; i < prodRows.size(); ++i) {
            if (i == 0 && !prodRows[i].empty() && prodRows[i][0] == "id") continue;
            if (!prodRows[i].empty() && prodRows[i][0] == maSP) {
                targetIdx = static_cast<int>(i);
                break;
            }
        }

        if (targetIdx < 0) {
            return OperationResult::Fail("Không tìm thấy sản phẩm [" + maSP + "] để xóa!");
        }

        bool hasTrans = daTungCoGiaoDich(maSP);

        std::vector<std::string> lines;
        lines.push_back("id|phoneName|brand|operatingSystem|importPrice|sellingPrice|manufacturer|specifications|status");

        if (hasTrans) {
            // Đã có giao dịch: chỉ chuyển status thành "NgungBan"
            for (size_t i = 0; i < prodRows.size(); ++i) {
                if (i == 0 && !prodRows[i].empty() && prodRows[i][0] == "id") continue;
                if (static_cast<int>(i) == targetIdx) {
                    while (prodRows[i].size() < 9) prodRows[i].push_back("");
                    prodRows[i][8] = "NgungBan";
                }
                lines.push_back(FileStore::ghepTruong(prodRows[i]));
            }
            bool ok = FileStore::ghiFileAnToan(productsPath, lines);
            if (!ok) return OperationResult::Fail("Lỗi khi cập nhật trạng thái ngừng bán vào file!");
            return OperationResult::Ok("Sản phẩm [" + maSP + "] đã phát sinh giao dịch nhập/bán, hệ thống đã chuyển trạng thái sang 'Ngừng bán'!");
        } else {
            // Chưa từng có giao dịch: xóa hẳn dòng
            for (size_t i = 0; i < prodRows.size(); ++i) {
                if (i == 0 && !prodRows[i].empty() && prodRows[i][0] == "id") continue;
                if (static_cast<int>(i) == targetIdx) continue; // Bỏ qua dòng bị xóa
                lines.push_back(FileStore::ghepTruong(prodRows[i]));
            }
            bool ok = FileStore::ghiFileAnToan(productsPath, lines);
            if (!ok) return OperationResult::Fail("Lỗi khi xóa dòng sản phẩm khỏi file!");
            giaVonCache.erase(maSP);
            return OperationResult::Ok("Sản phẩm [" + maSP + "] chưa từng phát sinh giao dịch, đã xóa hoàn toàn khỏi cơ sở dữ liệu!");
        }
    }

    /**
     * @brief Thêm sản phẩm mới theo Quy tắc 6.
     * Kiểm tra các trường không chứa '|'. Không trùng ID.
     */
    OperationResult themSanPham(const ProductItemViewModel& p) {
        if (p.id.empty()) return OperationResult::Fail("Mã sản phẩm không được để trống!");
        if (p.name.empty()) return OperationResult::Fail("Tên sản phẩm không được để trống!");
        if (p.sellingPrice <= 0) return OperationResult::Fail("Giá bán phải lớn hơn 0!");

        // Kiểm tra không chứa ký tự '|'
        if (!FileStore::kiemTraTruong(p.id) ||
            !FileStore::kiemTraTruong(p.name) ||
            !FileStore::kiemTraTruong(p.brand) ||
            !FileStore::kiemTraTruong(p.os) ||
            !FileStore::kiemTraTruong(p.manufacturer) ||
            !FileStore::kiemTraTruong(p.specs)) {
            return OperationResult::Fail("Các trường thông tin không được chứa ký tự ngăn cách '|'!");
        }

        auto prodRows = FileStore::docFile(productsPath);
        std::vector<std::string> lines;
        lines.push_back("id|phoneName|brand|operatingSystem|importPrice|sellingPrice|manufacturer|specifications|status");

        for (size_t i = 0; i < prodRows.size(); ++i) {
            if (i == 0 && !prodRows[i].empty() && prodRows[i][0] == "id") continue;
            if (!prodRows[i].empty() && prodRows[i][0] == p.id) {
                return OperationResult::Fail("Mã sản phẩm [" + p.id + "] đã tồn tại!");
            }
            lines.push_back(FileStore::ghepTruong(prodRows[i]));
        }

        long long impLL = static_cast<long long>(std::round(p.importPrice));
        long long sellLL = static_cast<long long>(std::round(p.sellingPrice));

        std::vector<std::string> newRow = {
            p.id,
            p.name,
            p.brand,
            p.os,
            std::to_string(impLL),
            std::to_string(sellLL),
            p.manufacturer,
            p.specs,
            "DangBan"
        };
        lines.push_back(FileStore::ghepTruong(newRow));

        bool ok = FileStore::ghiFileAnToan(productsPath, lines);
        if (!ok) return OperationResult::Fail("Lỗi khi ghi sản phẩm mới vào file!");

        giaVonCache[p.id] = p.importPrice;
        return OperationResult::Ok("Thêm sản phẩm mới [" + p.id + "] thành công!");
    }

    /**
     * @brief Sửa thông tin sản phẩm theo Quy tắc 6.
     * Không cho sửa mã sản phẩm. Kiểm tra các trường không chứa '|'.
     */
    OperationResult suaSanPham(const ProductItemViewModel& p) {
        if (p.id.empty()) return OperationResult::Fail("Mã sản phẩm không được để trống!");
        if (p.name.empty()) return OperationResult::Fail("Tên sản phẩm không được để trống!");
        if (p.sellingPrice <= 0) return OperationResult::Fail("Giá bán phải lớn hơn 0!");

        if (!FileStore::kiemTraTruong(p.name) ||
            !FileStore::kiemTraTruong(p.brand) ||
            !FileStore::kiemTraTruong(p.os) ||
            !FileStore::kiemTraTruong(p.manufacturer) ||
            !FileStore::kiemTraTruong(p.specs)) {
            return OperationResult::Fail("Các trường thông tin không được chứa ký tự ngăn cách '|'!");
        }

        auto prodRows = FileStore::docFile(productsPath);
        std::vector<std::string> lines;
        lines.push_back("id|phoneName|brand|operatingSystem|importPrice|sellingPrice|manufacturer|specifications|status");

        bool found = false;
        for (size_t i = 0; i < prodRows.size(); ++i) {
            if (i == 0 && !prodRows[i].empty() && prodRows[i][0] == "id") continue;
            if (!prodRows[i].empty() && prodRows[i][0] == p.id) {
                found = true;
                std::string status = (prodRows[i].size() >= 9) ? prodRows[i][8] : "DangBan";
                std::string curImpPrice = (prodRows[i].size() >= 5) ? prodRows[i][4] : std::to_string(static_cast<long long>(p.importPrice));
                long long sellLL = static_cast<long long>(std::round(p.sellingPrice));

                std::vector<std::string> updatedRow = {
                    p.id, // Không đổi mã
                    p.name,
                    p.brand,
                    p.os,
                    curImpPrice,
                    std::to_string(sellLL),
                    p.manufacturer,
                    p.specs,
                    status
                };
                lines.push_back(FileStore::ghepTruong(updatedRow));
            } else {
                lines.push_back(FileStore::ghepTruong(prodRows[i]));
            }
        }

        if (!found) {
            return OperationResult::Fail("Không tìm thấy sản phẩm [" + p.id + "] để cập nhật!");
        }

        bool ok = FileStore::ghiFileAnToan(productsPath, lines);
        if (!ok) return OperationResult::Fail("Lỗi khi cập nhật sản phẩm vào file!");

        return OperationResult::Ok("Cập nhật thông tin sản phẩm [" + p.id + "] thành công!");
    }

    /**
     * @brief Đọc danh sách tất cả sản phẩm kèm tồn kho tính toán và giá vốn hiện tại.
     */
    std::vector<ProductItemViewModel> layDanhSachSanPham() {
        std::vector<ProductItemViewModel> list;
        auto prodRows = FileStore::docFile(productsPath);

        for (size_t i = 0; i < prodRows.size(); ++i) {
            const auto& row = prodRows[i];
            if (row.empty() || row[0] == "id") continue;
            if (row.size() < 8) continue;

            ProductItemViewModel item;
            item.id = row[0];
            item.name = row[1];
            item.brand = row[2];
            item.os = row[3];
            item.sellingPrice = StringHelper::parsePrice(row[5]);
            item.manufacturer = (row.size() >= 7) ? row[6] : "";
            item.specs = (row.size() >= 8) ? row[7] : "";
            
            std::string st = (row.size() >= 9) ? row[8] : "DangBan";

            // 1. Giá vốn hiện tại: Lấy từ cache hoặc phiếu nhập mới nhất
            item.importPrice = giaVonHienTai(item.id);

            // 2. Tồn kho tính toán: Không đọc cột quantity cũ
            item.actualStock = tinhTonKho(item.id);
            item.reservedStock = 0; // Sẽ được cập nhật thêm từ các đơn hàng đang chờ fulfillment nếu có
            item.availableStock = item.actualStock;

            if (st == "NgungBan") {
                item.status = StockDisplayStatus::OUT_OF_STOCK;
            } else {
                item.calculateStockStatus();
            }

            list.push_back(item);
        }

        return list;
    }
};

#endif // INVENTORYSERVICE_H