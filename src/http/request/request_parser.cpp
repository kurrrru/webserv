#include <string>
#include <vector>
#include <deque>
#include <numeric>
#include <cstdlib>

#include "request_parser.hpp"

namespace http {
static std::size_t addLength(std::size_t sum, const std::string& s) {
    return sum + s.size();
}

static std::size_t calcTotalLength(const std::deque<std::string>& deq) {
    return std::accumulate(deq.begin(), deq.end(),
        static_cast<std::size_t>(0), addLength);
}

BaseParser::ParseStatus RequestParser::processFieldLine() {
    if (getBuf()->find(symbols::CRLF) == std::string::npos) {
        return P_NEED_MORE_DATA;
    }
    if (_request.fields.get().empty()) {
        _request.fields.initFieldsMap();
    }
    while (getBuf()->find(symbols::CRLF) != std::string::npos) {
        if (getBuf()->find(symbols::CRLF) == 0) {
            if (!FieldValidator::validateRequestHeaders
                (_request.fields, _request.httpStatus)) {
                throw ParseException("");
            }
            setValidatePos(V_BODY);
            setBuf(getBuf()->substr(sizeof(*symbols::CRLF)));
            break;
        }
        std::string line = toolbox::trim(getBuf(), symbols::CRLF);
        if (!FieldValidator::validateFieldLine(line)) {
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
            continue;
        }
        HTTPFields::FieldPair pair = RequestFieldParser::splitFieldLine(&line);
        if (!_fieldParser.parseFieldLine(pair, _request.fields.get(),
                                        _request.httpStatus)) {
            throw ParseException("");
        }
    }
    return P_IN_PROGRESS;
}

BaseParser::ParseStatus RequestParser::processRequestLine() {
    parseRequestLine();
    validateVersion();
    validateMethod();
    processURI();
    if (_request.httpStatus.get() != HttpStatus::OK) {
        throw ParseException("");
    }
    setValidatePos(V_FIELD);
    return P_IN_PROGRESS;
}

void RequestParser::parseRequestLine() {
    std::string line = toolbox::trim(getBuf(), symbols::CRLF);
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
            std::string hexStr = line.substr(i + 1, parser::HEX_DIGIT_LENGTH);
            std::string decodedStr;
            if (decodeHex(hexStr, decodedStr)) {
                res += decodedStr;
                i += 3;  // % + hex num len
            } else {
                if (line[0] == '/' ||
                    i + parser::HEX_DIGIT_LENGTH < line.size()) {  // is path
                    _request.httpStatus.set(HttpStatus::BAD_REQUEST);
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
    if (!utils::hasCtlChar(res)) {
        line = res;
    }
    return;
}

bool RequestParser::decodeHex(std::string& hexStr, std::string& decodedStr) {
    if (hexStr.size() != parser::HEX_DIGIT_LENGTH ||
        !isxdigit(hexStr[0]) || !isxdigit(hexStr[1])) {
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
        normalizePath.replace(slashPos, parser::HEX_DIGIT_LENGTH, "/");
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

BaseParser::ParseStatus RequestParser::processBody() {
    if (_request.body.isChunked || isChunkedEncoding()) {
        parseChunkedEncoding();
        return P_COMPLETED;
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
        _request.body.content.append(getBuf()->substr(
            0, _request.body.contentLength - _request.body.recvedLength));
        _request.body.recvedLength += getBuf()->size();
    }
    if (_request.body.contentLength <= _request.body.recvedLength ||
        getBuf()->empty()) {
            setValidatePos(V_COMPLETED);
    }
    getBuf()->clear();
    return P_IN_PROGRESS;
}

bool RequestParser::isChunkedEncoding() {
    HTTPFields::FieldValue value =
        _request.fields.getFieldValue(fields::TRANSFER_ENCODING);
    return !value.empty() && value[0] == "chunked";
}

void RequestParser::parseChunkedEncoding() {
    if (!_request.body.isChunked) {
        _request.body.isChunked = true;
        _request.body.lastChunk = false;
    }
    while (!getBuf()->empty() && !_request.body.lastChunk) {
        std::size_t pos = getBuf()->find(symbols::CRLF);
        if (pos == std::string::npos) {
            return;
        }
        std::string hexStr = getBuf()->substr(0, pos + 1);
        std::size_t chunkSize;
        char* endPtr;
        chunkSize = strtol(hexStr.c_str(), &endPtr, 16);
        if (*endPtr != *http::symbols::CR) {
            throw ParseException("");
        }
        if (chunkSize == 0) {
            _request.body.lastChunk = true;
            setValidatePos(V_COMPLETED);
            return;
        }
        setBuf(getBuf()->substr(pos + parser::HEX_DIGIT_LENGTH));
        if (getBuf()->size() < chunkSize + parser::HEX_DIGIT_LENGTH) {
            return;
        }
        std::string chunkData = getBuf()->substr(0, chunkSize);
        if (_request.body.recvedLength + chunkSize > fields::MAX_BODY_SIZE) {
            _request.httpStatus.set(HttpStatus::PAYLOAD_TOO_LARGE);
            throw ParseException("");
        }
        _request.body.content.append(chunkData);
        _request.body.recvedLength += chunkSize;
        setBuf(getBuf()->substr(chunkSize + parser::HEX_DIGIT_LENGTH));
    }
}

}  // namespace http
