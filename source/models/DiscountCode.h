#ifndef DISCOUNTCODE_H
#define DISCOUNTCODE_H

#include <string>
#include <algorithm>
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
    DiscountCode() 
        : code(""), type(DiscountType::PERCENTAGE), value(0.0),
          startDate(), endDate(), minOrderValue(0.0), maxDiscountValue(0.0),
          maxUsage(100), usedCount(0) {}

    DiscountCode(std::string code, DiscountType type, double value,
                 DateTime start, DateTime end, double minOrder = 0.0, 
                 double maxDiscount = 0.0, int maxUsage = 100, int usedCount = 0)
        : code(code), type(type), value(value), startDate(start), endDate(end),
          minOrderValue(minOrder), maxDiscountValue(maxDiscount),
          maxUsage(maxUsage), usedCount(usedCount) {}

    std::string getCode() const { return code; }
    DiscountType getType() const { return type; }
    double getValue() const { return value; }
    DateTime getStartDate() const { return startDate; }
    DateTime getEndDate() const { return endDate; }
    double getMinOrderValue() const { return minOrderValue; }
    double getMaxDiscountValue() const { return maxDiscountValue; }
    int getMaxUsage() const { return maxUsage; }
    int getUsedCount() const { return usedCount; }

    void setCode(const std::string& c) { code = c; }
    void setUsedCount(int u) { usedCount = u; }

    bool isValid(const DateTime& currentTime = DateTime::now()) const {
        if (usedCount >= maxUsage) return false;
        if (currentTime < startDate || currentTime > endDate) return false;
        return true;
    }

    bool isApplicable(double orderValue) const {
        if (!isValid()) return false;
        return orderValue >= minOrderValue;
    }

    double calculateDiscount(double orderValue) const {
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

    void increaseUsage() {
        if (usedCount < maxUsage) {
            ++usedCount;
        }
    }
};

#endif // DISCOUNTCODE_H
