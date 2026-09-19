#ifndef DATETIME_H
#define DATETIME_H

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

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
    static DateTime fromDateString(const std::string& str);
    static DateTime fromString(const std::string& str);

    // Member methods
    void setTime(int y, int m, int d, int h = 0, int min = 0, int s = 0);
    std::string toString() const;
    std::string toDateString() const;

    // Comparison operators
    bool operator<(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
};

// ================= IMPLEMENTATION =================

inline DateTime::DateTime() {
    std::time_t t = std::time(nullptr);
    std::tm* nowTime = std::localtime(&t);
    if (nowTime) {
        year = nowTime->tm_year + 1900;
        month = nowTime->tm_mon + 1;
        day = nowTime->tm_mday;
        hour = nowTime->tm_hour;
        minute = nowTime->tm_min;
        second = nowTime->tm_sec;
    } else {
        year = 2026; month = 1; day = 1;
        hour = 0; minute = 0; second = 0;
    }
}

inline DateTime::DateTime(int y, int m, int d, int h, int min, int s)
    : year(y), month(m), day(d), hour(h), minute(min), second(s) {}

inline DateTime DateTime::now() {
    return DateTime();
}

inline void DateTime::setTime(int y, int m, int d, int h, int min, int s) {
    year = y; month = m; day = d;
    hour = h; minute = min; second = s;
}

inline std::string DateTime::toString() const {
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << day << "/"
        << std::setw(2) << month << "/"
        << year << " "
        << std::setw(2) << hour << ":"
        << std::setw(2) << minute << ":"
        << std::setw(2) << second;
    return oss.str();
}

inline std::string DateTime::toDateString() const {
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << day << "/"
        << std::setw(2) << month << "/"
        << year;
    return oss.str();
}

inline DateTime DateTime::fromDateString(const std::string& str) {
    // Hỗ trợ định dạng DD/MM/YYYY hoặc YYYY-MM-DD
    DateTime dt(2026, 1, 1);
    if (str.find('/') != std::string::npos) {
        char sep;
        std::istringstream iss(str);
        iss >> dt.day >> sep >> dt.month >> sep >> dt.year;
    } else if (str.find('-') != std::string::npos) {
        char sep;
        std::istringstream iss(str);
        iss >> dt.year >> sep >> dt.month >> sep >> dt.day;
    }
    return dt;
}

inline DateTime DateTime::fromString(const std::string& str) {
    DateTime dt(2026, 1, 1);
    if (str.empty()) return dt;
    std::istringstream iss(str);
    std::string datePart, timePart;
    iss >> datePart >> timePart;
    dt = fromDateString(datePart);
    if (!timePart.empty()) {
        char sep;
        std::istringstream timeIss(timePart);
        timeIss >> dt.hour >> sep >> dt.minute >> sep >> dt.second;
    }
    return dt;
}

inline bool DateTime::operator<(const DateTime& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    if (day != other.day) return day < other.day;
    if (hour != other.hour) return hour < other.hour;
    if (minute != other.minute) return minute < other.minute;
    return second < other.second;
}

inline bool DateTime::operator>(const DateTime& other) const {
    return other < *this;
}

inline bool DateTime::operator<=(const DateTime& other) const {
    return !(*this > other);
}

inline bool DateTime::operator>=(const DateTime& other) const {
    return !(*this < other);
}

inline bool DateTime::operator==(const DateTime& other) const {
    return year == other.year && month == other.month && day == other.day &&
           hour == other.hour && minute == other.minute && second == other.second;
}

inline bool DateTime::operator!=(const DateTime& other) const {
    return !(*this == other);
}

// Giữ lại alias Calender để tương thích ngược với code cũ nếu cần
using Calender = DateTime;

#endif // DATETIME_H
