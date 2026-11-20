#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H

#include "../core/Queue.h"
#include "../core/Vector.h"
#include "../models/Order.h"
#include "../repositories/OrderRepository.h"
#include "../utils/IdGenerator.h"
#include "../utils/TransactionCoordinator.h"
#include "DiscountService.h"
#include "PhoneService.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ đơn hàng,
// hàng đợi duyệt đơn FIFO (Queue), máy trạng thái đơn hàng (D4),
// giữ chỗ khi PENDING / trừ tồn kho vật lý khi CONFIRMED / hoàn kho & hoàn voucher khi hủy.
class OrderService {
private:
  OrderRepository &orderRepo;
  PhoneService *phoneService;
  DiscountService *discountService;
  Queue<Order> pendingQueue; // Hàng đợi FIFO xử lý đơn hàng chờ duyệt

  void loadPendingQueue();

public:
  OrderService(OrderRepository &oRepo, PhoneService *pService = nullptr,
               DiscountService *dService = nullptr);

  void setPhoneService(PhoneService *pService);
  void setDiscountService(DiscountService *dService);
  Queue<Order> &getPendingQueue();
  const Vector<Order> &getAllOrders() const;
  OrderRepository& getRepository() { return orderRepo; }
  const OrderRepository& getRepository() const { return orderRepo; }
  void reload() { orderRepo.loadFromFile(); loadPendingQueue(); }

  Order *getOrderById(const std::string &orderId);
  Vector<Order> getOrdersByCustomer(const std::string &customerId) const;

  static bool hasStockDeducted(OrderStatus s);
  static bool isValidTransition(OrderStatus from, OrderStatus to);

  int getReservedStockForPhone(const std::string &phoneId) const;
  std::unordered_map<std::string, int> getReservedStockMap() const;
  bool hasCustomerUsedDiscountCode(const std::string &customerId,
                                   const std::string &discountCode) const;

  // Nghiệp vụ tạo đơn hàng: Kiểm tra tồn kho khả dụng (gồm giữ chỗ PENDING) và snapshot unitCost
  bool createOrder(const Order &order, std::string &errorMessage);
  bool createOrder(const Order &order);

  // Nghiệp vụ thêm đơn hàng đã hoàn tất (dành cho POS tại quầy)
  bool addCompletedOrder(const Order &order, std::string &errorMessage);

  // Chuyển trạng thái cốt lõi (gọi bên trong FileTransaction đã mở sẵn bởi caller)
  bool transitionStatusCore(const std::string &orderId,
                            OrderStatus newStatus,
                            std::string &errorMessage);

  // Nghiệp vụ duyệt đơn hàng tiếp theo từ hàng đợi FIFO (chuyển PENDING -> CONFIRMED)
  bool processNextPendingOrder(Order &processedOrder,
                               std::string &errorMessage);
  bool processNextPendingOrder(Order &processedOrder);

  // Nghiệp vụ cập nhật trạng thái đơn hàng (tự quản lý FileTransaction)
  bool updateOrderStatus(const std::string &orderId, OrderStatus newStatus,
                         std::string &errorMessage);

  // Nghiệp vụ hủy đơn hàng (chuyển -> CANCELLED, hoàn kho nếu đã qua CONFIRMED, hoàn voucher)
  bool cancelOrder(const std::string &orderId, std::string &errorMessage);
  bool cancelOrder(const std::string &orderId);

  // Nghiệp vụ tính tổng doanh thu thuần (trước VAT, sau giảm giá) từ các đơn COMPLETED
  double calculateTotalRevenue() const;
};

// ================= IMPLEMENTATION =================

inline void OrderService::loadPendingQueue() {
  while (!pendingQueue.isEmpty()) {
    pendingQueue.pop();
  }
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() == OrderStatus::PENDING) {
      pendingQueue.push(orders[i]);
    }
  }
}

inline OrderService::OrderService(OrderRepository &oRepo,
                                  PhoneService *pService,
                                  DiscountService *dService)
    : orderRepo(oRepo), phoneService(pService), discountService(dService) {
  loadPendingQueue();
}

inline void OrderService::setPhoneService(PhoneService *pService) {
  phoneService = pService;
}

inline void OrderService::setDiscountService(DiscountService *dService) {
  discountService = dService;
}

inline Queue<Order> &OrderService::getPendingQueue() { return pendingQueue; }

inline const Vector<Order> &OrderService::getAllOrders() const {
  return orderRepo.getAllOrders();
}

inline Order *OrderService::getOrderById(const std::string &orderId) {
  return orderRepo.findById(orderId);
}

inline Vector<Order>
OrderService::getOrdersByCustomer(const std::string &customerId) const {
  Vector<Order> result;
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getCustomerId() == customerId) {
      result.push_back(orders[i]);
    }
  }
  return result;
}

inline bool OrderService::hasStockDeducted(OrderStatus s) {
  return s == OrderStatus::CONFIRMED ||
         s == OrderStatus::PREPARING ||
         s == OrderStatus::READY_FOR_PICKUP ||
         s == OrderStatus::SHIPPING ||
         s == OrderStatus::COMPLETED;
}

inline bool OrderService::isValidTransition(OrderStatus from, OrderStatus to) {
  if (from == to) return false;
  switch (from) {
    case OrderStatus::PENDING:
      return to == OrderStatus::CONFIRMED ||
             to == OrderStatus::CANCELLED;
    case OrderStatus::CONFIRMED:
      return to == OrderStatus::PREPARING ||
             to == OrderStatus::READY_FOR_PICKUP ||
             to == OrderStatus::SHIPPING ||
             to == OrderStatus::COMPLETED ||
             to == OrderStatus::CANCELLED;
    case OrderStatus::PREPARING:
      return to == OrderStatus::READY_FOR_PICKUP ||
             to == OrderStatus::SHIPPING ||
             to == OrderStatus::COMPLETED ||
             to == OrderStatus::CANCELLED;
    case OrderStatus::READY_FOR_PICKUP:
    case OrderStatus::SHIPPING:
      return to == OrderStatus::COMPLETED ||
             to == OrderStatus::CANCELLED;
    case OrderStatus::COMPLETED:
    case OrderStatus::CANCELLED:
    default:
      return false;
  }
}

inline int OrderService::getReservedStockForPhone(const std::string &phoneId) const {
  if (phoneId.empty()) return 0;
  int reserved = 0;
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() != OrderStatus::PENDING) continue;
    const Vector<OrderDetail> &details = orders[i].getDetails();
    for (int j = 0; j < details.getSize(); ++j) {
      if (details[j].getPhoneId() == phoneId) {
        reserved += details[j].getQuantity();
      }
    }
  }
  return reserved;
}

inline std::unordered_map<std::string, int> OrderService::getReservedStockMap() const {
  std::unordered_map<std::string, int> reservedMap;
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() != OrderStatus::PENDING) continue;
    const Vector<OrderDetail> &details = orders[i].getDetails();
    for (int j = 0; j < details.getSize(); ++j) {
      reservedMap[details[j].getPhoneId()] += details[j].getQuantity();
    }
  }
  return reservedMap;
}

inline bool OrderService::hasCustomerUsedDiscountCode(const std::string &customerId,
                                                      const std::string &discountCode) const {
  const std::string cid = StringHelper::trim(customerId);
  const std::string code = StringHelper::trim(discountCode);
  if (cid.empty() || cid == "WALK-IN" || code.empty()) {
    return false;
  }
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() == OrderStatus::CANCELLED) continue;
    if (StringHelper::trim(orders[i].getCustomerId()) == cid &&
        StringHelper::trim(orders[i].getDiscountCode()) == code) {
      return true;
    }
  }
  return false;
}

inline bool OrderService::createOrder(const Order &order,
                                      std::string &errorMessage) {
  Order newOrder = order;
  if (newOrder.getID().empty()) {
    std::vector<std::string> existingIds;
    const Vector<Order>& orders = orderRepo.getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
      existingIds.push_back(orders[i].getID());
    }
    std::string generatedId;
    const std::string idDate = IdGenerator::currentDateKey();
    if (idDate.empty() ||
        !IdGenerator::nextId("ORD", idDate,
                             existingIds, generatedId, errorMessage)) {
      if (errorMessage.empty()) errorMessage = "Không thể xác định ngày để sinh mã đơn hàng.";
      return false;
    }
    newOrder.setID(generatedId);
  } else if (!IdGenerator::isValidId(newOrder.getID(), "ORD", true)) {
    errorMessage = "Mã đơn hàng phải đúng định dạng ORD-ddmmyyyy-000001.";
    return false;
  }

  if (newOrder.getDetails().getSize() == 0) {
    errorMessage = "Đơn hàng phải có ít nhất một sản phẩm!";
    return false;
  }

  // Kiểm tra tồn kho khả dụng (cộng gộp trùng mã & trừ giữ chỗ PENDING) & snapshot giá vốn khi bán (AUD-03)
  if (phoneService != nullptr) {
    const std::unordered_map<std::string, int> reservedMap = getReservedStockMap();
    std::unordered_map<std::string, int> reqByPhone;
    Vector<OrderDetail> &details = newOrder.getDetails();
    for (int i = 0; i < details.getSize(); ++i) {
      reqByPhone[details[i].getPhoneId()] += details[i].getQuantity();
    }
    for (int i = 0; i < details.getSize(); ++i) {
      Phone *p = phoneService->getPhoneById(details[i].getPhoneId());
      if (p == nullptr) {
        errorMessage = "Sản phẩm [" + details[i].getPhoneId() + "] không tồn tại!";
        return false;
      }
      auto resIt = reservedMap.find(details[i].getPhoneId());
      const int reserved = (resIt != reservedMap.end()) ? resIt->second : 0;
      const int totalReq = reqByPhone[details[i].getPhoneId()];
      const int avail = p->getQuantity() - reserved;
      if (avail < totalReq) {
        errorMessage = "Sản phẩm [" + details[i].getPhoneName() +
                       "] không đủ số lượng tồn kho khả dụng!";
        return false;
      }
      if (!details[i].hasUnitCost()) {
        details[i].setUnitCost(p->getImportPrice());
      }
    }
  }

  if (!orderRepo.addOrder(newOrder)) {
    errorMessage = "Không thể lưu đơn hàng vào hệ thống!";
    return false;
  }

  if (newOrder.getStatus() == OrderStatus::PENDING) {
    pendingQueue.push(newOrder);
  }

  return true;
}

inline bool OrderService::createOrder(const Order &order) {
  std::string err;
  return createOrder(order, err);
}

inline bool OrderService::addCompletedOrder(const Order &order, std::string &errorMessage) {
  Order toSave = order;
  if (phoneService != nullptr) {
    Vector<OrderDetail> &details = toSave.getDetails();
    for (int i = 0; i < details.getSize(); ++i) {
      if (!details[i].hasUnitCost()) {
        Phone *p = phoneService->getPhoneById(details[i].getPhoneId());
        if (p != nullptr) {
          details[i].setUnitCost(p->getImportPrice());
        }
      }
    }
  }
  if (!orderRepo.addOrder(toSave)) {
    errorMessage = "Lỗi khi lưu đơn hàng POS vào cơ sở dữ liệu!";
    return false;
  }
  return true;
}

inline bool OrderService::transitionStatusCore(const std::string &orderId,
                                               OrderStatus newStatus,
                                               std::string &errorMessage) {
  Order *ord = orderRepo.findById(orderId);
  if (ord == nullptr) {
    errorMessage = "Không tìm thấy đơn hàng [" + orderId + "]!";
    return false;
  }

  const OrderStatus oldStatus = ord->getStatus();
  if (oldStatus == OrderStatus::COMPLETED) {
    errorMessage = "Đơn hàng đã hoàn thành, không thể thay đổi trạng thái!";
    return false;
  }
  if (oldStatus == OrderStatus::CANCELLED) {
    errorMessage = "Đơn hàng đã bị hủy, không thể thay đổi trạng thái!";
    return false;
  }
  if (!isValidTransition(oldStatus, newStatus)) {
    errorMessage = "Chuyển trạng thái không hợp lệ từ '" +
                   orderStatusToString(oldStatus) + "' sang '" +
                   orderStatusToString(newStatus) + "'!";
    return false;
  }

  const bool needDeduct = (!hasStockDeducted(oldStatus) && hasStockDeducted(newStatus));
  const bool needRestore = (hasStockDeducted(oldStatus) && newStatus == OrderStatus::CANCELLED);
  const bool needReleaseVoucher =
      (newStatus == OrderStatus::CANCELLED &&
       !StringHelper::trim(ord->getDiscountCode()).empty() &&
       discountService != nullptr);

  // Kiểm tra tồn kho vật lý toàn bộ dòng (cộng gộp trùng mã) trước khi trừ khi xác nhận đơn (D4)
  const Vector<OrderDetail> details = ord->getDetails();
  if (needDeduct && phoneService != nullptr) {
    std::unordered_map<std::string, int> reqByPhone;
    for (int i = 0; i < details.getSize(); ++i) {
      reqByPhone[details[i].getPhoneId()] += details[i].getQuantity();
    }
    for (int i = 0; i < details.getSize(); ++i) {
      const int totalReq = reqByPhone[details[i].getPhoneId()];
      if (!phoneService->hasStock(details[i].getPhoneId(), totalReq)) {
        errorMessage = "Sản phẩm [" + details[i].getPhoneName() +
                       "] không đủ số lượng tồn kho vật lý để xác nhận đơn!";
        return false;
      }
    }
    for (int i = 0; i < details.getSize(); ++i) {
      std::string stockErr;
      if (!phoneService->updateStock(details[i].getPhoneId(),
                                     -details[i].getQuantity(),
                                     stockErr)) {
        errorMessage = "Lỗi khi trừ kho sản phẩm [" + details[i].getPhoneName() + "]: " + stockErr;
        return false;
      }
    }
  }

  if (needRestore && phoneService != nullptr) {
    for (int i = 0; i < details.getSize(); ++i) {
      std::string stockErr;
      if (!phoneService->updateStock(details[i].getPhoneId(),
                                     details[i].getQuantity(),
                                     stockErr)) {
        errorMessage = "Lỗi khi hoàn kho sản phẩm [" + details[i].getPhoneName() + "]: " + stockErr;
        return false;
      }
    }
  }

  if (needReleaseVoucher) {
    std::string discErr;
    if (!discountService->releaseUsage(ord->getDiscountCode(), discErr)) {
      errorMessage = !discErr.empty() ? discErr : "Lỗi khi hoàn trả lượt sử dụng voucher!";
      return false;
    }
  }

  ord->setStatus(newStatus);
  if (!orderRepo.updateOrder(*ord)) {
    ord->setStatus(oldStatus);
    errorMessage = "Lỗi khi lưu trạng thái đơn hàng vào cơ sở dữ liệu!";
    return false;
  }

  loadPendingQueue();
  return true;
}

inline bool OrderService::processNextPendingOrder(Order &processedOrder,
                                                  std::string &errorMessage) {
  std::string targetId;
  while (!pendingQueue.isEmpty()) {
    Order frontOrd = pendingQueue.front();
    pendingQueue.pop();
    Order *ord = orderRepo.findById(frontOrd.getID());
    if (ord != nullptr && ord->getStatus() == OrderStatus::PENDING) {
      targetId = ord->getID();
      break;
    }
  }

  if (targetId.empty()) {
    errorMessage = "Hàng đợi rỗng hoặc không có đơn hàng nào đang chờ duyệt!";
    return false;
  }

  if (!updateOrderStatus(targetId, OrderStatus::CONFIRMED, errorMessage)) {
    loadPendingQueue();
    return false;
  }

  Order *updated = orderRepo.findById(targetId);
  if (updated != nullptr) {
    processedOrder = *updated;
  }
  return true;
}

inline bool OrderService::processNextPendingOrder(Order &processedOrder) {
  std::string err;
  return processNextPendingOrder(processedOrder, err);
}

inline bool OrderService::updateOrderStatus(const std::string& orderId,
                                            OrderStatus newStatus,
                                            std::string& errorMessage) {
  Order* ord = orderRepo.findById(orderId);
  if (ord == nullptr) {
    errorMessage = "Không tìm thấy đơn hàng [" + orderId + "]!";
    return false;
  }
  const OrderStatus oldStatus = ord->getStatus();
  if (oldStatus == OrderStatus::COMPLETED) {
    errorMessage = "Đơn hàng đã hoàn thành, không thể thay đổi trạng thái!";
    return false;
  }
  if (oldStatus == OrderStatus::CANCELLED) {
    errorMessage = "Đơn hàng đã bị hủy, không thể thay đổi trạng thái!";
    return false;
  }
  if (!isValidTransition(oldStatus, newStatus)) {
    errorMessage = "Chuyển trạng thái không hợp lệ từ '" +
                   orderStatusToString(oldStatus) + "' sang '" +
                   orderStatusToString(newStatus) + "'!";
    return false;
  }

  const bool needDeduct = (!hasStockDeducted(oldStatus) && hasStockDeducted(newStatus));
  const bool needRestore = (hasStockDeducted(oldStatus) && newStatus == OrderStatus::CANCELLED);
  const bool needReleaseVoucher =
      (newStatus == OrderStatus::CANCELLED &&
       !StringHelper::trim(ord->getDiscountCode()).empty() &&
       discountService != nullptr);

  FileTransaction tx;
  tx.addParticipant(orderRepo.getFilePath());
  if ((needDeduct || needRestore) && phoneService != nullptr) {
    tx.addParticipant(phoneService->getRepository().getFilePath());
  }
  if (needReleaseVoucher) {
    tx.addParticipant(discountService->getRepository().getFilePath());
  }
  tx.onRollback([this, needDeduct, needRestore, needReleaseVoucher]() {
    this->reload();
    if ((needDeduct || needRestore) && this->phoneService != nullptr) {
      this->phoneService->reload();
    }
    if (needReleaseVoucher && this->discountService != nullptr) {
      this->discountService->reload();
    }
  });

  if (!tx.begin()) {
    errorMessage = "Không thể khởi tạo giao dịch cập nhật trạng thái đơn hàng!";
    return false;
  }

  if (!transitionStatusCore(orderId, newStatus, errorMessage)) {
    tx.rollback();
    return false;
  }

  if (!tx.commit()) {
    tx.rollback();
    errorMessage = "Lỗi khi xác nhận giao dịch cập nhật trạng thái đơn hàng!";
    return false;
  }
  return true;
}

inline bool OrderService::cancelOrder(const std::string &orderId,
                                      std::string &errorMessage) {
  return updateOrderStatus(orderId, OrderStatus::CANCELLED, errorMessage);
}

inline bool OrderService::cancelOrder(const std::string &orderId) {
  std::string err;
  return cancelOrder(orderId, err);
}

inline double OrderService::calculateTotalRevenue() const {
  double revenue = 0.0;
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() == OrderStatus::COMPLETED) {
      revenue += orders[i].calculateNetRevenue();
    }
  }
  return revenue;
}

#endif // ORDERSERVICE_H
