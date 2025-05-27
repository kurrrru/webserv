#include <vector>
#include <string>

#include "fetch_config_test.hpp"
#include "../../../src/core/client.hpp"
#include "../../../src/config/config.hpp"
#include "../../../src/config/config_inherit.hpp"
#include "../../../src/http/request/request_parser.hpp"
#include "../../../src/http/response/response.hpp"
#include "../../../toolbox/shared.hpp"



namespace test {

void RequestTest::fetchConfig() {
    toolbox::SharedPtr<config::ServerConfig> selectedServer =
                                                    selectServer();
    if (!selectedServer) {
        response.setStatus(http::HttpStatus::INTERNAL_SERVER_ERROR);
        return;
    }
    if (processReturn(selectedServer->getReturnValue())) {
        return;
    }
    if (!selectLocation(selectedServer)) {
        response.setStatus(http::HttpStatus::INTERNAL_SERVER_ERROR);
        return;
    }
}

toolbox::SharedPtr<config::ServerConfig> RequestTest::selectServer() {
    const toolbox::SharedPtr<config::HttpConfig>& httpConfig =
                                                config::Config::getHttpConfig();
    std::vector<toolbox::SharedPtr<config::ServerConfig> > candidateServers;
    if (!extractCandidateServers(httpConfig->getServers(), candidateServers)) {
        response.setStatus(http::HttpStatus::BAD_REQUEST);
        return toolbox::SharedPtr<config::ServerConfig>(NULL);
    }
    std::string hostName = extractHostName();
    toolbox::SharedPtr<config::ServerConfig> selectedServer =
            matchServerByName(candidateServers, hostName);
    if (!selectedServer) {
        selectedServer = candidateServers[0];
    }
    return selectedServer;
}

bool RequestTest::extractCandidateServers(
    const std::vector<toolbox::SharedPtr<config::ServerConfig> >& servers,
    std::vector<toolbox::SharedPtr<config::ServerConfig> >& candidateServers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        const std::vector<config::Listen>& listens = servers[i]->getListens();
        for (size_t j = 0; j < listens.size(); ++j) {
            if (listens[j].getPort() == client->getServerPort()) {
                if (listens[j].getIp() ==  client->getServerIp()) {
                    candidateServers.push_back(servers[i]);
                } else if (listens[j].getIp() == "0.0.0.0") {
                    candidateServers.push_back(servers[i]);
                }
            }
        }
    }
    return !candidateServers.empty();
}

std::string RequestTest::extractHostName() {
    std::vector<std::string> hostHeaders =
                parsedRequest.get().fields.getFieldValue(http::fields::HOST);
    std::string hostName;
    if (!hostHeaders.empty()) {
        hostName = hostHeaders[0];
        std::size_t colonPos = hostName.find(":");
        if (colonPos != std::string::npos) {
            hostName = hostName.substr(0, colonPos);
        }
    }
    return hostName;
}

toolbox::SharedPtr<config::ServerConfig> RequestTest::matchServerByName(
    const std::vector<toolbox::SharedPtr<config::ServerConfig> >& candidates,
    const std::string& hostName) {
    for (size_t i = 0; i < candidates.size(); ++i) {
        const std::vector<config::ServerName>& serverNames =
                                    candidates[i]->getServerNames();
        for (size_t j = 0; j < serverNames.size(); ++j) {
            if (serverNames[j].getType() == config::ServerName::EXACT &&
                serverNames[j].getName() == hostName) {
                return candidates[i];
            }
        }
    }
    return toolbox::SharedPtr<config::ServerConfig>(NULL);
}

bool RequestTest::processReturn(const config::Return& returnValue) {
    if (!returnValue.hasReturnValue()) {
        return false;
    }
    size_t statusCode = returnValue.getStatusCode();
    response.setStatus(statusCode);
    if (statusCode == 204) {
        return true;
    }
    if (returnValue.isTextOrUrlSetting()) {
        processReturnWithContent(statusCode, returnValue.getTextOrUrl());
    } else {
        processReturnWithoutContent(statusCode);
    }
    return true;
}

void RequestTest::processReturnWithContent(size_t statusCode, const std::string& content) {
    if (isRedirectStatus(statusCode)) {
        setRedirectResponse(statusCode, content);
    } else {
        setTextResponse(content);
    }
}

void RequestTest::processReturnWithoutContent(size_t statusCode) {
    if (isRedirectStatus(statusCode)) {
        setRedirectResponse(statusCode, "");
    } else if (hasDefaultErrorPage(statusCode)) {
        setHtmlErrorResponse(statusCode);
    } else if (isMinimalResponse(statusCode)) {
        response.setHeader(http::fields::CONTENT_LENGTH, "0");
    } else {
        setEmptyTextResponse();
    }
}

bool RequestTest::isRedirectStatus(size_t statusCode) const {
    return (statusCode == 301 || statusCode == 302 || statusCode == 303 ||
            statusCode == 307 || statusCode == 308);
}

bool RequestTest::hasDefaultErrorPage(size_t statusCode) const {
    return (statusCode >= 400 && statusCode <= 505) &&
           !isMinimalResponse(statusCode);
}

bool RequestTest::isMinimalResponse(size_t statusCode) const {
    return (statusCode == 407 || statusCode == 417 || statusCode == 418 ||
            statusCode == 422 || statusCode == 426);
}

void RequestTest::setRedirectResponse(size_t statusCode,
                                const std::string& location) {
    response.setHeader(http::fields::LOCATION, location);
    std::string defaultBody = generateDefaultBody(statusCode);
    response.setBody(defaultBody);
    response.setHeader(http::fields::CONTENT_TYPE, "text/html");
    response.setHeader(http::fields::CONTENT_LENGTH,
                        toolbox::to_string(defaultBody.size()));
}

void RequestTest::setTextResponse(const std::string& content) {
    response.setBody(content);
    response.setHeader(http::fields::CONTENT_TYPE, "text/plain");
    response.setHeader(http::fields::CONTENT_LENGTH,
                        toolbox::to_string(content.size()));
}

void RequestTest::setHtmlErrorResponse(size_t statusCode) {
    std::string defaultBody = generateDefaultBody(statusCode);
    response.setBody(defaultBody);
    response.setHeader(http::fields::CONTENT_TYPE, "text/html");
    response.setHeader(http::fields::CONTENT_LENGTH,
                        toolbox::to_string(defaultBody.size()));
}

void RequestTest::setEmptyTextResponse() {
    response.setBody("");
    response.setHeader(http::fields::CONTENT_TYPE, "text/plain");
    response.setHeader(http::fields::CONTENT_LENGTH, "0");
}

std::string RequestTest::generateDefaultBody(size_t statusCode) {
    std::string statusText = response.getStatusMessage(statusCode);
    return "<html>\n"
           "<head><title>" + toolbox::to_string(statusCode)
           + " " + statusText + "</title></head>\n"
           "<body>\n"
           "<center><h1>" + toolbox::to_string(statusCode)
           + " " + statusText + "</h1></center>\n"
           "<hr><center>webserv</center>\n"
           "</body>\n"
           "</html>";
}

bool RequestTest::selectLocation(
    const toolbox::SharedPtr<config::ServerConfig>& server) {
    std::string requestPath = parsedRequest.get().uri.path;
    toolbox::SharedPtr<config::LocationConfig> matchedLocation =
        findDeepestMatchingLocation(server->getLocations(), requestPath);
    if (matchedLocation) {
        locationConfig = *matchedLocation;
        return true;
    } else {
        if (requestPath.empty() ||
            requestPath == config::DEFAULT_LOCATION_PATH) {
            server->addLocation(toolbox::SharedPtr
                <config::LocationConfig>(new config::LocationConfig()));
            config::ConfigInherit
            inherit(config::Config::getHttpConfig().get());
            inherit.applyInheritance();
            matchedLocation =
                findDeepestMatchingLocation(server->getLocations(),
                                            requestPath);
            return true;
        }
    }
    return false;
}

toolbox::SharedPtr
<config::LocationConfig> RequestTest::findDeepestMatchingLocation(
    const std::vector<toolbox::SharedPtr<config::LocationConfig> >& locations,
    const std::string& path) {
    toolbox::SharedPtr<config::LocationConfig> bestMatch;
    std::size_t longestMatchLength = 0;
    for (size_t i = 0; i < locations.size(); ++i) {
        std::string locPath = locations[i]->getPath();
        if (locPath == path) {
            return locations[i];
        }
        if (path.find(locPath) == 0 && locPath.size() > longestMatchLength) {
            if (locPath[locPath.size() - 1] == '/' ||
                path.size() == locPath.size() ||
                path[locPath.size()] == '/') {
                bestMatch = locations[i];
                longestMatchLength = locPath.size();
            }
        }
    }
    if (bestMatch && bestMatch->hasLocations()) {
        toolbox::SharedPtr<config::LocationConfig> childMatch =
            findDeepestMatchingLocation(bestMatch->getLocations(), path);
        if (childMatch) {
            return childMatch;
        }
    }
    return bestMatch;
}

}  // namespace test
