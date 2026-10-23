#ifndef ADMINUI_H
#define ADMINUI_H

#include "../source/PBL2.h"
#include "UICommon.h"

class AdminUI {
private:
    PhoneService& phoneService;
    EmployeeService& employeeService;
    DiscountService& discountService;
    OrderService& orderService;
    AdminEmployee* currentAdmin;

public:
    AdminUI(PhoneService& pService, EmployeeService& eService, 
            DiscountService& dService, OrderService& oService,
            AdminEmployee* admin = nullptr);

    void setCurrentAdmin(AdminEmployee* admin);

    void showMenu();
    void manageProducts();
    void addNewProduct();
    void updateProduct();
    void deleteProduct();
    void manageEmployees();
    void addNewEmployee();
    void viewAllEmployees();
    void manageDiscounts();
    void viewRevenueStatistics();
};

// ================= IMPLEMENTATION =================

inline AdminUI::AdminUI(PhoneService& pService, EmployeeService& eService, 
                        DiscountService& dService, OrderService& oService,
                        AdminEmployee* admin)
    : phoneService(pService), employeeService(eService), discountService(dService), 
      orderService(oService), currentAdmin(admin) {}

inline void AdminUI::setCurrentAdmin(AdminEmployee* admin) { 
    currentAdmin = admin; 
}

#endif // ADMINUI_H
