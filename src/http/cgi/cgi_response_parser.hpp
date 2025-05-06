#pragma once

#include <cstdlib>

#include "../parsing/base_field_parser.hpp"
#include "../parsing/field_validator.hpp"
#include "../parsing/base_parser.hpp"
#include "../string_utils.hpp"
#include "cgi_field_parser.hpp"
#include "cgi_response.hpp"

namespace http {
class CgiResponseParser : public BaseParser {
 public:
    inline CgiResponseParser() { setValidatePos(V_FIELD); }
    ~CgiResponseParser() {}

    inline CgiResponse& get() { return _response; }

 private:
    ParseStatus processRequestLine() { return BaseParser::P_ERROR; }
    ParseStatus processFieldLine();
    ParseStatus processBody();
    bool parseStatus(HTTPFields::FieldPair& pair);

    CgiResponse _response;
    CgiFieldParser _fieldParser;
};

}  // namespace http
