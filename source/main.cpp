#include <iostream>
#include "PBL2.h"
#include "../UI/UICommon.h"

using namespace std;

int main() {
    // Tối ưu nhập xuất luồng console
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    UICommon::printHeader("HE THONG QUAN LY CUA HANG DIEN THOAI - PBL2");
    cout << "[+] Khoi tao cac cau truc du lieu tu viet (Custom DSA Templates)...\n";

    // 1. Khoi tao cac Repositories
    PhoneRepository phoneRepo("data/Device/Products.txt");
    EmployeeRepository employeeRepo("data/people/Employee.txt");
    CustomerRepository customerRepo("data/people/Customers.txt");
    DiscountRepository discountRepo("data/PAY/Discount.txt");
    OrderRepository orderRepo("data/PAY/Order/");

    // 2. Nap du lieu tu tep tin
    cout << "[+] Dang nap du lieu tu thu muc data/...\n";

    if (phoneRepo.loadFromFile()) {
        cout << "  -> Nap thanh cong " << phoneRepo.getAll().getSize() 
             << " san pham vao Vector, HashTable O(1) va BST O(log N).\n";
    } else {
        cout << "  [!] Khong the nap du lieu san pham.\n";
    }

    if (employeeRepo.loadFromFile()) {
        cout << "  -> Nap thanh cong " << employeeRepo.getAll().getSize() 
             << " nhan vien vao Vector va HashTable O(1).\n";
    } else {
        cout << "  [!] Khong the nap du lieu nhan vien.\n";
    }

    if (customerRepo.loadFromFile()) {
        cout << "  -> Nap thanh cong " << customerRepo.getAll().getSize() 
             << " khach hang.\n";
    }

    if (discountRepo.loadFromFile()) {
        cout << "  -> Nap thanh cong " << discountRepo.getAll().getSize() 
             << " ma giam gia.\n";
    }

    // 3. Demo kiem tra tinh dung dan cua cac cau truc du lieu
    cout << "\n================ KIEM TRA HE THONG CAU TRUC DU LIEU ================\n";

    // A. Kiem tra bang bam HashTable O(1) tim kiem san pham theo ID
    string testPhoneId = "101";
    Phone* p = phoneRepo.findById(testPhoneId);
    if (p) {
        cout << "[HashTable] Tim kiem san pham ID " << testPhoneId << ":\n";
        cout << "  - Ten: " << p->getName() << "\n";
        cout << "  - Hang: " << p->getBrand() << "\n";
        cout << "  - He dieu hanh: " << p->getOperatingSystem() << "\n";
        cout << "  - Gia ban: " << StringHelper::formatCurrency(p->getSellingPrice()) << "\n";
        cout << "  - Ton kho: " << p->getQuantity() << " chiec\n";
    }

    // B. Kiem tra cay nhi phan tim kiem BST O(log N) loc san pham theo khoang gia
    double minPrice = 10000000.0;
    double maxPrice = 20000000.0;
    Vector<Phone> filtered = phoneRepo.findByPriceRange(minPrice, maxPrice);
    cout << "\n[BST Tree] Loc san pham trong khoang gia tu " 
         << StringHelper::formatCurrency(minPrice) << " den " 
         << StringHelper::formatCurrency(maxPrice) << ":\n";
    cout << "  -> Tim thay " << filtered.getSize() << " san pham phu hop:\n";
    UICommon::printPhoneTableHeader();
    for (int i = 0; i < filtered.getSize(); ++i) {
        UICommon::printPhoneRow(
            filtered[i].getID(),
            filtered[i].getName(),
            filtered[i].getBrand(),
            StringHelper::formatCurrency(filtered[i].getSellingPrice()),
            filtered[i].getQuantity()
        );
    }
    UICommon::printLine(74, '-');

    // C. Kiem tra xac thuc dang nhap qua HashTable
    cout << "\n[Authentication] Kiem tra xac thuc tai khoan:\n";
    Employee* admin = employeeRepo.authenticate("admin@", "123456");
    if (admin) {
        cout << "  -> Dang nhap thanh cong: " << admin->getFullName() 
             << " (Vai tro: " << admin->getRole() << ")\n";
    } else {
        cout << "  -> Dang nhap that bai!\n";
    }

    // D. Kiem tra Queue FIFO xu ly don hang
    cout << "\n[Queue FIFO] Kiem tra hang doi don hang:\n";
    Order testOrder("ORD-001", "CUST-01", DeliveryMethod::DELIVERY, PaymentMethod::CASH, 
                    ShippingInfo("Nguyen Van A", "0901234567", "Da Nang"));
    testOrder.addDetail(OrderDetail("101", "Oppo Reno 16f", 2, 14000000.0));
    orderRepo.addOrder(testOrder);
    cout << "  -> So luong don hang trong hang doi cho duyet: " 
         << orderRepo.getPendingQueue().getSize() << "\n";

    Order processed;
    if (orderRepo.processNextPendingOrder(processed)) {
        cout << "  -> Da duyet thanh cong don hang: " << processed.getID() 
             << " voi tong tien: " << StringHelper::formatCurrency(processed.calculateTotal()) << "\n";
    }

    cout << "\n================ HE THONG HEADER & DSA DA SAN SANG! ================\n";
    return 0;
}