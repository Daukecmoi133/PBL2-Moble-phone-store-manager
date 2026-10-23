#pragma once
// Repository đơn hàng: tra cứu theo khách / trạng thái và hàng đợi đơn PENDING.
// Hàng đợi chỉ lưu ID đơn. Khi lấy ra phải đọc lại trạng thái và bỏ qua đơn không còn PENDING.

#include <string>

#include "Repository.h"
#include "../model/sales/Order.h"
#include "../storage/ds/Queue.h"

class OrderRepository : public Repository<Order> {
public:
    explicit OrderRepository(FileManager& fm);

    Result<Vector<Order>> findByCustomer(const std::string& customerId) const;
    Result<Vector<Order>> findByStatus(OrderStatus status) const;
    Result<Order> peekNextPending() const;   // đơn PENDING cũ nhất (FIFO), trả bản sao
    void rebuildPendingQueue();              // dựng lại hàng đợi ID đơn PENDING

protected:
    std::string getId(const Order& o) const override;
    std::string serialize(const Order& o) const override;
    Result<Order> deserialize(const std::string& line) const override;
    void rebuildIndexes() override;

private:
    Queue<std::string> pendingQueue; // chỉ lưu ID đơn PENDING
};
