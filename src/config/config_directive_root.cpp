// Copyright 2025 Ideal Broccoli

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseRootDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* root) {
    if (!root) {
        toolbox::logger::StepMark::error("Invalid root pointer");
        return false;
    }
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing root directive");
        return false;
    }
    // ルートパスを読み取る
    std::string path = tokens[(*pos)++];
    // パスの検証
    if (path.empty()) {
        toolbox::logger::StepMark::error("Root path cannot be empty");
        return false;
    }
    // 不正な文字チェック
    if (path.find('\0') != std::string::npos) {
        toolbox::logger::StepMark::error("Root path contains null character");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after root directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    // パスを設定
    *root = path;
    toolbox::logger::StepMark::debug("Root path set to " + path);
    
    return true;
}

}  // namespace config
