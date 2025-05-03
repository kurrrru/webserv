#pragma once

#include "http_status.hpp"
#include "request/http_fields.hpp"

namespace http {

class FieldValidator {
 public:
    FieldValidator();
    ~FieldValidator();

    bool validateFieldLine(std::string& line);
    bool validateRequestHeaders(HTTPFields& fields, HttpStatus& hs);
    bool validateResponseHeaders(HTTPFields& fields, HttpStatus& hs);

 private:
    bool validateHostExists(HTTPFields& fields, HttpStatus& hs);
    bool validateContentHeaders(HTTPFields& fields, HttpStatus& hs);
    bool validateContentLength
        (HTTPFields::FieldMap::iterator contentLength, HttpStatus& hs);
    bool validateTransferEncoding
        (HTTPFields::FieldMap::iterator transferEncoding, HttpStatus& hs);
};

}