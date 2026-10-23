#ifndef UTF8_HELPER_H
#define UTF8_HELPER_H

#include <string>
#include <glib.h>

namespace Utf8Helper {

// Chuyển chuỗi UTF-8 sang chữ thường an toàn (hỗ trợ tiếng Việt có dấu, không crash khi gõ IME)
inline std::string toLower(const std::string& input) {
    if (input.empty()) return "";
    gchar* down = g_utf8_strdown(input.c_str(), -1);
    if (!down) return input;
    std::string result(down);
    g_free(down);
    return result;
}

// Kiểm tra xem haystack có chứa needle không (không phân biệt hoa thường theo chuẩn UTF-8 tiếng Việt)
inline bool containsCaseInsensitive(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    if (haystack.empty()) return false;

    std::string hLower = toLower(haystack);
    std::string nLower = toLower(needle);
    return hLower.find(nLower) != std::string::npos;
}

} // namespace Utf8Helper

#endif // UTF8_HELPER_H
