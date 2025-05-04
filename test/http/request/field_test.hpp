// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>

#include "../../../src/http/request/http_fields.hpp"
#include "../../../src/http/http_namespace.hpp"
#include "../../../src/http/http_status.hpp"
#include "base_test.hpp"

class FieldTest : public BaseTest {
 public:
    FieldTest() : BaseTest() {}
    ~FieldTest() {}
    FieldTest(const FieldTest& other) :
        BaseTest(other),
        _exceptMap(other._exceptMap)
    {}
    http::HTTPFields::FieldMap _exceptMap;
};
