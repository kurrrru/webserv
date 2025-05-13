#include <string>

#include "cgi_response_parser.hpp"

namespace http {
BaseParser::ParseStatus CgiResponseParser::processFieldLine() {
    while (true) {
        LineEndInfo lineEnd = findLineEnd();
        if (lineEnd.pos == std::string::npos) {
            return P_NEED_MORE_DATA;
        }
        if (lineEnd.pos == 0) {
            return handleFieldEnd(lineEnd.len);
        }
        std::string line = getBuf()->substr(0, lineEnd.pos);
        setBuf(getBuf()->substr(lineEnd.pos + lineEnd.len));
        if (!processFieldLineContent(line)) {
            continue;
        }
    }
}

BaseParser::ParseStatus CgiResponseParser::processBody() {
    _response.body.append(*getBuf());
    getBuf()->clear();
    return P_NEED_MORE_DATA;
}

BaseParser::ParseStatus CgiResponseParser::handleFieldEnd(
    const std::size_t lineEndLen) {
    if (!FieldValidator::validateCgiHeaders(_response.fields,
                                            _response.httpStatus)) {
        throw ParseException("");
    }
    setValidatePos(V_BODY);
    setBuf(getBuf()->substr(lineEndLen));
    return P_IN_PROGRESS;
}

bool CgiResponseParser::processFieldLineContent(std::string& line) {
    if (!FieldValidator::validateFieldLine(line)) {
        _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    HTTPFields::FieldPair pair = BaseFieldParser::splitFieldLine(&line);
    if (pair.second.empty()) {
        return false;
    }
    if (utils::isEqualCaseInsensitive(pair.first, "status") &&
        _response.httpStatus.get() == HttpStatus::UNSET) {
        parseStatus(pair);
    } else {
        _fieldParser.parseFieldLine(pair, _response.fields.get(),
                                    _response.httpStatus);
    }
    return true;
}


CgiResponseParser::LineEndInfo CgiResponseParser::findLineEnd() {
    std::size_t crlfPos = getBuf()->find(symbols::CRLF);
    std::size_t lfPos = getBuf()->find(symbols::LF);
    if (crlfPos == std::string::npos && lfPos == std::string::npos) {
        return LineEndInfo(std::string::npos, 0);
    }
    if (crlfPos < lfPos) {
        return LineEndInfo(crlfPos, symbols::CRLF_SIZE);
    }
    return LineEndInfo(lfPos, symbols::LF_SIZE);
}

bool CgiResponseParser::isValidStatusMessage(int code,
                                             const std::string& message) {
    return message == Response::getStatusMessage(code);
}

bool CgiResponseParser::parseStatus(HTTPFields::FieldPair& pair) {
    if (pair.second.empty() || pair.second.size() > 1) {
        _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    std::string value = pair.second[0];
    if (value.empty()) {
        _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    std::istringstream iss(value);
    int statusCode;
    iss >> statusCode;
    std::string message;
    std::getline(iss >> std::ws, message);
    if (statusCode < 100 || statusCode > 599 ||
        !isValidStatusMessage(statusCode, message)) {
        _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    _response.httpStatus.set(static_cast<HttpStatus::EHttpStatus>(statusCode));
    return true;
}

}  // namespace http
