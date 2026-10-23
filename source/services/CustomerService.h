#ifndef CUSTOMERSERVICE_H
#define CUSTOMERSERVICE_H

#include <iostream>
#include <string>
#include <vector>
#include "../models/Customer.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../repositories/CustomerRepository.h"
#include "../utils/StringHelper.h"
#include "Check.h"

// Tầng Service: Toàn bộ nghiệp vụ quản lý khách hàng và xác thực tài khoản
// Ứng dụng cấu trúc dữ liệu:
// - HashTable<std::string, Customer> usernameTable: Tra cứu O(1) theo tên đăng nhập
// - HashTable<std::string, Customer> phoneTable: Tra cứu O(1) theo số điện thoại
class CustomerService {
private:
    CustomerRepository& repo;
    HashTable<std::string, Customer> usernameTable; // Tra cứu O(1) theo username
    HashTable<std::string, Customer> phoneTable;    // Tra cứu O(1) theo số điện thoại

public:
    CustomerService(CustomerRepository& repository);

    void rebuildIndices();
    const Vector<Customer>& getAllCustomers() const;

    // Xác thực đăng nhập khách hàng
    Customer* authenticate(const std::string& username, const std::string& password);

    Customer* getCustomerByUsername(const std::string& username);
    Customer* getCustomerByPhone(const std::string& phone);
    Customer* getCustomerById(const std::string& id);

    // Nghiệp vụ đăng ký khách hàng mới
    bool registerCustomer(const Customer& cust, std::string& errorMessage);
    bool registerCustomer(const Customer& cust);

    // Nghiệp vụ cập nhật thông tin khách hàng
    bool updateCustomer(const Customer& updatedCust, std::string& errorMessage);
    bool updateCustomer(const Customer& updatedCust);

    // Nghiệp vụ xóa khách hàng
    bool deleteCustomer(const std::string& id, std::string& errorMessage);
    bool deleteCustomer(const std::string& id);

    // ==== NGHIỆP VỤ TÍCH ĐIỂM KHÁCH HÀNG ====

    // Cộng điểm tích lũy (gọi sau khi thanh toán thành công)
    // Rule: points >= 0; customer phải tồn tại; luu file ngay
    bool addPoints(const std::string& customerId, int points, std::string& errorMessage);
    bool addPoints(const std::string& customerId, int points);

    // Trừ điểm khi đổi quà/ưu đãi (call trước khi commit giao dịch)
    // Rule: points phải đủ; không cho trừ xuống âm
    bool deductPoints(const std::string& customerId, int points, std::string& errorMessage);
    bool deductPoints(const std::string& customerId, int points);

    // Validate số điện thoại (10-11 chữ số, bắt đầu bằng 0)
    static bool validatePhone(const std::string& phone, std::string& errorMessage);
};

// ================= IMPLEMENTATION =================

inline CustomerService::CustomerService(CustomerRepository& repository)
    : repo(repository) {
    rebuildIndices();
}

inline void CustomerService::rebuildIndices() {
    usernameTable.clear();
    phoneTable.clear();
    const Vector<Customer>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        std::string uname = StringHelper::trim(list[i].getAccount().getUsername());
        std::string phone = StringHelper::trim(list[i].getPhoneNumber());
        if (!uname.empty() && usernameTable.find(uname) == nullptr) {
            usernameTable.insert(uname, list[i]);
        }
        if (!phone.empty() && phoneTable.find(phone) == nullptr) {
            phoneTable.insert(phone, list[i]);
        }
    }
}

inline const Vector<Customer>& CustomerService::getAllCustomers() const {
    return repo.getAll();
}

inline Customer* CustomerService::authenticate(const std::string& username, const std::string& password) {
    std::string trimmedUname = StringHelper::trim(username);
    if (trimmedUname.empty() || password.empty()) {
        return nullptr;
    }

    Customer* custInIndex = usernameTable.find(trimmedUname);
    if (custInIndex != nullptr && custInIndex->getAccount().authenticate(trimmedUname, password)) {
        return repo.findById(custInIndex->getID());
    }
    return nullptr;
}

inline Customer* CustomerService::getCustomerByUsername(const std::string& username) {
    Customer* cust = usernameTable.find(StringHelper::trim(username));
    if (cust != nullptr) {
        return repo.findById(cust->getID());
    }
    return nullptr;
}

inline Customer* CustomerService::getCustomerByPhone(const std::string& phone) {
    std::string trimmedPhone = StringHelper::trim(phone);
    if (trimmedPhone.empty()) return nullptr;
    Customer* cust = phoneTable.find(trimmedPhone);
    if (cust != nullptr) {
        return repo.findById(cust->getID());
    }
    return nullptr;
}

inline Customer* CustomerService::getCustomerById(const std::string& id) {
    return repo.findById(id);
}

inline bool CustomerService::registerCustomer(const Customer& cust, std::string& errorMessage) {
    Customer registered = cust;
    if (registered.getID().empty()) {
        std::vector<std::string> existingIds;
        const Vector<Customer>& customers = repo.getAll();
        for (int i = 0; i < customers.getSize(); ++i) {
            existingIds.push_back(customers[i].getID());
        }
        std::string generatedId;
        if (!IdGenerator::nextId("CUS", "", existingIds, generatedId, errorMessage)) {
            return false;
        }
        registered.setID(generatedId);
    } else if (!isValidID(registered.getID(), "CUS", errorMessage)) {
        return false;
    }

    if (registered.getAccount().getID().empty()) {
        std::vector<std::string> accountIds;
        const Vector<Customer>& customers = repo.getAll();
        for (int i = 0; i < customers.getSize(); ++i) {
            accountIds.push_back(customers[i].getAccount().getID());
        }
        std::string generatedAccountId;
        if (!IdGenerator::nextId("ACC", "", accountIds,
                                 generatedAccountId, errorMessage)) {
            return false;
        }
        registered.getAccount().setID(generatedAccountId);
    } else if (!IdGenerator::isValidId(registered.getAccount().getID(), "ACC")) {
        errorMessage = "Mã tài khoản phải đúng định dạng ACC-000001.";
        return false;
    }

    if (repo.findById(registered.getID()) != nullptr) {
        errorMessage = "Mã khách hàng [" + registered.getID() + "] đã tồn tại!";
        return false;
    }

    std::string uname = StringHelper::trim(registered.getAccount().getUsername());
    if (uname.empty()) {
        errorMessage = "Tên đăng nhập không được để trống!";
        return false;
    }

    if (usernameTable.contains(uname)) {
        errorMessage = "Tên đăng nhập [" + uname + "] đã được sử dụng!";
        return false;
    }

    std::string phone = StringHelper::trim(registered.getPhoneNumber());
    if (!phone.empty() && phoneTable.contains(phone)) {
        errorMessage = "Số điện thoại [" + phone + "] đã được đăng ký bởi khách hàng khác!";
        return false;
    }

    if (!repo.addCustomer(registered)) {
        errorMessage = "Lỗi khi lưu thông tin khách hàng vào cơ sở dữ liệu!";
        return false;
    }

    rebuildIndices();
    errorMessage = "Đăng ký khách hàng thành công!";
    return true;
}

inline bool CustomerService::registerCustomer(const Customer& cust) {
    std::string err;
    return registerCustomer(cust, err);
}

inline bool CustomerService::updateCustomer(const Customer& updatedCust, std::string& errorMessage) {
    Customer* existing = repo.findById(updatedCust.getID());
    if (existing == nullptr) {
        errorMessage = "Không tìm thấy khách hàng có mã [" + updatedCust.getID() + "] để cập nhật!";
        return false;
    }

    std::string newUname = StringHelper::trim(updatedCust.getAccount().getUsername());
    if (!newUname.empty() && newUname != existing->getAccount().getUsername()) {
        if (usernameTable.contains(newUname)) {
            errorMessage = "Tên đăng nhập [" + newUname + "] đã được sử dụng bởi người khác!";
            return false;
        }
    }

    std::string newPhone = StringHelper::trim(updatedCust.getPhoneNumber());
    if (!newPhone.empty() && newPhone != existing->getPhoneNumber()) {
        if (phoneTable.contains(newPhone)) {
            errorMessage = "Số điện thoại [" + newPhone + "] đã thuộc về tài khoản khác!";
            return false;
        }
    }

    if (!repo.updateCustomer(updatedCust)) {
        errorMessage = "Lỗi khi cập nhật thông tin khách hàng!";
        return false;
    }

    rebuildIndices();
    errorMessage = "Cập nhật thông tin khách hàng thành công!";
    return true;
}

inline bool CustomerService::updateCustomer(const Customer& updatedCust) {
    std::string err;
    return updateCustomer(updatedCust, err);
}

inline bool CustomerService::deleteCustomer(const std::string& id, std::string& errorMessage) {
    if (repo.findById(id) == nullptr) {
        errorMessage = "Không tìm thấy khách hàng có mã [" + id + "] để xóa!";
        return false;
    }

    if (!repo.deleteCustomer(id)) {
        errorMessage = "Lỗi khi xóa khách hàng khỏi cơ sở dữ liệu!";
        return false;
    }

    rebuildIndices();
    errorMessage = "Xóa khách hàng thành công!";
    return true;
}

inline bool CustomerService::deleteCustomer(const std::string& id) {
    std::string err;
    return deleteCustomer(id, err);
}

// ===== TÍCH ĐIỂM: NGHIỆP VỤ TÍCH LŨY VÀ TRỪ ĐIỂM =====

// Cộng điểm tích lũy sau khi khách hàng thanh toán thành công
// Rule: điểm cộng phải > 0; khách hàng phải tồn tại; lưu file ngay
inline bool CustomerService::addPoints(const std::string& customerId, int points, std::string& errorMessage) {
    if (points <= 0) {
        errorMessage = "So diem cong phai lon hon 0!";
        return false;
    }
    Customer* cust = repo.findById(customerId);
    if (cust == nullptr) {
        errorMessage = "Khong tim thay khach hang [" + customerId + "]!";
        return false;
    }
    cust->addPoints(points);
    if (!repo.updateCustomer(*cust)) {
        errorMessage = "Loi khi cap nhat diem tich luy!";
        cust->deductPoints(points); // Rollback in-memory
        return false;
    }
    rebuildIndices();
    errorMessage = "Cong " + std::to_string(points) + " diem thanh cong!";
    return true;
}

inline bool CustomerService::addPoints(const std::string& customerId, int points) {
    std::string err;
    return addPoints(customerId, points, err);
}

// Trừ điểm khi khách hàng đổi quà / áp dụng ưu đãi
// Rule: khách phải đủ điểm; không được trừ xuống âm; lưu file ngay
inline bool CustomerService::deductPoints(const std::string& customerId, int points, std::string& errorMessage) {
    if (points <= 0) {
        errorMessage = "So diem tru phai lon hon 0!";
        return false;
    }
    Customer* cust = repo.findById(customerId);
    if (cust == nullptr) {
        errorMessage = "Khong tim thay khach hang [" + customerId + "]!";
        return false;
    }
    if (cust->getPoints() < points) {
        errorMessage = "Khach hang khong du diem (hien co: " +
                       std::to_string(cust->getPoints()) +
                       ", can: " + std::to_string(points) + ")!";
        return false;
    }
    cust->deductPoints(points);
    if (!repo.updateCustomer(*cust)) {
        errorMessage = "Loi khi cap nhat diem tich luy!";
        cust->addPoints(points); // Rollback in-memory
        return false;
    }
    rebuildIndices();
    errorMessage = "Tru " + std::to_string(points) + " diem thanh cong!";
    return true;
}

inline bool CustomerService::deductPoints(const std::string& customerId, int points) {
    std::string err;
    return deductPoints(customerId, points, err);
}

// Validate so dien thoai: 10-11 chu so, bat dau bang 0
inline bool CustomerService::validatePhone(const std::string& phone, std::string& errorMessage) {
    std::string p = phone;
    while (!p.empty() && (p.front() == ' ' || p.front() == '\t')) p.erase(p.begin());
    while (!p.empty() && (p.back() == ' ' || p.back() == '\t')) p.pop_back();

    if (p.empty()) {
        errorMessage = "So dien thoai khong duoc de trong!";
        return false;
    }
    if (p.length() < 10 || p.length() > 11) {
        errorMessage = "So dien thoai phai co 10-11 chu so!";
        return false;
    }
    if (p[0] != '0') {
        errorMessage = "So dien thoai phai bat dau bang 0!";
        return false;
    }
    for (char c : p) {
        if (c < '0' || c > '9') {
            errorMessage = "So dien thoai chi duoc chua chu so!";
            return false;
        }
    }
    return true;
}

#endif // CUSTOMERSERVICE_H
