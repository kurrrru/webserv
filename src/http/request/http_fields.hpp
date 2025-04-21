// Copyright 2025 Ideal Broccoli

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../../../toolbox/string.hpp"
#include "../case_insensitive_less.hpp"
#include "../http_namespace.hpp"

class HTTPFields {  // will change Request class
 public:
    typedef std::string FieldKey;
    typedef std::vector<std::string> FieldValue;
    typedef std::pair<FieldKey, FieldValue> FieldPair;
    typedef std::map<FieldKey, FieldValue, CaseInsensitiveLess> FieldMap;

    HTTPFields() {}
    ~HTTPFields() {}
    void initFieldsMap();
    bool parse_header_line(const FieldPair& pair);
    bool addField(const FieldPair& pair);
    bool validateField(const std::string& key,
            const FieldValue& values);
    bool validateAllFields();

    FieldValue& getFieldValue(const std::string& key);
    FieldMap& get();

 private:
    HTTPFields(const HTTPFields& other);
    HTTPFields& operator=(const HTTPFields& other);

    bool hostFieldLine(const FieldPair& pair);
    bool uniqueFieldLine(const FieldPair& pair);
    bool nomalFieldLine(const FieldPair& pair);

    bool isValidFieldKey(const std::string& key);
    bool isValidFieldValue(const std::string& value);
    bool validateContentLength(const std::vector<std::string>& values);
    bool validateHost(const std::vector<std::string>& values);
    FieldMap _fieldsMap;
};
