// Copyright 2025 Ideal Broccoli

#include "request_parser.hpp"

#include <map>
#include <vector>
#include <deque>
#include <string>
#include <utility>

namespace http {

// Parser utils

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

static void trimSpace(std::string &line) {
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

bool isChunkedEncoding(HTTPRequest* request) {
    return (
        !(request->fields.getFieldValue(fields::TRANSFER_ENCODING).empty()) &&
        *(request->fields.getFieldValue(fields::TRANSFER_ENCODING).begin()) ==
            "chunked");
}

static void skipSpace(std::string* line) {
    std::size_t not_sp_pos = line->find_first_not_of(symbols::SP);
    if (not_sp_pos == std::string::npos) {
        return;
    }
    *line = line->substr(not_sp_pos);
}

// Class method

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
    if (line.find(symbols::SP) == std::string::npos) {
        _request.setHttpStatus(BAD_REQUEST);
        return;
    }
    _request.method = toolbox::trim(&line, symbols::SP);
    skipSpace(&line);
    _request.uri.fullUri = toolbox::trim(&line, symbols::SP);
    skipSpace(&line);
    _request.version = toolbox::trim(&line, symbols::SP);
    if (!line.empty()) {
        _request.setHttpStatus(BAD_REQUEST);
    }
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
    validatePath();
    pathDecode();
    parseQuery();
    normalizationPath();
    verifySafePath();
}

void RequestParser::normalizationPath() {
    std::string normalizePath = _request.uri.path;
    std::size_t slashPos = 0;
    while ((slashPos = normalizePath.find("//", slashPos))
        != std::string::npos) {
        normalizePath.replace(slashPos, 2, "/");
    }
    std::string parts;
    for (std::size_t i = 0; i < normalizePath.size(); ++i) {
        if (normalizePath[i] == '/' && !parts.empty()) {
            _request.uri.splitPath.push_back(parts);
            parts.clear();
        }
        parts += normalizePath[i];
    }
    if (!parts.empty()) {
        _request.uri.splitPath.push_back(parts);
    }
}

void RequestParser::verifySafePath() {
    std::deque<std::string> pathDeque;
    for (std::size_t i = 0; i < _request.uri.splitPath.size(); ++i) {
        if (_request.uri.splitPath[i] == "/..") {
            if (pathDeque.empty()) {
                _request.setHttpStatus(BAD_REQUEST);
                return;
            } else {
                pathDeque.pop_front();
            }
        } else if (_request.uri.splitPath[i] != "/.") {
            pathDeque.push_front(_request.uri.splitPath[i]);
        }
    }
    _request.uri.path.clear();
    for (int i = pathDeque.size() - 1; i >= 0; --i) {
        _request.uri.path += pathDeque[i];
    }
}

void RequestParser::parseURI() {
    std::size_t query_pos = _request.uri.fullUri.find(symbols::QUESTION);
    std::size_t frag_pos = _request.uri.fullUri.find(symbols::HASH);
    if (query_pos != std::string::npos && query_pos < frag_pos) {
        _request.uri.path = _request.uri.fullUri.substr(0, query_pos);
        if (frag_pos != std::string::npos) {
            _request.uri.fullQuery =
                _request.uri.fullUri.substr(query_pos, frag_pos - query_pos);
        } else {
            _request.uri.fullQuery =
                _request.uri.fullUri.substr(query_pos);
        }
    } else {
        _request.uri.path = _request.uri.fullUri.substr
            (0, frag_pos);
    }
}

void RequestParser::parseQuery() {
    if (_request.uri.fullQuery.empty()) {
        return;
    }
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

void RequestParser::pathDecode() {
    if (_request.httpStatus != OK) {
        return;
    }
    percentDecode(_request.uri.path);
    percentDecode(_request.uri.fullQuery);
}

void RequestParser::percentDecode(std::string& line) {
    std::size_t p_pos = line.find(symbols::PERCENT);
    if (p_pos == std::string::npos) {
        return;
    }
    std::string res;
    std::size_t i = 0;
    while (i < line.size()) {
        if (line[i] == *symbols::PERCENT) {
            std::string hexStr = line.substr(i + 1, 2);
            std::string decodedStr;
            if (decodeHex(hexStr, decodedStr)) {
                res += decodedStr;
                i += 3;  // % + hex num len
            } else {
                if (line[0] == '/' || i + 2 < line.size()) {  // is path
                    _request.setHttpStatus(BAD_REQUEST);
                    toolbox::logger::StepMark::info
                        ("RequestParser: path has invalid hexdecimal");
                    return;
                }
                res += line[i];
                ++i;
            }
        } else {
            res += line[i];
            ++i;
        }
    }
    if (!hasCtlChar(res)) {
        line = res;
    }
    return;
}

bool RequestParser::decodeHex(std::string& hexStr, std::string& decodedStr) {
    if (hexStr.size() != 2) {
        return false;
    }
    for (std::size_t i = 0; i < 2; ++i) {
        if (!isxdigit(hexStr[i])) {
            return false;
        }
    }
    char* endptr = NULL;
    std::size_t hex = strtol(hexStr.c_str(), &endptr, 16);
    decodedStr = static_cast<char>(hex);
    if (*endptr != '\0') {
        return false;
    }
    if (hex == '\0') {
        return false;
    }
    return true;
}

void RequestParser::validatePath() {
    if (_request.uri.fullUri.empty()) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info("RequestParser: uri not found");
        return;
    }
    if (_request.uri.path.size() > http::uri::MAX_URI_SIZE) {
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
}

void RequestParser::validateVersion() {
    if (_request.version.empty()) {
        toolbox::logger::StepMark::info
            ("RequestParser: version not found");
        _request.setHttpStatus(BAD_REQUEST);
        return;
    }
    if (hasCtlChar(_request.version)) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info
            ("RequestParser: version has control character");
        return;
    }
    if (!isValidFormat()) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info
            ("RequestParser: version invalid format");
        return;
    }
    if (_request.version != uri::HTTP_VERSION_1_0 &&
        _request.version != uri::HTTP_VERSION_1_1) {
        _request.setHttpStatus(BAD_REQUEST);
        toolbox::logger::StepMark::info
            ("RequestParser: invalid version");
    }
}

bool RequestParser::isValidFormat() {
    if (_request.version.empty() || _request.version.find("HTTP/") != 0) {
        _request.setHttpStatus(BAD_REQUEST);
        return false;
    }
    std::size_t dotPos = _request.version.find(".");
    if (dotPos == std::string::npos) {
        _request.setHttpStatus(BAD_REQUEST);
        return false;
    }
    char *endptr = NULL;
    std::size_t majorVersion =
        strtol(_request.version.substr(5, dotPos).c_str(), &endptr, 10);
    std::size_t minorVersion =
        strtol(_request.version.substr(dotPos + 1).c_str(), &endptr, 10);
    if (majorVersion < uri::HTTP_MAJOR_VERSION ||
            minorVersion > uri::HTTP_MINOR_VERSION_MAX) {
        _request.setHttpStatus(BAD_REQUEST);
        return false;
    }
    if (majorVersion > 1) {
        _request.setHttpStatus(HTTP_VERSION_NOT_SUPPORTED);
        return false;
    }
    return true;
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
    }
    if (line.size() > fields::MAX_FIELDLINE_SIZE) {
        _request.setHttpStatus(BAD_REQUEST);
        logInfo(BAD_REQUEST, "line is too long");
        return;
    }
}

HTTPFields::FieldPair RequestParser::splitFieldLine(std::string* line) {
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
