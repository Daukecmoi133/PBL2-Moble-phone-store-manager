#ifndef APP_CLOCK_H
#define APP_CLOCK_H

#include "DateTime.h"
#include "FileHelper.h"
#include "PathHelper.h"
#include "StringHelper.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

/**
 * @file AppClock.h
 * @brief Nguồn đồng hồ mô phỏng thống nhất cho toàn hệ thống (Core + UI).
 *
 * Lưu trữ tại `data/realTime.txt` theo chuẩn "DD/MM/YYYY HH:MM:SS".
 * Đọc tương thích ngược "YYYY-MM-DD HH:MM:SS" / "YYYY-MM-DD".
 */
class AppClock {
private:
    std::string currentSimulatedTime;
    std::string loadedDataRoot;

    std::string getFilePath() const {
        return PathHelper::resolveDataPath("data/realTime.txt");
    }

    void ensureLoaded() {
        const std::string curRoot = PathHelper::getDataRoot();
        if (currentSimulatedTime.empty() || loadedDataRoot != curRoot) {
            loadFromFile();
        }
    }

    AppClock() {
        loadFromFile();
        if (currentSimulatedTime.empty()) {
            currentSimulatedTime = "17/10/2026 12:00:00";
        }
    }

public:
    static AppClock& instance() {
        static AppClock inst;
        return inst;
    }

    void loadFromFile() {
        loadedDataRoot = PathHelper::getDataRoot();
        std::ifstream file(getFilePath());
        if (file.is_open()) {
            std::string line;
            if (std::getline(file, line)) {
                StringHelper::stripUtf8Bom(line);
                line = StringHelper::trim(line);
                DateTime dt(0, 0, 0, 0, 0, 0);
                if (DateTime::tryParse(line, dt)) {
                    currentSimulatedTime = dt.toString();
                    file.close();
                    return;
                }
            }
            file.close();
        }
        currentSimulatedTime = "17/10/2026 12:00:00";
    }

    bool saveToFile() {
        return FileHelper::atomicWrite(getFilePath(), [&](std::ofstream& file) {
            file << currentSimulatedTime << "\n";
            return file.good();
        });
    }

    std::string getCurrentDateTime() {
        ensureLoaded();
        return currentSimulatedTime;
    }

    std::string getCurrentDate() {
        ensureLoaded();
        DateTime dt(0, 0, 0, 0, 0, 0);
        if (DateTime::tryParse(currentSimulatedTime, dt)) {
            return dt.toDateString();
        }
        if (currentSimulatedTime.length() >= 10) {
            return currentSimulatedTime.substr(0, 10);
        }
        return "17/10/2026";
    }

    DateTime getCurrentDateTimeObj() {
        ensureLoaded();
        DateTime dt(2026, 10, 17, 12, 0, 0);
        DateTime::tryParse(currentSimulatedTime, dt);
        return dt;
    }

    std::time_t getSimulationTime() {
        ensureLoaded();
        long long sec = dateTimeToEpochSeconds(currentSimulatedTime);
        return (sec > 0) ? static_cast<std::time_t>(sec) : std::time(nullptr);
    }

    int getCurrentYear() {
        return getCurrentDateTimeObj().year;
    }

    void advanceSeconds(int seconds = 1) {
        ensureLoaded();
        DateTime dt(0, 0, 0, 0, 0, 0);
        if (!DateTime::tryParse(currentSimulatedTime, dt)) {
            dt = DateTime(2026, 10, 17, 12, 0, 0);
        }
        std::tm tmTime = {};
        tmTime.tm_year = dt.year - 1900;
        tmTime.tm_mon = dt.month - 1;
        tmTime.tm_mday = dt.day;
        tmTime.tm_hour = dt.hour;
        tmTime.tm_min = dt.minute;
        tmTime.tm_sec = dt.second + seconds;
        tmTime.tm_isdst = -1;

        std::time_t nextTime = std::mktime(&tmTime);
        if (nextTime != (std::time_t)(-1)) {
            std::tm nextTm;
#ifdef _WIN32
            localtime_s(&nextTm, &nextTime);
#else
            localtime_r(&nextTime, &nextTm);
#endif
            DateTime nextDt(nextTm.tm_year + 1900, nextTm.tm_mon + 1, nextTm.tm_mday,
                            nextTm.tm_hour, nextTm.tm_min, nextTm.tm_sec);
            currentSimulatedTime = nextDt.toString();
        }
    }

    bool setDateTime(const std::string& dtStr) {
        ensureLoaded();
        DateTime dt(0, 0, 0, 0, 0, 0);
        if (!DateTime::tryParse(dtStr, dt)) {
            return false;
        }
        std::string previous = currentSimulatedTime;
        currentSimulatedTime = dt.toString();
        if (!saveToFile()) {
            currentSimulatedTime = previous;
            return false;
        }
        return true;
    }

    bool resetToSystemNow() {
        ensureLoaded();
        std::string previous = currentSimulatedTime;
        currentSimulatedTime = DateTime::now().toString();
        if (!saveToFile()) {
            currentSimulatedTime = previous;
            return false;
        }
        return true;
    }

    bool advanceDays(int days) {
        ensureLoaded();
        DateTime dt(0, 0, 0, 0, 0, 0);
        if (!DateTime::tryParse(currentSimulatedTime, dt)) {
            dt = DateTime(2026, 10, 17, 12, 0, 0);
        }
        std::tm tmTime = {};
        tmTime.tm_year = dt.year - 1900;
        tmTime.tm_mon = dt.month - 1;
        tmTime.tm_mday = dt.day + days;
        tmTime.tm_hour = dt.hour;
        tmTime.tm_min = dt.minute;
        tmTime.tm_sec = dt.second;
        tmTime.tm_isdst = -1;

        std::time_t nextTime = std::mktime(&tmTime);
        if (nextTime == (std::time_t)(-1)) {
            return false;
        }
        std::tm nextTm;
#ifdef _WIN32
        localtime_s(&nextTm, &nextTime);
#else
        localtime_r(&nextTime, &nextTm);
#endif
        DateTime nextDt(nextTm.tm_year + 1900, nextTm.tm_mon + 1, nextTm.tm_mday,
                        nextTm.tm_hour, nextTm.tm_min, nextTm.tm_sec);
        std::string previous = currentSimulatedTime;
        currentSimulatedTime = nextDt.toString();
        if (!saveToFile()) {
            currentSimulatedTime = previous;
            return false;
        }
        return true;
    }

    bool isDateExpired(const std::string& expiryDate) {
        if (expiryDate.empty()) return false;
        long long expDays = dateToEpochDays(expiryDate);
        long long currentDays = dateToEpochDays(getCurrentDate());
        if (expDays == 0 || currentDays == 0) return false;
        return currentDays > expDays;
    }

    int getRemainingDays(const std::string& expiryDate) {
        if (expiryDate.empty()) return 0;
        long long expDays = dateToEpochDays(expiryDate);
        long long currentDays = dateToEpochDays(getCurrentDate());
        return static_cast<int>(expDays - currentDays);
    }

    static DateTime now() {
        return instance().getCurrentDateTimeObj();
    }

    static std::string currentDateKey() {
        const DateTime dt = now();
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << dt.day
            << std::setw(2) << dt.month
            << std::setw(4) << dt.year;
        return oss.str();
    }

    static long long dateToEpochDays(const std::string& dateStr) {
        DateTime dt(0, 0, 0, 0, 0, 0);
        if (!DateTime::tryParse(dateStr, dt)) {
            return 0;
        }
        std::tm tmDate = {};
        tmDate.tm_year = dt.year - 1900;
        tmDate.tm_mon = dt.month - 1;
        tmDate.tm_mday = dt.day;
        tmDate.tm_hour = 0;
        tmDate.tm_min = 0;
        tmDate.tm_sec = 0;
        tmDate.tm_isdst = -1;
        std::time_t t = std::mktime(&tmDate);
        if (t == (std::time_t)-1) return 0;
        return static_cast<long long>(t) / 86400;
    }

    static long long dateTimeToEpochSeconds(const std::string& timeStr) {
        DateTime dt(0, 0, 0, 0, 0, 0);
        if (!DateTime::tryParse(timeStr, dt)) {
            return 0;
        }
        std::tm tmVal = {};
        tmVal.tm_year = dt.year - 1900;
        tmVal.tm_mon = dt.month - 1;
        tmVal.tm_mday = dt.day;
        tmVal.tm_hour = dt.hour;
        tmVal.tm_min = dt.minute;
        tmVal.tm_sec = dt.second;
        tmVal.tm_isdst = -1;
        std::time_t t = std::mktime(&tmVal);
        return (t != (std::time_t)-1) ? static_cast<long long>(t) : 0;
    }

    static bool isDateInRange(const std::string& targetTime,
                              const std::string& fromDate,
                              const std::string& toDate) {
        long long t = dateTimeToEpochSeconds(targetTime);
        if (!fromDate.empty()) {
            long long f = dateTimeToEpochSeconds(fromDate);
            if (f > 0 && t < f) return false;
        }
        if (!toDate.empty()) {
            long long to = dateTimeToEpochSeconds(toDate) + 86399;
            if (to > 86399 && t > to) return false;
        }
        return true;
    }
};

#endif // APP_CLOCK_H
