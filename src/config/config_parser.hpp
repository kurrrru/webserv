// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>

#include "config.hpp"
#include "config_lexer.hpp"
#include "config_directive_handler.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

class ConfigParser {
 public:
    static toolbox::SharedPtr<Config> parseFile(const std::string& filepath);

 private:
    ConfigParser();
    ~ConfigParser();
    ConfigParser(const ConfigParser&);
    ConfigParser& operator=(const ConfigParser&);
    bool readFile(const std::string& filepath);
    bool parse();
    // HTTP ブロック関連
    bool parseHttpBlock(const std::vector<std::string>& tokens, size_t* pos);
    bool parseHttpDirectives(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* config);
    bool validateServerBlocks();
    // サーバーブロック関連
    bool parseServerBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* config);
    bool parseServerDirectives(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* config);
    bool parseServerDirectiveContent(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive_name);
    // ロケーションブロック関連
    bool parseLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* config);
    bool parseLocationDirectives(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* config);
    // ヘルパーメソッド
    bool loadDefaultConfig();
    bool validateBlockStart(const std::vector<std::string>& tokens, size_t* pos, const std::string& blockType);
    bool validateAndParseLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* config);
    bool validateBlockEnd(const std::vector<std::string>& tokens, size_t* pos);

    std::string _input;
    std::vector<std::string> _tokens;
    config::HttpConfig _config;
    config::DirectiveParser _directiveParser;
};

}  // namespace config
