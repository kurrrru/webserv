#include <string>

namespace http {

namespace utils {
    bool hasWhiteSpace(const std::string& str);
    bool hasCtlChar(const std::string& str);
    bool isDigitStr(const std::string& str);
    bool isAlnumStr(const std::string& str);
    void trimSpace(std::string* str);
    bool isEqualCaseInsensitive(const std::string& str1, const std::string& str2);
}
}