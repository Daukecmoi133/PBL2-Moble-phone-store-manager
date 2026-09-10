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

    DateTime() {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        if (now) {
            year = now->tm_year + 1900;
            month = now->tm_mon + 1;
            day = now->tm_mday;
            hour = now->tm_hour;
            minute = now->tm_min;
            second = now->tm_sec;
        } else {
            year = 2026; month = 1; day = 1;
            hour = 0; minute = 0; second = 0;
        }
    }

    DateTime(int y, int m, int d, int h = 0, int min = 0, int s = 0)
        : year(y), month(m), day(d), hour(h), minute(min), second(s) {}

    static DateTime now() {
        return DateTime();
    }

    void setTime(int y, int m, int d, int h = 0, int min = 0, int s = 0) {
        year = y; month = m; day = d;
        hour = h; minute = min; second = s;
    }

    std::string toString() const {
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

    std::string toDateString() const {
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(2) << day << "/"
            << std::setw(2) << month << "/"
            << year;
        return oss.str();
    }

    static DateTime fromDateString(const std::string& str) {
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

    // Các toán tử so sánh thời gian
    bool operator<(const DateTime& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        if (day != other.day) return day < other.day;
        if (hour != other.hour) return hour < other.hour;
        if (minute != other.minute) return minute < other.minute;
        return second < other.second;
    }

    bool operator>(const DateTime& other) const {
        return other < *this;
    }

    bool operator<=(const DateTime& other) const {
        return !(*this > other);
    }

    bool operator>=(const DateTime& other) const {
        return !(*this < other);
    }

    bool operator==(const DateTime& other) const {
        return year == other.year && month == other.month && day == other.day &&
               hour == other.hour && minute == other.minute && second == other.second;
    }

    bool operator!=(const DateTime& other) const {
        return !(*this == other);
    }
};

// Giữ lại alias Calender để tương thích ngược với code cũ nếu cần
using Calender = DateTime;

#endif // DATETIME_H
