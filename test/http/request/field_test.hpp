// Copyright 2025 Ideal Broccoli

#include <string>

#include "../../../src/http/request/request_parser.hpp"
#include "../../../src/http/request/http_fields.hpp"
#include "../../../src/http/http_namespace.hpp"
#include "../../../src/http/http_status.hpp"

class FieldTest {
 public:
    FieldTest() : _httpStatus(http::OK) {}
    FieldTest(const FieldTest& src) :
        _name(src._name),
        _request(src._request),
        _isSuccessTest(src._isSuccessTest),
        _httpStatus(src._httpStatus),
        _exceptMap(src._exceptMap)
    {}
    std::string _name;
    std::string _request;
    bool _isSuccessTest;
    http::HttpStatus _httpStatus;

    http::HTTPFields::FieldMap _exceptMap;
};
