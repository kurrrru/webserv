#include <string>
#include <cctype>
#include <cstdlib>

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"
#include "http_namespace.hpp"
#include "case_insensitive_less.hpp"

namespace http {
namespace utils {
bool hasWhiteSpace(const std::string& str);
bool hasCtlChar(const std::string& str);
bool isUpperStr(const std::string& str);
bool isDigitStr(const std::string& str);
bool isAlnumStr(const std::string& str);
void trimSpace(std::string* str);
void skipSpace(std::string* line);
std::size_t getLineEndLen(std::string& line, std::size_t lineEndPos);
bool isEqualCaseInsensitive(
        const std::string& str1, const std::string& str2);
bool percentDecode(std::string& str, std::string* buf);

}  // namespace utils
}  // namespace http
