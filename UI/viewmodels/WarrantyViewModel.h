#ifndef WARRANTY_VIEWMODEL_H
#define WARRANTY_VIEWMODEL_H

#include "../../source/models/Common.h"
#include <string>

// ViewModel hiển thị một yêu cầu bảo hành
struct WarrantyItemViewModel {
  std::string id;
  std::string orderId;
  std::string invoiceId;
  std::string phoneId;
  std::string phoneName;
  std::string customerId;
  std::string customerName;
  std::string customerPhone;
  std::string requestDate;
  std::string issueDescription;
  WarrantyStatus status;
  std::string statusText;
  std::string technicianNote;
  std::string resolution;
  std::string expireDate;
  int remainingDays;
  std::string createdAt;
  std::string updatedAt;

  WarrantyItemViewModel()
      : status(WarrantyStatus::RECEIVED), remainingDays(0) {}
};

// ViewModel cho sản phẩm đủ điều kiện bảo hành của khách hàng
struct WarrantyEligibleItemViewModel {
  std::string orderId;
  std::string invoiceId;
  std::string phoneId;
  std::string phoneName;
  int quantity = 0;
  std::string purchaseDate;
  std::string expireDate;
  int remainingDays = 0;
  bool active = false;
  int openRequests = 0;
};

// Form tạo yêu cầu bảo hành
struct CreateWarrantyRequest {
  std::string customerId;
  std::string orderId;
  std::string phoneId;
  std::string issueDescription;
};

#endif // WARRANTY_VIEWMODEL_H
