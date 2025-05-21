#include <string>
#include <vector>
#include <cstring>

#include "config_lexer.hpp"
#include "config_namespace.hpp"
#include "config.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

ConfigLexer::ConfigLexer() {
}

ConfigLexer::~ConfigLexer() {
}

bool isWhitespace(const char c) {
    return std::strchr(config::token::WHITESPACE_CHARS, c) != NULL;
}

void skipWhitespace(const std::string& input, size_t* pos) {
    while (*pos < input.length() && isWhitespace(input[*pos])) {
        (*pos)++;
    }
}

bool isNewline(const std::string& input, size_t pos) {
    if (input[pos] == config::token::LF[0] || input[pos] == config::token::CR[0]) {
        return true;
    }
    return false;
}

void skipComment(const std::string& input, size_t* pos) {
    while (*pos < input.length() && !isNewline(input, *pos)) {
        (*pos)++;
    }
    if (*pos < input.length()) {
        (*pos)++;
    }
}

void processEscapeSequence(const std::string& input, size_t* pos, std::string* token) {
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
            (*token) += input[*pos];
            break;
    }
    (*pos)++;
}

void validateAfterQuotedString(const std::string& input, size_t pos) {
    if (pos < input.length()) {
        char ch = input[pos];
        if (!isWhitespace(ch) &&
            ch != config::token::SEMICOLON[0] &&
            ch != config::token::OPEN_BRACE[0] &&
            ch != config::token::CLOSE_BRACE[0] &&
            !isNewline(input, pos)) {
            throwConfigError("unexpected \"" + std::string(1, ch) + "\"");
        }
    }
}

std::vector<std::string> ConfigLexer::tokenize(const std::string& input) {
    size_t pos = 0;
    std::vector<std::string> tokens;
    std::string token;
    std::string preToken;
    while (pos < input.length()) {
        skipWhitespace(input, &pos);
        if (pos >= input.length()) {
            break;
        }
        if (input[pos] == config::token::COMMENT_CHAR[0]) {
            skipComment(input, &pos);
            continue;
        }
        preToken = token;
        if (!readToken(input, &pos, &token)) {
            throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \"" + std::string(config::token::CLOSE_BRACE) + "\"");
        }
        if (token.size() + std::string(config::token::SEMICOLON).size() > config::CONF_BUFFER) {
            throwConfigError("too long parameter");
        }
        if (preToken == config::token::OPEN_BRACE ||
            preToken == config::token::CLOSE_BRACE ||
            preToken == config::token::SEMICOLON) {
            if (token == config::token::OPEN_BRACE ||
                token == config::token::SEMICOLON) {
                throwConfigError("unexpected \"" + std::string(1, token[0]) + "\"");
            }
        }
        tokens.push_back(token);
    }
    return tokens;
}

bool ConfigLexer::readToken(const std::string& input, size_t* pos, std::string* token) {
    token->clear();
    if (input[*pos] == config::token::DOUBLE_QUOTE[0] ||
        input[*pos] == config::token::SINGLE_QUOTE[0]) {
            if (!readQuotedString(input, pos, token)) {
                return false;
            }
    } else if (input[*pos] == config::token::OPEN_BRACE[0] ||
                input[*pos] == config::token::CLOSE_BRACE[0] ||
                input[*pos] == config::token::SEMICOLON[0]) {
                *token = input[(*pos)++];
    } else {
        if (!readPlainToken(input, pos, token)) {
            return false;
        }
    }
    return true;
}

bool ConfigLexer::readQuotedString(const std::string& input, size_t* pos, std::string* token) {
    token->clear();
    char quote = input[*pos];
    (*pos)++;
    while (*pos < input.length() && input[*pos] != quote) {
        if ((*token).length() >= config::CONF_BUFFER) {
            throwConfigError("too long parameter, probably missing terminating character");
        }
        if (input[*pos] == config::token::BACKSLASH[0] && *pos + 1 < input.length()) {
            processEscapeSequence(input, pos, token);
        } else {
            (*token) += input[(*pos)++];
        }
    }
    if (*pos >= input.length()) {
        return false;
    }
    (*pos)++;
    validateAfterQuotedString(input, *pos);
    return true;
}

bool ConfigLexer::readPlainToken(const std::string& input, size_t* pos, std::string* token) {
    token->clear();
    size_t start = *pos;
    while (*pos < input.length() &&
            !isWhitespace(input[*pos]) &&
            input[*pos] != config::token::OPEN_BRACE[0] &&
            input[*pos] != config::token::CLOSE_BRACE[0] &&
            input[*pos] != config::token::SEMICOLON[0] &&
            input[*pos] != config::token::COMMENT_CHAR[0]) {
        (*pos)++;
    }
    *token = input.substr(start, *pos - start);
    return true;
}

}  // namespace config
