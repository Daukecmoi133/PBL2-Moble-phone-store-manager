#ifndef PHONE_H
#define PHONE_H

#include <iostream>
#include <string>
#include "Common.h"
#include "../utils/StringHelper.h"

class Phone {
private:
    std::string id;             // Mã sản phẩm
    std::string name;           // Tên sản phẩm
    std::string brand;          // Thương hiệu (Apple, Samsung, Oppo,...)
    std::string manufacturer;   // Nhà sản xuất
    std::string operatingSystem;// Hệ điều hành
    std::string specifications; // Thông số kỹ thuật tóm tắt
    double importPrice;         // Giá nhập hàng
    double sellingPrice;        // Giá bán lẻ
    int quantity;               // Số lượng tồn kho

public:
    Phone() 
        : id(""), name(""), brand(""), manufacturer(""), 
          operatingSystem(""), specifications(""), 
          importPrice(0.0), sellingPrice(0.0), quantity(0) {}

    Phone(std::string id, std::string name, std::string brand, 
          std::string os, double importPrice, double sellingPrice, int quantity,
          std::string manufacturer = "", std::string specs = "")
        : id(id), name(name), brand(brand), manufacturer(manufacturer),
          operatingSystem(os), specifications(specs),
          importPrice(importPrice), sellingPrice(sellingPrice), quantity(quantity) {}

    // Getters
    std::string getID() const { return id; }
    std::string getName() const { return name; }
    std::string getBrand() const { return brand; }
    std::string getManufacturer() const { return manufacturer; }
    std::string getOperatingSystem() const { return operatingSystem; }
    std::string getSpecifications() const { return specifications; }
    double getImportPrice() const { return importPrice; }
    double getSellingPrice() const { return sellingPrice; }
    int getQuantity() const { return quantity; }

    // Setters
    void setID(const std::string& id) { this->id = id; }
    void setName(const std::string& name) { this->name = name; }
    void setBrand(const std::string& brand) { this->brand = brand; }
    void setManufacturer(const std::string& manufacturer) { this->manufacturer = manufacturer; }
    void setOperatingSystem(const std::string& os) { this->operatingSystem = os; }
    void setSpecifications(const std::string& specs) { this->specifications = specs; }
    void setImportPrice(double price) { this->importPrice = price; }
    void setSellingPrice(double price) { this->sellingPrice = price; }
    void setQuantity(int qty) { this->quantity = qty; }

    // Nghiệp vụ tồn kho
    bool isAvailable() const { return quantity > 0; }
    bool reduceStock(int qty) {
        if (qty <= quantity) {
            quantity -= qty;
            return true;
        }
        return false;
    }
    void addStock(int qty) {
        if (qty > 0) {
            quantity += qty;
        }
    }

    // Chuyển đối tượng thành dòng văn bản ghi vào Products.txt: id|name|brand|os|importPrice|sellingPrice|quantity
    std::string toFileString() const {
        return id + "|" + name + "|" + brand + "|" + operatingSystem + "|" +
               StringHelper::formatCurrency(importPrice) + "|" +
               StringHelper::formatCurrency(sellingPrice) + "|" +
               std::to_string(quantity);
    }
};

#endif // PHONE_H
