#include "string.hpp"

#include <fcntl.h>
#include <sstream>

namespace toolbox {
    std::string to_string(int val) {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    std::string trim(std::string& src, const std::string& sep) {
        if (src.length() == 0) {
            return "";
        }
        std::size_t pos = src.find(sep);
        std::string newStr;
        if (pos == std::string::npos) {
            src.swap(newStr);
            src.clear();
        } else {
            newStr = src.substr(0, pos);
            src.erase(0, pos + sep.length());
        }
        return newStr;
    }

    bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
        if (str1.length() != str2.length()) {
            return false;
        }
        if (str1 == str2) {
            return true;
        }
        return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(),
                          [](char c1, char c2) {
                              return std::tolower(static_cast<unsigned char>(c1)) ==
                                     std::tolower(static_cast<unsigned char>(c2));
                          });
    }
}
