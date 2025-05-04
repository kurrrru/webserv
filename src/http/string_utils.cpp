#include <string>

#include "string_utils.hpp"
#include "http_namespace.hpp"
#include "case_insensitive_less.hpp"

namespace http {
namespace utils {

bool hasWhiteSpace(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (std::isspace(str[i])) {
            return true;
        }
    }
    return false;
}

bool hasCtlChar(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (std::iscntrl(str[i])) {
            return true;
        }
    }
    return false;
}

bool isDigitStr(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool isAlnumStr(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (!std::isalnum(str[i])) {
            return false;
        }
    }
    return true;
}

void trimSpace(std::string* str) {
    if (str->empty()) {
        return;
    }
    std::size_t frontPos = str->find_first_not_of(symbols::SP);
    if (frontPos == std::string::npos) {
        str->clear();
        return;
    }
    std::size_t rearPos = str->find_last_not_of(symbols::SP);
    *str = str->substr(frontPos, rearPos - frontPos + 1);
}

void skipSpace(std::string* line) {
    std::size_t not_sp_pos = line->find_first_not_of(symbols::SP);
    if (not_sp_pos == std::string::npos) {
        return;
    }
    *line = line->substr(not_sp_pos);
}

bool isEqualCaseInsensitive(const std::string& str1, const std::string& str2) {
    CaseInsensitiveLess less;
    return !less(str1, str2) && !less(str2, str1);
}

}
}