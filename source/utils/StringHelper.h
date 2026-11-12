#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "../core/Vector.h"

class StringHelper {
public:
    // Cắt chuỗi theo ký tự phân cách (trả về Vector tự cài đặt)
    static Vector<std::string> split(const std::string& str, char delimiter);

    // Xóa khoảng trắng đầu và cuối chuỗi
    static std::string trim(const std::string& str);

    // Chuyển chuỗi thành chữ thường
    static std::string toLower(std::string str);

    // Kiểm tra chuỗi con không phân biệt hoa thường
    static bool containsIgnoreCase(const std::string& haystack, const std::string& needle);

    // Phân tích chuỗi giá tiền tệ (ví dụ: "10.600.000" hoặc "10600000" -> double 10600000.0)
    static double parsePrice(std::string priceStr);

    // Định dạng số tiền sang định dạng tiền Việt Nam (ví dụ 10600000 -> "10.600.000 đ")
    static std::string formatCurrency(double amount);
};

// ================= IMPLEMENTATION =================

inline Vector<std::string> StringHelper::split(const std::string& str, char delimiter) {
    Vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline std::string StringHelper::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

inline std::string StringHelper::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return str;
}

inline bool StringHelper::containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    return toLower(haystack).find(toLower(needle)) != std::string::npos;
}

inline double StringHelper::parsePrice(std::string priceStr) {
    priceStr = trim(priceStr);
    std::string cleanStr = "";
    for (char c : priceStr) {
        if (isdigit(c) || c == '.') {
            // Nếu dấu chấm là phân cách hàng nghìn (ví dụ 10.600.000)
            // ta bỏ qua dấu chấm. Nếu sau dấu chấm có 3 chữ số thì xem là hàng nghìn.
            if (c != '.') {
                cleanStr += c;
            }
        }
    }
    if (cleanStr.empty()) return 0.0;
    try {
        return std::stod(cleanStr);
    } catch (...) {
        return 0.0;
    }
}

inline std::string StringHelper::formatCurrency(double amount) {
    long long val = static_cast<long long>(amount);
    std::string numStr = std::to_string(val);
    std::string result = "";
    int count = 0;
    for (int i = static_cast<int>(numStr.length()) - 1; i >= 0; --i) {
        result = numStr[i] + result;
        ++count;
        if (count % 3 == 0 && i > 0) {
            result = "." + result;
        }
    }
    return result;
}

#endif // STRINGHELPER_H
