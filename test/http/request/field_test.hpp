// Copyright 2025 Ideal Broccoli

#include <string>

#include "../../../src/http/request/request_parser.hpp"
#include "../../../src/http/request/http_fields.hpp"
#include "../../../src/http/http_namespace.hpp"

class FieldTest {
 public:
    std::string _name;
    std::string _request;
    bool _isSuccessTest;

    HTTPFields::FieldMap _exceptMap;
};
