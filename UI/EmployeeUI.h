#ifndef EMPLOYEEUI_H
#define EMPLOYEEUI_H

#include "../source/PBL2.h"
#include "UICommon.h"

class EmployeeUI {
private:
    PhoneRepository& phoneRepo;
    OrderRepository& orderRepo;
    Employee* currentEmployee;

public:
    EmployeeUI(PhoneRepository& pRepo, OrderRepository& oRepo, Employee* emp = nullptr)
        : phoneRepo(pRepo), orderRepo(oRepo), currentEmployee(emp) {}

    void setCurrentEmployee(Employee* emp) { currentEmployee = emp; }

    void showMenu();
    void viewInventory();
    void searchProducts();
    void processOrdersQueue(); // Duyệt đơn hàng bằng Queue FIFO
    void importStock();        // Nhập thêm số lượng tồn kho
};

#endif // EMPLOYEEUI_H
