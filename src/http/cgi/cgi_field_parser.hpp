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
    CgiFieldParser(const CgiFieldParser& other);
    CgiFieldParser& operator=(const CgiFieldParser& other);

    bool isUnique(const std::string& key);
    void handleInvalidFieldError(const std::string& key, HttpStatus& hs);
    void handleDuplicateFieldError(const std::string& key, HttpStatus& hs);
};

}  // namespace http
