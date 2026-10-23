#include "CustomerService.h"

CustomerService::CustomerService(CustomerRepository& repo) : customerRepo(repo) {}

Result<void> CustomerService::registerCustomer(const Customer&) {
    // TODO(logic): kiểm tra hợp lệ, trùng username/số điện thoại, tạo ID, add + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: CustomerService::registerCustomer");
}

Result<Actor> CustomerService::authenticate(const std::string&, const std::string&) const {
    // TODO(logic): findByUsername, đối chiếu mật khẩu, kiểm tra tài khoản còn active, trả Actor{id, CUSTOMER}.
    return Result<Actor>::fail(ErrorCode::NotImplemented, "TODO: CustomerService::authenticate");
}

Result<Customer> CustomerService::getProfile(const Actor&) const {
    // TODO(logic): trả hồ sơ của chính Actor (khách chỉ xem được hồ sơ của mình).
    return Result<Customer>::fail(ErrorCode::NotImplemented, "TODO: CustomerService::getProfile");
}

Result<void> CustomerService::updateProfile(const Actor&, const Customer&) {
    // TODO(logic): kiểm tra quyền sở hữu và hợp lệ dữ liệu, update + save.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: CustomerService::updateProfile");
}

Result<Customer> CustomerService::getCustomerById(const Actor&, const std::string&) const {
    // TODO(logic): kiểm tra quyền (nhân viên hoặc chính khách), findById.
    return Result<Customer>::fail(ErrorCode::NotImplemented, "TODO: CustomerService::getCustomerById");
}

Result<Vector<Customer>> CustomerService::listAll(const Actor&) const {
    // TODO(logic): kiểm tra quyền ManageCustomer rồi trả danh sách.
    return Result<Vector<Customer>>::fail(ErrorCode::NotImplemented, "TODO: CustomerService::listAll");
}

Result<void> CustomerService::deleteCustomer(const Actor&, const std::string&) {
    // TODO(logic): kiểm tra quyền ManageCustomer và ràng buộc tham chiếu (đơn hàng) trước khi xóa.
    return Result<void>::fail(ErrorCode::NotImplemented, "TODO: CustomerService::deleteCustomer");
}
