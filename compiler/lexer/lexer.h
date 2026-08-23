#ifndef ENGLEXER_LEXER_H
#define ENGLEXER_LEXER_H

#include "token.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

namespace eng {

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

    Token peek() const;
    Token advance();
    bool eof() const;

    struct Diag {
        int line;
        int col;
        std::string message;
        enum class Level { Error, Warning } level;
    };

    std::vector<Diag> diagnostics;

private:
    char peekChar() const;
    char advanceChar();
    void skipWhitespace();
    void skipLineComment();
    void skipBlockComment();

    Token tokenizeIdentifier();
    Token tokenizeInteger();
    Token tokenizeFloat();
    Token tokenizeString();
    Token tokenizeChar();

    bool isIdentStart(char c) const;
    bool isIdentContinue(char c) const;
    bool isDigit(char c) const;
    bool isAlpha(char c) const;

    static const std::unordered_map<std::string, TokenType>& keywords();

    std::string source_;
    size_t pos_;
    int line_;
    int col_;
    std::vector<Token> tokens_;
};

} // namespace eng

#endif // ENGLEXER_LEXER_H
