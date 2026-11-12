// ĐỐI TƯỢNG NHÂN VIÊN
#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "People.h"
#include "Account.h"
#include "../utils/StringHelper.h"

class Employee : public People {
protected:
    Account account; // Tài khoản đăng nhập của nhân viên
    double salary;   // Mức lương cơ bản

public:
    Employee(); // Hàm dựng mặc định
    Employee(std::string id, Name name, std::string phone, std::string email, 
             std::string address, std::string username, std::string password, 
             RoleType roleType, double salary = 0.0); // Hàm dựng đủ tham số
    virtual ~Employee() = default; // Hàm hủy ảo đảm bảo hủy đúng đối tượng lớp dẫn xuất

    Account& getAccount(); // Hàm trả về tài khoản nhân viên - cho phép sửa
    const Account& getAccount() const; // Hàm trả về tài khoản nhân viên - chỉ đọc
    void setAccount(const Account& acc); // Hàm gán tài khoản nhân viên

    double getSalary() const; // Hàm trả về mức lương
    void setSalary(double s); // Hàm gán mức lương

    // Phương thức thuần ảo biểu diễn tính Đa hình (Polymorphism)
    virtual std::string getRole() const = 0; // Phương thức thuần ảo trả về chuỗi tên chức vụ
    virtual RoleType getRoleType() const = 0; // Phương thức thuần ảo trả về loại chức vụ RoleType

    virtual std::string toFileString() const; // Hàm chuyển thông tin nhân viên thành định dạng lưu file

    // Đọc và phân tích một dòng từ Employee.txt, tạo đối tượng Employee*
    // Caller phải tự quản lý vòng đời của con trỏ (delete sau khi dùng)
    static Employee* fromFileLine(const std::string& line);
};

// Role ADMIN - Quản trị viên cửa hàng (kế thừa từ Employee)
class AdminEmployee : public Employee { 
public:
    AdminEmployee(); // Hàm dựng mặc định cho Quản trị viên
    AdminEmployee(std::string id, Name name, std::string phone, std::string email, 
                  std::string address, std::string username, std::string password, double salary = 0.0); // Hàm dựng đủ tham số

    std::string getRole() const override; // Ghi đè phương thức trả về tên chức danh Admin
    RoleType getRoleType() const override; // Ghi đè phương thức trả về mã RoleType::ADMIN
};

// Role SALESEMPLOYEE - Nhân viên bán hàng (kế thừa từ Employee)
class SalesEmployee : public Employee {
public:
    SalesEmployee(); // Hàm dựng mặc định cho Nhân viên bán hàng
    SalesEmployee(std::string id, Name name, std::string phone, std::string email, 
                  std::string address, std::string username, std::string password, double salary = 0.0); // Hàm dựng đủ tham số

    std::string getRole() const override; // Ghi đè phương thức trả về tên chức danh Nhân viên bán hàng
    RoleType getRoleType() const override; // Ghi đè phương thức trả về mã RoleType::SALE_EMPLOYEE
};

// Role WAREHOUSEEMPLOYEE - Nhân viên kho (kế thừa từ Employee)
class WarehouseEmployee : public Employee {
public:
    WarehouseEmployee(); // Hàm dựng mặc định cho Nhân viên kho
    WarehouseEmployee(std::string id, Name name, std::string phone, std::string email, 
                      std::string address, std::string username, std::string password, double salary = 0.0); // Hàm dựng đủ tham số

    std::string getRole() const override; // Ghi đè phương thức trả về tên chức danh Nhân viên kho
    RoleType getRoleType() const override; // Ghi đè phương thức trả về mã RoleType::WAREHOUSE_EMPLOYEE
};

// ================= IMPLEMENTATION =================

// --- Employee ---

// Hàm dựng mặc định
inline Employee::Employee() 
    : People(), account("", "", RoleType::SALE_EMPLOYEE), salary(0.0) {}

// Hàm dựng đủ tham số
inline Employee::Employee(std::string id, Name name, std::string phone, std::string email, 
                          std::string address, std::string username, std::string password, 
                          RoleType roleType, double salary)
    : People(id, name, phone, email, address), 
      account(username, password, roleType), salary(salary) {}

// Hàm trả về tài khoản - cho phép thay đổi
inline Account& Employee::getAccount() { return account; }

// Hàm trả về tài khoản - không cho phép thay đổi
inline const Account& Employee::getAccount() const { return account; }

// Hàm setter tài khoản
inline void Employee::setAccount(const Account& acc) { account = acc; }

// Hàm getter / setter cho mức lương
inline double Employee::getSalary() const { return salary; }
inline void Employee::setSalary(double s) { salary = s; }

// Hàm chuyển thông tin nhân viên thành định dạng chuỗi lưu file Employee.txt
// Format: id|firstName|middleName|lastName|phoneNumber|email|username|passwordHash|role|address|salary|accountId|active
inline std::string Employee::toFileString() const {
    // FIX LỖI: Chuyển đổi RoleType sang mã file chuẩn (1=Admin, 2=Sale, 3=Warehouse) theo DATA_FORMAT_GUIDE.md
    int fileRoleCode = 0;
    if (getRoleType() == RoleType::ADMIN) {
        fileRoleCode = 1;
    } else if (getRoleType() == RoleType::SALE_EMPLOYEE) {
        fileRoleCode = 2;
    } else if (getRoleType() == RoleType::WAREHOUSE_EMPLOYEE) {
        fileRoleCode = 3;
    }

    return id + "|" + name.firstName + "|" + name.middleName + "|" + name.lastName + "|" +
           phoneNumber + "|" + email + "|" + account.getUsername() + "|" + account.getPasswordHash() + "|" +
           std::to_string(fileRoleCode) + "|" + address + "|" +
           std::to_string(static_cast<long long>(salary)) + "|" + account.getID() + "|" +
           (account.isActive() ? "1" : "0");
}

// Phân tích dòng dữ liệu từ Employee.txt, trả về con trỏ Employee* đúng kiểu
// Note: Caller có trách nhiệm gọi delete sau khi sử dụng xong
inline Employee* Employee::fromFileLine(const std::string& line) {
    Vector<std::string> parts = StringHelper::split(line, '|');
    if (parts.getSize() < 10) return nullptr; // Dòng không hợp lệ

    std::string id       = StringHelper::trim(parts[0]);
    Name name(StringHelper::trim(parts[1]), StringHelper::trim(parts[2]), StringHelper::trim(parts[3]));
    std::string phone    = StringHelper::trim(parts[4]);
    std::string email    = StringHelper::trim(parts[5]);
    std::string username = StringHelper::trim(parts[6]);
    std::string password = StringHelper::trim(parts[7]); // Giá trị đã lưu: hash PBKDF2 hoặc plaintext cũ
    
    // FIX LỖI: Bắt ngoại lệ chuyển đổi kiểu số an toàn, tránh crash ứng dụng khi gặp dữ liệu lỗi
    int roleInt = 0;
    double salary = 0.0;
    try {
        roleInt = std::stoi(StringHelper::trim(parts[8]));
        if (parts.getSize() >= 11) {
            salary = std::stod(StringHelper::trim(parts[10]));
        }
    } catch (...) {
        return nullptr; // Dòng dữ liệu số không hợp lệ
    }
    std::string address  = StringHelper::trim(parts[9]);

    // Mapper từ số role sang RoleType (1=Admin, 2=Sale, 3=Warehouse)
    Employee* employee = nullptr;
    if (roleInt == 1) {
        employee = new AdminEmployee(id, name, phone, email, address, username, password, salary);
    } else if (roleInt == 2) {
        employee = new SalesEmployee(id, name, phone, email, address, username, password, salary);
    } else if (roleInt == 3) {
        employee = new WarehouseEmployee(id, name, phone, email, address, username, password, salary);
    }
    if (employee != nullptr && parts.getSize() >= 12) {
        employee->getAccount().setID(StringHelper::trim(parts[11]));
    }
    if (employee != nullptr && parts.getSize() >= 13) {
        // Chỉ "0" mới là tài khoản bị vô hiệu hóa; thiếu/giá trị khác = đang hoạt động
        employee->getAccount().setActive(StringHelper::trim(parts[12]) != "0");
    }
    return employee; // Role không xác định
}

// --- AdminEmployee ---

// Hàm dựng mặc định Admin
inline AdminEmployee::AdminEmployee() : Employee() {
    account.setRole(Role(RoleType::ADMIN));
}

// Hàm dựng đủ tham số Admin
inline AdminEmployee::AdminEmployee(std::string id, Name name, std::string phone, std::string email, 
                                    std::string address, std::string username, std::string password, double salary)
    : Employee(id, name, phone, email, address, username, password, RoleType::ADMIN, salary) {}

// Trả về tên hiển thị chức vụ Admin
inline std::string AdminEmployee::getRole() const {
    return "Quản trị viên (Admin)";
}

// Trả về mã chức vụ Admin
inline RoleType AdminEmployee::getRoleType() const {
    return RoleType::ADMIN;
}

// --- SalesEmployee ---

// Hàm dựng mặc định Nhân viên bán hàng
inline SalesEmployee::SalesEmployee() : Employee() {
    account.setRole(Role(RoleType::SALE_EMPLOYEE));
}

// Hàm dựng đủ tham số Nhân viên bán hàng
inline SalesEmployee::SalesEmployee(std::string id, Name name, std::string phone, std::string email, 
                                    std::string address, std::string username, std::string password, double salary)
    : Employee(id, name, phone, email, address, username, password, RoleType::SALE_EMPLOYEE, salary) {}

// Trả về tên hiển thị chức vụ Nhân viên bán hàng
inline std::string SalesEmployee::getRole() const {
    return "Nhân viên Bán hàng";
}

// Trả về mã chức vụ Nhân viên bán hàng
inline RoleType SalesEmployee::getRoleType() const {
    return RoleType::SALE_EMPLOYEE;
}

// --- WarehouseEmployee ---

// Hàm dựng mặc định Nhân viên kho
inline WarehouseEmployee::WarehouseEmployee() : Employee() {
    account.setRole(Role(RoleType::WAREHOUSE_EMPLOYEE));
}

// Hàm dựng đủ tham số Nhân viên kho
inline WarehouseEmployee::WarehouseEmployee(std::string id, Name name, std::string phone, std::string email, 
                                            std::string address, std::string username, std::string password, double salary)
    : Employee(id, name, phone, email, address, username, password, RoleType::WAREHOUSE_EMPLOYEE, salary) {}

// Trả về tên hiển thị chức vụ Nhân viên kho
inline std::string WarehouseEmployee::getRole() const {
    return "Nhân viên Kho";
}

// Trả về mã chức vụ Nhân viên kho
inline RoleType WarehouseEmployee::getRoleType() const {
    return RoleType::WAREHOUSE_EMPLOYEE;
}

#endif // EMPLOYEE_H
