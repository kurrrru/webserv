#include <string>

#include "string_utils.hpp"

namespace http {
namespace utils {
bool hasWhiteSpace(const std::string& str) {
    int (*is_space)(int) = std::isspace;
    return toolbox::any_true(str.begin(), str.end(), is_space);
}

bool hasCtlChar(const std::string& str) {
    int (*is_cntrl)(int) = std::iscntrl;
    return toolbox::any_true(str.begin(), str.end(), is_cntrl);
}

bool isDigitStr(const std::string& str) {
    int (*is_digit)(int) = std::isdigit;
    return toolbox::all_true(str.begin(), str.end(), is_digit);
}

bool isAlnumStr(const std::string& str) {
    int (*is_alnum)(int) = std::isalnum;
    return toolbox::all_true(str.begin(), str.end(), is_alnum);
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

}  // namespace utils
}  // namespace http
