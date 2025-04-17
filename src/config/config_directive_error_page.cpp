// Copyright 2025 Ideal Broccoli

#include <cstdlib>
#include <string>
#include <cctype>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool DirectiveParser::parseErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ErrorPage>* error_pages) {    
    if (!error_pages) {
        toolbox::logger::StepMark::error("Invalid error_pages pointer");
        return false;
    }
    std::vector<int> codes;
    std::string path;
    // ステータスコードを読み取る（数値の列）
    while (*pos < tokens.size() && tokens[*pos] != ";") {
        std::string token = tokens[*pos];
        // 数値かどうか確認
        bool is_number = true;
        for (size_t i = 0; i < token.size(); ++i) {
            if (!std::isdigit(token[i])) {
                is_number = false;
                break;
            }
        }
        if (is_number) {
            // ステータスコードの検証
            int code = std::atoi(token.c_str());
            if (code < 300 || code > 599) {
                toolbox::logger::StepMark::error("Invalid HTTP status code: " + token + ". Must be between 300 and 599");
                return false;
            }
            codes.push_back(code);
        } else {
            // パスとして扱う
            if (!path.empty()) {
                toolbox::logger::StepMark::error("Multiple paths specified for error_page directive");
                return false;
            }
            path = token;
        }
        (*pos)++;
    }
    // 少なくとも1つのステータスコードとパスが必要
    if (codes.empty()) {
        toolbox::logger::StepMark::error("At least one HTTP status code must be specified for error_page directive");
        return false;
    }
    if (path.empty()) {
        toolbox::logger::StepMark::error("Path must be specified for error_page directive");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after error_page directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    // ErrorPage オブジェクトを作成して追加
    ErrorPage error_page;
    error_page.codes = codes;
    error_page.path = path;
    error_pages->push_back(error_page);
    toolbox::logger::StepMark::debug("Added error page mapping for " + toolbox::to_string(codes.size()) + " status codes to path: " + path);
    return true;
}

}  // namespace config
