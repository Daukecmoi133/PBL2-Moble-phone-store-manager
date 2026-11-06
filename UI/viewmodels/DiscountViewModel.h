#ifndef DISCOUNT_VIEWMODEL_H
#define DISCOUNT_VIEWMODEL_H

#include <string>

// ViewModel hiển thị một mã giảm giá
struct DiscountItemViewModel {
    std::string id;
    std::string code;
    std::string typeName; // "Giảm %" hoặc "Số tiền cố định"
    double value;
    std::string formattedValue;
    double minOrderValue;
    double maxDiscountAmount;
    std::string expiryDate;
    int usedCount;
    int maxUsage;
    bool isValid;
    std::string description;

    DiscountItemViewModel()
        : value(0.0), minOrderValue(0.0), maxDiscountAmount(0.0),
          usedCount(0), maxUsage(100), isValid(true) {}
};

// Form tạo / sửa mã giảm giá
struct DiscountFormRequest {
    std::string id;
    std::string code;
    int type; // 0: %, 1: Cố định
    double value;
    double minOrderValue;
    double maxDiscountAmount;
    std::string expiryDate;
    int maxUsage;
    std::string description;
};

#endif // DISCOUNT_VIEWMODEL_H
