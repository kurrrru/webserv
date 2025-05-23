#include <string>
#include <vector>
#include <deque>
#include <numeric>
#include <cstdlib>

#include "request_parser.hpp"

namespace http {
namespace {
std::size_t addLength(std::size_t sum, const std::string& s) {
    return sum + s.size();
}

std::size_t calcTotalLength(const std::deque<std::string>& deq) {
    return std::accumulate(deq.begin(), deq.end(),
        static_cast<std::size_t>(0), addLength);
}

std::string removeConsecutiveSpaces(const std::string& str) {
    std::stringstream ss(str);

    std::string word;
    std::string result;
    while (ss >> word) {
        if (!result.empty()) {
            result += " ";
        }
        result += word;
    }
    return result;
}

}  // namespace

BaseParser::ParseStatus RequestParser::processFieldLine() {
    while (true) {
        std::size_t lineEndPos = getBuf()->find(symbols::CRLF);
        if (lineEndPos == std::string::npos) {
            return P_NEED_MORE_DATA;
        }
        if (lineEndPos == 0) {
            if (!FieldValidator::validateRequestHeaders(_request.fields, _request.httpStatus)) {
                toolbox::logger::StepMark::error(
                    "RequestParser: invalid request headers");
                throw ParseException("");
            }
            setValidatePos(V_BODY);
            setBuf(getBuf()->substr(symbols::CRLF_SIZE));
            return P_IN_PROGRESS;
        }

        std::string line = toolbox::trim(getBuf(), symbols::CRLF);
        if (!FieldValidator::validateFieldLine(line)) {
            toolbox::logger::StepMark::error(
                "RequestParser: invalid character in field line");
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
            continue;
        }
        HTTPFields::FieldPair pair = RequestFieldParser::splitFieldLine(&line);
        if (!_fieldParser.parseFieldLine(pair, _request.fields.get(),
                                        _request.httpStatus)) {
            toolbox::logger::StepMark::error("RequestParser: invalid field line");
            throw ParseException("");
        }
    }
    return P_NEED_MORE_DATA;
}

BaseParser::ParseStatus RequestParser::processRequestLine() {
    if (getBuf()->find(symbols::CRLF) == std::string::npos) {
        return P_NEED_MORE_DATA;
    }
    parseRequestLine();
    validateVersion();
    validateMethod();
    processURI();
    if (_request.httpStatus.get() != HttpStatus::OK) {
        toolbox::logger::StepMark::error(
            "RequestParser: invalid request line");
        throw ParseException("");
    }
    setValidatePos(V_FIELD);
    return P_IN_PROGRESS;
}

void RequestParser::parseRequestLine() {
    std::string line = toolbox::trim(getBuf(), symbols::CRLF);
    if (line.find(symbols::SP) == std::string::npos) {
        toolbox::logger::StepMark::error(
            "RequestParser: request line has no space");
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        return;
    }
    _request.originalRequestLine = removeConsecutiveSpaces(line);
    std::stringstream ss(_request.originalRequestLine);

    std::getline(ss, _request.method, *symbols::SP);
    std::getline(ss, _request.uri.fullUri, *symbols::SP);
    std::getline(ss, _request.version);

    if (_request.version.find(symbols::SP) != std::string::npos) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::error(
            "RequestParser: invalid request line");
    }
}

void RequestParser::validateVersion() {
    if (_request.version.empty()) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::error(
            "RequestParser: version not found");
        return;
    }
    if (utils::hasCtlChar(_request.version)) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::error(
            "RequestParser: version has control character");
        return;
    }
    if (!isValidFormat()) {
        toolbox::logger::StepMark::error(
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
        toolbox::logger::StepMark::error(
            "RequestParser: method not found");
        return;
    }
    if (!utils::isUpperStr(_request.method)) {
        _request.httpStatus.set(HttpStatus::BAD_REQUEST);
        toolbox::logger::StepMark::error(
            "RequestParser: method is not uppercase");
        return;
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
        toolbox::logger::StepMark::error(
            "RequestParser: uri not found");
        return;
    }
    if (_request.uri.path.size() > http::uri::MAX_URI_SIZE) {
        _request.httpStatus.set(HttpStatus::URI_TOO_LONG);
        toolbox::logger::StepMark::error(
            "RequestParser: uri too large");
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
                    toolbox::logger::StepMark::error
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
    line = res;
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
                toolbox::logger::StepMark::error(
                    "RequestParser: invalid path, try parent directory");
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
            _request.body.contentLength = std::atoi(contentLen.front().c_str());
        }
    }
    if (_request.body.contentLength > _request.body.recvedLength) {
        std::size_t remainLen = _request.body.contentLength - _request.body.recvedLength;

        _request.body.content += getBuf()->substr(0, remainLen);
        _request.body.recvedLength += _request.body.content.size();
    }
    if (_request.body.contentLength <= _request.body.recvedLength) {
        setValidatePos(V_COMPLETED);
        return P_COMPLETED;
    }
    getBuf()->clear();
    return P_NEED_MORE_DATA;
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
            toolbox::logger::StepMark::error("RequestParser: invalid chunk size");
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
            toolbox::logger::StepMark::error(
                "RequestParser: body size too large");
            throw ParseException("");
        }
        _request.body.content.append(chunkData);
        _request.body.recvedLength += chunkSize;
        setBuf(getBuf()->substr(chunkSize + parser::HEX_DIGIT_LENGTH));
    }
}

}  // namespace http
