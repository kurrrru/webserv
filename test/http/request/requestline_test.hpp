// Copyright 2025 Ideal Broccoli

#pragma once

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "../../../src/http/request/http_fields.hpp"
#include "../../../src/http/http_namespace.hpp"
#include "../../../src/http/http_status.hpp"
#include "base_test.hpp"

typedef std::pair<std::string, std::string> QueryPair;
typedef std::vector<QueryPair> QueryVec;
typedef std::map<std::string, std::string> QueryMap;  // 修正予定

class RequestLineTest : public BaseTest {
 public:
    struct ExceptRequestLine {
        std::string method;
        std::string path;
        QueryVec queryVec;
        std::string version;
    };
    RequestLineTest() : BaseTest() {}
    ~RequestLineTest() {}

    ExceptRequestLine _exceptRequest;
};
