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
    bool parseHeaderLine(const FieldPair& pair, HttpStatus& hs);
    bool validateRequestHeaders(HttpStatus& hs);

    FieldValue& getFieldValue(const std::string& key);
    FieldMap& get();

 private:
    HTTPFields(const HTTPFields& other);
    HTTPFields& operator=(const HTTPFields& other);

    bool hostFieldLine(FieldMap::iterator& target,
                        const FieldPair& pair, HttpStatus& hs);
    bool uniqueFieldLine(FieldMap::iterator& target,
                        const FieldPair& pair, HttpStatus& hs);
    void normalFieldLine(FieldMap::iterator& target, const FieldPair& pair);
    bool validateHost(const FieldValue& values);
    bool validateHostExists(HttpStatus& hs);
    bool validateContentHeaders(HttpStatus& hs);
    bool validateContentLength
                        (FieldMap::iterator& content_length, HttpStatus& hs);
    bool validateTransferEncoding
                        (FieldMap::iterator& transfer_encoding, HttpStatus& hs);

    FieldMap _fieldsMap;
};

}  // namespace http
