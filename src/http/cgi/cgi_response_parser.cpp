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
            setValidatePos(V_BODY);
            setBuf(getBuf()->substr(lineEndLen));
            return;
        }
        std::string line = getBuf()->substr(0, lineEndPos);
        setBuf(getBuf()->substr(lineEndPos + lineEndLen));
        if (!FieldValidator::validateFieldLine(line)) {
            break;
        }
        HTTPFields::FieldPair pair = BaseFieldParser::splitFieldLine(&line);
        if (pair.second.empty()) {
            continue;
        }
        if (utils::isEqualCaseInsensitive(pair.first, "status") &&
            _response.httpStatus.get() == HttpStatus::UNSET) {
                _response.httpStatus.set(std::atoi(pair.second[0].c_str()));
        } else {
            _fieldParser.parseFieldLine(pair, _response.fields.get(),
                _response.httpStatus);
        }
        crlfPos = getBuf()->find(symbols::CRLF);
        lfPos = getBuf()->find(symbols::LF);
    }
    return P_NEED_MORE_DATA;
}

}  // namespace http
