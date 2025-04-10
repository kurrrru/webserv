#include "http_fields.hpp"
#include "request_parser.hpp"
#include "../http_namespace.hpp"

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    for (std::size_t i = 0; i < str1.length(); ++i) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }
    return true;
}

bool HTTPFields::isInitialized() {
    return !_fieldsMap.empty();
}

void HTTPFields::initFieldsMap() {
    for (std::size_t i = 0; i < http::FIELD_SIZE; ++i) {
        _fieldsMap.insert(
            std::make_pair(http::FIELDS[i], std::vector<std::string>()));
    }
}

bool HTTPFields::add(std::pair<std::string, std::vector<std::string>>& pair) {
    for (std::map<std::string, std::vector<std::string>>::iterator m_it =
             _fieldsMap.begin();
         m_it != _fieldsMap.end(); ++m_it) {
        if (caseInsensitiveCompare(m_it->first, pair.first)) {
            for (std::vector<std::string>::iterator it = pair.second.begin(); it != pair.second.end(); ++it) {
                m_it->second.push_back(*it);
            }
            return true;
        }
    }
    return false;
}

std::vector<std::string> HTTPFields::get(const std::string& key) {
    return _fieldsMap[key];
}
