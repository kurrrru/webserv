#include <string>
#include <utility>

#include "http_fields.hpp"

namespace http {
void HTTPFields::initFieldsMap() {
    for (std::size_t i = 0; i < fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(
            std::make_pair(fields::FIELDS[i], FieldValue()));
        }
}

std::size_t HTTPFields::countNonEmptyValues() {
    std::size_t count = 0;
    for (FieldMap::iterator it = _fieldsMap.begin();
         it != _fieldsMap.end(); ++it) {
        if (!it->second.empty()) {
            ++count;
        }
    }
    return count;
}


HTTPFields::FieldValue& HTTPFields::getFieldValue(const std::string& key) {
    static HTTPFields::FieldValue emptyVector;
    if (_fieldsMap.find(key) != _fieldsMap.end()) {
        return _fieldsMap[key];
    }
    return emptyVector;
}

}  // namespace http
