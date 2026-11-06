#ifndef UTF8_HELPER_H
#define UTF8_HELPER_H

#include <string>
#include <glib.h>

namespace Utf8Helper {

// Chuẩn hóa Unicode sang dạng NFC (Precomposed)
inline std::string normalizeNfc(const std::string& input) {
    if (input.empty()) return "";
    gchar* norm = g_utf8_normalize(input.c_str(), -1, G_NORMALIZE_NFC);
    if (!norm) return input;
    std::string result(norm);
    g_free(norm);
    return result;
}

// Chuyển chuỗi UTF-8 sang chữ thường an toàn (hỗ trợ tiếng Việt có dấu, không crash khi gõ IME)
inline std::string toLower(const std::string& input) {
    if (input.empty()) return "";
    gchar* down = g_utf8_strdown(input.c_str(), -1);
    if (!down) return input;
    std::string result(down);
    g_free(down);
    return result;
}

// Bỏ dấu tiếng Việt và chuẩn hóa về ký tự ASCII cơ bản (ví dụ: "Điện thoại" -> "dien thoai")
inline std::string stripAccents(const std::string& input) {
    if (input.empty()) return "";
    std::string s = toLower(normalizeNfc(input));
    std::string result;
    result.reserve(s.size());

    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) {
            result.push_back(static_cast<char>(c));
            i += 1;
        } else if (c == 0xC3) { // 2-byte UTF-8 starting with 0xC3
            if (i + 1 < s.size()) {
                unsigned char c2 = static_cast<unsigned char>(s[i + 1]);
                switch (c2) {
                    case 0xA0: case 0xA1: case 0xA2: case 0xA3: result.push_back('a'); break; // à, á, â, ã
                    case 0xA8: case 0xA9: case 0xAA: result.push_back('e'); break;             // è, é, ê
                    case 0xAC: case 0xAD: result.push_back('i'); break;                         // ì, í
                    case 0xB2: case 0xB3: case 0xB4: case 0xB5: result.push_back('o'); break; // ò, ó, ô, õ
                    case 0xB9: case 0xBA: result.push_back('u'); break;                         // ù, ú
                    case 0xBD: result.push_back('y'); break;                                     // ý
                    default:
                        result.push_back(static_cast<char>(c));
                        result.push_back(static_cast<char>(c2));
                        break;
                }
                i += 2;
            } else {
                result.push_back(static_cast<char>(c));
                i += 1;
            }
        } else if (c == 0xC4) { // 2-byte UTF-8 starting with 0xC4
            if (i + 1 < s.size()) {
                unsigned char c2 = static_cast<unsigned char>(s[i + 1]);
                if (c2 == 0x83) { // ă
                    result.push_back('a');
                } else if (c2 == 0x91) { // đ
                    result.push_back('d');
                } else if (c2 == 0xA9) { // ĩ
                    result.push_back('i');
                } else {
                    result.push_back(static_cast<char>(c));
                    result.push_back(static_cast<char>(c2));
                }
                i += 2;
            } else {
                result.push_back(static_cast<char>(c));
                i += 1;
            }
        } else if (c == 0xC5) { // 2-byte UTF-8 starting with 0xC5
            if (i + 1 < s.size()) {
                unsigned char c2 = static_cast<unsigned char>(s[i + 1]);
                if (c2 == 0xA9) { // ũ
                    result.push_back('u');
                } else {
                    result.push_back(static_cast<char>(c));
                    result.push_back(static_cast<char>(c2));
                }
                i += 2;
            } else {
                result.push_back(static_cast<char>(c));
                i += 1;
            }
        } else if (c == 0xC6) { // 2-byte UTF-8 starting with 0xC6
            if (i + 1 < s.size()) {
                unsigned char c2 = static_cast<unsigned char>(s[i + 1]);
                if (c2 == 0xA1) { // ơ
                    result.push_back('o');
                } else if (c2 == 0xB0) { // ư
                    result.push_back('u');
                } else {
                    result.push_back(static_cast<char>(c));
                    result.push_back(static_cast<char>(c2));
                }
                i += 2;
            } else {
                result.push_back(static_cast<char>(c));
                i += 1;
            }
        } else if (c == 0xCC || c == 0xCD) { // Combining diacritical marks in NFD
            i += 2; // Bỏ qua dấu kết hợp
        } else if (c == 0xE1) { // 3-byte UTF-8 starting with 0xE1
            if (i + 2 < s.size()) {
                unsigned char c2 = static_cast<unsigned char>(s[i + 1]);
                unsigned char c3 = static_cast<unsigned char>(s[i + 2]);
                if (c2 == 0xBA) {
                    if (c3 >= 0xA1 && c3 <= 0xB7) { // ạ, ả, ấ, ầ, ẩ, ẫ, ậ, ắ, ằ, ẳ, ẵ, ặ
                        result.push_back('a');
                    } else if (c3 >= 0xB9 && c3 <= 0xBF) { // ẹ, ẻ, ẽ, ế
                        result.push_back('e');
                    } else {
                        result.push_back(static_cast<char>(c));
                        result.push_back(static_cast<char>(c2));
                        result.push_back(static_cast<char>(c3));
                    }
                } else if (c2 == 0xBB) {
                    if (c3 >= 0x81 && c3 <= 0x87) { // ề, ể, ễ, ệ
                        result.push_back('e');
                    } else if (c3 == 0x89 || c3 == 0x8B) { // ỉ, ị
                        result.push_back('i');
                    } else if (c3 >= 0x8D && c3 <= 0xA3) { // ọ, ỏ, ố, ồ, ổ, ỗ, ộ, ớ, ờ, ở, ỡ, ợ
                        result.push_back('o');
                    } else if (c3 >= 0xA5 && c3 <= 0xB1) { // ụ, ủ, ứ, ừ, ử, ữ, ự
                        result.push_back('u');
                    } else if (c3 >= 0xB3 && c3 <= 0xB9) { // ỳ, ỵ, ỷ, ỹ
                        result.push_back('y');
                    } else {
                        result.push_back(static_cast<char>(c));
                        result.push_back(static_cast<char>(c2));
                        result.push_back(static_cast<char>(c3));
                    }
                } else {
                    result.push_back(static_cast<char>(c));
                    result.push_back(static_cast<char>(c2));
                    result.push_back(static_cast<char>(c3));
                }
                i += 3;
            } else {
                result.push_back(static_cast<char>(c));
                i += 1;
            }
        } else {
            int len = g_utf8_skip[c];
            if (len > 0 && i + len <= s.size()) {
                result.append(s, i, len);
                i += len;
            } else {
                result.push_back(static_cast<char>(c));
                i += 1;
            }
        }
    }
    return result;
}

// Kiểm tra xem haystack có chứa needle không:
// - Chuẩn hóa NFC trước khi so sánh.
// - Không phân biệt hoa thường theo chuẩn UTF-8 tiếng Việt.
// - Nếu từ khóa người dùng không có dấu, tự động so khớp linh hoạt trên văn bản đã bỏ dấu (ví dụ "dien thoai" khớp "Điện thoại").
// - Nếu từ khóa người dùng có dấu, yêu cầu khớp chính xác dấu để tránh so khớp nhầm (ví dụ "đỏ" không khớp "đồ").
inline bool containsCaseInsensitive(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    if (haystack.empty()) return false;

    std::string hNorm = normalizeNfc(haystack);
    std::string nNorm = normalizeNfc(needle);

    std::string hLower = toLower(hNorm);
    std::string nLower = toLower(nNorm);

    // 1. So khớp trực tiếp chữ thường chuẩn Unicode
    if (hLower.find(nLower) != std::string::npos) {
        return true;
    }

    // 2. So khớp không dấu
    std::string nStripped = stripAccents(nLower);
    if (nStripped == nLower) {
        // Từ khóa không có dấu: so khớp trên văn bản đã bỏ dấu
        std::string hStripped = stripAccents(hLower);
        return hStripped.find(nStripped) != std::string::npos;
    }

    return false;
}

} // namespace Utf8Helper

#endif // UTF8_HELPER_H
