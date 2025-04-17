// Copyright 2025 Ideal Broccoli

#include "config_parser.hpp"
#include "config_namespace.hpp"

#include "../toolbox/stepmark.hpp"
#include "../toolbox/string.hpp"

namespace config {

// サーバーブロックを解析する
bool ConfigParser::parseServerBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* config) {
    toolbox::logger::StepMark::debug("Parsing Server block starting at token " + toolbox::to_string(static_cast<int>(*pos)) + ": " + tokens[*pos]);
    // サーバーブロックの開始を確認
    if (!validateBlockStart(tokens, pos, config::context::CONTEXT_SERVER)) {
        return false;
    }
    // サーバーブロック内のディレクティブを解析
    if (!parseServerDirectives(tokens, pos, config)) {
        return false;
    }
    // サーバーブロックの終了を確認
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    toolbox::logger::StepMark::debug("Finished parsing Server block.");
    return true;
}

// Serverブロック内のディレクティブを解析
bool ConfigParser::parseServerDirectives(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* config) {
    toolbox::logger::StepMark::debug("Parsing directives in server context");
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        // ディレクティブがlocationブロック
        if (directive_name == config::context::CONTEXT_LOCATION) {
            config::LocationConfig location_config;
            // locationブロックにtoken位置を戻す
            (*pos)--;
            if (!parseLocationBlock(tokens, pos, &location_config)) {
                return false;
            }
            config->locations.push_back(location_config); 
        // 対象のディレクティブ
        } else if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_SERVER)) {
            // TODO(yootsubo) : 重複チェック
            // Serverディレクティブの解析
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, config, NULL, NULL)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in server context.");
                return false;
            }
        // 対象外のディレクティブ
        } else {
            toolbox::logger::StepMark::error("Unknown or disallowed directive '" + directive_name + "' in server context.");
            return false;
        }
    }
    toolbox::logger::StepMark::debug("Finished parsing directives in server context");
    return true;
}

}  // namespace config
