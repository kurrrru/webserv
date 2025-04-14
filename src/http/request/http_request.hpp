#pragma once

#include <string>
#include <map>

#include "http_fields.hpp"

class HTTPRequest {
 public:
    struct Body {
        Body() : isChunked(0), lastChunk(0) {}
        bool isChunked;
        bool lastChunk;
        std::string content;
        std::size_t contentLength;
        std::size_t recvedLength;
    };
    struct URI {
        std::string fullUri;
        std::string path;
        std::string fullQuery;
        std::map<std::string, std::string> queryMap;
        std::string fragment;  // client dependent
    };

    HTTPRequest() {
        body.contentLength = 0;
        body.recvedLength = 0;
    }
    ~HTTPRequest() {}

    std::string method;
    URI uri;
    std::string version;
    HTTPFields fields;
    Body body;

 private:
    HTTPRequest(const HTTPRequest& otheer);
    HTTPRequest& operator=(const HTTPRequest& other);
};
