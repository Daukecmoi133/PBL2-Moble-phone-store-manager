#ifndef COMMON_H
#define COMMON_H

#include <string>

// Khởi tạo kiểu dữ liệu cho Tên con người
struct Name {
    std::string firstName;
    std::string middleName;
    std::string lastName;

    Name();
    Name(std::string first, std::string middle, std::string last);

    std::string getFullName() const;
};

// ================= IMPLEMENTATION NAME =================

// Hàm dựng
inline Name::Name() : firstName(""), middleName(""), lastName("") {}

// Hàm dựng đủ đối số
inline Name::Name(std::string first, std::string middle, std::string last)
    : firstName(first), middleName(middle), lastName(last) {}

// Hàm trả về chuỗi đầy đủ tên của người
inline std::string Name::getFullName() const {
    std::string full = "";
    if (!firstName.empty()) full += firstName;
    if (!middleName.empty()) {
        if (!full.empty()) full += " ";
        full += middleName;
    }
    if (!lastName.empty()) {
        if (!full.empty()) full += " ";
        full += lastName;
    }
    return full;
}

// ================= ENUMS & ENUM HELPERS =================

// TÊN ROLE
enum class RoleType {
    ADMIN,
    SALE_EMPLOYEE,
    WAREHOUSE_EMPLOYEE,
    CUSTOMER
};

// Trả về tên Role dưới giải nghĩa tiếng Việt
inline std::string roleTypeToString(RoleType role) {
    switch (role) {
        case RoleType::ADMIN: return "Quản trị viên (Admin)";
        case RoleType::SALE_EMPLOYEE: return "Nhân viên Bán hàng";
        case RoleType::WAREHOUSE_EMPLOYEE: return "Nhân viên Kho";
        case RoleType::CUSTOMER: return "Khách hàng";
        default: return "Chưa xác định";
    }
}

// TÊN QUYỀN
enum class Permission {
    PRODUCT_VIEW, 
    PRODUCT_CREATE,
    PRODUCT_UPDATE,
    PRODUCT_DELETE,

    ORDER_VIEW,
    ORDER_CREATE,

    INVOICE_CREATE,

    STOCK_VIEW,
    STOCK_UPDATE,

    EMPLOYEE_VIEW,
    EMPLOYEE_CREATE
};

// TÊN TRẠNG THÁI ĐƠN HÀNG 
enum class OrderStatus {
    PENDING,        // Đang chờ xử lý
    CONFIRMED,      // Đã xác nhận
    PREPARING,      // Đang chuẩn bị hàng
    READY_FOR_PICKUP, // Sẵn sàng nhận hàng
    SHIPPING,       // Đang giao hàng
    COMPLETED,      // Hoàn tất
    CANCELLED       // Đã hủy
};

// Trả về trạng thái đơn hàng với dịch nghĩa tiếng Việt
inline std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::PENDING: return "Chờ xử lý";
        case OrderStatus::CONFIRMED: return "Đã xác nhận";
        case OrderStatus::PREPARING: return "Đang đóng gói";
        case OrderStatus::READY_FOR_PICKUP: return "Sẵn sàng nhận hàng";
        case OrderStatus::SHIPPING: return "Đang giao hàng";
        case OrderStatus::COMPLETED: return "Hoàn thành";
        case OrderStatus::CANCELLED: return "Đã hủy";
        default: return "Không rõ";
    }
}

// PHƯƠNG THỨC NHẬN HÀNG
enum class DeliveryMethod {
    PICKUP,         // Nhận tại cửa hàng
    DELIVERY        // Giao hàng tận nơi
};

// Chuyển về phương thức nhận hàng với dịch nghĩa tiếng Việt
inline std::string deliveryMethodToString(DeliveryMethod method) {
    switch (method) {
        case DeliveryMethod::PICKUP: return "Nhận tại cửa hàng";
        case DeliveryMethod::DELIVERY: return "Giao tận nơi";
        default: return "Không rõ";
    }
}

// PHƯƠNG THỨC THANH TOÁN
enum class PaymentMethod {
    CASH,           // Tiền mặt
    BANK_TRANSFER,  // Chuyển khoản ngân hàng
    CREDIT_CARD     // Thẻ tín dụng
};

// Chuyển về phương thức thanh toán với dịch nghĩa tiếng Việt
inline std::string paymentMethodToString(PaymentMethod method) {
    switch (method) {
        case PaymentMethod::CASH: return "Tiền mặt";
        case PaymentMethod::BANK_TRANSFER: return "Chuyển khoản";
        case PaymentMethod::CREDIT_CARD: return "Thẻ tín dụng";
        default: return "Không rõ";
    }
}

// PHƯƠNG THỨC GIẢM GIÁ
enum class DiscountType {
    PERCENTAGE,     // Giảm theo %
    FIXED_AMOUNT    // Giảm số tiền cố định
};

#endif // COMMON_H
