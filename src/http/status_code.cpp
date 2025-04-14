#include "status_code.hpp"

bool StatusCode::_isInitialized = false;
std::map<EStatusCode, std::string> StatusCode::_statusMap;

const std::pair<std::string, std::string> StatusCode::getStatusPair(
    EStatusCode status) {
    std::map<EStatusCode, std::string>& statusMap = getStatusMap();
    std::map<EStatusCode, std::string>::iterator it = statusMap.find(status);
    if (it == statusMap.end()) {
        throw std::runtime_error("invalid status getStatusPair");
    }
    std::ostringstream oss;
    oss << status;
    return std::make_pair(oss.str(), it->second);
}

std::map<EStatusCode, std::string>& StatusCode::getStatusMap() {
    if (!_isInitialized) {
        _statusMap[CONTINUE] = "Continue";
        _statusMap[OK] = "OK";
        _statusMap[CREATED] = "Created";
        _statusMap[ACCEPTED] = "Accepted";
        _statusMap[NO_CONTENT] = "No Content";
        _statusMap[RESET_CONTENT] = "Reset Content";
        _statusMap[MOVED_PERMANENTLY] = "Moved Permanently";
        _statusMap[FOUND] = "Found";
        _statusMap[TEMPORARY_REDIRECT] = "Temporary Redirect";
        _statusMap[PERMANENT_REDIRECT] = "Permanent Redirect";
        _statusMap[BAD_REQUEST] = "Bad Request";
        _statusMap[UNAUTHORIZED] = "Unauthorized";
        _statusMap[FORBIDDEN] = "Forbidden";
        _statusMap[NOT_FOUND] = "Not Found";
        _statusMap[METHOD_NOT_ALLOWED] = "Method Not Allowed";
        _statusMap[REQUEST_TIMEOUT] = "Request Timeout";
        _statusMap[LENGTH_REQUIRED] = "Length Required";
        _statusMap[CONTENT_TOO_LARGE] = "Content Too Large";
        _statusMap[URI_TOO_LONG] = "URI Too long";
        _statusMap[I_AM_A_TEAPOT] = "I'm a teapot";
        _statusMap[INTERNAL_SERVER_ERROR] = "Internal Server Error";
        _statusMap[NOT_IMPLEMENTED] = "Not Implemented";
        _statusMap[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
        _isInitialized = true;
    }
    return _statusMap;
}
