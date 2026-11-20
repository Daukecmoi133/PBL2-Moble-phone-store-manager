#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include "../core/Vector.h"

class StringHelper {
public:
    // Cắt chuỗi theo ký tự phân cách (trả về Vector tự cài đặt)
    static Vector<std::string> split(const std::string& str, char delimiter);

    // Xóa khoảng trắng đầu và cuối chuỗi
    static std::string trim(const std::string& str);

    // Xóa UTF-8 BOM (0xEF 0xBB 0xBF) nếu có ở đầu chuỗi
    static void stripUtf8Bom(std::string& str);

    // Chuyển chuỗi UTF-8 (hỗ trợ tiếng Việt có dấu) thành chữ thường
    static std::string toLower(std::string str);

    // Kiểm tra chuỗi con không phân biệt hoa thường (UTF-8 tiếng Việt)
    static bool containsIgnoreCase(const std::string& haystack, const std::string& needle);

    // Phân tích chuỗi giá tiền tệ (ví dụ: "-100.000", "10.600.000", "100000.50", "14000000.000000")
    static double parsePrice(std::string priceStr);

    // Định dạng số tiền sang chuỗi hiển thị Việt Nam (ví dụ 10600000 -> "10.600.000", -100000 -> "-100.000")
    static std::string formatCurrency(double amount);

    // Định dạng số tiền nguyên VND để ghi file .txt (không dấu chấm, không .000000)
    static std::string formatMoneyDisk(double amount);
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

inline void StringHelper::stripUtf8Bom(std::string& str) {
    if (str.size() >= 3 &&
        static_cast<unsigned char>(str[0]) == 0xEF &&
        static_cast<unsigned char>(str[1]) == 0xBB &&
        static_cast<unsigned char>(str[2]) == 0xBF) {
        str.erase(0, 3);
    }
}

namespace detail {
inline uint32_t lowercaseVietnameseCodePoint(uint32_t cp) {
    if (cp >= 0x0041 && cp <= 0x005A) {
        return cp + 0x20;
    }
    if (cp >= 0x00C0 && cp <= 0x00DE && cp != 0x00D7) {
        return cp + 0x20;
    }
    // Latin Extended-A: Ă(0102), Đ(0110), Ĩ(0128), Ũ(0168) và các cặp chẵn -> lẻ
    if ((cp >= 0x0100 && cp <= 0x012E && (cp & 1u) == 0u) ||
        (cp >= 0x0132 && cp <= 0x0136 && (cp & 1u) == 0u) ||
        (cp >= 0x014A && cp <= 0x0176 && (cp & 1u) == 0u)) {
        return cp + 1u;
    }
    // Latin Extended-B: Ơ(01A0) -> ơ(01A1), Ư(01AF) -> ư(01B0)
    if (cp == 0x01A0 || cp == 0x01AF) {
        return cp + 1u;
    }
    // Latin Extended Additional: toàn bộ nguyên âm/chữ tiếng Việt có dấu (Ạ..Ỹ: 1EA0..1EF9)
    if (cp >= 0x1EA0 && cp <= 0x1EF8 && (cp & 1u) == 0u) {
        return cp + 1u;
    }
    return cp;
}

inline void appendUtf8CodePoint(std::string& out, uint32_t cp) {
    if (cp <= 0x7Fu) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FFu) {
        out.push_back(static_cast<char>(0xC0u | ((cp >> 6) & 0x1Fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    } else if (cp <= 0xFFFFu) {
        out.push_back(static_cast<char>(0xE0u | ((cp >> 12) & 0x0Fu)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    } else {
        out.push_back(static_cast<char>(0xF0u | ((cp >> 18) & 0x07u)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    }
}
} // namespace detail

inline std::string StringHelper::toLower(std::string str) {
    std::string out;
    out.reserve(str.size());
    const size_t n = str.size();
    for (size_t i = 0; i < n;) {
        const unsigned char b0 = static_cast<unsigned char>(str[i]);
        if (b0 < 0x80u) {
            if (b0 >= 'A' && b0 <= 'Z') {
                out.push_back(static_cast<char>(b0 + ('a' - 'A')));
            } else {
                out.push_back(static_cast<char>(b0));
            }
            ++i;
            continue;
        }
        if ((b0 & 0xE0u) == 0xC0u && i + 1 < n) {
            const unsigned char b1 = static_cast<unsigned char>(str[i + 1]);
            if ((b1 & 0xC0u) == 0x80u) {
                uint32_t cp = (static_cast<uint32_t>(b0 & 0x1Fu) << 6) |
                              static_cast<uint32_t>(b1 & 0x3Fu);
                cp = detail::lowercaseVietnameseCodePoint(cp);
                detail::appendUtf8CodePoint(out, cp);
                i += 2;
                continue;
            }
        } else if ((b0 & 0xF0u) == 0xE0u && i + 2 < n) {
            const unsigned char b1 = static_cast<unsigned char>(str[i + 1]);
            const unsigned char b2 = static_cast<unsigned char>(str[i + 2]);
            if ((b1 & 0xC0u) == 0x80u && (b2 & 0xC0u) == 0x80u) {
                uint32_t cp = (static_cast<uint32_t>(b0 & 0x0Fu) << 12) |
                              (static_cast<uint32_t>(b1 & 0x3Fu) << 6) |
                              static_cast<uint32_t>(b2 & 0x3Fu);
                cp = detail::lowercaseVietnameseCodePoint(cp);
                detail::appendUtf8CodePoint(out, cp);
                i += 3;
                continue;
            }
        } else if ((b0 & 0xF8u) == 0xF0u && i + 3 < n) {
            const unsigned char b1 = static_cast<unsigned char>(str[i + 1]);
            const unsigned char b2 = static_cast<unsigned char>(str[i + 2]);
            const unsigned char b3 = static_cast<unsigned char>(str[i + 3]);
            if ((b1 & 0xC0u) == 0x80u && (b2 & 0xC0u) == 0x80u && (b3 & 0xC0u) == 0x80u) {
                out.push_back(str[i]);
                out.push_back(str[i + 1]);
                out.push_back(str[i + 2]);
                out.push_back(str[i + 3]);
                i += 4;
                continue;
            }
        }
        out.push_back(str[i]);
        ++i;
    }
    return out;
}

inline bool StringHelper::containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    return toLower(haystack).find(toLower(needle)) != std::string::npos;
}

inline double StringHelper::parsePrice(std::string priceStr) {
    priceStr = trim(priceStr);
    if (priceStr.empty()) return 0.0;

    bool negative = false;
    size_t pos = 0;
    if (priceStr[pos] == '-') {
        negative = true;
        ++pos;
    } else if (priceStr[pos] == '+') {
        ++pos;
    }

    std::string rawNum;
    int dotCount = 0;
    bool hasDigit = false;
    for (size_t i = pos; i < priceStr.size(); ++i) {
        const unsigned char uc = static_cast<unsigned char>(priceStr[i]);
        if (std::isdigit(uc)) {
            rawNum.push_back(priceStr[i]);
            hasDigit = true;
        } else if (priceStr[i] == '.') {
            rawNum.push_back('.');
            ++dotCount;
        }
    }
    if (!hasDigit || rawNum.empty()) return 0.0;

    std::string normalized;
    if (dotCount == 0) {
        normalized = rawNum;
    } else if (dotCount >= 2) {
        // Nhiều dấu chấm (vd: "10.600.000", "1.000.000"): chắc chắn là phân cách hàng nghìn VN
        for (char ch : rawNum) {
            if (ch != '.') normalized.push_back(ch);
        }
    } else {
        // Đúng 1 dấu chấm: phân biệt hàng nghìn VN ("100.000", "1.500") với số thập phân ("100000.50", "14000000.000000")
        const size_t dotPos = rawNum.find('.');
        const std::string intPart = rawNum.substr(0, dotPos);
        const std::string fracPart = rawNum.substr(dotPos + 1);
        if (fracPart.empty()) {
            normalized = intPart;
        } else if (fracPart.size() == 3 && !intPart.empty() && intPart.size() <= 3) {
            normalized = intPart + fracPart;
        } else {
            normalized = rawNum;
        }
    }

    if (normalized.empty()) return 0.0;
    try {
        double val = std::stod(normalized);
        if (val == 0.0) return 0.0;
        return negative ? -val : val;
    } catch (...) {
        return 0.0;
    }
}

inline std::string StringHelper::formatCurrency(double amount) {
    const long long val = std::llround(amount);
    const bool negative = (val < 0);
    const unsigned long long mag = negative
        ? (static_cast<unsigned long long>(-(val + 1)) + 1ULL)
        : static_cast<unsigned long long>(val);

    const std::string numStr = std::to_string(mag);
    std::string result;
    int count = 0;
    for (int i = static_cast<int>(numStr.length()) - 1; i >= 0; --i) {
        result = numStr[static_cast<size_t>(i)] + result;
        ++count;
        if (count % 3 == 0 && i > 0) {
            result = "." + result;
        }
    }
    if (negative && mag > 0) {
        result = "-" + result;
    }
    return result;
}

inline std::string StringHelper::formatMoneyDisk(double amount) {
    long long val = std::llround(amount);
    if (val == 0) val = 0;
    return std::to_string(val);
}

#endif // STRINGHELPER_H
