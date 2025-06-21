// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>

namespace config {

class ConfigLexer {
 public:
    ConfigLexer();
    ~ConfigLexer();
    std::vector<std::string> tokenize(const std::string& input);
 
 private:
    ConfigLexer(const ConfigLexer&);
    ConfigLexer& operator=(const ConfigLexer&);
    bool readToken(const std::string& input, size_t* pos, std::string* token);
    bool readPlainToken(const std::string& input, size_t* pos, std::string* token);
    bool readQuotedString(const std::string& input, size_t* pos, std::string* token);
};

}  // namespace config
