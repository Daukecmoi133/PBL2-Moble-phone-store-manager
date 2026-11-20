#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <string>
#include "../core/Vector.h"

/**
 * @file TextCodec.h
 * @brief Bộ mã hóa / giải mã ký tự đặc biệt cho cơ sở dữ liệu dạng văn bản (.txt).
 *
 * Quy ước escape 2 cấp cùng 1 bộ giải mã unescape():
 *  - Cấp cột (ngăn bởi '|'): escape '\', '|', '\n', '\r' -> '\\', '\p', '\n', '\r'
 *  - Cấp danh sách con (ngăn bởi ';' và ','): escape thêm ';', ',' -> '\s', '\c'
 *  - Bộ unescape() giải mã đồng nhất cả 6 token trên, bảo đảm song ánh:
 *      unescape(escapeField(s)) == s
 *      unescape(escapeSubField(s)) == s
 */
class TextCodec {
public:
    static std::string escapeField(const std::string& input) {
        std::string out;
        out.reserve(input.size() + 8);
        for (char ch : input) {
            switch (ch) {
                case '\\': out += "\\\\"; break;
                case '|':  out += "\\p";  break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                default:   out.push_back(ch); break;
            }
        }
        return out;
    }

    static std::string escapeSubField(const std::string& input) {
        std::string out;
        out.reserve(input.size() + 8);
        for (char ch : input) {
            switch (ch) {
                case '\\': out += "\\\\"; break;
                case '|':  out += "\\p";  break;
                case ';':  out += "\\s";  break;
                case ',':  out += "\\c";  break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                default:   out.push_back(ch); break;
            }
        }
        return out;
    }

    static std::string unescape(const std::string& input) {
        std::string out;
        out.reserve(input.size());
        for (std::size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '\\' && i + 1 < input.size()) {
                const char next = input[i + 1];
                switch (next) {
                    case '\\': out.push_back('\\'); ++i; break;
                    case 'p':  out.push_back('|');  ++i; break;
                    case 's':  out.push_back(';');  ++i; break;
                    case 'c':  out.push_back(',');  ++i; break;
                    case 'n':  out.push_back('\n'); ++i; break;
                    case 'r':  out.push_back('\r'); ++i; break;
                    default:
                        // Tương thích ngược nếu dữ liệu cũ có dấu '\' không phải escape token
                        out.push_back('\\');
                        out.push_back(next);
                        ++i;
                        break;
                }
            } else {
                out.push_back(input[i]);
            }
        }
        return out;
    }

    /**
     * @brief Tách chuỗi theo delimiter nhưng bảo toàn các trường rỗng (kể cả cuối dòng).
     * Khắc phục hạn chế của std::getline vốn bỏ qua trường rỗng sau dấu '|' cuối cùng.
     */
    static Vector<std::string> splitFields(const std::string& line, char delimiter) {
        Vector<std::string> tokens;
        if (line.empty()) return tokens;

        std::size_t start = 0;
        for (;;) {
            const std::size_t pos = line.find(delimiter, start);
            if (pos == std::string::npos) {
                tokens.push_back(line.substr(start));
                break;
            }
            tokens.push_back(line.substr(start, pos - start));
            start = pos + 1;
        }
        return tokens;
    }
};

#endif // TEXTCODEC_H
