#pragma once

#include <string>

#include "../request/http_fields.hpp"
#include "../http_status.hpp"

namespace http {
class CgiResponse {
 public:
    inline CgiResponse() {
        httpStatus.set(HttpStatus::EHttpStatus::UNSET);
        fields.initFieldsMap();
    }
    ~CgiResponse() {}

    HttpStatus httpStatus;
    HTTPFields fields;
    std::string body;

 private:
    CgiResponse(const CgiResponse& other);
    CgiResponse& operator=(const CgiResponse& other);
};

}  // namespace http
