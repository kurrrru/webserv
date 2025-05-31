#include <string>
#include <vector>
#include <algorithm>

#include "../response/server_method_handler.hpp"
#include "../cgi/cgi_handler.hpp"
#include "request.hpp"

namespace http {
void Request::handleRequest() {
    if (_response.getStatus() != 200) {
        toolbox::logger::StepMark::error(
            "Request::handleRequest: Response already set status "
                + std::to_string(_response.getStatus()) + " does not handle request");
        return;
    }

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

    const std::string& targetPath = http::joinPath(_config.getRoot(), httpRequest.uri.path);
    const std::string& cgiPath = _config.getCgiPath();
    const std::vector<std::string>& cgiExtensionVector = _config.getCgiExtensions();

    if (cgiHandler.isCgiRequest(targetPath, cgiExtensionVector, cgiPath)) {
        
        cgiHandler.handleRequest(httpRequest, _response,
            _config.getRoot(), cgiExtensionVector, cgiPath);
        

/*
            //ここでCGIの種類が帰ってくる。
            DOCUMENT = 0,
            LOCAL_REDIRECT = 1,
            CLIENT_REDIRECT = 2,
            CLIENT_REDIRECT_DOCUMENT = 3,
            INVALID = 4
*/
    } else {
        serverMethod::serverMethodHandler(
            _parsedRequest, _config, httpRequest.fields, _response);
    }

    toolbox::logger::StepMark::info("Request::handleRequest: handled request for "
        + httpRequest.uri.path + " with method " + httpRequest.method);
}

}  // namespace http
