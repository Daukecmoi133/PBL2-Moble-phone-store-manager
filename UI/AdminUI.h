#ifndef ADMINUI_H
#define ADMINUI_H

#include "../source/PBL2.h"
#include "UICommon.h"

class AdminUI {
private:
    PhoneRepository& phoneRepo;
    EmployeeRepository& employeeRepo;
    DiscountRepository& discountRepo;
    OrderRepository& orderRepo;
    AdminEmployee* currentAdmin;

public:
    AdminUI(PhoneRepository& pRepo, EmployeeRepository& eRepo, 
            DiscountRepository& dRepo, OrderRepository& oRepo,
            AdminEmployee* admin = nullptr)
        : phoneRepo(pRepo), employeeRepo(eRepo), discountRepo(dRepo), 
          orderRepo(oRepo), currentAdmin(admin) {}

    void setCurrentAdmin(AdminEmployee* admin) { currentAdmin = admin; }

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

#endif // ADMINUI_H
