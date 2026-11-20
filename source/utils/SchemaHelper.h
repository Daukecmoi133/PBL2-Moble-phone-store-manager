#ifndef SCHEMAHELPER_H
#define SCHEMAHELPER_H

#include <functional>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file SchemaHelper.h
 * @brief Chuẩn hóa header v2 và kiểm chứng dòng đầu tiên của các tệp dữ liệu .txt.
 *
 * Phân biệt rõ 4 trường hợp khi đọc dòng đầu tiên không rỗng (đã strip UTF-8 BOM):
 *  - V2_HEADER:      Khớp đúng header chuẩn v2 -> bỏ qua dòng header.
 *  - V1_HEADER:      Khớp header cũ hợp lệ (vd: thiếu accountId|active, Email viết hoa) -> bỏ qua an toàn.
 *  - DATA_ROW:       Tệp bị mất header, dòng 1 là dữ liệu thực -> cảnh báo và KHÔNG bỏ qua dòng 1.
 *  - UNKNOWN_HEADER: Dòng header lạ / sai tên cột -> cảnh báo schema rõ ràng và bỏ qua dòng header đó.
 */
class SchemaHelper {
public:
    enum class FirstLineKind {
        V2_HEADER,
        V1_HEADER,
        DATA_ROW,
        UNKNOWN_HEADER
    };

    static inline constexpr const char* PRODUCTS_HEADER =
        "id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications";
    static inline constexpr const char* CUSTOMERS_HEADER =
        "id|firstName|middleName|lastName|phoneNumber|email|username|password|points|address|accountId|active";
    static inline constexpr const char* CUSTOMERS_V1_HEADER =
        "id|firstName|middleName|lastName|phoneNumber|Email|username|password|points|address";
    static inline constexpr const char* CUSTOMERS_V1_HEADER_LOWER =
        "id|firstName|middleName|lastName|phoneNumber|email|username|password|points|address";
    static inline constexpr const char* EMPLOYEES_HEADER =
        "id|firstName|middleName|lastName|phoneNumber|email|username|password|role|address|salary|accountId|active";
    static inline constexpr const char* EMPLOYEES_V1_HEADER =
        "id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address|salary";
    static inline constexpr const char* EMPLOYEES_V1_HEADER_LOWER =
        "id|firstName|middleName|lastName|phoneNumber|email|username|password|role|address|salary";
    static inline constexpr const char* EMPLOYEES_V0_HEADER =
        "id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address";
    static inline constexpr const char* SUPPLIERS_HEADER =
        "id|companyName|taxCode|contactPerson|phoneNumber|email|address";
    static inline constexpr const char* ORDERS_HEADER =
        "id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details|discountCode|vatRate|vatAmount";
    static inline constexpr const char* ORDERS_V2_HEADER =
        "id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details|discountCode";
    static inline constexpr const char* ORDERS_V1_HEADER =
        "id|customerId|deliveryMethod|paymentMethod|status|orderTime|discountAmount|receiverName|phone|address|details";
    static inline constexpr const char* INVOICES_HEADER =
        "id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details|discountCode|vatRate|vatAmount";
    static inline constexpr const char* INVOICES_V2_HEADER =
        "id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details|discountCode";
    static inline constexpr const char* INVOICES_V1_HEADER =
        "id|orderId|customerId|employeeId|paymentMethod|createTime|discountAmount|details";
    static inline constexpr const char* STOCK_RECEIPTS_HEADER =
        "id|supplierId|employeeId|importTime|details";
    static inline constexpr const char* DISCOUNTS_HEADER =
        "code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount";
    static inline constexpr const char* WARRANTY_REQUESTS_HEADER =
        "id|orderId|phoneId|phoneName|customerId|createdAt|status|issueDescription|technicianNote|resolution|updatedAt";
    static inline constexpr const char* WARRANTIES_HEADER =
        "id|phoneId|phoneName|imei|customerId|invoiceId|startDate|endDate|status";
    static inline constexpr const char* PRODUCT_IMAGES_HEADER =
        "productId|imagePath|isPrimary|displayOrder";
    static inline constexpr const char* PRODUCT_IMAGES_V1_HEADER =
        "id link";
    static inline constexpr const char* PRODUCT_MARKETING_HEADER =
        "productId|listPrice|saleStart|saleEnd|installmentEnabled|installmentNote|colors|warrantyMonths|returnDays";
    static inline constexpr const char* PRODUCT_DESCRIPTIONS_HEADER =
        "productId|shortHighlights|description";
    static inline constexpr const char* PRODUCT_REVIEWS_HEADER =
        "phoneId|customerId|customerName|ratingStars|comment|reviewTime";
    static inline constexpr const char* STORE_SETTINGS_HEADER =
        "key|value";
    static inline constexpr const char* SUPPLIER_PRODUCTS_HEADER =
        "supplierId|phoneId";

    static bool startsWithPrefix(const std::string& line, const std::string& prefix) {
        return line.size() >= prefix.size() && line.compare(0, prefix.size(), prefix) == 0;
    }

    static bool looksLikePrefixedRecord(const std::string& line, const std::string& idPrefix) {
        return startsWithPrefix(line, idPrefix + "-");
    }

    static FirstLineKind inspectFirstLine(
        const std::string& firstLine,
        const std::string& expectedV2Header,
        const std::vector<std::string>& legacyV1Headers,
        const std::string& filePath,
        const std::function<bool(const std::string&)>& looksLikeDataRow = nullptr) {
        if (firstLine == expectedV2Header) {
            return FirstLineKind::V2_HEADER;
        }
        for (const auto& legacy : legacyV1Headers) {
            if (firstLine == legacy) {
                return FirstLineKind::V1_HEADER;
            }
        }
        if (looksLikeDataRow && looksLikeDataRow(firstLine)) {
            std::cerr << "[Canh bao Schema] File '" << filePath
                      << "' thieu dong header chuan; dang doc dong 1 nhu du lieu thuc."
                      << std::endl;
            return FirstLineKind::DATA_ROW;
        }
        std::cerr << "[Canh bao Schema] Header khong khop trong '" << filePath
                  << "': nhan '" << firstLine << "', ky vong '" << expectedV2Header << "'."
                  << std::endl;
        return FirstLineKind::UNKNOWN_HEADER;
    }
};

#endif // SCHEMAHELPER_H
