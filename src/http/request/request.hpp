#pragma once

#include <vector>
#include <string>

#include "request_parser.hpp"
#include "../response/response.hpp"
#include "../../config/config.hpp"
#include "../../../toolbox/shared.hpp"

class Client;

namespace http {

enum IOPendingState {
  REQUEST_READING,
  CGI_BODY_SENDING,
  CGI_OUTPUT_READING,
  CGI_LOCAL_REDIRECT_IO_PENDING,
  ERROR_LOCAL_REDIRECT_IO_PENDING,
  RESPONSE_SENDING,
  NO_IO_PENDING
};

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
     * @brief Runs the request processing, including receiving the request,
     * fetching configuration, handling the request, and sending the response.
     */
    void run();

    /**
     * @brief recieve and parses the raw HTTP request string.
     * @note fetchConfig() is called inside this method to retrieve
     * the configuration for the request.
     * @return True if the request was fully received and parsed,
     * false otherwise.
     */
    bool recvRequest();

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
    void sendResponse();

 private:
    http::RequestParser _parsedRequest;
    config::LocationConfig _config;
    http::Response _response;
    toolbox::SharedPtr<Client> _client;
    std::size_t _requestDepth;
    IOPendingState _ioPendingState;

    Request();
    Request(const Request& other);
    Request& operator=(const Request& other);

    // recvRequest helper methods
    bool performRecv(std::string& receivedData);
    bool loadConfig();
    bool isValidBodySize();
    // fetchConfig helper methods
    toolbox::SharedPtr<config::ServerConfig> selectServer();
    bool extractCandidateServers(
        const std::vector<toolbox::SharedPtr<config::ServerConfig> >& servers,
        std::vector<toolbox::SharedPtr<config::ServerConfig> >& candidateServers);
    std::string extractHostName();
    toolbox::SharedPtr<config::ServerConfig> matchServerByName(
        const std::vector<toolbox::SharedPtr<config::ServerConfig> >& servers,
        const std::string& hostName);
    bool processReturn(const config::Return& returnValue);
    void processReturnWithContent(size_t statusCode,
                                const std::string& content);
    void processReturnWithoutContent(size_t statusCode);
    bool isRedirectStatus(size_t statusCode) const;
    bool hasDefaultErrorPage(size_t statusCode) const;
    bool isMinimalResponse(size_t statusCode) const;
    void setRedirectResponse(size_t statusCode, const std::string& location);
    void setTextResponse(const std::string& content);
    void setHtmlErrorResponse(size_t statusCode);
    void setEmptyTextResponse();
    std::string generateDefaultBody(size_t statusCode);
    bool selectLocation(
        const toolbox::SharedPtr<config::ServerConfig>& server);
    toolbox::SharedPtr<config::LocationConfig> findDeepestMatchingLocation(
        const std::vector<toolbox::SharedPtr
        <config::LocationConfig> >& locations,
        const std::string& path);
};

}  // namespace http
