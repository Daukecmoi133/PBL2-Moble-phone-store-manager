#ifndef EMPLOYEESERVICE_H
#define EMPLOYEESERVICE_H

#include <iostream>
#include <string>
#include <vector>
#include "../models/Employee.h"
#include "../core/Vector.h"
#include "../core/HashTable.h"
#include "../repositories/EmployeeRepository.h"
#include "../utils/IdGenerator.h"
#include "../utils/StringHelper.h"
#include "Check.h"

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ nhân sự và xác thực tài khoản
class EmployeeService {
private:
    EmployeeRepository& repo;
    HashTable<std::string, Employee*> usernameTable; // Tra cứu O(1) theo tên đăng nhập
    HashTable<std::string, Employee*> idTable;       // Tra cứu O(1) theo mã nhân viên
    HashTable<std::string, Employee*> phoneTable;    // Tra cứu O(1) theo số điện thoại (khóa tài khoản)
    ReferenceChecker* refChecker;

public:
    EmployeeService(EmployeeRepository& repository, ReferenceChecker* checker = nullptr);
    void setReferenceChecker(ReferenceChecker* checker) { refChecker = checker; }

    void rebuildIndices();
    void reload() { repo.loadFromFile(); rebuildIndices(); }
    EmployeeRepository& getRepository() { return repo; }
    const EmployeeRepository& getRepository() const { return repo; }
    const Vector<Employee*>& getAllEmployees() const;

    // Xác thực đăng nhập nhân viên theo Username hoặc Số điện thoại (khóa) với độ phức tạp O(1) (tài khoản phải đang hoạt động)
    Employee* authenticate(const std::string& usernameOrPhone, const std::string& password);

    // ==== QUẢN LÝ THÔNG TIN XÁC THỰC (ghi xuống Employee.txt, hoàn nguyên RAM nếu ghi thất bại) ====

    // Thay thông tin xác thực đã lưu (hash). Chỉ chấp nhận chuỗi đã băm.
    bool setPasswordCredential(const std::string& employeeId, const std::string& hashedCredential,
                               std::string& errorMessage);

    // Khóa / mở khóa tài khoản và lưu trạng thái
    bool setAccountActive(const std::string& employeeId, bool active, std::string& errorMessage);

    // Chuyển toàn bộ mật khẩu plaintext cũ sang hash (giữ nguyên mật khẩu, ghi file một lần)
    bool migrateLegacyCredentials(int& migratedCount, std::string& errorMessage);

    // Dữ liệu nhân viên không nạp được từ file (đã gọi loadFromFile nhưng thất bại)
    bool hasLoadError() const { return repo.hasLoadError(); }

    Employee* getEmployeeByUsername(const std::string& username);
    Employee* getEmployeeById(const std::string& id);
    Employee* getEmployeeByPhone(const std::string& phone);
    bool phoneExists(const std::string& phone) const;
    static bool validatePhone(const std::string& phone, std::string& errorMessage);

    // Nghiệp vụ thêm nhân viên mới: kiểm tra tính duy nhất của username & id
    bool addEmployee(Employee* emp, std::string& errorMessage);
    bool addEmployee(Employee* emp);

    bool updateEmployee(Employee* emp, std::string& errorMessage);
    bool updateEmployee(Employee* emp);

    bool deleteEmployee(const std::string& id, std::string& errorMessage);
    bool deleteEmployee(const std::string& id);
};

// ================= IMPLEMENTATION =================

inline EmployeeService::EmployeeService(EmployeeRepository& repository, ReferenceChecker* checker)
    : repo(repository), refChecker(checker) {
    rebuildIndices();
}

inline void EmployeeService::rebuildIndices() {
    usernameTable.clear();
    idTable.clear();
    phoneTable.clear();
    const Vector<Employee*>& list = repo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        if (list[i] != nullptr) {
            std::string uname = StringHelper::trim(list[i]->getAccount().getUsername());
            std::string id = StringHelper::trim(list[i]->getID());
            std::string phone = StringHelper::trim(list[i]->getPhoneNumber());
            if (!uname.empty() && usernameTable.find(uname) == nullptr) {
                usernameTable.insert(uname, list[i]);
            }
            if (!id.empty() && idTable.find(id) == nullptr) {
                idTable.insert(id, list[i]);
            }
            if (!phone.empty() && phoneTable.find(phone) == nullptr) {
                phoneTable.insert(phone, list[i]);
            }
        }
    }
}

inline const Vector<Employee*>& EmployeeService::getAllEmployees() const {
    return repo.getAll();
}

inline Employee* EmployeeService::authenticate(const std::string& usernameOrPhone, const std::string& password) {
    const std::string trimmedKey = StringHelper::trim(usernameOrPhone);
    if (trimmedKey.empty() || password.empty()) return nullptr;

    Employee** ptr = usernameTable.find(trimmedKey);
    if (ptr == nullptr || *ptr == nullptr) {
        ptr = phoneTable.find(trimmedKey); // Hỗ trợ đăng nhập bằng Số điện thoại làm khóa
    }
    if (ptr != nullptr && *ptr != nullptr) {
        if ((*ptr)->getAccount().isActive() && (*ptr)->getAccount().verifyPassword(password)) {
            return *ptr;
        }
    }
    return nullptr;
}

inline bool EmployeeService::setPasswordCredential(const std::string& employeeId,
                                                   const std::string& hashedCredential,
                                                   std::string& errorMessage) {
    if (!PasswordHasher::isHashed(hashedCredential)) {
        errorMessage = "Thong tin xac thuc chua duoc bam, tu choi luu!";
        return false;
    }
    Employee* emp = repo.findById(employeeId);
    if (emp == nullptr) {
        errorMessage = "Không tìm thấy nhân viên có mã [" + employeeId + "]!";
        return false;
    }
    const std::string previous = emp->getAccount().getPasswordHash();
    emp->getAccount().setPasswordHash(hashedCredential);
    if (!repo.saveToFile()) {
        emp->getAccount().setPasswordHash(previous); // Hoàn nguyên RAM: mật khẩu cũ vẫn còn hiệu lực
        errorMessage = "Lỗi khi lưu thông tin xác thực vào cơ sở dữ liệu!";
        return false;
    }
    return true;
}

inline bool EmployeeService::setAccountActive(const std::string& employeeId, bool active,
                                              std::string& errorMessage) {
    Employee* emp = repo.findById(employeeId);
    if (emp == nullptr) {
        errorMessage = "Không tìm thấy nhân viên có mã [" + employeeId + "]!";
        return false;
    }
    const bool previous = emp->getAccount().isActive();
    emp->getAccount().setActive(active);
    if (!repo.saveToFile()) {
        emp->getAccount().setActive(previous);
        errorMessage = "Lỗi khi lưu trạng thái tài khoản vào cơ sở dữ liệu!";
        return false;
    }
    return true;
}

inline bool EmployeeService::migrateLegacyCredentials(int& migratedCount, std::string& errorMessage) {
    migratedCount = 0;
    Vector<Employee*>& list = repo.getAll();
    std::vector<std::pair<int, std::string>> changed; // (vị trí, giá trị plaintext cũ) để hoàn nguyên nếu lỗi

    for (int i = 0; i < list.getSize(); ++i) {
        if (list[i] == nullptr) continue;
        Account& acc = list[i]->getAccount();
        if (acc.getPasswordHash().empty() || acc.isPasswordHashed()) continue;
        const std::string legacy = acc.getPasswordHash();
        if (!acc.ensureCredentialHashed()) {
            acc.setPasswordHash(legacy);
            for (const auto& c : changed) list[c.first]->getAccount().setPasswordHash(c.second);
            errorMessage = "Khong the bam mat khau cho tai khoan nhan vien [" + acc.getUsername() + "]!";
            return false;
        }
        changed.push_back(std::make_pair(i, legacy));
    }
    if (changed.empty()) return true;

    if (!repo.saveToFile()) {
        // Không ghi được: giữ nguyên plaintext cũ trong RAM để tài khoản vẫn đăng nhập được như trước
        for (const auto& c : changed) list[c.first]->getAccount().setPasswordHash(c.second);
        errorMessage = "Lỗi khi lưu mật khẩu đã băm của nhân viên vào cơ sở dữ liệu!";
        return false;
    }
    migratedCount = static_cast<int>(changed.size());
    return true;
}

inline Employee* EmployeeService::getEmployeeByUsername(const std::string& username) {
    Employee** ptr = usernameTable.find(StringHelper::trim(username));
    return ptr ? *ptr : nullptr;
}

inline Employee* EmployeeService::getEmployeeById(const std::string& id) {
    Employee** ptr = idTable.find(StringHelper::trim(id));
    return ptr ? *ptr : nullptr;
}

inline Employee* EmployeeService::getEmployeeByPhone(const std::string& phone) {
    std::string trimmedPhone = StringHelper::trim(phone);
    if (trimmedPhone.empty()) return nullptr;
    Employee** ptr = phoneTable.find(trimmedPhone);
    return (ptr != nullptr && *ptr != nullptr) ? *ptr : repo.findByPhone(trimmedPhone);
}

inline bool EmployeeService::phoneExists(const std::string& phone) const {
    std::string trimmedPhone = StringHelper::trim(phone);
    if (trimmedPhone.empty()) return false;
    return const_cast<HashTable<std::string, Employee*>&>(phoneTable).contains(trimmedPhone) ||
           repo.existsByPhone(trimmedPhone);
}

inline bool EmployeeService::validatePhone(const std::string& phone, std::string& errorMessage) {
    std::string p = StringHelper::trim(phone);
    if (p.empty()) {
        errorMessage = "Số điện thoại không được để trống!";
        return false;
    }
    if (p.length() < 10 || p.length() > 11) {
        errorMessage = "Số điện thoại phải có 10-11 chữ số!";
        return false;
    }
    if (p[0] != '0') {
        errorMessage = "Số điện thoại phải bắt đầu bằng 0!";
        return false;
    }
    for (char c : p) {
        if (c < '0' || c > '9') {
            errorMessage = "Số điện thoại chỉ được chứa chữ số!";
            return false;
        }
    }
    return true;
}

inline bool EmployeeService::addEmployee(Employee* emp, std::string& errorMessage) {
    if (emp == nullptr) {
        errorMessage = "Dữ liệu nhân viên không hợp lệ!";
        return false;
    }

    std::string uname = StringHelper::trim(emp->getAccount().getUsername());
    if (uname.empty()) {
        errorMessage = "Tên đăng nhập không được để trống!";
        return false;
    }

    std::string phone = StringHelper::trim(emp->getPhoneNumber());
    if (phone.empty()) {
        errorMessage = "Số điện thoại của nhân viên không được để trống!";
        return false;
    }
    if (!validatePhone(phone, errorMessage)) {
        return false;
    }

    if (emp->getID().empty()) {
        std::vector<std::string> existingIds;
        const Vector<Employee*>& employees = repo.getAll();
        for (int i = 0; i < employees.getSize(); ++i) {
            if (employees[i] != nullptr) existingIds.push_back(employees[i]->getID());
        }
        std::string generatedId;
        if (!IdGenerator::nextId("EMP", "", existingIds, generatedId, errorMessage)) {
            return false;
        }
        emp->setID(generatedId);
    } else if (!IdGenerator::isValidId(emp->getID(), "EMP")) {
        errorMessage = "Mã nhân viên phải đúng định dạng EMP-000001.";
        return false;
    }

    if (emp->getAccount().getID().empty()) {
        std::vector<std::string> accountIds;
        const Vector<Employee*>& employees = repo.getAll();
        for (int i = 0; i < employees.getSize(); ++i) {
            if (employees[i] != nullptr) {
                accountIds.push_back(employees[i]->getAccount().getID());
            }
        }
        std::string generatedAccountId;
        if (!IdGenerator::nextId("ACC", "", accountIds,
                                 generatedAccountId, errorMessage)) {
            return false;
        }
        emp->getAccount().setID(generatedAccountId);
    } else if (!IdGenerator::isValidId(emp->getAccount().getID(), "ACC")) {
        errorMessage = "Mã tài khoản phải đúng định dạng ACC-000001.";
        return false;
    }

    if (usernameTable.contains(uname)) {
        errorMessage = "Tên đăng nhập [" + uname + "] đã được sử dụng!";
        return false;
    }

    if (phoneTable.contains(phone)) {
        errorMessage = "Số điện thoại [" + phone + "] đã được sử dụng bởi nhân viên khác!";
        return false;
    }

    if (idTable.contains(emp->getID())) {
        errorMessage = "Mã nhân viên [" + emp->getID() + "] đã tồn tại!";
        return false;
    }

    // Không bao giờ lưu mật khẩu dạng rõ: băm trước khi ghi
    if (!emp->getAccount().ensureCredentialHashed()) {
        errorMessage = "Không thể mã hóa mật khẩu cho tài khoản nhân viên mới!";
        return false;
    }

    if (!repo.addEmployee(emp)) {
        errorMessage = "Lỗi khi lưu nhân viên vào cơ sở dữ liệu!";
        return false;
    }

    usernameTable.insert(uname, emp);
    phoneTable.insert(phone, emp);
    idTable.insert(emp->getID(), emp);
    return true;
}

inline bool EmployeeService::addEmployee(Employee* emp) {
    std::string err;
    return addEmployee(emp, err);
}

inline bool EmployeeService::updateEmployee(Employee* emp, std::string& errorMessage) {
    if (emp == nullptr) {
        errorMessage = "Dữ liệu nhân viên không hợp lệ!";
        return false;
    }
    if (!idTable.contains(emp->getID())) {
        errorMessage = "Không tìm thấy nhân viên có mã [" + emp->getID() + "] để cập nhật!";
        return false;
    }

    Employee* existing = repo.findById(emp->getID());
    if (existing == nullptr) {
        errorMessage = "Không tìm thấy nhân viên có mã [" + emp->getID() + "] để cập nhật!";
        return false;
    }

    std::string newPhone = StringHelper::trim(emp->getPhoneNumber());
    if (newPhone.empty()) {
        errorMessage = "Số điện thoại của nhân viên không được để trống!";
        return false;
    }
    if (!validatePhone(newPhone, errorMessage)) {
        return false;
    }
    std::string oldPhone = StringHelper::trim(existing->getPhoneNumber());
    if (newPhone != oldPhone && phoneTable.contains(newPhone)) {
        errorMessage = "Số điện thoại [" + newPhone + "] đã được sử dụng bởi nhân viên khác!";
        return false;
    }

    if (!repo.updateEmployee(emp)) {
        errorMessage = "Lỗi khi cập nhật nhân viên vào cơ sở dữ liệu!";
        return false;
    }
    rebuildIndices();
    return true;
}

inline bool EmployeeService::updateEmployee(Employee* emp) {
    std::string err;
    return updateEmployee(emp, err);
}

inline bool EmployeeService::deleteEmployee(const std::string& id, std::string& errorMessage) {
    if (!idTable.contains(id)) {
        errorMessage = "Không tìm thấy nhân viên có mã [" + id + "]!";
        return false;
    }

    if (refChecker != nullptr && refChecker->isEmployeeReferenced(id)) {
        errorMessage = "Không thể xóa nhân viên [" + id + "] vì đang được tham chiếu trong hóa đơn hoặc phiếu nhập!";
        return false;
    }

    if (!repo.deleteEmployee(id)) {
        errorMessage = "Lỗi khi xóa nhân viên khỏi cơ sở dữ liệu!";
        return false;
    }

    rebuildIndices();
    return true;
}

inline bool EmployeeService::deleteEmployee(const std::string& id) {
    std::string err;
    return deleteEmployee(id, err);
}

#endif // EMPLOYEESERVICE_H
