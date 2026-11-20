#ifndef WARRANTYSERVICE_H
#define WARRANTYSERVICE_H

#include "../core/Vector.h"
#include "../models/Common.h"
#include "../models/Invoice.h"
#include "../models/Phone.h"
#include "../models/Warranty.h"
#include "../models/WarrantyRequest.h"
#include "../repositories/CustomerRepository.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/OrderRepository.h"
#include "../repositories/PhoneRepository.h"
#include "../repositories/WarrantyRequestRepository.h"
#include "../utils/DateTime.h"
#include "../utils/IdGenerator.h"

/**
 * @class WarrantyService
 * @brief Tầng xử lý nghiệp vụ bảo hành sản phẩm (Product Warranty Business
 * Service)
 *
 * Nguyên tắc thiết kế:
 * - Không kiểm tra vai trò người dùng (thuộc trách nhiệm của Controller).
 * - Không gọi RealTimeManager.
 * - Không tự ý lấy giờ hệ thống (nhận tham số DateTime now từ bên ngoài).
 * - Chỉ đọc từ 4 repository: OrderRepository, InvoiceRepository,
 * PhoneRepository, CustomerRepository.
 * - Ghi nhận trạng thái và phiếu qua WarrantyRequestRepository.
 */
class WarrantyService {
public:
  struct Policy {
    int defaultMonths = 12;
  };

  struct WarrantyCoverage {
    std::string orderId;
    std::string invoiceId;
    std::string phoneId;
    std::string phoneName;
    std::string customerId;
    int quantity = 0;
    DateTime startDate;
    DateTime expireDate;
    int remainingDays = 0;
    bool active = false;
    int openRequests = 0;
  };


private:
  WarrantyRequestRepository &requestRepo;
  const OrderRepository &orderRepo;
  const InvoiceRepository &invoiceRepo;
  const PhoneRepository &phoneRepo;
  const CustomerRepository &customerRepo;
  Policy policy;

  // Thuật toán tính chênh lệch số ngày giữa 2 mốc thời gian (Civil Days)
  static int daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int>(doe) - 719468;
  }

  static int calculateRemainingDays(const DateTime &fromDate,
                                    const DateTime &toDate) {
    return daysFromCivil(toDate.year, toDate.month, toDate.day) -
           daysFromCivil(fromDate.year, fromDate.month, fromDate.day);
  }


public:
  WarrantyService(WarrantyRequestRepository &reqRepo,
                  const OrderRepository &ordRepo,
                  const InvoiceRepository &invRepo,
                  const PhoneRepository &phnRepo,
                  const CustomerRepository &custRepo)
      : requestRepo(reqRepo), orderRepo(ordRepo), invoiceRepo(invRepo),
        phoneRepo(phnRepo), customerRepo(custRepo) {}

  const Policy &getPolicy() const { return policy; }
  void setPolicy(const Policy &p) { policy = p; }

    const Vector<WarrantyRequest> &getAll() const {
      return requestRepo.getAll();
    }
    WarrantyRequest *getById(const std::string &id) {
      return requestRepo.findById(id);
    }
    const WarrantyRequest *getById(const std::string &id) const {
      return requestRepo.findById(id);
    }
    Vector<WarrantyRequest> getByCustomerId(const std::string &customerId)
        const {
      return requestRepo.findByCustomerId(customerId);
    }
  void reload() { requestRepo.loadFromFile(); }

  /**
   * @brief Kiểm tra và lấy thông tin bảo hành (coverage) của một sản phẩm trong
   * đơn hàng
   */
  bool getCoverage(const std::string &orderId, const std::string &phoneId,
                   const DateTime &now, WarrantyCoverage &out, std::string &err)
      const {
    const Order *order = orderRepo.findById(orderId);
    if (!order) {
      err = "Đơn hàng không tồn tại: " + orderId;
      return false;
    }

    if (order->getStatus() != OrderStatus::COMPLETED) {
      err = "Đơn hàng " + orderId + " chưa hoàn tất (COMPLETED)!";
      return false;
    }

    const Vector<OrderDetail> &details = order->getDetails();
    const OrderDetail *targetDetail = nullptr;
    for (int i = 0; i < details.getSize(); ++i) {
      if (details[i].getPhoneId() == phoneId) {
        targetDetail = &details[i];
        break;
      }
    }

    if (!targetDetail) {
      err = "Đơn hàng " + orderId + " không chứa sản phẩm " + phoneId;
      return false;
    }

    out.orderId = orderId;
    out.phoneId = phoneId;
    out.phoneName = targetDetail->getPhoneName();
    if (out.phoneName.empty()) {
      const Phone *phn = phoneRepo.findById(phoneId);
      if (phn)
        out.phoneName = phn->getName();
    }
    out.customerId = order->getCustomerId();
    out.quantity = targetDetail->getQuantity();

    // Mốc bắt đầu = invoice.createTime nếu có hóa đơn của đơn, ngược lại
    // order.orderTime
    const Invoice *invoice = invoiceRepo.findByOrderId(orderId);
    if (invoice) {
      out.invoiceId = invoice->getID();
      out.startDate = invoice->getCreateTime();
    } else {
      out.invoiceId = "";
      out.startDate = order->getOrderTime();
    }

    out.expireDate = out.startDate.addMonths(policy.defaultMonths);
    int diff = calculateRemainingDays(now, out.expireDate);
    out.remainingDays = (diff >= 0 ? diff : 0);
    out.active = (now <= out.expireDate);

    out.openRequests = requestRepo.countOpenByOrderAndPhone(orderId, phoneId);

        return true;
      }

      /**
       * @brief Liệt kê danh sách các sản phẩm đủ điều kiện bảo hành của một
       * khách hàng
       */
      Vector<WarrantyCoverage> listEligibleForCustomer(
          const std::string &customerId, const DateTime &now) const {
        Vector<WarrantyCoverage> result;
        Vector<Order> custOrders = orderRepo.findByCustomerId(customerId);

          for (int i = 0; i < custOrders.getSize(); ++i) {
            const Order &ord = custOrders[i];
            if (ord.getStatus() == OrderStatus::COMPLETED) {
              const Vector<OrderDetail> &details = ord.getDetails();
              for (int j = 0; j < details.getSize(); ++j) {
                WarrantyCoverage cov;
                std::string err;
                if (getCoverage(ord.getID(), details[j].getPhoneId(), now, cov,
                                err)) {
                  result.push_back(cov);
                }
              }
            }
          }
          return result;
        }

        /**
         * @brief Tạo mới yêu cầu bảo hành, tuân thủ đúng 5 điều kiện nghiệp vụ
         */
        bool createRequest(
            const std::string &customerId, const std::string &orderId,
            const std::string &phoneId, const std::string &issue,
            const DateTime &now, std::string &outId, std::string &err) {
          // Điều kiện 1: Đơn tồn tại + COMPLETED + chứa phoneId
          WarrantyCoverage cov;
          if (!getCoverage(orderId, phoneId, now, cov, err)) {
            return false;
          }

          // Điều kiện 2: Đơn thuộc customerId (đơn không có khách thì chỉ cho
          // customerId rỗng)
          if (cov.customerId.empty()) {
            if (!customerId.empty()) {
              err = "Đơn hàng này là đơn hàng vãng lai (không gắn khách hàng), "
                    "không thể gán cho khách hàng: " +
                    customerId;
              return false;
            }
          } else {
            if (cov.customerId != customerId) {
              err = "Đơn hàng " + orderId + " không thuộc khách hàng " +
                    customerId + " (thuộc " + cov.customerId + ")!";
              return false;
            }
          }

          // Điều kiện 3: now <= expireDate
          if (now > cov.expireDate) {
            err = "Sản phẩm đã hết hạn bảo hành vào ngày " +
                  cov.expireDate.toDateString() + "!";
            return false;
          }

          // Điều kiện 4: openRequests < quantity
          if (cov.openRequests >= cov.quantity) {
            err = "Số lượng yêu cầu bảo hành đang mở (" +
                  std::to_string(cov.openRequests) +
                  ") đã đạt tối đa số lượng sản phẩm mua trong đơn (" +
                  std::to_string(cov.quantity) + ")!";
            return false;
          }

          // Điều kiện 5: Mô tả sự cố sau sanitize không rỗng
          std::string cleanIssue = WarrantyRequest::sanitizeText(issue, 500);
          if (cleanIssue.empty()) {
            err = "Mô tả sự cố không được để trống!";
            return false;
          }

          // Sinh ID bằng IdGenerator::nextId("WREQ", ngày của now dạng
          // ddmmyyyy, ...)
          char dateBuf[16];
          std::snprintf(dateBuf, sizeof(dateBuf), "%02d%02d%04d", now.day,
                        now.month, now.year);
          std::string dateKey(dateBuf);

        std::vector<std::string> existingIds;
        const Vector<WarrantyRequest> &allReqs = requestRepo.getAll();
        for (int i = 0; i < allReqs.getSize(); ++i) {
          existingIds.push_back(allReqs[i].getId());
        }

          std::string newId;
          if (!IdGenerator::nextId("WREQ", dateKey, existingIds, newId, err)) {
            return false;
          }

          WarrantyRequest newReq(newId, orderId, phoneId, cov.phoneName,
                                 customerId, now, WarrantyStatus::RECEIVED,
                                 cleanIssue, "", "", now);

          outId = newId;
          if (!requestRepo.addRequest(newReq)) {
            err = "Lỗi khi lưu phiếu yêu cầu bảo hành vào cơ sở dữ liệu!";
            return false;
          }

            return true;
          }

          /**
           * @brief Cập nhật trạng thái phiếu bảo hành theo máy trạng thái
           * (State Machine)
           */
          bool updateStatus(const std::string &id, WarrantyStatus next,
                            const std::string &technicianNote,
                            const std::string &resolution, const DateTime &now,
                            std::string &err) {
            WarrantyRequest *req = requestRepo.findById(id);
            if (!req) {
              err = "Không tìm thấy yêu cầu bảo hành ID: " + id;
              return false;
            }

            WarrantyStatus current = req->getStatus();

            // 1. Trạng thái cuối bất biến
            if (current == WarrantyStatus::RESOLVED) {
              err = "Yêu cầu bảo hành " + id +
                    " đã ở trạng thái HOÀN TẤT (RESOLVED), không thể thay đổi!";
              return false;
            }
            if (current == WarrantyStatus::REJECTED) {
              err = "Yêu cầu bảo hành " + id +
                    " đã bị TỪ CHỐI (REJECTED), không thể thay đổi!";
              return false;
            }

            // 2. Chuyển trạng thái hợp lệ
            // RECEIVED -> PROCESSING | REJECTED
            if (current == WarrantyStatus::RECEIVED) {
              if (next != WarrantyStatus::PROCESSING &&
                  next != WarrantyStatus::REJECTED) {
                err = "Chuyển trạng thái không hợp lệ: Từ Mới tiếp nhận "
                      "(RECEIVED) chỉ có thể chuyển sang Đang xử lý "
                      "(PROCESSING) hoặc Từ chối (REJECTED)!";
                return false;
              }
            }
            // PROCESSING -> RESOLVED | REJECTED
            else if (current == WarrantyStatus::PROCESSING) {
              if (next != WarrantyStatus::RESOLVED &&
                  next != WarrantyStatus::REJECTED) {
                err = "Chuyển trạng thái không hợp lệ: Từ Đang xử lý "
                      "(PROCESSING) chỉ có thể chuyển sang Đã hoàn tất "
                      "(RESOLVED) hoặc Từ chối (REJECTED)!";
                return false;
              }
            }

          // 3. RESOLVED / REJECTED bắt buộc ghi chú hoặc resolution không rỗng
          std::string cleanNote =
              WarrantyRequest::sanitizeText(technicianNote, 500);
          std::string cleanResolution =
              WarrantyRequest::sanitizeText(resolution, 500);

            if (next == WarrantyStatus::RESOLVED ||
                next == WarrantyStatus::REJECTED) {
              if (cleanNote.empty() && cleanResolution.empty()) {
                err = "Khi hoàn tất hoặc từ chối bảo hành, bắt buộc phải có "
                      "ghi chú kỹ thuật hoặc kết quả giải quyết!";
                return false;
              }
            }

            // Cập nhật đối tượng
            WarrantyRequest updated = *req;
            updated.setStatus(next);
            if (!cleanNote.empty()) {
              updated.setTechnicianNote(cleanNote);
            }
            if (!cleanResolution.empty()) {
              updated.setResolution(cleanResolution);
            }
            updated.setUpdatedAt(now);

              if (!requestRepo.updateRequest(updated)) {
                err = "Lỗi khi cập nhật yêu cầu bảo hành " + id +
                      " vào cơ sở dữ liệu!";
                return false;
              }

              return true;
            }
          };

#endif // WARRANTYSERVICE_H
