#include <string>

#include "cgi_response_parser.hpp"

namespace http {
BaseParser::ParseStatus CgiResponseParser::processFieldLine() {
    if (getBuf()->find(symbols::CRLF) == std::string::npos) {
        return P_NEED_MORE_DATA;
    }
    std::size_t crlfPos = getBuf()->find(symbols::CRLF);
    std::size_t lfPos = getBuf()->find(symbols::LF);
    while (crlfPos != std::string::npos || lfPos != std::string::npos) {
        std::size_t lineEndPos;
        std::size_t lineEndLen;
        if (crlfPos < lfPos) {
            lineEndPos = crlfPos;
            lineEndLen = 2;  // symbolx::CRLF.size()
        } else {
            lineEndPos = lfPos;
            lineEndLen = 1;  // symbols::LF.size()
        }
        if (lineEndPos == 0) {
            if (!FieldValidator::validateCgiHeaders
                (_response.fields, _response.httpStatus)) {
                throw ParseException("");
            }
            setValidatePos(V_BODY);
            setBuf(getBuf()->substr(lineEndLen));
            return P_IN_PROGRESS;
        }
        std::string line = getBuf()->substr(0, lineEndPos);
        setBuf(getBuf()->substr(lineEndPos + lineEndLen));
        if (!FieldValidator::validateFieldLine(line)) {
            _response.httpStatus.set(HttpStatus::INTERNAL_SERVER_ERROR);
            continue;
        }
        HTTPFields::FieldPair pair = BaseFieldParser::splitFieldLine(&line);
        if (pair.second.empty()) {
            continue;
        }
        if (utils::isEqualCaseInsensitive(pair.first, "status") &&
            _response.httpStatus.get() == HttpStatus::UNSET) {
                parseStatus(pair);
        } else {
            _fieldParser.parseFieldLine(pair, _response.fields.get(),
                _response.httpStatus);
        }
        crlfPos = getBuf()->find(symbols::CRLF);
        lfPos = getBuf()->find(symbols::LF);
    }
    return P_NEED_MORE_DATA;
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

BaseParser::ParseStatus CgiResponseParser::processBody() {
    _response.body.append(*getBuf());
    getBuf()->clear();
    return P_NEED_MORE_DATA;
}

}  // namespace http
