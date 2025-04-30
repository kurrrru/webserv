// Copyright 2025 Ideal Broccoli

#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_server.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

// サーバーブロックを解析する
bool ConfigParser::parseServerBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config) {
    // サーバーブロックの開始を確認
    if (!validateBlockStart(tokens, pos, config::context::SERVER)) {
        return false;
    }
    // サーバーブロック内のディレクティブを解析
    if (!parseServerDirectives(tokens, pos, server_config)) {
        return false;
    }
    // サーバーブロックの終了を確認
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    // サーバーの親を設定
    server_config->setParent(_config.get());
    return true;
}

// Serverブロック内のディレクティブを解析
bool ConfigParser::parseServerDirectives(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config) {
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        // ディレクティブがlocationブロック
        if (directive_name == config::context::LOCATION) {
            toolbox::SharedPtr<config::LocationConfig> location_config(new config::LocationConfig());
            // locationブロックにtoken位置を戻す
            (*pos)--;
            if (!parseLocationBlock(tokens, pos, server_config, location_config.get())) {
                return false;
            }
            server_config->locations.push_back(location_config);
        // 対象のディレクティブ
        } else if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_SERVER)) {
            // TODO(yootsubo) : 重複チェック
            // Serverディレクティブの解析
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, server_config, NULL)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in server context.");
                return false;
            }
        // 対象外のディレクティブ
        } else {
            toolbox::logger::StepMark::error("Unknown or disallowed directive '" + directive_name + "' in server context.");
            return false;
        }
    }
    return true;
}

}  // namespace config
