#pragma once

#include <exception>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

enum EStatusCode {
    CONTINUE = 100,
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    RESET_CONTENT = 205,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    TEMPORARY_REDIRECT = 307,
    PERMANENT_REDIRECT = 308,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    REQUEST_TIMEOUT = 408,
    LENGTH_REQUIRED = 411,
    CONTENT_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    I_AM_A_TEAPOT = 418,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    HTTP_VERSION_NOT_SUPPORTED = 505
};

class StatusCode {
   public:
    static const std::pair<std::string, std::string> getStatusPair(
        EStatusCode state);

   private:
    static std::map<EStatusCode, std::string>& getStatusMap();
    static bool _isInitialized;
    static std::map<EStatusCode, std::string> _statusMap;
};
