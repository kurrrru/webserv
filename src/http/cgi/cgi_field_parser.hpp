#pragma once

#include <string>

#include "../parsing/base_field_parser.hpp"
#include "../http_namespace.hpp"

namespace http {
class CgiFieldParser : public BaseFieldParser {
 public:
    CgiFieldParser() {}
    ~CgiFieldParser() {}

 private:
    bool isUnique(const std::string& key);
    void handleInvalidFieldError(const std::string& key, HttpStatus& hs);
    void handleDuplicateFieldError(const std::string& key, HttpStatus& hs);
};

}  // namespace http
