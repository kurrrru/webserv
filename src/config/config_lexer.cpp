// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>
#include <cstring>

#include "config_lexer.hpp"
#include "config_namespace.hpp"
#include "config.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

ConfigLexer::ConfigLexer() {
}

ConfigLexer::~ConfigLexer() {
}


// 空白文字かどうかを判定するメソッド
bool isWhitespace(const char c) {
    return std::strchr(config::token::WHITESPACE_CHARS, c) != NULL;
}


// 空白をスキップするメソッド
void skipWhitespace(const std::string& input, size_t* pos) {
    while (*pos < input.length() && isWhitespace(input[*pos])) {
        (*pos)++;
    }
}

// 改行かどうかを判定するメソッド
bool isNewline(const std::string& input, size_t pos) {
    // 単一文字の改行 (LF または CR)
    if (input[pos] == config::token::LF[0] || input[pos] == config::token::CR[0]) {
        return true;
    }
    // CRLFの検出 (2文字の連続)
    if (pos + 1 < input.length() &&
        input[pos] == config::token::CR[0] &&
        input[pos + 1] == config::token::LF[0]) {
        return true;
    }
    return false;
}

// コメントをスキップするメソッド
void skipComment(const std::string& input, size_t* pos) {
    while (*pos < input.length() && !isNewline(input, *pos)) {
        (*pos)++;
    }
    // コメントの終わりでポジションを1つ進める
    if (*pos < input.length()) {
        (*pos)++;
    }
}

// エスケープシーケンスを処理する関数
void processEscapeSequence(const std::string& input, size_t* pos, std::string* token) {
    // バックスラッシュをスキップ
    (*pos)++;
    switch (input[*pos]) {
        case 'n':
            (*token) += config::token::LF[0];
            break;
        case 'r':
            (*token) += config::token::CR[0];
            break;
        case 't':
            (*token) += config::token::TAB[0];
            break;
        default:
            // その他のエスケープシーケンスはそのまま文字として追加
            (*token) += input[*pos];
            break;
    }
    (*pos)++;
}

// 引用符後の文字が有効な区切り文字かをチェックする関数
void validateAfterQuotedString(const std::string& input, size_t pos) {
    if (pos < input.length()) {
        char ch = input[pos];
        // 有効な区切り文字かチェック
        if (!isWhitespace(ch) &&
            ch != config::token::SEMICOLON[0] &&
            ch != config::token::OPEN_BRACE[0] &&
            ch != config::token::CLOSE_BRACE[0] &&
            !isNewline(input, pos)) {
            toolbox::logger::StepMark::error("unexpected \"" + std::string(1, ch) + "\"");
            throw ConfigException("unexpected \"" + std::string(1, ch) + "\"");
        }
    }
}

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
        if (input[pos] == config::token::COMMENT_CHAR[0]) {
            skipComment(input, &pos);
            continue;
        }
        std::string token;
        // トークンの読み取り
        if (!readToken(input, &pos, &token)) {
            toolbox::logger::StepMark::error("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \"" + std::string(config::token::CLOSE_BRACE) + "\"");
            throw ConfigException("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \"" + std::string(config::token::CLOSE_BRACE) + "\"");
        }
        // トークンの最長サイズ制限
        if (token.size() + std::string(config::token::SEMICOLON).size() > config::CONF_BUFFER) {
            toolbox::logger::StepMark::error("too long parameter");
            throw ConfigException("too long parameter");
        }
        // トークンが空でない場合、トークンを追加
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// トークンを読み取るメソッド
bool ConfigLexer::readToken(const std::string& input, size_t* pos, std::string* token) {
    // トークンの状態を初期化
    token->clear();
    // クォート文字列の処理
    if (input[*pos] == config::token::DOUBLE_QUOTE[0] ||
        input[*pos] == config::token::SINGLE_QUOTE[0]) {
            if (!readQuotedString(input, pos, token)) {
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
        // トークンの最長サイズ制限
        if ((*token).length() >= config::CONF_BUFFER) {
            toolbox::logger::StepMark::error("too long parameter, probably missing terminating character");
            throw ConfigException("too long parameter, probably missing terminating character");
        }
        // エスケープシーケンスの処理
        if (input[*pos] == config::token::BACKSLASH[0] && *pos + 1 < input.length()) {
            processEscapeSequence(input, pos, token);
        } else {
            // 通常の文字として追加
            (*token) += input[(*pos)++];
        }
    }
    if (*pos >= input.length()) {
        return false;
    }
    (*pos)++;
    // Nginxと同様の処理: 引用符の後は区切り文字であるべき
    validateAfterQuotedString(input, *pos);
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
            input[*pos] != config::token::COMMENT_CHAR[0]) {
        (*token) += input[(*pos)++];
    }
    if (*pos < input.length() && input[*pos] == config::token::COMMENT_CHAR[0]) {
        skipComment(input, pos);
    }
    return true;
}

}  // namespace config
