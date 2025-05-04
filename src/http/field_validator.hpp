#pragma once

#include<string>

#include "http_status.hpp"
#include "request/http_fields.hpp"

namespace http {

class FieldValidator {
 public:
    FieldValidator() {}
    ~FieldValidator() {}

    static bool validateFieldLine(std::string& line);
    static bool validateRequestHeaders(HTTPFields& fields, HttpStatus& hs);
    // static bool validateResponseHeaders(HTTPFields& fields, HttpStatus& hs);

 private:
    static bool validateHostExists(HTTPFields& fields, HttpStatus& hs);
    static bool validateContentHeaders(HTTPFields& fields, HttpStatus& hs);
    static bool validateContentLength
        (HTTPFields::FieldMap::iterator contentLength, HttpStatus& hs);
    static bool validateTransferEncoding
        (HTTPFields::FieldMap::iterator transferEncoding, HttpStatus& hs);
};

}  // namespace http
