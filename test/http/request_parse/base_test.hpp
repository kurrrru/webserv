// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>

#include "../../../src/http/request/http_fields.hpp"
#include "../../../src/http/http_namespace.hpp"
#include "../../../src/http/http_status.hpp"

class BaseTest {
 public:
    BaseTest() {}
    virtual ~BaseTest() {}
    BaseTest(const BaseTest& other) :
        _name(other._name),
        _request(other._request),
        _isSuccessTest(other._isSuccessTest),
        _httpStatus(other._httpStatus)
    {}
    std::string _name;
    std::string _request;
    bool _isSuccessTest;
    http::HttpStatus _httpStatus;
};
