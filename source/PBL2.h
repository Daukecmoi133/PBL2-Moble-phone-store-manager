#ifndef PBL2_H
#define PBL2_H

// ================= THƯ VIỆN CHUẨN C++ =================
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <utility>
#include <ctime>

// ================= CẤU TRÚC DỮ LIỆU TỰ VIẾT (DSA) =================
#include "core/Vector.h"
#include "core/LinkedList.h"
#include "core/Queue.h"
#include "core/Stack.h"
#include "core/HashTable.h"
#include "core/BST.h"

// ================= TIỆN ÍCH BỔ TRỢ =================
#include "utils/DateTime.h"
#include "utils/StringHelper.h"

// ================= MÔ HÌNH HƯỚNG ĐỐI TƯỢNG (OOP MODELS) =================
#include "models/Common.h"
#include "models/Phone.h"
#include "models/People.h"
#include "models/Customer.h"
#include "models/Employee.h"
#include "models/Supplier.h"
#include "models/Account.h"
#include "models/Cart.h"
#include "models/Order.h"
#include "models/Invoice.h"
#include "models/ImportReceipt.h"
#include "models/DiscountCode.h"

// ================= TẦNG LƯU TRỮ VÀ NẠP TỆP (REPOSITORIES) =================
#include "repositories/PhoneRepository.h"
#include "repositories/EmployeeRepository.h"
#include "repositories/CustomerRepository.h"
#include "repositories/DiscountRepository.h"
#include "repositories/OrderRepository.h"

#endif // PBL2_H