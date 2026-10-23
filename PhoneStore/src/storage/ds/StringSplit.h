#pragma once
// Định nghĩa StringHelper::split (chuyển nguyên văn từ utils/StringHelper.h cũ).
// Lý do tách: common/StringUtil.h không được include storage/ds, còn split trả về Vector<std::string>.
// Header này thuộc storage/ds (được phép include common). Ai gọi split phải include header này.

#include <sstream>
#include <string>

#include "Vector.h"
#include "../../common/StringUtil.h"

inline Vector<std::string> StringHelper::split(const std::string& str, char delimiter) {
    Vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
