// Copyright 2025 Ideal Broccoli

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgi_pass) {
    if (!cgi_pass) {
        toolbox::logger::StepMark::error("Invalid cgi_pass pointer");
        return false;
    }
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing cgi_pass directive");
        return false;
    }
    // CGIパスを読み取る
    std::string path = tokens[(*pos)++];
    // パスの検証
    if (path.empty()) {
        toolbox::logger::StepMark::error("CGI path cannot be empty");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after cgi_pass directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    // 値を設定
    *cgi_pass = path;
    toolbox::logger::StepMark::debug("cgi_pass set to " + path);
    return true;
}

}  // namespace config
