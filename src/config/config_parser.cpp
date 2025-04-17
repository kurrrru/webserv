// Copyright 2025 Ideal Broccoli

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include "config_parser.hpp"
#include "config.hpp"
#include "config_namespace.hpp"

#include "../toolbox/stepmark.hpp"
#include "../toolbox/string.hpp"

namespace config {

// コンストラクタ
ConfigParser::ConfigParser() : _directiveParser() {
}

// ファイルを解析してConfigオブジェクトを生成する
Config ConfigParser::parseFile(const std::string& filepath) {
    ConfigParser parser;
    // ファイルを読み込む
    if (!parser.readFile(filepath)) {
        throw ConfigException("Failed to read config file: " + filepath);
    }
    ConfigLexer lexer;
    // トークン化を行う
    parser._tokens = lexer.tokenize(parser._input);
    // トークン化結果の処理をヘルパーメソッドに委譲
    // TODO(yootsubo) :トークンが空、または空白/コメントのみの場合の処理
    // デフォルト設定ファイルを見るために一旦、警告出すだけ
    // トークンが空の場合はdefalut.confを読み込む
    if (parser._tokens.empty()) {
        toolbox::logger::StepMark::warning("No tokens found in config file.");
        if (!parser.loadDefaultConfig()) {
            throw ConfigException("Failed to load default config");
        }
    }
    // トークン化結果を解析する
    if (!parser.parse()) {
        throw ConfigException("Failed to parse configuration");
    }
    Config config;
    // 解析結果をConfigオブジェクトにセット
    config.setConfig(parser._config);
    config.setTokenCount(parser._tokens.size());
    return config;
}

// ファイルを読み込む
bool ConfigParser::readFile(const std::string& filepath) {
    // ファイルを開く
    std::ifstream file(filepath.c_str());
    if (!file) {
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    _input = buffer.str();
    file.close();
    return true;
}

// デフォルト設定ファイルを読み込む
bool ConfigParser::loadDefaultConfig() {
    toolbox::logger::StepMark::info("Using default configuration.");
    // デフォルト設定ファイルを読み込む
    if (!readFile(config::DEFAULT_FILE)) {
        toolbox::logger::StepMark::error("Failed to read default config file: " + std::string(config::DEFAULT_FILE));
        return false;
    }
    // 読み込んだ内容をトークン化
    ConfigLexer lexer;
    _tokens = lexer.tokenize(_input);
    // デフォルト設定ファイルのトークンが空の場合
    if (_tokens.empty()) {
        toolbox::logger::StepMark::error("No tokens found in default config file.");
        return false;
    }
    return true;
}

// 設定ファイルを解析する
bool ConfigParser::parse() {
    size_t pos = 0;
    if (_tokens.empty()) {
        return false;
    } else {
        // 空でない場合はhttpブロックから始まる必要がある
        if (pos >= _tokens.size() || _tokens[pos] != config::context::CONTEXT_HTTP) {
            toolbox::logger::StepMark::error("Configuration must start with 'http' block.");
            return false;
        }
        // httpブロックを解析
        if (!parseHttpBlock(_tokens, &pos)) {
            return false;
        }
        // httpブロック後に余分なトークンがないかチェック
        if (pos < _tokens.size()) {
            toolbox::logger::StepMark::error("Unexpected token '" + _tokens[pos] + "' after http block.");
            return false;
        }
    }
    return true;
}

// ブロックの開始を確認
bool ConfigParser::validateBlockStart(const std::vector<std::string>& tokens, size_t* pos, const std::string& expectedDirective) {
    // 対象のディレクティブか確認
    if (*pos >= tokens.size() || tokens[*pos] != expectedDirective) {
        toolbox::logger::StepMark::error("Expected '" + expectedDirective + "' directive.");
        return false;
    }
    (*pos)++;
    // ブロックの開始を確認
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        toolbox::logger::StepMark::error("Expected '{' after '" + expectedDirective + "' directive.");
        return false;
    }
    (*pos)++;
    return true;
}

// ブロックの終了を確認
bool ConfigParser::validateBlockEnd(const std::vector<std::string>& tokens, size_t* pos) {
    // ブロックの終了を確認
    if (*pos >= tokens.size() || tokens[*pos] != config::token::CLOSE_BRACE) {
        toolbox::logger::StepMark::error("Expected '}' to close block.");
        return false;
    }
    (*pos)++;
    return true;
}

}  // namespace config
