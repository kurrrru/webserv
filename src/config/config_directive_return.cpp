// Copyright 2025 Ideal Broccoli

#include <cstdlib>
#include <string>
#include <cctype>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool DirectiveParser::parseReturnDirective(const std::vector<std::string>& tokens, size_t* pos, int* return_code, std::string* return_url) {
    if (!return_code || !return_url) {
        toolbox::logger::StepMark::error("Invalid return_code or return_url pointer");
        return false;
    }
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing return directive");
        return false;
    }
    std::string first_token = tokens[(*pos)++];
    // 最初のトークンがステータスコード（数値）かURLか確認
    bool first_is_code = true;
    for (size_t i = 0; i < first_token.size(); ++i) {
        if (!std::isdigit(first_token[i])) {
            first_is_code = false;
            break;
        }
    }
    if (first_is_code) {
        // ステータスコードとして解析
        int code = std::atoi(first_token.c_str());
        // 有効なリダイレクトコードか確認（主に3XX）
        if ((code < 300 || code > 308) && code != 201) {
            toolbox::logger::StepMark::warning("Return code " + first_token + " is not a standard redirect status code");
            // 警告だけ出して処理は続行
        }
        *return_code = code;
        // 次のトークンはURLのはず
        if (*pos >= tokens.size() || tokens[*pos] == ";") {
            toolbox::logger::StepMark::error("URL expected after status code in return directive");
            return false;
        }
        *return_url = tokens[(*pos)++];
    } else {
        // デフォルトのリダイレクトコード（302）を使用
        *return_code = 302; // Found
        *return_url = first_token;
    }
    // URLの簡易検証
    if (return_url->empty()) {
        toolbox::logger::StepMark::error("Return URL cannot be empty");
        return false;
    }
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after return directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    toolbox::logger::StepMark::debug("Return directive set with code " + toolbox::to_string(*return_code) + " and URL: " + *return_url);
    return true;
}

}  // namespace config
