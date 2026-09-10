#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "People.h"
#include "Account.h"

class Customer : public People {
private:
    int points;         // Điểm tích lũy thành viên
    Account account;    // Tài khoản đăng nhập

public:
    Customer() : People(), points(0), account("", "", RoleType::CUSTOMER) {}

    Customer(std::string id, Name name, std::string phone, std::string email, 
             std::string address, std::string username, std::string password, int points = 0)
        : People(id, name, phone, email, address), points(points),
          account(username, password, RoleType::CUSTOMER) {}

    int getPoints() const { return points; }
    void setPoints(int p) { points = p; }

    void addPoints(int p) {
        if (p > 0) points += p;
    }

    bool deductPoints(int p) {
        if (p <= points) {
            points -= p;
            return true;
        }
        return false;
    }

    Account& getAccount() { return account; }
    const Account& getAccount() const { return account; }
    void setAccount(const Account& acc) { account = acc; }

    // Định dạng lưu file Customers.txt: id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address
    std::string toFileString() const {
        return id + "|" + name.firstName + "|" + name.middleName + "|" + name.lastName + "|" +
               phoneNumber + "|" + email + "|" + account.getUsername() + "|******|4|" + address;
    }
};

#endif // CUSTOMER_H
