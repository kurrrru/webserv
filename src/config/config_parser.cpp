#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include "config_parser.hpp"
#include "config.hpp"
#include "config_namespace.hpp"
#include "config_inherit.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

ConfigParser::ConfigParser() :
_input(),
_tokens(),
_config(toolbox::SharedPtr<HttpConfig>(new HttpConfig())) {
}

ConfigParser::~ConfigParser() {
}

toolbox::SharedPtr<Config> ConfigParser::parseFile(const std::string& filepath) {
    ConfigParser parser;
    if (!parser.readFile(filepath)) {
        throwConfigError("open() \"" + filepath + "\" failed");
    }
    ConfigLexer lexer;
    parser._tokens = lexer.tokenize(parser._input);
    toolbox::SharedPtr<Config> config(new Config());
    if (parser._tokens.empty()) {
        toolbox::logger::StepMark::warning("No tokens found in config file.");
        return config;
    }
    if (!parser.parse()) {
        throwConfigError("Failed to parse configuration");
    }
    config->setHttpConfig(parser._config);
    config->setTokenCount(parser._tokens.size());
    return config;
}

bool ConfigParser::readFile(const std::string& filepath) {
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

bool ConfigParser::parse() {
    toolbox::logger::StepMark::info("Parsing configuration");
    size_t pos = 0;
    if (_tokens.empty()) {
        return false;
    } else {
        if (!parseHttpBlock(_tokens, &pos)) {
            return false;
        }
        if (pos < _tokens.size()) {
            throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \""+ std::string(config::token::CLOSE_BRACE) + "\"");
        }
    }
    ConfigInherit inherit(_config.get());
    inherit.applyInheritance();
    toolbox::logger::StepMark::info("Configuration parsing completed successfully.");
    return true;
}

void ConfigParser::validateBlockEnd(const std::vector<std::string>& tokens, size_t* pos) {
    if (*pos >= tokens.size() || tokens[*pos] != config::token::CLOSE_BRACE) {
        throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::CLOSE_BRACE) + "\"");
    }
    (*pos)++;
}

}  // namespace config
