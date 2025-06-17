#include "response.hpp"

#include <sys/socket.h>

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <utility>
#include <algorithm>

#include "../../core/constant.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../get_gmt.hpp"

namespace http {

Response::Response() : _status(200), _headers(), _body(), 
_wholeResponseStr(), _wholeResponsePtr(NULL), _lengthSent(0),
_errorPageNewStatus(-1), _errorPageOverwrite(false) {
}
Response::Response(const Response& other)
: _status(other._status), _headers(other._headers), _body(other._body),
_wholeResponseStr(other._wholeResponseStr), _wholeResponsePtr(other._wholeResponsePtr),
_lengthSent(other._lengthSent), _errorPageNewStatus(other._errorPageNewStatus),
_errorPageOverwrite(other._errorPageOverwrite) {
}
Response& Response::operator=(const Response& other) {
    if (this != &other) {
        _status = other._status;
        _headers = other._headers;
        _body = other._body;
        _wholeResponseStr = other._wholeResponseStr;
        _wholeResponsePtr = other._wholeResponsePtr;
        _lengthSent = other._lengthSent;
        _errorPageNewStatus = other._errorPageNewStatus;
        _errorPageOverwrite = other._errorPageOverwrite;
    }
    return *this;
}
Response::~Response() {}

void Response::setStatus(int code) {
    _status = code;
}

// - Content-Length can be calculated from the body size, so it is not
//   necessary to set it explicitly.
void Response::setHeader(const FieldName& name,
const FieldContent& value, ResponseFlag enabled) {
    _headers[name] = std::make_pair(enabled, value);
}

// - Content-Length can be calculated from the body size, so it is not
//   necessary to set it explicitly.
void Response::setHeader(const FieldName& name,
const std::vector<FieldContent>& values, ResponseFlag enabled) {
    std::ostringstream oss;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << values[i];
    }
    _headers[name] = std::make_pair(enabled, oss.str());
}

void Response::setBody(const std::string& body) {
    _body = body;
}

bool Response::sendResponse(int client_fd) {
    if (_wholeResponsePtr == NULL) {
        _wholeResponseStr = buildResponse();
        _wholeResponsePtr = _wholeResponseStr.c_str();
    }
    if (_lengthSent >= static_cast<ssize_t>(_wholeResponseStr.size())) {
        return true;
    }
    ssize_t remaining = _wholeResponseStr.size() - _lengthSent;
    ssize_t sent = send(client_fd, _wholeResponsePtr + _lengthSent,
        std::min(remaining, static_cast<ssize_t>(core::IO_BUFFER_SIZE)), 0);
    if (sent <= 0) {
        std::ostringstream oss;
        oss << "Failed to send response:\n";
        oss << "  Sent: " << _lengthSent << "\n";
        oss << "  Total: " << _wholeResponseStr.size() << "\n";
        oss << "  Error: " << (sent == -1 ? "send() failed" : "Connection closed");
        throw std::runtime_error(oss.str());
    }
    _lengthSent += sent;
    if (_lengthSent >= static_cast<ssize_t>(_wholeResponseStr.size())) {
        _wholeResponseStr.clear();
        _wholeResponsePtr = NULL;
        _lengthSent = 0;
        return true;
    }
    return false;
}

std::string Response::buildResponse() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << _status;
    if (getStatusMessage(_status) != "Unknown Status") {
        oss << " " << getStatusMessage(_status);
    }
    oss << "\r\n";

    std::map<FieldName, HeaderField>::const_iterator it = _headers.find("Server");
    if (it != _headers.end() && it->second.first) {
        oss << "Server: " << it->second.second << "\r\n";
    } else {
        oss << "Server: Webserv/Ideal Broccoli\r\n";
    }
    oss << "Date: " << http::getCurrentGMT() << "\r\n";

    for (std::map<FieldName, HeaderField>::const_iterator header = _headers.begin();
        header != _headers.end(); ++header) {
        if (header->first == "Server" || header->first == "Date" ||
            header->first == "Content-Length") {
            continue;
        }
        if (header->second.first) {
            oss << header->first << ": " << header->second.second << "\r\n";
        }
    }

    const int noContentStatus = 204;
    const int notModifiedStatus = 304;
    if (_status != noContentStatus && _status != notModifiedStatus &&
        _headers.count("Transfer-Encoding") == 0) {
        oss << "Content-Length: " << _body.size() << "\r\n";
    }
    oss << "\r\n";
    oss << _body;

    return oss.str();
}

std::string Response::getStatusMessage(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 203: return "Non-Authoritative Information";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 206: return "Partial Content";

        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        // case 305: return "Use Proxy"; // Not recommended
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";

        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment Required";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 407: return "Proxy Authentication Required";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Payload Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 416: return "Range Not Satisfiable";
        case 417: return "Expectation Failed";
        case 418: return "I'm a teapot";
        case 421: return "Misdirected Request";
        case 422: return "Unprocessable Entity";
        case 426: return "Upgrade Required";

        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";

        default: return "Unknown Status";
    }
}

int Response::getStatus() const {
    return _status;
}

std::string Response::getHeader(const FieldName& name) const {
    std::map<FieldName, HeaderField>::const_iterator it = _headers.find(name);
    if (it != _headers.end() && it->second.first) {
        return it->second.second;
    }
    return "";  // Return empty string if header not found or disabled
}

std::size_t Response::getContentLength() const {
    return _body.size();
}

const std::string& Response::getBody() const {
    return _body;
}

bool Response::isErrorPageOverwrite() const {
    return _errorPageOverwrite;
}

int Response::getErrorPageNewStatus() const {
    return _errorPageNewStatus;
}

void Response::setErrorPage(bool overwrite, int newStatus) {
    _errorPageOverwrite = overwrite;
    _errorPageNewStatus = newStatus;
}

}  // namespace http
