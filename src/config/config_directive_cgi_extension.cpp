// Copyright 2025 Ideal Broccoli

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgi_extension) {
    if (!cgi_extension) {
        toolbox::logger::StepMark::error("Invalid cgi_extension pointer");
        return false;
    }
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing cgi_extension directive");
        return false;
    }
    // 拡張子を読み取る
    std::string extension = tokens[(*pos)++];
    // 拡張子の検証（ピリオドで始まることを推奨）
    if (extension.empty()) {
        toolbox::logger::StepMark::error("CGI extension cannot be empty");
        return false;
    }
    if (extension[0] != '.') {
        toolbox::logger::StepMark::warning("CGI extension should start with a dot (e.g. \".php\")");
        // エラーにはせずに警告だけ出す
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after cgi_extension directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    // 値を設定
    *cgi_extension = extension;
    return true;
}

}  // namespace config
