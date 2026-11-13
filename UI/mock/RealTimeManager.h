#ifndef REAL_TIME_MANAGER_H
#define REAL_TIME_MANAGER_H

#include "../../source/utils/PathHelper.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>


/**
 * @file RealTimeManager.h
 * @brief ĐẶC TẢ HỆ THỐNG KIỂM SOÁT THỜI GIAN THỰC GIẢ LẬP (REAL-TIME SIMULATION
 * SUBSYSTEM)
 *
 * =========================================================================================
 * 1. MỤC ĐÍCH NGHIỆP VỤ:
 *    - Giải quyết phản hồi: "Thiết kế thêm hệ thống kiểm soát thời gian thực...
 * tạo file realTime để lưu".
 *    - Lưu trữ thời gian mô phỏng tại `data/realTime.txt` để đối soát:
 *      + Ngày bắt đầu / ngày hết hạn của Voucher khuyến mãi (discountDate,
 * voucher validity).
 *      + Thời hạn bảo hành còn lại của linh kiện / thiết bị điện thoại.
 *      + Thống kê doanh thu theo khoảng thời gian giả lập.
 *    - Thiết kế mẫu Singleton (RealTimeManager::getInstance()) bảo đảm nhất
 * quán toàn cục.
 * =========================================================================================
 */
class RealTimeManager {
private:
  std::string currentSimulatedTime; // Format: "YYYY-MM-DD HH:MM:SS"

  std::string getFilePath() const {
    return PathHelper::resolveDataPath("data/realTime.txt");
  }

  RealTimeManager() { resetToSystemNow(); }

public:
  static RealTimeManager &getInstance() {
    static RealTimeManager instance;
    return instance;
  }

  // Tải thời gian thực từ file data/realTime.txt
  void loadFromFile() {
    std::ifstream file(getFilePath());
    if (file.is_open()) {
      std::string line;
      if (std::getline(file, line) && line.length() >= 10) {
        currentSimulatedTime = line;
        file.close();
        return;
      }
      file.close();
    }
    // Mặc định nếu chưa có file
    currentSimulatedTime = "2026-10-17 11:30:00";
    saveToFile();
  }

  // Lưu thời gian thực vào file data/realTime.txt
  void saveToFile() {
    std::ofstream file(getFilePath());
    if (file.is_open()) {
      file << currentSimulatedTime << std::endl;
      file.close();
    }
  }

  // Lấy chuỗi ngày giờ hiện tại: "YYYY-MM-DD HH:MM:SS"
  std::string getCurrentDateTime() const { return currentSimulatedTime; }

  // Lấy chuỗi ngày hiện tại: "YYYY-MM-DD"
  std::string getCurrentDate() const {
    if (currentSimulatedTime.length() >= 10) {
      return currentSimulatedTime.substr(0, 10);
    }
    return "2026-10-17";
  }

  // Lấy năm hiện tại của đồng hồ hệ thống
  int getCurrentYear() const {
    int y = 2026, m = 1, d = 1;
    if (sscanf(currentSimulatedTime.c_str(), "%d-%d-%d", &y, &m, &d) >= 1) {
      return y;
    }
    return 2026;
  }

  // Tăng thời gian thêm số giây (mặc định 1 giây cho đồng hồ thời gian thực)
  void advanceSeconds(int seconds = 1) {
    std::tm tmTime = {};
    int y = 2026, m = 10, d = 17, hh = 11, mm = 30, ss = 0;
    if (sscanf(currentSimulatedTime.c_str(), "%d-%d-%d %d:%d:%d", &y, &m, &d,
               &hh, &mm, &ss) >= 3) {
      tmTime.tm_year = y - 1900;
      tmTime.tm_mon = m - 1;
      tmTime.tm_mday = d;
      tmTime.tm_hour = hh;
      tmTime.tm_min = mm;
      tmTime.tm_sec = ss + seconds;
      tmTime.tm_isdst = -1;

      std::time_t nextTime = std::mktime(&tmTime);
      if (nextTime != (std::time_t)(-1)) {
        std::tm nextTm;
#ifdef _WIN32
        localtime_s(&nextTm, &nextTime);
#else
        localtime_r(&nextTime, &nextTm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&nextTm, "%Y-%m-%d %H:%M:%S");
        currentSimulatedTime = oss.str();
      }
    }
  }

  // Thiết lập thời gian giả lập mới
  void setDateTime(const std::string &dt) {
    if (dt.length() >= 10) {
      currentSimulatedTime = dt;
      if (currentSimulatedTime.length() == 10) {
        currentSimulatedTime += " 00:00:00";
      }
      saveToFile();
    }
  }

  // Đặt lại theo thời gian thực tế của máy tính
  void resetToSystemNow() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow;
#ifdef _WIN32
    localtime_s(&tmNow, &tt);
#else
    localtime_r(&tt, &tmNow);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S");
    currentSimulatedTime = oss.str();
    saveToFile();
  }

  // Tua nhanh / lùi lại một số ngày
  void advanceDays(int days) {
    std::tm tmTime = {};
    int y = 2026, m = 10, d = 17, hh = 11, mm = 30, ss = 0;
    if (sscanf(currentSimulatedTime.c_str(), "%d-%d-%d %d:%d:%d", &y, &m, &d,
               &hh, &mm, &ss) >= 3) {
      tmTime.tm_year = y - 1900;
      tmTime.tm_mon = m - 1;
      tmTime.tm_mday = d + days;
      tmTime.tm_hour = hh;
      tmTime.tm_min = mm;
      tmTime.tm_sec = ss;
      tmTime.tm_isdst = -1;

      std::time_t nextTime = std::mktime(&tmTime);
      if (nextTime != (std::time_t)(-1)) {
        std::tm nextTm;
#ifdef _WIN32
        localtime_s(&nextTm, &nextTime);
#else
        localtime_r(&nextTime, &nextTm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&nextTm, "%Y-%m-%d %H:%M:%S");
        currentSimulatedTime = oss.str();
        saveToFile();
      }
    }
  }

  // Chuyển đổi chuỗi ngày bất kỳ (YYYY-MM-DD hoặc DD/MM/YYYY) thành số ngày
  // epoch
  static long long dateToEpochDays(const std::string &dateStr) {
    int y = 0, m = 0, d = 0;
    if (sscanf(dateStr.c_str(), "%d-%d-%d", &y, &m, &d) == 3) {
      // YYYY-MM-DD
    } else if (sscanf(dateStr.c_str(), "%d/%d/%d", &d, &m, &y) == 3) {
      // DD/MM/YYYY
    } else {
      return 0;
    }

    std::tm tmDate = {};
    tmDate.tm_year = y - 1900;
    tmDate.tm_mon = m - 1;
    tmDate.tm_mday = d;
    tmDate.tm_isdst = -1;
    std::time_t t = std::mktime(&tmDate);
    if (t == (std::time_t)-1)
      return 0;
    return t / 86400;
  }

  // Kiểm tra xem hạn dùng (expiryDate) đã hết hạn so với thời gian giả lập chưa
  bool isDateExpired(const std::string &expiryDate) const {
    if (expiryDate.empty())
      return false;
    long long expDays = dateToEpochDays(expiryDate);
    long long currentDays = dateToEpochDays(getCurrentDate());
    if (expDays == 0 || currentDays == 0)
      return false;
    return currentDays > expDays;
  }

  // Tính số ngày còn lại (hoặc số ngày đã quá hạn nếu âm)
  int getRemainingDays(const std::string &expiryDate) const {
    if (expiryDate.empty())
      return 0;
    long long expDays = dateToEpochDays(expiryDate);
    long long currentDays = dateToEpochDays(getCurrentDate());
    return static_cast<int>(expDays - currentDays);
  }

  // Chuyển đổi định dạng ngày giờ (DD/MM/YYYY HH:MM:SS, DD/MM/YYYY HH:MM,
  // DD/MM/YYYY, YYYY-MM-DD...) thành epoch seconds (ANTY.md)
  static long long dateTimeToEpochSeconds(const std::string &timeStr) {
    if (timeStr.empty())
      return 0;
    int d = 1, m = 1, y = 1970, h = 0, min = 0, s = 0;
    if (timeStr.find('/') != std::string::npos) {
      if (std::sscanf(timeStr.c_str(), "%d/%d/%d %d:%d:%d", &d, &m, &y, &h,
                      &min, &s) < 3) {
        if (std::sscanf(timeStr.c_str(), "%d/%d/%d %d:%d", &d, &m, &y, &h,
                        &min) < 3) {
          std::sscanf(timeStr.c_str(), "%d/%d/%d", &d, &m, &y);
        }
      }
    } else if (timeStr.find('-') != std::string::npos) {
      if (std::sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d", &y, &m, &d, &h,
                      &min, &s) < 3) {
        if (std::sscanf(timeStr.c_str(), "%d-%d-%d %d:%d", &y, &m, &d, &h,
                        &min) < 3) {
          std::sscanf(timeStr.c_str(), "%d-%d-%d", &y, &m, &d);
        }
      }
    }
    std::tm tmVal = {};
    tmVal.tm_year = y - 1900;
    tmVal.tm_mon = m - 1;
    tmVal.tm_mday = d;
    tmVal.tm_hour = h;
    tmVal.tm_min = min;
    tmVal.tm_sec = s;
    tmVal.tm_isdst = -1;
    std::time_t t = std::mktime(&tmVal);
    return (t != (std::time_t)-1) ? static_cast<long long>(t) : 0;
  }

  // Kiểm tra thời điểm có nằm trong khoảng [fromDate, toDate] hay không
  // (ANTY.md)
  static bool isDateInRange(const std::string &targetTime,
                            const std::string &fromDate,
                            const std::string &toDate) {
    long long t = dateTimeToEpochSeconds(targetTime);
    if (!fromDate.empty()) {
      long long f = dateTimeToEpochSeconds(fromDate);
      if (f > 0 && t < f)
        return false;
    }
    if (!toDate.empty()) {
      long long to = dateTimeToEpochSeconds(toDate) + 86399; // Hết ngày toDate
      if (to > 86399 && t > to)
        return false;
    }
    return true;
  }
};

#endif // REAL_TIME_MANAGER_H
