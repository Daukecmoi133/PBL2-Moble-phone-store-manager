// ĐỐI TƯỢNG SẢN PHẨM ĐIỆN THOẠI
#ifndef PHONE_H
#define PHONE_H

#include "../utils/StringHelper.h"
#include "../utils/TextCodec.h"
#include "Common.h"
#include <iostream>
#include <string>

class Phone {
private:
  std::string id;              // Mã sản phẩm
  std::string name;            // Tên sản phẩm
  std::string brand;           // Thương hiệu (Apple, Samsung, Oppo,...)
  std::string manufacturer;    // Nhà sản xuất
  std::string operatingSystem; // Hệ điều hành
  std::string specifications;  // Thông số kỹ thuật tóm tắt
  std::string detailedSpecifications; // Thông số kỹ thuật chi tiết từ file riêng
  double importPrice;          // Giá nhập hàng
  double sellingPrice;         // Giá bán lẻ
  int quantity;                // Số lượng tồn kho

public:
  Phone(); // Hàm dựng mặc định
  Phone(std::string id, std::string name, std::string brand, std::string os,
        double importPrice, double sellingPrice, int quantity,
        std::string manufacturer = "",
        std::string specs = ""); // Hàm dựng đủ tham số

  // Getters
  std::string getID() const;              // Hàm trả về mã sản phẩm
  std::string getName() const;            // Hàm trả về tên sản phẩm
  std::string getBrand() const;           // Hàm trả về thương hiệu
  std::string getManufacturer() const;    // Hàm trả về nhà sản xuất
  std::string getOperatingSystem() const; // Hàm trả về hệ điều hành
  std::string getSpecifications() const;  // Hàm trả về thông số kỹ thuật
  std::string getDetailedSpecifications() const;
  double getImportPrice() const;          // Hàm trả về giá nhập
  double getSellingPrice() const;         // Hàm trả về giá bán lẻ
  int getQuantity() const;                // Hàm trả về số lượng tồn kho

  // Setters
  void setID(const std::string &id);       // Hàm thiết lập mã sản phẩm
  void setName(const std::string &name);   // Hàm thiết lập tên sản phẩm
  void setBrand(const std::string &brand); // Hàm thiết lập thương hiệu
  void setManufacturer(
      const std::string &manufacturer);           // Hàm thiết lập nhà sản xuất
  void setOperatingSystem(const std::string &os); // Hàm thiết lập hệ điều hành
  void setSpecifications(
      const std::string &specs);      // Hàm thiết lập thông số kỹ thuật
  void setDetailedSpecifications(const std::string &specs);
  void setImportPrice(double price);  // Hàm thiết lập giá nhập
  void setSellingPrice(double price); // Hàm thiết lập giá bán
  void setQuantity(int qty);          // Hàm thiết lập số lượng tồn kho

  // Nghiệp vụ tồn kho
  bool isAvailable() const; // Hàm kiểm tra sản phẩm còn hàng để bán không
  bool reduceStock(
      int qty); // Hàm trừ bớt số lượng tồn kho (khi bán hàng thành công)
  void addStock(int qty); // Hàm cộng thêm số lượng tồn kho (khi nhập thêm hàng)

  // Chuyển đối tượng thành dòng văn bản ghi vào Products.txt
  std::string toFileString() const; // Hàm xuất chuỗi dữ liệu theo định dạng lưu file Products.txt

  // Đọc và phân tích một dòng từ Products.txt, trả về đối tượng Phone
  static Phone fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

// Hàm dựng mặc định
inline Phone::Phone()
    : id(""), name(""), brand(""), manufacturer(""), operatingSystem(""),
      specifications(""), detailedSpecifications(""), importPrice(0.0),
      sellingPrice(0.0), quantity(0) {}

// Hàm dựng đủ tham số
inline Phone::Phone(std::string id, std::string name, std::string brand,
                    std::string os, double importPrice, double sellingPrice,
                    int quantity, std::string manufacturer, std::string specs)
    : id(id), name(name), brand(brand), manufacturer(manufacturer),
      operatingSystem(os), specifications(specs), detailedSpecifications(""),
      importPrice(importPrice), sellingPrice(sellingPrice), quantity(quantity) {}

// Hàm getter các thuộc tính
inline std::string Phone::getID() const { return id; }
inline std::string Phone::getName() const { return name; }
inline std::string Phone::getBrand() const { return brand; }
inline std::string Phone::getManufacturer() const { return manufacturer; }
inline std::string Phone::getOperatingSystem() const { return operatingSystem; }
inline std::string Phone::getSpecifications() const { return specifications; }
inline std::string Phone::getDetailedSpecifications() const {
  return detailedSpecifications;
}
inline double Phone::getImportPrice() const { return importPrice; }
inline double Phone::getSellingPrice() const { return sellingPrice; }
inline int Phone::getQuantity() const { return quantity; }

// Hàm setter các thuộc tính
inline void Phone::setID(const std::string &id) { this->id = id; }
inline void Phone::setName(const std::string &name) { this->name = name; }
inline void Phone::setBrand(const std::string &brand) { this->brand = brand; }
inline void Phone::setManufacturer(const std::string &manufacturer) {
  this->manufacturer = manufacturer;
}
inline void Phone::setOperatingSystem(const std::string &os) {
  this->operatingSystem = os;
}
inline void Phone::setSpecifications(const std::string &specs) {
  this->specifications = specs;
}
inline void Phone::setDetailedSpecifications(const std::string &specs) {
  detailedSpecifications = specs;
}
inline void Phone::setImportPrice(double price) { this->importPrice = price; }
inline void Phone::setSellingPrice(double price) { this->sellingPrice = price; }
inline void Phone::setQuantity(int qty) { this->quantity = qty; }

// Kiểm tra số lượng tồn kho còn lớn hơn 0 không
inline bool Phone::isAvailable() const { return quantity > 0; }

// Giảm số lượng tồn kho khi mua hàng (trả về false nếu qty <= 0 hoặc không đủ hàng)
inline bool Phone::reduceStock(int qty) {
  if (qty <= 0 || qty > quantity) {
    return false;
  }
  quantity -= qty;
  return true;
}

// Tăng số lượng tồn kho khi nhập thêm hàng
inline void Phone::addStock(int qty) {
  if (qty > 0) {
    quantity += qty;
  }
}

// Định dạng lưu trữ dữ liệu dòng vào tệp tin Products.txt (giá nguyên VND, có escape TextCodec)
// Format: id|phoneName|brand|operatingSystem|importPrice|sellingPrice|quantity|manufacturer|specifications
inline std::string Phone::toFileString() const {
  return TextCodec::escapeField(id) + "|" +
         TextCodec::escapeField(name) + "|" +
         TextCodec::escapeField(brand) + "|" +
         TextCodec::escapeField(operatingSystem) + "|" +
         StringHelper::formatMoneyDisk(importPrice) + "|" +
         StringHelper::formatMoneyDisk(sellingPrice) + "|" +
         std::to_string(quantity) + "|" +
         TextCodec::escapeField(manufacturer) + "|" +
         TextCodec::escapeField(specifications);
}

// Phân tích dòng dữ liệu từ Products.txt thành đối tượng Phone
// Hỗ trợ cả định dạng 7 trường (cũ) và 9 trường (mới có manufacturer, specs)
inline Phone Phone::fromFileLine(const std::string& line) {
  Vector<std::string> parts = TextCodec::splitFields(line, '|');
  if (parts.getSize() < 7) return Phone(); // Dòng không hợp lệ

  Phone p;
  p.setID(StringHelper::trim(TextCodec::unescape(parts[0])));
  p.setName(TextCodec::unescape(parts[1]));
  p.setBrand(TextCodec::unescape(parts[2]));
  p.setOperatingSystem(TextCodec::unescape(parts[3]));
  p.setImportPrice(StringHelper::parsePrice(parts[4]));
  p.setSellingPrice(StringHelper::parsePrice(parts[5]));
  try {
    p.setQuantity(std::stoi(StringHelper::trim(parts[6])));
  } catch (...) {
    return Phone();
  }
  if (parts.getSize() >= 8) p.setManufacturer(TextCodec::unescape(parts[7]));
  if (parts.getSize() >= 9) p.setSpecifications(TextCodec::unescape(parts[8]));
  return p;
}

#endif // PHONE_H
