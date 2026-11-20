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
    ReferenceChecker* refChecker;

public:
    CustomerService(CustomerRepository& repository, ReferenceChecker* checker = nullptr);
    void setReferenceChecker(ReferenceChecker* checker) { refChecker = checker; }

    void rebuildIndices();
    void reload() { repo.loadFromFile(); rebuildIndices(); }
    CustomerRepository& getRepository() { return repo; }
    const CustomerRepository& getRepository() const { return repo; }
    const Vector<Customer>& getAllCustomers() const;

    // Xác thực đăng nhập khách hàng (kiểm tra trên bản ghi gốc trong repository, tài khoản phải đang hoạt động)
    Customer* authenticate(const std::string& username, const std::string& password);

    // ==== QUẢN LÝ THÔNG TIN XÁC THỰC (ghi xuống Customers.txt, hoàn nguyên RAM nếu ghi thất bại) ====

    // Thay thông tin xác thực đã lưu (hash). Chỉ chấp nhận chuỗi đã băm.
    bool setPasswordCredential(const std::string& customerId, const std::string& hashedCredential,
                               std::string& errorMessage);

    // Khóa / mở khóa tài khoản và lưu trạng thái
    bool setAccountActive(const std::string& customerId, bool active, std::string& errorMessage);

    // Chuyển toàn bộ mật khẩu plaintext cũ sang hash (giữ nguyên mật khẩu, ghi file một lần)
    bool migrateLegacyCredentials(int& migratedCount, std::string& errorMessage);

    // Dữ liệu khách hàng không nạp được từ file (đã gọi loadFromFile nhưng thất bại)
    bool hasLoadError() const { return repo.hasLoadError(); }

    Customer* getCustomerByUsername(const std::string& username);
    Customer* getCustomerByPhone(const std::string& phone);
    Customer* getCustomerById(const std::string& id);
    bool phoneExists(const std::string& phone) const;

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

inline CustomerService::CustomerService(CustomerRepository& repository, ReferenceChecker* checker)
    : repo(repository), refChecker(checker) {
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
    std::string trimmedKey = StringHelper::trim(username);
    if (trimmedKey.empty() || password.empty()) {
        return nullptr;
    }

    // getCustomerByUsername / getCustomerByPhone trả về bản ghi gốc trong repository
    Customer* cust = getCustomerByUsername(trimmedKey);
    if (cust == nullptr) {
        cust = getCustomerByPhone(trimmedKey); // Hỗ trợ đăng nhập bằng Số điện thoại làm khóa
    }
    if (cust != nullptr && cust->getAccount().isActive() && cust->getAccount().verifyPassword(password)) {
        return cust;
    }
    return nullptr;
}

inline bool CustomerService::setPasswordCredential(const std::string& customerId,
                                                   const std::string& hashedCredential,
                                                   std::string& errorMessage) {
    if (!PasswordHasher::isHashed(hashedCredential)) {
        errorMessage = "Thong tin xac thuc chua duoc bam, tu choi luu!";
        return false;
    }
    Customer* cust = repo.findById(customerId);
    if (cust == nullptr) {
        errorMessage = "Không tìm thấy khách hàng có mã [" + customerId + "]!";
        return false;
    }
    const std::string previous = cust->getAccount().getPasswordHash();
    cust->getAccount().setPasswordHash(hashedCredential);
    if (!repo.saveToFile()) {
        cust->getAccount().setPasswordHash(previous); // Hoàn nguyên RAM: mật khẩu cũ vẫn còn hiệu lực
        errorMessage = "Lỗi khi lưu thông tin xác thực vào cơ sở dữ liệu!";
        return false;
    }
    return true;
}

inline bool CustomerService::setAccountActive(const std::string& customerId, bool active,
                                              std::string& errorMessage) {
    Customer* cust = repo.findById(customerId);
    if (cust == nullptr) {
        errorMessage = "Không tìm thấy khách hàng có mã [" + customerId + "]!";
        return false;
    }
    const bool previous = cust->getAccount().isActive();
    cust->getAccount().setActive(active);
    if (!repo.saveToFile()) {
        cust->getAccount().setActive(previous);
        errorMessage = "Lỗi khi lưu trạng thái tài khoản vào cơ sở dữ liệu!";
        return false;
    }
    return true;
}

inline bool CustomerService::migrateLegacyCredentials(int& migratedCount, std::string& errorMessage) {
    migratedCount = 0;
    Vector<Customer>& list = repo.getAll();
    std::vector<std::pair<int, std::string>> changed; // (vị trí, giá trị plaintext cũ) để hoàn nguyên nếu lỗi

    for (int i = 0; i < list.getSize(); ++i) {
        Account& acc = list[i].getAccount();
        if (acc.getPasswordHash().empty() || acc.isPasswordHashed()) continue;
        const std::string legacy = acc.getPasswordHash();
        if (!acc.ensureCredentialHashed()) {
            acc.setPasswordHash(legacy);
            for (const auto& c : changed) list[c.first].getAccount().setPasswordHash(c.second);
            errorMessage = "Khong the bam mat khau cho tai khoan khach hang [" + acc.getUsername() + "]!";
            return false;
        }
        changed.push_back(std::make_pair(i, legacy));
    }
    if (changed.empty()) return true;

    if (!repo.saveToFile()) {
        // Không ghi được: giữ nguyên plaintext cũ trong RAM để tài khoản vẫn đăng nhập được như trước
        for (const auto& c : changed) list[c.first].getAccount().setPasswordHash(c.second);
        errorMessage = "Lỗi khi lưu mật khẩu đã băm của khách hàng vào cơ sở dữ liệu!";
        return false;
    }
    migratedCount = static_cast<int>(changed.size());
    return true;
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
    return repo.findByPhone(trimmedPhone);
}

inline bool CustomerService::phoneExists(const std::string& phone) const {
    std::string trimmedPhone = StringHelper::trim(phone);
    if (trimmedPhone.empty()) return false;
    return const_cast<HashTable<std::string, Customer>&>(phoneTable).contains(trimmedPhone) ||
           repo.existsByPhone(trimmedPhone);
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
    if (phone.empty()) {
        errorMessage = "Số điện thoại không được để trống!";
        return false;
    }
    if (!validatePhone(phone, errorMessage)) {
        return false;
    }
    if (phoneTable.contains(phone) || repo.existsByPhone(phone)) {
        errorMessage = "Số điện thoại [" + phone + "] đã được đăng ký bởi khách hàng khác!";
        return false;
    }

    // Không bao giờ lưu mật khẩu dạng rõ: băm trước khi ghi (mật khẩu rỗng bị từ chối)
    if (!registered.getAccount().ensureCredentialHashed()) {
        errorMessage = "Không thể mã hóa mật khẩu cho tài khoản mới!";
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
    if (newPhone.empty()) {
        errorMessage = "Số điện thoại không được để trống!";
        return false;
    }
    if (!validatePhone(newPhone, errorMessage)) {
        return false;
    }
    std::string oldPhone = StringHelper::trim(existing->getPhoneNumber());
    if (newPhone != oldPhone) {
        if (phoneTable.contains(newPhone) || repo.existsByPhone(newPhone)) {
            errorMessage = "Số điện thoại [" + newPhone + "] đã thuộc về tài khoản khác!";
            return false;
        }
    }

    // Cập nhật hồ sơ không được làm mất/đặt lại mật khẩu: thiếu thông tin xác thực thì giữ bản đang lưu,
    // còn plaintext cũ (nếu có) được băm lại mà không đổi mật khẩu.
    Customer toSave = updatedCust;
    if (toSave.getAccount().getPasswordHash().empty()) {
        toSave.getAccount().setPasswordHash(existing->getAccount().getPasswordHash());
    }
    if (!toSave.getAccount().getPasswordHash().empty() && !toSave.getAccount().ensureCredentialHashed()) {
        errorMessage = "Không thể mã hóa mật khẩu khi lưu tài khoản!";
        return false;
    }

    if (!repo.updateCustomer(toSave)) {
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

    if (refChecker != nullptr && refChecker->isCustomerReferenced(id)) {
        errorMessage = "Không thể xóa khách hàng [" + id + "] vì đang có đơn hàng, hóa đơn hoặc yêu cầu bảo hành liên quan!";
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
