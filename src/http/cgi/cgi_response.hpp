#pragma once

#include <string>

#include "../request/http_fields.hpp"
#include "../http_status.hpp"

namespace http {
class CgiResponse {
 public:
    enum CgiType {
        DOCUMENT = 0,
        LOCAL_REDIRECT = 1,
        CLIENT_REDIRECT = 2,
        CLIENT_REDIRECT_DOCUMENT = 3,
        INVALID = 4
    };

    inline CgiResponse() : cgiType(INVALID) {
        httpStatus.set(HttpStatus::UNSET);
    }
    ~CgiResponse() {}

    void identifyCgiType();

    HttpStatus httpStatus;
    HTTPFields fields;
    std::string body;
    CgiType cgiType;

 private:
    CgiResponse(const CgiResponse& other);
    CgiResponse& operator=(const CgiResponse& other);
};

}  // namespace http
