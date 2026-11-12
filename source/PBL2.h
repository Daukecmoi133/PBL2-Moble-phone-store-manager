#ifndef PBL2_H
#define PBL2_H

// ================= THƯ VIỆN CHUẨN C++ =================
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>


// ================= CẤU TRÚC DỮ LIỆU TỰ VIẾT (DSA) =================
#include "core/BST.h"
#include "core/HashTable.h"
#include "core/LinkedList.h"
#include "core/Queue.h"
#include "core/Stack.h"
#include "core/Vector.h"


// ================= TIỆN ÍCH BỔ TRỢ =================
#include "utils/DateTime.h"
#include "utils/StringHelper.h"

// ================= MÔ HÌNH HƯỚNG ĐỐI TƯỢNG (OOP MODELS) =================
#include "models/Account.h"
#include "models/Cart.h"
#include "models/Common.h"
#include "models/Customer.h"
#include "models/DiscountCode.h"
#include "models/Employee.h"
#include "models/ImportReceipt.h"
#include "models/Invoice.h"
#include "models/Order.h"
#include "models/People.h"
#include "models/Phone.h"
#include "models/Supplier.h"
#include "models/WarrantyRequest.h"


// ================= TẦNG LƯU TRỮ VÀ NẠP TỆP (REPOSITORIES) =================
#include "repositories/AccountRepository.h"
#include "repositories/CustomerRepository.h"
#include "repositories/DiscountRepository.h"
#include "repositories/EmployeeRepository.h"
#include "repositories/InvoiceRepository.h"
#include "repositories/OrderRepository.h"
#include "repositories/PhoneRepository.h"
#include "repositories/ProductImageRepository.h"
#include "repositories/StockReceiptRepository.h"
#include "repositories/SupplierRepository.h"
#include "repositories/WarrantyRequestRepository.h"


// ================= TẦNG XỬ LÝ NGHIỆP VỤ VÀ DỮ LIỆU (SERVICES)
// =================
#include "services/AuthService.h"
#include "services/CustomerService.h"
#include "services/DiscountService.h"
#include "services/EmployeeService.h"
#include "services/InvoiceService.h"
#include "services/OrderService.h"
#include "services/PhoneService.h"
#include "services/ReportService.h"
#include "services/SupplierService.h"
#include "services/WarrantyService.h"


#endif // PBL2_H