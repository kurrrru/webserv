#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>

#include <sys/stat.h>

#include "cgi_handler.hpp"
#include "cgi_execute.hpp"
#include "../../test/http/cgi/cgi_request_handle.hpp"
#include "../http_namespace.hpp"
#include "../../../toolbox/stepmark.hpp"

namespace http {

CgiHandler::CgiHandler() : _redirectCount(0) {
}

CgiHandler::~CgiHandler() {
}

/**
 * @brief Determines if the specified path can be processed as a CGI request
 * 
 * This method checks the following conditions to determine if the request
 * should be handled as CGI:
 * 1. If the specified path (targetPath) actually exists
 * 2. If CGI extensions (cgiExtension) are configured
 * 3. If the CGI interpreter path (cgiPass) is configured
 * 
 * Returns true only when all conditions are met.
 * 
 * @param targetPath The file path to verify
 * @param cgiExtension List of extensions to be processed as CGI
 * @param cgiPass Path to the CGI interpreter (e.g., "/usr/bin/python3")
 * @return bool true if the path can be processed as a CGI request, false otherwise
 */
bool CgiHandler::isCgiRequest(const std::string& targetPath,
                            const std::vector<std::string>& cgiExtension,
                            const std::string& cgiPass) const {
    struct stat buffer;
    if (stat(targetPath.c_str(), &buffer) != 0) {
        toolbox::logger::StepMark::debug(
            "isCgiRequest: stat failed");
        return false;
    }
    if (cgiExtension.empty()) {
        return false;
    }
    if (cgiPass.empty()) {
        return false;
    }
    return true;
}

/**
 * Handles a CGI request by executing the script and processing the output
 * 
 * This method performs the complete CGI request handling workflow:
 * 1. Resolves the script path based on the request and root path
 * 2. Validates all parameters (script path, interpreter, extensions)
 * 3. Executes the CGI script using the CgiExecute component
 * 4. Processes the CGI output to generate an appropriate HTTP response
 * 
 * If any step fails, it sets an appropriate HTTP status code in the response
 * object and returns false. Error handling includes:
 * 
 * - Timeouts: 504 Gateway Timeout
 * - Other execution errors: 500 Internal Server Error
 * 
 * @param request The HTTP request to process
 * @param response The HTTP response to populate
 * @param rootPath The root directory path for resolving scripts
 * @param cgiExtension List of file extensions to be processed as CGI
 * @param cgiPass Path to the CGI interpreter (e.g., "/usr/bin/python3")
 * @param config Server configuration object for additional settings
 * @return bool true if CGI processing was successful, false otherwise
 */
bool CgiHandler::handleRequest(const HTTPRequest& request,
                            Response& response,
                            const std::string& rootPath,
                            const std::vector<std::string>& cgiExtension,
                            const std::string& cgiPass,
                            const config::Config& config) {
    std::string scriptPath = getScriptPath(request, rootPath);
    std::string interpreter = cgiPass;
    if (!validateParameters(scriptPath,
                            interpreter,
                            cgiExtension,
                            response)) {
        return false;
    }
    std::string output;
    CgiExecute::ExecuteResult result = _execute.execute(
        scriptPath, interpreter, request, output);
    if (result != CgiExecute::EXECUTE_SUCCESS) {
        switch (result) {
            case CgiExecute::EXECUTE_PATH_ERROR:
                response.setStatus(HttpStatus::FORBIDDEN);
                break;
            case CgiExecute::EXECUTE_TIMEOUT:
                response.setStatus(HttpStatus::GATEWAY_TIMEOUT);
                break;
            default:
                response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        }
        toolbox::logger::StepMark::error(
            "CGI execution failed: "
            + scriptPath
            + " (Error code: "
            + toolbox::to_string(result) + ")");
        return false;
    }
    return processCgiOutput(output, response, config);
}

bool CgiHandler::processCgiOutput(const std::string& output,
                                Response& response,
                                const config::Config& config) {
    CgiResponseParser parser;
    try {
        parser.run(output);
        parser.get().identifyCgiType();
        toolbox::logger::StepMark::debug(
            "CGI output: " + output);
        toolbox::logger::StepMark::debug(
            "CGI response type: " + toolbox::to_string(parser.get().cgiType));
        return cgiTypeHandler(response, parser.get(), config);
    } catch (const std::exception& e) {
        toolbox::logger::StepMark::error(
            std::string("CGI response processing exception: ") + e.what());
        return false;
    }
}

bool CgiHandler::cgiTypeHandler(Response& response,
                                const CgiResponse& cgiResponse,
                                const config::Config& config) {
    switch (cgiResponse.cgiType) {
        case CgiResponse::DOCUMENT:
            return handleDocument(response, cgiResponse);
        case CgiResponse::LOCAL_REDIRECT:
            return handleLocalRedirect(response, cgiResponse, config);
        case CgiResponse::CLIENT_REDIRECT:
            return handleClientRedirect(response, cgiResponse);
        case CgiResponse::CLIENT_REDIRECT_DOCUMENT:
            return handleClientRedirectDocument(response, cgiResponse);
        case CgiResponse::INVALID:
        default:
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            return false;
    }
}

void copyCgiResponseToResponse(const CgiResponse& cgiResponse,
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

bool CgiHandler::handleLocalRedirect(Response& response,
                                    const CgiResponse& cgiResponse,
                                    const config::Config& config) {
    if (!validateRedirectRequest(response, cgiResponse)) {
        return false;
    }
    RedirectInfo redirectInfo = extractRedirectInfo(cgiResponse);
    size_t newRedirectCount = _redirectCount + 1;
    Response redirectResponse = executeInternalRequest(
                                        redirectInfo.location,
                                        redirectInfo.host,
                                        newRedirectCount,
                                        config);
    if (redirectResponse.getStatus() == HttpStatus::INTERNAL_SERVER_ERROR) {
        response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    copyRedirectResponse(redirectResponse, response);
    return true;
}

bool CgiHandler::validateRedirectRequest(Response& response,
                                        const CgiResponse& cgiResponse) {
    if (_redirectCount >= http::cgi::MAX_REDIRECTS) {
        toolbox::logger::StepMark::error("CGI redirect loop detected");
        response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    const HTTPFields::FieldValue& locationValues =
        cgiResponse.fields.getFieldValue(fields::LOCATION);
    if (locationValues.empty()) {
        toolbox::logger::StepMark::error(
            "CGI local redirect missing Location header");
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
    const HTTPFields::FieldValue& hostValues =
        cgiResponse.fields.getFieldValue(fields::HOST);
    if (!hostValues.empty()) {
        info.host = hostValues.front();
    }
    return info;
}

Response CgiHandler::executeInternalRequest(
                            const std::string& location,
                            const std::string& host,
                            size_t redirectCount,
                            const config::Config& config) {
    request::Request request;
    request.setCgiRedirectCount(redirectCount);
    request.initializeRequest(http::method::GET, location, host);
    request.fetchConfig(config);
    request.handleRequest(config);
    return request.getResponse();
}

void CgiHandler::copyRedirectResponse(
                        const Response& redirectResponse,
                        Response& response) {
    response.setStatus(redirectResponse.getStatus());
    const std::map<std::string, std::pair<bool, std::string> >& headers =
        redirectResponse.getHeaders();
    for (std::map<std::string, std::pair<bool, std::string> >::const_iterator
            it = headers.begin(); it != headers.end(); ++it) {
        if (it->second.first) {
            response.setHeader(it->first, it->second.second);
        }
    }
    response.setBody(redirectResponse.getBody());
}

std::string CgiHandler::getScriptPath(const HTTPRequest& request,
                                    const std::string& root_path) const {
    std::string urlPath = root_path + request.uri.path;
    std::string scriptPath = urlPath;
    size_t pos = urlPath.find_first_of('/', root_path.length());
    while (pos != std::string::npos) {
        scriptPath = urlPath.substr(0, pos);
        struct stat st;
        if (stat(scriptPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            return scriptPath;
        }
        pos = urlPath.find_first_of('/', pos + 1);
    }
    struct stat st;
    if (stat(urlPath.c_str(), &st) != 0) {
        return "";
    }
    if (!S_ISREG(st.st_mode)) {
        return "";
    }
    return urlPath;
}

bool CgiHandler::validateParameters(const std::string& scriptPath,
                                const std::string& interpreter,
                                const std::vector<std::string>& cgi_extension,
                                Response& response) const {
    if (cgi_extension.empty()) {
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    std::string fileName;
    size_t lastSlash = scriptPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        fileName = scriptPath.substr(lastSlash + 1);
    } else {
        fileName = scriptPath;
    }
    std::string fileExtension;
    size_t lastDot = fileName.find_last_of('.');
    if (lastDot != std::string::npos) {
        fileExtension = fileName.substr(lastDot);
    } else {
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    bool isValidExtension = false;
    for (size_t i = 0; i < cgi_extension.size(); ++i) {
        if (fileExtension == cgi_extension[i]) {
            isValidExtension = true;
            break;
        }
    }
    if (isValidExtension == false) {
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    struct stat st;
    if (stat(scriptPath.c_str(), &st) != 0) {
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    if (!(st.st_mode & S_IXUSR)) {
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    if (!interpreter.empty() && access(interpreter.c_str(), X_OK) != 0) {
        response.setStatus(HttpStatus::FORBIDDEN);
        return false;
    }
    return true;
}

}  // namespace http
