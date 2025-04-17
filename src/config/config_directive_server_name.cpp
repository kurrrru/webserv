// Copyright 2025 Ideal Broccoli

#include <algorithm>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* server_name) {
    if (!server_name) {
        toolbox::logger::StepMark::error("Invalid server_name pointer");
        return false;
    }
    
    server_name->clear();
    
    // サーバー名のリストを読み取る（セミコロンまで）
    while (*pos < tokens.size() && tokens[*pos] != ";") {
        std::string name = tokens[*pos];
        
        // サーバー名の検証
        if (name.empty()) {
            toolbox::logger::StepMark::error("Server name cannot be empty");
            return false;
        }
        
        // 無効な文字チェック
        if (name.find('\0') != std::string::npos) {
            toolbox::logger::StepMark::error("Server name contains null character");
            return false;
        }
        
        // 重複チェック
        if (std::find(server_name->begin(), server_name->end(), name) != server_name->end()) {
            toolbox::logger::StepMark::warning("Duplicate server name: " + name + " - ignoring");
        } else {
            server_name->push_back(name);
        }
        
        (*pos)++;
    }
    
    // 少なくとも1つのサーバー名が必要
    if (server_name->empty()) {
        toolbox::logger::StepMark::error("At least one server name must be specified");
        return false;
    }
    
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after server_name directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    
    return true;
}

}  // namespace config
