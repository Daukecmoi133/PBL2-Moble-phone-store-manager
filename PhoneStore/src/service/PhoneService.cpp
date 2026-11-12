#include "PhoneService.h"
#include "../common/Permission.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

PhoneService::PhoneService(PhoneRepository& repo) : phoneRepo(repo) {}

Result<Vector<PhoneView>> PhoneService::listForCustomer(Vector<std::string>* outIds) const {
    if (outIds) outIds->clear();
    Vector<Phone> all = phoneRepo.getAll();
    Vector<PhoneView> views;
    for (int i = 0; i < all.getSize(); ++i) {
        const Phone& p = all[i];
        if (!p.isListed()) continue; // Khách không bao giờ thấy sản phẩm listed == false

        PhoneView pv;
        pv.name = p.getName();
        pv.brand = p.getBrand();
        pv.operatingSystem = p.getOperatingSystem();
        pv.specifications = p.getSpecifications();
        pv.sellingPrice = static_cast<Money>(p.getSellingPrice());
        pv.inStock = p.availableQuantity() > 0;
        views.push_back(pv);

        if (outIds) outIds->push_back(p.getID());
    }
    return Result<Vector<PhoneView>>::ok(views);
}

Result<Vector<PhoneStaffView>> PhoneService::listForStaff(const Actor& actor) const {
    if (actor.role == Role::CUSTOMER) {
        return Result<Vector<PhoneStaffView>>::fail(ErrorCode::PermissionDenied, "Khách hàng không có quyền xem thông tin nội bộ của nhân viên");
    }
    Vector<Phone> all = phoneRepo.getAll();
    Vector<PhoneStaffView> views;
    for (int i = 0; i < all.getSize(); ++i) {
        const Phone& p = all[i];
        PhoneStaffView sv;
        sv.id = p.getID();
        sv.name = p.getName();
        sv.brand = p.getBrand();
        sv.operatingSystem = p.getOperatingSystem();
        sv.importPrice = static_cast<Money>(p.getImportPrice());
        sv.sellingPrice = static_cast<Money>(p.getSellingPrice());
        sv.quantity = p.getQuantity();
        sv.reservedQuantity = p.getReservedQuantity();
        sv.listed = p.isListed();
        sv.manufacturer = p.getManufacturer();
        views.push_back(sv);
    }
    return Result<Vector<PhoneStaffView>>::ok(views);
}

Result<Vector<PhoneStaffView>> PhoneService::listDrafts(const Actor& actor) const {
    if (!Permission::isAllowed(actor.role, Action::ManagePhone)) {
        return Result<Vector<PhoneStaffView>>::fail(ErrorCode::PermissionDenied, "Chỉ quản trị viên mới có quyền xem danh sách bản nháp");
    }
    Vector<Phone> all = phoneRepo.getAll();
    Vector<PhoneStaffView> views;
    for (int i = 0; i < all.getSize(); ++i) {
        const Phone& p = all[i];
        if (p.isListed()) continue; // Chỉ lấy các sản phẩm chưa đăng bán

        PhoneStaffView sv;
        sv.id = p.getID();
        sv.name = p.getName();
        sv.brand = p.getBrand();
        sv.operatingSystem = p.getOperatingSystem();
        sv.importPrice = static_cast<Money>(p.getImportPrice());
        sv.sellingPrice = static_cast<Money>(p.getSellingPrice());
        sv.quantity = p.getQuantity();
        sv.reservedQuantity = p.getReservedQuantity();
        sv.listed = p.isListed();
        sv.manufacturer = p.getManufacturer();
        views.push_back(sv);
    }
    return Result<Vector<PhoneStaffView>>::ok(views);
}

Result<Vector<PhoneView>> PhoneService::search(const std::string& keyword, Vector<std::string>* outIds) const {
    if (outIds) outIds->clear();
    Vector<Phone> all = phoneRepo.getAll();
    Vector<PhoneView> views;
    for (int i = 0; i < all.getSize(); ++i) {
        const Phone& p = all[i];
        if (!p.isListed()) continue;
        if (!StringHelper::containsIgnoreCase(p.getName(), keyword) &&
            !StringHelper::containsIgnoreCase(p.getBrand(), keyword)) continue;

        PhoneView pv;
        pv.name = p.getName();
        pv.brand = p.getBrand();
        pv.operatingSystem = p.getOperatingSystem();
        pv.specifications = p.getSpecifications();
        pv.sellingPrice = static_cast<Money>(p.getSellingPrice());
        pv.inStock = p.availableQuantity() > 0;
        views.push_back(pv);

        if (outIds) outIds->push_back(p.getID());
    }
    return Result<Vector<PhoneView>>::ok(views);
}

Result<Vector<PhoneView>> PhoneService::filterByPrice(Money minPrice, Money maxPrice, Vector<std::string>* outIds) const {
    if (outIds) outIds->clear();
    Vector<Phone> all = phoneRepo.getAll();
    Vector<PhoneView> views;
    for (int i = 0; i < all.getSize(); ++i) {
        const Phone& p = all[i];
        if (!p.isListed()) continue;
        if (p.getSellingPrice() < minPrice || p.getSellingPrice() > maxPrice) continue;

        PhoneView pv;
        pv.name = p.getName();
        pv.brand = p.getBrand();
        pv.operatingSystem = p.getOperatingSystem();
        pv.specifications = p.getSpecifications();
        pv.sellingPrice = static_cast<Money>(p.getSellingPrice());
        pv.inStock = p.availableQuantity() > 0;
        views.push_back(pv);

        if (outIds) outIds->push_back(p.getID());
    }
    return Result<Vector<PhoneView>>::ok(views);
}

Result<void> PhoneService::addPhone(const Actor& actor, const Phone& phone) {
    if (!Permission::isAllowed(actor.role, Action::ManagePhone)) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Không có quyền thêm sản phẩm");
    }
    if (phone.getID().empty() || phone.getName().empty() || phone.getBrand().empty()) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Mã, tên và hãng sản phẩm không được để trống");
    }
    Phone p = phone;
    p.setListed(true); // addPhone trực tiếp do Admin gọi có listed = true
    auto addRes = phoneRepo.add(p);
    if (!addRes.isOk()) return addRes;
    return phoneRepo.save();
}

Result<void> PhoneService::updatePhone(const Actor& actor, const Phone& phone) {
    if (!Permission::isAllowed(actor.role, Action::ManagePhone)) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Không có quyền sửa sản phẩm");
    }
    auto updateRes = phoneRepo.update(phone);
    if (!updateRes.isOk()) return updateRes;
    return phoneRepo.save();
}

Result<void> PhoneService::removePhone(const Actor& actor, const std::string& id) {
    if (!Permission::isAllowed(actor.role, Action::ManagePhone)) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Không có quyền xóa sản phẩm");
    }
    // TODO(logic): kiểm tra ràng buộc tham chiếu (đơn, hóa đơn, phiếu nhập) trước khi xóa (xem OPEN_QUESTIONS.md).
    auto remRes = phoneRepo.remove(id);
    if (!remRes.isOk()) return remRes;
    return phoneRepo.save();
}

Result<void> PhoneService::publishPhone(const Actor& actor, const std::string& id) {
    if (!Permission::isAllowed(actor.role, Action::ManagePhone)) {
        return Result<void>::fail(ErrorCode::PermissionDenied, "Chỉ quản trị viên mới có quyền đăng bán sản phẩm");
    }
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) return Result<void>::fail(phoneRes.code(), phoneRes.message());

    Phone p = phoneRes.value();
    if (p.getSellingPrice() <= 0 || p.getName().empty() || p.getBrand().empty()) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Không thể đăng bán: giá bán phải lớn hơn 0 và tên, hãng không được để trống");
    }
    p.setListed(true);
    auto updateRes = phoneRepo.update(p);
    if (!updateRes.isOk()) return updateRes;
    return phoneRepo.save();
}

Result<Phone> PhoneService::getPhoneById(const std::string& id) const {
    return phoneRepo.findById(id);
}

Result<void> PhoneService::checkAvailable(const std::string& id, int qty) const {
    if (qty <= 0) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng yêu cầu phải lớn hơn 0");
    }
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) {
        return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy sản phẩm");
    }
    const Phone& p = phoneRes.value();
    if (!p.isListed()) {
        return Result<void>::fail(ErrorCode::OutOfStock, "Sản phẩm chưa được đăng bán");
    }
    if (p.availableQuantity() < qty) {
        return Result<void>::fail(ErrorCode::OutOfStock, "Không đủ tồn kho khả dụng");
    }
    return Result<void>::ok();
}

Result<void> PhoneService::reserve(UnitOfWork& uow, const std::string& id, int qty) {
    if (qty <= 0) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng giữ chỗ phải lớn hơn 0");
    }
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) return Result<void>::fail(phoneRes.code(), phoneRes.message());

    Phone p = phoneRes.value();
    if (!p.isListed()) {
        return Result<void>::fail(ErrorCode::OutOfStock, "Sản phẩm chưa được đăng bán");
    }
    if (p.availableQuantity() < qty) {
        return Result<void>::fail(ErrorCode::OutOfStock, "Không đủ tồn kho khả dụng để giữ chỗ");
    }
    uow.enlist(phoneRepo);
    p.setReservedQuantity(p.getReservedQuantity() + qty);
    phoneRepo.update(p);
    return Result<void>::ok();
}

Result<void> PhoneService::releaseReserved(UnitOfWork& uow, const std::string& id, int qty) {
    if (qty <= 0) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng giải phóng phải lớn hơn 0");
    }
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) return Result<void>::fail(phoneRes.code(), phoneRes.message());

    Phone p = phoneRes.value();
    uow.enlist(phoneRepo);
    p.setReservedQuantity(std::max(0, p.getReservedQuantity() - qty));
    phoneRepo.update(p);
    return Result<void>::ok();
}

Result<void> PhoneService::commitReserved(UnitOfWork& uow, const std::string& id, int qty) {
    if (qty <= 0) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng xuất kho phải lớn hơn 0");
    }
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) return Result<void>::fail(phoneRes.code(), phoneRes.message());

    Phone p = phoneRes.value();
    uow.enlist(phoneRepo);
    p.setReservedQuantity(std::max(0, p.getReservedQuantity() - qty));
    p.setQuantity(std::max(0, p.getQuantity() - qty));
    phoneRepo.update(p);
    return Result<void>::ok();
}

Result<void> PhoneService::restock(UnitOfWork& uow, const std::string& id, int qty) {
    if (qty <= 0) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng hoàn kho phải lớn hơn 0");
    }
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) return Result<void>::fail(phoneRes.code(), phoneRes.message());

    Phone p = phoneRes.value();
    uow.enlist(phoneRepo);
    p.setQuantity(p.getQuantity() + qty);
    phoneRepo.update(p);
    return Result<void>::ok();
}

Result<void> PhoneService::receiveStock(UnitOfWork& uow, const std::string& id, int qty) {
    if (qty <= 0) {
        return Result<void>::fail(ErrorCode::InvalidInput, "Số lượng nhập kho phải lớn hơn 0");
    }
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) return Result<void>::fail(phoneRes.code(), phoneRes.message());

    Phone p = phoneRes.value();
    uow.enlist(phoneRepo);
    p.setQuantity(p.getQuantity() + qty);
    phoneRepo.update(p);
    return Result<void>::ok();
}

Result<void> PhoneService::updateImportPrice(UnitOfWork& uow, const std::string& id, Money price) {
    auto phoneRes = phoneRepo.findById(id);
    if (!phoneRes.isOk()) return Result<void>::fail(phoneRes.code(), phoneRes.message());

    Phone p = phoneRes.value();
    uow.enlist(phoneRepo);
    p.setImportPrice(static_cast<double>(price));
    phoneRepo.update(p);
    return Result<void>::ok();
}

Result<std::string> PhoneService::createDraftForImport(UnitOfWork& uow, const NewPhoneInfo& info, Money importPrice) {
    std::string nameTrimmed = StringHelper::trim(info.name);
    std::string brandTrimmed = StringHelper::trim(info.brand);
    if (nameTrimmed.empty() || brandTrimmed.empty()) {
        return Result<std::string>::fail(ErrorCode::InvalidInput, "Tên và hãng sản phẩm mới không được để trống");
    }

    // Chống trùng: kiểm tra (tên + hãng) không phân biệt hoa thường
    Vector<Phone> all = phoneRepo.getAll();
    for (int i = 0; i < all.getSize(); ++i) {
        if (StringHelper::toLower(all[i].getName()) == StringHelper::toLower(nameTrimmed) &&
            StringHelper::toLower(all[i].getBrand()) == StringHelper::toLower(brandTrimmed)) {
            return Result<std::string>::fail(ErrorCode::Duplicate, "Sản phẩm đã tồn tại trong hệ thống, hãy nhập vào sản phẩm có sẵn");
        }
    }

    // Sinh ID mới theo định dạng PHN-xxxxxx
    int maxId = 0;
    for (int i = 0; i < all.getSize(); ++i) {
        std::string curId = all[i].getID();
        if (curId.rfind("PHN-", 0) == 0) {
            try {
                int num = std::stoi(curId.substr(4));
                if (num > maxId) maxId = num;
            } catch (...) {}
        }
    }
    std::ostringstream oss;
    oss << "PHN-" << std::setfill('0') << std::setw(6) << (maxId + 1);
    std::string newId = oss.str();

    Phone p;
    p.setID(newId);
    p.setName(nameTrimmed);
    p.setBrand(brandTrimmed);
    p.setOperatingSystem(StringHelper::trim(info.operatingSystem));
    p.setManufacturer(StringHelper::trim(info.manufacturer));
    p.setSpecifications(StringHelper::trim(info.specifications));
    p.setQuantity(0);
    p.setReservedQuantity(0);
    p.setListed(false);
    p.setSellingPrice(0.0);
    p.setImportPrice(static_cast<double>(importPrice));

    uow.enlist(phoneRepo);
    auto addRes = phoneRepo.add(p);
    if (!addRes.isOk()) return Result<std::string>::fail(addRes.code(), addRes.message());

    return Result<std::string>::ok(newId);
}
