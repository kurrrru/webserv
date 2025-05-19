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

toolbox::SharedPtr<HttpConfig> ConfigParser::parseFile(const std::string& filepath) {
    if (!readFile(filepath)) {
        throwConfigError("open() \"" + filepath + "\" failed");
    }
    ConfigLexer lexer;
    _tokens = lexer.tokenize(_input);
    if (_tokens.empty()) {
        toolbox::logger::StepMark::warning("No tokens found in config file.");
        return toolbox::SharedPtr<HttpConfig>(new HttpConfig());
    }
    if (!parse()) {
        throwConfigError("Failed to parse configuration");
    }
    return _config;
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
