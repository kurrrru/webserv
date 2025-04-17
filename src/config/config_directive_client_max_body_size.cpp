// Copyright 2025 Ideal Broccoli

#include <string>
#include <cctype>
#include <limits>
#include <cstdlib>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t* pos, size_t* client_max_body_size) {
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing client_max_body_size");
        return false;
    }
    std::string size_str = tokens[(*pos)++];
    // 数値チェック
    for (size_t i = 0; i < size_str.size(); ++i) {
        if (!std::isdigit(size_str[i])) {
            toolbox::logger::StepMark::error("Invalid client_max_body_size value: " + size_str + ". Must be a number.");
            return false;
        }
    }
    // 値の変換と検証
    unsigned long size = 0;
    try {
        size = std::strtoul(size_str.c_str(), NULL, 10);
        if (size == 0) {
            toolbox::logger::StepMark::error("client_max_body_size must be greater than 0");
            return false;
        }
        if (size > std::numeric_limits<size_t>::max()) {
            toolbox::logger::StepMark::error("client_max_body_size value too large");
            return false;
        }
    } catch (...) {
        toolbox::logger::StepMark::error("Failed to parse client_max_body_size value: " + size_str);
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after client_max_body_size value");
        return false;
    }
    (*pos)++; // セミコロンを飛ばす
    // 値を設定
    *client_max_body_size = static_cast<size_t>(size);
    toolbox::logger::StepMark::debug("client_max_body_size set to " + size_str);
    return true;
}

}  // namespace config
