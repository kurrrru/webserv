#pragma once

#include <string>
#include <sstream>

#include "../parsing/base_parser.hpp"
#include "../parsing/field_validator.hpp"
#include "../parsing/base_field_parser.hpp"
#include "../http_status.hpp"
#include "../string_utils.hpp"
#include "request_field_parser.hpp"
#include "http_request.hpp"


namespace http {
namespace parser {
const std::size_t HEX_DIGIT_LENGTH = 2;
}

class RequestParser : public BaseParser {
 public:
    RequestParser() { setValidatePos(V_REQUEST_LINE); }
    ~RequestParser() {}
    HTTPRequest& get() { return _request; }

 private:
    RequestParser(const RequestParser& other);
    RequestParser& operator=(const RequestParser& other);

    ParseStatus processRequestLine();
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
    ParseStatus processFieldLine();
    ParseStatus processBody();
    bool isChunkedEncoding();
    ParseStatus parseChunkedEncoding();
    void solveChunkedBody(std::string& recvBody);

    HTTPRequest _request;
    RequestFieldParser _fieldParser;
};

}  // namespace http
