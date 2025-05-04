#pragma once

#include <string>

#include "../base_parser.hpp"
#include "../http_status.hpp"
#include "../field_validator.hpp"
#include "../base_field_parser.hpp"
#include "http_request.hpp"
#include "request_field_parser.hpp"

namespace http {

class RequestParser : public BaseParser {
 public:
    RequestParser() { _validatePos = REQUEST_LINE; }
    ~RequestParser() {}
    void run(const std::string& buf);
    HTTPRequest& get() { return _request; }

 private:
    HTTPRequest _request;
    RequestFieldParser _fieldParser;

    void processRequestLine();
    void parseRequestLine();
    void validateVersion();
    bool isValidFormat();
    void validateMethod();
    void processURI();
    void parseURI();
    void validatePath();
    void pathDecode();
    void percentDecode(std::string& line);
    bool decodeHex(std::string& hexStr, std::string& decodedStr);
    void parseQuery();
    void normalizationPath();
    void verifySafePath();

    void processFieldLine();
    void processBody();
    bool isChunkedEncoding();
    void parseChunkedEncoding();

};

}
