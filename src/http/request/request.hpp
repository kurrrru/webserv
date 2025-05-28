#pragma once

#include "request_parser.hpp"
#include "../response/response.hpp"
#include "../../config/config.hpp"
#include "../../core/client.hpp"
#include "../../../toolbox/shared.hpp"

namespace http {

class Request {
 public:
    /**
     * @brief Constructs a Request object with the given client.
     * @param client A shared pointer to the Client object
     * @param requestDepth The depth of the request, used to track local redirects
     * associated with this request.
     */
    Request(const toolbox::SharedPtr<Client>& client, std::size_t requestDepth = 0);

    /**
     * @brief Destructor for the Request object.
     */
    ~Request();

    /**
     * @brief recieve and parses the raw HTTP request string.
     * @note fetchConfig() is called inside this method to retrieve
     * the configuration for the request.
     * @return True if the request was fully received and parsed,
     * false otherwise.
     */
    bool recvRequest();
    bool performRecv(std::string& recievedData);
    bool loadConfig();
    bool validateBodySize();

    void setLocalRedirectInfo(const std::string& method, 
        const std::string& path, const std::string& host);

    /**
    * @brief Fetches the configuration for the current request.
    */
    void fetchConfig();

    /**
     * @brief Handles the HTTP request and prepares a response.
     */
    void handleRequest();

    /**
     * @brief Sends the prepared HTTP response to the client.
     */
    void sendResponse() const;

 private:
    http::RequestParser _parsedRequest;
    config::LocationConfig _config;
    http::Response _response;
    toolbox::SharedPtr<Client> _client;
    std::size_t _requestDepth;

    Request();
    Request(const Request& other);
    Request& operator=(const Request& other);

};

}
