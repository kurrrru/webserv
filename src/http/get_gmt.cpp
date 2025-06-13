#include "get_gmt.hpp"

#include <iostream>
#include <string>
#include <ctime>

namespace http {

std::string getCurrentGMT() {
    char buffer[32];
    std::time_t now = std::time(NULL);
    std::tm* gmtm = std::gmtime(&now);
    if (gmtm == NULL) {
        return "";
    }
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtm);
    return std::string(buffer);
}

}  // namespace http
