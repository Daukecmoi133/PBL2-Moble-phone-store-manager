#ifndef PEOPLE_H
#define PEOPLE_H

#include <string>
#include "Common.h"

class People {
protected:
    std::string id;
    Name name;
    std::string phoneNumber;
    std::string email;
    std::string address;

public:
    People() : id(""), name(), phoneNumber(""), email(""), address("") {}

    People(std::string id, Name name, std::string phone, std::string email, std::string address)
        : id(id), name(name), phoneNumber(phone), email(email), address(address) {}

    virtual ~People() = default;

    // Getters
    std::string getID() const { return id; }
    Name getName() const { return name; }
    std::string getFullName() const { return name.getFullName(); }
    std::string getPhoneNumber() const { return phoneNumber; }
    std::string getEmail() const { return email; }
    std::string getAddress() const { return address; }

    // Setters
    void setID(const std::string& id) { this->id = id; }
    void setName(const Name& name) { this->name = name; }
    void setPhoneNumber(const std::string& phone) { this->phoneNumber = phone; }
    void setEmail(const std::string& email) { this->email = email; }
    void setAddress(const std::string& address) { this->address = address; }
};

#endif // PEOPLE_H
