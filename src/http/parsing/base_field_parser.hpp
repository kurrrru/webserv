// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>

#include "../request/http_fields.hpp"
#include "../string_utils.hpp"

namespace http {
class BaseFieldParser {
 public:
    BaseFieldParser() {}
    virtual ~BaseFieldParser() {}

    bool parseFieldLine(const HTTPFields::FieldPair& pair,
        HTTPFields::FieldMap& fields, HttpStatus& hs);
    static HTTPFields::FieldPair splitFieldLine(std::string* line);
    bool parseCgiFieldLine(const HTTPFields::FieldPair& pair,
                            HttpStatus& hs);

 protected:
    BaseFieldParser(const BaseFieldParser& other);
    BaseFieldParser& operator=(const BaseFieldParser& other);

    bool hostFieldLine(HTTPFields::FieldMap::iterator& target,
                    const HTTPFields::FieldPair& pair,
                    HttpStatus& hs);
    bool uniqueFieldLine(HTTPFields::FieldMap::iterator& target,
                    const HTTPFields::FieldPair& pair,
                    HttpStatus& hs);
    void normalFieldLine(HTTPFields::FieldMap::iterator& target,
                        const HTTPFields::FieldPair& pair);
    virtual bool isUnique(const std::string& key) = 0;
    bool validateHost(const HTTPFields::FieldValue& values);
    virtual void handleInvalidFieldError(const std::string& key,
                                        HttpStatus& hs) = 0;
    virtual void handleDuplicateFieldError(const std::string& key,
                                        HttpStatus& hs) = 0;
};

}  // namespace http
