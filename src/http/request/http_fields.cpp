#include "http_fields.hpp"

#include "../http_namespace.hpp"
#include "request_parser.hpp"

static bool caseInsensitiveCompare(const std::string& str1,
                                   const std::string& str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    for (std::size_t i = 0; i < str1.length(); ++i) {
        if (std::tolower(static_cast<unsigned char>(str1[i])) !=
            std::tolower(static_cast<unsigned char>(str2[i]))) {
            return false;
        }
    }
    return true;
}

bool HTTPFields::isInitialized() { return _isInitialized; }

void HTTPFields::initFieldsMap() {
    if (_isInitialized) {
        return;
    }
    for (std::size_t i = 0; i < http::fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(
            std::make_pair(http::fields::FIELDS[i], std::vector<std::string>()));
    }
    _isInitialized = true;
}

bool HTTPFields::addField(
    std::pair<std::string, std::vector<std::string>>& pair) {
    for (std::map<std::string, std::vector<std::string>>::iterator m_it =
             _fieldsMap.begin();
         m_it != _fieldsMap.end(); ++m_it) {
        if (caseInsensitiveCompare(m_it->first, pair.first)) {
            if (!m_it->second.empty()) {
                return false;
            }
            for (std::vector<std::string>::iterator it = pair.second.begin();
                 it != pair.second.end(); ++it) {
                m_it->second.push_back(*it);
            }
            for (std::size_t i = 0; i < pair.second.size(); ++i) {
                m_it->second.push_back(pair.second[i]);
            }
            return true;
        }
    }
    return false;
}

std::vector<std::string>& HTTPFields::getFieldValue(const std::string& key) {
    return _fieldsMap[key];
}

std::map<std::string, std::vector<std::string>>& HTTPFields::get() {
    return _fieldsMap;
}
