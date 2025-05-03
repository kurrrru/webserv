#pragma once

#include <string>

#include "request/http_fields.hpp"

namespace http {

class BaseFieldParser {
 public:
    BaseFieldParser() {}
    virtual ~BaseFieldParser() {}

    bool parseFieldLine(const HTTPFields::FieldPair& pair, HttpStatus& hs);
    static HTTPFields::FieldPair splitFieldLine(std::string& line);
    // bool parseCgiFieldLine(const HTTPFields::FieldPair& pair, HttpStatus& hs);

 protected:
    bool hostFieldLine(HTTPFields::FieldMap::iterator& target,
                     const HTTPFields::FieldPair& pair, HttpStatus& hs);
    bool uniqueFieldLine(HTTPFields::FieldMap::iterator& target,
                     const HTTPFields::FieldPair& pair, HttpStatus& hs);
    bool normalFieldLine(HTTPFields::FieldMap::iterator& target,
                     const HTTPFields::FieldPair& pair);
    bool validateHost(const HTTPFields::FieldValue& values);
};

}
