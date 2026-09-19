#ifndef CUSTOMERSERVICE_H
#define CUSTOMERSERVICE_H

#include "../core/HashTable.h"
#include "../core/Vector.h"
#include "../models/Customer.h"
#include "../repositories/CustomerRepository.h"
#include "../utils/StringHelper.h"
#include <iostream>
#include <string>

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ khách hàng và xác thực
class CustomerService {
private:
  CustomerRepository &repo;
  HashTable<std::string, Customer> usernameTable; // Tra cứu O(1) theo username
  HashTable<std::string, Customer>
      phoneTable; // Tra cứu O(1) theo số điện thoại

public:
  CustomerService(CustomerRepository &repository);

  void rebuildIndices();
  const Vector<Customer> &getAllCustomers() const;

  // Xác thực đăng nhập khách hàng
  Customer *authenticate(const std::string &username,
                         const std::string &password);

  Customer *getCustomerByUsername(const std::string &username);
  Customer *getCustomerByPhone(const std::string &phone);
  Customer *getCustomerById(const std::string &id);

  // Nghiệp vụ đăng ký khách hàng mới
  bool registerCustomer(const Customer &cust, std::string &errorMessage);
  bool registerCustomer(const Customer &cust);

  bool updateCustomer(const Customer &updatedCust, std::string &errorMessage);
  bool updateCustomer(const Customer &updatedCust);
};

// ================= IMPLEMENTATION =================

inline CustomerService::CustomerService(CustomerRepository &repository)
    : repo(repository) {
  rebuildIndices();
}

inline void CustomerService::rebuildIndices() {
  usernameTable.clear();
  phoneTable.clear();
  const Vector<Customer> &list = repo.getAll();
  for (int i = 0; i < list.getSize(); ++i) {
    usernameTable.insert(list[i].getAccount().getUsername(), list[i]);
    phoneTable.insert(list[i].getPhoneNumber(), list[i]);
  }
}

inline const Vector<Customer> &CustomerService::getAllCustomers() const {
  return repo.getAll();
}

inline Customer *CustomerService::authenticate(const std::string &username,
                                               const std::string &password) {
  Customer *cust = usernameTable.find(username);
  if (cust != nullptr && cust->getAccount().authenticate(username, password)) {
    return cust;
  }
  return nullptr;
}

inline Customer *
CustomerService::getCustomerByUsername(const std::string &username) {
  return usernameTable.find(username);
}

inline Customer *CustomerService::getCustomerByPhone(const std::string &phone) {
  return phoneTable.find(phone);
}

inline Customer *CustomerService::getCustomerById(const std::string &id) {
  return repo.findById(id);
}

inline bool CustomerService::registerCustomer(const Customer &cust,
                                              std::string &errorMessage) {
  std::string uname = cust.getAccount().getUsername();
  if (uname.empty()) {
    errorMessage = "Tên đăng nhập không được để trống!";
    return false;
  }

  if (usernameTable.contains(uname)) {
    errorMessage = "Tên đăng nhập [" + uname + "] đã tồn tại!";
    return false;
  }

  if (!cust.getPhoneNumber().empty() &&
      phoneTable.contains(cust.getPhoneNumber())) {
    errorMessage =
        "Số điện thoại [" + cust.getPhoneNumber() + "] đã được đăng ký!";
    return false;
  }

  if (!repo.addCustomer(cust)) {
    errorMessage = "Lỗi khi lưu thông tin khách hàng!";
    return false;
  }

  usernameTable.insert(uname, cust);
  if (!cust.getPhoneNumber().empty()) {
    phoneTable.insert(cust.getPhoneNumber(), cust);
  }
  return true;
}

inline bool CustomerService::registerCustomer(const Customer &cust) {
  std::string err;
  return registerCustomer(cust, err);
}

inline bool CustomerService::updateCustomer(const Customer &updatedCust,
                                            std::string &errorMessage) {
  if (!repo.updateCustomer(updatedCust)) {
    errorMessage = "Không tìm thấy khách hàng để cập nhật!";
    return false;
  }

  rebuildIndices();
  return true;
}

inline bool CustomerService::updateCustomer(const Customer &updatedCust) {
  std::string err;
  return updateCustomer(updatedCust, err);
}

#endif // CUSTOMERSERVICE_H
