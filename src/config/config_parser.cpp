// Copyright 2025 Ideal Broccoli

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include "config_parser.hpp"
#include "config.hpp"
#include "config_namespace.hpp"
#include "config_inherit.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

ConfigParser::ConfigParser() :
_input(""),
_tokens(),
_config(toolbox::SharedPtr<HttpConfig>(new HttpConfig())) {
}

ConfigParser::~ConfigParser() {
}

toolbox::SharedPtr<Config> ConfigParser::parseFile(const std::string& filepath, bool is_default) {
    ConfigParser parser;
    if (is_default) {
        if (!parser.loadDefaultConfig()) {
            throw ConfigException("Failed to load default config file");
        }
    } else {
        if (!parser.readFile(filepath)) {
            throw ConfigException("Failed to read config file: " + filepath);
        }
    }
    ConfigLexer lexer;
    parser._tokens = lexer.tokenize(parser._input);
    toolbox::SharedPtr<Config> config(new Config());
    if (parser._tokens.empty()) {
        toolbox::logger::StepMark::warning("No tokens found in config file.");
        return config;
    }
    if (!parser.parse()) {
        throw ConfigException("Failed to parse configuration");
    }
    config->setHttpConfig(parser._config);
    config->setTokenCount(parser._tokens.size());
    return config;
}

bool ConfigParser::readFile(const std::string& filepath) {
    std::ifstream file(filepath.c_str());
    if (!file) {
        toolbox::logger::StepMark::error("open() \"" + filepath + "\" failed");
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    _input = buffer.str();
    file.close();
    return true;
}

bool ConfigParser::loadDefaultConfig() {
    toolbox::logger::StepMark::info("Using default configuration.");
    if (!readFile(config::DEFAULT_FILE)) {
        toolbox::logger::StepMark::error("Failed to read default config file: " + std::string(config::DEFAULT_FILE));
        return false;
    }
    ConfigLexer lexer;
    _tokens = lexer.tokenize(_input);
    if (_tokens.empty()) {
        toolbox::logger::StepMark::error("No tokens found in default config file.");
        return false;
    }
    return true;
}

bool ConfigParser::parse() {
    toolbox::logger::StepMark::info("Parsing configuration");
    size_t pos = 0;
    if (_tokens.empty()) {
        return false;
    } else {
        if (pos >= _tokens.size() || _tokens[pos] != config::context::HTTP) {
            toolbox::logger::StepMark::error("Configuration must start with 'http' block.");
            return false;
        }
        if (!parseHttpBlock(_tokens, &pos)) {
            return false;
        }
        if (pos < _tokens.size()) {
            toolbox::logger::StepMark::error("Unexpected token '" + _tokens[pos] + "' after http block.");
            return false;
        }
    }
    ConfigInherit inherit(_config.get());
    inherit.applyInheritance();
    toolbox::logger::StepMark::info("Configuration parsing completed successfully.");
    return true;
}

bool ConfigParser::validateBlockStart(const std::vector<std::string>& tokens, size_t* pos, const std::string& expectedDirective) {
    if (*pos >= tokens.size() || tokens[*pos] != expectedDirective) {
        toolbox::logger::StepMark::error("Expected '" + expectedDirective + "' directive.");
        return false;
    }
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        toolbox::logger::StepMark::error("Expected '{' after '" + expectedDirective + "' directive.");
        return false;
    }
    (*pos)++;
    return true;
}

bool ConfigParser::validateBlockEnd(const std::vector<std::string>& tokens, size_t* pos) {
    if (*pos >= tokens.size() || tokens[*pos] != config::token::CLOSE_BRACE) {
        toolbox::logger::StepMark::error("Expected '}' to close block.");
        return false;
    }
    (*pos)++;
    return true;
}

}  // namespace config
