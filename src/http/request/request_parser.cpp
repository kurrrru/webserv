#include <deque>
#include <numeric>
#include <cstdlib>

#include "request_parser.hpp"
#include "../string_utils.hpp"


namespace http {

static std::size_t addLength(std::size_t sum, const std::string& s) {
    return sum + s.size();
}

static std::size_t calcTotalLength(const std::deque<std::string>& deq) {
    return std::accumulate(deq.begin(), deq.end(),
        static_cast<std::size_t>(0), addLength);
}

void RequestParser::run(const std::string& buf) {
    _buf += buf;
    if (_buf.find(http::symbols::CRLF) == std::string::npos &&
        _validatePos != BODY) {
        return;
    }
    processRequestLine();
    processFieldLine();
    processBody();
}

void RequestParser::processFieldLine() {
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
        if (!FieldValidator::validateFieldLine(line)) {
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
            continue;
        }
        HTTPFields::FieldPair pair = RequestFieldParser::splitFieldLine(&line);
        if (!_fieldParser.parseFieldLine(pair, _request.fields.get(), _request.httpStatus)) {
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
            throw ParseException("");
        }
    }
    if (!FieldValidator::validateRequestHeaders(_request.fields, _request.httpStatus)) {
        throw ParseException("");
    }
}

void RequestParser::processRequestLine() {
    if (_validatePos != REQUEST_LINE) {
        return;
    }
    parseRequestLine();
    validateVersion();
    validateMethod();
    processURI();
    if (_request.httpStatus.get() != HttpStatus::OK) {
        throw ParseException("");
    }
    _validatePos = HEADERS;
}

void RequestParser::parseRequestLine() {
    std::string line = toolbox::trim(&_buf, symbols::CRLF);
    if (line.find(symbols::SP) == std::string::npos) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        return;
    }
    _request.method = toolbox::trim(&line, symbols::SP);
    utils::skipSpace(&line);
    _request.uri.fullUri = toolbox::trim(&line, symbols::SP);
    utils::skipSpace(&line);
    _request.version = toolbox::trim(&line, symbols::SP);
    if (!line.empty()) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
    }
}

void RequestParser::validateVersion() {
    if (_request.version.empty()) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::info(
            "RequestParser: version not found");
        return;
    }
    if (utils::hasCtlChar(_request.version)) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::info(
            "RequestParser: version has control character");
        return;
    }
    if (!isValidFormat()) {
        toolbox::logger::StepMark::info(
            "RequestParser: invalid version format");
        return;
    }
}

bool RequestParser::isValidFormat() {
    if (_request.version.empty() || _request.version.find("HTTP/") != 0) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        return false;
    }
    std::size_t dotPos = _request.version.find(".");
    if (dotPos == std::string::npos) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        return false;
    }
    char* endptr = NULL;
    std::size_t majorVersion =
        strtol(_request.version.substr(5, dotPos).c_str(), &endptr, 10);
    std::size_t minorVersion =
        strtol(_request.version.substr(dotPos + 1).c_str(), &endptr, 10);
    if (majorVersion < uri::HTTP_MAJOR_VERSION ||
        minorVersion > uri::HTTP_MINOR_VERSION_MAX) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        return false;
    }
    if (majorVersion > 1) {
        _request.httpStatus.set(HttpStatus::HTTP_VERSION_NOT_SUPPORTED);
        return false;
    }
    return true;
}

void RequestParser::validateMethod() {
    if (_request.method.empty()) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        return;
    }
    if (_request.version == uri::HTTP_VERSION_1_0) {
        if (_request.method != method::GET &&
            _request.method != method::HEAD &&
            _request.method != method::POST) {
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        }
    } else {
        if (_request.method != method::GET &&
            _request.method != method::HEAD &&
            _request.method != method::POST &&
            _request.method != method::DELETE) {
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        }
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

void RequestParser::parseURI() {
    std::size_t queryPos = _request.uri.fullUri.find(symbols::QUESTION);
    std::size_t fragPos = _request.uri.fullUri.find(symbols::HASH);
    if (queryPos != std::string::npos && queryPos < fragPos) {
        _request.uri.path = _request.uri.fullUri.substr(0, queryPos);
        if (fragPos != std::string::npos) {
            _request.uri.fullQuery =
                _request.uri.fullUri.substr(queryPos, fragPos - queryPos);
        } else {
            _request.uri.fullQuery = _request.uri.fullUri.substr(queryPos);
        }
    } else {
        _request.uri.path = _request.uri.fullUri.substr(0, fragPos);
    }
}

void RequestParser::validatePath() {
    if (_request.uri.fullUri.empty()) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::info(
            "RequestParser: uri not found");
        return;
    }
    if (_request.uri.path.size() > http::uri::MAX_URI_SIZE) {
        _request.httpStatus.set(HttpStatus::URI_TOO_LONG);
        toolbox::logger::StepMark::info(
            "RequestParser: uri too large");

        return;
    }
    if (_request.uri.fullUri[0] != *symbols::SLASH ||
        utils::hasCtlChar(_request.uri.fullUri)) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::info(
            "RequestParser: invalid uri");

        return;
    }
}

void RequestParser::pathDecode() {
    if (_request.httpStatus.get() != HttpStatus::OK) {
        return;
    }
    percentDecode(_request.uri.path);
    percentDecode(_request.uri.fullQuery);
}

void RequestParser::percentDecode(std::string& line) {
    if (line.find(symbols::PERCENT) == std::string::npos) {
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
                    _request.httpStatus.set(HttpStatus::BAD_REQUEST);
                    toolbox::logger::StepMark::info("RequestParser: path has invalid hexdecimal");
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
    if (!utils::hasCtlChar(res)) {
        line = res;
    }
    return;
}

bool RequestParser::decodeHex(std::string& hexStr, std::string& decodedStr) {
    if (hexStr.size() != 2 || !isxdigit(hexStr[0]) || !isxdigit(hexStr[1])) {
        return false;
    }
    char* endptr = NULL;
    std::size_t hex = strtol(hexStr.c_str(), &endptr, 16);
    decodedStr = static_cast<char>(hex);
    if (*endptr != '\0' || hex == '\0') {
        return false;
    }
    return true;
}

void RequestParser::parseQuery() {
    if (_request.uri.fullQuery.empty()) {
        return;
    }
    std::string line = _request.uri.fullQuery.substr(1);  // skip '?'
    while (true) {
        std::size_t ePos = line.find(symbols::EQUAL);
        std::size_t aPos = line.find(symbols::AMPERSAND);
        if (ePos == std::string::npos) {
            return;
        }
        if (aPos != std::string::npos) {
            _request.uri.queryMap[line.substr(0, ePos)] =
                line.substr(ePos + 1, aPos - ePos - 1);
        } else {
            _request.uri.queryMap[line.substr(0, ePos)] = line.substr(ePos + 1);
            break;
        }
        line.erase(0, aPos + 1);
    }
}

void RequestParser::normalizationPath() {
    std::string normalizePath = _request.uri.path;
    std::size_t slashPos = 0;
    while ((slashPos = normalizePath.find("//", slashPos)) !=
           std::string::npos) {
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
                _request.httpStatus.set(HttpStatus::BAD_REQUEST);
                return;
            } else {
                pathDeque.pop_front();
            }
        } else if (_request.uri.splitPath[i] != "/.") {
            pathDeque.push_front(_request.uri.splitPath[i]);
        }
    }
    _request.uri.path.clear();
    std::size_t totalLength = calcTotalLength(pathDeque);
    _request.uri.path.reserve(totalLength);
    for (int i = pathDeque.size() - 1; i >= 0; --i) {
        _request.uri.path += pathDeque[i];
    }
}

void RequestParser::processBody() {
    if (_validatePos != BODY) {
        return;
    }
    if (_request.body.isChunked || isChunkedEncoding()) {
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

bool RequestParser::isChunkedEncoding() {
    HTTPFields::FieldValue value = _request.fields.getFieldValue(fields::TRANSFER_ENCODING);
    return !value.empty() && value[0] == "chunked";

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

}
