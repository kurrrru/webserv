#include <string>
#include <cstdlib>

#include "field_validator.hpp"

namespace http {
bool FieldValidator::validateFieldLine(std::string& line) {
    if (utils::hasCtlChar(line) || line.size() > fields::MAX_FIELDLINE_SIZE ||
        line.find(symbols::COLON) == std::string::npos) {
        return false;
    }
    return true;
}

bool FieldValidator::validateRequestHeaders(HTTPFields& fields,
                                            HttpStatus& hs) {
    if (!validateHostExists(fields, hs)) {
        return false;
    }
    if (!validateContentHeaders(fields, hs)) {
        return false;
    }
    return true;
}

bool FieldValidator::validateHostExists(HTTPFields& fields, HttpStatus& hs) {
    if (fields.get().find(fields::HOST)->second.empty()) {
        toolbox::logger::StepMark::info(
            "FieldValidator: host does not exist");
        hs.set(HttpStatus::BAD_REQUEST);
        return false;
    }
    return true;
}

bool FieldValidator::validateContentHeaders(HTTPFields& fields,
                                            HttpStatus& hs) {
    HTTPFields::FieldMap::iterator content_length =
        fields.get().find(fields::CONTENT_LENGTH);
    HTTPFields::FieldMap::iterator transfer_encoding =
        fields.get().find(fields::TRANSFER_ENCODING);
    if (!content_length->second.empty()) {
        if (!transfer_encoding->second.empty()) {
            toolbox::logger::StepMark::info(
                "FieldValidator: content-length and transfer-encoding must not "
                "coexist");
            hs.set(HttpStatus::BAD_REQUEST);
            return false;
        }
        return validateContentLength(content_length, hs);
    } else if (!transfer_encoding->second.empty()) {
        return validateTransferEncoding(transfer_encoding, hs);
    }
    return true;
}

bool FieldValidator::validateContentLength(
    HTTPFields::FieldMap::iterator contentLength, HttpStatus& hs) {
    if (!utils::isDigitStr(contentLength->second[0])) {
        toolbox::logger::StepMark::info(
            "FieldValidator: content-length is not number");
        hs.set(HttpStatus::BAD_REQUEST);
        return false;
    }
    for (std::size_t i = 1; i < contentLength->second.size(); ++i) {
        if (contentLength->second[0] != contentLength->second[i]) {
            toolbox::logger::StepMark::info(
                "FieldValidator: content-length has different multiple number");
            hs.set(HttpStatus::BAD_REQUEST);
            return false;
        }
    }
    std::size_t size = std::strtol(contentLength->second[0].c_str(), NULL, 10);
    if (size > fields::MAX_BODY_SIZE) {
        toolbox::logger::StepMark::info
            ("FieldValidator: content-length too large");
        hs.set(HttpStatus::PAYLOAD_TOO_LARGE);
        return false;
    }
    return true;
}

bool FieldValidator::validateTransferEncoding(
    HTTPFields::FieldMap::iterator transferEncoding, HttpStatus& hs) {
    for (std::size_t i = 0; i < transferEncoding->second.size(); ++i) {
        if ("chunked" != transferEncoding->second[i]) {
            toolbox::logger::StepMark::info
                ("FieldValidator: transfer-encoding not implemented");
            hs.set(HttpStatus::NOT_IMPLEMENTED);
            return false;
        }
    }
    return true;
}

}  // namespace http
