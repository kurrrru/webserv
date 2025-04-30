// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>

#include "config.hpp"
#include "config_lexer.hpp"
#include "config_directive_handler.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

class ServerConfig;
class LocationConfig;
class HttpConfig;
class ConfigParser {
 public:
    static toolbox::SharedPtr<Config> parseFile(const std::string& filepath, bool is_default);

 private:
    ConfigParser();
    ~ConfigParser();
    ConfigParser(const ConfigParser&);
    ConfigParser& operator=(const ConfigParser&);
    bool readFile(const std::string& filepath);
    bool parse();
    bool parseHttpBlock(const std::vector<std::string>& tokens, size_t* pos);
    bool parseHttpDirectives(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http_config);
    bool validateServerBlocks();
    bool parseServerBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config);
    bool parseServerDirectives(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config);
    bool parseServerDirectiveContent(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive_name);
    bool parseLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config ,config::LocationConfig* location_config);
    bool parseLocationDirectives(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config);
    bool validateAndParseLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config);
    bool parseNestedLocationBlock(const std::vector<std::string>& tokens,  size_t* pos,  config::LocationConfig* parent_location, config::LocationConfig* location_config);
    bool handleNestedLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* parent_location);
    bool loadDefaultConfig();
    bool validateBlockStart(const std::vector<std::string>& tokens, size_t* pos, const std::string& blockType);
    bool validateBlockEnd(const std::vector<std::string>& tokens, size_t* pos);

    std::string _input;
    std::vector<std::string> _tokens;
    toolbox::SharedPtr<config::HttpConfig> _config;
    config::DirectiveParser _directiveParser;
};

}  // namespace config
