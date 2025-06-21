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
    ConfigParser();
    ~ConfigParser();
    toolbox::SharedPtr<HttpConfig> parseFile(const std::string& filepath);
    std::size_t getTokenCount() const { return _tokens.size(); }

 private:
    ConfigParser(const ConfigParser&);
    ConfigParser& operator=(const ConfigParser&);

    bool readFile(const std::string& filepath);
    bool parse();
    bool parseHttpBlock(const std::vector<std::string>& tokens, std::size_t* pos);
    bool parseHttpDirectives(const std::vector<std::string>& tokens, std::size_t* pos, config::HttpConfig* httpConfig);
    bool parseServerBlock(const std::vector<std::string>& tokens, std::size_t* pos, config::ServerConfig* serverConfig);
    bool parseServerDirectives(const std::vector<std::string>& tokens, std::size_t* pos, config::ServerConfig* serverConfig);
    bool parseServerDirectiveContent(const std::vector<std::string>& tokens, std::size_t* pos, const std::string& directiveName);
    bool parseLocationBlock(const std::vector<std::string>& tokens, std::size_t* pos, config::ServerConfig* serverConfig ,config::LocationConfig* locationConfig);
    bool parseLocationDirectives(const std::vector<std::string>& tokens, std::size_t* pos, config::LocationConfig* locationConfig);
    bool parseNestedLocationBlock(const std::vector<std::string>& tokens,  std::size_t* pos,  config::LocationConfig* parentLocation, config::LocationConfig* locationConfig);
    bool handleNestedLocationBlock(const std::vector<std::string>& tokens, std::size_t* pos, config::LocationConfig* parentLocation);
    void validateBlockEnd(const std::vector<std::string>& tokens, std::size_t* pos);

    std::string _input;
    std::vector<std::string> _tokens;
    toolbox::SharedPtr<config::HttpConfig> _config;
    config::DirectiveParser _directiveParser;
};

}  // namespace config
