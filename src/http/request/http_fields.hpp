// Copyright 2025 Ideal Broccoli

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../../../toolbox/string.hpp"
#include "../http_namespace.hpp"

class HTTPFields {
 public:
    typedef std::pair<std::string, std::vector<std::string> > FieldPair;
    typedef std::map<std::string, std::vector<std::string> > FieldMap;
    HTTPFields() {}
    ~HTTPFields() {}
    void initFieldsMap();
    bool addField(const FieldPair& pair);
    bool validateField(const std::string& key,
            const std::vector<std::string>& values);
    bool validateAllFields();

    std::vector<std::string>& getFieldValue(const std::string& key);
    FieldMap& get();

 private:
    HTTPFields(const HTTPFields& other);
    HTTPFields& operator=(const HTTPFields& other);

    bool isValidFieldKey(const std::string& key);
    bool isValidFieldValue(const std::string& value);
    bool validateContentLength(const std::vector<std::string>& values);
    bool validateHost(const std::vector<std::string>& values);
    FieldMap _fieldsMap;
};
