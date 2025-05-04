#pragma once

#include "../base_field_parser.hpp"

namespace http {
class RequestFieldParser : public BaseFieldParser {
 public:
    
 private:
    bool isUnique(const std::string& key);
    void handleInvalidFieldError(const std::string& key, HttpStatus& hs);
    void handleDuplicateFieldError(const std::string& key, HttpStatus& hs);
};

}