#pragma once

#include <string>

#include "../../../src/config/config.hpp"
#include "../../../src/config/config_location.hpp"
#include "../../../src/http/response/response.hpp"
#include "../../../src/http/request/request_parser.hpp"
#include "../../../src/http/cgi/cgi_handler.hpp"

namespace http {
namespace request {

class Request {
 public:
    Request();
    ~Request();

    void parseRawRequest(const std::string& rawRequest);
    void fetchConfig(const config::Config& config);
    void handleRequest(const config::Config& config);
    void sendRequest(const int client_fd) const;
    void initializeRequest(const std::string& method,
                        const std::string& path,
                        const std::string& host);
    Response& getResponse() { return _response; }
    const Response& getResponse() const { return _response; }
    RequestParser& getRequest() { return _request; }
    const RequestParser& getRequest() const { return _request; }
    void setCgiRedirectCount(size_t count) { _cgiRedirectCount = count; }
    int getCgiRedirectCount() const { return _cgiRedirectCount; }

 private:
    Request(const Request& other);
    Request& operator=(const Request& other);

    bool processReturn(const config::Return& returnValue);
    RequestParser _request;
    toolbox::SharedPtr<config::LocationConfig> _config;
    Response _response;
    std::string _serverName;
    std::string _serverPort;
    CgiHandler _cgiHandler;
    size_t _cgiRedirectCount;
    // void runHead();
    // void runPost();
    // void runDelete();
};

}  // namespace request
}  // namespace http
