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

HTTPFields::FieldValue& HTTPFields::getFieldValue(const std::string& key) {
    static HTTPFields::FieldValue emptyVector;
    if (_fieldsMap.find(key) != _fieldsMap.end()) {
        return _fieldsMap[key];
    }
    return emptyVector;
}

}  // namespace http
