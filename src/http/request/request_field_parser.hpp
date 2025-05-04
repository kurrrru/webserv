#pragma once

#include <string>

#include "../base_field_parser.hpp"

namespace http {
class RequestFieldParser : public BaseFieldParser {
 public:
    RequestFieldParser() {}
    ~RequestFieldParser() {}
 private:
    RequestFieldParser(const RequestFieldParser& other);
    RequestFieldParser& operator=(const RequestFieldParser& other);

    bool isUnique(const std::string& key);
    void handleInvalidFieldError(const std::string& key, HttpStatus& hs);
    void handleDuplicateFieldError(const std::string& key, HttpStatus& hs);
};

}  // namespace http
