#pragma once

#include <vector>
#include <string>

#include "request_parser.hpp"
#include "../response/response.hpp"
#include "../cgi/cgi_handler.hpp"
#include "../../config/config.hpp"
#include "../../../toolbox/shared.hpp"
#include "io_pending_state.hpp"

class Client;

namespace http {

class Request {
 public:
    /**
     * @brief Constructs a Request object with the given client.
     * @param client A pointer to the Client object
     * @param requestDepth The depth of the request, used to track local redirects
     * associated with this request.
     */
    Request(const Client* client, std::size_t requestDepth = 0);

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
     */
    void recvRequest();

    /**
     * @brief Sets the local redirect information for the request.
     * This is used to handle internal redirects within the server.
     * @param method The HTTP method (e.g., GET, POST)
     * @param path The path of the resource being requested
     * @param host The host header value
     */
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

    /**
     * @brief Returns the current I/O pending state of the request.
     * @return The current I/O pending state.
     */
    IOPendingState getIOPendingState() const {
        return _ioPendingState;
    }

    /**
     * @brief Returns the prepared HTTP response.
     * @return A copy of the Response object.
     */
    http::Response getResponse() const;

    /**
     * @brief Sets the redirect count for the request.
     * This is used to track how many redirects have occurred
     * for this request, which is important for handling local redirects.
     * @param count The number of redirects to set.
     */
    void setRedirectCount(std::size_t count);

    bool isErrorInternalRedirect() const;
    void setErrorInternalRedirect();

    /**
     * @brief Returns the upload path for the request.
     * @return A const reference to the upload path string.
     */
    const std::string& getUploadPath() const;

 private:
    http::RequestParser _parsedRequest;
    config::LocationConfig _config;
    http::Response _response;
    const Client* _client;
    std::size_t _requestDepth;
    IOPendingState _ioPendingState;
    toolbox::SharedPtr<http::Request> _errorPageRequest;
    CgiHandler _cgiHandler;
    bool _isErrorInternalRedirect;

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
    void processReturnWithContent(std::size_t statusCode,
                                  const std::string& content);
    void processReturnWithoutContent(std::size_t statusCode);
    bool isRedirectStatus(std::size_t statusCode) const;
    bool hasDefaultErrorPage(std::size_t statusCode) const;
    bool isMinimalResponse(std::size_t statusCode) const;
    void setRedirectResponse(std::size_t statusCode,
                             const std::string& location);
    void setTextResponse(const std::string& content);
    void setHtmlErrorResponse(std::size_t statusCode);
    void setEmptyTextResponse();
    std::string generateDefaultBody(std::size_t statusCode);
    bool selectLocation(
        const toolbox::SharedPtr<config::ServerConfig>& server);
    toolbox::SharedPtr<config::LocationConfig> findDeepestMatchingLocation(
        const std::vector<toolbox::SharedPtr
        <config::LocationConfig> >& locations,
        const std::string& path);
};

}  // namespace http
