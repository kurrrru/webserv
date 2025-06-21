// Copyright 2025 Ideal Broccoli

#pragma once

#include<string>

#include "../request/http_fields.hpp"
#include "../http_status.hpp"
#include "../http_namespace.hpp"
#include "../string_utils.hpp"

namespace http {
class FieldValidator {
 public:
    FieldValidator() {}
    ~FieldValidator() {}

    static bool validateFieldLine(const std::string& line);
    static bool validateRequestHeaders(HTTPFields& fields, HttpStatus& hs);
    static bool validateCgiHeaders(HTTPFields& fields, HttpStatus& hs);

 private:
    FieldValidator(const FieldValidator& other);
    FieldValidator& operator=(const FieldValidator& other);

    static HttpStatus::EHttpStatus validateHostExists(HTTPFields& fields);
    static HttpStatus::EHttpStatus validateContentHeaders(HTTPFields& fields);
    static HttpStatus::EHttpStatus validateContentLength
        (HTTPFields::FieldMap::iterator contentLength);
    static HttpStatus::EHttpStatus validateTransferEncoding
        (HTTPFields::FieldMap::iterator transferEncoding);
};

}  // namespace http
