#ifndef MOCK_DATA_STORE_H
#define MOCK_DATA_STORE_H

#include "../viewmodels/AccountViewModel.h"
#include "../viewmodels/CartViewModel.h"
#include "../viewmodels/CustomerViewModel.h"
#include "../viewmodels/DiscountViewModel.h"
#include "../viewmodels/EmployeeViewModel.h"
#include "../viewmodels/GoodsReceiptViewModel.h"
#include "../viewmodels/InvoiceViewModel.h"
#include "../viewmodels/OrderViewModel.h"
#include "../viewmodels/ProductViewModel.h"
#include "../viewmodels/StatisticsViewModel.h"
#include "../viewmodels/SupplierViewModel.h"
#include "../viewmodels/WarrantyViewModel.h"
#include <algorithm>
#include <string>
#include <vector>


#include "../../source/utils/StringHelper.h"
#include "RealTimeManager.h"
#include <map>


/**
 * @file MockDataStore.h
 * @brief ĐẶC TẢ KHO DỮ LIỆU MẪU ĐẦY ĐỦ CHO TẦNG UI VIEWMODELS (UI DATA STORE &
 * BUSINESS SEED DATA)
 *
 * =========================================================================================
 * 1. MỤC ĐÍCH & VAI TRÒ:
 *    - Cung cấp nguồn dữ liệu mẫu độc lập đầy đủ (Products, Customers,
 * Employees, Orders, Invoices, GoodsReceipts, Suppliers, Discounts, Warranties,
 * Statistics).
 *    - Kết nối với RealTimeManager để đồng bộ trạng thái voucher hết hạn, đơn
 * hàng theo thời gian thực.
 *    - Cho phép chạy demo nghiệm thu luồng UI đồ án hoàn chỉnh mà không bị phụ
 * thuộc vào tầng file I/O chưa wire.
 * =========================================================================================
 */
class MockDataStore {
public:
  std::vector<ProductItemViewModel> products;
  std::vector<CustomerItemViewModel> customers;
  std::vector<EmployeeItemViewModel> employees;
  std::vector<SupplierItemViewModel> suppliers;
  std::vector<DiscountItemViewModel> discounts;
  std::vector<OrderItemViewModel> orders;
  std::vector<InvoiceItemViewModel> invoices;
  std::vector<GoodsReceiptSummaryViewModel> receipts;
  std::vector<WarrantyItemViewModel> warranties;
  std::vector<AccountItemViewModel> accounts;
  CartSummaryViewModel customerCart;
  std::map<std::string, CartSummaryViewModel> userCarts;
  std::map<std::string, std::string> customPasswords;

  static MockDataStore &getInstance() {
    static MockDataStore instance;
    return instance;
  }

  bool registerCustomer(const std::string &username,
                        const std::string &password,
                        const std::string &fullName, const std::string &phone,
                        const std::string &email) {
    for (const auto &c : customers) {
      if (c.username == username)
        return false;
    }
    for (const auto &e : employees) {
      if (e.username == username)
        return false;
    }
    CustomerItemViewModel newC;
    newC.id = "CUS-00000" + std::to_string(customers.size() + 1);
    newC.fullName = fullName;
    newC.username = username;
    newC.phone = phone;
    newC.email = email;
    newC.points = 0;
    newC.membershipTier = "Đồng";
    customers.push_back(newC);

    AccountItemViewModel acc;
    acc.id = newC.id;
    acc.username = username;
    acc.linkedFullName = fullName;
    acc.linkedEntityId = newC.id;
    acc.role = RoleType::CUSTOMER;
    acc.roleName = "Khách hàng";
    acc.isActive = true;
    accounts.push_back(acc);

    customPasswords[username] = password;
    return true;
  }

  bool resetPassword(const std::string &username,
                     const std::string &newPassword) {
    customPasswords[username] = newPassword;
    return true;
  }

  bool hasCustomPassword(const std::string &username) const {
    return customPasswords.find(username) != customPasswords.end();
  }

  bool verifyPassword(const std::string &username,
                      const std::string &password) {
    auto it = customPasswords.find(username);
    if (it != customPasswords.end()) {
      return it->second == password;
    }
    return false;
  }

  bool updateDiscount(const DiscountFormRequest &req) {
    for (auto &d : discounts) {
      if (d.id == req.id || d.code == req.id || d.code == req.code) {
        d.id = req.code;
        d.code = req.code;
        d.value = req.value;
        d.minOrderValue = req.minOrderValue;
        d.maxDiscountAmount =
            (req.type == 0) ? req.maxDiscountAmount : req.value;
        d.expiryDate = req.expiryDate;
        d.maxUsage = req.maxUsage;
        d.description = req.description;
        if (req.type == 0) {
          d.typeName = "Giảm phần trăm (%)";
          d.formattedValue = std::to_string(static_cast<int>(req.value)) + "%";
        } else {
          d.typeName = "Số tiền cố định";
          d.formattedValue = StringHelper::formatCurrency(req.value) + " đ";
        }
        d.isValid = !RealTimeManager::getInstance().isDateExpired(d.expiryDate);
        return true;
      }
    }
    return false;
  }

  void resetToDefault() {
    initProducts();
    initCustomers();
    initEmployees();
    initSuppliers();
    initDiscounts();
    initOrders();
    initInvoices();
    initReceipts();
    initWarranties();
    initAccounts();
    initCart();
  }

private:
  MockDataStore() {
    // KHÔNG gọi resetToDefault() tự động để tránh ghi đè dữ liệu tệp tin của
    // người dùng. Dữ liệu sẽ được AppController nạp từ tầng Repository/Service
    // theo đúng kiến trúc.
    initWarranties();
    initAccounts();
    initCart();
  }

  void initProducts() {
    products.clear();

    ProductItemViewModel p1;
    p1.id = "PHN-000101";
    p1.name = "Oppo Reno 16F 5G";
    p1.brand = "Oppo";
    p1.os = "Android - ColorOS 16";
    p1.importPrice = 10600000;
    p1.sellingPrice = 14000000;
    p1.actualStock = 42;
    p1.reservedStock = 2;
    p1.manufacturer = "Oppo Electronics";
    p1.specs = "6.57 inch AMOLED, 8GB RAM, 256GB ROM, Dimensity 7300-Energy, "
               "Pin 7000mAh, Sạc 45W";
    p1.imageUrl = "Device/productImages/oppo_reno_16_f_5g_bac.png";
    p1.calculateStockStatus();
    products.push_back(p1);

    ProductItemViewModel p2;
    p2.id = "PHN-000102";
    p2.name = "iPhone 15 Pro Max 256GB";
    p2.brand = "Apple";
    p2.os = "iOS 17";
    p2.importPrice = 27500000;
    p2.sellingPrice = 31990000;
    p2.actualStock = 18;
    p2.reservedStock = 3;
    p2.manufacturer = "Apple Inc.";
    p2.specs = "6.7 inch Super Retina XDR OLED, 8GB RAM, 256GB ROM, Apple A17 "
               "Pro, Khung Titanium";
    p2.calculateStockStatus();
    products.push_back(p2);

    ProductItemViewModel p3;
    p3.id = "PHN-000103";
    p3.name = "Samsung Galaxy S24 Ultra 256GB";
    p3.brand = "Samsung";
    p3.os = "Android 14 - One UI 6.1";
    p3.importPrice = 25000000;
    p3.sellingPrice = 29990000;
    p3.actualStock = 25;
    p3.reservedStock = 1;
    p3.manufacturer = "Samsung Electronics";
    p3.specs = "6.8 inch Dynamic AMOLED 2X 120Hz, 12GB RAM, 256GB ROM, "
               "Snapdragon 8 Gen 3 for Galaxy, Bút S-Pen";
    p3.calculateStockStatus();
    products.push_back(p3);

    ProductItemViewModel p4;
    p4.id = "PHN-000104";
    p4.name = "Xiaomi 14 Ultra 512GB";
    p4.brand = "Xiaomi";
    p4.os = "Android 14 - HyperOS";
    p4.importPrice = 21000000;
    p4.sellingPrice = 25490000;
    p4.actualStock = 15;
    p4.reservedStock = 0;
    p4.manufacturer = "Xiaomi Corporation";
    p4.specs = "6.73 inch LTPO AMOLED, 16GB RAM, 512GB ROM, Camera Leica "
               "1-inch, Snapdragon 8 Gen 3";
    p4.calculateStockStatus();
    products.push_back(p4);

    ProductItemViewModel p5;
    p5.id = "PHN-000105";
    p5.name = "iPhone 13 128GB";
    p5.brand = "Apple";
    p5.os = "iOS 15";
    p5.importPrice = 11200000;
    p5.sellingPrice = 13890000;
    p5.actualStock = 30;
    p5.reservedStock = 5;
    p5.manufacturer = "Apple Inc.";
    p5.specs = "6.1 inch OLED Super Retina XDR, 4GB RAM, 128GB ROM, Apple A15 "
               "Bionic, Pin 3227mAh";
    p5.calculateStockStatus();
    products.push_back(p5);

    ProductItemViewModel p6;
    p6.id = "PHN-000106";
    p6.name = "Samsung Galaxy A55 5G 128GB";
    p6.brand = "Samsung";
    p6.os = "Android 14 - One UI 6.1";
    p6.importPrice = 7500000;
    p6.sellingPrice = 9690000;
    p6.actualStock = 50;
    p6.reservedStock = 4;
    p6.manufacturer = "Samsung Electronics";
    p6.specs = "6.6 inch Super AMOLED 120Hz, 8GB RAM, 128GB ROM, Exynos 1480, "
               "Pin 5000mAh";
    p6.calculateStockStatus();
    products.push_back(p6);

    ProductItemViewModel p7;
    p7.id = "PHN-000107";
    p7.name = "Oppo Find X7 Ultra 256GB";
    p7.brand = "Oppo";
    p7.os = "Android 14 - ColorOS 14";
    p7.importPrice = 18500000;
    p7.sellingPrice = 22900000;
    p7.actualStock = 12;
    p7.reservedStock = 2;
    p7.manufacturer = "Oppo Electronics";
    p7.specs = "6.82 inch LTPO AMOLED, 16GB RAM, 256GB ROM, 2 ống kính tiềm "
               "vọng Hasselblad";
    p7.calculateStockStatus();
    products.push_back(p7);

    // Thêm 1 sản phẩm sắp hết hàng để test Low Stock Badge (<= 5)
    ProductItemViewModel p8;
    p8.id = "PHN-000108";
    p8.name = "iPhone 16 Pro 128GB (Hot)";
    p8.brand = "Apple";
    p8.os = "iOS 18";
    p8.importPrice = 24500000;
    p8.sellingPrice = 28990000;
    p8.actualStock = 4;
    p8.reservedStock = 1;
    p8.manufacturer = "Apple Inc.";
    p8.specs = "6.3 inch Super Retina XDR, 8GB RAM, 128GB ROM, Apple A18 Pro, "
               "Nút Camera Control";
    p8.calculateStockStatus();
    products.push_back(p8);
  }

  void initCustomers() {
    customers.clear();

    CustomerItemViewModel c1;
    c1.id = "CUS-000001";
    c1.firstName = "Nguyen";
    c1.middleName = "Van";
    c1.lastName = "B";
    c1.fullName = "Nguyễn Văn B";
    c1.phone = "0912345678";
    c1.email = "customer001@example.com";
    c1.username = "customer001.demo";
    c1.points = 150;
    c1.address = "Hải Châu, Đà Nẵng";
    c1.updateTier();
    customers.push_back(c1);

    CustomerItemViewModel c2;
    c2.id = "CUS-000002";
    c2.firstName = "Tran";
    c2.middleName = "Thi";
    c2.lastName = "Mai";
    c2.fullName = "Trần Thị Mai";
    c2.phone = "0905111222";
    c2.email = "customer002@example.com";
    c2.username = "customer002.demo";
    c2.points = 80;
    c2.address = "Thanh Khê, Đà Nẵng";
    c2.updateTier();
    customers.push_back(c2);

    CustomerItemViewModel c3;
    c3.id = "CUS-000003";
    c3.firstName = "Le";
    c3.middleName = "Hoang";
    c3.lastName = "Nam";
    c3.fullName = "Lê Hoàng Nam";
    c3.phone = "0988777666";
    c3.email = "customer003@example.com";
    c3.username = "customer003.demo";
    c3.points = 10;
    c3.address = "Sơn Trà, Đà Nẵng";
    c3.updateTier();
    customers.push_back(c3);
  }

  void initEmployees() {
    employees.clear();

    EmployeeItemViewModel e1;
    e1.id = "EMP-000001";
    e1.firstName = "Admin";
    e1.middleName = "He";
    e1.lastName = "Thong";
    e1.fullName = "Admin Hệ Thống";
    e1.phone = "0900000001";
    e1.email = "admin@example.com";
    e1.username = "admin.demo";
    e1.role = RoleType::ADMIN;
    e1.roleName = "Quản trị viên (Admin)";
    e1.address = "Hải Châu, Đà Nẵng";
    e1.salary = 15000000;
    employees.push_back(e1);

    EmployeeItemViewModel e2;
    e2.id = "EMP-000002";
    e2.firstName = "Tran";
    e2.middleName = "Khanh";
    e2.lastName = "Duy";
    e2.fullName = "Trần Khánh Duy";
    e2.phone = "0987654321";
    e2.email = "sales@example.com";
    e2.username = "sales.demo";
    e2.role = RoleType::SALE_EMPLOYEE;
    e2.roleName = "Nhân viên Bán hàng";
    e2.address = "Hòa Khánh, Đà Nẵng";
    e2.salary = 8000000;
    employees.push_back(e2);

    EmployeeItemViewModel e3;
    e3.id = "EMP-000003";
    e3.firstName = "Le";
    e3.middleName = "Van";
    e3.lastName = "Kho";
    e3.fullName = "Lê Văn Kho";
    e3.phone = "0935123456";
    e3.email = "warehouse@example.com";
    e3.username = "warehouse.demo";
    e3.role = RoleType::WAREHOUSE_EMPLOYEE;
    e3.roleName = "Nhân viên Kho";
    e3.address = "Liên Chiểu, Đà Nẵng";
    e3.salary = 7500000;
    employees.push_back(e3);
  }

  void initSuppliers() {
    suppliers.clear();

    SupplierItemViewModel s1;
    s1.id = "SUP-000001";
    s1.companyName = "Apple Vietnam (DEMO)";
    s1.taxCode = "0312345678";
    s1.contactPerson = "Nguyễn Văn Đại";
    s1.phone = "02838222333";
    s1.email = "apple.supplier@example.com";
    s1.address = "Quận 1, TP Hồ Chí Minh";
    suppliers.push_back(s1);

    SupplierItemViewModel s2;
    s2.id = "SUP-000002";
    s2.companyName = "Samsung Vina (DEMO)";
    s2.taxCode = "0387654321";
    s2.contactPerson = "Park Jin Ho";
    s2.phone = "02838111222";
    s2.email = "samsung.supplier@example.com";
    s2.address = "TP Thủ Đức, TP Hồ Chí Minh";
    suppliers.push_back(s2);

    SupplierItemViewModel s3;
    s3.id = "SUP-000003";
    s3.companyName = "Oppo Vietnam (DEMO)";
    s3.taxCode = "0399887766";
    s3.contactPerson = "Trần Tuấn Vũ";
    s3.phone = "02839998888";
    s3.email = "oppo.supplier@example.com";
    s3.address = "Quận 7, TP Hồ Chí Minh";
    suppliers.push_back(s3);

    SupplierItemViewModel s4;
    s4.id = "SUP-000004";
    s4.companyName = "Xiaomi Vietnam (DEMO)";
    s4.taxCode = "0311223344";
    s4.contactPerson = "Phạm Minh An";
    s4.phone = "02837776655";
    s4.email = "xiaomi.supplier@example.com";
    s4.address = "Quận 3, TP Hồ Chí Minh";
    suppliers.push_back(s4);
  }

  void initDiscounts() {
    discounts.clear();

    DiscountItemViewModel d1;
    d1.id = "SALE10";
    d1.code = "SALE10";
    d1.typeName = "Giảm phần trăm (%)";
    d1.value = 10.0;
    d1.formattedValue = "10%";
    d1.minOrderValue = 5000000;
    d1.maxDiscountAmount = 500000;
    d1.expiryDate = "31/12/2026";
    d1.usedCount = 12;
    d1.maxUsage = 100;
    d1.isValid = true;
    d1.description = "Giảm 10% tối đa 500.000 đ cho đơn hàng từ 5 triệu";
    discounts.push_back(d1);

    DiscountItemViewModel d2;
    d2.id = "GIAM500K";
    d2.code = "GIAM500K";
    d2.typeName = "Số tiền cố định";
    d2.value = 500000;
    d2.formattedValue = "500.000 đ";
    d2.minOrderValue = 10000000;
    d2.maxDiscountAmount = 500000;
    d2.expiryDate = "30/11/2026";
    d2.usedCount = 35;
    d2.maxUsage = 200;
    d2.isValid = true;
    d2.description = "Giảm trực tiếp 500.000 đ cho đơn hàng từ 10 triệu";
    discounts.push_back(d2);

    DiscountItemViewModel d3;
    d3.id = "VIP2026";
    d3.code = "VIP2026";
    d3.typeName = "Giảm phần trăm (%)";
    d3.value = 15.0;
    d3.formattedValue = "15%";
    d3.minOrderValue = 20000000;
    d3.maxDiscountAmount = 1500000;
    d3.expiryDate = "31/12/2026";
    d3.usedCount = 5;
    d3.maxUsage = 50;
    d3.isValid = true;
    d3.description = "Đặc quyền VIP: Giảm 15% tối đa 1.5 triệu cho Flagship";
    discounts.push_back(d3);

    DiscountItemViewModel d4;
    d4.id = "EXPIRED2025";
    d4.code = "EXPIRED2025";
    d4.typeName = "Số tiền cố định";
    d4.value = 200000;
    d4.formattedValue = "200.000 đ";
    d4.minOrderValue = 2000000;
    d4.maxDiscountAmount = 200000;
    d4.expiryDate = "01/01/2025";
    d4.usedCount = 50;
    d4.maxUsage = 50;
    d4.isValid = false;
    d4.description =
        "Mã khuyến mãi đã hết hạn (Dùng để kiểm thử lỗi validation)";
    discounts.push_back(d4);
  }

  void initOrders() {
    orders.clear();

    // Order from 15/09/2026: COMPLETED, PAID
    OrderItemViewModel o1;
    o1.id = "ORD-15092026-000001";
    o1.customerId = "CUS-000001";
    o1.customerName = "Nguyễn Văn B";
    o1.orderTime = "15/09/2026 10:15";
    o1.status = OrderStatus::COMPLETED;
    o1.statusText = "Hoàn thành";
    o1.deliveryMethod = DeliveryMethod::DELIVERY;
    o1.deliveryMethodText = "Giao tận nơi";
    o1.paymentMethod = PaymentMethod::CASH;
    o1.paymentMethodText = "Tiền mặt";
    o1.paymentStatus = PaymentStatus::PAID;
    o1.paymentStatusText = "Đã thanh toán";
    o1.subtotal = 42000000;
    o1.discountAmount = 0;
    o1.total = 42000000;
    o1.receiverName = "Nguyễn Văn B";
    o1.receiverPhone = "0912345678";
    o1.deliveryAddress = "Hải Châu, Đà Nẵng";
    OrderDetailItemViewModel d1_1;
    d1_1.phoneId = "PHN-000101";
    d1_1.phoneName = "Oppo Reno 16F 5G";
    d1_1.quantity = 3;
    d1_1.unitPrice = 14000000;
    d1_1.subtotal = 42000000;
    o1.details.push_back(d1_1);
    orders.push_back(o1);

    // Order from 16/09/2026: COMPLETED, PAID
    OrderItemViewModel o2;
    o2.id = "ORD-16092026-000001";
    o2.customerId = "CUS-000002";
    o2.customerName = "Trần Thị Mai";
    o2.orderTime = "16/09/2026 14:20";
    o2.status = OrderStatus::COMPLETED;
    o2.statusText = "Hoàn thành";
    o2.deliveryMethod = DeliveryMethod::DELIVERY;
    o2.deliveryMethodText = "Giao tận nơi";
    o2.paymentMethod = PaymentMethod::BANK_TRANSFER;
    o2.paymentMethodText = "Chuyển khoản";
    o2.paymentStatus = PaymentStatus::PAID;
    o2.paymentStatusText = "Đã thanh toán";
    o2.subtotal = 63980000;
    o2.discountAmount = 500000;
    o2.total = 63480000;
    o2.receiverName = "Trần Thị Mai";
    o2.receiverPhone = "0905111222";
    o2.deliveryAddress = "Thanh Khê, Đà Nẵng";
    OrderDetailItemViewModel d2_1;
    d2_1.phoneId = "PHN-000102";
    d2_1.phoneName = "iPhone 15 Pro Max 256GB";
    d2_1.quantity = 2;
    d2_1.unitPrice = 31990000;
    d2_1.subtotal = 63980000;
    o2.details.push_back(d2_1);
    orders.push_back(o2);

    // First order from 17/10/2026: PENDING, UNPAID
    OrderItemViewModel o3;
    o3.id = "ORD-17102026-000001";
    o3.customerId = "CUS-000003";
    o3.customerName = "Lê Hoàng Nam";
    o3.orderTime = "17/10/2026 09:05";
    o3.status = OrderStatus::PENDING;
    o3.statusText = "Chờ xử lý";
    o3.deliveryMethod = DeliveryMethod::PICKUP;
    o3.deliveryMethodText = "Nhận tại cửa hàng";
    o3.paymentMethod = PaymentMethod::CASH;
    o3.paymentMethodText = "Tiền mặt";
    o3.paymentStatus = PaymentStatus::UNPAID;
    o3.paymentStatusText = "Chưa thanh toán";
    o3.subtotal = 13890000;
    o3.discountAmount = 0;
    o3.total = 13890000;
    o3.receiverName = "Lê Hoàng Nam";
    o3.receiverPhone = "0988777666";
    o3.deliveryAddress = "Cửa hàng PBL2 Store - 54 Nguyễn Lương Bằng, Đà Nẵng";
    OrderDetailItemViewModel d3_1;
    d3_1.phoneId = "PHN-000105";
    d3_1.phoneName = "iPhone 13 128GB";
    d3_1.quantity = 1;
    d3_1.unitPrice = 13890000;
    d3_1.subtotal = 13890000;
    o3.details.push_back(d3_1);
    orders.push_back(o3);

    // Second order from 17/10/2026: CONFIRMED, UNPAID
    OrderItemViewModel o4;
    o4.id = "ORD-17102026-000002";
    o4.customerId = "CUS-000001";
    o4.customerName = "Nguyễn Văn B";
    o4.orderTime = "17/10/2026 09:30";
    o4.status = OrderStatus::CONFIRMED;
    o4.statusText = "Đã xác nhận";
    o4.deliveryMethod = DeliveryMethod::DELIVERY;
    o4.deliveryMethodText = "Giao tận nơi";
    o4.paymentMethod = PaymentMethod::BANK_TRANSFER;
    o4.paymentMethodText = "Chuyển khoản";
    o4.paymentStatus = PaymentStatus::PAID;
    o4.paymentStatusText = "Đã thanh toán";
    o4.subtotal = 29990000;
    o4.discountAmount = 500000;
    o4.total = 29490000;
    o4.receiverName = "Nguyễn Văn B";
    o4.receiverPhone = "0912345678";
    o4.deliveryAddress = "Hải Châu, Đà Nẵng";
    OrderDetailItemViewModel d4_1;
    d4_1.phoneId = "PHN-000103";
    d4_1.phoneName = "Samsung Galaxy S24 Ultra 256GB";
    d4_1.quantity = 1;
    d4_1.unitPrice = 29990000;
    d4_1.subtotal = 29990000;
    o4.details.push_back(d4_1);
    orders.push_back(o4);

    // Third order from 17/10/2026: PREPARING
    OrderItemViewModel o5;
    o5.id = "ORD-17102026-000003";
    o5.customerId = "CUS-000002";
    o5.customerName = "Trần Thị Mai";
    o5.orderTime = "17/10/2026 10:00";
    o5.status = OrderStatus::PREPARING;
    o5.statusText = "Đang đóng gói";
    o5.deliveryMethod = DeliveryMethod::PICKUP;
    o5.deliveryMethodText = "Nhận tại cửa hàng";
    o5.paymentMethod = PaymentMethod::CASH;
    o5.paymentMethodText = "Tiền mặt";
    o5.paymentStatus = PaymentStatus::UNPAID;
    o5.paymentStatusText = "Chưa thanh toán";
    o5.subtotal = 25490000;
    o5.discountAmount = 0;
    o5.total = 25490000;
    o5.receiverName = "Trần Thị Mai";
    o5.receiverPhone = "0905111222";
    o5.deliveryAddress = "Cửa hàng PBL2 Store - 54 Nguyễn Lương Bằng, Đà Nẵng";
    OrderDetailItemViewModel d5_1;
    d5_1.phoneId = "PHN-000104";
    d5_1.phoneName = "Xiaomi 14 Ultra 512GB";
    d5_1.quantity = 1;
    d5_1.unitPrice = 25490000;
    d5_1.subtotal = 25490000;
    o5.details.push_back(d5_1);
    orders.push_back(o5);
  }

  void initInvoices() {
    invoices.clear();

    InvoiceItemViewModel inv1;
    inv1.id = "INV-15092026-000001";
    inv1.orderId = "ORD-15092026-000001";
    inv1.customerId = "CUS-000001";
    inv1.customerName = "Nguyễn Văn B";
    inv1.employeeId = "EMP-000002";
    inv1.employeeName = "Trần Khánh Duy";
    inv1.paymentMethod = PaymentMethod::CASH;
    inv1.paymentMethodText = "Tiền mặt";
    inv1.createTime = "15/09/2026 10:20";
    inv1.subtotal = 42000000;
    inv1.discountAmount = 0;
    inv1.total = 42000000;
    OrderDetailItemViewModel d1;
    d1.phoneId = "PHN-000101";
    d1.phoneName = "Oppo Reno 16F 5G";
    d1.quantity = 3;
    d1.unitPrice = 14000000;
    d1.subtotal = 42000000;
    inv1.details.push_back(d1);
    invoices.push_back(inv1);

    InvoiceItemViewModel inv2;
    inv2.id = "INV-16092026-000001";
    inv2.orderId = "ORD-16092026-000001";
    inv2.customerId = "CUS-000002";
    inv2.customerName = "Trần Thị Mai";
    inv2.employeeId = "EMP-000002";
    inv2.employeeName = "Trần Khánh Duy";
    inv2.paymentMethod = PaymentMethod::BANK_TRANSFER;
    inv2.paymentMethodText = "Chuyển khoản";
    inv2.createTime = "16/09/2026 14:25";
    inv2.subtotal = 63980000;
    inv2.discountAmount = 500000;
    inv2.total = 63480000;
    OrderDetailItemViewModel d2;
    d2.phoneId = "PHN-000102";
    d2.phoneName = "iPhone 15 Pro Max 256GB";
    d2.quantity = 2;
    d2.unitPrice = 31990000;
    d2.subtotal = 63980000;
    inv2.details.push_back(d2);
    invoices.push_back(inv2);
  }

  void initReceipts() {
    receipts.clear();

    GoodsReceiptSummaryViewModel r1;
    r1.id = "IMP-10092026-000001";
    r1.supplierId = "SUP-000001";
    r1.supplierName = "Apple Vietnam (DEMO)";
    r1.employeeId = "EMP-000003";
    r1.employeeName = "Lê Văn Kho";
    r1.importTime = "10/09/2026 08:30";
    GoodsReceiptDetailViewModel rd1;
    rd1.phoneId = "PHN-000102";
    rd1.phoneName = "iPhone 15 Pro Max 256GB";
    rd1.quantity = 20;
    rd1.importPrice = 27500000;
    rd1.subtotal = 550000000;
    r1.details.push_back(rd1);
    GoodsReceiptDetailViewModel rd2;
    rd2.phoneId = "PHN-000105";
    rd2.phoneName = "iPhone 13 128GB";
    rd2.quantity = 35;
    rd2.importPrice = 11200000;
    rd2.subtotal = 392000000;
    r1.details.push_back(rd2);
    r1.totalAmount = rd1.subtotal + rd2.subtotal;
    receipts.push_back(r1);

    GoodsReceiptSummaryViewModel r2;
    r2.id = "IMP-12092026-000001";
    r2.supplierId = "SUP-000002";
    r2.supplierName = "Samsung Vina (DEMO)";
    r2.employeeId = "EMP-000003";
    r2.employeeName = "Lê Văn Kho";
    r2.importTime = "12/09/2026 09:15";
    GoodsReceiptDetailViewModel rd3;
    rd3.phoneId = "PHN-000103";
    rd3.phoneName = "Samsung Galaxy S24 Ultra 256GB";
    rd3.quantity = 25;
    rd3.importPrice = 25000000;
    rd3.subtotal = 625000000;
    r2.details.push_back(rd3);
    r2.totalAmount = rd3.subtotal;
    receipts.push_back(r2);
  }

public:
  void initWarranties() {
    warranties.clear();

    WarrantyItemViewModel w1;
    w1.id = "WREQ-18092026-000001";
    w1.orderId = "ORD-15092026-000001";
    w1.phoneId = "PHN-000101";
    w1.phoneName = "Oppo Reno 16F 5G";
    w1.customerId = "CUS-000001";
    w1.customerName = "Nguyễn Văn B";
    w1.customerPhone = "0912345678";
    w1.requestDate = "18/09/2026 15:00";
    w1.issueDescription =
        "Máy bị chập chờn khi cắm sạc nhanh 45W, tự ngắt sau 5 phút";
    w1.status = WarrantyStatus::PROCESSING;
    w1.statusText = warrantyStatusToString(w1.status);
    w1.technicianNote = "Đã nhận máy kiểm tra mạch sạc. Dự kiến thay chân sạc "
                        "Type-C chính hãng.";
    w1.resolution = "Đang xử lý linh kiện";
    warranties.push_back(w1);

    WarrantyItemViewModel w2;
    w2.id = "WREQ-20092026-000001";
    w2.orderId = "ORD-16092026-000001";
    w2.phoneId = "PHN-000102";
    w2.phoneName = "iPhone 15 Pro Max 256GB";
    w2.customerId = "CUS-000002";
    w2.customerName = "Trần Thị Mai";
    w2.customerPhone = "0905111222";
    w2.requestDate = "20/09/2026 11:20";
    w2.issueDescription =
        "Màn hình có 1 đường kẻ sọc xanh nhấp nháy góc trên bên phải";
    w2.status = WarrantyStatus::RESOLVED;
    w2.statusText = warrantyStatusToString(w2.status);
    w2.technicianNote = "Lỗi panel hiển thị do nhà sản xuất. Đã thay màn hình "
                        "OLED chính hãng Apple.";
    w2.resolution = "Đã bàn giao lại máy cho khách ngày 24/09/2026";
    warranties.push_back(w2);
  }

  void initAccounts() {
    accounts.clear();

    AccountItemViewModel a1;
    a1.id = "ACC-000001";
    a1.username = "admin.demo";
    a1.role = RoleType::ADMIN;
    a1.roleName = "Quản trị viên";
    a1.isActive = true;
    a1.linkedEntityId = "EMP-000001";
    a1.linkedFullName = "Admin Hệ Thống";
    accounts.push_back(a1);

    AccountItemViewModel a2;
    a2.id = "ACC-000002";
    a2.username = "sales.demo";
    a2.role = RoleType::SALE_EMPLOYEE;
    a2.roleName = "Nhân viên Bán hàng";
    a2.isActive = true;
    a2.linkedEntityId = "EMP-000002";
    a2.linkedFullName = "Trần Khánh Duy";
    accounts.push_back(a2);

    AccountItemViewModel a3;
    a3.id = "ACC-000003";
    a3.username = "warehouse.demo";
    a3.role = RoleType::WAREHOUSE_EMPLOYEE;
    a3.roleName = "Nhân viên Kho";
    a3.isActive = true;
    a3.linkedEntityId = "EMP-000003";
    a3.linkedFullName = "Lê Văn Kho";
    accounts.push_back(a3);

    AccountItemViewModel a4;
    a4.id = "ACC-000004";
    a4.username = "customer001.demo";
    a4.role = RoleType::CUSTOMER;
    a4.roleName = "Khách hàng";
    a4.isActive = true;
    a4.linkedEntityId = "CUS-000001";
    a4.linkedFullName = "Nguyễn Văn B";
    accounts.push_back(a4);
  }

  void initCart() {
    customerCart = CartSummaryViewModel();
    userCarts.clear();
  }
};

#endif // MOCK_DATA_STORE_H
