// Copyright 2025 Ideal Broccoli

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../../../toolbox/string.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../case_insensitive_less.hpp"
#include "../http_namespace.hpp"
#include "../http_status.hpp"

namespace http {
class HTTPFields {
 public:
    typedef std::string FieldKey;
    typedef std::vector<std::string> FieldValue;
    typedef std::pair<FieldKey, FieldValue> FieldPair;
    typedef std::map<FieldKey, FieldValue, CaseInsensitiveLess> FieldMap;

    HTTPFields() {}
    ~HTTPFields() {}
    void initFieldsMap();
    FieldValue& getFieldValue(const std::string& key);
    FieldMap& get() { return _fieldsMap; };

 private:
    HTTPFields(const HTTPFields& other);
    HTTPFields& operator=(const HTTPFields& other);

    FieldMap _fieldsMap;
};

}  // namespace http
