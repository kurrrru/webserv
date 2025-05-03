#pragma once

#include <string>

#include "../base_parser.hpp"
#include "http_request.hpp"

namespace http {

class RequestParser : public BaseParser {
 public:
    RequestParser() { _validatePos = REQUEST_LINE; }
    ~RequestParser() {}
    void run(const std::string& buf);

 private:
    HTTPRequest _request;

    void processRequestLine();
    void parseRequestLine();
    void validateMethod();
    void processURI();
    void parseURI();
    void pathDecode();
    void percentDecode(std::string& line);
    bool decodeHex(std::string& hexStr, std::string& decodedChar);
    void parseQuery();
    void validatePath();
    void normalizationPath();
    void verifySafePath();
    void validateVersion();
    bool isValidFormat();
    void parseChunkedEncoding();
    void processBody();
};

}
