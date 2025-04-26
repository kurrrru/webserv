// Copyright 2025 Ideal Broccoli

#include "request_parser.hpp"

#include <map>
#include <vector>
#include <string>
#include <utility>

namespace http {

/*
Parser utils
*/

void logInfo(HttpStatus status, const std::string& message) {
    toolbox::logger::StepMark::info("HTTP " +
        toolbox::to_string(static_cast<int>(status)) + ": " + message);
}

bool hasCtlChar(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (std::iscntrl(str[i])) {
            return true;
        }
    }
    return false;
}

bool isUppStr(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (!std::isupper(str[i])) {
            return false;
        }
    }
    return true;
}

bool isTraversalAttack(const std::string& path) {
    std::string normalizePath = path;
    std::size_t pos = 0;
    while ((pos = normalizePath.find("\\")) != std::string::npos) {
        normalizePath.replace(pos, 2, "/");
    }
    while ((pos = normalizePath.find("//", pos)) != std::string::npos) {
        normalizePath.replace(pos, 2, "/");
    }
    std::vector<std::string> splitPath;
    std::string parts;
    // i = 1 start, skip path[0] = '/'
    for (std::size_t i = 1; i < normalizePath.size(); ++i) {
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

void trimSpace(std::string &line) {
    if (line.empty()) {
        return;
    }
    std::size_t front_pos = line.find_first_not_of(symbols::SP);
    if (front_pos == std::string::npos) {
        line.clear();
        return;
    }
    std::size_t rear_pos = line.find_last_not_of(symbols::SP);
    line = line.substr(front_pos, rear_pos - front_pos + 1);
}

HTTPFields::FieldPair splitFieldLine(
    std::string* line) {
        HTTPFields::FieldPair pair;
        std::size_t pos = line->find_first_of(symbols::COLON);
    if (pos != std::string::npos) {
        pair.first = toolbox::trim(line, symbols::COLON);
        while (!line->empty()) {
            std::string value = toolbox::trim(line, symbols::COMMASP);
            trimSpace(value);
            if (!value.empty()) {
                pair.second.push_back(value);
            }
        }
    }
    return pair;
}

bool isChunkedEncoding(HTTPRequest* request) {
    return (
        !(request->fields.getFieldValue(fields::TRANSFER_ENCODING).empty()) &&
        *(request->fields.getFieldValue(fields::TRANSFER_ENCODING).begin()) ==
            "chunked");
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
    _buf += buf;
    if (_buf.find(http::symbols::CRLF) == std::string::npos
        && _validatePos != BODY) {
        return;
    }
    processRequestLine();
    processFields();
    processBody();
    toolbox::logger::StepMark::info("RequestParser: failed");
    if (_validatePos == COMPLETED) {
        logInfo(OK, "Request parse completed");
    }
}

void RequestParser::processRequestLine() {
    if (_validatePos != REQUEST_LINE) {
        return;
    }
    parseRequestLine();
    validateMethod();
    processURI();
    validateVersion();
    if (_request.httpStatus != OK) {
        throw ParseException("");
    }
    _validatePos = HEADERS;
}

void RequestParser::parseRequestLine() {
    std::string line = toolbox::trim(&_buf, symbols::CRLF);
    _request.method = toolbox::trim(&line, symbols::SP);
    _request.uri.fullUri = toolbox::trim(&line, symbols::SP);
    _request.version = toolbox::trim(&line, symbols::SP);
}

void RequestParser::validateMethod() {
    if (_request.method.empty()) {
        _request.setHttpStatus(BAD_REQUEST);
        return;
    }
    if (_request.method != method::GET && _request.method != method::POST &&
        _request.method != method::DELETE && _request.method != method::HEAD) {
        _request.setHttpStatus(BAD_REQUEST);
    }
}

void RequestParser::processURI() {
    parseURI();
    urlDecode();
    validatePath();
}

void RequestParser::parseURI() {
    std::size_t query_pos = _request.uri.fullUri.find(symbols::QUESTION);
    std::size_t frag_pos = _request.uri.fullUri.find(symbols::HASH);
    if (query_pos != std::string::npos && query_pos > frag_pos) {
        _request.setHttpStatus(BAD_REQUEST);
        return;
    }
    if (query_pos != std::string::npos) {
        _request.uri.path = _request.uri.fullUri.substr(0, query_pos);
        if (frag_pos != std::string::npos) {
            _request.uri.fullQuery =
                _request.uri.fullUri.substr(query_pos, frag_pos - query_pos);
        } else {
            _request.uri.fullQuery =
                _request.uri.fullUri.substr(query_pos);
        }
    } else {
        if (frag_pos != std::string::npos) {
            _request.uri.path = _request.uri.fullUri.substr(0);
        } else {
            _request.uri.path = _request.uri.fullUri.substr(0, frag_pos);
        }
    }
    if (query_pos != std::string::npos) {
        parseQuery();
    }
}

void RequestParser::parseQuery() {
    std::string line = _request.uri.fullQuery.substr(1);  // skip '?'
    while (true) {
        std::size_t e_pos = line.find(symbols::EQUAL);
        std::size_t a_pos = line.find(symbols::AMPERSAND);
        if (e_pos == std::string::npos) {
            return;
        }
        if (a_pos != std::string::npos) {
            _request.uri.queryMap[line.substr(0, e_pos)] =
                line.substr(e_pos + 1, a_pos - e_pos - 1);
        } else {
            _request.uri.queryMap[line.substr(0, e_pos)] =
                line.substr(e_pos + 1);
            break;
        }
        line.erase(0, a_pos + 1);
    }
}

void RequestParser::urlDecode() {
    std::size_t p_pos = _request.uri.path.find(symbols::PERCENT);
    if (p_pos == std::string::npos) {
        return;
    }
    std::string res;
    std::size_t i = 0;
    while (i < _request.uri.path.size()) {
        if (_request.uri.path[i] == *symbols::PERCENT) {
            std::string hexStr = _request.uri.path.substr(i + 1, 2);
            std::size_t hex = strtol(hexStr.c_str(), NULL, 16);
            res += static_cast<char>(hex);
            i += 3;
        } else {
            res += _request.uri.path[i];
            ++i;
        }
    }
    _request.uri.path = res;
}

void RequestParser::validatePath() {
    if (_request.uri.fullUri.empty()) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info("RequestParser: uri not found");
        return;
    }
    if (_request.uri.path.size() > http::uri::MAX_PATH_SIZE) {
        _request.setHttpStatus(URI_TOO_LONG);
        toolbox::logger::StepMark::info("RequestParser: uri too long");
        return;
    }
    if (_request.uri.fullUri[0] != *symbols::SLASH ||
        hasCtlChar(_request.uri.fullUri)) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info("RequestParser: invalid uri");
        return;
    }
    if (isTraversalAttack(_request.uri.path)) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info("RequestParser: traversal attack");
    }
}

void RequestParser::validateVersion() {
    if (_request.version.empty()) {
        _request.version = uri::HTTP_VERSION;
    }
    if (hasCtlChar(_request.version)) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info
            ("RequestParser: version has control character");
        return;
    }
    if (_request.version != uri::HTTP_VERSION) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info
            ("RequestParser: invalid version");
    }
}

void RequestParser::processFields() {
    if (_validatePos != HEADERS ||
            _buf.find(symbols::CRLF) == std::string::npos) {
        return;
    }
    if (_request.fields.get().empty()) {
        _request.fields.initFieldsMap();
    }
    while (_buf.find(symbols::CRLF) != std::string::npos) {
        if (_buf.find(symbols::CRLF) == 0) {
            _validatePos = BODY;
            _buf = _buf.substr(sizeof(*symbols::CRLF));
            break;
        }
        std::string line = toolbox::trim(&_buf, symbols::CRLF);
        validateFieldLine(line);
        HTTPFields::FieldPair pair = splitFieldLine(&line);
        if (!_request.fields.parseHeaderLine(pair, _request.httpStatus)) {
            throw ParseException("");
        }
    }
    if (!_request.fields.validateRequestHeaders(_request.httpStatus)) {
        throw ParseException("");
    }
}

void RequestParser::validateFieldLine(std::string& line) {
    if (hasCtlChar(line)) {
        _request.setHttpStatus(BAD_REQUEST);
        logInfo(BAD_REQUEST, "line has CtlChar");
        return;
        // throw ParseException("");
    }
    if (line.size() > fields::MAX_FIELDLINE_SIZE) {
        _request.setHttpStatus(BAD_REQUEST);
        logInfo(BAD_REQUEST, "line is too long");
        return;
        // throw ParseException("");
    }
}

void RequestParser::processBody() {
    if (_validatePos != BODY) {
        return;
    }
    if (_request.body.isChunked || isChunkedEncoding(&_request)) {
        parseChunkedEncoding();
        return;
    }
    if (!_request.body.contentLength) {
        std::vector<std::string>& contentLen =
            _request.fields.getFieldValue(fields::CONTENT_LENGTH);
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
        _request.body.recvedLength += _buf.size();
    }
    if (_request.body.contentLength <= _request.body.recvedLength ||
        _buf.empty()) {
        _validatePos = COMPLETED;
    }
    _buf.clear();
}

void RequestParser::parseChunkedEncoding() {
    if (!_request.body.isChunked) {
        _request.body.isChunked = true;
        _request.body.lastChunk = false;
    }
    while (!_buf.empty() && !_request.body.lastChunk) {
        std::size_t pos = _buf.find(symbols::CRLF);
        if (pos == std::string::npos) {
            return;
        }
        std::string hexStr = _buf.substr(0, pos + 1);
        std::size_t chunkSize;
        char* endPtr;
        chunkSize = strtol(hexStr.c_str(), &endPtr, 16);
        if (*endPtr != *http::symbols::CR) {
            throw ParseException("Error: chunked encoding failed read hexStr");
        }
        if (chunkSize == 0) {
            _request.body.lastChunk = true;
            _validatePos = COMPLETED;
            return;
        }
        _buf = _buf.substr(pos + 2);
        if (_buf.size() < chunkSize + 2) {  // buf + CRLF
            return;
        }
        std::string chunkData = _buf.substr(0, chunkSize);
        _request.body.content.append(chunkData);
        _request.body.recvedLength += chunkSize;
        _buf = _buf.substr(chunkSize + 2);
    }
}

}  // namespace http
