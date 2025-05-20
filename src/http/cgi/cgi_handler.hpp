#pragma once

#include <string>
#include <vector>

#include "../request/http_request.hpp"
#include "../response/response.hpp"
#include "../../config/config.hpp"
#include "cgi_execute.hpp"
#include "cgi_response_parser.hpp"

namespace http {
/**
 * CGI (Common Gateway Interface) request processing handler.
 * 
 * This class is responsible for managing the execution of CGI scripts 
 * and processing their output to generate HTTP responses. It provides:
 * 
 * - Detection of CGI requests through file extensions and configuration
 * - Script execution via the CgiExecute component
 * - Output parsing through CgiResponseParser
 * - Support for all CGI response types:
 *   - Document (regular content)
 *   - Local redirects (server-side)
 *   - Client redirects (browser-side)
 *   - Client redirect with document
 * 
 * The handler properly manages CGI environment variables, interprets CGI
 * output according to the specification, and handles error conditions with
 * appropriate HTTP status codes.
 * 
 * It also implements loop detection for internal redirects to prevent
 * infinite redirect cycles when CGI scripts redirect to each other.
 */
class CgiHandler {
 public:
    CgiHandler();
    ~CgiHandler();

    bool isCgiRequest(const std::string& targetPath,
                      const std::vector<std::string>& cgiExtension,
                      const std::string& cgiPath) const;
    bool handleRequest(const HTTPRequest& request,
                       Response& Response,
                       const std::string& rootPath,
                       const std::vector<std::string>& cgiExtension,
                       const std::string& cgiPath,
                       const config::Config& config);
    void setRedirectCount(size_t count) { _redirectCount = count; }

 private:
    CgiHandler(const CgiHandler& other);
    CgiHandler& operator=(const CgiHandler& other);

    struct RedirectInfo {
       std::string location;
       std::string host;
    };

    std::string getScriptPath(const HTTPRequest& request,
                              const std::string& rootPath) const;
    bool validateParameters(const std::string& scriptPath,
                            const std::string& interpreter,
                            const std::vector<std::string>& cgiExtension,
                            Response& Response) const;
    bool processCgiOutput(const std::string& output, Response& response, const config::Config& config);
    bool cgiTypeHandler(Response& response, const CgiResponse& cgiResponse, const config::Config& config);
    bool handleDocument(Response& response, const CgiResponse& cgiResponse);
    bool handleLocalRedirect(Response& response, const CgiResponse& cgiResponse, const config::Config& config);
    bool handleClientRedirect(Response& response, const CgiResponse& cgiResponse);
    bool handleClientRedirectDocument(Response& response, const CgiResponse& cgiResponse);
    bool validateRedirectRequest(Response& response, const CgiResponse& cgiResponse);
    RedirectInfo extractRedirectInfo(const CgiResponse& cgiResponse);
    Response executeInternalRequest(
                     const std::string& location,
                     const std::string& host,
                     size_t redirectCount,
                     const config::Config& config);
    void copyRedirectResponse(const Response& redirectResponse,
                        Response& response);
    CgiExecute _execute;
    size_t _redirectCount;
};

} // namespace http
