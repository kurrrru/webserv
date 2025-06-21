// Copyright 2025 Ideal Broccoli

#include <string>

#include "base_field_parser.hpp"

namespace http {
bool BaseFieldParser::parseFieldLine(const HTTPFields::FieldPair& pair,
    HTTPFields::FieldMap& fieldMap, HttpStatus& hs) {
    if (pair.first.empty() || utils::hasWhiteSpace(pair.first) ||
            utils::hasCtlChar(pair.first)) {
        handleInvalidFieldError(pair.first, hs);
        return false;
    }
    HTTPFields::FieldMap::iterator target = fieldMap.find(pair.first);
    if (target == fieldMap.end()) {
        return true;
    }
    if (target->first == fields::HOST) {
        return hostFieldLine(target, pair, hs);
    } else if (isUnique(pair.first)) {
        return uniqueFieldLine(target, pair, hs);
    } else {
        normalFieldLine(target, pair);
    }
    return true;
}

HTTPFields::FieldPair BaseFieldParser::splitFieldLine(std::string* line) {
    HTTPFields::FieldPair pair;
    std::size_t pos = line->find_first_of(symbols::COLON);
    if (pos != std::string::npos) {
        pair.first = toolbox::trim(line, symbols::COLON);
        while (!line->empty()) {
            std::string value = toolbox::trim(line, symbols::COMMASP);
            utils::trimSpace(&value);
            if (!value.empty()) {
                pair.second.push_back(value);
            }
        }
    }
    return pair;
}

bool BaseFieldParser::hostFieldLine(HTTPFields::FieldMap::iterator& target,
                                    const HTTPFields::FieldPair& pair,
                                    HttpStatus& hs) {
    if (!target->second.empty()) {
        handleInvalidFieldError(pair.first, hs);
        return false;
    }
    if (!validateHost(pair.second)) {
        handleInvalidFieldError(pair.first, hs);
        return false;
    }
    target->second = pair.second;
    return true;
}

bool BaseFieldParser::uniqueFieldLine(HTTPFields::FieldMap::iterator& target,
                                    const HTTPFields::FieldPair& pair,
                                    HttpStatus& hs) {
    if (!target->second.empty()) {
        handleDuplicateFieldError(pair.first, hs);
        return false;
    }
    target->second = pair.second;
    return true;
}

void BaseFieldParser::normalFieldLine(HTTPFields::FieldMap::iterator& target,
                                      const HTTPFields::FieldPair& pair) {
    for (std::size_t i = 0; i < pair.second.size(); ++i) {
        target->second.push_back(pair.second[i]);
    }
}

bool BaseFieldParser::validateHost(const HTTPFields::FieldValue& values) {
    if (values.empty()) {
        toolbox::logger::StepMark::info("FieldParser: host value not found");
        return false;
    }
    if (values.size() != 1) {
        toolbox::logger::StepMark::info("FieldParser: too many host");
        return false;
    }
    if (utils::hasWhiteSpace(values[0])) {
        toolbox::logger::StepMark::info("FieldParser: host has invalid char");
        return false;
    }
    return true;
}

}  // namespace http
