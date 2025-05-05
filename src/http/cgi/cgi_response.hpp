#pragma once

#include <string>

#include "../request/http_fields.hpp"
#include "../http_status.hpp"

namespace http {
class CgiResponse {
 public:
    CgiResponse() { httpStatus.set(HttpStatus::EHttpStatus::UNSET); }
    ~CgiResponse() {}

    HttpStatus httpStatus;
    HTTPFields fields;
    std::string body;

 private:
    CgiResponse(const CgiResponse& other);
    CgiResponse& operator=(const CgiResponse& other);
};

}  // namespace http
