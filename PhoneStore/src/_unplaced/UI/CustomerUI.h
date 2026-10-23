#ifndef CUSTOMERUI_H
#define CUSTOMERUI_H

#include "../source/PBL2.h"
#include "UICommon.h"

class CustomerUI {
private:
    PhoneService& phoneService;
    OrderService& orderService;
    DiscountService& discountService;
    CustomerService& customerService;
    Customer* currentCustomer;
    Cart cart;

public:
    CustomerUI(PhoneService& pService, OrderService& oService, 
               DiscountService& dService, CustomerService& cService, 
               Customer* cust = nullptr);

    void setCurrentCustomer(Customer* cust);

    void showMenu();
    void browseAllPhones();
    void searchPhones();
    void filterPhonesByPrice(); // Lọc điện thoại bằng BST findRange O(log N)
    void viewCart();
    void addToCart();
    void checkout();
    void viewOrderHistory();
};

// ================= IMPLEMENTATION =================

inline CustomerUI::CustomerUI(PhoneService& pService, OrderService& oService, 
                              DiscountService& dService, CustomerService& cService, 
                              Customer* cust)
    : phoneService(pService), orderService(oService), discountService(dService), 
      customerService(cService), currentCustomer(cust), 
      cart(cust ? cust->getID() : "") {}

inline void CustomerUI::setCurrentCustomer(Customer* cust) { 
    currentCustomer = cust; 
    if (cust) cart.setCustomerId(cust->getID());
}

#endif // CUSTOMERUI_H
