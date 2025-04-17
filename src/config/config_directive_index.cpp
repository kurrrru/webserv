// Copyright 2025 Ideal Broccoli

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseIndexDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* index_files) {
    if (!index_files) {
        toolbox::logger::StepMark::error("Invalid index_files pointer");
        return false;
    }
    index_files->clear();
    while (*pos < tokens.size() && tokens[*pos] != ";") {
        std::string file = tokens[*pos];
        if (file.empty()) {
            toolbox::logger::StepMark::error("Index file name cannot be empty");
            return false;
        }
        if (file.find('\0') != std::string::npos) {
            toolbox::logger::StepMark::error("Index file name contains null character");
            return false;
        }
        index_files->push_back(file);
        (*pos)++;
    }
    if (index_files->empty()) {
        toolbox::logger::StepMark::error("At least one index file must be specified");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after index directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    return true;
}
}  // namespace config
