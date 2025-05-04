// Copyright 2025 Ideal Broccoli

#pragma once

#include <map>
#include <vector>
#include <string>

#include "http_fields.hpp"
#include "../http_status.hpp"

namespace http {
class HTTPRequest {
 public:
    struct Body {
        Body() :
            isChunked(false),
            lastChunk(false),
            contentLength(0),
            recvedLength(0) {
            }
        bool isChunked;
        bool lastChunk;
        std::string content;
        std::size_t contentLength;
        std::size_t recvedLength;
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
