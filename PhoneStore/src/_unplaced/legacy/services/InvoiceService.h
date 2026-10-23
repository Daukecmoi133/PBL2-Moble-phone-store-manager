// INVOICE SERVICE - Tầng nghiệp vụ quản lý hóa đơn thanh toán
// InvoiceService tạo hóa đơn từ đơn hàng đã xác nhận, liên kết khách hàng và nhân viên.
// Business rule: Mỗi đơn hàng chỉ được có tối đa 1 hóa đơn (one invoice per order).
#ifndef INVOICESERVICE_H
#define INVOICESERVICE_H

#include <iostream>
#include <string>
#include "../models/Invoice.h"
#include "../models/Order.h"
#include "../core/Vector.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/OrderRepository.h"
#include "../utils/StringHelper.h"
#include "Check.h"

// Tầng Service: Nghiệp vụ tạo, tra cứu và quản lý hóa đơn thanh toán.
// Đảm bảo: một đơn hàng chỉ có một hóa đơn; chỉ tạo được từ đơn CONFIRMED/COMPLETED.
class InvoiceService {
private:
    InvoiceRepository& invoiceRepo;
    OrderRepository& orderRepo;

    // Tạo ID hóa đơn tự động theo format INV-XXXX
    std::string generateInvoiceId() const;

public:
    InvoiceService(InvoiceRepository& iRepo, OrderRepository& oRepo);

    const Vector<Invoice>& getAll() const;

    Invoice* getById(const std::string& invoiceId);
    Invoice* getByOrderId(const std::string& orderId);

    // Lọc hóa đơn theo khách hàng / nhân viên
    Vector<Invoice> getByCustomerId(const std::string& customerId) const;
    Vector<Invoice> getByEmployeeId(const std::string& employeeId) const;

    // ==== NGHIỆP VỤ CỐT LÕI ====

    // Tạo hóa đơn từ đơn hàng đã xác nhận
    // Business rules:
    //   - orderId phải tồn tại và ở trạng thái CONFIRMED/COMPLETED
    //   - Mỗi orderId chỉ được xuất 1 hóa đơn (one invoice per order)
    //   - employeeId, paymentMethod phải hợp lệ
    //   - discountAmount không được âm và không vượt subtotal
    bool createInvoiceFromOrder(const std::string& orderId,
                                const std::string& employeeId,
                                PaymentMethod payMethod,
                                double discountAmount,
                                std::string& errorMessage);

    bool createInvoiceFromOrder(const std::string& orderId,
                                const std::string& employeeId,
                                PaymentMethod payMethod,
                                double discountAmount);

    // Xóa hóa đơn (hiếm khi dùng, cần quyền Admin)
    bool deleteInvoice(const std::string& invoiceId, std::string& errorMessage);
};

// ================= IMPLEMENTATION =================

// Tự sinh ID hóa đơn theo format INV-XXXX (số tăng dần)
inline std::string InvoiceService::generateInvoiceId() const {
    int maxNum = 0;
    const Vector<Invoice>& list = invoiceRepo.getAll();
    for (int i = 0; i < list.getSize(); ++i) {
        const std::string& id = list[i].getID();
        // Format: INV-XXXX → lấy phần số sau "INV-"
        if (id.size() > 4 && id.substr(0, 4) == "INV-") {
            try {
                int num = std::stoi(id.substr(4));
                if (num > maxNum) maxNum = num;
            } catch (...) {}
        }
    }
    // Tạo ID mới với số tiếp theo, padding 3 chữ số
    std::string numStr = std::to_string(maxNum + 1);
    while (numStr.size() < 3) numStr = "0" + numStr;
    return "INV-" + numStr;
}

inline InvoiceService::InvoiceService(InvoiceRepository& iRepo, OrderRepository& oRepo)
    : invoiceRepo(iRepo), orderRepo(oRepo) {}

inline const Vector<Invoice>& InvoiceService::getAll() const {
    return invoiceRepo.getAll();
}

inline Invoice* InvoiceService::getById(const std::string& invoiceId) {
    return invoiceRepo.findById(invoiceId);
}

inline Invoice* InvoiceService::getByOrderId(const std::string& orderId) {
    return invoiceRepo.findByOrderId(orderId);
}

inline Vector<Invoice> InvoiceService::getByCustomerId(const std::string& customerId) const {
    return invoiceRepo.findByCustomerId(customerId);
}

inline Vector<Invoice> InvoiceService::getByEmployeeId(const std::string& employeeId) const {
    return invoiceRepo.findByEmployeeId(employeeId);
}

// Tạo hóa đơn từ đơn hàng đã xác nhận
// Business rules được kiểm tra theo thứ tự:
//  1. orderId không rỗng, tồn tại trong hệ thống
//  2. Trạng thái đơn phải là CONFIRMED hoặc COMPLETED (đã xử lý)
//  3. Chưa có hóa đơn nào cho orderId này (one invoice per order)
//  4. employeeId không rỗng
//  5. discountAmount không âm, không vượt subtotal đơn hàng
inline bool InvoiceService::createInvoiceFromOrder(
        const std::string& orderId,
        const std::string& employeeId,
        PaymentMethod payMethod,
        double discountAmount,
        std::string& errorMessage)
{
    // R1: orderId hợp lệ
    if (orderId.empty()) {
        errorMessage = "Ma don hang khong duoc de trong!";
        return false;
    }

    Order* order = orderRepo.findById(orderId);
    if (order == nullptr) {
        errorMessage = "Khong tim thay don hang [" + orderId + "]!";
        return false;
    }

    // R2: Trạng thái đơn phải phù hợp
    if (order->getStatus() != OrderStatus::CONFIRMED &&
        order->getStatus() != OrderStatus::COMPLETED) {
        errorMessage = "Chi co the xuat hoa don cho don hang da xac nhan hoac hoan tat!";
        return false;
    }

    // R3: Kiểm tra đơn hàng chưa có hóa đơn
    if (invoiceRepo.findByOrderId(orderId) != nullptr) {
        errorMessage = "Don hang [" + orderId + "] da duoc xuat hoa don truoc do!";
        return false;
    }

    // R4: employeeId không rỗng
    if (employeeId.empty()) {
        errorMessage = "Ma nhan vien khong duoc de trong!";
        return false;
    }

    // R5: discountAmount hợp lệ
    if (discountAmount < 0) {
        errorMessage = "So tien chiet khau khong duoc am!";
        return false;
    }

    double subtotal = order->calculateTotal();
    if (discountAmount > subtotal) {
        errorMessage = "So tien chiet khau khong the vuot gia tri don hang!";
        return false;
    }

    // Tạo hóa đơn từ đơn hàng
    std::string invId = generateInvoiceId();
    Invoice inv(invId,
                orderId,
                order->getCustomerId(),
                employeeId,
                payMethod,
                order->getDetails(),
                discountAmount);
    inv.setCreateTime(DateTime::now());

    if (!invoiceRepo.addInvoice(inv)) {
        errorMessage = "Loi khi luu hoa don vao co so du lieu!";
        return false;
    }

    errorMessage = "Xuat hoa don [" + invId + "] thanh cong!";
    return true;
}

inline bool InvoiceService::createInvoiceFromOrder(
        const std::string& orderId,
        const std::string& employeeId,
        PaymentMethod payMethod,
        double discountAmount)
{
    std::string err;
    return createInvoiceFromOrder(orderId, employeeId, payMethod, discountAmount, err);
}

// Xóa hóa đơn (tối thiểu, cần xem xét policy nghiệp vụ khi triển khai thực tế)
inline bool InvoiceService::deleteInvoice(const std::string& invoiceId, std::string& errorMessage) {
    if (invoiceRepo.findById(invoiceId) == nullptr) {
        errorMessage = "Khong tim thay hoa don [" + invoiceId + "] de xoa!";
        return false;
    }
    if (!invoiceRepo.deleteInvoice(invoiceId)) {
        errorMessage = "Loi khi xoa hoa don!";
        return false;
    }
    errorMessage = "Xoa hoa don thanh cong!";
    return true;
}

#endif // INVOICESERVICE_H
