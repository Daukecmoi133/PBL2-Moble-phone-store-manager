#ifndef CUSTOMERREPOSITORY_H
#define CUSTOMERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Customer.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../utils/StringHelper.h"

class CustomerRepository {
private:
    std::string filePath;
    Vector<Customer> customers;
    HashTable<std::string, Customer> usernameTable;

    void rebuildIndices() {
        usernameTable.clear();
        for (int i = 0; i < customers.getSize(); ++i) {
            usernameTable.insert(customers[i].getAccount().getUsername(), customers[i]);
        }
    }

public:
    CustomerRepository(std::string path = "data/people/Customers.txt") : filePath(path) {}

    bool loadFromFile() {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[Cảnh báo] Không thể mở file: " << filePath << std::endl;
            return false;
        }

        customers.clear();
        std::string line;
        bool isHeader = true;

        while (std::getline(file, line)) {
            line = StringHelper::trim(line);
            if (line.empty() || line == "...") continue;

            if (isHeader) {
                isHeader = false;
                continue;
            }

            Vector<std::string> parts = StringHelper::split(line, '|');
            if (parts.getSize() >= 10) {
                std::string id = StringHelper::trim(parts[0]);
                Name name(StringHelper::trim(parts[1]), 
                          StringHelper::trim(parts[2]), 
                          StringHelper::trim(parts[3]));
                std::string phone = StringHelper::trim(parts[4]);
                std::string email = StringHelper::trim(parts[5]);
                std::string username = StringHelper::trim(parts[6]);
                std::string password = StringHelper::trim(parts[7]);
                std::string address = StringHelper::trim(parts[9]);

                Customer c(id, name, phone, email, address, username, password);
                customers.push_back(c);
            }
        }
        file.close();

        rebuildIndices();
        return true;
    }

    bool saveToFile() const {
        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        file << "id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address\n";
        for (int i = 0; i < customers.getSize(); ++i) {
            file << customers[i].toFileString() << "\n";
        }
        file.close();
        return true;
    }

    const Vector<Customer>& getAll() const { return customers; }

    Customer* findByUsername(const std::string& username) {
        return usernameTable.find(username);
    }

    Customer* authenticate(const std::string& username, const std::string& password) {
        Customer* cust = findByUsername(username);
        if (cust != nullptr && cust->getAccount().authenticate(username, password)) {
            return cust;
        }
        return nullptr;
    }

    bool addCustomer(const Customer& cust) {
        if (usernameTable.contains(cust.getAccount().getUsername())) {
            return false; // Đã tồn tại tên đăng nhập
        }
        customers.push_back(cust);
        usernameTable.insert(cust.getAccount().getUsername(), cust);
        saveToFile();
        return true;
    }
};

#endif // CUSTOMERREPOSITORY_H
