#ifndef DATETIME_H
#define DATETIME_H

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

struct DateTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;

  // Constructors
  DateTime();
  DateTime(int y, int m, int d, int h = 0, int min = 0, int s = 0);

  // Static helper methods
  static DateTime now();
  static DateTime fromDateString(const std::string &str);
  static DateTime fromString(const std::string &str);
  static bool isValidDate(int y, int m, int d);
  static bool isValidDateString(const std::string &str);

  // Member methods
  void setTime(int y, int m, int d, int h = 0, int min = 0, int s = 0);
  std::string toString() const;
  std::string toDateString() const;
  DateTime addDays(int days) const;
  DateTime addMonths(int months) const;

  // Comparison operators
  bool operator<(const DateTime &other) const;
  bool operator>(const DateTime &other) const;
  bool operator<=(const DateTime &other) const;
  bool operator>=(const DateTime &other) const;
  bool operator==(const DateTime &other) const;
  bool operator!=(const DateTime &other) const;
};

// ================= IMPLEMENTATION =================

inline DateTime::DateTime() {
  std::time_t t = std::time(nullptr);
  std::tm *nowTime = std::localtime(&t);
  if (nowTime) {
    year = nowTime->tm_year + 1900;
    month = nowTime->tm_mon + 1;
    day = nowTime->tm_mday;
    hour = nowTime->tm_hour;
    minute = nowTime->tm_min;
    second = nowTime->tm_sec;
  } else {
    year = 2026;
    month = 1;
    day = 1;
    hour = 0;
    minute = 0;
    second = 0;
  }
}

inline DateTime::DateTime(int y, int m, int d, int h, int min, int s)
    : year(y), month(m), day(d), hour(h), minute(min), second(s) {}

inline DateTime DateTime::now() { return DateTime(); }

inline void DateTime::setTime(int y, int m, int d, int h, int min, int s) {
  year = y;
  month = m;
  day = d;
  hour = h;
  minute = min;
  second = s;
}

inline std::string DateTime::toString() const {
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << day << "/" << std::setw(2)
      << month << "/" << year << " " << std::setw(2) << hour << ":"
      << std::setw(2) << minute << ":" << std::setw(2) << second;
  return oss.str();
}

inline std::string DateTime::toDateString() const {
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << day << "/" << std::setw(2)
      << month << "/" << year;
  return oss.str();
}

// FIX LỖI: Hàm phụ trợ kiểm tra năm nhuận và ngày hợp lệ
inline bool isLeapYear(int y) {
  return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

inline bool isValidDate(int y, int m, int d) {
  if (y < 1 || m < 1 || m > 12 || d < 1)
    return false;
  static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};
  int maxDays = daysInMonth[m];
  if (m == 2 && isLeapYear(y))
    maxDays = 29;
  return d <= maxDays;
}

inline bool isValidTime(int h, int min, int s) {
  return (h >= 0 && h <= 23) && (min >= 0 && min <= 59) && (s >= 0 && s <= 59);
}

inline bool DateTime::isValidDate(int y, int m, int d) {
  return ::isValidDate(y, m, d);
}

inline bool DateTime::isValidDateString(const std::string &str) {
  if (str.empty())
    return false;
  int d = 0, m = 0, y = 0;
  if (str.find('/') != std::string::npos) {
    char sep1 = 0, sep2 = 0;
    std::istringstream iss(str);
    if ((iss >> d >> sep1 >> m >> sep2 >> y) && sep1 == '/' && sep2 == '/') {
      return ::isValidDate(y, m, d);
    }
  } else if (str.find('-') != std::string::npos) {
    char sep1 = 0, sep2 = 0;
    std::istringstream iss(str);
    if ((iss >> y >> sep1 >> m >> sep2 >> d) && sep1 == '-' && sep2 == '-') {
      return ::isValidDate(y, m, d);
    }
  }
  return false;
}

inline DateTime DateTime::addDays(int days) const {
  DateTime result = *this;
  if (days == 0)
    return result;

  static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};

  auto getMonthDays = [](int y, int m) -> int {
    if (m == 2 && isLeapYear(y))
      return 29;
    return daysInMonth[m];
  };

  if (days > 0) {
    int d = result.day + days;
    int m = result.month;
    int y = result.year;
    while (d > getMonthDays(y, m)) {
      d -= getMonthDays(y, m);
      m++;
      if (m > 12) {
        m = 1;
        y++;
      }
    }
    result.day = d;
    result.month = m;
    result.year = y;
  } else {
    int d = result.day + days;
    int m = result.month;
    int y = result.year;
    while (d < 1) {
      m--;
      if (m < 1) {
        m = 12;
        y--;
      }
      d += getMonthDays(y, m);
    }
    result.day = d;
    result.month = m;
    result.year = y;
  }
  return result;
}

inline DateTime DateTime::addMonths(int months) const {
  DateTime result = *this;
  if (months == 0)
    return result;

  static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};

  auto getMonthDays = [](int y, int m) -> int {
    if (m == 2 && isLeapYear(y))
      return 29;
    return daysInMonth[m];
  };

  int totalMonths = (result.year * 12 + (result.month - 1)) + months;
  int newYear = totalMonths / 12;
  int newMonth = (totalMonths % 12) + 1;
  if (newMonth <= 0) {
    newYear -= 1;
    newMonth += 12;
  }

  result.year = newYear;
  result.month = newMonth;

  // Kẹp ngày về cuối tháng đích nếu ngày vượt quá số ngày của tháng đó
  int maxDays = getMonthDays(result.year, result.month);
  if (result.day > maxDays) {
    result.day = maxDays;
  }

  return result;
}

inline DateTime DateTime::fromDateString(const std::string &str) {
  // Hỗ trợ định dạng DD/MM/YYYY hoặc YYYY-MM-DD
  DateTime dt(2026, 1, 1);
  if (str.empty())
    return dt;

  int d = 1, m = 1, y = 2026;
  if (str.find('/') != std::string::npos) {
    char sep1, sep2;
    std::istringstream iss(str);
    if (iss >> d >> sep1 >> m >> sep2 >> y) {
      if (isValidDate(y, m, d)) {
        dt.day = d;
        dt.month = m;
        dt.year = y;
      }
    }
  } else if (str.find('-') != std::string::npos) {
    char sep1, sep2;
    std::istringstream iss(str);
    if (iss >> y >> sep1 >> m >> sep2 >> d) {
      if (isValidDate(y, m, d)) {
        dt.year = y;
        dt.month = m;
        dt.day = d;
      }
    }
  }
  return dt;
}

inline DateTime DateTime::fromString(const std::string &str) {
  DateTime dt(2026, 1, 1);
  if (str.empty())
    return dt;
  std::istringstream iss(str);
  std::string datePart, timePart;
  iss >> datePart >> timePart;
  dt = fromDateString(datePart);
  if (!timePart.empty()) {
    char sep1, sep2;
    int h = 0, min = 0, s = 0;
    std::istringstream timeIss(timePart);
    if (timeIss >> h >> sep1 >> min >> sep2 >> s) {
      if (isValidTime(h, min, s)) {
        dt.hour = h;
        dt.minute = min;
        dt.second = s;
      }
    }
  }
  return dt;
}

inline bool DateTime::operator<(const DateTime &other) const {
  if (year != other.year)
    return year < other.year;
  if (month != other.month)
    return month < other.month;
  if (day != other.day)
    return day < other.day;
  if (hour != other.hour)
    return hour < other.hour;
  if (minute != other.minute)
    return minute < other.minute;
  return second < other.second;
}

inline bool DateTime::operator>(const DateTime &other) const {
  return other < *this;
}

inline bool DateTime::operator<=(const DateTime &other) const {
  return !(*this > other);
}

inline bool DateTime::operator>=(const DateTime &other) const {
  return !(*this < other);
}

inline bool DateTime::operator==(const DateTime &other) const {
  return year == other.year && month == other.month && day == other.day &&
         hour == other.hour && minute == other.minute && second == other.second;
}

inline bool DateTime::operator!=(const DateTime &other) const {
  return !(*this == other);
}

// Giữ lại alias Calender để tương thích ngược với code cũ nếu cần
using Calender = DateTime;

#endif // DATETIME_H
