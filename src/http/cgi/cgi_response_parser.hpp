#pragma once

#include <string>

#include <cstdlib>

#include "../parsing/base_field_parser.hpp"
#include "../parsing/field_validator.hpp"
#include "../parsing/base_parser.hpp"
#include "../response/response.hpp"
#include "../string_utils.hpp"
#include "cgi_field_parser.hpp"
#include "cgi_response.hpp"

namespace http {
class CgiResponseParser : public BaseParser {
 public:
    struct LineEndInfo {
        LineEndInfo(std::size_t pos, std::size_t len) : pos(pos), len(len) {}
        std::size_t pos;
        std::size_t len;
    };
    inline CgiResponseParser() { setValidatePos(V_FIELD); }
    ~CgiResponseParser() {}

    inline CgiResponse& get() { return _response; }
    void reset();

 private:
    ParseStatus processRequestLine() { return BaseParser::P_ERROR; }
    ParseStatus processFieldLine();
    ParseStatus processBody();
    bool processFieldLineContent(std::string& line);
    LineEndInfo findLineEnd();
    BaseParser::ParseStatus handleFieldEnd(const std::size_t lineEndLen);
    bool parseStatus(HTTPFields::FieldPair& pair);
    bool isValidStatusMessage(int code, const std::string& message);

    CgiResponse _response;
    CgiFieldParser _fieldParser;
};

}  // namespace http
