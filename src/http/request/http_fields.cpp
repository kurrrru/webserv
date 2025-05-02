// Copyright 2025 Ideal Broccoli

#include <string>
#include <utility>
#include <map>
#include <vector>

#include "http_fields.hpp"
#include "request_parser.hpp"

namespace http {

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

void HTTPFields::initFieldsMap() {
    for (std::size_t i = 0; i < fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(
            std::make_pair(fields::FIELDS[i], FieldValue()));
        }
}

bool HTTPFields::parseHeaderLine(const FieldPair& pair, HttpStatus& hs) {
    if (pair.first.empty() || hasWhiteSpace(pair.first) ||
            hasCtlChar(pair.first)) {
        logInfo(BAD_REQUEST, "invalid field key");
        hs = BAD_REQUEST;
        return false;
    }
    FieldMap::iterator target = _fieldsMap.find(pair.first);
    if (target == _fieldsMap.end()) {
        logInfo(OK, "does not exist field key");
        return true;
    }
    if (target->first == fields::HOST) {
        return hostFieldLine(target, pair, hs);
    } else if (target->first == fields::CONTENT_LENGTH ||
        target->first == fields::TRANSFER_ENCODING) {
        return uniqueFieldLine(target, pair, hs);
    } else {
        normalFieldLine(target, pair);
    }
    return true;
}


bool HTTPFields::hostFieldLine(FieldMap::iterator& target,
    const FieldPair& pair, HttpStatus& hs) {
    if (!target->second.empty()) {
        logInfo(BAD_REQUEST, "duplicate host field");
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
        logInfo(BAD_REQUEST, "host value not found");
        return false;
    }
    if (values.size() != 1) {
        logInfo(BAD_REQUEST, "too many host");
        return false;
    }
    if (hasWhiteSpace(values[0])) {
        logInfo(BAD_REQUEST, "host has invalid char");
        return false;
    }
    return true;
}

bool HTTPFields::uniqueFieldLine(FieldMap::iterator& target,
    const FieldPair& pair, HttpStatus& hs) {
    if (!target->second.empty()) {
        logInfo(BAD_REQUEST, pair.first + " is already set");
        hs = BAD_REQUEST;
        return false;
    }
    target->second = pair.second;
    return true;
}


void HTTPFields::normalFieldLine(FieldMap::iterator& target,
    const FieldPair& pair) {
    if (target == _fieldsMap.end()) {
        logInfo(OK, pair.first + " not found field");
        return;
    }
    for (std::size_t i = 0; i < pair.second.size(); ++i) {
        target->second.push_back(pair.second[i]);
    }
    return;
}

bool HTTPFields::validateRequestHeaders(HttpStatus& hs) {
    if (!validateHostExists(hs)) {
        return false;
    }
    if (!validateContentHeaders(hs)) {
        return false;
    }
    return true;
}

bool HTTPFields::validateHostExists(HttpStatus& hs) {
    if (_fieldsMap.find(fields::HOST)->second.empty()) {
        logInfo(BAD_REQUEST, "host does not exist");
        hs = BAD_REQUEST;
        return false;
    }
    return true;
}

bool HTTPFields::validateContentHeaders(HttpStatus& hs) {
    FieldMap::iterator content_length =
                        _fieldsMap.find(fields::CONTENT_LENGTH);
    FieldMap::iterator transfer_encoding =
                        _fieldsMap.find(fields::TRANSFER_ENCODING);
    if (!content_length->second.empty()) {
        if (!transfer_encoding->second.empty()) {
            logInfo(BAD_REQUEST,
                "content-length and transfer-encoding must not coexist");
            hs = BAD_REQUEST;
            return false;
        }
        return validateContentLength(content_length, hs);
    } else if (!transfer_encoding->second.empty()) {
        return validateTransferEncoding(transfer_encoding, hs);
    }
    return true;
}

bool HTTPFields::validateContentLength(
        FieldMap::iterator& content_length, HttpStatus& hs) {
    if (!isDigitStr(content_length->second[0])) {
        logInfo(BAD_REQUEST, "content-length is not number");
        hs = BAD_REQUEST;
        return false;
    }
    for (std::size_t i = 1; i < content_length->second.size(); ++i) {
        if (content_length->second[0] != content_length->second[i]) {
            logInfo(BAD_REQUEST, "content-length has multiple number");
            hs = BAD_REQUEST;
            return false;
        }
    }
    std::size_t size = std::strtol(content_length->second[0].c_str(), NULL, 10);
    if (size > fields::MAX_BODY_SIZE) {
        logInfo(PAYLOAD_TOO_LARGE, "content-length is too large");
        hs = PAYLOAD_TOO_LARGE;
        return false;
    }
    return true;
}

bool HTTPFields::validateTransferEncoding \
        (FieldMap::iterator& transfer_encoding, HttpStatus& hs) {
    for (std::size_t i = 0; i < transfer_encoding->second.size(); ++i) {
        if ("chunked" != transfer_encoding->second[i]) {
            logInfo(NOT_IMPLEMENTED, "transfer-encoding has invalid value");
            hs = NOT_IMPLEMENTED;
            return false;
        }
    }
    return true;
}

HTTPFields::FieldValue& HTTPFields::getFieldValue(const std::string& key) {
    static HTTPFields::FieldValue emptyVector;
    if (_fieldsMap.find(key) != _fieldsMap.end()) {
        return _fieldsMap[key];
    }
    return emptyVector;
}

HTTPFields::FieldMap& HTTPFields::get() {
    return _fieldsMap;
}

}  // namespace http
