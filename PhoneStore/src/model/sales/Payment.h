#pragma once
// Thông tin thanh toán của một đơn (phương thức + trạng thái + số tiền + thời điểm thu).
// Lớp mới của bộ khung: chưa có trong code cũ, chỉ giữ dữ liệu, chưa có nghiệp vụ.

#include "../enums/PaymentMethod.h"
#include "../enums/PaymentStatus.h"
#include "../../common/DateTime.h"
#include "../../common/Money.h"

class Payment {
private:
    PaymentMethod method;  // Phương thức thanh toán
    PaymentStatus status;  // UNPAID / PAID
    Money amount;          // Số tiền cần thu (VND)
    DateTime paidTime;     // Thời điểm thu tiền (chỉ có nghĩa khi PAID)

public:
    Payment();
    Payment(PaymentMethod method, Money amount);

    PaymentMethod getMethod() const;
    PaymentStatus getStatus() const;
    Money getAmount() const;
    DateTime getPaidTime() const;

    void setMethod(PaymentMethod m);
    void setStatus(PaymentStatus s);
    void setAmount(Money a);
    void setPaidTime(const DateTime& t);
};
