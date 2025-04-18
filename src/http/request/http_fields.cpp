// Copyright 2025 Ideal Broccoli

#include <string>
#include <utility>
#include <map>
#include <vector>

#include "http_fields.hpp"
#include "request_parser.hpp"

void HTTPFields::initFieldsMap() {
    for (std::size_t i = 0; i < http::fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(std::make_pair(http::fields::FIELDS[i],
                                        std::vector<std::string>()));
    }
}

void HTTPFields::addField(
    const FieldPair& pair) {
        for (FieldMap::iterator m_it =
            _fieldsMap.begin(); m_it != _fieldsMap.end(); ++m_it) {
        if (toolbox::isEqualIgnoreCase(m_it->first, pair.first)) {
            for (std::size_t i = 0; i < pair.second.size(); ++i) {
                m_it->second.push_back(pair.second[i]);
            }
            if (m_it->first == http::fields::HOST && m_it->second.size() != 1) {
                throw http::RequestParser::ParseException
                                ("Error: Host has too many value");
            } else if (m_it->first == http::fields::CONTENT_LENGTH) {
                if (!http::isDigitStr(m_it->second[0])) {
                    throw http::RequestParser::ParseException
                            ("Error: Content-Length has string");
                }
                // need change 1048576 to Config client_max_body_size
                if (std::strtol(m_it->second[0].c_str(), NULL, 10) > 1048576) {
                    throw http::RequestParser::ParseException
                            ("Error: content_length too large");
                }
                for (std::size_t i = 0; i < m_it->second.size(); ++i) {
                    if (m_it->second[0] != m_it->second[i]) {
                        throw http::RequestParser::ParseException
                                ("Error: Content-Length has different values");
                    }
                }
            }
            return;
        }
    }
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
