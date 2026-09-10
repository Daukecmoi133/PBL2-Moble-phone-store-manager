#ifndef CART_H
#define CART_H

#include <string>
#include "Phone.h"
#include "../core/Vector.h"

class CartItem {
private:
    Phone phone;
    int quantity;
    double unitPrice;

public:
    CartItem() : phone(), quantity(0), unitPrice(0.0) {}

    CartItem(const Phone& phone, int quantity)
        : phone(phone), quantity(quantity), unitPrice(phone.getSellingPrice()) {}

    Phone getPhone() const { return phone; }
    void setPhone(const Phone& p) { phone = p; }

    int getQuantity() const { return quantity; }
    void setQuantity(int q) { quantity = q; }

    double getUnitPrice() const { return unitPrice; }
    void setUnitPrice(double price) { unitPrice = price; }

    double getSubtotal() const {
        return unitPrice * quantity;
    }

    void increaseQuantity(int amount) {
        if (amount > 0) quantity += amount;
    }
};

class Cart {
private:
    std::string customerId;
    Vector<CartItem> items;

public:
    Cart(std::string custId = "") : customerId(custId) {}

    std::string getCustomerId() const { return customerId; }
    void setCustomerId(const std::string& id) { customerId = id; }

    const Vector<CartItem>& getItems() const { return items; }
    Vector<CartItem>& getItems() { return items; }

    void addItem(const Phone& phone, int quantity) {
        if (quantity <= 0) return;

        for (int i = 0; i < items.getSize(); ++i) {
            if (items[i].getPhone().getID() == phone.getID()) {
                items[i].increaseQuantity(quantity);
                return;
            }
        }
        items.push_back(CartItem(phone, quantity));
    }

    bool removeItem(const std::string& phoneId) {
        for (int i = 0; i < items.getSize(); ++i) {
            if (items[i].getPhone().getID() == phoneId) {
                return items.removeAt(i);
            }
        }
        return false;
    }

    bool updateQuantity(const std::string& phoneId, int newQuantity) {
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

    double calculateTotal() const {
        double total = 0.0;
        for (int i = 0; i < items.getSize(); ++i) {
            total += items[i].getSubtotal();
        }
        return total;
    }

    int getTotalItemCount() const {
        int count = 0;
        for (int i = 0; i < items.getSize(); ++i) {
            count += items[i].getQuantity();
        }
        return count;
    }

    void clear() {
        items.clear();
    }

    bool isEmpty() const {
        return items.isEmpty();
    }
};

#endif // CART_H
