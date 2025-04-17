// Copyright 2025 Ideal Broccoli

#include <algorithm>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../toolbox/stepmark.hpp"
#include "../toolbox/string.hpp"

namespace config {

bool DirectiveParser::parseAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* methods) {
    if (!methods) {
        toolbox::logger::StepMark::error("Invalid method vector pointer");
        return false;
    }
    methods->clear();
    // メソッドリストを読み取る（セミコロンまで）
    while (*pos < tokens.size() && tokens[*pos] != ";") {
        std::string method = tokens[*pos];
        // 重複チェック
        if (std::find(methods->begin(), methods->end(), method) != methods->end()) {
            toolbox::logger::StepMark::warning("Duplicate method: " + method + " - ignoring");
        } else {
            methods->push_back(method);
        }
        (*pos)++;
    }
    // 少なくとも1つのメソッドが必要
    if (methods->empty()) {
        toolbox::logger::StepMark::error("At least one HTTP method must be specified");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after allowed_methods directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    return true;
}

}  // namespace config
