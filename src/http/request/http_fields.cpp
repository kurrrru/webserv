#include "http_fields.hpp"

bool HTTPFields::isInitialized() { return _isInitialized; }

void HTTPFields::initFieldsMap() {
    if (_isInitialized) {
        return;
    }
    for (std::size_t i = 0; i < http::fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(std::make_pair(http::fields::FIELDS[i],
                                        std::vector<std::string>()));
    }
    _isInitialized = true;
}

bool HTTPFields::addField(
    const std::pair<std::string, std::vector<std::string>>& pair) {
    for (std::map<std::string, std::vector<std::string>>::iterator m_it =
        _fieldsMap.begin(); m_it != _fieldsMap.end(); ++m_it) {
        if (toolbox::isEqualIgnoreCase(m_it->first, pair.first)) {
            if (!m_it->second.empty()) {
                return false;
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
