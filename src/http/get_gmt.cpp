#include "get_gmt.hpp"

#include <string>
#include <ctime>

#include "../../toolbox/stepmark.hpp"

namespace http {

std::string getCurrentGMT() {
    char buffer[32];
    std::time_t now = std::time(NULL);
    std::tm* gmtm = std::gmtime(&now);
    if (gmtm == NULL) {
        toolbox::logger::StepMark::error("getCurrentGMT: Failed to get GMT time");
        return "";
    }
    if (!std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtm)) {
        toolbox::logger::StepMark::error("getCurrentGMT: Failed to format GMT time");
        return "";
    }
    return std::string(buffer);
}

}  // namespace http
