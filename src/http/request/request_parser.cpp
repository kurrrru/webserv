#include <string>
#include <vector>
#include <deque>
#include <numeric>
#include <cstdlib>
#include <limits>
#include <sstream>

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

bool parseChunkSize(const std::string& chunkSizeStr, std::size_t& chunkSize) {
    if (chunkSizeStr.empty()) {
        return false;
    }

    char* endPtr = NULL;
    chunkSize = std::strtol(chunkSizeStr.c_str(), &endPtr, 16);
    return (*endPtr == '\0');
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
        std::strtol(_request.version.substr(5, dotPos).c_str(), &endptr, 10);
    std::size_t minorVersion =
        std::strtol(_request.version.substr(dotPos + 1).c_str(), &endptr, 10);
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
        !std::isxdigit(hexStr[0]) || !std::isxdigit(hexStr[1])) {
        return false;
    }
    char* endptr = NULL;
    std::size_t hex = std::strtol(hexStr.c_str(), &endptr, 16);
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
    if (isChunkedEncoding()) {
        _request.body.isChunked = true;
        ParseStatus status = parseChunkedEncoding();
        if (status == P_COMPLETED || status == P_ERROR) {
            return status;
        }
        return P_NEED_MORE_DATA;
    }
    if (_request.body.contentLength == std::numeric_limits<std::size_t>::max()) {
        std::vector<std::string>& contentLen =
            _request.fields.getFieldValue(fields::CONTENT_LENGTH);
        if (contentLen.empty()) {
            _request.body.contentLength = 0;
        } else {
            _request.body.contentLength = std::atoi(contentLen.front().c_str());
        }
    }
    if (_request.body.contentLength > _request.body.receivedLength) {
        std::size_t remainLen = _request.body.contentLength - _request.body.receivedLength;

        _request.body.content += getBuf()->substr(0, remainLen);
        _request.body.receivedLength = _request.body.content.size();
    }
    if (_request.body.contentLength <= _request.body.receivedLength) {
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

void RequestParser::solveChunkedBody(std::string& recvBody) {
    std::string unchunkedBody;
    std::size_t pos = 0;

    while (pos < recvBody.size()) {
        std::size_t chunkSizeEnd = recvBody.find(symbols::CRLF, pos);
        if (chunkSizeEnd == std::string::npos) {
            toolbox::logger::StepMark::error(
                "runPost: solveChunkedBody failed: chunk size end not found");
            throw HttpStatus::BAD_REQUEST;
        }

        std::string chunkSizeStr = recvBody.substr(pos, chunkSizeEnd - pos);

        std::size_t chunkSize;
        if (!parseChunkSize(chunkSizeStr, chunkSize)) {
            toolbox::logger::StepMark::error(
                "runPost: solveChunkedBody failed: invalid chunk size " +
                chunkSizeStr);
            throw HttpStatus::BAD_REQUEST;
        }
        if (chunkSize == 0) {
            break;
        }

        pos = chunkSizeEnd + symbols::CRLF_SIZE;
        if (pos + chunkSize > recvBody.size()) {
            toolbox::logger::StepMark::error(
                "runPost: solveChunkedBody failed: chunk size exceeds body "
                "size");
            throw HttpStatus::BAD_REQUEST;
        }

        std::string chunkData = recvBody.substr(pos, chunkSize);
        pos += chunkSize;

        if (pos + symbols::CRLF_SIZE > recvBody.size() ||
            recvBody.find(symbols::CRLF, pos) != pos) {
            toolbox::logger::StepMark::error(
                "runPost: solveChunkedBody failed: CRLF not found after chunk "
                "data");
            throw HttpStatus::BAD_REQUEST;
        }
        pos += symbols::CRLF_SIZE;

        unchunkedBody += chunkData;
    }
    recvBody = unchunkedBody;
}

BaseParser::ParseStatus RequestParser::parseChunkedEncoding() {
    _request.body.content += *getBuf();
    getBuf()->clear();

    std::string& body = _request.body.content;
    std::size_t pos = _request.body.receivedLength;

    while (pos < body.size()) {
        std::size_t chunkSizeEnd = body.find(symbols::CRLF, pos);
        if (chunkSizeEnd == std::string::npos) {
            return P_NEED_MORE_DATA;
        }

        std::string chunkSizeStr = body.substr(pos, chunkSizeEnd - pos);
        std::size_t chunkSize;
        if (!parseChunkSize(chunkSizeStr, chunkSize)) {
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
            toolbox::logger::StepMark::error("parseChunkedEncoding: invalid chunk size");
            return P_ERROR;
        }

        if (chunkSize == 0) {
            std::size_t finalCRLFPos = chunkSizeEnd + symbols::CRLF_SIZE;
            if (finalCRLFPos + symbols::CRLF_SIZE <= body.size() &&
                body.find(symbols::CRLF, finalCRLFPos) == finalCRLFPos) {
                solveChunkedBody(_request.body.content);
                return P_COMPLETED;
            }
            return P_NEED_MORE_DATA;
        }

        std::size_t chunkDataStart = chunkSizeEnd + symbols::CRLF_SIZE;
        std::size_t chunkDataEnd = chunkDataStart + chunkSize;
        std::size_t chunkEnd = chunkDataEnd + symbols::CRLF_SIZE;

        if (chunkEnd > body.size()) {
            return P_NEED_MORE_DATA;
        }

        if (body.substr(chunkDataEnd, symbols::CRLF_SIZE) != symbols::CRLF) {
            _request.httpStatus.set(HttpStatus::BAD_REQUEST);
            toolbox::logger::StepMark::error("parseChunkedEncoding: CRLF not found after chunk");
            return P_ERROR;
        }

        pos = chunkEnd;
        _request.body.receivedLength = pos;
    }
    return P_NEED_MORE_DATA;
}

}  // namespace http
