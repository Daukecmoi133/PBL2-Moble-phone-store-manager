#ifndef APP_CONTROLLER_SUPPORT_H
#define APP_CONTROLLER_SUPPORT_H
#include "../../UI/mock/MockDataStore.h"
#include "../../UI/mock/RealTimeManager.h"
#include "../services/AuthService.h"
#include "../services/CustomerService.h"
#include "../services/DiscountService.h"
#include "../services/EmployeeService.h"
#include "../services/InvoiceService.h"
#include "../services/OrderService.h"
#include "../services/PhoneService.h"
#include "../services/ReportService.h"
#include "../services/SupplierService.h"
#include "../services/WarrantyService.h"
#include "../utils/IdGenerator.h"
#include "../utils/PathHelper.h"
#include "../utils/TransactionCoordinator.h"
#include "../utils/Utf8Helper.h"
#include "AppController.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace AppControllerHelpers {
// ---- Tiện ích xác thực dữ liệu đầu vào dùng chung ----
inline std::string trimCopy(const std::string &s) {
  size_t b = 0, e = s.size();
  while (b < e && std::isspace(static_cast<unsigned char>(s[b])))
    ++b;
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1])))
    --e;
  return s.substr(b, e - b);
}

// SĐT Việt Nam: đúng 10 chữ số, bắt đầu bằng 0
inline bool isValidPhone(const std::string &phone) {
  if (phone.size() != 10 || phone[0] != '0')
    return false;
  for (char c : phone) {
    if (!std::isdigit(static_cast<unsigned char>(c)))
      return false;
  }
  return true;
}

// Email cơ bản: có đúng 1 '@', phần tên khác rỗng, miền có dấu '.' không ở
// đầu/cuối, không có khoảng trắng
inline bool isValidEmail(const std::string &email) {
  size_t at = email.find('@');
  if (at == std::string::npos || at == 0 ||
      email.find('@', at + 1) != std::string::npos)
    return false;
  for (char c : email) {
    if (std::isspace(static_cast<unsigned char>(c)))
      return false;
  }
  std::string domain = email.substr(at + 1);
  size_t dot = domain.find('.');
  return dot != std::string::npos && dot > 0 && domain.back() != '.';
}

// Phân tích ngày DD/MM/YYYY nghiêm ngặt (kiểm tra tháng, số ngày trong tháng,
// năm nhuận)
inline bool parseDateDMY(const std::string &s, int &d, int &m, int &y) {
  int consumed = 0;
  if (std::sscanf(s.c_str(), "%d/%d/%d%n", &d, &m, &y, &consumed) != 3)
    return false;
  if (static_cast<size_t>(consumed) != s.size())
    return false;
  if (y < 1900 || y > 9999 || m < 1 || m > 12 || d < 1)
    return false;
  static const int daysInMonth[] = {31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};
  int maxDay = daysInMonth[m - 1];
  bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
  if (m == 2 && leap)
    maxDay = 29;
  return d <= maxDay;
}

inline std::string formatDateTimeMinute(const DateTime &dt) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%02d/%02d/%04d %02d:%02d", dt.day, dt.month,
                dt.year, dt.hour, dt.minute);
  return std::string(buf);
}

inline int calculateCivilDayDiff(const DateTime &fromDate,
                                 const DateTime &toDate) {
  auto daysFromCivil = [](int y, int m, int d) -> int {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int>(doe) - 719468;
  };
  return daysFromCivil(toDate.year, toDate.month, toDate.day) -
         daysFromCivil(fromDate.year, fromDate.month, fromDate.day);
}

// Phân quyền ở tầng nghiệp vụ: chặn thao tác nhạy cảm dù giao diện có bị gọi
// trực tiếp hay không
#define REQUIRE_ADMIN_ROLE()                                                   \
  do {                                                                         \
    if (!currentSession.isAuthenticated || !currentSession.isAdmin()) {        \
      return OperationResult::Fail(                                            \
          "Từ chối truy cập: Thao tác này chỉ dành cho Quản trị viên!");       \
    }                                                                          \
  } while (0)

#define REQUIRE_ADMIN_OR_WAREHOUSE_ROLE()                                      \
  do {                                                                         \
    if (!currentSession.isAuthenticated ||                                     \
        !(currentSession.isAdmin() || currentSession.isWarehouse())) {         \
      return OperationResult::Fail("Từ chối truy cập: Thao tác này chỉ dành "  \
                                   "cho Quản trị viên hoặc Thủ kho!");         \
    }                                                                          \
  } while (0)

#define REQUIRE_ADMIN_OR_SALES_ROLE()                                          \
  do {                                                                         \
    if (!currentSession.isAuthenticated ||                                     \
        !(currentSession.isAdmin() || currentSession.isSales())) {             \
      return OperationResult::Fail(                                            \
          "Từ chối truy cập: Thao tác này chỉ dành cho Nhân viên Bán hàng "    \
          "hoặc Quản trị viên!");                                              \
    }                                                                          \
  } while (0)

template <typename Records>
inline std::vector<std::string> collectIds(const Records &records) {
  std::vector<std::string> ids;
  ids.reserve(records.size());
  for (const auto &record : records)
    ids.push_back(record.id);
  return ids;
}

// ==== Các hàm chuyển đổi Model <-> ViewModel ====

inline std::vector<std::pair<std::string, std::string>>
parseDetailedSpecs(const std::string &raw) {
  std::vector<std::pair<std::string, std::string>> result;
  std::istringstream stream(raw);
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    std::string trimmed = StringHelper::trim(line);
    if (trimmed.empty())
      continue;
    size_t col = trimmed.find(':');
    if (col != std::string::npos) {
      std::string label = StringHelper::trim(trimmed.substr(0, col));
      std::string val = StringHelper::trim(trimmed.substr(col + 1));
      while (!label.empty() &&
             (label[0] == '-' || label[0] == '+' || label[0] == ' ')) {
        label = StringHelper::trim(label.substr(1));
      }
      if (!label.empty() && !val.empty()) {
        result.push_back({label, val});
      }
    }
  }
  return result;
}

inline std::string serializeDetailedSpecs(
    const std::vector<std::pair<std::string, std::string>> &specs) {
  std::ostringstream ss;
  for (const auto &kv : specs) {
    ss << kv.first << ":" << kv.second << "\n";
  }
  return ss.str();
}

inline ProductItemViewModel
phoneToViewModel(const Phone &p, PhoneService *ps = nullptr,
                 ProductMarketingRepository *mktRepo = nullptr,
                 ProductDescriptionRepository *descRepo = nullptr) {
  ProductItemViewModel vm;
  vm.id = p.getID();
  vm.name = p.getName();
  vm.brand = p.getBrand();
  vm.os = p.getOperatingSystem();
  vm.importPrice = p.getImportPrice();
  vm.sellingPrice = p.getSellingPrice();
  vm.actualStock = p.getQuantity();
  vm.reservedStock = 0;
  vm.manufacturer = p.getManufacturer();
  vm.specs = p.getSpecifications();

  // Thông số chi tiết từ Thongsokythuat.txt
  std::string rawSpecs = p.getDetailedSpecifications();
  if (!rawSpecs.empty()) {
    vm.detailedSpecs = parseDetailedSpecs(rawSpecs);
  }
  if (vm.detailedSpecs.empty() && !vm.specs.empty()) {
    auto parts = StringHelper::split(vm.specs, ',');
    for (int i = 0; i < parts.getSize(); ++i) {
      std::string t = StringHelper::trim(parts[i]);
      if (t.find("inch") != std::string::npos ||
          t.find("AMOLED") != std::string::npos ||
          t.find("OLED") != std::string::npos) {
        vm.detailedSpecs.push_back({"Màn hình", t});
      } else if (t.find("RAM") != std::string::npos) {
        vm.detailedSpecs.push_back({"Bộ nhớ RAM", t});
      } else if (t.find("GB") != std::string::npos ||
                 t.find("TB") != std::string::npos) {
        vm.detailedSpecs.push_back({"Bộ nhớ trong (ROM)", t});
      } else if (t.find("mAh") != std::string::npos) {
        vm.detailedSpecs.push_back({"Dung lượng pin", t});
      } else {
        vm.detailedSpecs.push_back({"Vi xử lý (CPU)", t});
      }
    }
  }

  // Ảnh sản phẩm
  if (ps != nullptr && ps->getProductImageRepository() != nullptr) {
    auto imgRecs = ps->getProductImageRepository()->getImagesForProduct(vm.id);
    for (const auto &r : imgRecs) {
      vm.images.push_back(
          ProductImageItem(r.imagePath, r.isPrimary == 1, r.displayOrder));
    }
    std::string primaryImg = ps->getProductImagePath(vm.id);
    if (!primaryImg.empty()) {
      vm.imageUrl = primaryImg;
    }
  }
  if (vm.imageUrl.empty() && !vm.images.empty()) {
    vm.imageUrl = vm.images[0].path;
  }
  if (vm.imageUrl.empty() && vm.id == "PHN-000101") {
    vm.imageUrl = "Device/productImages/oppo_reno_16_f_5g_bac.png";
  }

  // Dữ liệu Marketing & Khuyến mãi
  if (mktRepo != nullptr) {
    ProductMarketingRecord mkt = mktRepo->getByProductId(vm.id);
    vm.listPrice = mkt.listPrice;
    vm.saleStart = mkt.saleStart;
    vm.saleEnd = mkt.saleEnd;
    vm.installmentEnabled = mkt.installmentEnabled;
    vm.installmentNote = mkt.installmentNote;
    vm.colors = mkt.colors;
    vm.warrantyMonths = mkt.warrantyMonths;
    vm.returnDays = mkt.returnDays;
  }

  // Dữ liệu mô tả sản phẩm
  if (descRepo != nullptr) {
    ProductDescriptionRecord desc = descRepo->getByProductId(vm.id);
    vm.shortHighlights = desc.shortHighlights;
    vm.description = desc.description;
  }

  // Phân tích Dòng máy (baseModel), Màu sắc (color), Dung lượng (capacity)
  // (ANTY.md mục 1)
  std::string rawName = vm.name;
  size_t dashPos = rawName.find(" - ");
  std::string modelPart =
      (dashPos != std::string::npos) ? rawName.substr(0, dashPos) : rawName;
  if (dashPos != std::string::npos) {
    vm.color = StringHelper::trim(rawName.substr(dashPos + 3));
  } else if (!vm.colors.empty()) {
    vm.color = vm.colors[0];
  } else {
    vm.color = "Mặc định";
  }

  std::vector<std::string> knownCaps = {"1TB", "512GB", "256GB", "128GB",
                                        "64GB"};
  for (const auto &cap : knownCaps) {
    if (modelPart.find(cap) != std::string::npos ||
        vm.specs.find(cap) != std::string::npos) {
      vm.capacity = cap;
      break;
    }
  }
  if (vm.capacity.empty())
    vm.capacity = "256GB";

  std::string base = modelPart;
  size_t capPos = base.find(vm.capacity);
  if (capPos != std::string::npos) {
    base.erase(capPos, vm.capacity.length());
  }
  vm.baseModel = StringHelper::trim(base);
  if (vm.baseModel.empty())
    vm.baseModel = modelPart;

  if (vm.imageUrl.empty()) {
    if (vm.baseModel.find("iPhone 15") != std::string::npos) {
      vm.imageUrl = "Device/productImages/iphone_15_pro_max_256gb.png";
    } else if (vm.baseModel.find("Oppo Reno") != std::string::npos) {
      vm.imageUrl = "Device/productImages/oppo_reno_16_f_5g_bac.png";
    }
  }

  vm.calculateStockStatus();
  return vm;
}

inline CustomerItemViewModel customerToViewModel(const Customer &c) {
  CustomerItemViewModel vm;
  vm.id = c.getID();
  vm.username = c.getAccount().getUsername();
  vm.firstName = c.getName().firstName;
  vm.middleName = c.getName().middleName;
  vm.lastName = c.getName().lastName;
  vm.fullName = c.getName().getFullName();
  vm.phone = c.getPhoneNumber();
  vm.email = c.getEmail();
  vm.address = c.getAddress();
  vm.points = c.getPoints();
  vm.updateTier();
  return vm;
}

inline EmployeeItemViewModel employeeToViewModel(const Employee *e) {
  EmployeeItemViewModel vm;
  if (!e)
    return vm;
  vm.id = e->getID();
  vm.firstName = e->getName().firstName;
  vm.middleName = e->getName().middleName;
  vm.lastName = e->getName().lastName;
  vm.fullName = e->getName().getFullName();
  vm.phone = e->getPhoneNumber();
  vm.email = e->getEmail();
  vm.address = e->getAddress();
  vm.username = e->getAccount().getUsername();
  vm.role = e->getRoleType();
  vm.roleName = roleTypeToString(vm.role);
  vm.salary = e->getSalary();
  vm.isActive = e->getAccount().isActive();
  return vm;
}

inline AccountItemViewModel customerToAccountViewModel(const Customer &c) {
  AccountItemViewModel vm;
  vm.id = c.getAccount().getID().empty() ? ("ACC-" + c.getID())
                                         : c.getAccount().getID();
  vm.username = c.getAccount().getUsername();
  vm.role = RoleType::CUSTOMER;
  vm.roleName = "Khách hàng";
  vm.isActive = c.getAccount().isActive();
  vm.linkedEntityId = c.getID();
  vm.linkedFullName = c.getName().getFullName();
  return vm;
}

inline AccountItemViewModel employeeToAccountViewModel(const Employee *e) {
  AccountItemViewModel vm;
  if (!e)
    return vm;
  vm.id = e->getAccount().getID().empty() ? ("ACC-" + e->getID())
                                          : e->getAccount().getID();
  vm.username = e->getAccount().getUsername();
  vm.role = e->getRoleType();
  vm.roleName = roleTypeToString(vm.role);
  vm.isActive = e->getAccount().isActive();
  vm.linkedEntityId = e->getID();
  vm.linkedFullName = e->getName().getFullName();
  return vm;
}

inline SupplierItemViewModel supplierToViewModel(const Supplier &s) {
  SupplierItemViewModel vm;
  vm.id = s.getID();
  vm.companyName = s.getCompanyName();
  vm.taxCode = s.getTaxCode();
  vm.contactPerson = s.getFullName();
  vm.phone = s.getPhoneNumber();
  vm.email = s.getEmail();
  vm.address = s.getAddress();
  return vm;
}

inline DiscountItemViewModel discountToViewModel(const DiscountCode &d) {
  DiscountItemViewModel vm;
  vm.id = d.getCode();
  vm.code = d.getCode();
  vm.typeName = (d.getType() == DiscountType::PERCENTAGE) ? "Giảm phần trăm (%)"
                                                          : "Số tiền cố định";
  vm.value = d.getValue();
  if (d.getType() == DiscountType::PERCENTAGE) {
    vm.formattedValue = std::to_string(static_cast<int>(d.getValue())) + "%";
  } else {
    vm.formattedValue = StringHelper::formatCurrency(d.getValue()) + " đ";
  }
  vm.minOrderValue = d.getMinOrderValue();
  vm.maxDiscountAmount = d.getMaxDiscountValue();
  vm.expiryDate = d.getEndDate().toDateString();
  vm.usedCount = d.getUsedCount();
  vm.maxUsage = d.getMaxUsage();
  vm.isValid = d.isValid();
  std::string conditionText =
      (d.getMinOrderValue() <= 0)
          ? "Áp dụng cho mọi đơn hàng"
          : ("Đơn từ " + StringHelper::formatCurrency(d.getMinOrderValue()) +
             " đ");
  if (d.getType() == DiscountType::PERCENTAGE) {
    vm.description = "Giảm " + std::to_string(static_cast<int>(d.getValue())) +
                     "% (" + conditionText + ")";
  } else {
    vm.description = "Giảm " + StringHelper::formatCurrency(d.getValue()) +
                     " đ (" + conditionText + ")";
  }
  return vm;
}

inline OrderItemViewModel
orderToViewModel(const Order &o,
                 const std::vector<CustomerItemViewModel> &customers) {
  OrderItemViewModel vm;
  vm.id = o.getID();
  vm.customerId = o.getCustomerId();
  vm.customerName = o.getShippingInfo().getReceiverName();
  if (vm.customerName.empty()) {
    for (const auto &c : customers) {
      if (c.id == vm.customerId) {
        vm.customerName = c.fullName;
        break;
      }
    }
  }
  vm.orderTime = o.getOrderTime().toString();
  vm.status = o.getStatus();
  vm.statusText = orderStatusToString(o.getStatus());
  vm.deliveryMethod = o.getDeliveryMethod();
  vm.deliveryMethodText = deliveryMethodToString(o.getDeliveryMethod());
  vm.paymentMethod = o.getPaymentMethod();
  vm.paymentMethodText = paymentMethodToString(o.getPaymentMethod());
  vm.paymentStatus = (o.getStatus() == OrderStatus::COMPLETED ||
                      o.getPaymentMethod() == PaymentMethod::BANK_TRANSFER)
                         ? PaymentStatus::PAID
                         : PaymentStatus::UNPAID;
  vm.paymentStatusText = paymentStatusToString(vm.paymentStatus);
  vm.subtotal = o.calculateSubtotal();
  vm.discountAmount = o.getDiscountAmount();
  vm.total = o.calculateTotal();
  vm.receiverName = o.getShippingInfo().getReceiverName();
  vm.receiverPhone = o.getShippingInfo().getPhoneNumber();
  vm.deliveryAddress = o.getShippingInfo().getAddress();

  const Vector<OrderDetail> &details = o.getDetails();
  for (int i = 0; i < details.getSize(); ++i) {
    OrderDetailItemViewModel dvm;
    dvm.phoneId = details[i].getPhoneId();
    dvm.phoneName = details[i].getPhoneName();
    dvm.quantity = details[i].getQuantity();
    dvm.unitPrice = details[i].getUnitPrice();
    dvm.subtotal = details[i].getSubtotal();
    vm.details.push_back(dvm);
  }
  return vm;
}

inline InvoiceItemViewModel
invoiceToViewModel(const Invoice &inv,
                   const std::vector<CustomerItemViewModel> &customers,
                   const std::vector<EmployeeItemViewModel> &employees) {
  InvoiceItemViewModel vm;
  vm.id = inv.getID();
  vm.orderId = inv.getOrderId();
  vm.customerId = inv.getCustomerId();
  for (const auto &c : customers) {
    if (c.id == vm.customerId) {
      vm.customerName = c.fullName;
      break;
    }
  }
  if (vm.customerName.empty())
    vm.customerName = "Khách hàng " + vm.customerId;
  vm.employeeId = inv.getEmployeeId();
  for (const auto &e : employees) {
    if (e.id == vm.employeeId) {
      vm.employeeName = e.fullName;
      break;
    }
  }
  if (vm.employeeName.empty())
    vm.employeeName = "Nhân viên " + vm.employeeId;
  vm.paymentMethod = inv.getPaymentMethod();
  vm.paymentMethodText = paymentMethodToString(inv.getPaymentMethod());
  vm.createTime = inv.getCreateTime().toString();
  vm.subtotal = inv.calculateSubtotal();
  vm.discountAmount = inv.getDiscountAmount();
  vm.total = inv.calculateTotal();

  const Vector<OrderDetail> &details = inv.getDetails();
  for (int i = 0; i < details.getSize(); ++i) {
    OrderDetailItemViewModel dvm;
    dvm.phoneId = details[i].getPhoneId();
    dvm.phoneName = details[i].getPhoneName();
    dvm.quantity = details[i].getQuantity();
    dvm.unitPrice = details[i].getUnitPrice();
    dvm.subtotal = details[i].getSubtotal();
    vm.details.push_back(dvm);
  }
  return vm;
}

inline GoodsReceiptSummaryViewModel
receiptToViewModel(const ImportReceipt &r,
                   const std::vector<SupplierItemViewModel> &suppliers,
                   const std::vector<EmployeeItemViewModel> &employees) {
  GoodsReceiptSummaryViewModel vm;
  vm.id = r.getID();
  vm.supplierId = r.getSupplierId();
  for (const auto &s : suppliers) {
    if (s.id == vm.supplierId) {
      vm.supplierName = s.companyName;
      break;
    }
  }
  if (vm.supplierName.empty())
    vm.supplierName = "NCC " + vm.supplierId;
  vm.employeeId = r.getEmployeeId();
  for (const auto &e : employees) {
    if (e.id == vm.employeeId) {
      vm.employeeName = e.fullName;
      break;
    }
  }
  if (vm.employeeName.empty())
    vm.employeeName = "Nhân viên kho " + vm.employeeId;
  vm.importTime = r.getImportTime().toString();
  vm.totalAmount = r.calculateTotal();

  const Vector<OrderDetail> &details = r.getDetails();
  for (int i = 0; i < details.getSize(); ++i) {
    GoodsReceiptDetailViewModel dvm;
    dvm.phoneId = details[i].getPhoneId();
    dvm.phoneName = details[i].getPhoneName();
    dvm.quantity = details[i].getQuantity();
    dvm.importPrice = details[i].getUnitPrice();
    dvm.subtotal = details[i].getSubtotal();
    vm.details.push_back(dvm);
  }
  return vm;
}

inline WarrantyItemViewModel
warrantyToViewModel(const WarrantyRequest &req,
                    CustomerService *customerService,
                    WarrantyService *warrantyService, const DateTime &now) {
  WarrantyItemViewModel vm;
  vm.id = req.getId();
  vm.orderId = req.getOrderId();
  vm.phoneId = req.getPhoneId();
  vm.phoneName = req.getPhoneName();
  vm.customerId = req.getCustomerId();

  if (customerService != nullptr && !req.getCustomerId().empty()) {
    Customer *cust = customerService->getCustomerById(req.getCustomerId());
    if (cust != nullptr) {
      vm.customerName = cust->getName().getFullName();
      vm.customerPhone = cust->getPhoneNumber();
    }
  }
  if (vm.customerName.empty()) {
    vm.customerName =
        req.getCustomerId().empty() ? "Khách vãng lai" : req.getCustomerId();
  }

  vm.requestDate = formatDateTimeMinute(req.getCreatedAt());
  vm.status = req.getStatus();
  vm.statusText = warrantyStatusToString(req.getStatus());
  vm.issueDescription = req.getIssueDescription();
  vm.technicianNote = req.getTechnicianNote();
  vm.resolution = req.getResolution();
  vm.createdAt = req.getCreatedAt().toString();
  vm.updatedAt = req.getUpdatedAt().toString();

  if (warrantyService != nullptr) {
    WarrantyService::WarrantyCoverage cov;
    std::string covErr;
    if (warrantyService->getCoverage(req.getOrderId(), req.getPhoneId(), now,
                                     cov, covErr)) {
      vm.invoiceId = cov.invoiceId;
      vm.expireDate = cov.expireDate.toDateString();
      vm.remainingDays = calculateCivilDayDiff(now, cov.expireDate);
      if (vm.phoneName.empty() && !cov.phoneName.empty()) {
        vm.phoneName = cov.phoneName;
      }
    }
  }

  return vm;
}

} // namespace AppControllerHelpers

#endif // APP_CONTROLLER_SUPPORT_H
