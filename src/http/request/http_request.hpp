#pragma once

#include <map>
#include <vector>
#include <string>
#include <limits>

#include "http_fields.hpp"
#include "../http_status.hpp"

namespace http {
class HTTPRequest {
 public:
    struct Body {
        Body() :
            isChunked(false),
            lastChunk(false),
            contentLength(std::numeric_limits<std::size_t>::max()),
            receivedLength(0) {
            }
        bool isChunked;
        bool lastChunk;
        std::string content;
        std::size_t contentLength;
        std::size_t receivedLength;
    };
    struct URI {
        std::string fullUri;
        std::vector<std::string> splitPath;
        std::string path;
        std::string fullQuery;
        std::map<std::string, std::string> queryMap;
    };

    HTTPRequest() {}
    ~HTTPRequest() {}

    HttpStatus httpStatus;
    std::string originalRequestLine;
    std::string method;
    URI uri;
    std::string version;
    HTTPFields fields;
    Body body;

 private:
    HTTPRequest(const HTTPRequest& other);
    HTTPRequest& operator=(const HTTPRequest& other);
};

}  // namespace http
