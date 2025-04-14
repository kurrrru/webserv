#include "string.hpp"

#include <fcntl.h>
#include <sstream>
#include <iostream>

namespace toolbox {
    std::string to_string(int val) {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    std::string trim(std::string& src, const std::string& sep) {
        std::size_t pos = src.find(sep);
        std::string formerPart;
        if (pos != std::string::npos) {
            formerPart = src.substr(pos + sep.size());
            src.erase(pos);
        }
        src.swap(formerPart);
        return formerPart;
    }

    bool caseInsensitiveCharCompare(char c1, char c2) {
        return std::tolower(static_cast<unsigned char>(c1)) ==
                std::tolower(static_cast<unsigned char>(c2));
    }

    bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
        if (str1.length() != str2.length()) {
            return false;
        }
        if (str1 == str2) {
            return true;
        }
        return std::equal(str1.begin(), str1.end(), str2.begin(),
                            caseInsensitiveCharCompare);
    }
}
