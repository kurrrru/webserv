// Copyright 2025 Ideal Broccoli

#include "config_parser.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool ConfigParser::parseLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* config) {
    toolbox::logger::StepMark::debug("Parsing Location block starting at token " + toolbox::to_string(static_cast<int>(*pos)) + ": " + tokens[*pos]);
    // locationブロックの開始とパスを検証
    if (!validateAndParseLocationBlockStart(tokens, pos, config)) {
        return false;
    }
    // locationブロック内のディレクティブを解析
    if (!parseLocationDirectives(tokens, pos, config)) {
        return false;
    }
    // locationブロックの終了を確認
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    toolbox::logger::StepMark::debug("Finished parsing Location block.");
    return true;
}

// locationブロックの開始を確認し、パスを取得
bool ConfigParser::validateAndParseLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* config) {
    // location ディレクティブであることを確認
    if (*pos >= tokens.size() || tokens[*pos] != config::context::CONTEXT_LOCATION) {
        toolbox::logger::StepMark::error("Expected 'location' directive.");
        return false;
    }
    (*pos)++;
    // locationディレクティブの後には必ずパスが必要
    if (*pos >= tokens.size() || tokens[*pos] == config::token::OPEN_BRACE) {
        toolbox::logger::StepMark::error("Expected path after 'location' directive.");
        return false;
    }
    // パスを取得して設定
    config->path = tokens[*pos];
    toolbox::logger::StepMark::debug("Location path: " + config->path);
    (*pos)++;
    // { の確認
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        toolbox::logger::StepMark::error("Expected '{' after location path.");
        return false;
    }
    (*pos)++;
    return true;
}

// Locationブロック内のディレクティブを解析
bool ConfigParser::parseLocationDirectives(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* config) {
    toolbox::logger::StepMark::debug("Parsing directives in location context");
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        // ディレクティブがlocationブロック
        if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_LOCATION)) {
            // Locationディレクティブの解析
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, config, NULL)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in location context.");
                return false;
            }
        // 対象外のディレクティブ
        } else {
            toolbox::logger::StepMark::error("Unknown or disallowed directive '" + directive_name + "' in location context.");
            return false;
        }
    }
    toolbox::logger::StepMark::debug("Finished parsing directives in location context");
    return true;
}

}  // namespace config
