// Copyright 2025 Ideal Broccoli

#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_location.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool ConfigParser::parseLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config, config::LocationConfig* location_config) {
    // locationブロックの開始とパスを検証
    if (!validateAndParseLocationBlockStart(tokens, pos, location_config)) {
        return false;
    }
    // locationブロック内のディレクティブを解析
    if (!parseLocationDirectives(tokens, pos, location_config)) {
        return false;
    }
    // locationブロックの終了を確認
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    // ロケーションの親を設定
    location_config->setParent(server_config);
    return true;
}

// 入れ子のロケーションブロック専用のパース関数
bool ConfigParser::parseNestedLocationBlock(const std::vector<std::string>& tokens,  size_t* pos,  config::LocationConfig* parent_location, config::LocationConfig* location_config) {
    // locationブロックの開始とパスを検証
    if (!validateAndParseLocationBlockStart(tokens, pos, location_config)) {
        return false;
    }
    // locationブロック内のディレクティブを解析
    if (!parseLocationDirectives(tokens, pos, location_config)) {
        return false;
    }
    // ブロック終了を確認
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    // 親ロケーションを設定
    location_config->setParent(parent_location);
    return true;
}

// locationブロックの開始を確認し、パスを取得
bool ConfigParser::validateAndParseLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config) {
    // location ディレクティブであることを確認
    if (*pos >= tokens.size() || tokens[*pos] != config::context::LOCATION) {
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
    location_config->path = tokens[*pos];
    toolbox::logger::StepMark::debug("Location path: " + location_config->path);
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
bool ConfigParser::parseLocationDirectives(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config) {
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        // 入れ子のディレクティブの処理
        if (directive_name == config::context::LOCATION) {
            // 入れ子のロケーションブロックを処理
            if (!handleNestedLocationBlock(tokens, pos, location_config)) {
                return false;
            }
            continue;
        }
        (*pos)++;
        // ディレクティブがlocationブロック
        if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_LOCATION)) {
            // Locationディレクティブの解析
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, NULL, location_config)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in location context.");
                return false;
            }
        // 対象外のディレクティブ
        } else {
            toolbox::logger::StepMark::error("Unknown or disallowed directive '" + directive_name + "' in location context.");
            return false;
        }
    }
    return true;
}

// 入れ子のロケーションブロックを処理する関数
bool ConfigParser::handleNestedLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* parent_location) {
    // 新しいLocationConfigオブジェクトを作成
    toolbox::SharedPtr<config::LocationConfig> nested_location(new config::LocationConfig());
    // 専用の関数を使用
    if (!parseNestedLocationBlock(tokens, pos, parent_location, nested_location.get())) {
        return false;
    }
    // 親ロケーションへの参照を設定して追加
    parent_location->addLocation(*nested_location.get());
    toolbox::logger::StepMark::debug("Added nested location: " + nested_location->path + " to parent: " + parent_location->path);
    return true;
}

}  // namespace config
