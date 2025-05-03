#include <string>

#include "http_namespace.hpp"
#include "base_parser.hpp"

namespace http {
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
        return 2;
    }
    return 1;
}

}