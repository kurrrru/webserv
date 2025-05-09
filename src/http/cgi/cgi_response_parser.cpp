#include <string>

#include "cgi_response_parser.hpp"

namespace http {
BaseParser::ParseStatus CgiResponseParser::processFieldLine() {
    if (getBuf()->find(symbols::CRLF) == std::string::npos) {
        return P_NEED_MORE_DATA;
    }
    while (true) {
        LineEndInfo lineEnd = findLineEnd();
        if (lineEnd.pos == std::string::npos) {
            return P_NEED_MORE_DATA;
        }
        if (lineEnd.pos == 0) {
            return handleFieldEnd();
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

BaseParser::ParseStatus CgiResponseParser::handleFieldEnd() {
    if (!FieldValidator::validateCgiHeaders(_response.fields,
                                            _response.httpStatus)) {
        throw ParseException("");
    }
    setValidatePos(V_BODY);
    setBuf(getBuf()->substr(2));  // Skip CRLF
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
        return LineEndInfo(crlfPos, 2);  // symbols::CRLF.size()
    }
    return LineEndInfo(lfPos, 1);  // symbols::LF.size()
}


bool CgiResponseParser::parseStatus(HTTPFields::FieldPair& pair) {
    if (pair.second.empty()) {
        _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    std::string value = pair.second[0];
    if (value.empty()) {
        _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    int statusCode = 0;
    std::istringstream iss(value);
    iss >> statusCode;
    if (statusCode < 100 || statusCode > 599) {
        _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }
    _response.httpStatus.set(static_cast<HttpStatus::EHttpStatus>(statusCode));
    return true;
}

}  // namespace http
