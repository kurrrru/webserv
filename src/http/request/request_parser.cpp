#include "request_parser.hpp"

#include <iostream>

#include "../http_namespace.hpp"
#include "../status_code.hpp"

/*
Parser utils
*/

std::string trim(std::string& src, const std::string& sep) {
    std::size_t pos = src.find(sep);
    if (pos == std::string::npos && src.length() == 0) {
        return "";
    }
    std::string newStr = src.substr(0, pos);
    src.erase(0, pos + sep.length());
    return newStr;
}

bool hasCtlChar(std::string& str) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (std::iscntrl(*it)) {
            return true;
        }
    }
    return false;
}

bool isUppStr(std::string& str) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (!std::isupper(*it)) {
            return false;
        }
    }
    return true;
}

bool isTraversalAttack(std::string& path) {
    std::string normalizePath = path;
    std::size_t pos = 0;
    while ((pos = normalizePath.find("//", pos)) != std::string::npos) {
        normalizePath.replace(pos, 2, "/");
    }
    std::vector<std::string> splitPath;
    std::string parts;
    for (std::size_t i = 0; i < normalizePath.length(); ++i) {
        if (normalizePath[i] == '/' && !parts.empty()) {
            splitPath.push_back(parts);
            parts.clear();
        } else {
            parts += normalizePath[i];
        }
    }
    if (!parts.empty()) {
        splitPath.push_back(parts);
    }
    int level = 0;
    for (std::size_t i = 0; i < splitPath.size(); ++i) {
        if (splitPath[i] == "..") {
            --level;
            if (level < 0) {
                return true;
            }
        } else if (splitPath[i] != ".") {
            ++level;
        }
    }
    return false;
}

std::pair<std::string, std::vector<std::string>> splitFieldLine(
    std::string& line) {
    std::size_t pos = line.find_first_of(':');
    if (pos == std::string::npos) {
        throw RequestParser::ParseException("Error: invalid field line");
    }
    std::pair<std::string, std::vector<std::string>> pair;
    pair.first = trim(line, ": ");
    pair.second.push_back(line);
    return pair;
}

void splitQuery(std::map<std::string, std::string>& queryMap,
                std::string& fullQuery) {
    std::string line = fullQuery.substr(1);  // skip '?'
    for (;;) {
        std::size_t e_pos = line.find(http::EQUAL);
        std::size_t a_pos = line.find(http::AMPERSAND);
        if (a_pos != std::string::npos) {
            queryMap[line.substr(0, e_pos)] =
                line.substr(e_pos + 1, a_pos - e_pos - 1);
        } else {
            queryMap[line.substr(0, e_pos)] = line.substr(e_pos + 1);
            break;
        }
        line.erase(0, a_pos + 1);
    }
}

/*
Class method
*/

RequestParser::ParseException::ParseException(const char* message)
    : _message(message) {}

const char* RequestParser::ParseException::what() const throw() {
    return _message;
}

void RequestParser::run(const std::string& buf) {
    _buf.append(buf);
    parseRequestLine();
    parseFields();
    parseBody();
}

void RequestParser::parseRequestLine() {
    if (_state != REQUEST_LINE) {
        return;
    }
    if (_buf.find(http::CRLF) == std::string::npos) {
        return;
    }
    std::string line = trim(_buf, http::CRLF);
    _request.method = trim(line, http::SP);
    _request.uri.fullUri = trim(line, http::SP);
    _request.version = trim(line, http::SP);
    parseURI();
    validateMethod();
    validateURI();
    validateVersion();
    _state = HEADERS;
}

void RequestParser::parseURI() {
    std::size_t uriLen = _request.uri.fullUri.length();
    std::size_t q_pos = _request.uri.fullUri.find(http::QUESTION);
    std::size_t h_pos = _request.uri.fullUri.find(http::HASH);
    // path
    if (q_pos == h_pos) {  // no such
        _request.uri.path = _request.uri.fullUri.substr(0);
        return;
    } else if (q_pos < h_pos) {  // front query
        _request.uri.path = _request.uri.fullUri.substr(0, q_pos);
    } else {  // front frag
        _request.uri.path = _request.uri.fullUri.substr(0, h_pos);
    }
    // frag
    if (h_pos != std::string::npos) {
        _request.uri.fragment = _request.uri.fullUri.substr(h_pos + 1);
    }
    // query
    if (q_pos != std::string::npos) {
        if (h_pos == std::string::npos) {
            _request.uri.fullQuery = _request.uri.fullUri.substr(q_pos);
        } else {
            _request.uri.fullQuery =
                _request.uri.fullUri.substr(q_pos, h_pos - q_pos);
        }
    }
    splitQuery(_request.uri.queryMap, _request.uri.fullQuery);
}

void RequestParser::validateMethod() {
    if (_request.method.empty()) {
        _requestState = StatusCode::getStatusPair(BAD_REQUEST);
        throw ParseException("Error: method doesn't exist");
    }
    if (hasCtlChar(_request.method) || !isUppStr(_request.method)) {
        _requestState = StatusCode::getStatusPair(METHOD_NOT_ALLOWED);
        throw ParseException(
            "Error: method has control or lowercase character");
    }
    if (_request.method != http::GET && _request.method != http::POST &&
        _request.method != http::DELETE) {
        _requestState = StatusCode::getStatusPair(METHOD_NOT_ALLOWED);
        throw ParseException("Error: method isn't supported");
    }
    return;
}

void RequestParser::validateURI() {
    if (_request.uri.fullUri.empty()) {
        _requestState = StatusCode::getStatusPair(BAD_REQUEST);
        throw ParseException("Error: uri doesn't exist");
    }
    if (*(_request.uri.fullUri.begin()) != '/' ||
        hasCtlChar(_request.uri.fullUri)) {
        _requestState = StatusCode::getStatusPair(NOT_FOUND);
        throw ParseException("Error: missing '/' or uri has control character");
    }
    if (isTraversalAttack(_request.uri.path)) {
        _requestState = StatusCode::getStatusPair(FOUND);
        throw ParseException("Error: warning! path is TraversalAttack");
    }
    return;
}

void RequestParser::validateVersion() {
    if (_request.version.empty()) {
        _request.version = http::HTTP_VERSION;
    }
    if (hasCtlChar(_request.version)) {
        _requestState = StatusCode::getStatusPair(BAD_REQUEST);
        throw ParseException("Error: version has control character");
    }
    if (_request.version != http::HTTP_VERSION) {
        _requestState = StatusCode::getStatusPair(BAD_REQUEST);
        throw ParseException("Error: version isn't supported");
    }
    return;
}

// if CRLF + CRLF nothing, don't work.
void RequestParser::parseFields() {
    if (_state != HEADERS) {
        return;
    }
    if (_buf.find(http::CRLF) == std::string::npos) {
        return;
    }
    if (!_request.fields.isInitialized()) {
        _request.fields.initFieldsMap();
    }
    for (;;) {
        if (_buf.find(http::CRLF) == 0) {
            _state = BODY;
            _buf = _buf.substr(http::CRLF.length());
            return;
        }
        if (_buf.find(http::CRLF) == std::string::npos) {
            return;
        }
        std::string line = trim(_buf, http::CRLF);
        if (hasCtlChar(line)) {
            throw ParseException("Error: field value has ctlchar");
        }
        std::pair<std::string, std::vector<std::string>> pair =
            splitFieldLine(line);
        _request.fields.add(pair);
    }
}

void RequestParser::parseBody() {
    if (_state != BODY) {
        return;
    }
    if (!_request.body.contentLength) {
        std::vector<std::string>& contentLen =
            _request.fields.get(http::CONTENT_LENGTH);
        if (contentLen.empty()) {
            _request.body.contentLength = 0;
        } else {
            _request.body.contentLength =
                std::atoi(contentLen.begin()->c_str());
        }
    }
    if (_request.body.contentLength > _request.body.recvedLength) {
        _request.body.content.append(_buf.substr(
            0, _request.body.contentLength - _request.body.recvedLength));
        _request.body.recvedLength += _buf.length();
    }
    if (_request.body.contentLength <= _request.body.recvedLength ||
        _buf.empty()) {
        _state = COMPLETED;
        _requestState = StatusCode::getStatusPair(OK);
    }
    _buf.clear();
}
