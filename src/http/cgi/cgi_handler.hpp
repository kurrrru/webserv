#pragma once

#include <string>
#include <vector>

#include "../request/http_request.hpp"
#include "../request/io_pending_state.hpp"
#include "../http_namespace.hpp"
#include "../../config/config.hpp"
#include "../../core/client.hpp"
#include "cgi_execute.hpp"
#include "cgi_response_parser.hpp"
#include "cgi_response.hpp"

namespace http {
class CgiHandler {
 public:
    struct RedirectInfo {
       std::string location;
       std::string host;
    };

    CgiHandler();
    ~CgiHandler();

    bool isCgiRequest(const std::string& targetPath,
                      const std::vector<std::string>& cgiExtension,
                      const std::string& cgiPath) const;
    IOPendingState handleRequest(const HTTPRequest& request,
                       Response& Response,
                       const Client* client,
                       const config::LocationConfig& config,
                       const http::IOPendingState ioPendingState);
    void setRedirectCount(size_t count) { _redirectCount = count; }
    void reset();
    void forceTerminate();

 private:
    CgiHandler(const CgiHandler& other);
    CgiHandler& operator=(const CgiHandler& other);

    IOPendingState executeInitialCgiRequest(
                        const HTTPRequest& request,
                        Response& response,
                        const config::LocationConfig& locationConfig);
    IOPendingState continueCgiBodySending(Response& response);
    IOPendingState continueCgiOutputReading(Response& response);
    IOPendingState handleExecuteResult(CgiExecute::ExecuteResult result,
                                      Response& response);
    bool validateParameters(const std::string& scriptPath,
                            const std::string& interpreter,
                            const std::vector<std::string>& cgiExtension,
                            Response& Response) const;
    IOPendingState processCgiResponse(const CgiResponse& cgiResponse, Response& response);
    bool handleDocument(Response& response, const CgiResponse& cgiResponse);
    IOPendingState handleLocalRedirect(Response& response, const CgiResponse& cgiResponse);
    bool handleClientRedirect(Response& response, const CgiResponse& cgiResponse);
    bool handleClientRedirectDocument(Response& response, const CgiResponse& cgiResponse);
    bool validateRedirectRequest(Response& response, const CgiResponse& cgiResponse);
    RedirectInfo extractRedirectInfo(const CgiResponse& cgiResponse);
    IOPendingState executeInternalRequest(
                     const std::string& location,
                     const std::string& host,
                     size_t redirectCount);
    void copyCgiResponseToResponse(const CgiResponse& cgiResponse,
                            Response& response);
    std::string buildScriptPath(const std::string& root,
                               const std::string& uriPath,
                               const std::vector<std::string>& cgiExtensions) const;
    CgiExecute _execute;
    size_t _redirectCount;
    const Client* _client;
};

}  // namespace http
