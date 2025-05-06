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
    Result result;
    result = validateHostExists(fields);
    if (!result) {
        hs.set(result.status);
        return false;
    }
    result = validateContentHeaders(fields);
    if (!result) {
        hs.set(result.status);
        return false;
    }
    return true;
}

bool FieldValidator::validateCgiHeaders(HTTPFields& fields,
    HttpStatus& hs) {
    Result result;
    result = validateContentHeaders(fields);
    if (!result) {
        hs.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    return true;
}


FieldValidator::Result FieldValidator::validateHostExists(HTTPFields& fields) {
    if (fields.get().find(fields::HOST)->second.empty()) {
        toolbox::logger::StepMark::info(
            "FieldValidator: host does not exist");
        return Result(false, HttpStatus::BAD_REQUEST);
    }
    return Result(true, HttpStatus::OK);
}

FieldValidator::Result FieldValidator::validateContentHeaders
(HTTPFields& fields) {
    HTTPFields::FieldMap::iterator content_length =
        fields.get().find(fields::CONTENT_LENGTH);
    HTTPFields::FieldMap::iterator transfer_encoding =
        fields.get().find(fields::TRANSFER_ENCODING);
    if (!content_length->second.empty()) {
        if (!transfer_encoding->second.empty()) {
            toolbox::logger::StepMark::info(
                "FieldValidator: content-length and transfer-encoding must not "
                "coexist");
            return Result(false, HttpStatus::BAD_REQUEST);
        }
        return validateContentLength(content_length);
    } else if (!transfer_encoding->second.empty()) {
        return validateTransferEncoding(transfer_encoding);
    }
    return Result(true, HttpStatus::OK);
}

FieldValidator::Result FieldValidator::validateContentLength(
    HTTPFields::FieldMap::iterator contentLength) {
    if (!utils::isDigitStr(contentLength->second[0])) {
        toolbox::logger::StepMark::info(
            "FieldValidator: content-length is not number");
        return Result(false, HttpStatus::BAD_REQUEST);
    }
    for (std::size_t i = 1; i < contentLength->second.size(); ++i) {
        if (contentLength->second[0] != contentLength->second[i]) {
            toolbox::logger::StepMark::info(
                "FieldValidator: content-length has different multiple number");
            return Result(false, HttpStatus::BAD_REQUEST);
        }
    }
    std::size_t size = std::strtol(contentLength->second[0].c_str(), NULL, 10);
    if (size > fields::MAX_BODY_SIZE) {
        toolbox::logger::StepMark::info
            ("FieldValidator: content-length too large");
        return Result(false, HttpStatus::PAYLOAD_TOO_LARGE);
    }
    return Result(true, HttpStatus::OK);
}

FieldValidator::Result FieldValidator::validateTransferEncoding(
    HTTPFields::FieldMap::iterator transferEncoding) {
    for (std::size_t i = 0; i < transferEncoding->second.size(); ++i) {
        if ("chunked" != transferEncoding->second[i]) {
            toolbox::logger::StepMark::info
                ("FieldValidator: transfer-encoding not implemented");
            return Result(false, HttpStatus::NOT_IMPLEMENTED);
        }
    }
    return Result(true, HttpStatus::OK);
}

}  // namespace http
