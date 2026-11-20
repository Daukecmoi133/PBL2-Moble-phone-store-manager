#ifndef DATETIME_H
#define DATETIME_H

#include <cctype>
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
  static bool tryParseDate(const std::string &str, DateTime &out);
  static bool tryParse(const std::string &str, DateTime &out);
  static DateTime fromDateString(const std::string &str);
  static DateTime fromString(const std::string &str);
  static bool isValidDate(int y, int m, int d);
  static bool isValidDateString(const std::string &str);

  // Member methods
  bool isValid() const;
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

namespace dt_detail {
inline std::string trimWs(const std::string &s) {
  size_t first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  size_t last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

inline bool parseUnsignedIntStrict(const std::string &tok, int &out) {
  if (tok.empty() || tok.size() > 6)
    return false;
  long long v = 0;
  for (char c : tok) {
    if (!std::isdigit(static_cast<unsigned char>(c)))
      return false;
    v = v * 10 + (c - '0');
  }
  out = static_cast<int>(v);
  return true;
}
} // namespace dt_detail

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

inline bool DateTime::isValid() const {
  return ::isValidDate(year, month, day) && ::isValidTime(hour, minute, second);
}

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
      << month << "/" << std::setw(4) << year << " " << std::setw(2) << hour
      << ":" << std::setw(2) << minute << ":" << std::setw(2) << second;
  return oss.str();
}

inline std::string DateTime::toDateString() const {
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << day << "/" << std::setw(2)
      << month << "/" << std::setw(4) << year;
  return oss.str();
}

inline bool DateTime::isValidDate(int y, int m, int d) {
  return ::isValidDate(y, m, d);
}

inline bool DateTime::tryParseDate(const std::string &str, DateTime &out) {
  const std::string trimmed = dt_detail::trimWs(str);
  if (trimmed.empty())
    return false;

  int d = 0, m = 0, y = 0;
  if (trimmed.find('/') != std::string::npos) {
    if (trimmed.find('-') != std::string::npos)
      return false;
    const size_t p1 = trimmed.find('/');
    const size_t p2 = trimmed.find('/', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos ||
        trimmed.find('/', p2 + 1) != std::string::npos) {
      return false;
    }
    if (!dt_detail::parseUnsignedIntStrict(trimmed.substr(0, p1), d) ||
        !dt_detail::parseUnsignedIntStrict(trimmed.substr(p1 + 1, p2 - p1 - 1), m) ||
        !dt_detail::parseUnsignedIntStrict(trimmed.substr(p2 + 1), y)) {
      return false;
    }
    if (y < 100 || y > 9999 || !::isValidDate(y, m, d))
      return false;
    out = DateTime(y, m, d, 0, 0, 0);
    return true;
  }

  if (trimmed.find('-') != std::string::npos) {
    const size_t p1 = trimmed.find('-');
    const size_t p2 = trimmed.find('-', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos ||
        trimmed.find('-', p2 + 1) != std::string::npos) {
      return false;
    }
    const std::string yStr = trimmed.substr(0, p1);
    if (yStr.size() != 4)
      return false;
    if (!dt_detail::parseUnsignedIntStrict(yStr, y) ||
        !dt_detail::parseUnsignedIntStrict(trimmed.substr(p1 + 1, p2 - p1 - 1), m) ||
        !dt_detail::parseUnsignedIntStrict(trimmed.substr(p2 + 1), d)) {
      return false;
    }
    if (y < 100 || y > 9999 || !::isValidDate(y, m, d))
      return false;
    out = DateTime(y, m, d, 0, 0, 0);
    return true;
  }

  return false;
}

inline bool DateTime::tryParse(const std::string &str, DateTime &out) {
  const std::string trimmed = dt_detail::trimWs(str);
  if (trimmed.empty())
    return false;

  std::istringstream iss(trimmed);
  std::string datePart, timePart, extraPart;
  if (!(iss >> datePart))
    return false;
  if (iss >> timePart) {
    if (iss >> extraPart)
      return false; // Dữ liệu thừa sau giờ
  }

  DateTime dt(0, 0, 0, 0, 0, 0);
  if (!tryParseDate(datePart, dt))
    return false;

  if (!timePart.empty()) {
    const size_t c1 = timePart.find(':');
    if (c1 == std::string::npos)
      return false;
    const size_t c2 = timePart.find(':', c1 + 1);
    int h = 0, min = 0, s = 0;
    if (c2 == std::string::npos) {
      // Định dạng cũ HH:MM (ví dụ ProductReviews.txt v1)
      if (!dt_detail::parseUnsignedIntStrict(timePart.substr(0, c1), h) ||
          !dt_detail::parseUnsignedIntStrict(timePart.substr(c1 + 1), min)) {
        return false;
      }
    } else {
      if (timePart.find(':', c2 + 1) != std::string::npos)
        return false;
      if (!dt_detail::parseUnsignedIntStrict(timePart.substr(0, c1), h) ||
          !dt_detail::parseUnsignedIntStrict(timePart.substr(c1 + 1, c2 - c1 - 1), min) ||
          !dt_detail::parseUnsignedIntStrict(timePart.substr(c2 + 1), s)) {
        return false;
      }
    }
    if (!::isValidTime(h, min, s))
      return false;
    dt.hour = h;
    dt.minute = min;
    dt.second = s;
  }

  out = dt;
  return true;
}

inline bool DateTime::isValidDateString(const std::string &str) {
  DateTime tmp(0, 0, 0, 0, 0, 0);
  return tryParseDate(str, tmp);
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
  DateTime dt(0, 0, 0, 0, 0, 0);
  if (tryParseDate(str, dt)) {
    return dt;
  }
  return DateTime(0, 0, 0, 0, 0, 0);
}

inline DateTime DateTime::fromString(const std::string &str) {
  DateTime dt(0, 0, 0, 0, 0, 0);
  if (tryParse(str, dt)) {
    return dt;
  }
  return DateTime(0, 0, 0, 0, 0, 0);
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

using Calender = DateTime;

#endif // DATETIME_H
