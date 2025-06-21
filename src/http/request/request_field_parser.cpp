// Copyright 2025 Ideal Broccoli

#include <string>

#include "request_field_parser.hpp"

namespace http {
bool RequestFieldParser::isUnique(const std::string& key) {
    return key == fields::CONTENT_LENGTH || key == fields::TRANSFER_ENCODING;
}

void RequestFieldParser::handleInvalidFieldError(const std::string& key,
                                                HttpStatus& hs) {
    toolbox::logger::StepMark::info("RequestFieldParser: invalid " + key);
    hs.set(HttpStatus::BAD_REQUEST);
}

void RequestFieldParser::handleDuplicateFieldError(const std::string& key,
                                                HttpStatus& hs) {
    toolbox::logger::StepMark::info("RequestFieldParser: duplicate " + key);
    hs.set(HttpStatus::BAD_REQUEST);
}

}  // namespace http
