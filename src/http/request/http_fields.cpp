// Copyright 2025 Ideal Broccoli

#include <string>
#include <utility>
#include <map>
#include <vector>

#include "http_fields.hpp"
#include "request_parser.hpp"

bool hasWhiteSpace(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (std::isspace(str[i])) {
            return true;
        }
    }
    return false;
}

bool isDigitStr(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool isAlnumStr(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (!std::isalnum(str[i])) {
            return false;
        }
    }
    return true;
}

namespace http {

void HTTPFields::initFieldsMap() {
    for (std::size_t i = 0; i < http::fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(
            std::make_pair(http::fields::FIELDS[i], FieldValue()));
        }
}

bool HTTPFields::validateRequestHeaders(HttpStatus& hs) {
    if (_fieldsMap.find(http::fields::HOST)->second.empty()) {
        toolbox::logger::StepMark::info("Field parse[400]: "
                                        "host does not exist");
        hs = BAD_REQUEST;
        return false;
    }
    FieldMap::iterator content_length =
        _fieldsMap.find(http::fields::CONTENT_LENGTH);
    FieldMap::iterator transfer_encoding =
        _fieldsMap.find(http::fields::TRANSFER_ENCODING);
    if (!content_length->second.empty() && !transfer_encoding->second.empty()) {
        toolbox::logger::StepMark::info("Field parse[400]: content-length and"
            " transfer-encoding must not coexist");
        hs = BAD_REQUEST;
        return false;
    } else if (!content_length->second.empty()) {
        if (!isDigitStr(content_length->second[0])) {
            toolbox::logger::StepMark::info("Field parse[400]: "
                "content-length is not number");
            hs = BAD_REQUEST;
            return false;
        }
        for (std::size_t i = 1; i < content_length->second.size(); ++i) {
            if (content_length->second[0] != content_length->second[i]) {
                toolbox::logger::StepMark::info("Field parse[400]: "
                    "content_length has multiple number");
                hs = BAD_REQUEST;
                return false;
            }
        }
        int64_t size = std::strtol(content_length->second[0].c_str(), NULL, 10);
        if (size > http::fields::MAX_BODY_SIZE) {
            toolbox::logger::StepMark::info("Field parse[413]: "
                "content-length is too large");
            hs = PAYLOAD_TOO_LARGE;
            return false;
        }
    } else if (!transfer_encoding->second.empty()) {
        for (std::size_t i = 0; i < transfer_encoding->second.size(); ++i) {
            if ("chunked" != transfer_encoding->second[i]) {
                toolbox::logger::StepMark::info("Field parse[501]: "
                    "transfer-encoding has invalid value");
                hs = NOT_IMPLEMENTED;
                return false;
            }
        }
    }
    return true;
}

bool HTTPFields::parseHeaderLine(const FieldPair& pair, HttpStatus& hs) {
    if (pair.first.empty() || hasWhiteSpace(pair.first) ||
            http::hasCtlChar(pair.first)) {
        toolbox::logger::StepMark::info("Field parse[400]: "
            "invalid header field key");
            hs = BAD_REQUEST;
        return false;
    }
    FieldMap::iterator target = _fieldsMap.find(pair.first);
    if (target == _fieldsMap.end()) {
        toolbox::logger::StepMark::info("Field parse[200]: "
            "does not exist field key");
        return true;
    }
    if (target->first == http::fields::HOST) {
        return hostFieldLine(target, pair, hs);
    } else if (target->first == http::fields::CONTENT_LENGTH ||
        target->first == http::fields::TRANSFER_ENCODING) {
        return uniqueFieldLine(target, pair, hs);
    } else {
        nomalFieldLine(target, pair);
    }
    return true;
}

bool HTTPFields::hostFieldLine(FieldMap::iterator& target,
        const FieldPair& pair, HttpStatus& hs) {
    if (!target->second.empty()) {
        toolbox::logger::StepMark::info("Field parse[400]: "
                                        "duplicate host field");
        hs = BAD_REQUEST;
        return false;
    }
    if (!validateHost(pair.second)) {
        hs = BAD_REQUEST;
        return false;
    }
    target->second = pair.second;
    return true;
}

bool HTTPFields::validateHost(const FieldValue& values) {
    if (values.empty()) {
        toolbox::logger::StepMark::info("Field parse[400]: "
                                        "host value not found");
        return false;
    }
    if (values.size() != 1) {
        toolbox::logger::StepMark::info("Field parse[400]: too many Host");
        return false;
    }
    if (hasWhiteSpace(values[0])) {
        toolbox::logger::StepMark::info("Field parse[400]: "
                                        "host has invalid char");
        return false;
    }
    return true;
}

bool HTTPFields::uniqueFieldLine(FieldMap::iterator& target,
            const FieldPair& pair, HttpStatus& hs) {
    if (!target->second.empty()) {
        toolbox::logger::StepMark::info("Field parse[400]: " + pair.first
                                        + " is already set");
        hs = BAD_REQUEST;
        return false;
    }
    target->second = pair.second;
    return true;
}

void HTTPFields::nomalFieldLine(FieldMap::iterator& target,
                                    const FieldPair& pair) {
    if (target == _fieldsMap.end()) {
        toolbox::logger::StepMark::info("Field parse[200]: " + pair.first
                    + " not found field");
        return;
    }
    for (std::size_t i = 0; i < pair.second.size(); ++i) {
        target->second.push_back(pair.second[i]);
    }
    return;
}

std::vector<std::string>& HTTPFields::getFieldValue(const std::string& key) {
    static std::vector<std::string> emptyVector;
    if (_fieldsMap.find(key) != _fieldsMap.end()) {
        return _fieldsMap[key];
    }
    return emptyVector;
}

HTTPFields::FieldMap& HTTPFields::get() {
    return _fieldsMap;
}

}  // namespace http
