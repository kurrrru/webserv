#include <string>

#include "cgi_field_parser.hpp"

namespace http {
bool CgiFieldParser::isUnique(const std::string& key) {
    return key == fields::CONTENT_LENGTH ||
        key == fields::TRANSFER_ENCODING ||
        key == fields::CONTENT_TYPE ||
        key == fields::LOCATION;
}

void CgiFieldParser::handleInvalidFieldError
(const std::string& key, HttpStatus& hs) {
    toolbox::logger::StepMark::info("RequestFieldParser: invalid " + key);
    hs.set(HttpStatus::INTERNAL_SERVER_ERROR);
}

void CgiFieldParser::handleDuplicateFieldError
(const std::string& key, HttpStatus& hs) {
    toolbox::logger::StepMark::info("RequestFieldParser: duplicate " + key);
    hs.set(HttpStatus::INTERNAL_SERVER_ERROR);
}

}  // namespace http
