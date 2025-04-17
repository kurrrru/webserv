// Copyright 2025 Ideal Broccoli

#include <cstdlib>
#include <string>
#include <limits>
#include <cctype>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseListenDirective(const std::vector<std::string>& tokens, size_t* pos, int* port) {
    if (!port) {
        toolbox::logger::StepMark::error("Invalid port pointer");
        return false;
    }
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing listen directive");
        return false;
    }
    // ポート番号を読み取る
    std::string port_str = tokens[(*pos)++];
    // 数値であることを確認
    for (size_t i = 0; i < port_str.size(); ++i) {
        if (!std::isdigit(port_str[i])) {
            toolbox::logger::StepMark::error("Port must be a number: " + port_str);
            return false;
        }
    }
    // 値を変換して検証
    int port_value = std::atoi(port_str.c_str());
    if (port_value <= 0 || port_value > 65535) {
        toolbox::logger::StepMark::error("Invalid port number: " + port_str + ". Must be between 1 and 65535");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after listen directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    // ポート番号を設定
    *port = port_value;
    toolbox::logger::StepMark::debug("Server listening port set to " + port_str);
    return true;
}

}  // namespace config
