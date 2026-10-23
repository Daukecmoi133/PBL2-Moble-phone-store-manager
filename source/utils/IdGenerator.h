#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

class IdGenerator {
private:
    struct CounterKey {
        std::string prefix;
        std::string date;

        bool operator<(const CounterKey& other) const {
            return prefix < other.prefix ||
                   (prefix == other.prefix && date < other.date);
        }
    };

    static bool parseNumber(const std::string& value, std::uint32_t& number) {
        if (value.empty()) return false;

        std::uint64_t parsed = 0;
        for (char character : value) {
            if (character < '0' || character > '9') return false;
            parsed = parsed * 10 + static_cast<unsigned>(character - '0');
            if (parsed > 1000000) return false;
        }
        if (parsed == 0) return false;
        number = static_cast<std::uint32_t>(parsed);
        return true;
    }

    static bool validDate(const std::string& date) {
        if (date.size() != 8) return false;
        for (char character : date) {
            if (character < '0' || character > '9') return false;
        }

        const int day = std::stoi(date.substr(0, 2));
        const int month = std::stoi(date.substr(2, 2));
        const int year = std::stoi(date.substr(4, 4));
        if (year < 1 || month < 1 || month > 12 || day < 1) return false;

        static const int daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30,
                                           31, 31, 30, 31, 30, 31};
        int lastDay = daysPerMonth[month];
        if (month == 2 && (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))) {
            lastDay = 29;
        }
        return day <= lastDay;
    }

    static bool replaceFile(const std::string& temporaryPath,
                            const std::string& destinationPath) {
#ifdef _WIN32
        return MoveFileExA(temporaryPath.c_str(), destinationPath.c_str(),
                           MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        return std::rename(temporaryPath.c_str(), destinationPath.c_str()) == 0;
#endif
    }

    static bool readCounters(const std::string& path,
                             std::map<CounterKey, std::uint32_t>& counters,
                             std::string& errorMessage) {
        std::ifstream input(path);
        if (!input.is_open()) {
            if (!std::filesystem::exists(path)) return true;
            errorMessage = "Không thể đọc bộ đếm ID tại " + path + ".";
            return false;
        }

        std::string line;
        std::size_t lineNumber = 0;
        while (std::getline(input, line)) {
            ++lineNumber;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::size_t start = 0;
            for (;;) {
                const std::size_t separator = line.find('|', start);
                parts.push_back(line.substr(start, separator - start));
                if (separator == std::string::npos) break;
                start = separator + 1;
            }

            CounterKey key;
            std::string numberText;
            if (parts.size() == 2) {
                key = {parts[0], ""};
                numberText = parts[1];
            } else if (parts.size() == 3 && validDate(parts[1])) {
                key = {parts[0], parts[1]};
                numberText = parts[2];
            } else {
                errorMessage = "Định dạng bộ đếm ID không hợp lệ tại dòng " +
                               std::to_string(lineNumber) + ".";
                return false;
            }

            std::uint32_t nextNumber = 0;
            if (key.prefix.empty() || !parseNumber(numberText, nextNumber) ||
                counters.find(key) != counters.end()) {
                errorMessage = "Giá trị bộ đếm ID không hợp lệ tại dòng " +
                               std::to_string(lineNumber) + ".";
                return false;
            }
            counters[key] = nextNumber;
        }
        if (input.bad()) {
            errorMessage = "Lỗi khi đọc bộ đếm ID tại " + path + ".";
            return false;
        }
        return true;
    }

    static bool writeCounters(
        const std::string& path,
        const std::map<CounterKey, std::uint32_t>& counters,
        std::string& errorMessage) {
        const std::string temporaryPath = path + ".tmp";
        std::ofstream output(temporaryPath, std::ios::trunc);
        if (!output.is_open()) {
            errorMessage = "Không thể ghi bộ đếm ID tại " + temporaryPath + ".";
            return false;
        }

        for (const auto& entry : counters) {
            output << entry.first.prefix << '|';
            if (!entry.first.date.empty()) output << entry.first.date << '|';
            output << entry.second << '\n';
        }
        output.flush();
        if (!output) {
            errorMessage = "Lỗi khi ghi bộ đếm ID tại " + temporaryPath + ".";
            output.close();
            std::remove(temporaryPath.c_str());
            return false;
        }
        output.close();
        if (!output || !replaceFile(temporaryPath, path)) {
            errorMessage = "Không thể cập nhật bộ đếm ID tại " + path + ".";
            std::remove(temporaryPath.c_str());
            return false;
        }
        return true;
    }

public:
    static std::string currentDateKey() {
        const std::time_t now = std::time(nullptr);
        const std::tm* localTime = std::localtime(&now);
        if (localTime == nullptr) return "";
        char date[9];
        std::strftime(date, sizeof(date), "%d%m%Y", localTime);
        return date;
    }

    static bool isValidId(const std::string& id, const std::string& prefix,
                          bool includesDate = false) {
        if (prefix.empty()) return false;
        const std::size_t suffixLength = includesDate ? 16 : 7;
        if (id.size() != prefix.size() + suffixLength ||
            id.compare(0, prefix.size(), prefix) != 0 ||
            id[prefix.size()] != '-') {
            return false;
        }

        std::size_t sequenceStart = prefix.size() + 1;
        if (includesDate) {
            const std::string date = id.substr(sequenceStart, 8);
            if (!validDate(date) || id[sequenceStart + 8] != '-') return false;
            sequenceStart += 9;
        }
        for (std::size_t index = sequenceStart; index < id.size(); ++index) {
            if (id[index] < '0' || id[index] > '9') return false;
        }
        return id.substr(sequenceStart) != "000000";
    }

    static bool nextId(const std::string& prefix, const std::string& date,
                       const std::vector<std::string>& existingIds,
                       std::string& id, std::string& errorMessage) {
        if (prefix.empty() || (!date.empty() && !validDate(date))) {
            errorMessage = "Tiền tố hoặc ngày dùng để sinh ID không hợp lệ.";
            return false;
        }

        static std::mutex counterMutex;
        const std::lock_guard<std::mutex> lock(counterMutex);

        const std::string path = "data/system/id_counters.txt";
        std::error_code filesystemError;
        std::filesystem::create_directories("data/system", filesystemError);
        if (filesystemError) {
            errorMessage = "Không thể tạo thư mục lưu bộ đếm ID: " +
                           filesystemError.message() + ".";
            return false;
        }

        std::map<CounterKey, std::uint32_t> counters;
        if (!readCounters(path, counters, errorMessage)) return false;

        const CounterKey key{prefix, date};
        std::uint32_t minimumNext = 1;
        for (const std::string& existingId : existingIds) {
            if (!isValidId(existingId, prefix, !date.empty())) continue;
            if (!date.empty() && existingId.substr(prefix.size() + 1, 8) != date) continue;

            const std::size_t sequenceStart =
                prefix.size() + (date.empty() ? 1 : 10);
            std::uint32_t sequence = 0;
            if (parseNumber(existingId.substr(sequenceStart), sequence) &&
                sequence >= minimumNext) {
                if (sequence == 999999) {
                    errorMessage = "Đã hết số ID khả dụng cho tiền tố " + prefix + ".";
                    return false;
                }
                minimumNext = sequence + 1;
            }
        }

        const auto savedCounter = counters.find(key);
        const std::uint32_t nextNumber =
            savedCounter == counters.end()
                ? minimumNext
                : std::max(minimumNext, savedCounter->second);
        if (nextNumber > 999999) {
            errorMessage = "Đã hết số ID khả dụng cho tiền tố " + prefix + ".";
            return false;
        }

        counters[key] = nextNumber + 1;
        if (!writeCounters(path, counters, errorMessage)) return false;

        id = prefix + '-';
        if (!date.empty()) id += date + '-';
        std::string sequence = std::to_string(nextNumber);
        id.append(6 - sequence.size(), '0');
        id += sequence;
        return true;
    }
};

#endif
