// ĐỐI TƯƠNG MÃ GIẢM GIÁ
#ifndef DISCOUNTCODE_H
#define DISCOUNTCODE_H

#include <string>
#include <algorithm>
#include <sstream>
#include "Common.h"
#include "../utils/DateTime.h"

class DiscountCode {
private:
    std::string code;               // Mã voucher (ví dụ: SALE10, GIAM500K)
    DiscountType type;              // PERCENTAGE hoặc FIXED_AMOUNT
    double value;                   // Giá trị (% hoặc số tiền)
    DateTime startDate;             // Ngày bắt đầu hiệu lực
    DateTime endDate;               // Ngày hết hạn
    double minOrderValue;           // Giá trị đơn hàng tối thiểu để áp dụng
    double maxDiscountValue;        // Mức giảm giá tối đa (dành cho loại %)
    int maxUsage;                   // Lượt sử dụng tối đa
    int usedCount;                  // Số lượt đã dùng

public:
    DiscountCode();
    DiscountCode(std::string code, DiscountType type, double value,
                 DateTime start, DateTime end, double minOrder = 0.0, 
                 double maxDiscount = 0.0, int maxUsage = 100, int usedCount = 0);

    std::string getCode() const;
    DiscountType getType() const;
    double getValue() const;
    DateTime getStartDate() const;
    DateTime getEndDate() const;
    double getMinOrderValue() const;
    double getMaxDiscountValue() const;
    int getMaxUsage() const;
    int getUsedCount() const;

    void setCode(const std::string& c);
    void setUsedCount(int u);

    bool isValid(const DateTime& currentTime = DateTime::now()) const;
    bool isApplicable(double orderValue) const;
    double calculateDiscount(double orderValue) const;
    void increaseUsage();

    // Đọc và phân tích một dòng từ Discount.txt
    // Format: code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount
    static DiscountCode fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

// Hàm dựng mặc định
inline DiscountCode::DiscountCode() 
    : code(""), type(DiscountType::PERCENTAGE), value(0.0),
      startDate(), endDate(), minOrderValue(0.0), maxDiscountValue(0.0),
      maxUsage(100), usedCount(0) {}

//Hàm dựng đủ tham số
inline DiscountCode::DiscountCode(std::string code, DiscountType type, double value,
                                  DateTime start, DateTime end, double minOrder, 
                                  double maxDiscount, int maxUsage, int usedCount)
    : code(code), type(type), value(value), startDate(start), endDate(end),
      minOrderValue(minOrder), maxDiscountValue(maxDiscount),
      maxUsage(maxUsage), usedCount(usedCount) {}

// Hàm getter tất cả thuộc tính của DiscountCode
inline std::string DiscountCode::getCode() const { return code; }                       // getter mã code
inline DiscountType DiscountCode::getType() const { return type; }                      // getter phương thức giảm giá
inline double DiscountCode::getValue() const { return value; }                          // getter giá trị được giảm tùy theo phương thức
inline DateTime DiscountCode::getStartDate() const { return startDate; }                // getter thời điểm mã có hiệu lực
inline DateTime DiscountCode::getEndDate() const { return endDate; }                    // getter thời điểm mã hết hiệu lực
inline double DiscountCode::getMinOrderValue() const { return minOrderValue; }          // getter giá trị đơn hàng nhỏ nhất mã có hiệu lực
inline double DiscountCode::getMaxDiscountValue() const { return maxDiscountValue; }    // getter giá trị mã giảm tối đa
inline int DiscountCode::getMaxUsage() const { return maxUsage; }                       // getter số lượng mã được phép sử dụng
inline int DiscountCode::getUsedCount() const { return usedCount; }                     // getter số lượng mã đã được sử dụng

// setter các thuộc tính
inline void DiscountCode::setCode(const std::string& c) { code = c; }                   // setter mã code           
inline void DiscountCode::setUsedCount(int u) { usedCount = u; }  
                      // setter số lượng mã được phép sử dụng
// Hàm xác minh mã còn hạn không
inline bool DiscountCode::isValid(const DateTime& currentTime) const {
    
    if (usedCount >= maxUsage) return false;
    if (currentTime < startDate || currentTime > endDate) return false;
    return true;
}
// Hàm xác minh mã hợp lệ không
inline bool DiscountCode::isApplicable(double orderValue) const {
    if (!isValid()) return false;
    return orderValue >= minOrderValue;
}
// Hàm tính giá trị mã có thể giảm
inline double DiscountCode::calculateDiscount(double orderValue) const {
     
    if (!isApplicable(orderValue)) return 0.0;
    double discount = 0.0;
    if (type == DiscountType::PERCENTAGE) {
        discount = orderValue * (value / 100.0);
        if (maxDiscountValue > 0.0 && discount > maxDiscountValue) {
            discount = maxDiscountValue;
        }
    } else {
        discount = value;
        if (discount > orderValue) {
            discount = orderValue;
        }
    }
    return discount;
}
// Hàm tăng số lượng sử dụng khi mã được dùng thành công
inline void DiscountCode::increaseUsage() {
    
    if (usedCount < maxUsage) {
        ++usedCount;
    }
}

// Phân tích dòng dữ liệu từ Discount.txt thành đối tượng DiscountCode
// Format chuẩn: code|type|value|startDate|endDate|minOrder|maxDiscount|maxUsage|usedCount
// Hỗ trợ định dạng cũ: ID TYPE VALUE (ví dụ: DIS00001 1 30)
inline DiscountCode DiscountCode::fromFileLine(const std::string& line) {
    if (line.find('|') != std::string::npos) {
        Vector<std::string> parts = StringHelper::split(line, '|');
        if (parts.getSize() < 9) return DiscountCode(); // Dòng không hợp lệ

        std::string code   = StringHelper::trim(parts[0]);
        int typeInt        = std::stoi(StringHelper::trim(parts[1])); // 0=PERCENTAGE, 1=FIXED_AMOUNT
        double value       = std::stod(StringHelper::trim(parts[2]));
        DateTime startDate = DateTime::fromDateString(StringHelper::trim(parts[3]));
        DateTime endDate   = DateTime::fromDateString(StringHelper::trim(parts[4]));
        double minOrder    = std::stod(StringHelper::trim(parts[5]));
        double maxDiscount = std::stod(StringHelper::trim(parts[6]));
        int maxUsage       = std::stoi(StringHelper::trim(parts[7]));
        int usedCount      = std::stoi(StringHelper::trim(parts[8]));

        return DiscountCode(code, static_cast<DiscountType>(typeInt), value,
                            startDate, endDate, minOrder, maxDiscount, maxUsage, usedCount);
    } else {
        // Tương thích ngược với định dạng mẫu cũ: ID TYPE VALUE (ví dụ: DIS00001 1 30)
        std::stringstream ss(line);
        std::string code;
        int typeInt = 0;
        double val = 0.0;
        if (ss >> code >> typeInt >> val) {
            DiscountType dt = (typeInt == 2) ? DiscountType::FIXED_AMOUNT : DiscountType::PERCENTAGE;
            return DiscountCode(code, dt, val,
                                DateTime(1, 1, 2026, 0, 0, 0),
                                DateTime(31, 12, 2026, 23, 59, 59),
                                0.0, 5000000.0, 100, 0);
        }
        return DiscountCode();
    }
}

#endif // DISCOUNTCODE_H
