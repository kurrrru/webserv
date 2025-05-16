#include <string>
#include <iostream>

#include "string_utils.hpp"

namespace http {
namespace utils {
bool hasWhiteSpace(const std::string& str) {
    int (*isSpace)(int) = std::isspace;
    return toolbox::any_true(str.begin(), str.end(), isSpace);
}

bool hasCtlChar(const std::string& str) {
    int (*isCntrl)(int) = std::iscntrl;
    return toolbox::any_true(str.begin(), str.end(), isCntrl);
}

bool isUpperStr(const std::string& str) {
    int (*isUpper)(int) = std::isupper;
    return toolbox::all_true(str.begin(), str.end(), isUpper);
}

bool isDigitStr(const std::string& str) {
    int (*isDigit)(int) = std::isdigit;
    return toolbox::all_true(str.begin(), str.end(), isDigit);
}

bool isAlnumStr(const std::string& str) {
    int (*isAlnum)(int) = std::isalnum;
    return toolbox::all_true(str.begin(), str.end(), isAlnum);
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

std::size_t getLineEndLen(std::string& line, std::size_t lineEndPos) {
    if (line.find(symbols::CRLF) == lineEndPos) {
        return 2;
    } else if (line.find(symbols::LF) == lineEndPos) {
        return 1;
    }
    return 0;
}

bool isEqualCaseInsensitive(const std::string& str1, const std::string& str2) {
    CaseInsensitiveLess less;
    return !less(str1, str2) && !less(str2, str1);
}

std::string decodeHex(const std::string& hexStr) {
    if (hexStr.size() != 2 || !isxdigit(hexStr[0]) || !isxdigit(hexStr[1])) {
        return "";
    }

    std::string decodedStr;
    char* endptr = NULL;
    std::size_t hex = strtol(hexStr.c_str(), &endptr, 16);
    decodedStr = static_cast<char>(hex);
    if (*endptr != '\0' || hex == '\0') {
        return "";
    }
    return decodedStr;
}

bool percentDecode(std::string& str, std::string* buf) {
    std::size_t pos = str.find(symbols::PERCENT);
    if (pos == std::string::npos) {
        *buf += str;
        return true;
    }

    std::size_t start = 0;
    while (pos != std::string::npos) {
        *buf += str.substr(start, pos - start);

        if (pos + 2 >= str.length()) {  // 2 hex digits
            return false;
        }

        std::string hexStr = str.substr(pos + 1, 2);
        std::string decodedStr = decodeHex(hexStr);
        if (decodedStr.empty()) {
            return false;
        }
        *buf += decodedStr;

        start = pos + 3;  // % + 2 hex digits
        pos = str.find(symbols::PERCENT, start);
    }
    if (start < str.length()) {
        *buf += str.substr(start);
    }
    return true;
}

}  // namespace utils
}  // namespace http
