#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "../../UI/viewmodels/AccountViewModel.h"
#include "../../UI/viewmodels/AuthViewModels.h"
#include "../../UI/viewmodels/CartViewModel.h"
#include "../../UI/viewmodels/CustomerViewModel.h"
#include "../../UI/viewmodels/DiscountViewModel.h"
#include "../../UI/viewmodels/EmployeeViewModel.h"
#include "../../UI/viewmodels/GoodsReceiptViewModel.h"
#include "../../UI/viewmodels/InvoiceViewModel.h"
#include "../../UI/viewmodels/OperationResult.h"
#include "../../UI/viewmodels/OrderViewModel.h"
#include "../../UI/viewmodels/ProductViewModel.h"
#include "../../UI/viewmodels/ReviewViewModel.h"
#include "../../UI/viewmodels/StatisticsViewModel.h"
#include "../../UI/viewmodels/SupplierViewModel.h"
#include "../../UI/viewmodels/WarrantyViewModel.h"
#include "../repositories/ProductDescriptionRepository.h"
#include "../repositories/ProductMarketingRepository.h"
#include "../repositories/StoreSettingsRepository.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>


// Forward declarations of Core Services
class PhoneService;
class OrderService;
class CustomerService;
class EmployeeService;
class InvoiceService;
class SupplierService;
class DiscountService;
class ReportService;
class AuthService;
class WarrantyService;

class AppController {
private:
  PhoneService *phoneService;
  OrderService *orderService;
  CustomerService *customerService;
  EmployeeService *employeeService;
  InvoiceService *invoiceService;
  SupplierService *supplierService;
  DiscountService *discountService;
  ReportService *reportService;
  AuthService *authService;
  WarrantyService *warrantyService;

  UserSessionViewModel currentSession;

  ProductMarketingRepository marketingRepo;
  ProductDescriptionRepository descriptionRepo;
  StoreSettingsRepository storeSettingsRepo;
  std::map<std::string, CartSummaryViewModel> userCarts;

  std::unordered_map<std::string, int> computeReservedStockMap() const;
  int computeReservedStockForPhone(const std::string &phoneId) const;

public:
  StoreSettingsRepository &getStoreSettings() { return storeSettingsRepo; }
  const StoreSettingsRepository &getStoreSettings() const {
    return storeSettingsRepo;
  }
  ProductMarketingRepository &getMarketingRepo() { return marketingRepo; }
  ProductDescriptionRepository &getDescriptionRepo() { return descriptionRepo; }
  AppController(PhoneService *ps = nullptr, OrderService *os = nullptr,
                CustomerService *cs = nullptr, EmployeeService *es = nullptr,
                InvoiceService *is = nullptr, SupplierService *ss = nullptr,
                DiscountService *ds = nullptr, ReportService *rs = nullptr,
                AuthService *as = nullptr, WarrantyService *ws = nullptr);

  // === XÁC THỰC & PHIÊN LÀM VIỆC ===
  LoginResponse login(const LoginRequest &request);
  OperationResult registerCustomer(const std::string &username,
                                   const std::string &password,
                                   const std::string &fullName,
                                   const std::string &phone,
                                   const std::string &email);
  OperationResult resetPasswordSimulation(const std::string &username,
                                          const std::string &newPassword,
                                          const std::string &verifyContact);
  OperationResult changePassword(const std::string &oldPassword,
                                 const std::string &newPassword);
  void logout();
  const UserSessionViewModel &getCurrentUser() const;
  OperationResult updateProfile(const std::string &fullName,
                                const std::string &phone,
                                const std::string &email,
                                const std::string &address);

  // === HỆ THỐNG KIỂM SOÁT THỜI GIAN THỰC (REAL-TIME SIMULATION) ===
  std::string getSimulatedTime();
  std::string getSimulatedDate();
  OperationResult setSimulatedTime(const std::string &newTimeStr);
  OperationResult advanceSimulatedDays(int days);
  void advanceSimulatedSeconds(int seconds = 1);
  OperationResult resetSimulatedTimeToNow();

  // === SẢN PHẨM ===
  std::vector<ProductItemViewModel>
  getProducts(const ProductFilterRequest &filter = ProductFilterRequest());
  ProductItemViewModel getProductById(const std::string &id);
  std::vector<ProductItemViewModel> getProductVariants(const std::string &phoneIdOrBaseModel);
  OperationResult addProduct(const ProductItemViewModel &product);
  OperationResult updateProduct(const ProductItemViewModel &product);
  OperationResult deleteProduct(const std::string &id);
  OperationResult updateProductImage(const std::string &productId,
                                     const std::string &imageFilePath);
  std::vector<ProductReviewViewModel>
  getProductReviews(const std::string &phoneId);
  bool canCustomerReviewProduct(const std::string &customerId, const std::string &phoneId);
  OperationResult addProductReview(const std::string &phoneId, int ratingStars,
                                   const std::string &comment);

  // === GIỎ HÀNG & CHECKOUT ===
  CartSummaryViewModel getCart(const std::string &customerId);
  OperationResult addToCart(const std::string &customerId,
                            const std::string &phoneId, int quantity = 1);
  OperationResult updateCartQuantity(const std::string &customerId,
                                     const std::string &phoneId,
                                     int newQuantity);
  OperationResult removeFromCart(const std::string &customerId,
                                 const std::string &phoneId);
  OperationResult applyCoupon(const std::string &customerId,
                              const std::string &couponCode);
  OperationResult checkout(const CheckoutRequest &request,
                           std::string &outOrderId);

  // === ĐƠN HÀNG ===
  std::vector<OrderItemViewModel> getAllOrders();
  std::vector<OrderItemViewModel> filterOrders(const std::string &keyword,
                                               int statusFilter = -1,
                                               int paymentFilter = -1,
                                               int typeFilter = -1);
  std::vector<OrderItemViewModel>
  getCustomerOrders(const std::string &customerId);
  OrderItemViewModel getOrderById(const std::string &orderId);
  OperationResult updateOrderStatus(const std::string &orderId,
                                    OrderStatus newStatus);
  OperationResult cancelOrder(const std::string &orderId);
  void autoCompleteDeliveredOrders();

  // === HÓA ĐƠN ===
  std::vector<InvoiceItemViewModel> getAllInvoices();
  InvoiceItemViewModel getInvoiceById(const std::string &invoiceId);
  OperationResult createInvoiceForOrder(const std::string &orderId,
                                        const std::string &employeeId,
                                        PaymentMethod method);
  OperationResult createPosInvoice(const std::string &customerId,
                                   const std::string &employeeId,
                                   const std::vector<CartItemViewModel> &items,
                                   const std::string &discountCode,
                                   PaymentMethod payMethod, double cashGiven,
                                   double &outChange,
                                   std::string &outInvoiceId);

  // === KHÁCH HÀNG ===
  std::vector<CustomerItemViewModel>
  getCustomers(const std::string &keyword = "");
  CustomerItemViewModel getCustomerById(const std::string &id);
  OperationResult addCustomer(const CustomerFormRequest &req);
  OperationResult updateCustomer(const CustomerFormRequest &req);
  OperationResult deleteCustomer(const std::string &id);

  // === NHÂN VIÊN ===
  std::vector<EmployeeItemViewModel>
  getEmployees(const std::string &keyword = "");
  EmployeeItemViewModel getEmployeeById(const std::string &id);
  OperationResult addEmployee(const EmployeeFormRequest &req);
  OperationResult updateEmployee(const EmployeeFormRequest &req);
  OperationResult deleteEmployee(const std::string &id);

  // === KHO & NHÀ CUNG CẤP ===
  std::vector<ProductItemViewModel> getInventory(bool lowStockOnly = false);
  std::vector<ProductItemViewModel>
  getProductsBySupplier(const std::string &supplierId);
  std::vector<SupplierItemViewModel>
  getSuppliers(const std::string &keyword = "");
  OperationResult addSupplier(const SupplierFormRequest &req);
  OperationResult updateSupplier(const SupplierFormRequest &req);
  OperationResult deleteSupplier(const std::string &id);
  std::vector<GoodsReceiptSummaryViewModel> getGoodsReceipts();
  OperationResult createGoodsReceipt(const CreateReceiptRequest &req);

  // === CẤU HÌNH THUẾ VAT & GIẢM GIÁ ===
  double getVatRate() const;
  OperationResult setVatRate(double newRate);

  // === MÃ GIẢM GIÁ ===
  std::vector<DiscountItemViewModel> getDiscounts();
  OperationResult addDiscount(const DiscountFormRequest &req);
  OperationResult updateDiscount(const DiscountFormRequest &req);
  OperationResult deleteDiscount(const std::string &id);
  OperationResult validateDiscount(const std::string &code, double orderTotal,
                                   double &outDiscount);
  OperationResult validateDiscountForCustomer(const std::string &code,
                                              double orderTotal,
                                              const std::string &customerId,
                                              double &outDiscount);

  // === TÀI KHOẢN & PHÂN QUYỀN ===
  std::vector<AccountItemViewModel> getAccounts();
  OperationResult toggleAccountStatus(const std::string &username);
  OperationResult changeAccountRole(const std::string &username,
                                    RoleType newRole);

  // === BẢO HÀNH ===
  std::vector<WarrantyItemViewModel> getWarranties();
  std::vector<WarrantyItemViewModel>
  getCustomerWarranties(const std::string &customerId);
  std::vector<WarrantyEligibleItemViewModel>
  getEligibleWarrantyItems(const std::string &customerId);
  OperationResult createWarranty(const CreateWarrantyRequest &req);
  OperationResult updateWarrantyStatus(const std::string &id,
                                       WarrantyStatus status,
                                         const std::string& technicianNote = "", const std::string& resolution = "");

  // === THỐNG KÊ & BÁO CÁO ===
  // fromDate/toDate: DD/MM/YYYY, bao gồm cả hai đầu. Để trống = toàn thời gian.
  DashboardStatsViewModel getDashboardStats(const std::string &fromDate = "",
                                            const std::string &toDate = "",
                                            int timelineMode = 0);
  OperationResult validateReportRange(const std::string &fromDate,
                                      const std::string &toDate);

  // === XUẤT DỮ LIỆU (.CSV) & HÓA ĐƠN / CHỨNG TỪ (.TXT) ===
  OperationResult exportCustomersCsv(const std::string &filePath,
                                     const std::string &keyword = "");
  OperationResult exportEmployeesCsv(const std::string &filePath,
                                     const std::string &keyword = "");
  OperationResult exportProductsCsv(
      const std::string &filePath,
      const ProductFilterRequest &filter = ProductFilterRequest());
  OperationResult exportSuppliersCsv(const std::string &filePath,
                                     const std::string &keyword = "");
  OperationResult exportDiscountsCsv(const std::string &filePath);
  OperationResult exportOrdersCsv(const std::string &filePath,
                                  const std::string &keyword = "",
                                  int statusFilter = -1,
                                  int paymentFilter = -1,
                                  const std::string &fromDate = "",
                                  const std::string &toDate = "");
  OperationResult exportInvoicesCsv(const std::string &filePath,
                                    const std::string &keyword = "",
                                    const std::string &fromDate = "",
                                    const std::string &toDate = "");
  OperationResult exportGoodsReceiptsCsv(const std::string &filePath,
                                         const std::string &keyword = "",
                                         const std::string &fromDate = "",
                                         const std::string &toDate = "");
  OperationResult exportWarrantiesCsv(const std::string &filePath,
                                      const std::string &keyword = "",
                                      int statusFilter = -1);
  OperationResult exportAccountsCsv(const std::string &filePath);
  OperationResult exportRevenueReportCsv(const std::string &filePath,
                                         const std::string &fromDate = "",
                                         const std::string &toDate = "",
                                         int timelineMode = 0);

  OperationResult exportInvoiceTxt(const std::string &filePath,
                                   const std::string &invoiceId);
  OperationResult exportOrderTxt(const std::string &filePath,
                                 const std::string &orderId);
  OperationResult exportGoodsReceiptTxt(const std::string &filePath,
                                        const std::string &receiptId);
  OperationResult exportWarrantyTxt(const std::string &filePath,
                                    const std::string &warrantyId);
};

#endif // APP_CONTROLLER_H
