#ifndef EXPORT_HELPER_H
#define EXPORT_HELPER_H

#include "FileHelper.h"
#include "PricingCalculator.h"
#include "StringHelper.h"
#include "../../UI/viewmodels/AccountViewModel.h"
#include "../../UI/viewmodels/CustomerViewModel.h"
#include "../../UI/viewmodels/DiscountViewModel.h"
#include "../../UI/viewmodels/EmployeeViewModel.h"
#include "../../UI/viewmodels/GoodsReceiptViewModel.h"
#include "../../UI/viewmodels/InvoiceViewModel.h"
#include "../../UI/viewmodels/OrderViewModel.h"
#include "../../UI/viewmodels/ProductViewModel.h"
#include "../../UI/viewmodels/StatisticsViewModel.h"
#include "../../UI/viewmodels/SupplierViewModel.h"
#include "../../UI/viewmodels/WarrantyViewModel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

/**
 * @file ExportHelper.h
 * @brief Tiện ích định dạng và xuất dữ liệu sang file .CSV (Excel UTF-8 BOM)
 *        và xuất các loại hóa đơn / phiếu chứng từ sang file .TXT (chuẩn 80 ký tự UTF-8).
 */
namespace ExportHelper {

struct ExportContext {
    std::string storeName = "D&N PHONE STORE";
    std::string storeSubtitle = "HỆ THỐNG BÁN LẺ ĐIỆN THOẠI CHÍNH HÃNG";
    std::string storeAddress = "54 Nguyễn Lương Bằng, Liên Chiểu, TP. Đà Nẵng";
    std::string storeHotline = "1900 6868";
    std::string storeTaxCode = "0401234567";
    std::string exportedAt;
    std::string exportedBy;
    std::string filterSummary = "Tất cả";
    std::string shippingPolicy = "Miễn phí vận chuyển hỏa tốc & Cho phép đồng kiểm trước khi nhận hàng";
    std::string returnPolicy = "1 đổi 1 trong vòng 30 ngày nếu có lỗi từ nhà sản xuất";
    std::string warrantyPolicy = "Bảo hành chính hãng 12 tháng tại các trung tâm bảo hành ủy quyền";
};

// ============================================================================
// 1. TIỆN ÍCH CHUỖI UTF-8 & ĐỊNH DẠNG SỐ TIỀN (CĂN LỀ CHUẨN TIẾNG VIỆT)
// ============================================================================

inline size_t utf8DisplayWidth(const std::string& s) {
    size_t count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

inline std::string utf8SubstrByCodepoints(const std::string& s, size_t maxCp) {
    size_t cp = 0;
    size_t i = 0;
    const size_t n = s.size();
    while (i < n && cp < maxCp) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t charLen = 1;
        if ((c & 0x80) == 0) charLen = 1;
        else if ((c & 0xE0) == 0xC0) charLen = 2;
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        else if ((c & 0xF8) == 0xF0) charLen = 4;
        if (i + charLen > n) break;
        i += charLen;
        ++cp;
    }
    return s.substr(0, i);
}

inline std::string truncateUtf8(const std::string& s, size_t maxWidth) {
    if (utf8DisplayWidth(s) <= maxWidth) return s;
    if (maxWidth <= 3) return utf8SubstrByCodepoints(s, maxWidth);
    return utf8SubstrByCodepoints(s, maxWidth - 3) + "...";
}

inline std::string padRightUtf8(const std::string& s, size_t width) {
    std::string t = truncateUtf8(s, width);
    size_t w = utf8DisplayWidth(t);
    if (w >= width) return t;
    return t + std::string(width - w, ' ');
}

inline std::string padLeftUtf8(const std::string& s, size_t width) {
    std::string t = truncateUtf8(s, width);
    size_t w = utf8DisplayWidth(t);
    if (w >= width) return t;
    return std::string(width - w, ' ') + t;
}

inline std::string centerUtf8(const std::string& s, size_t width) {
    std::string t = truncateUtf8(s, width);
    size_t w = utf8DisplayWidth(t);
    if (w >= width) return t;
    size_t left = (width - w) / 2;
    size_t right = width - w - left;
    return std::string(left, ' ') + t + std::string(right, ' ');
}

inline std::vector<std::string> wrapUtf8(const std::string& text, size_t maxWidth) {
    std::vector<std::string> lines;
    if (maxWidth == 0) {
        lines.push_back(text);
        return lines;
    }
    std::string clean = text;
    std::replace(clean.begin(), clean.end(), '\r', ' ');
    std::istringstream paraStream(clean);
    std::string paragraph;
    while (std::getline(paraStream, paragraph, '\n')) {
        std::istringstream wordStream(paragraph);
        std::string word;
        std::string currentLine;
        while (wordStream >> word) {
            if (currentLine.empty()) {
                if (utf8DisplayWidth(word) <= maxWidth) {
                    currentLine = word;
                } else {
                    // Cắt từ quá dài an toàn theo codepoint
                    std::string rem = word;
                    while (utf8DisplayWidth(rem) > maxWidth) {
                        std::string chunk = utf8SubstrByCodepoints(rem, maxWidth);
                        lines.push_back(chunk);
                        rem = rem.substr(chunk.size());
                    }
                    currentLine = rem;
                }
            } else {
                if (utf8DisplayWidth(currentLine) + 1 + utf8DisplayWidth(word) <= maxWidth) {
                    currentLine += " " + word;
                } else {
                    lines.push_back(currentLine);
                    if (utf8DisplayWidth(word) <= maxWidth) {
                        currentLine = word;
                    } else {
                        std::string rem = word;
                        while (utf8DisplayWidth(rem) > maxWidth) {
                            std::string chunk = utf8SubstrByCodepoints(rem, maxWidth);
                            lines.push_back(chunk);
                            rem = rem.substr(chunk.size());
                        }
                        currentLine = rem;
                    }
                }
            }
        }
        if (!currentLine.empty() || lines.empty()) {
            lines.push_back(currentLine);
        }
    }
    if (lines.empty()) lines.push_back("");
    return lines;
}

inline std::string formatIntegerDots(long long value) {
    bool neg = value < 0;
    unsigned long long absVal = neg ? static_cast<unsigned long long>(-value)
                                    : static_cast<unsigned long long>(value);
    std::string raw = std::to_string(absVal);
    std::string res;
    int cnt = 0;
    for (int i = static_cast<int>(raw.size()) - 1; i >= 0; --i) {
        if (cnt > 0 && cnt % 3 == 0) res.push_back('.');
        res.push_back(raw[i]);
        ++cnt;
    }
    if (neg) res.push_back('-');
    std::reverse(res.begin(), res.end());
    return res;
}

inline std::string formatVNDPlain(double amount) {
    return formatIntegerDots(static_cast<long long>(std::llround(amount)));
}

inline std::string formatVNDCurrency(double amount) {
    return formatVNDPlain(amount) + " đ";
}

inline std::string formatNumericCsv(double amount) {
    return std::to_string(static_cast<long long>(std::llround(amount)));
}

inline std::string formatPercent(double pct) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f%%", pct);
    return std::string(buf);
}

inline std::string csvEscape(const std::string& field) {
    bool needQuotes = false;
    for (char c : field) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needQuotes = true;
            break;
        }
    }
    if (!needQuotes) return field;
    std::string out;
    out.reserve(field.size() + 4);
    out.push_back('"');
    for (char c : field) {
        if (c == '"') out.append("\"\"");
        else out.push_back(c);
    }
    out.push_back('"');
    return out;
}

inline void writeCsvRow(std::ostream& out, const std::vector<std::string>& cols) {
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i > 0) out << ',';
        out << csvEscape(cols[i]);
    }
    out << "\r\n";
}

inline void writeCsvHeaderMeta(std::ostream& out,
                               const std::string& reportTitle,
                               size_t recordCount,
                               const ExportContext& ctx) {
    // UTF-8 BOM để Microsoft Excel nhận diện tiếng Việt chuẩn xác
    out << "\xEF\xBB\xBF";
    writeCsvRow(out, {ctx.storeName + " - " + ctx.storeSubtitle});
    writeCsvRow(out, {"TIÊU ĐỀ", reportTitle});
    writeCsvRow(out, {"Thời gian xuất", ctx.exportedAt});
    writeCsvRow(out, {"Người xuất", ctx.exportedBy});
    writeCsvRow(out, {"Phạm vi / Bộ lọc", ctx.filterSummary});
    writeCsvRow(out, {"Tổng số bản ghi", std::to_string(recordCount)});
    writeCsvRow(out, {});
}

inline std::string stockStatusToVietnamese(StockDisplayStatus st) {
    switch (st) {
        case StockDisplayStatus::IN_STOCK: return "Còn hàng";
        case StockDisplayStatus::LOW_STOCK: return "Sắp hết hàng";
        case StockDisplayStatus::OUT_OF_STOCK: return "Hết hàng";
        default: return "Không rõ";
    }
}

inline std::string ensureParentDirAndExtension(const std::string& filePath,
                                               const std::string& requiredExt) {
    std::string path = StringHelper::trim(filePath);
    if (path.empty()) return "";
    if (!requiredExt.empty()) {
        std::string lowerPath = StringHelper::toLower(path);
        std::string lowerExt = StringHelper::toLower(requiredExt);
        if (lowerPath.size() < lowerExt.size() ||
            lowerPath.compare(lowerPath.size() - lowerExt.size(), lowerExt.size(), lowerExt) != 0) {
            path += requiredExt;
        }
    }
    std::error_code ec;
    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path(), ec);
    }
    return path;
}

inline std::string makeSafeDateToken(const std::string& dateStr) {
    std::string out;
    for (char c : dateStr) {
        if (c >= '0' && c <= '9') out.push_back(c);
    }
    return out.empty() ? "export" : out;
}

// ============================================================================
// 2. XUẤT DANH SÁCH & BÁO CÁO DOANH THU SANG FILE .CSV
// ============================================================================

inline bool exportCustomersToCsv(const std::string& filePath,
                                 const std::vector<CustomerItemViewModel>& customers,
                                 const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH KHÁCH HÀNG", customers.size(), ctx);
        writeCsvRow(out, {"STT", "Mã khách hàng", "Họ và tên", "Số điện thoại",
                          "Email", "Tên đăng nhập", "Điểm tích lũy", "Hạng thành viên", "Địa chỉ"});
        long long totalPoints = 0;
        for (size_t i = 0; i < customers.size(); ++i) {
            const auto& c = customers[i];
            totalPoints += c.points;
            writeCsvRow(out, {
                std::to_string(i + 1), c.id, c.fullName, c.phone,
                c.email, c.username, std::to_string(c.points), c.membershipTier, c.address
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số KH: " + std::to_string(customers.size()),
                          "", "", "", "", std::to_string(totalPoints), "", ""});
        return true;
    });
}

inline bool exportEmployeesToCsv(const std::string& filePath,
                                 const std::vector<EmployeeItemViewModel>& employees,
                                 const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH NHÂN VIÊN", employees.size(), ctx);
        writeCsvRow(out, {"STT", "Mã nhân viên", "Họ và tên", "Số điện thoại",
                          "Email", "Tên đăng nhập", "Vai trò / Chức vụ",
                          "Lương cơ bản (VNĐ)", "Trạng thái", "Địa chỉ"});
        double totalSalary = 0.0;
        int activeCount = 0;
        for (size_t i = 0; i < employees.size(); ++i) {
            const auto& e = employees[i];
            totalSalary += e.salary;
            if (e.isActive) ++activeCount;
            writeCsvRow(out, {
                std::to_string(i + 1), e.id, e.fullName, e.phone, e.email,
                e.username, e.roleName, formatNumericCsv(e.salary),
                e.isActive ? "Đang hoạt động" : "Đã khóa", e.address
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số NV: " + std::to_string(employees.size()),
                          "", "", "", "", "Đang hoạt động: " + std::to_string(activeCount),
                          formatNumericCsv(totalSalary), "", ""});
        return true;
    });
}

inline bool exportProductsToCsv(const std::string& filePath,
                                const std::vector<ProductItemViewModel>& products,
                                const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH SẢN PHẨM & TỒN KHO", products.size(), ctx);
        writeCsvRow(out, {"STT", "Mã sản phẩm", "Tên sản phẩm", "Thương hiệu",
                          "Hệ điều hành", "Dung lượng", "Màu sắc",
                          "Giá nhập (VNĐ)", "Giá bán (VNĐ)", "Tồn thực tế",
                          "Đang giữ chỗ", "Tồn khả dụng", "Giá trị tồn kho (VNĐ)",
                          "Trạng thái kho", "Nhà sản xuất"});
        long long sumActual = 0, sumReserved = 0, sumAvail = 0;
        double sumInvVal = 0.0;
        for (size_t i = 0; i < products.size(); ++i) {
            const auto& p = products[i];
            double invVal = p.importPrice * p.actualStock;
            sumActual += p.actualStock;
            sumReserved += p.reservedStock;
            sumAvail += p.availableStock;
            sumInvVal += invVal;
            writeCsvRow(out, {
                std::to_string(i + 1), p.id, p.name, p.brand, p.os,
                p.capacity, p.color,
                formatNumericCsv(p.importPrice), formatNumericCsv(p.sellingPrice),
                std::to_string(p.actualStock), std::to_string(p.reservedStock),
                std::to_string(p.availableStock), formatNumericCsv(invVal),
                stockStatusToVietnamese(p.status), p.manufacturer
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng mã SP: " + std::to_string(products.size()),
                          "", "", "", "", "", "", "",
                          std::to_string(sumActual), std::to_string(sumReserved),
                          std::to_string(sumAvail), formatNumericCsv(sumInvVal), "", ""});
        return true;
    });
}

inline bool exportSuppliersToCsv(const std::string& filePath,
                                 const std::vector<SupplierItemViewModel>& suppliers,
                                 const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH NHÀ CUNG CẤP", suppliers.size(), ctx);
        writeCsvRow(out, {"STT", "Mã NCC", "Tên công ty", "Mã số thuế",
                          "Người liên hệ", "Số điện thoại", "Email", "Địa chỉ"});
        for (size_t i = 0; i < suppliers.size(); ++i) {
            const auto& s = suppliers[i];
            writeCsvRow(out, {
                std::to_string(i + 1), s.id, s.companyName, s.taxCode,
                s.contactPerson, s.phone, s.email, s.address
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số NCC: " + std::to_string(suppliers.size()),
                          "", "", "", "", "", ""});
        return true;
    });
}

inline bool exportDiscountsToCsv(const std::string& filePath,
                                 const std::vector<DiscountItemViewModel>& discounts,
                                 const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH MÃ GIẢM GIÁ (VOUCHER)", discounts.size(), ctx);
        writeCsvRow(out, {"STT", "Mã voucher", "Loại khuyến mãi", "Giá trị giảm",
                          "Đơn tối thiểu (VNĐ)", "Giảm tối đa (VNĐ)", "Ngày hết hạn",
                          "Đã sử dụng", "Giới hạn lượt", "Trạng thái", "Mô tả"});
        long long totalUsed = 0;
        for (size_t i = 0; i < discounts.size(); ++i) {
            const auto& d = discounts[i];
            totalUsed += d.usedCount;
            writeCsvRow(out, {
                std::to_string(i + 1), d.code, d.typeName, d.formattedValue,
                formatNumericCsv(d.minOrderValue), formatNumericCsv(d.maxDiscountAmount),
                d.expiryDate, std::to_string(d.usedCount), std::to_string(d.maxUsage),
                d.isValid ? "Còn hiệu lực" : "Hết hiệu lực / Hết lượt", d.description
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số mã: " + std::to_string(discounts.size()),
                          "", "", "", "", "", std::to_string(totalUsed), "", "", ""});
        return true;
    });
}

inline bool exportOrdersToCsv(const std::string& filePath,
                              const std::vector<OrderItemViewModel>& orders,
                              const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH ĐƠN HÀNG", orders.size(), ctx);
        writeCsvRow(out, {"STT", "Mã đơn hàng", "Thời gian đặt", "Mã KH", "Khách hàng",
                          "Người nhận", "SĐT người nhận", "Địa chỉ giao hàng",
                          "Giao nhận", "Phương thức TT", "Trạng thái TT",
                          "Trạng thái đơn", "Tổng SL máy", "Tạm tính (VNĐ)",
                          "Giảm giá (VNĐ)", "Thuế VAT (VNĐ)", "Thuế suất / Ghi chú VAT",
                          "Tổng thanh toán (VNĐ)"});
        long long sumQty = 0;
        double sumSub = 0.0, sumDisc = 0.0, sumVat = 0.0, sumTotal = 0.0;
        for (size_t i = 0; i < orders.size(); ++i) {
            const auto& o = orders[i];
            int qty = 0;
            for (const auto& d : o.details) qty += d.quantity;
            sumQty += qty;
            sumSub += o.subtotal;
            sumDisc += o.discountAmount;
            sumVat += o.vatAmount;
            sumTotal += o.total;
            std::string vatNote = o.isLegacyNoVat ? "HĐ cũ (không VAT)"
                                                  : PricingCalculator::formatVatPercent(o.vatRate);
            writeCsvRow(out, {
                std::to_string(i + 1), o.id, o.orderTime, o.customerId, o.customerName,
                o.receiverName, o.receiverPhone, o.deliveryAddress,
                o.deliveryMethodText, o.paymentMethodText, o.paymentStatusText,
                o.statusText, std::to_string(qty), formatNumericCsv(o.subtotal),
                formatNumericCsv(o.discountAmount), formatNumericCsv(o.vatAmount),
                vatNote, formatNumericCsv(o.total)
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số đơn: " + std::to_string(orders.size()),
                          "", "", "", "", "", "", "", "", "", "",
                          std::to_string(sumQty), formatNumericCsv(sumSub),
                          formatNumericCsv(sumDisc), formatNumericCsv(sumVat),
                          "", formatNumericCsv(sumTotal)});
        return true;
    });
}

inline bool exportInvoicesToCsv(const std::string& filePath,
                                const std::vector<InvoiceItemViewModel>& invoices,
                                const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH HÓA ĐƠN BÁN HÀNG", invoices.size(), ctx);
        writeCsvRow(out, {"STT", "Số hóa đơn", "Mã đơn hàng", "Thời gian lập",
                          "Mã KH", "Khách hàng", "Mã NV", "Thu ngân lập phiếu",
                          "Phương thức thanh toán", "Tổng SL máy",
                          "Tạm tính (VNĐ)", "Giảm giá (VNĐ)", "Thuế VAT (VNĐ)",
                          "Thuế suất / Ghi chú VAT", "Tổng thanh toán (VNĐ)"});
        long long sumQty = 0;
        double sumSub = 0.0, sumDisc = 0.0, sumVat = 0.0, sumTotal = 0.0;
        for (size_t i = 0; i < invoices.size(); ++i) {
            const auto& inv = invoices[i];
            int qty = 0;
            for (const auto& d : inv.details) qty += d.quantity;
            sumQty += qty;
            sumSub += inv.subtotal;
            sumDisc += inv.discountAmount;
            sumVat += inv.vatAmount;
            sumTotal += inv.total;
            std::string vatNote = inv.isLegacyNoVat ? "HĐ cũ (không VAT)"
                                                    : PricingCalculator::formatVatPercent(inv.vatRate);
            writeCsvRow(out, {
                std::to_string(i + 1), inv.id, inv.orderId, inv.createTime,
                inv.customerId, inv.customerName, inv.employeeId, inv.employeeName,
                inv.paymentMethodText, std::to_string(qty),
                formatNumericCsv(inv.subtotal), formatNumericCsv(inv.discountAmount),
                formatNumericCsv(inv.vatAmount), vatNote, formatNumericCsv(inv.total)
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số HĐ: " + std::to_string(invoices.size()),
                          "", "", "", "", "", "", "",
                          std::to_string(sumQty), formatNumericCsv(sumSub),
                          formatNumericCsv(sumDisc), formatNumericCsv(sumVat),
                          "", formatNumericCsv(sumTotal)});
        return true;
    });
}

inline bool exportGoodsReceiptsToCsv(const std::string& filePath,
                                     const std::vector<GoodsReceiptSummaryViewModel>& receipts,
                                     const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH PHIẾU NHẬP KHO", receipts.size(), ctx);
        writeCsvRow(out, {"STT", "Mã phiếu nhập", "Thời gian nhập", "Mã NCC",
                          "Nhà cung cấp", "Mã NV", "Người lập phiếu",
                          "Số dòng SP", "Tổng số máy nhập", "Tổng tiền nhập (VNĐ)"});
        long long sumUnits = 0;
        double sumAmount = 0.0;
        for (size_t i = 0; i < receipts.size(); ++i) {
            const auto& r = receipts[i];
            int units = 0;
            for (const auto& d : r.details) units += d.quantity;
            sumUnits += units;
            sumAmount += r.totalAmount;
            writeCsvRow(out, {
                std::to_string(i + 1), r.id, r.importTime, r.supplierId,
                r.supplierName, r.employeeId, r.employeeName,
                std::to_string(r.details.size()), std::to_string(units),
                formatNumericCsv(r.totalAmount)
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số phiếu: " + std::to_string(receipts.size()),
                          "", "", "", "", "", "", std::to_string(sumUnits),
                          formatNumericCsv(sumAmount)});
        return true;
    });
}

inline bool exportWarrantiesToCsv(const std::string& filePath,
                                  const std::vector<WarrantyItemViewModel>& warranties,
                                  const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH PHIẾU BẢO HÀNH", warranties.size(), ctx);
        writeCsvRow(out, {"STT", "Mã phiếu BH", "Mã đơn hàng", "Mã SP", "Tên thiết bị",
                          "Khách hàng", "SĐT khách hàng", "Ngày tiếp nhận",
                          "Hạn bảo hành", "Còn lại (ngày)", "Trạng thái",
                          "Mô tả lỗi", "Ghi chú kỹ thuật", "Hướng xử lý"});
        for (size_t i = 0; i < warranties.size(); ++i) {
            const auto& w = warranties[i];
            writeCsvRow(out, {
                std::to_string(i + 1), w.id, w.orderId, w.phoneId, w.phoneName,
                w.customerName, w.customerPhone,
                w.requestDate.empty() ? w.createdAt : w.requestDate,
                w.expireDate, std::to_string(w.remainingDays), w.statusText,
                w.issueDescription, w.technicianNote, w.resolution
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng số phiếu BH: " + std::to_string(warranties.size()),
                          "", "", "", "", "", "", "", "", "", "", "", ""});
        return true;
    });
}

inline bool exportAccountsToCsv(const std::string& filePath,
                                const std::vector<AccountItemViewModel>& accounts,
                                const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;
    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeCsvHeaderMeta(out, "DANH SÁCH TÀI KHOẢN HỆ THỐNG", accounts.size(), ctx);
        writeCsvRow(out, {"STT", "Tên đăng nhập", "Họ và tên", "Số điện thoại",
                          "Vai trò", "Trạng thái"});
        int activeCnt = 0;
        for (size_t i = 0; i < accounts.size(); ++i) {
            const auto& a = accounts[i];
            if (a.isActive) ++activeCnt;
            writeCsvRow(out, {
                std::to_string(i + 1), a.username, a.linkedFullName, a.phone,
                a.roleName, a.isActive ? "Đang hoạt động" : "Đã khóa"
            });
        }
        writeCsvRow(out, {});
        writeCsvRow(out, {"TỔNG KẾT", "Tổng tài khoản: " + std::to_string(accounts.size()),
                          "Hoạt động: " + std::to_string(activeCnt),
                          "Đã khóa: " + std::to_string(static_cast<int>(accounts.size()) - activeCnt),
                          "", ""});
        return true;
    });
}

inline bool exportRevenueReportToCsv(const std::string& filePath,
                                     const DashboardStatsViewModel& stats,
                                     const std::string& fromDate,
                                     const std::string& toDate,
                                     int timelineMode,
                                     const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".csv");
    if (target.empty()) return false;

    std::string rangeDesc = "Toàn thời gian";
    if (!fromDate.empty() && !toDate.empty()) {
        rangeDesc = "Từ " + fromDate + " đến " + toDate;
    } else if (!fromDate.empty()) {
        rangeDesc = "Từ " + fromDate + " đến hiện tại";
    } else if (!toDate.empty()) {
        rangeDesc = "Từ đầu kỳ đến " + toDate;
    }
    std::string modeLabel = (timelineMode == 1) ? "Theo Tháng"
                          : (timelineMode == 2) ? "Theo Năm"
                                                : "Theo Ngày";

    double profitMargin = (stats.totalRevenue > 0.0)
                              ? (stats.estimatedProfit * 100.0 / stats.totalRevenue)
                              : 0.0;

    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        out << "\xEF\xBB\xBF";
        writeCsvRow(out, {ctx.storeName + " - " + ctx.storeSubtitle});
        writeCsvRow(out, {"BÁO CÁO DOANH THU & LỢI NHUẬN TỔNG HỢP"});
        writeCsvRow(out, {"Kỳ báo cáo", rangeDesc});
        writeCsvRow(out, {"Chu kỳ thống kê", modeLabel});
        writeCsvRow(out, {"Thời gian xuất", ctx.exportedAt});
        writeCsvRow(out, {"Người lập báo cáo", ctx.exportedBy});
        writeCsvRow(out, {});

        // PHẦN 1: CHỈ SỐ TỔNG QUAN (KPI)
        writeCsvRow(out, {"PHẦN 1: CHỈ SỐ TỔNG QUAN KINH DOANH (KPI)"});
        writeCsvRow(out, {"Chỉ tiêu", "Giá trị", "Đơn vị / Ghi chú"});
        writeCsvRow(out, {"Tổng doanh thu thuần", formatNumericCsv(stats.totalRevenue), "VNĐ (Trước VAT, sau chiết khấu)"});
        writeCsvRow(out, {"Tổng thuế VAT thu hộ", formatNumericCsv(stats.totalVat), "VNĐ (Từ hóa đơn có áp VAT)"});
        writeCsvRow(out, {"Tổng tiền thanh toán (gồm VAT)", formatNumericCsv(stats.totalGrossRevenue), "VNĐ (Doanh thu thuần + VAT)"});
        writeCsvRow(out, {"Lợi nhuận gộp ước tính", formatNumericCsv(stats.estimatedProfit), "VNĐ (Doanh thu thuần - Giá vốn snapshot)"});
        writeCsvRow(out, {"Tỷ suất lợi nhuận gộp", formatPercent(profitMargin), "%"});
        writeCsvRow(out, {"Tổng số đơn hàng trong kỳ", std::to_string(stats.totalOrders), "Đơn"});
        writeCsvRow(out, {"Đơn hàng hoàn tất", std::to_string(stats.completedOrders), "Đơn"});
        writeCsvRow(out, {"Đơn hàng đang xử lý / giao", std::to_string(stats.pendingOrders + stats.shippingOrders), "Đơn"});
        writeCsvRow(out, {"Đơn hàng đã hủy", std::to_string(stats.cancelledOrders), "Đơn"});
        writeCsvRow(out, {"Tổng mã sản phẩm kinh doanh", std::to_string(stats.totalProducts), "Mã SP"});
        writeCsvRow(out, {"Tổng số lượng tồn kho", std::to_string(stats.totalStockQuantity), "Máy"});
        writeCsvRow(out, {"Sản phẩm sắp hết hàng (<= 5)", std::to_string(stats.lowStockCount), "Mã SP"});
        writeCsvRow(out, {"Tổng giá trị tồn kho (giá nhập)", formatNumericCsv(stats.totalInventoryValue), "VNĐ"});
        writeCsvRow(out, {"Tổng số khách hàng", std::to_string(stats.totalCustomers), "Khách hàng"});
        writeCsvRow(out, {"Tổng số nhân viên", std::to_string(stats.totalEmployees), "Nhân viên"});
        writeCsvRow(out, {});

        // PHẦN 2: DOANH THU THEO THỜI GIAN
        writeCsvRow(out, {"PHẦN 2: CHI TIẾT DOANH THU & LỢI NHUẬN THEO THỜI GIAN (" + modeLabel + ")"});
        writeCsvRow(out, {"STT", "Mốc thời gian", "Số đơn hoàn tất", "Doanh thu (VNĐ)", "Lợi nhuận gộp (VNĐ)", "Tỷ suất LN (%)"});
        int sumOrd = 0;
        double sumRev = 0.0, sumProf = 0.0;
        for (size_t i = 0; i < stats.timelineStats.size(); ++i) {
            const auto& pt = stats.timelineStats[i];
            sumOrd += pt.orderCount;
            sumRev += pt.revenue;
            sumProf += pt.profit;
            double m = (pt.revenue > 0.0) ? (pt.profit * 100.0 / pt.revenue) : 0.0;
            writeCsvRow(out, {
                std::to_string(i + 1), pt.label, std::to_string(pt.orderCount),
                formatNumericCsv(pt.revenue), formatNumericCsv(pt.profit), formatPercent(m)
            });
        }
        double totalM = (sumRev > 0.0) ? (sumProf * 100.0 / sumRev) : 0.0;
        writeCsvRow(out, {"TỔNG CỘNG", "", std::to_string(sumOrd),
                          formatNumericCsv(sumRev), formatNumericCsv(sumProf), formatPercent(totalM)});
        writeCsvRow(out, {});

        // PHẦN 3: CƠ CẤU DOANH THU THEO THƯƠNG HIỆU
        writeCsvRow(out, {"PHẦN 3: CƠ CẤU DOANH THU THEO THƯƠNG HIỆU"});
        writeCsvRow(out, {"STT", "Thương hiệu", "Số máy bán ra", "Doanh thu (VNĐ)", "Tỷ trọng (%)"});
        int sumBrandQty = 0;
        double sumBrandRev = 0.0;
        for (size_t i = 0; i < stats.brandStats.size(); ++i) {
            const auto& b = stats.brandStats[i];
            sumBrandQty += b.soldQuantity;
            sumBrandRev += b.revenue;
            writeCsvRow(out, {
                std::to_string(i + 1), b.brand, std::to_string(b.soldQuantity),
                formatNumericCsv(b.revenue), formatPercent(b.percentage)
            });
        }
        writeCsvRow(out, {"TỔNG CỘNG", "", std::to_string(sumBrandQty),
                          formatNumericCsv(sumBrandRev), stats.brandStats.empty() ? "0.00%" : "100.00%"});
        writeCsvRow(out, {});

        // PHẦN 4: TOP SẢN PHẨM BÁN CHẠY
        writeCsvRow(out, {"PHẦN 4: TOP SẢN PHẨM BÁN CHẠY TRONG KỲ"});
        writeCsvRow(out, {"Hạng", "Mã sản phẩm", "Tên sản phẩm", "Thương hiệu",
                          "Số lượng bán", "Doanh thu (VNĐ)", "Lợi nhuận (VNĐ)"});
        int sumTopQty = 0;
        double sumTopRev = 0.0, sumTopProf = 0.0;
        for (size_t i = 0; i < stats.topProducts.size(); ++i) {
            const auto& tp = stats.topProducts[i];
            sumTopQty += tp.soldQuantity;
            sumTopRev += tp.revenue;
            sumTopProf += tp.profit;
            writeCsvRow(out, {
                std::to_string(i + 1), tp.phoneId, tp.phoneName, tp.brand,
                std::to_string(tp.soldQuantity), formatNumericCsv(tp.revenue),
                formatNumericCsv(tp.profit)
            });
        }
        writeCsvRow(out, {"TỔNG CỘNG", "", "", "", std::to_string(sumTopQty),
                          formatNumericCsv(sumTopRev), formatNumericCsv(sumTopProf)});
        return true;
    });
}

// ============================================================================
// 3. XUẤT CÁC LOẠI HÓA ĐƠN & PHIẾU CHỨNG TỪ SANG FILE .TXT (CHUẨN 80 KÝ TỰ)
// ============================================================================

inline constexpr size_t kTxtWidth = 80;

inline void writeTxtStoreHeader(std::ostream& out,
                                const ExportContext& ctx,
                                const std::string& docTitle,
                                const std::string& docSubtitle) {
    std::string doubleLine(kTxtWidth, '=');
    std::string singleLine(kTxtWidth, '-');
    out << doubleLine << "\n";
    out << centerUtf8(ctx.storeName, kTxtWidth) << "\n";
    out << centerUtf8(ctx.storeSubtitle, kTxtWidth) << "\n";
    out << centerUtf8("Địa chỉ: " + ctx.storeAddress, kTxtWidth) << "\n";
    out << centerUtf8("Hotline: " + ctx.storeHotline + "  |  MST: " + ctx.storeTaxCode, kTxtWidth) << "\n";
    out << doubleLine << "\n";
    out << centerUtf8(docTitle, kTxtWidth) << "\n";
    if (!docSubtitle.empty()) {
        out << centerUtf8(docSubtitle, kTxtWidth) << "\n";
    }
    out << singleLine << "\n";
}

inline void writeTxtTwoColMeta(std::ostream& out,
                               const std::string& leftLabel, const std::string& leftVal,
                               const std::string& rightLabel, const std::string& rightVal) {
    // Trái 42 ký tự, Phải 38 ký tự (= 80)
    std::string leftPart = padRightUtf8(leftLabel, 15) + ": " + leftVal;
    std::string rightPart = padRightUtf8(rightLabel, 12) + ": " + rightVal;
    out << padRightUtf8(leftPart, 42) << " " << padRightUtf8(rightPart, 37) << "\n";
}

inline void writeTxtWrappedField(std::ostream& out, const std::string& label, const std::string& value) {
    const size_t labelWidth = 15;
    const size_t prefixWidth = labelWidth + 2; // "Label          : "
    const size_t valWidth = kTxtWidth - prefixWidth;
    auto lines = wrapUtf8(value.empty() ? "(Không có)" : value, valWidth);
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == 0) {
            out << padRightUtf8(label, labelWidth) << ": " << lines[i] << "\n";
        } else {
            out << std::string(prefixWidth, ' ') << lines[i] << "\n";
        }
    }
}

// Bảng sản phẩm 80 ký tự:
// STT (4) | Tên sản phẩm (35) | SL (4) | Đơn giá (14) | Thành tiền (15) => 4+3+35+3+4+3+14+3+11?
// Tính chính xác:
// " 1  | " (4) + " | " (3) + 32 + " | " (3) + 4 + " | " (3) + 14 + " | " (3) + 14 = 80!
// Kiểm tra: 4 + 3 + 32 + 3 + 4 + 3 + 14 + 3 + 14 = 80 ký tự chẵn!
inline void writeTxtItemTableHeader(std::ostream& out, const std::string& priceColTitle) {
    out << std::string(kTxtWidth, '=') << "\n";
    out << centerUtf8("STT", 4) << " | "
        << padRightUtf8("Tên sản phẩm / Thiết bị", 32) << " | "
        << padLeftUtf8("SL", 4) << " | "
        << padLeftUtf8(priceColTitle, 14) << " | "
        << padLeftUtf8("Thành tiền", 14) << "\n";
    out << std::string(4, '-') << "-+-"
        << std::string(32, '-') << "-+-"
        << std::string(4, '-') << "-+-"
        << std::string(14, '-') << "-+-"
        << std::string(14, '-') << "\n";
}

inline void writeTxtItemRow(std::ostream& out,
                            size_t index,
                            const std::string& phoneId,
                            const std::string& phoneName,
                            int quantity,
                            double unitPrice,
                            double subtotal) {
    auto nameLines = wrapUtf8(phoneName.empty() ? phoneId : phoneName, 32);
    for (size_t i = 0; i < nameLines.size(); ++i) {
        if (i == 0) {
            out << centerUtf8(std::to_string(index), 4) << " | "
                << padRightUtf8(nameLines[i], 32) << " | "
                << padLeftUtf8(std::to_string(quantity), 4) << " | "
                << padLeftUtf8(formatVNDPlain(unitPrice), 14) << " | "
                << padLeftUtf8(formatVNDPlain(subtotal), 14) << "\n";
        } else {
            out << std::string(4, ' ') << " | "
                << padRightUtf8(nameLines[i], 32) << " | "
                << std::string(4, ' ') << " | "
                << std::string(14, ' ') << " | "
                << std::string(14, ' ') << "\n";
        }
    }
    if (!phoneId.empty()) {
        out << std::string(4, ' ') << " | "
            << padRightUtf8("Mã SP: " + phoneId, 32) << " | "
            << std::string(4, ' ') << " | "
            << std::string(14, ' ') << " | "
            << std::string(14, ' ') << "\n";
    }
}

inline void writeTxtSummaryLine(std::ostream& out, const std::string& label, const std::string& value) {
    // 40 spaces + 20 label + ": " + 18 right-aligned value = 80
    out << std::string(38, ' ') << padRightUtf8(label, 21) << ": " << padLeftUtf8(value, 19) << "\n";
}

// 3.1. XUẤT HÓA ĐƠN BÁN HÀNG (.TXT)
inline bool exportInvoiceToTxt(const std::string& filePath,
                               const InvoiceItemViewModel& inv,
                               const std::string& customerPhone,
                               const std::string& customerAddress,
                               const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return false;

    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeTxtStoreHeader(out, ctx, "HÓA ĐƠN BÁN HÀNG", "(Chứng từ giao dịch & Bảo hành điện tử)");
        writeTxtTwoColMeta(out, "Số hóa đơn", inv.id, "Ngày lập", inv.createTime);
        writeTxtTwoColMeta(out, "Mã đơn hàng", inv.orderId.empty() ? "(Mua tại quầy)" : inv.orderId,
                           "Thu ngân", inv.employeeName.empty() ? inv.employeeId : inv.employeeName);
        writeTxtTwoColMeta(out, "Khách hàng", inv.customerName.empty() ? "Khách lẻ" : inv.customerName,
                           "Mã KH", inv.customerId.empty() ? "GUEST" : inv.customerId);
        writeTxtTwoColMeta(out, "Số điện thoại", customerPhone.empty() ? "(Không có)" : customerPhone,
                           "Thanh toán", inv.paymentMethodText);
        if (!customerAddress.empty()) {
            writeTxtWrappedField(out, "Địa chỉ", customerAddress);
        }

        writeTxtItemTableHeader(out, "Đơn giá (VNĐ)");
        int totalQty = 0;
        for (size_t i = 0; i < inv.details.size(); ++i) {
            const auto& d = inv.details[i];
            totalQty += d.quantity;
            writeTxtItemRow(out, i + 1, d.phoneId, d.phoneName, d.quantity, d.unitPrice, d.subtotal);
        }
        if (inv.details.empty()) {
            out << centerUtf8("(Không có dữ liệu chi tiết sản phẩm)", kTxtWidth) << "\n";
        }

        out << std::string(kTxtWidth, '=') << "\n";
        writeTxtSummaryLine(out, "Tổng số lượng máy", std::to_string(totalQty) + " máy");
        writeTxtSummaryLine(out, "Tạm tính", formatVNDCurrency(inv.subtotal));
        std::string discStr = (inv.discountAmount > 0.0)
                                  ? ("-" + formatVNDCurrency(inv.discountAmount))
                                  : "0 đ";
        writeTxtSummaryLine(out, "Giảm giá / Voucher", discStr);
        if (inv.isLegacyNoVat) {
            writeTxtSummaryLine(out, "Thuế VAT", "Không áp dụng (HĐ cũ)");
        } else {
            writeTxtSummaryLine(out, "Thuế VAT (" + PricingCalculator::formatVatPercent(inv.vatRate) + ")",
                                formatVNDCurrency(inv.vatAmount));
        }
        out << std::string(38, ' ') << std::string(42, '-') << "\n";
        writeTxtSummaryLine(out, "TỔNG THANH TOÁN", formatVNDCurrency(inv.total));
        out << std::string(kTxtWidth, '=') << "\n";

        out << "CHÍNH SÁCH HẬU MÃI & BẢO HÀNH:\n";
        writeTxtWrappedField(out, "- Đổi trả", ctx.returnPolicy);
        writeTxtWrappedField(out, "- Bảo hành", ctx.warrantyPolicy);
        out << std::string(kTxtWidth, '-') << "\n";

        out << centerUtf8("Khách hàng", 40) << centerUtf8("Thu ngân / Lập phiếu", 40) << "\n";
        out << centerUtf8("(Ký, ghi rõ họ tên)", 40) << centerUtf8("(Ký, ghi rõ họ tên)", 40) << "\n\n\n";
        out << centerUtf8(inv.customerName.empty() ? "Khách lẻ" : inv.customerName, 40)
            << centerUtf8(inv.employeeName.empty() ? inv.employeeId : inv.employeeName, 40) << "\n";
        out << std::string(kTxtWidth, '-') << "\n";
        out << centerUtf8("Cảm ơn Quý khách đã tin tưởng mua sắm tại " + ctx.storeName + "!", kTxtWidth) << "\n";
        out << centerUtf8("Xuất lúc: " + ctx.exportedAt, kTxtWidth) << "\n";
        out << centerUtf8("Người xuất: " + ctx.exportedBy, kTxtWidth) << "\n";
        out << std::string(kTxtWidth, '=') << "\n";
        return true;
    });
}

// 3.2. XUẤT PHIẾU ĐƠN HÀNG / HÓA ĐƠN ĐẶT HÀNG (.TXT)
inline bool exportOrderToTxt(const std::string& filePath,
                             const OrderItemViewModel& ord,
                             const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return false;

    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeTxtStoreHeader(out, ctx, "PHIẾU XÁC NHẬN ĐƠN HÀNG", "(Thông tin chi tiết đơn đặt hàng & giao nhận)");
        writeTxtTwoColMeta(out, "Mã đơn hàng", ord.id, "Ngày đặt", ord.orderTime);
        writeTxtTwoColMeta(out, "Trạng thái đơn", ord.statusText, "Thanh toán", ord.paymentStatusText);
        writeTxtTwoColMeta(out, "Giao nhận", ord.deliveryMethodText, "PT Thanh toán", ord.paymentMethodText);
        out << std::string(kTxtWidth, '-') << "\n";
        writeTxtTwoColMeta(out, "Khách hàng", ord.customerName, "Mã KH", ord.customerId);
        writeTxtTwoColMeta(out, "Người nhận", ord.receiverName.empty() ? ord.customerName : ord.receiverName,
                           "SĐT nhận", ord.receiverPhone.empty() ? "(Không có)" : ord.receiverPhone);
        writeTxtWrappedField(out, "Địa chỉ giao", ord.deliveryAddress.empty() ? "Nhận trực tiếp tại cửa hàng" : ord.deliveryAddress);

        writeTxtItemTableHeader(out, "Đơn giá (VNĐ)");
        int totalQty = 0;
        for (size_t i = 0; i < ord.details.size(); ++i) {
            const auto& d = ord.details[i];
            totalQty += d.quantity;
            writeTxtItemRow(out, i + 1, d.phoneId, d.phoneName, d.quantity, d.unitPrice, d.subtotal);
        }
        if (ord.details.empty()) {
            out << centerUtf8("(Không có dữ liệu chi tiết sản phẩm)", kTxtWidth) << "\n";
        }

        out << std::string(kTxtWidth, '=') << "\n";
        writeTxtSummaryLine(out, "Tổng số lượng máy", std::to_string(totalQty) + " máy");
        writeTxtSummaryLine(out, "Tạm tính", formatVNDCurrency(ord.subtotal));
        std::string discStr = (ord.discountAmount > 0.0)
                                  ? ("-" + formatVNDCurrency(ord.discountAmount))
                                  : "0 đ";
        writeTxtSummaryLine(out, "Giảm giá / Voucher", discStr);
        if (ord.isLegacyNoVat) {
            writeTxtSummaryLine(out, "Thuế VAT", "Không áp dụng (Đơn cũ)");
        } else {
            writeTxtSummaryLine(out, "Thuế VAT (" + PricingCalculator::formatVatPercent(ord.vatRate) + ")",
                                formatVNDCurrency(ord.vatAmount));
        }
        out << std::string(38, ' ') << std::string(42, '-') << "\n";
        writeTxtSummaryLine(out, "TỔNG GIÁ TRỊ ĐƠN", formatVNDCurrency(ord.total));
        out << std::string(kTxtWidth, '=') << "\n";

        out << "CHÍNH SÁCH GIAO HÀNG & ĐỔI TRẢ:\n";
        writeTxtWrappedField(out, "- Giao nhận", ctx.shippingPolicy);
        writeTxtWrappedField(out, "- Đổi trả", ctx.returnPolicy);
        out << std::string(kTxtWidth, '-') << "\n";

        out << centerUtf8("Người nhận hàng", 40) << centerUtf8("Đại diện cửa hàng", 40) << "\n";
        out << centerUtf8("(Ký, ghi rõ họ tên)", 40) << centerUtf8("(Ký, đóng dấu)", 40) << "\n\n\n";
        out << centerUtf8(ord.receiverName.empty() ? ord.customerName : ord.receiverName, 40)
            << centerUtf8(ctx.storeName, 40) << "\n";
        out << std::string(kTxtWidth, '-') << "\n";
        out << centerUtf8("Xuất lúc: " + ctx.exportedAt, kTxtWidth) << "\n";
        out << centerUtf8("Người xuất: " + ctx.exportedBy, kTxtWidth) << "\n";
        out << std::string(kTxtWidth, '=') << "\n";
        return true;
    });
}

// 3.3. XUẤT PHIẾU NHẬP KHO / HÓA ĐƠN NHẬP HÀNG (.TXT)
inline bool exportGoodsReceiptToTxt(const std::string& filePath,
                                    const GoodsReceiptSummaryViewModel& receipt,
                                    const SupplierItemViewModel& supplier,
                                    const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return false;

    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeTxtStoreHeader(out, ctx, "PHIẾU NHẬP KHO HÀNG HÓA", "(Chứng từ nhập kho từ Nhà cung cấp)");
        writeTxtTwoColMeta(out, "Số phiếu nhập", receipt.id, "Ngày nhập", receipt.importTime);
        writeTxtTwoColMeta(out, "Thủ kho lập", receipt.employeeName.empty() ? receipt.employeeId : receipt.employeeName,
                           "Mã NV", receipt.employeeId);
        out << std::string(kTxtWidth, '-') << "\n";
        writeTxtTwoColMeta(out, "Nhà cung cấp", receipt.supplierName.empty() ? supplier.companyName : receipt.supplierName,
                           "Mã NCC", receipt.supplierId);
        if (!supplier.taxCode.empty() || !supplier.phone.empty()) {
            writeTxtTwoColMeta(out, "Mã số thuế", supplier.taxCode.empty() ? "(Không có)" : supplier.taxCode,
                               "SĐT NCC", supplier.phone.empty() ? "(Không có)" : supplier.phone);
        }
        if (!supplier.contactPerson.empty()) {
            writeTxtTwoColMeta(out, "Người liên hệ", supplier.contactPerson,
                               "Email", supplier.email.empty() ? "(Không có)" : supplier.email);
        }
        if (!supplier.address.empty()) {
            writeTxtWrappedField(out, "Địa chỉ NCC", supplier.address);
        }

        writeTxtItemTableHeader(out, "Giá nhập (VNĐ)");
        int totalQty = 0;
        for (size_t i = 0; i < receipt.details.size(); ++i) {
            const auto& d = receipt.details[i];
            totalQty += d.quantity;
            writeTxtItemRow(out, i + 1, d.phoneId, d.phoneName, d.quantity, d.importPrice, d.subtotal);
        }
        if (receipt.details.empty()) {
            out << centerUtf8("(Không có dữ liệu chi tiết hàng nhập)", kTxtWidth) << "\n";
        }

        out << std::string(kTxtWidth, '=') << "\n";
        writeTxtSummaryLine(out, "Số dòng mặt hàng", std::to_string(receipt.details.size()) + " dòng");
        writeTxtSummaryLine(out, "Tổng số lượng nhập", std::to_string(totalQty) + " máy");
        out << std::string(38, ' ') << std::string(42, '-') << "\n";
        writeTxtSummaryLine(out, "TỔNG TIỀN NHẬP KHO", formatVNDCurrency(receipt.totalAmount));
        out << std::string(kTxtWidth, '=') << "\n";

        // Chữ ký 3 bên (26 + 27 + 27 = 80)
        out << centerUtf8("Người giao hàng", 26)
            << centerUtf8("Thủ kho lập phiếu", 27)
            << centerUtf8("Quản lý / Kế toán", 27) << "\n";
        out << centerUtf8("(Ký, ghi rõ họ tên)", 26)
            << centerUtf8("(Ký, ghi rõ họ tên)", 27)
            << centerUtf8("(Ký, ghi rõ họ tên)", 27) << "\n\n\n";
        out << centerUtf8(supplier.contactPerson.empty() ? "Đại diện NCC" : supplier.contactPerson, 26)
            << centerUtf8(receipt.employeeName.empty() ? receipt.employeeId : receipt.employeeName, 27)
            << centerUtf8("Ban Quản Lý", 27) << "\n";
        out << std::string(kTxtWidth, '-') << "\n";
        out << centerUtf8("Xuất lúc: " + ctx.exportedAt, kTxtWidth) << "\n";
        out << centerUtf8("Người xuất: " + ctx.exportedBy, kTxtWidth) << "\n";
        out << std::string(kTxtWidth, '=') << "\n";
        return true;
    });
}

// 3.4. XUẤT PHIẾU TIẾP NHẬN & BẢO HÀNH THIẾT BỊ (.TXT)
inline bool exportWarrantyToTxt(const std::string& filePath,
                                const WarrantyItemViewModel& w,
                                const ExportContext& ctx) {
    std::string target = ensureParentDirAndExtension(filePath, ".txt");
    if (target.empty()) return false;

    return FileHelper::atomicWrite(target, [&](std::ofstream& out) {
        writeTxtStoreHeader(out, ctx, "PHIẾU TIẾP NHẬN & BẢO HÀNH THIẾT BỊ", "(Chứng từ theo dõi sửa chữa & bảo hành chính hãng)");
        writeTxtTwoColMeta(out, "Mã phiếu BH", w.id, "Trạng thái", w.statusText);
        writeTxtTwoColMeta(out, "Mã đơn hàng", w.orderId, "Ngày nhận", w.requestDate.empty() ? w.createdAt : w.requestDate);
        if (!w.invoiceId.empty() || !w.updatedAt.empty()) {
            writeTxtTwoColMeta(out, "Mã hóa đơn", w.invoiceId.empty() ? "(Theo đơn)" : w.invoiceId,
                               "Cập nhật", w.updatedAt.empty() ? w.createdAt : w.updatedAt);
        }
        out << std::string(kTxtWidth, '-') << "\n";
        out << "1. THÔNG TIN KHÁCH HÀNG & THIẾT BỊ:\n";
        writeTxtTwoColMeta(out, "Khách hàng", w.customerName, "Mã KH", w.customerId);
        writeTxtTwoColMeta(out, "Số điện thoại", w.customerPhone.empty() ? "(Không có)" : w.customerPhone,
                           "Mã thiết bị", w.phoneId);
        writeTxtWrappedField(out, "Tên thiết bị", w.phoneName);
        if (!w.expireDate.empty()) {
            std::string remText = (w.remainingDays > 0)
                                      ? ("Còn " + std::to_string(w.remainingDays) + " ngày")
                                      : "Đã hết hạn bảo hành";
            writeTxtTwoColMeta(out, "Hạn bảo hành", w.expireDate, "Hiệu lực", remText);
        }

        out << std::string(kTxtWidth, '-') << "\n";
        out << "2. TÌNH TRẠNG TIẾP NHẬN & KẾT QUẢ XỬ LÝ:\n";
        writeTxtWrappedField(out, "Mô tả lỗi", w.issueDescription);
        writeTxtWrappedField(out, "Ghi chú KT", w.technicianNote.empty() ? "(Đang chờ kiểm tra kỹ thuật)" : w.technicianNote);
        writeTxtWrappedField(out, "Hướng xử lý", w.resolution.empty() ? "(Chưa có kết luận xử lý)" : w.resolution);

        out << std::string(kTxtWidth, '=') << "\n";
        out << "CHÍNH SÁCH BẢO HÀNH ÁP DỤNG:\n";
        writeTxtWrappedField(out, "- Bảo hành", ctx.warrantyPolicy);
        writeTxtWrappedField(out, "- Lưu ý", "Quý khách vui lòng giữ phiếu này để đối chiếu khi nhận lại thiết bị.");
        out << std::string(kTxtWidth, '-') << "\n";

        out << centerUtf8("Khách hàng", 40) << centerUtf8("Kỹ thuật viên / Tiếp nhận", 40) << "\n";
        out << centerUtf8("(Ký, ghi rõ họ tên)", 40) << centerUtf8("(Ký, ghi rõ họ tên)", 40) << "\n\n\n";
        out << centerUtf8(w.customerName, 40) << centerUtf8(ctx.exportedBy, 40) << "\n";
        out << std::string(kTxtWidth, '-') << "\n";
        out << centerUtf8("Xuất lúc: " + ctx.exportedAt + "  |  Hotline hỗ trợ: " + ctx.storeHotline, kTxtWidth) << "\n";
        out << std::string(kTxtWidth, '=') << "\n";
        return true;
    });
}

} // namespace ExportHelper

#endif // EXPORT_HELPER_H
