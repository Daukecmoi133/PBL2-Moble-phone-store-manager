#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H

#include "../core/Queue.h"
#include "../core/Vector.h"
#include "../models/Order.h"
#include "../repositories/OrderRepository.h"
#include "../utils/IdGenerator.h"
#include "PhoneService.h"
#include <iostream>
#include <string>
#include <vector>


// Tầng Service: Chịu trách nhiệm toàn bộ nghiệp vụ đơn hàng,
// hàng đợi duyệt đơn FIFO (Queue) và liên kết trừ tồn kho sản phẩm.
class OrderService {
private:
  OrderRepository &orderRepo;
  PhoneService *phoneService;
  Queue<Order> pendingQueue; // Hàng đợi FIFO xử lý đơn hàng chờ duyệt

  void loadPendingQueue();

public:
  OrderService(OrderRepository &oRepo, PhoneService *pService = nullptr);

  void setPhoneService(PhoneService *pService);
  Queue<Order> &getPendingQueue();
  const Vector<Order> &getAllOrders() const;
  OrderRepository& getRepository() { return orderRepo; }
  const OrderRepository& getRepository() const { return orderRepo; }
  void reload() { orderRepo.loadFromFile(); loadPendingQueue(); }

  Order *getOrderById(const std::string &orderId);
  Vector<Order> getOrdersByCustomer(const std::string &customerId) const;

  // Nghiệp vụ tạo đơn hàng: Kiểm tra tồn kho trước khi tạo
  bool createOrder(const Order &order, std::string &errorMessage);
  bool createOrder(const Order &order);

  // Nghiệp vụ thêm đơn hàng đã hoàn tất (dành cho POS tại quầy)
  bool addCompletedOrder(const Order &order, std::string &errorMessage);

  // Nghiệp vụ duyệt đơn hàng tiếp theo từ hàng đợi FIFO
  bool processNextPendingOrder(Order &processedOrder,
                               std::string &errorMessage);
  bool processNextPendingOrder(Order &processedOrder);

  // Nghiệp vụ cập nhật trạng thái đơn hàng
  bool updateOrderStatus(const std::string& orderId, OrderStatus newStatus, std::string& errorMessage);

  // Nghiệp vụ hủy đơn hàng
  bool cancelOrder(const std::string &orderId, std::string &errorMessage);
  bool cancelOrder(const std::string &orderId);

  // Nghiệp vụ tính tổng doanh thu từ các đơn đã xác nhận/hoàn thành
  double calculateTotalRevenue() const;
};

// ================= IMPLEMENTATION =================

inline void OrderService::loadPendingQueue() {
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() == OrderStatus::PENDING) {
      pendingQueue.push(orders[i]);
    }
  }
}

inline OrderService::OrderService(OrderRepository &oRepo,
                                  PhoneService *pService)
    : orderRepo(oRepo), phoneService(pService) {
  loadPendingQueue();
}

inline void OrderService::setPhoneService(PhoneService *pService) {
  phoneService = pService;
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

  // Kiểm tra số lượng tồn kho từng sản phẩm nếu có liên kết PhoneService
  if (phoneService != nullptr) {
    const Vector<OrderDetail> &details = newOrder.getDetails();
    for (int i = 0; i < details.getSize(); ++i) {
      if (!phoneService->hasStock(details[i].getPhoneId(),
                                  details[i].getQuantity())) {
        errorMessage = "Sản phẩm [" + details[i].getPhoneName() +
                       "] không đủ số lượng tồn kho!";
        return false;
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

inline bool OrderService::processNextPendingOrder(Order &processedOrder,
                                                  std::string &errorMessage) {
  // FIX LỖI: Lấy đơn từ FIFO, bỏ qua các đơn không còn ở trạng thái PENDING (ví
  // dụ đã bị hủy trước đó)
  Order *ord = nullptr;
  while (!pendingQueue.isEmpty()) {
    processedOrder = pendingQueue.front();
    pendingQueue.pop();
    ord = orderRepo.findById(processedOrder.getID());
    if (ord != nullptr && ord->getStatus() == OrderStatus::PENDING) {
      break; // Tìm thấy đơn PENDING hợp lệ
    }
    ord = nullptr;
  }

  if (ord == nullptr) {
    errorMessage = "Hàng đợi rỗng hoặc không có đơn hàng nào đang chờ duyệt!";
    return false;
  }

  const Vector<OrderDetail> &details = ord->getDetails();

  // Bước 1: Kiểm tra lại tồn kho của TẤT CẢ sản phẩm trước khi trừ
  if (phoneService != nullptr) {
    for (int i = 0; i < details.getSize(); ++i) {
      if (!phoneService->hasStock(details[i].getPhoneId(),
                                  details[i].getQuantity())) {
        errorMessage = "Sản phẩm [" + details[i].getPhoneName() +
                       "] không đủ số lượng tồn kho để duyệt đơn!";
        return false;
      }
    }
  }

  // Bước 2: Tiến hành trừ kho có cơ chế Rollback nếu phát sinh lỗi giữa chừng
  int deductedCount = 0;
  if (phoneService != nullptr) {
    for (int i = 0; i < details.getSize(); ++i) {
      std::string errStock;
      if (!phoneService->updateStock(details[i].getPhoneId(),
                                     -details[i].getQuantity(), errStock)) {
        // Rollback lại số lượng các sản phẩm đã trừ trước đó
        for (int j = 0; j < deductedCount; ++j) {
          phoneService->updateStock(details[j].getPhoneId(),
                                    details[j].getQuantity());
        }
        errorMessage = "Lỗi khi trừ kho sản phẩm: " + errStock;
        return false;
      }
      deductedCount++;
    }
  }

  // Bước 3: Cập nhật trạng thái đơn sang CONFIRMED và lưu
  OrderStatus oldStatus = ord->getStatus();
  ord->setStatus(OrderStatus::CONFIRMED);
  if (!orderRepo.updateOrder(*ord)) {
    // Rollback kho nếu lưu thất bại
    if (phoneService != nullptr) {
      for (int j = 0; j < deductedCount; ++j) {
        phoneService->updateStock(details[j].getPhoneId(),
                                  details[j].getQuantity());
      }
    }
    ord->setStatus(oldStatus);
    errorMessage =
        "Lỗi khi lưu trạng thái xác nhận đơn hàng vào cơ sở dữ liệu!";
    return false;
  }

  processedOrder = *ord;
  return true;
}

inline bool OrderService::addCompletedOrder(const Order &order, std::string &errorMessage) {
  if (!orderRepo.addOrder(order)) {
    errorMessage = "Lỗi khi lưu đơn hàng POS vào cơ sở dữ liệu!";
    return false;
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
  OrderStatus oldStatus = ord->getStatus();
  ord->setStatus(newStatus);
  if (!orderRepo.updateOrder(*ord)) {
    ord->setStatus(oldStatus);
    errorMessage = "Lỗi khi lưu trạng thái đơn hàng vào cơ sở dữ liệu!";
    return false;
  }
  return true;
}

inline bool OrderService::cancelOrder(const std::string &orderId,
                                      std::string &errorMessage) {
  Order *ord = orderRepo.findById(orderId);
  if (ord == nullptr) {
    errorMessage = "Không tìm thấy đơn hàng [" + orderId + "]!";
    return false;
  }

  // FIX LỖI: Kiểm tra chuyển đổi trạng thái hợp lệ
  if (ord->getStatus() == OrderStatus::CANCELLED) {
    errorMessage = "Đơn hàng đã được hủy trước đó!";
    return false;
  }

  if (ord->getStatus() == OrderStatus::COMPLETED) {
    errorMessage = "Không thể hủy đơn hàng đã hoàn thành!";
    return false;
  }

  // Nếu đơn đã xác nhận (đã trừ kho), khi hủy cần hoàn lại số lượng tồn kho
  if (ord->getStatus() == OrderStatus::CONFIRMED && phoneService != nullptr) {
    const Vector<OrderDetail> &details = ord->getDetails();
    for (int i = 0; i < details.getSize(); ++i) {
      phoneService->updateStock(details[i].getPhoneId(),
                                details[i].getQuantity());
    }
  }

  OrderStatus oldStatus = ord->getStatus();
  ord->setStatus(OrderStatus::CANCELLED);
  if (!orderRepo.updateOrder(*ord)) {
    // Nếu lưu thất bại và trước đó đã hoàn kho thì phải trừ lại kho
    if (oldStatus == OrderStatus::CONFIRMED && phoneService != nullptr) {
      const Vector<OrderDetail> &details = ord->getDetails();
      for (int i = 0; i < details.getSize(); ++i) {
        phoneService->updateStock(details[i].getPhoneId(),
                                  -details[i].getQuantity());
      }
    }
    ord->setStatus(oldStatus);
    errorMessage = "Lỗi khi lưu trạng thái hủy đơn vào cơ sở dữ liệu!";
    return false;
  }

  return true;
}

inline bool OrderService::cancelOrder(const std::string &orderId) {
  std::string err;
  return cancelOrder(orderId, err);
}

inline double OrderService::calculateTotalRevenue() const {
  double revenue = 0.0;
  const Vector<Order> &orders = orderRepo.getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getStatus() == OrderStatus::CONFIRMED ||
        orders[i].getStatus() == OrderStatus::COMPLETED) {
      revenue += orders[i].calculateTotal();
    }
  }
  return revenue;
}

#endif // ORDERSERVICE_H
