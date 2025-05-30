#include <vector>
#include <string>

#include "cgi_request_handle.hpp"
#include "../../../src/http/cgi/cgi_handler.hpp"
#include "../../../src/http/response/get_method.hpp"
#include "../../../src/http/http_namespace.hpp"


namespace http {
namespace request {

Request::Request() : _response(), _serverName(), _serverPort(), _cgiRedirectCount(0) {
}

Request::~Request() {
}

void Request::parseRawRequest(const std::string& rawRequest) {
    BaseParser::ParseStatus status = _request.run(rawRequest);
    if (status == BaseParser::P_ERROR) {
        _response.setStatus(_request.get().httpStatus.get());
    } else if (status == BaseParser::P_NEED_MORE_DATA) {
        // TODO(yootsubo) : Handle P_NEED_MORE_DATA data
    } else if (status == BaseParser::P_COMPLETED) {
        _response.setStatus(HttpStatus::OK);
    }
}

bool Request::processReturn(const config::Return& returnValue) {
    if (returnValue.hasReturnValue()) {
        size_t returnStatusCode = returnValue.getStatusCode();
        if (returnValue.isTextOrUrlSetting()) {
            std::string textOrUrl = returnValue.getTextOrUrl();
            _response.setStatus(returnStatusCode);
            if (returnStatusCode >= 300 &&
                returnStatusCode < 400) {
                _response.setHeader(http::fields::LOCATION, textOrUrl);
            } else {
                _response.setBody(textOrUrl);
                _response.setHeader(http::fields::CONTENT_TYPE, "text/plain");
                _response.setHeader(http::fields::CONTENT_LENGTH,
                    toolbox::to_string(textOrUrl.size()));
            }
            return true;
        } else {
            _response.setStatus(returnStatusCode);
        }
        return true;
    }
    return false;
}

void Request::fetchConfig() {
    if (_request.get().httpStatus.get() != HttpStatus::OK) {
        return;
    }
    const toolbox::SharedPtr<config::HttpConfig>& httpConfig =
                                                config::Config::getHttpConfig();
    if (!httpConfig) {
        _response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return;
    }
    const std::vector<toolbox::SharedPtr
                <config::ServerConfig> >& servers = httpConfig->getServers();
    toolbox::SharedPtr<config::ServerConfig> selectedServer;
    std::vector<std::string> hostHeaders =
                    _request.get().fields.getFieldValue(fields::HOST);
    std::string hostName;
    if (!hostHeaders.empty()) {
        hostName = hostHeaders[0];
        std::size_t colonPos = hostName.find(":");
        if (colonPos != std::string::npos) {
            hostName = hostName.substr(0, colonPos);
        }
    }
    for (size_t i = 0; i < servers.size(); ++i) {
        const std::vector<config::ServerName>& serverNames =
                                        servers[i]->getServerNames();
        for (size_t j = 0; j < serverNames.size(); ++j) {
            if (serverNames[j].getName() == hostName) {
                selectedServer = servers[i];
                _serverName = serverNames[j].getName();
                _serverPort = servers[i]->getListens()[0].getPort();
                break;
            }
        }
        if (selectedServer) {
            break;
        }
    }
    if (!selectedServer && !servers.empty()) {
        selectedServer = servers[0];
    }
    if (!selectedServer) {
        _response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return;
    }

    if (processReturn(selectedServer->getReturnValue())) {
        return;
    }
    std::string requestPath = _request.get().uri.path;
    const std::vector<toolbox::SharedPtr
    <config::LocationConfig> >& locations = selectedServer->getLocations();
    toolbox::SharedPtr<config::LocationConfig> matchedLocation;
    std::size_t longestMatchLength = 0;
    for (std::size_t i = 0; i < locations.size(); ++i) {
        std::string locPath = locations[i]->getPath();
        if (locPath == requestPath) {
            matchedLocation = locations[i];
            break;
        }
        if (requestPath.find(locPath) == 0 &&
                    locPath.size() > longestMatchLength) {
            if (locPath[locPath.size() - 1] == '/' ||
                requestPath[locPath.size()] == '/') {
                matchedLocation = locations[i];
                longestMatchLength = locPath.size();
            }
        }
    }
    if (!matchedLocation) {
        for (std::size_t i = 0; i < locations.size(); ++i) {
            if (locations[i]->getPath() == "/") {
                matchedLocation = locations[i];
                break;
            }
        }
    }
    if (matchedLocation) {
        _config = matchedLocation;
    } else {
        _response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return;
    }
}

void Request::handleRequest() {
    if (_request.get().httpStatus.get() != HttpStatus::OK) {
        return;
    }
    if (processReturn(_config->getReturnValue())) {
        return;
    }
    std::string requestPath = _request.get().uri.path;
    std::string rootDir = _config->getRoot();
    std::vector<std::string> indexFile = _config->getIndices();
    std::string cgiPath = _config->getCgiPath();
    std::vector<std::string> cgiExtension = _config->getCgiExtensions();
    std::string fullPath = rootDir + requestPath;
    const std::string& method = _request.get().method;

    if (_cgiHandler.isCgiRequest(fullPath, cgiExtension, cgiPath)) {
        _cgiHandler.setRedirectCount(_cgiRedirectCount);
        bool success = _cgiHandler.handleRequest(_request.get(),
                                _response, rootDir,
                                cgiExtension,
                                cgiPath);
        if (!success) {
            return;
        }
    } else {
        _cgiRedirectCount = 0;
    }
    if (method == "GET") {
        // runGet(fullPath, indexFile, isAutoindex,
        //                                    _config.getTypes(), _response);
    } else if (method == "HEAD") {
        // runHead();
    } else if (method == "POST") {
        // runPost();
    } else if (method == "DELETE") {
        // runDelete();
    } else {
        _response.setStatus(HttpStatus::METHOD_NOT_ALLOWED);
        _response.setHeader("Allow", "GET, HEAD, POST, DELETE");
    }
}

void Request::sendRequest(const int client_fd) const {
    _response.sendResponse(client_fd);
}

void Request::initializeRequest(const std::string& method,
                                const std::string& path,
                                const std::string& host) {
    _request.get().method = method;
    _request.get().uri.path = path;
    http::HTTPFields::FieldValue& hostField =
                            _request.get().fields.getFieldValue(fields::HOST);
    hostField.clear();
    hostField.push_back(host);
    _request.get().httpStatus.set(HttpStatus::OK);
}
}  // namespace request
}  // namespace http

