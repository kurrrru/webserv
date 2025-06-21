#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>

#include <sys/stat.h>

#include "cgi_handler.hpp"
#include "cgi_execute.hpp"
#include "../request/request.hpp"
#include "../response/method_utils.hpp"
#include "../http_namespace.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../../event/epoll.hpp"

namespace http {

CgiHandler::CgiHandler() : _redirectCount(0) {
}

CgiHandler::~CgiHandler() {
}

void CgiHandler::reset() {
    _execute.reset();
}

bool CgiHandler::isCgiRequest(const std::string& targetPath,
                            const std::vector<std::string>& cgiExtension,
                            const std::string& cgiPath) const {
    if (cgiExtension.empty()) {
        return false;
    }
    if (cgiPath.empty()) {
        return false;
    }
    std::size_t componentStart = 0;
    while (componentStart < targetPath.length()) {
        std::size_t componentEnd = targetPath.find('/', componentStart);
        if (componentEnd == std::string::npos) {
            componentEnd = targetPath.length();
        }
        std::string component =
            targetPath.substr(componentStart, componentEnd - componentStart);
        std::size_t lastDot = component.find_last_of('.');
        if (lastDot != std::string::npos) {
            std::string extension = component.substr(lastDot);
            for (std::size_t i = 0; i < cgiExtension.size(); ++i) {
                if (extension == cgiExtension[i]) {
                    return true;
                }
            }
        }
        if (componentEnd == targetPath.length()) {
            break;
        }
        componentStart = componentEnd + 1;
    }
    return false;
}

http::IOPendingState CgiHandler::handleRequest(
                        const HTTPRequest& request,
                        Response& response,
                        const Client* client,
                        const config::LocationConfig& locationConfig,
                        const IOPendingState ioPendingState) {
    _client = client;
    switch (ioPendingState) {
        case NO_IO_PENDING:
        case CGI_LOCAL_REDIRECT_IO_PENDING:
            return executeInitialCgiRequest(request,
                                            response,
                                            locationConfig);
        case CGI_BODY_SENDING:
            return continueCgiBodySending(response);
        case CGI_OUTPUT_READING:
            return continueCgiOutputReading(response);
        default:
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            return NO_IO_PENDING;
    }
}

IOPendingState CgiHandler::executeInitialCgiRequest(
                        const HTTPRequest& request,
                        Response& response,
                        const config::LocationConfig& locationConfig) {
    std::string scriptPath =
                buildScriptPath(locationConfig.getRoot(),
                                request.uri.path,
                                locationConfig.getCgiExtensions());
    std::string interpreter = locationConfig.getCgiPath();
    if (!validateParameters(scriptPath, interpreter,
                            locationConfig.getCgiExtensions(), response)) {
        toolbox::logger::StepMark::error(
            "CgiHandler::executeInitialCgiRequest: validateParameters failed");
        return NO_IO_PENDING;
    }
    CgiExecute::ExecuteResult result = _execute.execute(
        scriptPath, interpreter, request, _client, locationConfig);
    return handleExecuteResult(result, response);
}

IOPendingState CgiHandler::continueCgiBodySending(Response& response) {
    _execute.continueWriteRequestBody();
    if (_execute.hasWriteError()) {
        if (_execute.hasTimedOut()) {
            toolbox::logger::StepMark::error(
                "CGI handler result: Write EXECUTE_TIMEOUT");
            response.setStatus(HttpStatus::GATEWAY_TIMEOUT);
        } else {
            toolbox::logger::StepMark::error(
                "CGI handler result: Write error detected");
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        }
        return NO_IO_PENDING;
    }
    if (_execute.isWriteComplete()) {
        return CGI_OUTPUT_READING;
    }
    return CGI_BODY_SENDING;
}

IOPendingState CgiHandler::continueCgiOutputReading(Response& response) {
    if (_execute.getReadState() == CgiExecute::READ_IDLE) {
        if (!_execute.initReadOutput()) {
            if (_execute.hasReadError()) {
                if (_execute.hasTimedOut()) {
                    toolbox::logger::StepMark::error(
                        "CGI execute result: EXECUTE_TIMEOUT");
                    response.setStatus(HttpStatus::GATEWAY_TIMEOUT);
                } else {
                    response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
                }
                return NO_IO_PENDING;
            }
            return CGI_OUTPUT_READING;
        }
    } else {
        _execute.continueReadOutput();
    }
    if (_execute.hasReadError()) {
        if (_execute.hasTimedOut()) {
            toolbox::logger::StepMark::error(
                "CGI execute result: read EXECUTE_TIMEOUT");
            response.setStatus(HttpStatus::GATEWAY_TIMEOUT);
        } else {
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        }
        return NO_IO_PENDING;
    }
    if (_execute.isReadComplete()) {
        IOPendingState result =
        processCgiResponse(_execute.getResponse(), response);
        _execute.cleanupPipes();
        return result;
    }
    return CGI_OUTPUT_READING;
}

IOPendingState CgiHandler::handleExecuteResult(
                        CgiExecute::ExecuteResult result,
                        Response& response) {
    switch (result) {
        case CgiExecute::EXECUTE_SUCCESS:
            return processCgiResponse(_execute.getResponse(), response);
        case CgiExecute::EXECUTE_WRITE_PENDING:
            return CGI_BODY_SENDING;
        case CgiExecute::EXECUTE_READ_PENDING:
            return CGI_OUTPUT_READING;
        case CgiExecute::EXECUTE_TIMEOUT:
            response.setStatus(HttpStatus::GATEWAY_TIMEOUT);
            return NO_IO_PENDING;
        case CgiExecute::EXECUTE_PATH_ERROR:
            response.setStatus(HttpStatus::FORBIDDEN);
            return NO_IO_PENDING;
        case CgiExecute::EXECUTE_FORK_ERROR:
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            return NO_IO_PENDING;
        case CgiExecute::EXECUTE_EXEC_ERROR:
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            return NO_IO_PENDING;
        case CgiExecute::EXECUTE_IO_ERROR:
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            return NO_IO_PENDING;
        default:
            toolbox::logger::StepMark::error(
                "Unknown CGI execute result: " +
                toolbox::to_string(static_cast<int>(result)));
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            return NO_IO_PENDING;
    }
}

IOPendingState CgiHandler::processCgiResponse(const CgiResponse& cgiResponse,
                                Response& response) {
    try {
        switch (cgiResponse.cgiType) {
            case CgiResponse::DOCUMENT:
                handleDocument(response, cgiResponse);
                return NO_IO_PENDING;
            case CgiResponse::LOCAL_REDIRECT:
                return handleLocalRedirect(response, cgiResponse);
            case CgiResponse::CLIENT_REDIRECT:
                handleClientRedirect(response, cgiResponse);
                return NO_IO_PENDING;
            case CgiResponse::CLIENT_REDIRECT_DOCUMENT:
                handleClientRedirectDocument(response, cgiResponse);
                return NO_IO_PENDING;
            case CgiResponse::INVALID:
            default:
                response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
                return NO_IO_PENDING;
        }
    } catch (const std::exception& e) {
        toolbox::logger::StepMark::error(
            std::string("CGI response processing exception: ") + e.what());
        response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return NO_IO_PENDING;
    }
}

void CgiHandler::copyCgiResponseToResponse(const CgiResponse& cgiResponse,
                            Response& response) {
    response.setStatus(cgiResponse.httpStatus.get());
    const HTTPFields::FieldMap& fields = cgiResponse.fields.get();
    for (HTTPFields::FieldMap::const_iterator it =
                                    fields.begin(); it != fields.end(); ++it) {
        if (it->first != http::fields::cgi::STATUS && !it->second.empty()) {
            response.setHeader(it->first, it->second.front());
        }
    }
    response.setBody(cgiResponse.body);
}

bool CgiHandler::handleDocument(Response& response,
                                const CgiResponse& cgiResponse) {
    copyCgiResponseToResponse(cgiResponse, response);
    if (cgiResponse.httpStatus.get() == 0 ||
        cgiResponse.httpStatus.get() == HttpStatus::UNSET) {
        response.setStatus(HttpStatus::OK);
    }
    return true;
}

bool CgiHandler::handleClientRedirect(Response& response,
                                    const CgiResponse& cgiResponse) {
    copyCgiResponseToResponse(cgiResponse, response);
    if (cgiResponse.httpStatus.get() == 0 ||
        cgiResponse.httpStatus.get() == HttpStatus::UNSET) {
        response.setStatus(HttpStatus::FOUND);
    }
    return true;
}

bool CgiHandler::handleClientRedirectDocument(Response& response,
                                const CgiResponse& cgiResponse) {
    copyCgiResponseToResponse(cgiResponse, response);
    return true;
}

IOPendingState CgiHandler::handleLocalRedirect(Response& response,
                                    const CgiResponse& cgiResponse) {
    if (!validateRedirectRequest(response, cgiResponse)) {
        return NO_IO_PENDING;
    }
    RedirectInfo redirectInfo = extractRedirectInfo(cgiResponse);
    return executeInternalRequest(redirectInfo.location,
                                redirectInfo.host,
                                _redirectCount + 1);
}

bool CgiHandler::validateRedirectRequest(Response& response,
                                        const CgiResponse& cgiResponse) {
    if (_redirectCount >= http::cgi::MAX_REDIRECTS) {
        toolbox::logger::StepMark::error("CGI redirect loop detected");
        response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        response.setBody("CGI redirect loop detected");
        response.setHeader("Content-Type", "text/plain");
        return false;
    }
    const HTTPFields::FieldValue& locationValues =
        cgiResponse.fields.getFieldValue(fields::LOCATION);
    if (locationValues.empty()) {
        response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    return true;
}

http::CgiHandler::RedirectInfo
CgiHandler::extractRedirectInfo(const CgiResponse& cgiResponse) {
    RedirectInfo info;
    const HTTPFields::FieldValue& locationValues =
        cgiResponse.fields.getFieldValue(fields::LOCATION);
    info.location = locationValues.front();
    toolbox::logger::StepMark::info(
        "CgiHandler::extractRedirectInfo: Location header value: "
        + info.location);
    const HTTPFields::FieldValue& hostValues =
        cgiResponse.fields.getFieldValue(fields::HOST);
    if (!hostValues.empty()) {
        info.host = hostValues.front();
    }
    return info;
}

IOPendingState CgiHandler::executeInternalRequest(
                            const std::string& location,
                            const std::string& host,
                            std::size_t redirectCount) {
    toolbox::SharedPtr<http::Request> clientRequest = _client->getRequest();
    clientRequest->setRedirectCount(redirectCount);
    clientRequest->setLocalRedirectInfo(http::method::GET, location, host);
    return CGI_LOCAL_REDIRECT_IO_PENDING;
}

bool CgiHandler::validateParameters(const std::string& scriptPath,
                                const std::string& interpreter,
                                const std::vector<std::string>& cgi_extension,
                                Response& response) const {
    if (cgi_extension.empty()) {
        toolbox::logger::StepMark::error(
            "validateParameters: cgi_extension is empty");
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    std::string fileName;
    std::size_t lastSlash = scriptPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        fileName = scriptPath.substr(lastSlash + 1);
    } else {
        fileName = scriptPath;
    }
    std::string fileExtension;
    std::size_t lastDot = fileName.find_last_of('.');
    if (lastDot != std::string::npos) {
        fileExtension = fileName.substr(lastDot);
    } else {
        toolbox::logger::StepMark::error(
            "validateParameters: No file extension found in: " + fileName);
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    bool isValidExtension = false;
    for (std::size_t i = 0; i < cgi_extension.size(); ++i) {
        if (fileExtension == cgi_extension[i]) {
            isValidExtension = true;
            break;
        }
    }
    if (isValidExtension == false) {
        toolbox::logger::StepMark::error(
            "validateParameters: Invalid extension '"
            + fileExtension + "' for file: " + scriptPath);
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    struct stat st;
    if (stat(scriptPath.c_str(), &st) != 0) {
        toolbox::logger::StepMark::error(
            "validateParameters: stat() failed for: " + scriptPath);
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    if (!(st.st_mode & S_IXUSR)) {
        toolbox::logger::StepMark::error(
            "validateParameters: File not executable: " + scriptPath);
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    if (!interpreter.empty() && access(interpreter.c_str(), X_OK) != 0) {
        toolbox::logger::StepMark::error(
            "validateParameters: Interpreter not executable: " + interpreter);
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    return true;
}

std::string CgiHandler::buildScriptPath(const std::string& root,
                        const std::string& uriPath,
                        const std::vector<std::string>& cgiExtensions) const {
    std::size_t componentStart = 0;
    while (componentStart < uriPath.length()) {
        std::size_t componentEnd = uriPath.find('/', componentStart);
        if (componentEnd == std::string::npos) {
            componentEnd = uriPath.length();
        }
        std::string component =
            uriPath.substr(componentStart, componentEnd - componentStart);
        for (std::size_t i = 0; i < cgiExtensions.size(); ++i) {
            if (component.length() >= cgiExtensions[i].length()) {
                std::string componentSuffix =
                    component.substr(
                    component.length() - cgiExtensions[i].length());
                if (componentSuffix == cgiExtensions[i]) {
                    return
                    http::joinPath(root, uriPath.substr(0, componentEnd));
                }
            }
        }
        if (componentEnd == uriPath.length()) {
            break;
        }
        componentStart = componentEnd + 1;
    }
    return http::joinPath(root, uriPath);
}

}  // namespace http
