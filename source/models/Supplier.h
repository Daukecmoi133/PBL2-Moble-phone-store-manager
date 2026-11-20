// ĐỐI TƯỢNG NHÀ CUNG CẤP (KẾ THỪA TỪ PEOPLE)
#ifndef SUPPLIER_H
#define SUPPLIER_H

#include "People.h"
#include "../utils/TextCodec.h"

class Supplier : public People {
private:
    std::string companyName; // Tên công ty / doanh nghiệp cung cấp
    std::string taxCode;     // Mã số thuế doanh nghiệp

public:
    Supplier(); // Hàm dựng mặc định
    Supplier(std::string id, std::string companyName, std::string taxCode, 
             Name contactPerson, std::string phone, std::string email, std::string address); // Hàm dựng đủ tham số

    std::string getCompanyName() const; // Hàm trả về tên công ty
    void setCompanyName(const std::string& name); // Hàm thiết lập tên công ty

    std::string getTaxCode() const; // Hàm trả về mã số thuế
    void setTaxCode(const std::string& code); // Hàm thiết lập mã số thuế

    // Format: id|companyName|taxCode|contactPerson|phoneNumber|email|address
    std::string toFileString() const;

    // Đọc và phân tích một dòng từ Suppliers.txt
    static Supplier fromFileLine(const std::string& line);
};

// ================= IMPLEMENTATION =================

// Hàm dựng mặc định
inline Supplier::Supplier() : People(), companyName(""), taxCode("") {}

// Hàm dựng đủ tham số
inline Supplier::Supplier(std::string id, std::string companyName, std::string taxCode, 
                          Name contactPerson, std::string phone, std::string email, std::string address)
    : People(id, contactPerson, phone, email, address), 
      companyName(companyName), taxCode(taxCode) {}

// Hàm getter / setter cho tên công ty
inline std::string Supplier::getCompanyName() const { return companyName; }
inline void Supplier::setCompanyName(const std::string& name) { companyName = name; }

// Hàm getter / setter cho mã số thuế
inline std::string Supplier::getTaxCode() const { return taxCode; }
inline void Supplier::setTaxCode(const std::string& code) { taxCode = code; }

// Format: id|companyName|taxCode|contactPerson|phoneNumber|email|address
// Note: contactPerson được lưu dưới dạng chuỗi đầy đủ (Fullname), không tách 3 trường
inline std::string Supplier::toFileString() const {
    return TextCodec::escapeField(id) + "|" +
           TextCodec::escapeField(companyName) + "|" +
           TextCodec::escapeField(taxCode) + "|" +
           TextCodec::escapeField(name.getFullName()) + "|" +
           TextCodec::escapeField(phoneNumber) + "|" +
           TextCodec::escapeField(email) + "|" +
           TextCodec::escapeField(address);
}

// Phân tích dòng dữ liệu từ Suppliers.txt thành đối tượng Supplier
inline Supplier Supplier::fromFileLine(const std::string& line) {
    Vector<std::string> parts = TextCodec::splitFields(line, '|');
    if (parts.getSize() < 7) return Supplier(); // Dòng không hợp lệ

    std::string id          = StringHelper::trim(TextCodec::unescape(parts[0]));
    std::string companyName = TextCodec::unescape(parts[1]);
    std::string taxCode     = StringHelper::trim(TextCodec::unescape(parts[2]));
    // contactPerson lưu dưới dạng fullname: "Nguyen Van Dai"
    // → đưa toàn bộ vào lastName, giữ rỗng first/middle (to maintain compatibility)
    std::string fullContact = TextCodec::unescape(parts[3]);
    Name contactName("", "", fullContact); // lưu như lastName để getFullName() trả đúng
    std::string phone       = StringHelper::trim(TextCodec::unescape(parts[4]));
    std::string email       = StringHelper::trim(TextCodec::unescape(parts[5]));
    std::string address     = TextCodec::unescape(parts[6]);

    return Supplier(id, companyName, taxCode, contactName, phone, email, address);
}

#endif // SUPPLIER_H
