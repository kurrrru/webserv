#include <string>
#include <vector>

#include "../response/server_method_handler.hpp"
#include "../cgi/cgi_handler.hpp"
#include "../cgi/cgi_execute.hpp"
#include "request.hpp"

namespace http {
void Request::handleRequest(const Client& client) {
    std::vector<std::string> allowedMethods = _config.getAllowedMethods();
    if (std::find(allowedMethods.begin(), allowedMethods.end(),
                  _parsedRequest.get().method) == allowedMethods.end()) {
        _response.setStatus(HttpStatus::METHOD_NOT_ALLOWED);
        return;
    }

    CgiHandler cgiHandler;

    std::string targetPath = _parsedRequest.get().uri.path;
    const std::vector<std::string>& cgiExtensionVector = _config.getCgiExtensions();
    const std::string& cgiPath = _config.getCgiPath();

    if (cgiHandler.isCgiRequest(targetPath, cgiExtensionVector, cgiPath)) {
        cgiHandler.handleRequest(_parsedRequest.get(), _response,
            _config.getRoot(), cgiExtensionVector, cgiPath);
    } else {
        serverMethod::serverMethodHandler(
            _parsedRequest, _config, _parsedRequest.get().fields, _response);
    }
}

}  // namespace http
