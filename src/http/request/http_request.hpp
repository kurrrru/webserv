#pragma once

#include <string>

#include "http_fields.hpp"

class HTTPRequest {
   public:
    std::string method;
    std::string uri;
    std::string version;
    // uri
    std::string path;
    std::string query;  //
    std::map<std::string, std::string> queryMap;
    std::string fragment;  //

    HTTPFields fields;
    std::string body;
};