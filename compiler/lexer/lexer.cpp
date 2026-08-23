#include "lexer.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace eng {

Lexer::Lexer(const std::string& source)
    : source_(source), pos_(0), line_(1), col_(0) {}

bool Lexer::isDigit(char c) const { return c >= '0' && c <= '9'; }
bool Lexer::isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Lexer::isIdentStart(char c) const { return isAlpha(c); }
bool Lexer::isIdentContinue(char c) const { return isAlpha(c) || isDigit(c); }

const std::unordered_map<std::string, TokenType>& Lexer::keywords() {
    static const std::unordered_map<std::string, TokenType> kw = {
        {"fn", TokenType::FN},
        {"let", TokenType::LET},
        {"mut", TokenType::MUT},
        {"const", TokenType::CONST},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"for", TokenType::FOR},
        {"while", TokenType::WHILE},
        {"return", TokenType::RETURN},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"match", TokenType::MATCH},
        {"struct", TokenType::STRUCT},
        {"enum", TokenType::ENUM},
        {"impl", TokenType::IMPL},
        {"interface", TokenType::INTERFACE},
        {"import", TokenType::IMPORT},
        {"pub", TokenType::PUB},
        {"as", TokenType::AS},
        {"in", TokenType::IN},
        {"elif", TokenType::ELIF},
        {"module", TokenType::MODULE},
        {"export", TokenType::EXPORT},
        {"load", TokenType::LOAD},
        {"try", TokenType::TRY},
        {"catch", TokenType::CATCH},
        {"throw", TokenType::THROW},
        {"except", TokenType::EXCEPT},
        {"finally", TokenType::FINALLY},
        {"bytecode", TokenType::BYTECODE},
        {"vm", TokenType::VM},
        {"op", TokenType::OP},
        // Testing framework
        {"test", TokenType::TEST},
        {"assert", TokenType::ASSERT},
        {"expect", TokenType::EXPECT},
        // Design by Contract
        {"requires", TokenType::REQUIRES},
        {"ensures", TokenType::ENSURES},
        {"invariant", TokenType::INVARIANT},
        // Hot reload
        {"hot", TokenType::HOT},
        {"reload", TokenType::RELOAD},
        // Coroutines
        {"coroutine", TokenType::COROUTINE},
        {"yield", TokenType::YIELD},
        {"await", TokenType::AWAIT},
        // Dimensional types (physical units)
        {"length", TokenType::LENGTH},
        {"time", TokenType::TIME},
        {"mass", TokenType::MASS},
        {"force", TokenType::FORCE},
        // Python-style word aliases for the logical operators
        {"and", TokenType::AND},
        {"or", TokenType::OR},
        {"not", TokenType::NOT},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"none", TokenType::NONE},
        {"int", TokenType::INT_T},
        {"int8", TokenType::INT8},
        {"int16", TokenType::INT16},
        {"int32", TokenType::INT32},
        {"int64", TokenType::INT64},
        {"uint", TokenType::UINT},
        {"uint8", TokenType::UINT8},
        {"uint16", TokenType::UINT16},
        {"uint32", TokenType::UINT32},
        {"uint64", TokenType::UINT64},
        {"float32", TokenType::F32},
        {"float64", TokenType::F64},
        {"bool", TokenType::BOOL_T},
        {"char", TokenType::CHAR_T},
        {"string", TokenType::STRING},
        {"byte", TokenType::BYTE},
        {"void", TokenType::VOID},
        {"option", TokenType::OPTION},
        {"result", TokenType::RESULT},
        // ETC script commands
        {"print", TokenType::PRINT},
        {"output", TokenType::OUTPUT},
        {"say", TokenType::SAY},
        {"input", TokenType::INPUT},
    };
    return kw;
}

char Lexer::peekChar() const {
    if (pos_ >= source_.size()) return '\0';
    return source_[pos_];
}

char Lexer::advanceChar() {
    if (pos_ >= source_.size()) return '\0';
    char c = source_[pos_];
    pos_++;
    if (c == '\n') { line_++; col_ = 0; }
    else { col_++; }
    return c;
}

void Lexer::skipWhitespace() {
    while (pos_ < source_.size() && (source_[pos_] == ' ' || source_[pos_] == '\t' ||
           source_[pos_] == '\r' || source_[pos_] == '\n')) {
        advanceChar();
    }
}

void Lexer::skipLineComment() {
    while (pos_ < source_.size() && source_[pos_] != '\n') {
        advanceChar();
    }
}

void Lexer::skipBlockComment() {
    while (pos_ < source_.size() - 1) {
        if (source_[pos_] == '*' && source_[pos_ + 1] == '/') {
            pos_ += 2;
            return;
        }
        advanceChar();
    }
    if (pos_ >= source_.size()) {
        diagnostics.push_back({line_, col_, "Unterminated block comment", Diag::Level::Error});
    }
}

Token Lexer::tokenizeIdentifier() {
    size_t start = pos_;
    while (pos_ < source_.size() && isIdentContinue(source_[pos_])) {
        advanceChar();
    }
    std::string lexeme = source_.substr(start, pos_ - start);
    auto& kw = keywords();
    auto it = kw.find(lexeme);
    if (it != kw.end()) {
        return Token(it->second, lexeme, line_, col_ - lexeme.size(), start);
    }
    return Token(TokenType::IDENT, lexeme, line_, col_ - lexeme.size(), start);
}

Token Lexer::tokenizeInteger() {
    size_t start = pos_;
    while (pos_ < source_.size() && isDigit(source_[pos_])) {
        advanceChar();
    }
    std::string lexeme = source_.substr(start, pos_ - start);
    try {
        long long val = std::stoll(lexeme);
        (void)val;
    } catch (...) {
        diagnostics.push_back({line_, col_, "Integer literal out of range", Diag::Level::Error});
    }
    return Token(TokenType::INT_LITERAL, lexeme, line_, (int)(col_ - (int)lexeme.size()), start);
}

Token Lexer::tokenizeFloat() {
    size_t start = pos_;
    while (pos_ < source_.size() && (isDigit(source_[pos_]) || source_[pos_] == '.')) {
        advanceChar();
    }
    std::string lexeme = source_.substr(start, pos_ - start);
    return Token(TokenType::FLOAT_LITERAL, lexeme, line_, (int)(col_ - (int)lexeme.size()), start);
}

Token Lexer::tokenizeString() {
    size_t start = pos_;
    advanceChar(); // skip opening quote
    std::string content;
    while (pos_ < source_.size() && source_[pos_] != '"') {
        if (source_[pos_] == '\\' && pos_ + 1 < source_.size()) {
            char next = source_[pos_ + 1];
            switch (next) {
                case 'n': content += '\n'; break;
                case 't': content += '\t'; break;
                case 'r': content += '\r'; break;
                case '"': content += '"'; break;
                case '\\': content += '\\'; break;
                case '/': content += '/'; break;
                default: content += next; break;
            }
            pos_ += 2;
        } else {
            content += source_[pos_];
            pos_++;
        }
    }
    if (pos_ >= source_.size()) {
        diagnostics.push_back({line_, col_, "Unterminated string literal", Diag::Level::Error});
        return Token(TokenType::STRING_LITERAL, content, line_, col_, start);
    }
    advanceChar(); // skip closing quote
    return Token(TokenType::STRING_LITERAL, content, line_, col_, start);
}

Token Lexer::tokenizeChar() {
    size_t start = pos_;
    advanceChar(); // skip opening quote
    if (pos_ >= source_.size()) {
        diagnostics.push_back({line_, col_, "Unterminated char literal", Diag::Level::Error});
        return Token(TokenType::CHAR_LITERAL, "", line_, col_, start);
    }
    std::string content;
    if (source_[pos_] == '\\' && pos_ + 1 < source_.size()) {
        char next = source_[pos_ + 1];
        switch (next) {
            case 'n': content += '\n'; break;
            case 't': content += '\t'; break;
            case 'r': content += '\r'; break;
            case '\'': content += '\''; break;
            case '\\': content += '\\'; break;
            default: content += next; break;
        }
        pos_ += 2;
    } else {
        content += source_[pos_];
        pos_++;
    }
    if (pos_ >= source_.size() || source_[pos_] != '\'') {
        diagnostics.push_back({line_, col_, "Unterminated char literal", Diag::Level::Error});
    } else {
        advanceChar();
    }
    return Token(TokenType::CHAR_LITERAL, content, line_, col_, start);
}

std::vector<Token> Lexer::tokenize() {
    tokens_.reserve(256);
    while (pos_ < source_.size()) {
        skipWhitespace();
        if (pos_ >= source_.size()) break;

        char c = source_[pos_];
        size_t start = pos_;

        if (c == '/' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '/') {
            skipLineComment();
            tokens_.emplace_back(TokenType::COMMENT, "", line_, col_);
            continue;
        }
        if (c == '/' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '*') {
            pos_ += 2;
            skipBlockComment();
            tokens_.emplace_back(TokenType::COMMENT, "", line_, col_);
            continue;
        }

        if (isAlpha(c) || c == '_') {
            tokens_.push_back(tokenizeIdentifier());
        } else if (isDigit(c)) {
            if (c == '0' && pos_ + 1 < source_.size() && source_[pos_ + 1] == 'x') {
                // hex — tokenize as identifier-like or error
                diagnostics.push_back({line_, col_, "Hex literals not yet supported", Diag::Level::Error});
                advanceChar(); advanceChar();
                while (pos_ < source_.size() && (isDigit(source_[pos_]) || (source_[pos_] >= 'a' && source_[pos_] <= 'f'))) advanceChar();
                tokens_.emplace_back(TokenType::UNKNOWN, source_.substr(start, pos_ - start), line_, col_, start);
            } else if (c == '0' && pos_ + 1 < source_.size() && source_[pos_ + 1] == 'b') {
                diagnostics.push_back({line_, col_, "Binary literals not yet supported", Diag::Level::Error});
                advanceChar(); advanceChar();
                while (pos_ < source_.size() && (source_[pos_] == '0' || source_[pos_] == '1')) advanceChar();
                tokens_.emplace_back(TokenType::UNKNOWN, source_.substr(start, pos_ - start), line_, col_, start);
            } else {
                // check for float
                size_t save = pos_;
                int saveLine = line_, saveCol = col_;
                while (pos_ < source_.size() && isDigit(source_[pos_])) advanceChar();
                if (pos_ < source_.size() && source_[pos_] == '.' && pos_ + 1 < source_.size() && isDigit(source_[pos_ + 1])) {
                    pos_ = save; line_ = saveLine; col_ = saveCol;
                    tokens_.push_back(tokenizeFloat());
                } else {
                    pos_ = save; line_ = saveLine; col_ = saveCol;
                    tokens_.push_back(tokenizeInteger());
                }
            }
        } else if (c == '"') {
            tokens_.push_back(tokenizeString());
        } else if (c == '\'') {
            tokens_.push_back(tokenizeChar());
        } else if (c == '+' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::PLUS_EQ, "+=", line_, col_, start);
        } else if (c == '-' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::MINUS_EQ, "-=", line_, col_, start);
        } else if (c == '*' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::STAR_EQ, "*=", line_, col_, start);
        } else if (c == '/' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::SLASH_EQ, "/=", line_, col_, start);
        } else if (c == '-' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '>') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::ARROW, "->", line_, col_, start);
        } else if (c == '=' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::EQ, "==", line_, col_, start);
        } else if (c == '!' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::NEQ, "!=", line_, col_, start);
        } else if (c == '<' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::LTE, "<=", line_, col_, start);
        } else if (c == '>' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::GTE, ">=", line_, col_, start);
        } else if (c == '&' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '&') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::AND, "&&", line_, col_, start);
        } else if (c == '|' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '|') {
            advanceChar(); advanceChar();
            tokens_.emplace_back(TokenType::OR, "||", line_, col_, start);
        } else if (c == '*') {
            advanceChar();
            tokens_.emplace_back(TokenType::STAR, "*", line_, col_, start);
        } else if (c == '/') {
            advanceChar();
            tokens_.emplace_back(TokenType::SLASH, "/", line_, col_, start);
        } else if (c == '%') {
            advanceChar();
            tokens_.emplace_back(TokenType::MOD, "%", line_, col_, start);
        } else if (c == '=') {
            advanceChar();
            tokens_.emplace_back(TokenType::EQUAL, "=", line_, col_, start);
        } else if (c == '<') {
            advanceChar();
            tokens_.emplace_back(TokenType::LT, "<", line_, col_, start);
        } else if (c == '>') {
            advanceChar();
            tokens_.emplace_back(TokenType::GT, ">", line_, col_, start);
        } else if (c == '!') {
            advanceChar();
            tokens_.emplace_back(TokenType::NOT, "!", line_, col_, start);
        } else if (c == '+') {
            advanceChar();
            tokens_.emplace_back(TokenType::PLUS, "+", line_, col_, start);
        } else if (c == '-') {
            advanceChar();
            tokens_.emplace_back(TokenType::MINUS, "-", line_, col_, start);
        } else if (c == '&') {
            advanceChar();
            tokens_.emplace_back(TokenType::AMPERSAND, "&", line_, col_, start);
        } else if (c == ',') {
            advanceChar();
            tokens_.emplace_back(TokenType::COMMA, ",", line_, col_, start);
        } else if (c == ';') {
            advanceChar();
            tokens_.emplace_back(TokenType::SEMICOLON, ";", line_, col_, start);
        } else if (c == ':') {
            advanceChar();
            tokens_.emplace_back(TokenType::COLON, ":", line_, col_, start);
        } else if (c == '?') {
            advanceChar();
            tokens_.emplace_back(TokenType::QUESTION, "?", line_, col_, start);
        } else if (c == '|') {
            advanceChar();
            tokens_.emplace_back(TokenType::PIPE, "|", line_, col_, start);
        } else if (c == '.') {
            advanceChar();
            if (pos_ < source_.size() && source_[pos_] == '.') {
                advanceChar();
                tokens_.emplace_back(TokenType::DOTDOT, "..", line_, col_, start);
            } else {
                tokens_.emplace_back(TokenType::DOT, ".", line_, col_, start);
            }
        } else if (c == '{') {
            advanceChar();
            tokens_.emplace_back(TokenType::LBRACE, "{", line_, col_, start);
        } else if (c == '}') {
            advanceChar();
            tokens_.emplace_back(TokenType::RBRACE, "}", line_, col_, start);
        } else if (c == '[') {
            advanceChar();
            tokens_.emplace_back(TokenType::LBRACKET, "[", line_, col_, start);
        } else if (c == ']') {
            advanceChar();
            tokens_.emplace_back(TokenType::RBRACKET, "]", line_, col_, start);
        } else if (c == '(') {
            advanceChar();
            tokens_.emplace_back(TokenType::LPAREN, "(", line_, col_, start);
        } else if (c == ')') {
            advanceChar();
            tokens_.emplace_back(TokenType::RPAREN, ")", line_, col_, start);
        } else if (c == '@') {
            advanceChar();
            tokens_.emplace_back(TokenType::AT, "@", line_, col_, start);
        } else if (c == '~') {
            // skip tilde
            advanceChar();
        } else {
            advanceChar();
            diagnostics.push_back({line_, col_, "Unexpected character: '" + std::string(1, c) + "'", Diag::Level::Error});
            tokens_.emplace_back(TokenType::UNKNOWN, std::string(1, c), line_, col_, start);
        }
    }

    tokens_.emplace_back(TokenType::TOKEN_EOF, "", line_, col_, pos_);
    return tokens_;
}

Token Lexer::peek() const {
    if (tokens_.empty()) return Token();
    return tokens_.back();
}

Token Lexer::advance() {
    // Not used — tokens are pre-computed in tokenize()
    return Token();
}

bool Lexer::eof() const {
    if (tokens_.empty()) return true;
    return tokens_.back().type == TokenType::TOKEN_EOF;
}

} // namespace eng
