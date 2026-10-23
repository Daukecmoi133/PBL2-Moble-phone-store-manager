#ifndef WARRANTY_VIEWMODEL_H
#define WARRANTY_VIEWMODEL_H

#include <string>

enum class WarrantyStatus {
    RECEIVED,    // Mới tiếp nhận
    PROCESSING,  // Đang kiểm tra / sửa chữa
    RESOLVED,    // Đã hoàn tất bảo hành
    REJECTED     // Từ chối (hết hạn / rơi vỡ ngoài điều kiện)
};

inline std::string warrantyStatusToString(WarrantyStatus s) {
    switch (s) {
        case WarrantyStatus::RECEIVED: return "Mới tiếp nhận";
        case WarrantyStatus::PROCESSING: return "Đang xử lý";
        case WarrantyStatus::RESOLVED: return "Đã hoàn tất";
        case WarrantyStatus::REJECTED: return "Từ chối bảo hành";
        default: return "Chưa rõ";
    }
}

// ViewModel hiển thị một yêu cầu bảo hành
struct WarrantyItemViewModel {
    std::string id;
    std::string orderId;
    std::string phoneId;
    std::string phoneName;
    std::string customerId;
    std::string customerName;
    std::string customerPhone;
    std::string requestDate;
    std::string issueDescription;
    WarrantyStatus status;
    std::string statusText;
    std::string technicianNote;
    std::string resolution;

    WarrantyItemViewModel() : status(WarrantyStatus::RECEIVED) {}
};

// Form tạo yêu cầu bảo hành
struct CreateWarrantyRequest {
    std::string customerId;
    std::string orderId;
    std::string phoneId;
    std::string issueDescription;
};

#endif // WARRANTY_VIEWMODEL_H
