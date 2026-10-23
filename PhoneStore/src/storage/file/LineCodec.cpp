#include "LineCodec.h"

namespace {

// Thoát các ký tự đặc biệt: '\' và xuống dòng luôn được thoát; `separator` là '|' (mức dòng) hoặc ';' (mức danh sách)
std::string escapeText(const std::string& text, char separator) {
    std::string out;
    for (char c : text) {
        if (c == '\\') out += "\\\\";
        else if (c == separator) { out += '\\'; out += separator; }
        else if (c == '\n') out += "\\n";
        else if (c == '\r') { /* bỏ qua '\r' để file luôn có dòng sạch */ }
        else out += c;
    }
    return out;
}

// Tách chuỗi theo `separator` không bị thoát, đồng thời giải mã các chuỗi thoát.
// Chuỗi thoát thuộc mức khác (ví dụ "\;" khi tách theo '|') được giữ nguyên để mức kia xử lý.
Result<Vector<std::string>> splitAndUnescape(const std::string& text, char separator) {
    Vector<std::string> parts;
    std::string current;
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '\\') {
            if (i + 1 >= text.size()) {
                return Result<Vector<std::string>>::fail(ErrorCode::ParseError,
                                                         "Ký tự thoát '\\' ở cuối dòng");
            }
            char next = text[++i];
            if (next == '\\') current += '\\';
            else if (next == separator) current += separator;
            else if (next == 'n') current += '\n';
            else { current += '\\'; current += next; } // thuộc mức khác: giữ nguyên
        } else if (c == separator) {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
    return Result<Vector<std::string>>::ok(std::move(parts));
}

std::string join(const Vector<std::string>& items, char separator) {
    std::string out;
    for (int i = 0; i < items.getSize(); ++i) {
        if (i > 0) out += separator;
        out += escapeText(items[i], separator);
    }
    return out;
}

} // namespace

std::string LineCodec::encode(const Vector<std::string>& fields) {
    return join(fields, '|');
}

Result<Vector<std::string>> LineCodec::decode(const std::string& line) {
    return splitAndUnescape(line, '|');
}

std::string LineCodec::encodeList(const Vector<std::string>& items) {
    return join(items, ';');
}

Result<Vector<std::string>> LineCodec::decodeList(const std::string& text) {
    if (text.empty()) return Result<Vector<std::string>>::ok(Vector<std::string>());
    return splitAndUnescape(text, ';');
}
