// Copyright 2025 Ideal Broccoli

#include <vector>

#include "config_parser.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

// HTTPブロックを解析する
bool ConfigParser::parseHttpBlock(const std::vector<std::string>& tokens, size_t* pos) {
    toolbox::logger::StepMark::debug("Parsing HTTP block starting at token " + toolbox::to_string(static_cast<int>(*pos)) + ": " + tokens[*pos]);
    // HTTPブロックの開始を確認
    if (!validateBlockStart(tokens, pos, config::context::CONTEXT_HTTP)) {
        return false;
    }
    // HTTPブロック内のディレクティブを解析
    if (!parseHttpDirectives(tokens, pos, &_config)) {
        return false;
    }
    // HTTPブロックの終了を確認
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    // サーバーブロックの存在を確認
    if (!validateServerBlocks()) {
        return false;
    }
    toolbox::logger::StepMark::debug("Finished parsing HTTP block.");
    return true;
}

// サーバーブロックの存在を確認
bool ConfigParser::validateServerBlocks() {
    if (this->_config.servers.empty()) {
        toolbox::logger::StepMark::error("No servers defined in HTTP context.");
        return false;
    }
    return true;
}

// HTTPブロック内のディレクティブを解析
bool ConfigParser::parseHttpDirectives(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* config) {
    toolbox::logger::StepMark::debug("Parsing directives in HTTP context");
    // トークンが`}`まで処理
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        // ディレクティブがserverブロック
        if (directive_name == config::context::CONTEXT_SERVER) {
            // serverブロックの解析
            config::ServerConfig serverConfig;
            // serverブロックにtoken位置を戻す
            (*pos)--;
            if (!parseServerBlock(tokens, pos, &serverConfig)) {
                return false;
            }
            config->servers.push_back(serverConfig);
        // 対象のディレクティブ
        } else if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_HTTP)) {
            // TODO(yootsubo): 重複チェック
            // httpディレクティブの解析
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, NULL, config)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in HTTP context.");
                return false;
            }
        // 対象外のディレクティブ
        } else {
            toolbox::logger::StepMark::error("Unknown or disallowed directive '" + directive_name + "' in HTTP context.");
            return false;
        }
    }
    toolbox::logger::StepMark::debug("Finished parsing directives in HTTP context.");
    return true;
}

}  // namespace config
