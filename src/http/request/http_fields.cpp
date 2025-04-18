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

void HTTPFields::initFieldsMap() {
    for (std::size_t i = 0; i < http::fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(std::make_pair(http::fields::FIELDS[i],
                                        std::vector<std::string>()));
    }
}

bool HTTPFields::addField(const FieldPair& pair) {
    if (pair.first.empty() || hasWhiteSpace(pair.first)) {
        // response status 400
        return false;
    }
    for (FieldMap::iterator m_it =
            _fieldsMap.begin(); m_it != _fieldsMap.end(); ++m_it) {
        if (toolbox::isEqualIgnoreCase(m_it->first, pair.first)) {
            if ((m_it->first == http::fields::CONTENT_LENGTH ||
                    m_it->first == http::fields::HOST) &&
                    !m_it->second.empty()) {
                // response status 400
                return false;
            }
            if (pair.first == http::fields::HOST &&
                    (pair.second.size() != 1 ||
                        hasWhiteSpace(pair.second[0]))) {
                // response status 400
                return false;
            }
            for (std::size_t i = 0; i < pair.second.size(); ++i) {
                m_it->second.push_back(pair.second[i]);
            }
            if (m_it->first == http::fields::CONTENT_LENGTH) {
                for (std::size_t i = 0; i < m_it->second.size(); ++i) {
                    if (m_it->second[0] != m_it->second[i]) {
                        // response status 400
                        return false;
                    }
                }
            }
            break;
        }
    }
    return true;
}

bool HTTPFields::validateAllFields() {
    if (getFieldValue(http::fields::HOST).empty()) {
        return false;
    }
    for (FieldMap::iterator it = _fieldsMap.begin();
            it != _fieldsMap.end(); ++it) {
        if (!it->second.empty() && !validateField(it->first, it->second)) {
            return false;
        }
    }
    return true;
}


bool HTTPFields::validateField(const std::string& key,
                const std::vector<std::string>& values) {
    if (!isValidFieldKey(key)) {
        // response status 400
        return false;
    }
    if (toolbox::isEqualIgnoreCase(key, http::fields::CONTENT_LENGTH)) {
        // response status 400, 413
        return validateContentLength(values);
    } else if (toolbox::isEqualIgnoreCase(key, http::fields::HOST)) {
        // response status 400
        return validateHost(values);
    }
    return true;
}

bool HTTPFields::validateHost(const std::vector<std::string>& values) {
    if (values.empty() || http::hasWhiteSpace(values[0])) {
        return false;
    }
    return true;
}

bool HTTPFields::validateContentLength(const std::vector<std::string>& values) {
    if (values.empty()) {
        return true;
    }
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (values[0] != values[i]) {
            // response status 400
            return false;
        }
    }
    if (!http::isDigitStr(values[0])) {
        // response status 400
        return false;
    }
    int64_t size = std::strtol(values[0].c_str(), NULL, 10);
    if (size > http::fields::MAX_BODY_SIZE) {
        // response status 413
        return false;
    }
    return true;
}

bool HTTPFields::isValidFieldKey(const std::string& key) {
    if (key.empty()) {
        return false;
    }
    if (hasWhiteSpace(key)) {
        return false;
    }
    return true;
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
