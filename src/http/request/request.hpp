#pragma once

#include "request_parser.hpp"
#include "../response/response.hpp"
#include "../../config/config.hpp"
#include "../../core/client.hpp"
#include "../../../toolbox/shared.hpp"

namespace http {

class Request {
 public:
    Request(toolbox::SharedPtr<Client> client);
    ~Request();
    void parseRawRequest(const std::string& rawRequest);
    void fetchConfig();
    void handleRequest();
    void sendResponse() const;

 private:
    http::RequestParser _parsedRequest;
    config::LocationConfig _config;
    http::Response _response;
    toolbox::SharedPtr<Client> _client;

    Request();
    Request(const Request& other);
    Request& operator=(const Request& other);
};

}
