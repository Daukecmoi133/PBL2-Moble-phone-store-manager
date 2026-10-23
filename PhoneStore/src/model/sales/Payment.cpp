#include "Payment.h"

Payment::Payment() : method(PaymentMethod::CASH), status(PaymentStatus::UNPAID), amount(0), paidTime() {}

Payment::Payment(PaymentMethod m, Money a)
    : method(m), status(PaymentStatus::UNPAID), amount(a), paidTime() {}

PaymentMethod Payment::getMethod() const { return method; }
PaymentStatus Payment::getStatus() const { return status; }
Money Payment::getAmount() const { return amount; }
DateTime Payment::getPaidTime() const { return paidTime; }

void Payment::setMethod(PaymentMethod m) { method = m; }
void Payment::setStatus(PaymentStatus s) { status = s; }
void Payment::setAmount(Money a) { amount = a; }
void Payment::setPaidTime(const DateTime& t) { paidTime = t; }
