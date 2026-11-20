// ORDER REPOSITORY - Tầng truy xuất dữ liệu đơn hàng
#ifndef ORDERREPOSITORY_H
#define ORDERREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include "../models/Order.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"
#include "../utils/SchemaHelper.h"

// Tầng Repository: CHỈ chịu trách nhiệm nạp và lưu dữ liệu đơn hàng từ/vào tệp tin.
// Lưu trữ danh sách Order trên RAM bằng Vector<Order>.
class OrderRepository {
private:
    std::string filePath;   // Đường dẫn đến file Orders.txt
    Vector<Order> orders;   // Danh sách đơn hàng trên RAM

    // Tạo file mới với header nếu file chưa tồn tại
    bool createEmptyFile() const;

public:
    // Hàm dựng
    OrderRepository(std::string path = "data/PAY/Order/Orders.txt");

    // Nạp dữ liệu từ file vào RAM
    bool loadFromFile();

    // Ghi toàn bộ danh sách RAM xuống file (ghi đè)
    bool saveToFile() const;
    const std::string& getFilePath() const { return filePath; }

    // Trả về toàn bộ danh sách (chỉ đọc / cho phép sửa)
    const Vector<Order>& getAllOrders() const;
    Vector<Order>& getAllOrders();

    // Tìm theo ID đơn hàng
    Order* findById(const std::string& orderId);
    const Order* findById(const std::string& orderId) const;

    // Lọc đơn hàng theo ID khách hàng
    Vector<Order> findByCustomerId(const std::string& customerId) const;

    // Lọc đơn hàng theo trạng thái
    Vector<Order> findByStatus(OrderStatus status) const;

    // Kiểm tra xem có đơn hàng nào chứa sản phẩm với ID tương ứng không
    bool containsPhone(const std::string& phoneId) const;

    // CRUD: Thêm, cập nhật, xóa
    bool addOrder(const Order& order);
    bool updateOrder(const Order& updatedOrder);
    bool deleteOrder(const std::string& id);

    // Kiểm tra ID có tồn tại không
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

// Tạo file Orders.txt mới với header chuẩn (12 cột bao gồm discountCode)
inline bool OrderRepository::createEmptyFile() const {
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        SchemaHelper::ORDERS_HEADER
    );
}

inline OrderRepository::OrderRepository(std::string path)
    : filePath(PathHelper::resolveDataPath(path)) {}

// Nạp dữ liệu từ Orders.txt vào RAM
// - Gọi Order::fromFileLine() để phân tích mỗi dòng
// - Nếu file không tồn tại: tự tạo file mới
// - Nếu một dòng bị lỗi: in cảnh báo, bỏ qua và đọc tiếp
inline bool OrderRepository::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Canh bao] Khong the mo file: " << filePath
                  << ". Dang tao file moi..." << std::endl;
        return createEmptyFile();
    }

    orders.clear();
    std::string line;
    bool isFirstNonEmpty = true;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        if (lineNum == 1) StringHelper::stripUtf8Bom(line);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (StringHelper::trim(line).empty()) continue;

        if (isFirstNonEmpty) {
            isFirstNonEmpty = false;
            auto kind = SchemaHelper::inspectFirstLine(
                line,
                SchemaHelper::ORDERS_HEADER,
                {SchemaHelper::ORDERS_V2_HEADER, SchemaHelper::ORDERS_V1_HEADER},
                filePath,
                [](const std::string& l) {
                    return SchemaHelper::looksLikePrefixedRecord(l, "ORD");
                });
            if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                continue;
            }
        }

        try {
            Order o = Order::fromFileLine(line);
            if (!o.getID().empty()) {
                orders.push_back(o);
            }
        } catch (const std::exception& e) {
            std::cerr << "[Canh bao] Loi doc dong " << lineNum
                      << " trong " << filePath << ": " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[Canh bao] Loi khong xac dinh tai dong " << lineNum
                      << " trong " << filePath << std::endl;
        }
    }
    file.close();
    return true;
}

// Ghi toàn bộ danh sách orders xuống file (qua Atomic File Write)
inline bool OrderRepository::saveToFile() const {
    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        file << SchemaHelper::ORDERS_HEADER << "\n";
        for (int i = 0; i < orders.getSize(); ++i) {
            file << orders[i].toFileString() << "\n";
        }
        return file.good();
    });
}

inline const Vector<Order>& OrderRepository::getAllOrders() const { return orders; }
inline Vector<Order>& OrderRepository::getAllOrders() { return orders; }

// Tìm Order theo ID
inline Order* OrderRepository::findById(const std::string& orderId) {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == orderId) return &orders[i];
    }
    return nullptr;
}

inline const Order* OrderRepository::findById(const std::string& orderId) const {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == orderId) return &orders[i];
    }
    return nullptr;
}

// Lọc tất cả đơn hàng của một khách hàng
inline Vector<Order> OrderRepository::findByCustomerId(const std::string& customerId) const {
    Vector<Order> result;
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getCustomerId() == customerId) {
            result.push_back(orders[i]);
        }
    }
    return result;
}

// Lọc đơn hàng theo trạng thái (ví dụ: PENDING, COMPLETED...)
inline Vector<Order> OrderRepository::findByStatus(OrderStatus status) const {
    Vector<Order> result;
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getStatus() == status) {
            result.push_back(orders[i]);
        }
    }
    return result;
}

// Kiểm tra xem một sản phẩm có trong bất kỳ đơn hàng nào không
inline bool OrderRepository::containsPhone(const std::string& phoneId) const {
    for (int i = 0; i < orders.getSize(); ++i) {
        const Vector<OrderDetail>& details = orders[i].getDetails();
        for (int j = 0; j < details.getSize(); ++j) {
            if (details[j].getPhoneId() == phoneId) return true;
        }
    }
    return false;
}

// Thêm đơn hàng mới
inline bool OrderRepository::addOrder(const Order& order) {
    if (existsById(order.getID())) {
        std::cerr << "[Canh bao] ID don hang da ton tai: " << order.getID() << std::endl;
        return false;
    }
    // FIX LỖI: Luu snapshot RAM de rollback neu ghi file that bai
    Vector<Order> backup = orders;
    orders.push_back(order);
    if (!saveToFile()) {
        orders = backup; // Rollback
        return false;
    }
    return true;
}

// Cập nhật đơn hàng theo ID
inline bool OrderRepository::updateOrder(const Order& updatedOrder) {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == updatedOrder.getID()) {
            // FIX LỖI: Luu snapshot RAM de rollback neu ghi file that bai
            Vector<Order> backup = orders;
            orders[i] = updatedOrder;
            if (!saveToFile()) {
                orders = backup; // Rollback
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay don hang ID: " << updatedOrder.getID() << std::endl;
    return false;
}

// Xóa đơn hàng theo ID
inline bool OrderRepository::deleteOrder(const std::string& id) {
    for (int i = 0; i < orders.getSize(); ++i) {
        if (orders[i].getID() == id) {
            // FIX LỖI: Luu snapshot RAM de rollback neu ghi file that bai
            Vector<Order> backup = orders;
            orders.removeAt(i);
            if (!saveToFile()) {
                orders = backup; // Rollback
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay don hang ID de xoa: " << id << std::endl;
    return false;
}

inline bool OrderRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // ORDERREPOSITORY_H
