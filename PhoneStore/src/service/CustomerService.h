#pragma once
// Nghiệp vụ khách hàng. Toàn bộ hàm hiện là stub.

#include <string>

#include "../common/Actor.h"
#include "../common/Result.h"
#include "../model/people/Customer.h"
#include "../repository/CustomerRepository.h"
#include "../storage/ds/Vector.h"

class CustomerService {
public:
    explicit CustomerService(CustomerRepository& repo);

    Result<void> registerCustomer(const Customer& customer);  // đăng ký: không cần Actor
    Result<Actor> authenticate(const std::string& username, const std::string& password) const;

    Result<Customer> getProfile(const Actor& actor) const;
    Result<void> updateProfile(const Actor& actor, const Customer& customer);
    Result<Customer> getCustomerById(const Actor& actor, const std::string& customerId) const;
    Result<Vector<Customer>> listAll(const Actor& actor) const;
    Result<void> deleteCustomer(const Actor& actor, const std::string& customerId);

private:
    CustomerRepository& customerRepo;
};
