#ifndef CUSTOMERUI_H
#define CUSTOMERUI_H

#include "../source/PBL2.h"
#include "UICommon.h"

class CustomerUI {
private:
    PhoneRepository& phoneRepo;
    OrderRepository& orderRepo;
    DiscountRepository& discountRepo;
    Customer* currentCustomer;
    Cart cart;

public:
    CustomerUI(PhoneRepository& pRepo, OrderRepository& oRepo, 
               DiscountRepository& dRepo, Customer* cust = nullptr)
        : phoneRepo(pRepo), orderRepo(oRepo), discountRepo(dRepo), 
          currentCustomer(cust), cart(cust ? cust->getID() : "") {}

    void setCurrentCustomer(Customer* cust) { 
        currentCustomer = cust; 
        if (cust) cart.setCustomerId(cust->getID());
    }

    void showMenu();
    void browseAllPhones();
    void searchPhones();
    void filterPhonesByPrice(); // Lọc điện thoại bằng BST findRange O(log N)
    void viewCart();
    void addToCart();
    void checkout();
    void viewOrderHistory();
};

#endif // CUSTOMERUI_H
