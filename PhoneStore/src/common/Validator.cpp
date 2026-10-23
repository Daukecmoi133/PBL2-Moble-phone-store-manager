#include "Validator.h"

#include "StringUtil.h"

bool Validator::isValidId(const std::string& id) {
    return isValidID(id);
}

bool Validator::isNotBlank(const std::string& text) {
    return !StringHelper::trim(text).empty();
}
