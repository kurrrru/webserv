#pragma once

#include <string>

#include "../../../src/http/request/http_fields.hpp"
#include "../../../src/http/http_status.hpp"

class CgiResponseTest {
 public:
    std::string name;
    std::string request;
    bool isSuccessTest;
    http::HttpStatus httpStatus;
    http::HTTPFields::FieldMap exceptMap;
    std::string body;
};