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

    void setTimeCurrent();
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

class Custumer { // định nghĩa khách hàng
private:
    string id;
    Name name;
    string phoneNumber;
    string email;
    string address;

    int points;
    Account* account;

public:
    Custumer();
    
    // getter
    string getID() const;
    Name getName() const;
    string getPhoneNumber() const;
    string getAddress() const;
    int getPoints() const;

    // setter
    void setID(string& id);
    void setName(Name& name);
    void setPhoneNumber(string& PhoneNumber);
    void setAddress(string& Address);

    // functions
    void addPoints(int points);
};

class Employee { //  định nghĩa nhân viên - dùng cho toàn bộ người trong cửa hàng
private:
    string id;
    Name name;
    string phoneNumber;
    string email;

    Account* account;
public:
    Employee();

    virtual ~Employee() = default; // Hủy ảo -> tránh leak Memory do đa hình

    // getter
    string getID() const;
    Name getName() const;
    string getPhoneNumber() const;
    string getEmail() const;

    // setter
    void setID(string& id);
    void setName(Name &name);
    void setPhoneNumber(string& phoneNumber);
    void setEmail(string& email);

    virtual string getRole() const = 0;
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

class Order {
private:
    string id;
    Custumer* custumer;
    DeliveryMethod method;
    Detail detail;

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
    Detail detail;

public:
    Invoice();

    // functions
    void addDetail();
    void delDetail();

    void calculateSubtotal() const;
    void calculateDiscount() const;
    void calculateTotal() const;
};

class Discount {
private:
    string id;

};