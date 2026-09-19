#ifndef CUSTOMERREPOSITORY_H
#define CUSTOMERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Customer.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu khách hàng từ/vào tệp tin
class CustomerRepository {
private:
    std::string filePath;
    Vector<Customer> customers;

public:
    CustomerRepository(std::string path = "data/people/Customers.txt");

    bool loadFromFile();
    bool saveToFile() const;

    const Vector<Customer>& getAll() const;
    Vector<Customer>& getAll();

    Customer* findById(const std::string& id);
    const Customer* findById(const std::string& id) const;

    bool addCustomer(const Customer& cust);
    bool updateCustomer(const Customer& updatedCust);
    bool deleteCustomer(const std::string& id);
};

// ================= IMPLEMENTATION =================

inline CustomerRepository::CustomerRepository(std::string path) : filePath(path) {}

inline bool CustomerRepository::loadFromFile() {
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
    return true;
}

inline bool CustomerRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    file << "id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address\n";
    for (int i = 0; i < customers.getSize(); ++i) {
        file << customers[i].toFileString() << "\n";
    }
    file.close();
    return true;
}

inline const Vector<Customer>& CustomerRepository::getAll() const { 
    return customers; 
}

inline Vector<Customer>& CustomerRepository::getAll() { 
    return customers; 
}

inline Customer* CustomerRepository::findById(const std::string& id) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == id) {
            return &customers[i];
        }
    }
    return nullptr;
}

inline const Customer* CustomerRepository::findById(const std::string& id) const {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == id) {
            return &customers[i];
        }
    }
    return nullptr;
}

inline bool CustomerRepository::addCustomer(const Customer& cust) {
    customers.push_back(cust);
    return saveToFile();
}

inline bool CustomerRepository::updateCustomer(const Customer& updatedCust) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == updatedCust.getID()) {
            customers[i] = updatedCust;
            return saveToFile();
        }
    }
    return false;
}

inline bool CustomerRepository::deleteCustomer(const std::string& id) {
    for (int i = 0; i < customers.getSize(); ++i) {
        if (customers[i].getID() == id) {
            customers.removeAt(i);
            return saveToFile();
        }
    }
    return false;
}

#endif // CUSTOMERREPOSITORY_H
