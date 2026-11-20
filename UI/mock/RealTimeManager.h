#ifndef REAL_TIME_MANAGER_H
#define REAL_TIME_MANAGER_H

#include "../../source/utils/AppClock.h"
#include <ctime>
#include <string>

/**
 * @file RealTimeManager.h
 * @brief Lớp bọc tương thích ngược tại tầng UI, ủy quyền hoàn toàn về AppClock.
 */
class RealTimeManager {
private:
    RealTimeManager() = default;

public:
    static RealTimeManager& getInstance() {
        static RealTimeManager instance;
        return instance;
    }

    void loadFromFile() { AppClock::instance().loadFromFile(); }
    bool saveToFile() { return AppClock::instance().saveToFile(); }

    std::string getCurrentDateTime() const {
        return AppClock::instance().getCurrentDateTime();
    }

    std::string getCurrentDate() const {
        return AppClock::instance().getCurrentDate();
    }

    std::time_t getSimulationTime() const {
        return AppClock::instance().getSimulationTime();
    }

    static std::time_t getSimulationTimeNow() {
        return AppClock::instance().getSimulationTime();
    }

    int getCurrentYear() const {
        return AppClock::instance().getCurrentYear();
    }

    void advanceSeconds(int seconds = 1) {
        AppClock::instance().advanceSeconds(seconds);
    }

    bool setDateTime(const std::string& dtStr) {
        return AppClock::instance().setDateTime(dtStr);
    }

    bool resetToSystemNow() {
        return AppClock::instance().resetToSystemNow();
    }

    bool advanceDays(int days) {
        return AppClock::instance().advanceDays(days);
    }

    static long long dateToEpochDays(const std::string& dateStr) {
        return AppClock::dateToEpochDays(dateStr);
    }

    bool isDateExpired(const std::string& expiryDate) const {
        return AppClock::instance().isDateExpired(expiryDate);
    }

    int getRemainingDays(const std::string& expiryDate) const {
        return AppClock::instance().getRemainingDays(expiryDate);
    }

    static long long dateTimeToEpochSeconds(const std::string& timeStr) {
        return AppClock::dateTimeToEpochSeconds(timeStr);
    }

    static bool isDateInRange(const std::string& targetTime,
                              const std::string& fromDate,
                              const std::string& toDate) {
        return AppClock::isDateInRange(targetTime, fromDate, toDate);
    }
};

#endif // REAL_TIME_MANAGER_H
