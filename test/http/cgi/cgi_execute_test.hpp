// Copyright 2023 webserv team
#pragma once

#include <string>
#include "../../../src/http/request/http_request.hpp"

void executeTest();

namespace cgi_execute_test {

bool runTest(const std::string& name, bool (*testFunc)());
http::HTTPRequest* createTestRequest(const std::string& method,
                            const std::string& path,
                            const std::string& query = "",
                            const std::string& body = "",
                            const std::string& contentType = "");
bool createTestScript(const std::string& filename,
                     const std::string& content,
                     bool executable);
std::string convertToString(size_t value);

}  // namespace cgi_execute_test
