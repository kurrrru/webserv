#pragma once

#include <string>
#include <vector>
#include <map>
#include "../../../src/http/request/http_request.hpp"
#include "../../../src/config/config.hpp"

struct CgiHandleTest {
 public:
    std::string name;
    std::string scriptPath;
    std::string scriptContent;
    http::HTTPRequest* request;
    bool shouldSucceed;
    bool isExecutable;

    std::vector<std::string> cgiExtension;
    std::string cgiPass;
    std::string rootPath;
    bool validateStatus;
    int expectedStatus;
    bool validateLocation;
    std::string expectedLocation;
    bool validateHeader;
    std::map<std::string, std::string> expectedHeaders;
    bool validateBody;
    std::string expectedBody;

    CgiHandleTest() :
        request(new http::HTTPRequest()),
        shouldSucceed(true),
        isExecutable(true),
        validateStatus(false),
        expectedStatus(200),
        validateLocation(false),
        validateHeader(false),
        validateBody(false),
        expectedBody("") {}

    ~CgiHandleTest() {
        delete request;
    }
};
