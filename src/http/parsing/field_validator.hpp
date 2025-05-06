#pragma once

#include<string>

#include "../request/http_fields.hpp"
#include "../http_status.hpp"
#include "../http_namespace.hpp"
#include "../string_utils.hpp"

namespace http {
class FieldValidator {
 public:
    struct Result {
        bool success;
        HttpStatus::EHttpStatus status;

        explicit Result(bool s = true,
            HttpStatus::EHttpStatus st = HttpStatus::OK)
            : success(s), status(st) {}
        operator bool() const { return success; }
        bool operator!() const { return !success; }
    };

    FieldValidator() {}
    ~FieldValidator() {}

    static bool validateFieldLine(std::string& line);
    static bool validateRequestHeaders(HTTPFields& fields, HttpStatus& hs);
    static bool validateCgiHeaders(HTTPFields& fields, HttpStatus& hs);

 private:
    FieldValidator(const FieldValidator& other);
    FieldValidator& operator=(const FieldValidator& other);

    static Result validateHostExists(HTTPFields& fields);
    static Result validateContentHeaders(HTTPFields& fields);
    static Result validateContentLength
        (HTTPFields::FieldMap::iterator contentLength);
    static Result validateTransferEncoding
        (HTTPFields::FieldMap::iterator transferEncoding);
};

}  // namespace http
