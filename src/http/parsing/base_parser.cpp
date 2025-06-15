#include <string>

#include "base_parser.hpp"

namespace http {
BaseParser::ParseException::ParseException(const char* message)
: _message(message) {}

BaseParser::ParseException::ParseException(const ParseException& other)
: _message(other._message) {}

const char* BaseParser::ParseException::what() const throw() {
    return _message;
}

BaseParser::ParseException::~ParseException() throw() {
}

BaseParser::ParseStatus BaseParser::run(const std::string& buf) {
    if (_parseStatus == P_ERROR || _parseStatus == P_COMPLETED) {
        return _parseStatus;
    }
    _buf += buf;
    _parseStatus = P_IN_PROGRESS;
    try {
        while (_parseStatus == P_IN_PROGRESS) {
            switch (_validatePos) {
                case V_REQUEST_LINE:
                    _parseStatus = processRequestLine();
                    break;
                case V_FIELD:
                    _parseStatus = processFieldLine();
                    break;
                case V_BODY:
                    _parseStatus = processBody();
                    break;
                case V_COMPLETED:
                    _parseStatus = P_COMPLETED;
                    break;
            }
        }
    } catch (std::exception& e) {
        _parseStatus = P_ERROR;
    }
    return _parseStatus;
}

std::size_t BaseParser::findNewLinePos(std::string& buffer) {
    std::size_t crlfPos = buffer.find(symbols::CRLF);
    std::size_t lfPos = buffer.find(symbols::LF);
    if (crlfPos == std::string::npos && lfPos == std::string::npos) {
        return std::string::npos;
    }
    if (crlfPos < lfPos) {
        return crlfPos;
    }
    return lfPos;
}

std::size_t BaseParser::getLineEndLen
(std::string& line, std::size_t lineEndPos) {
    if (line.find(symbols::CRLF) == lineEndPos) {
        return symbols::CRLF_SIZE;
    }
    return symbols::LF_SIZE;
}

void BaseParser::reset() {
    _parseStatus = P_IN_PROGRESS;
    _buf.clear();
    _validatePos = V_REQUEST_LINE;
}

}  // namespace http
