#include <string>
#include <vector>
#include <algorithm>

#include "../cgi/cgi_handler.hpp"
#include "request.hpp"
#include "../response/server_method_handler.hpp"
#include "io_pending_state.hpp"

namespace http {
void Request::handleRequest() {
    if (_response.getStatus() != 200) {
        toolbox::logger::StepMark::error(
            "Request::handleRequest: Response already set status "
                + toolbox::to_string(_response.getStatus()) + " does not handle request");
        return;
    }

    HTTPRequest& httpRequest = _parsedRequest.get();
    const std::vector<std::string>& allowedMethods = _config.getAllowedMethods();

    if (std::find(allowedMethods.begin(), allowedMethods.end(),
                    httpRequest.method) == allowedMethods.end()) {
        _response.setStatus(HttpStatus::METHOD_NOT_ALLOWED);
        _ioPendingState = NO_IO_PENDING;
        toolbox::logger::StepMark::error(
            "Request::handleRequest: Method not allowed: "
                + httpRequest.method);
        return;
    }

    switch (_ioPendingState) {
        case CGI_LOCAL_REDIRECT_IO_PENDING:
            _response = http::Response();
            fetchConfig();
            _cgiHandler.reset();
            _cgiHandler.setRedirectCount(_requestDepth);
            _ioPendingState = NO_IO_PENDING;
            toolbox::logger::StepMark::info(
                "CGI_LOCAL_REDIRECT_IO_PENDING: requestDepth="
                + toolbox::to_string(_requestDepth));
            break;
        case CGI_BODY_SENDING:
        case CGI_OUTPUT_READING:
            _ioPendingState = _cgiHandler.handleRequest(httpRequest, _response,
            _client, _config, _ioPendingState);
            return;
        default:
            break;
    }
    const std::string& targetPath =
                    http::joinPath(_config.getRoot(), httpRequest.uri.path);
    bool isCgi = _cgiHandler.isCgiRequest(targetPath,
                                _config.getCgiExtensions(),
                                _config.getCgiPath());
    if (isCgi) {
        if (_ioPendingState == NO_IO_PENDING) {
            _cgiHandler.reset();
        }
        _cgiHandler.setRedirectCount(_requestDepth);
        _ioPendingState = _cgiHandler.handleRequest(httpRequest, _response,
            _client, _config, _ioPendingState);
    } else {
        _requestDepth = 0;
        serverMethod::serverMethodHandler(
            _parsedRequest, _config, httpRequest.fields, _response);
    }

    toolbox::logger::StepMark::info("Request::handleRequest: handled request for "
        + httpRequest.uri.path + " with method " + httpRequest.method);
}

}  // namespace http
