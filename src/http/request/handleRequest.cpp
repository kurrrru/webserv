#include <string>
#include <vector>
#include <algorithm>

#include "../response/server_method_handler.hpp"
#include "../cgi/cgi_handler.hpp"
#include "request.hpp"

namespace http {
void Request::handleRequest() {
    try {
        HTTPRequest& httpRequest = _parsedRequest.get();
        const std::vector<std::string>& allowedMethods = _config.getAllowedMethods();

        if (std::find(allowedMethods.begin(), allowedMethods.end(),
                      httpRequest.method) == allowedMethods.end()) {
            _response.setStatus(HttpStatus::METHOD_NOT_ALLOWED);
            toolbox::logger::StepMark::error(
                "Request::handleRequest: Method not allowed: "
                    + httpRequest.method);
            return;
        }

        CgiHandler cgiHandler;

        const std::string& targetPath = httpRequest.uri.path;
        const std::vector<std::string>& cgiExtensionVector
            = _config.getCgiExtensions();
        const std::string& cgiPath = _config.getCgiPath();

        if (cgiHandler.isCgiRequest(targetPath, cgiExtensionVector, cgiPath)) {
            cgiHandler.handleRequest(httpRequest, _response,
                _config.getRoot(), cgiExtensionVector, cgiPath);
        } else {
            serverMethod::serverMethodHandler(
                _parsedRequest, _config, httpRequest.fields, _response);
        }

        toolbox::logger::StepMark::info("Request::handleRequest: Request processed successfully");
    } catch (const std::exception& e) {
        toolbox::logger::StepMark::error(
            "Request::handleRequest: Exception occurred: "
            + std::string(e.what()));
        _response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
    }
}

}  // namespace http
