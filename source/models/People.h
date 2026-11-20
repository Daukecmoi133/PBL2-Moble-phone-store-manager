// ĐỐI TƯỢNG CON NGƯỜI (LỚP CƠ SỞ TRỪU TƯỢNG CHO CUSTOMER, EMPLOYEE, SUPPLIER)
#ifndef PEOPLE_H
#define PEOPLE_H

#include <string>
#include "Common.h"

class People {
protected:
    std::string id;          // Mã định danh cá nhân (ID)
    Name name;               // Họ và tên
    std::string phoneNumber; // Số điện thoại liên lạc
    std::string email;       // Địa chỉ thư điện tử (email)
    std::string address;     // Địa chỉ liên hệ / nơi ở

public:
    People(); // Hàm dựng mặc định
    People(std::string id, Name name, std::string phone, std::string email, std::string address); // Hàm dựng đủ tham số
    virtual ~People() = default; // Hàm hủy ảo để hủy an toàn các lớp con kế thừa
    People(const People&) = default;
    People& operator=(const People&) = default;

    // Getters
    std::string getID() const; // Hàm trả về mã định danh
    Name getName() const; // Hàm trả về cấu trúc Name
    std::string getFullName() const; // Hàm trả về chuỗi họ và tên đầy đủ
    std::string getPhoneNumber() const; // Hàm trả về số điện thoại
    std::string getEmail() const; // Hàm trả về địa chỉ email
    std::string getAddress() const; // Hàm trả về địa chỉ nơi ở

    // Setters
    void setID(const std::string& id); // Hàm thiết lập mã định danh
    void setName(const Name& name); // Hàm thiết lập họ và tên
    void setPhoneNumber(const std::string& phone); // Hàm thiết lập số điện thoại
    void setEmail(const std::string& email); // Hàm thiết lập email
    void setAddress(const std::string& address); // Hàm thiết lập địa chỉ
};

// ================= IMPLEMENTATION =================

// Hàm dựng mặc định
inline People::People() 
    : id(""), name(), phoneNumber(""), email(""), address("") {}

// Hàm dựng đủ tham số
inline People::People(std::string id, Name name, std::string phone, std::string email, std::string address)
    : id(id), name(name), phoneNumber(phone), email(email), address(address) {}

// Hàm getters
inline std::string People::getID() const { return id; }
inline Name People::getName() const { return name; }
inline std::string People::getFullName() const { return name.getFullName(); }
inline std::string People::getPhoneNumber() const { return phoneNumber; }
inline std::string People::getEmail() const { return email; }
inline std::string People::getAddress() const { return address; }

// Hàm setters
inline void People::setID(const std::string& id) { this->id = id; }
inline void People::setName(const Name& name) { this->name = name; }
inline void People::setPhoneNumber(const std::string& phone) { this->phoneNumber = phone; }
inline void People::setEmail(const std::string& email) { this->email = email; }
inline void People::setAddress(const std::string& address) { this->address = address; }

#endif // PEOPLE_H
