#include <string>

#include "http_namespace.hpp"
#include "case_insensitive_less.hpp"

namespace http {
namespace utils {
bool hasWhiteSpace(const std::string& str);
bool hasCtlChar(const std::string& str);
bool isDigitStr(const std::string& str);
bool isAlnumStr(const std::string& str);
void trimSpace(std::string* str);
void skipSpace(std::string* line);
bool isEqualCaseInsensitive(
        const std::string& str1, const std::string& str2);

}  // namespace utils
}  // namespace http
