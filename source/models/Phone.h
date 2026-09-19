// ĐỐI TƯỢNG SẢN PHẨM ĐIỆN THOẠI
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
    Phone(); // Hàm dựng mặc định
    Phone(std::string id, std::string name, std::string brand, 
          std::string os, double importPrice, double sellingPrice, int quantity,
          std::string manufacturer = "", std::string specs = ""); // Hàm dựng đủ tham số

    // Getters
    std::string getID() const; // Hàm trả về mã sản phẩm
    std::string getName() const; // Hàm trả về tên sản phẩm
    std::string getBrand() const; // Hàm trả về thương hiệu
    std::string getManufacturer() const; // Hàm trả về nhà sản xuất
    std::string getOperatingSystem() const; // Hàm trả về hệ điều hành
    std::string getSpecifications() const; // Hàm trả về thông số kỹ thuật
    double getImportPrice() const; // Hàm trả về giá nhập
    double getSellingPrice() const; // Hàm trả về giá bán lẻ
    int getQuantity() const; // Hàm trả về số lượng tồn kho

    // Setters
    void setID(const std::string& id); // Hàm thiết lập mã sản phẩm
    void setName(const std::string& name); // Hàm thiết lập tên sản phẩm
    void setBrand(const std::string& brand); // Hàm thiết lập thương hiệu
    void setManufacturer(const std::string& manufacturer); // Hàm thiết lập nhà sản xuất
    void setOperatingSystem(const std::string& os); // Hàm thiết lập hệ điều hành
    void setSpecifications(const std::string& specs); // Hàm thiết lập thông số kỹ thuật
    void setImportPrice(double price); // Hàm thiết lập giá nhập
    void setSellingPrice(double price); // Hàm thiết lập giá bán
    void setQuantity(int qty); // Hàm thiết lập số lượng tồn kho

    // Nghiệp vụ tồn kho
    bool isAvailable() const; // Hàm kiểm tra sản phẩm còn hàng để bán không
    bool reduceStock(int qty); // Hàm trừ bớt số lượng tồn kho (khi bán hàng thành công)
    void addStock(int qty); // Hàm cộng thêm số lượng tồn kho (khi nhập thêm hàng)

    // Chuyển đối tượng thành dòng văn bản ghi vào Products.txt
    std::string toFileString() const; // Hàm xuất chuỗi dữ liệu theo định dạng lưu file Products.txt
};

// ================= IMPLEMENTATION =================

// Hàm dựng mặc định
inline Phone::Phone() 
    : id(""), name(""), brand(""), manufacturer(""), 
      operatingSystem(""), specifications(""), 
      importPrice(0.0), sellingPrice(0.0), quantity(0) {}

// Hàm dựng đủ tham số
inline Phone::Phone(std::string id, std::string name, std::string brand, 
                    std::string os, double importPrice, double sellingPrice, int quantity,
                    std::string manufacturer, std::string specs)
    : id(id), name(name), brand(brand), manufacturer(manufacturer),
      operatingSystem(os), specifications(specs),
      importPrice(importPrice), sellingPrice(sellingPrice), quantity(quantity) {}

// Hàm getter các thuộc tính
inline std::string Phone::getID() const { return id; }
inline std::string Phone::getName() const { return name; }
inline std::string Phone::getBrand() const { return brand; }
inline std::string Phone::getManufacturer() const { return manufacturer; }
inline std::string Phone::getOperatingSystem() const { return operatingSystem; }
inline std::string Phone::getSpecifications() const { return specifications; }
inline double Phone::getImportPrice() const { return importPrice; }
inline double Phone::getSellingPrice() const { return sellingPrice; }
inline int Phone::getQuantity() const { return quantity; }

// Hàm setter các thuộc tính
inline void Phone::setID(const std::string& id) { this->id = id; }
inline void Phone::setName(const std::string& name) { this->name = name; }
inline void Phone::setBrand(const std::string& brand) { this->brand = brand; }
inline void Phone::setManufacturer(const std::string& manufacturer) { this->manufacturer = manufacturer; }
inline void Phone::setOperatingSystem(const std::string& os) { this->operatingSystem = os; }
inline void Phone::setSpecifications(const std::string& specs) { this->specifications = specs; }
inline void Phone::setImportPrice(double price) { this->importPrice = price; }
inline void Phone::setSellingPrice(double price) { this->sellingPrice = price; }
inline void Phone::setQuantity(int qty) { this->quantity = qty; }

// Kiểm tra số lượng tồn kho còn lớn hơn 0 không
inline bool Phone::isAvailable() const { return quantity > 0; }

// Giảm số lượng tồn kho khi mua hàng (trả về false nếu không đủ hàng)
inline bool Phone::reduceStock(int qty) {
    if (qty <= quantity) {
        quantity -= qty;
        return true;
    }
    return false;
}

// Tăng số lượng tồn kho khi nhập thêm hàng
inline void Phone::addStock(int qty) {
    if (qty > 0) {
        quantity += qty;
    }
}

// Định dạng lưu trữ dữ liệu dòng vào tệp tin Products.txt
inline std::string Phone::toFileString() const {
    return id + "|" + name + "|" + brand + "|" + operatingSystem + "|" +
           StringHelper::formatCurrency(importPrice) + "|" +
           StringHelper::formatCurrency(sellingPrice) + "|" +
           std::to_string(quantity);
}

#endif // PHONE_H
