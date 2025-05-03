#include "base_field_parser.hpp"
#include "string_utils.hpp"

namespace http {
bool BaseFieldParser::parseFieldLine(const HTTPFields::FieldPair& pair,
    HttpStatus& hs) {
    if (pair.first.empty() || utils::hasWhiteSpace(pair.first) ||
            utils::hasCtlChar(pair.first)) {
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
}
