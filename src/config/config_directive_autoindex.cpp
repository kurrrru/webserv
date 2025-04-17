// Copyright 2025 Ideal Broccoli

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool DirectiveParser::parseAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, bool* autoindex) {
    if (!autoindex) {
        toolbox::logger::StepMark::error("Invalid autoindex pointer");
        return false;
    }
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing autoindex directive");
        return false;
    }
    // 値を読み取る
    std::string value = tokens[(*pos)++];
    // 値の検証
    if (value == "on") {
        *autoindex = true;
    } else if (value == "off") {
        *autoindex = false;
    } else {
        toolbox::logger::StepMark::error("Invalid autoindex value: " + value + ". Expected 'on' or 'off'");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after autoindex directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    return true;
}

}  // namespace config
