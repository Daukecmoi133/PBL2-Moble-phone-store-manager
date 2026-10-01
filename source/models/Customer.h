// ĐỐI TƯỢNG KHÁCH HÀNG
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "People.h"
#include "Account.h"

class Customer : public People {
private:
    int points;         // Điểm tích lũy thành viên
    Account account;    // Tài khoản đăng nhập

public:
    Customer();
    Customer(std::string id, Name name, std::string phone, std::string email, 
             std::string address, std::string username, std::string password, int points = 0);

    int getPoints() const;
    void setPoints(int p);
    void addPoints(int p);
    bool deductPoints(int p);

    Account& getAccount();
    const Account& getAccount() const;
    void setAccount(const Account& acc);

    // Định dạng lưu file Customers.txt
    // Format: id|firstName|middleName|lastName|phoneNumber|email|username|password|points|address
    std::string toFileString() const;

    // Đọc và phân tích một dòng từ Customers.txt
    static Customer fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

// Hàm dựng mặc định
inline Customer::Customer() 
    : People(), points(0), account("", "", RoleType::CUSTOMER) {}

// Hàm dựng đủ tham số
inline Customer::Customer(std::string id, Name name, std::string phone, std::string email, 
                          std::string address, std::string username, std::string password, int points)
    : People(id, name, phone, email, address), points(points),
      account(username, password, RoleType::CUSTOMER) {}

// Hàm setter / getter cho điểm tích lũy
inline int Customer::getPoints() const { return points; }
inline void Customer::setPoints(int p) { points = p; }

// Hàm tăng điểm tích lũy lên +p
inline void Customer::addPoints(int p) {
    if (p > 0) points += p;
}

// Hàm giảm điểm tích lũy đi -p
inline bool Customer::deductPoints(int p) {
    if (p <= points) {
        points -= p;
        return true;
    }
    return false;
}

// Hàm trả về tài khoản của khách hàng - cho phép thay đổi tài khoản đã trả về
inline Account& Customer::getAccount() { return account; }

// Hàm trả về tài khoản của khách hàng - không cho phép thay đổi tài khoản đã trả về
inline const Account& Customer::getAccount() const { return account; }

// Hàm setter tài khoản của khách hàng
inline void Customer::setAccount(const Account& acc) { account = acc; }

// Hàm trả về cấu trúc dữ liệu trong file .txt
// Format: id|firstName|middleName|lastName|phoneNumber|email|username|password|points|address
inline std::string Customer::toFileString() const {
    return id + "|" + name.firstName + "|" + name.middleName + "|" + name.lastName + "|" +
           phoneNumber + "|" + email + "|" + account.getUsername() + "|" + account.getPassword() + "|" +
           std::to_string(points) + "|" + address;
}

// Phân tích dòng dữ liệu từ Customers.txt thành đối tượng Customer
inline Customer Customer::fromFileLine(const std::string& line) {
    Vector<std::string> parts = StringHelper::split(line, '|');
    if (parts.getSize() < 10) return Customer(); // Dòng không hợp lệ

    std::string id       = StringHelper::trim(parts[0]);
    Name name(StringHelper::trim(parts[1]), StringHelper::trim(parts[2]), StringHelper::trim(parts[3]));
    std::string phone    = StringHelper::trim(parts[4]);
    std::string email    = StringHelper::trim(parts[5]);
    std::string username = StringHelper::trim(parts[6]);
    std::string password = StringHelper::trim(parts[7]);
    int points           = std::stoi(StringHelper::trim(parts[8]));
    std::string address  = StringHelper::trim(parts[9]);

    return Customer(id, name, phone, email, address, username, password, points);
}

#endif // CUSTOMER_H
