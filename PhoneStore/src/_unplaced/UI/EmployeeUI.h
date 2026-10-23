#ifndef EMPLOYEEUI_H
#define EMPLOYEEUI_H

#include "../source/PBL2.h"
#include "UICommon.h"

class EmployeeUI {
private:
    PhoneService& phoneService;
    OrderService& orderService;
    Employee* currentEmployee;

public:
    EmployeeUI(PhoneService& pService, OrderService& oService, Employee* emp = nullptr);

    void setCurrentEmployee(Employee* emp);

    void showMenu();
    void viewInventory();
    void searchProducts();
    void processOrdersQueue(); // Duyệt đơn hàng bằng Queue FIFO
    void importStock();        // Nhập thêm số lượng tồn kho
};

// ================= IMPLEMENTATION =================

inline EmployeeUI::EmployeeUI(PhoneService& pService, OrderService& oService, Employee* emp)
    : phoneService(pService), orderService(oService), currentEmployee(emp) {}

inline void EmployeeUI::setCurrentEmployee(Employee* emp) { 
    currentEmployee = emp; 
}

#endif // EMPLOYEEUI_H
