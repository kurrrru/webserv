// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>
#include <cstring>

#include "config_lexer.hpp"
#include "config_namespace.hpp"
#include "config/config.hpp"

#include "../toolbox/stepmark.hpp"
#include "../toolbox/string.hpp"

namespace config {

ConfigLexer::ConfigLexer() {}

// トークン化を行うメソッド
std::vector<std::string> ConfigLexer::tokenize(const std::string& input) {
    size_t pos = 0;
    std::vector<std::string> tokens;

    // トークン化を開始
    while (pos < input.length()) {
        // 空白をスキップ
        skipWhitespace(input, &pos);
        if (pos >= input.length()) {
            break;
        }
        // コメント行をスキップ
        if (input[pos] == config::token::COMMENT_CHAR) {
            skipComment(input, &pos);
            continue;
        }
        std::string token;
        // トークンの読み取り
        if (!readToken(input, &pos, &token)) {
            toolbox::logger::StepMark::error("Failed to read token while tokenizing");
            throw ConfigException("Failed to read token while tokenizing");
        }
        // トークンが空でない場合、トークンを追加
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    toolbox::logger::StepMark::debug("Tokenization complete: " + toolbox::to_string(static_cast<int>(tokens.size())) + " tokens");
    return tokens;
}

// 空白をスキップするメソッド
void ConfigLexer::skipWhitespace(const std::string& input, size_t* pos) {
    while (*pos < input.length() && isWhitespace(input[*pos])) {
        (*pos)++;
    }
}

// 空白文字かどうかを判定するメソッド
bool ConfigLexer::isWhitespace(char c) const {
    return std::strchr(config::token::WHITESPACE_CHARS, c) != NULL;
}

// コメントをスキップするメソッド
void ConfigLexer::skipComment(const std::string& input, size_t* pos) {
    while (*pos < input.length() && input[*pos] != '\n') {
        (*pos)++;
    }
    // コメントの終わりでポジションを1つ進める
    if (*pos < input.length()) {
        (*pos)++;
    }
}

// トークンを読み取るメソッド
bool ConfigLexer::readToken(const std::string& input, size_t* pos, std::string* token) {
    // トークンの状態を初期化
    token->clear();
    // クォート文字列の処理
    if (input[*pos] == config::token::DOUBLE_QUOTE ||
        input[*pos] == config::token::SINGLE_QUOTE) {
            if (!readQuotedString(input, pos, token)) {
                toolbox::logger::StepMark::error("Failed to read quoted string while tokenizing");
                return false;
            }
    // 特殊トークンの処理
    } else if (input[*pos] == config::token::OPEN_BRACE[0] ||
                input[*pos] == config::token::CLOSE_BRACE[0] ||
                input[*pos] == config::token::SEMICOLON[0]) {
                 *token = input[(*pos)++];
    // 通常トークンの処理
    } else {
        if (!readPlainToken(input, pos, token)) {
            toolbox::logger::StepMark::error("Failed to read plain token while tokenizing");
            return false;
        }
    }
    // トークンの内容をデバックで表示
    toolbox::logger::StepMark::debug("Token read: " + *token);
    return true;
}

// クォートされた文字列を読み取るメソッド
bool ConfigLexer::readQuotedString(const std::string& input, size_t* pos, std::string* token) {
    // トークンの状態を初期化
    token->clear();
    char quote = input[*pos];
    (*pos)++;
    // クォート文字列の読み取り
    // TODO(yootsubo) :もしクォートが閉じられない場合
    while (*pos < input.length() && input[*pos] != quote) {
        (*token) += input[(*pos)++];
    }
    if (*pos >= input.length()) {
        return false;
    }
    (*pos)++;
    return true;
}

// 通常トークンを読み取るメソッド
bool ConfigLexer::readPlainToken(const std::string& input, size_t* pos, std::string* token) {
    // トークンの状態を初期化
    token->clear();
    // トークンの読み取り
    while (*pos < input.length() &&
            !isWhitespace(input[*pos]) &&
            input[*pos] != config::token::OPEN_BRACE[0] &&
            input[*pos] != config::token::CLOSE_BRACE[0] &&
            input[*pos] != config::token::SEMICOLON[0] &&
            input[*pos] != config::token::COMMENT_CHAR) {
        (*token) += input[(*pos)++];
    }
    if (*pos < input.length() && input[*pos] == config::token::COMMENT_CHAR) {
        skipComment(input, pos);
    }
    return true;
}

}  // namespace config
