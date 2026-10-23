// EMPLOYEE REPOSITORY - Tầng truy xuất dữ liệu nhân viên
#ifndef EMPLOYEEREPOSITORY_H
#define EMPLOYEEREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Employee.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu nhân viên từ/vào tệp tin.
// Do Employee là class trừu tượng có đa hình, lưu trữ bằng Vector<Employee*> (con trỏ).
// QUAN TRỌNG: Repository này sở hữu (owns) các con trỏ → destructor phải giải phóng bộ nhớ.
class EmployeeRepository {
private:
    std::string filePath;       // Đường dẫn đến file Employee.txt
    Vector<Employee*> employees; // Danh sách nhân viên trên RAM (dùng con trỏ cho đa hình)

    // Giải phóng toàn bộ bộ nhớ động và xóa danh sách
    void clearMemory();

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng: nhận đường dẫn file
    EmployeeRepository(std::string path = "data/people/Employee.txt");

    // Destructor: giải phóng toàn bộ con trỏ Employee* trong danh sách
    ~EmployeeRepository();

    // Ngăn copy nông (shallow copy) tránh double-free
    EmployeeRepository(const EmployeeRepository&) = delete;
    EmployeeRepository& operator=(const EmployeeRepository&) = delete;

    // Nạp dữ liệu từ file vào RAM. Tự tạo file nếu chưa có.
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file (ghi đè).
    bool saveToFile() const;

    // Trả về toàn bộ danh sách (chỉ đọc / cho phép sửa)
    const Vector<Employee*>& getAll() const;
    Vector<Employee*>& getAll();

    // Tìm theo ID nhân viên
    Employee* findById(const std::string& id);
    const Employee* findById(const std::string& id) const;

    // Tìm theo username tài khoản (dùng cho đăng nhập)
    Employee* findByUsername(const std::string& username);
    const Employee* findByUsername(const std::string& username) const;

    // Lọc nhân viên theo chức vụ
    Vector<Employee*> findByRole(RoleType role) const;

    // CRUD: Thêm, cập nhật, xóa
    // addEmployee nhận ownership của con trỏ
    bool addEmployee(Employee* emp);
    // updateEmployee: xóa con trỏ cũ, thay bằng con trỏ mới (nhận ownership)
    bool updateEmployee(Employee* updatedEmp);
    bool deleteEmployee(const std::string& id);

    // Kiểm tra ID có tồn tại không
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

// Giải phóng toàn bộ bộ nhớ động
inline void EmployeeRepository::clearMemory() {
    for (int i = 0; i < employees.getSize(); ++i) {
        delete employees[i];
        employees[i] = nullptr;
    }
    employees.clear();
}

// Tạo file Employee.txt mới với header chuẩn (11 cột bao gồm salary)
inline bool EmployeeRepository::createEmptyFile() const {
    std::ofstream newFile(filePath);
    if (!newFile.is_open()) {
        std::cerr << "[Loi] Khong the tao file moi: " << filePath << std::endl;
        return false;
    }
    newFile << "id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address|salary\n";
    newFile.close();
    std::cout << "[Info] Da tao file moi: " << filePath << std::endl;
    return true;
}

inline EmployeeRepository::EmployeeRepository(std::string path) : filePath(path) {}

inline EmployeeRepository::~EmployeeRepository() {
    clearMemory();
}

// Nạp dữ liệu từ Employee.txt vào RAM
// - Gọi Employee::fromFileLine() để phân tích và tạo đúng subclass (Admin/Sales/Warehouse)
// - Nếu file không tồn tại: tự tạo file mới
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool EmployeeRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    clearMemory();
    std::string line;
    bool isHeader = true;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        line = StringHelper::trim(line);
        if (line.empty()) continue;

        if (isHeader) {
            isHeader = false; // Bỏ qua dòng tiêu đề
            continue;
        }

        try {
            // Employee::fromFileLine() trả về Employee* đúng subclass (Admin/Sales/Warehouse)
            Employee* emp = Employee::fromFileLine(line);
            if (emp != nullptr) {
                employees.push_back(emp);
            } else {
                std::cerr << "[Canh bao] Role khong hop le tai dong " << lineNum
                          << " trong " << filePath << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[Canh bao] Loi doc dong " << lineNum
                      << " trong " << filePath << ": " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[Canh bao] Loi khong xac dinh tai dong " << lineNum
                      << " trong " << filePath << std::endl;
        }
    }
    file.close();
    return true;
}

// Ghi toàn bộ danh sách employees xuống file (ghi đè)
inline bool EmployeeRepository::saveToFile() const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Loi] Khong the ghi file: " << filePath << std::endl;
        return false;
    }
    // Header khớp với định dạng Employee::toFileString() (11 cột, có salary)
    file << "id|firstName|middleName|lastName|phoneNumber|Email|username|password|role|address|salary\n";
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] != nullptr) {
            file << employees[i]->toFileString() << "\n";
        }
    }
    file.close();
    return true;
}

inline const Vector<Employee*>& EmployeeRepository::getAll() const { return employees; }
inline Vector<Employee*>& EmployeeRepository::getAll() { return employees; }

// Tìm Employee theo ID
inline Employee* EmployeeRepository::findById(const std::string& id) {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] && employees[i]->getID() == id) return employees[i];
    }
    return nullptr;
}

inline const Employee* EmployeeRepository::findById(const std::string& id) const {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] && employees[i]->getID() == id) return employees[i];
    }
    return nullptr;
}

// Tìm Employee theo username (dùng để xác thực đăng nhập)
inline Employee* EmployeeRepository::findByUsername(const std::string& username) {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] && employees[i]->getAccount().getUsername() == username)
            return employees[i];
    }
    return nullptr;
}

inline const Employee* EmployeeRepository::findByUsername(const std::string& username) const {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] && employees[i]->getAccount().getUsername() == username)
            return employees[i];
    }
    return nullptr;
}

// Lọc nhân viên theo chức vụ (Admin / Sale / Warehouse)
inline Vector<Employee*> EmployeeRepository::findByRole(RoleType role) const {
    Vector<Employee*> result;
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] && employees[i]->getRoleType() == role) {
            result.push_back(employees[i]);
        }
    }
    return result;
}

// Thêm nhân viên mới (nhận ownership của con trỏ)
inline bool EmployeeRepository::addEmployee(Employee* emp) {
    if (emp == nullptr) return false;
    if (existsById(emp->getID())) {
        std::cerr << "[Canh bao] ID nhan vien da ton tai: " << emp->getID() << std::endl;
        return false;
    }
    employees.push_back(emp);
    return saveToFile();
}

// Cập nhật nhân viên: xóa con trỏ cũ, thay bằng con trỏ mới
inline bool EmployeeRepository::updateEmployee(Employee* updatedEmp) {
    if (updatedEmp == nullptr) return false;
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] && employees[i]->getID() == updatedEmp->getID()) {
            delete employees[i]; // Giải phóng con trỏ cũ
            employees[i] = updatedEmp;
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de cap nhat: " << updatedEmp->getID() << std::endl;
    return false;
}

// Xóa nhân viên theo ID (giải phóng bộ nhớ động)
inline bool EmployeeRepository::deleteEmployee(const std::string& id) {
    for (int i = 0; i < employees.getSize(); ++i) {
        if (employees[i] && employees[i]->getID() == id) {
            delete employees[i];
            employees.removeAt(i);
            return saveToFile();
        }
    }
    std::cerr << "[Canh bao] Khong tim thay ID de xoa: " << id << std::endl;
    return false;
}

inline bool EmployeeRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // EMPLOYEEREPOSITORY_H
