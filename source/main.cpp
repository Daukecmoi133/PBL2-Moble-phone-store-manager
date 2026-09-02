#include <bits/stdc++.h>
#include <chrono>
#include <ctime>

using namespace std;
using namespace chrono;

// KHAI BÁO TRƯỚC
class Phone;
enum class Permission;
class Role;
class Account;
class Custumer;
class Employee;
class SalesEmployee;
class WarehouseEmployee;
class CardItem;
class Cart;
enum class OrderStatus;
enum class DeliveryMethod;
class OrderDetail;
class Order;
class Invoice;




typedef struct Name {
    string firstName, middleName, lastName;
};

typedef struct Calender {
    int year, month, day;
    int hour, minute, second;

    string getTimeCurrent();

    void setTime(int year, int month, int day, int hour, int minute, int second);
};

class Phone { // định nghĩa 1 chiếc điện thoại
private: 
    string id; // mã sản phẩm
    Name name; // tên sản phẩm
    string brand; // thương hiệu sản phẩm
    string manufacturer; // nhà sản xuất
    string operatingSystem; // hệ điều hành
    string specifications; // thông số kĩ thuật

    double importPrice; // giá nhập hàng
    double sellingPrice; // giá bán hàng
    int quantity; // số lượng tồn kho

public:
    Phone();

    // Getter
    string getID() const;
    Name getName() const;
    string getBrand() const;
    string getManuFacturer() const;
    string getOperatingSystem() const;
    string getSpecifications() const;
    double getImportPrice() const;
    double getSellingPrice() const;
    int getQuantity();

    // Setter
    void setID(string& id);
    void setName(Name& name);
    void setBrand(string& brand);
    void setManuFacturer(string& manuFacturer);
    void setOperatingSystem(string& operatingSystem);
    void setSpecifications(string& specifications);
    void setImportPrice(double importPrice);
    void setSellingPrice(double sellingPrice);

    // chức năng
};

enum class Permission { // định nghĩa các mục cho phép
    PRODUCT_VIEW, 
    PRODUCT_CREATE,
    PRODUCT_UPDATE,
    PRODUCT_DELETE,

    ORDER_VIEW,
    ORDER_CREATE,

    INVOICE_CREATE,

    STOCK_VIEW,
    STOCK_UPDATE,

    EMPLOYEE_MANAGE
};

class Role { // định nghĩa vai trò
private:
    Name name;
    vector<Permission> permissions;

public:
    // functions
    bool hasPermission(Permission& permission) const;
    void addPermission(Permission& permission);
    void delPermission(Permission& permission);

};

class Account { // định nghĩa tài khoản
private:
    string username;
    string password;
    Role role;

public:
    // functions
    bool authenticate(string& username, string& password) const;
    bool hasPermission(Permission& permission) const;

};

class People { // định nghĩa người dùng
private:
    string id;
    Name name;
    string phoneNumber;
    string email;
    string address;

public:
    // getter
    string getID() const;
    Name getName() const;
    string getPhoneNumber() const;
    string getEmail() const;
    string getAddress() const;

    // setter
    void setID(string& id);
    void setName(Name& name);
    void setPhoneNumber(string& phoneNumber);
    void setEmail(string& email);
    void setAddress(string& address);
};

class Custumer : public People { // định nghĩa khách hàng
private:
    int points; // điểm tích lũy
    Account* account;

public:
    Custumer();

    // functions
    void addPoints(int points);
};

class Employee : public People { //  định nghĩa nhân viên - dùng cho toàn bộ người trong cửa hàng
private:
    Account* account;
public:
    Employee();

    virtual ~Employee() = default; // Hủy ảo -> tránh leak Memory do đa hình

    // getter
    virtual string getRole() const = 0;
};
class AdminEmployee : public Employee { // định nghĩa lớp con nhân viên quản lý từ lớp cha nhân viên
public: 
    string getRole() const override;
};

class SalesEmployee : public Employee { // định nghĩa lớp con nhân viên sale từ lớp cha nhân viên
public: 
    string getRole() const override;
};

class WarehouseEmployee : public Employee { // định nghĩa lớp con nhân viên kho từ lớp cha nhân viên
public:
    string getRole() const override;
};

class CardItem { // định nghĩa các thành phần của giỏ hàng
private:
    Phone* phone;
    int quantiy;
    double unitPrice;

public:
    CardItem(Phone* phone, int quantity);

    // getter
    Phone* getPhone() const;
    int getQuantity() const;

    // setter
    void setQuantity(int quantity);
    
    // functions
    void increaseQuantity(int amount);
};

class Cart { // định nghĩa giỏ hàng
private:
    string id;
    Custumer* custumer;
    vector<CardItem> item;

public:
    // functions
    void addItem(Phone* phone, int quantity);
    bool delItem(string& id);
    void updateItem(string& id, int quanlity);

    double calculationSubtotal() const;
    
    void clear();
    bool isEmpry() const;
};

enum class OrderStatus { // trạng thái đơn hàng
    PENDING,
    CONFIRMED,
    PREPARING,
    REALLY,
    COMPLETED,
    CANCELLED
};

enum class DeliveryMethod { // phương thức nhận hàng
    PICKUP, 
    DELEVERY
};

class Detail {
private:
    Phone* phones;
    int quantity;
    double unitPrice;

public:
    // functions
    double calculateSubtotal() const;
};

class ShippingInfor { // thông tin giao hàng
private:
    string address;
    string phoneNumber;
    string receiverName;
public:
    // getter
    string getAddress() const;
    string getPhoneNumber() const;
    string getReceiverName() const;

    // setter
    void setAddress(string& address);
    void setPhoneNumber(string& phoneNumber);
    void setReceiverName(string& receiverName);
};

class Order {
private:
    string id;
    Custumer* custumer;
    DeliveryMethod method;
    vector<Detail> detail;

    ShippingInfor shippingInfor;
    OrderStatus status;
    Calender time;

public:
    Order();

    // functions

    void addDetail();
    void delDetail();

    void calculateSubtotal() const;
    void calculateDiscount() const;
    void calculateTotal() const;

    void confirm();
    void cancel();

    OrderStatus getStatus() const;
};

class Invoice {
private:
    string id;
    Custumer* custumer;
    Employee* employee;
    vector<Detail> detail;

public:
    Invoice();

    // functions
    void addDetail();
    void delDetail();

    void calculateSubtotal() const;
    void calculateDiscount() const;
    void calculateTotal() const;
};

class Supplier : public People { // định nghĩa nhà cung cấp
public:
    Supplier();
};

class ImportReceipt {
private: 
    string id;
    Supplier* supplier;
    Employee* employee;
    vector<Detail> detail;

public:
    ImportReceipt();

    // functions
    void addDetail();
    void delDetail();

    void calculateSubtotal() const;
    void calculateTotal() const;
};

enum class DiscountType { // định nghĩa loại giảm giá
    PERCENTAGE,
    FIXED_AMOUNT
};

class DiscountCode { // định nghĩa mã giảm giá
private: 
    string code;
    DiscountType type;
    double value;

    Calender startDate;
    Calender endDate;

    double minOrderValue; // giá trị đơn hàng tối thiểu để áp dụng mã giảm giá
    double maxDiscontValue; // giá trị giảm giá tối đa áp dụng cho đơn hàng

    int maxUsage;
    int usedCount;

public:
    DiscountCode();

    bool isValid(Calender& currentTime) const; // kiểm tra xem mã giảm giá có còn hiệu lực hay không
    bool isApplicable(double orderValue) const; // kiểm tra xem mã giảm giá có áp dụng được cho đơn hàng hay không

    double calculateDiscount(double orderValue) const; 

    void increaseUsage(); 

    string getCode() const; 
};

class Warranty { // định nghĩa bảo hành
    // làm sau
};