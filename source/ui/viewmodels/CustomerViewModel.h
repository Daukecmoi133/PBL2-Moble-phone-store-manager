#ifndef CUSTOMER_VIEWMODEL_H
#define CUSTOMER_VIEWMODEL_H

#include <string>

// ViewModel hiển thị một khách hàng
struct CustomerItemViewModel {
    std::string id;
    std::string fullName;
    std::string firstName;
    std::string middleName;
    std::string lastName;
    std::string phone;
    std::string email;
    std::string username;
    int points;
    std::string address;
    std::string membershipTier; // "Đồng", "Bạc", "Vàng", "Kim Cương"

    CustomerItemViewModel() : points(0), membershipTier("Đồng") {}

    void updateTier() {
        if (points >= 500) membershipTier = "Kim Cương (VIP)";
        else if (points >= 200) membershipTier = "Vàng";
        else if (points >= 50) membershipTier = "Bạc";
        else membershipTier = "Đồng";
    }
};

// Form tạo / sửa thông tin khách hàng
struct CustomerFormRequest {
    std::string id;
    std::string firstName;
    std::string middleName;
    std::string lastName;
    std::string phone;
    std::string email;
    std::string address;
    std::string username;
    std::string password;
};

#endif // CUSTOMER_VIEWMODEL_H
