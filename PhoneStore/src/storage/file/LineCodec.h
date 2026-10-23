#pragma once
// Mã hóa/giải mã một dòng văn bản thành các trường.
// Quy ước (đặc tả mục 8): trường cách nhau bằng '|', phần tử danh sách cách nhau bằng ';',
// ký tự thoát '\'  (\|  \;  \\  \n).
//
// Hai mức độc lập:
//  - Mức dòng:    encode / decode        (escape '\', '|', xuống dòng)
//  - Mức danh sách: encodeList / decodeList (escape '\', ';', xuống dòng), kết quả dùng làm MỘT trường ở mức dòng.

#include <string>

#include "../ds/Vector.h"
#include "../../common/Result.h"

class LineCodec {
public:
    static std::string encode(const Vector<std::string>& fields);
    static Result<Vector<std::string>> decode(const std::string& line);

    static std::string encodeList(const Vector<std::string>& items);
    static Result<Vector<std::string>> decodeList(const std::string& text);
};
