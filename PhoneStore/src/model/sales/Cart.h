// ĐỐI TƯỢNG GIỎ HÀNG
#ifndef CART_H
#define CART_H

#include <string>
// FIX LỖI: đổi include theo cây thư mục mới
#include "../catalog/Phone.h"
#include "../../storage/ds/Vector.h"

class CartItem {
private:
    Phone phone;
    int quantity;
    double unitPrice;
    bool selected;

public:
    CartItem();
    CartItem(const Phone& phone, int quantity);

    Phone getPhone() const;
    void setPhone(const Phone& p);

    int getQuantity() const;
    void setQuantity(int q);

    double getUnitPrice() const;
    void setUnitPrice(double price);

    bool isSelected() const;
    void setSelected(bool s);

    double getSubtotal() const;
    void increaseQuantity(int amount);
};

class Cart {
private:
    std::string customerId;
    Vector<CartItem> items;

public:
    Cart(std::string custId = "");

    std::string getCustomerId() const;
    void setCustomerId(const std::string& id);

    const Vector<CartItem>& getItems() const;
    Vector<CartItem>& getItems();

    void addItem(const Phone& phone, int quantity);
    bool removeItem(const std::string& phoneId);
    bool updateQuantity(const std::string& phoneId, int newQuantity);
    double calculateTotal() const;
    double calculateSelectedTotal() const;
    Vector<CartItem> getSelectedItems() const;
    void removeSelected();
    int getTotalItemCount() const;
    void clear();
    bool isEmpty() const;
};

// ================= IMPLEMENTATION CARTITEM =================

// Hàm dựng mặc định
inline CartItem::CartItem() : phone(), quantity(0), unitPrice(0.0), selected(true) {}

// Hàm dựng đủ tham số
inline CartItem::CartItem(const Phone& phone, int quantity)
    : phone(phone), quantity(quantity), unitPrice(phone.getSellingPrice()), selected(true) {}

// Hàm có chức năng trả về Phone mà CardItem đang chứa
inline Phone CartItem::getPhone() const { return phone; }

// Hàm có chức năng gán Phone cho CardItem chứa
inline void CartItem::setPhone(const Phone& p) { phone = p; }

// Hàm getter / setter cho số lượng
inline int CartItem::getQuantity() const { return quantity; }
inline void CartItem::setQuantity(int q) { quantity = q; }

// Hàm setter / getter cho đơn giá điện thoại
inline double CartItem::getUnitPrice() const { return unitPrice; }
inline void CartItem::setUnitPrice(double price) { unitPrice = price; }

// Hàm getter / setter cho trạng thái tick chọn
inline bool CartItem::isSelected() const { return selected; }
inline void CartItem::setSelected(bool s) { selected = s; }

// Hàm có chức năng tính tổng số tiền của sản phầm hiện tại
inline double CartItem::getSubtotal() const {
    return unitPrice * quantity;
}

// Hàm có chức năng tăng số lượng sản phẩm hiện tại lên amount
inline void CartItem::increaseQuantity(int amount) {
    if (amount > 0) quantity += amount;
}

// ================= IMPLEMENTATION CART =================

// Hàm dựng có đủ đối số
inline Cart::Cart(std::string custId) : customerId(custId) {}

// Hàm setter / getter cho CustomerID
inline std::string Cart::getCustomerId() const { return customerId; }
inline void Cart::setCustomerId(const std::string& id) { customerId = id; }

// Hàm setter / getter cho từng item
inline const Vector<CartItem>& Cart::getItems() const { return items; }
inline Vector<CartItem>& Cart::getItems() { return items; }

// Hàm thêm sản phẩm mới vào giỏ hàng
inline void Cart::addItem(const Phone& phone, int quantity) {
    if (quantity <= 0) return;

    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getPhone().getID() == phone.getID()) { // Cộng dồn vào nếu đã tồn tại
            items[i].increaseQuantity(quantity);
            return;
        }
    }
    items.push_back(CartItem(phone, quantity));
}

// Xóa sản phẩm trong giỏ hàng
inline bool Cart::removeItem(const std::string& phoneId) {
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getPhone().getID() == phoneId) { // Xóa nếu có tồn tại bên trong giỏ hàng
            return items.removeAt(i);
        }
    }
    return false;
}

// Cập nhật số lượng mới cho sản phẩm trong giỏ hàng
inline bool Cart::updateQuantity(const std::string& phoneId, int newQuantity) {
    if (newQuantity <= 0) {
        return removeItem(phoneId);
    }
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].getPhone().getID() == phoneId) {
            items[i].setQuantity(newQuantity);
            return true;
        }
    }
    return false;
}

// Tính tổng giá trị của cả giỏ hàng
inline double Cart::calculateTotal() const {
    double total = 0.0;
    for (int i = 0; i < items.getSize(); ++i) {
        total += items[i].getSubtotal();
    }
    return total;
}

// Tính tổng giá trị của chỉ các sản phẩm đã tick chọn
inline double Cart::calculateSelectedTotal() const {
    double total = 0.0;
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].isSelected()) {
            total += items[i].getSubtotal();
        }
    }
    return total;
}

// Lấy danh sách các dòng sản phẩm đã tick chọn
inline Vector<CartItem> Cart::getSelectedItems() const {
    Vector<CartItem> selected;
    for (int i = 0; i < items.getSize(); ++i) {
        if (items[i].isSelected()) {
            selected.push_back(items[i]);
        }
    }
    return selected;
}

// Xóa các dòng sản phẩm đã tick chọn khỏi giỏ hàng
inline void Cart::removeSelected() {
    Vector<CartItem> remaining;
    for (int i = 0; i < items.getSize(); ++i) {
        if (!items[i].isSelected()) {
            remaining.push_back(items[i]);
        }
    }
    items = remaining;
}

// Tính tổng số lượng sản phẩm có trong giỏ hàng
inline int Cart::getTotalItemCount() const {
    int count = 0;
    for (int i = 0; i < items.getSize(); ++i) {
        count += items[i].getQuantity();
    }
    return count;
}

// Hàm có chức năng dọn sạch giỏ hàng
inline void Cart::clear() {
    items.clear();
}

// Hàm trả về liệu giỏ hàng có rỗng không
inline bool Cart::isEmpty() const {
    return items.isEmpty();
}

#endif // CART_H
