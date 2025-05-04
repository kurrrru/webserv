#pragma once

#include <string>

#include "request/http_fields.hpp"

namespace http {

class BaseFieldParser {
 public:
    BaseFieldParser() {}
    virtual ~BaseFieldParser() {}

    bool parseFieldLine(const HTTPFields::FieldPair& pair,
        HTTPFields::FieldMap& fields, HttpStatus& hs);
    static HTTPFields::FieldPair splitFieldLine(std::string* line);
    // bool parseCgiFieldLine(const HTTPFields::FieldPair& pair,
                            // HttpStatus& hs);

 protected:
    virtual bool isUnique(const std::string& key) = 0;
    bool hostFieldLine(HTTPFields::FieldMap::iterator& target,
                    const HTTPFields::FieldPair& pair,
                    HttpStatus& hs);
    virtual bool uniqueFieldLine(HTTPFields::FieldMap::iterator& target,
                    const HTTPFields::FieldPair& pair,
                    HttpStatus& hs);
    bool normalFieldLine(HTTPFields::FieldMap::iterator& target,
                        HTTPFields::FieldMap& fieldMap,
                        const HTTPFields::FieldPair& pair);
    bool validateHost(const HTTPFields::FieldValue& values);
    virtual void handleInvalidFieldError(const std::string& key,
                                        HttpStatus& hs) = 0;
    virtual void handleDuplicateFieldError(const std::string& key,
                                        HttpStatus& hs) = 0;
};

}  // namespace http
