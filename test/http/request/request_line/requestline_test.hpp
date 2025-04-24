// Copyright 2025 Ideal Broccoli

#pragma once

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "../../../src/http/request/request_parser.hpp"
#include "../../../src/http/request/http_fields.hpp"
#include "../../../src/http/http_namespace.hpp"
// #include "../../../src/http/http_status.hpp"

typedef std::vector<RequestLineTest> TestVector;
typedef std::pair<std::string, std::string> QueryPair;
typedef std::vector<QueryPair> QueryVec;
typedef std::map<std::string, std::string> QueryMap;  // 修正予定

class RequestLineTest {
 public:
    struct ExceptRequestLine {
        std::string method;
        std::string path;
        QueryVec queryVec;
        std::string version;
    };

    RequestLineTest() : _httpStatus(200) {}

    std::string _name;
    std::string _request;
    bool _isSuccessTest;
    std::size_t _httpStatus;
    ExceptRequestLine _exceptRequest;
};
