#ifndef ENGLEXER_TOKEN_H
#define ENGLEXER_TOKEN_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <variant>
#include <optional>
#include <cstdint>
#include <sstream>
#include <iomanip>

namespace eng {

enum class TokenType : int {
    // Literals
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    BOOL_LITERAL,

    // Identifiers & keywords
    IDENT,
    FN,      // fn
    LET,     // let
    MUT,     // mut
    CONST,   // const
    IF,      // if
    ELSE,    // else
    FOR,     // for
    WHILE,   // while
    RETURN,  // return
    BREAK,   // break
    CONTINUE,// continue
    MATCH,   // match
    STRUCT,  // struct
    ENUM,    // enum
    IMPL,    // impl
    INTERFACE,// interface
    IMPORT,  // import
    PUB,     // pub
    AS,      // as
    IN,      // in
    ELIF,    // elif (Python-style else-if)
    TRUE,    // true
    FALSE,   // false
    NONE,    // none

    // Module management
    MODULE,  // module keyword
    EXPORT,  // export keyword
    LOAD,    // load module

    // Exception handling
    TRY,     // try
    CATCH,   // catch
    THROW,   // throw
    EXCEPT,  // except
    FINALLY, // finally

    // Bytecode
    BYTECODE, // bytecode keyword
    VM,       // vm keyword
    OP,       // opcode keyword

    // Testing framework
    TEST,      // test keyword
    ASSERT,    // assert keyword
    EXPECT,    // expect keyword

    // Design by Contract
    REQUIRES,  // requires
    ENSURES,   // ensures
    INVARIANT, // invariant

    // Hot reload
    HOT,       // hot keyword
    RELOAD,    // reload keyword

    // Coroutines
    COROUTINE, // coroutine
    YIELD,     // yield keyword
    AWAIT,     // await keyword

    // Dimensional types (physical units)
    LENGTH,    // length type unit
    TIME,      // time type unit
    MASS,      // mass type unit
    FORCE,     // force type unit

    // Types
    INT_T,     // int (alias)
    INT8,
    INT16,
    INT32,
    INT64,
    UINT,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    F32,       // float32
    F64,       // float64
    BOOL_T,    // bool
    CHAR_T,    // char
    STRING,    // string (type keyword)
    BYTE,
    VOID,
    OPTION,
    RESULT,

    // Operators
    PLUS,        // +
    MINUS,       // -
    STAR,        // *
    SLASH,       // /
    MOD,         // %
    EQUAL,       // =
    EQ,          // ==
    NEQ,         // !=
    LT,          // <
    GT,          // >
    LTE,         // <=
    GTE,         // >=
    AND,         // &&
    OR,          // ||
    NOT,         // !
    PLUS_EQ,     // +=
    MINUS_EQ,    // -=
    STAR_EQ,     // *=
    SLASH_EQ,    // /=
    ARROW,       // ->
    DOUBLE_ARROW,// =>
    DOT,         // .
    DOTDOT,      // .. (range)
    COMMA,       // ,
    SEMICOLON,   // ;
    COLON,       // :
    QUESTION,    // ?
    PIPE,        // |
    LBRACE,      // {
    RBRACE,      // }
    LBRACKET,    // [
    RBRACKET,    // ]
    LPAREN,      // (
    RPAREN,      // )
    AMPERSAND,   // &
    STAR_DEREF,  // * (dereference)
    AT,          // @
    ARROW_DOT,   // .> (struct field access)

    // ETC/Script keywords (for .etc files)
    PRINT,       // print (ETC script command)
    OUTPUT,      // output (alias for print)
    SAY,         // say (alias for print)
    INPUT,       // input (read from stdin)

    // Special
    TOKEN_EOF,
    NEWLINE,
    WHITESPACE,  // skipped but tracked
    COMMENT,     // skipped
    UNKNOWN,
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;
    size_t pos; // byte offset in source

    Token() : type(TokenType::TOKEN_EOF), line(0), col(0), pos(0) {}
    Token(TokenType t, const std::string& lex, int ln, int cl, size_t p = 0)
        : type(t), lexeme(lex), line(ln), col(cl), pos(p) {}
    Token(Token&& other) noexcept = default;
    Token& operator=(Token&& other) noexcept = default;
    Token(const Token&) = default;
    Token& operator=(const Token&) = default;

    std::string toString() const {
        std::ostringstream oss;
        oss << "Token(" << tokenName() << ", \"" << lexeme << "\", @" << line << ":" << col << ")";
        return oss.str();
    }

    std::string tokenName() const {
        static const std::map<TokenType, std::string> names = {
            {TokenType::INT_LITERAL, "INT"},
            {TokenType::FLOAT_LITERAL, "FLOAT"},
            {TokenType::STRING_LITERAL, "STRING"},
            {TokenType::CHAR_LITERAL, "CHAR"},
            {TokenType::BOOL_LITERAL, "BOOL"},
            {TokenType::IDENT, "IDENT"},
            {TokenType::FN, "FN"},
            {TokenType::LET, "LET"},
            {TokenType::MUT, "MUT"},
            {TokenType::CONST, "CONST"},
            {TokenType::IF, "IF"},
            {TokenType::ELSE, "ELSE"},
            {TokenType::FOR, "FOR"},
            {TokenType::WHILE, "WHILE"},
            {TokenType::RETURN, "RETURN"},
            {TokenType::BREAK, "BREAK"},
            {TokenType::CONTINUE, "CONTINUE"},
            {TokenType::MATCH, "MATCH"},
            {TokenType::STRUCT, "STRUCT"},
            {TokenType::ENUM, "ENUM"},
            {TokenType::IMPL, "IMPL"},
            {TokenType::INTERFACE, "INTERFACE"},
            {TokenType::IMPORT, "IMPORT"},
            {TokenType::PUB, "PUB"},
            {TokenType::AS, "AS"},
            {TokenType::IN, "IN"},
            {TokenType::ELIF, "ELIF"},
            {TokenType::TRUE, "TRUE"},
            {TokenType::FALSE, "FALSE"},
            {TokenType::NONE, "NONE"},
            {TokenType::INT_T, "INT"},
            {TokenType::INT8, "INT8"},
            {TokenType::INT16, "INT16"},
            {TokenType::INT32, "INT32"},
            {TokenType::INT64, "INT64"},
            {TokenType::UINT, "UINT"},
            {TokenType::UINT8, "UINT8"},
            {TokenType::UINT16, "UINT16"},
            {TokenType::UINT32, "UINT32"},
            {TokenType::UINT64, "UINT64"},
            {TokenType::F32, "FLOAT32"},
            {TokenType::F64, "FLOAT64"},
            {TokenType::BOOL_T, "BOOL"},
            {TokenType::CHAR_T, "CHAR"},
            {TokenType::STRING, "STRING"},
            {TokenType::BYTE, "BYTE"},
            {TokenType::VOID, "VOID"},
            {TokenType::OPTION, "OPTION"},
            {TokenType::RESULT, "RESULT"},
            {TokenType::BYTECODE, "bytecode"},
            {TokenType::VM, "vm"},
            {TokenType::OP, "op"},
            {TokenType::TEST, "test"},
            {TokenType::ASSERT, "assert"},
            {TokenType::EXPECT, "expect"},
            {TokenType::REQUIRES, "requires"},
            {TokenType::ENSURES, "ensures"},
            {TokenType::INVARIANT, "invariant"},
            {TokenType::HOT, "hot"},
            {TokenType::RELOAD, "reload"},
            {TokenType::COROUTINE, "coroutine"},
            {TokenType::YIELD, "yield"},
            {TokenType::AWAIT, "await"},
            {TokenType::LENGTH, "length"},
            {TokenType::TIME, "time"},
            {TokenType::MASS, "mass"},
            {TokenType::FORCE, "force"},
            {TokenType::PLUS, "+"},
            {TokenType::MINUS, "-"},
            {TokenType::STAR, "*"},
            {TokenType::SLASH, "/"},
            {TokenType::MOD, "%"},
            {TokenType::EQUAL, "="},
            {TokenType::EQ, "=="},
            {TokenType::NEQ, "!="},
            {TokenType::LT, "<"},
            {TokenType::GT, ">"},
            {TokenType::LTE, "<="},
            {TokenType::GTE, ">="},
            {TokenType::AND, "&&"},
            {TokenType::OR, "||"},
            {TokenType::NOT, "!"},
            {TokenType::PLUS_EQ, "+="},
            {TokenType::MINUS_EQ, "-="},
            {TokenType::STAR_EQ, "*="},
            {TokenType::SLASH_EQ, "/="},
            {TokenType::ARROW, "->"},
            {TokenType::DOUBLE_ARROW, "=>"},
            {TokenType::DOT, "."},
            {TokenType::DOTDOT, ".."},
            {TokenType::COMMA, ","},
            {TokenType::SEMICOLON, ";"},
            {TokenType::COLON, ":"},
            {TokenType::QUESTION, "?"},
            {TokenType::PIPE, "|"},
            {TokenType::LBRACE, "{"},
            {TokenType::RBRACE, "}"},
            {TokenType::LBRACKET, "["},
            {TokenType::RBRACKET, "]"},
            {TokenType::LPAREN, "("},
            {TokenType::RPAREN, ")"},
            {TokenType::AMPERSAND, "&"},
            {TokenType::STAR_DEREF, "DEREF"},
            {TokenType::ARROW_DOT, ".>"},
            {TokenType::PRINT, "PRINT"},
            {TokenType::OUTPUT, "OUTPUT"},
            {TokenType::SAY, "SAY"},
            {TokenType::INPUT, "INPUT"},
            {TokenType::TOKEN_EOF, "EOF"},
            {TokenType::NEWLINE, "NEWLINE"},
            {TokenType::WHITESPACE, "WS"},
            {TokenType::COMMENT, "COMMENT"},
            {TokenType::UNKNOWN, "UNKNOWN"},
        };
        auto it = names.find(type);
        return it != names.end() ? it->second : "UNKNOWN";
    }
};

} // namespace eng

#endif // ENGLEXER_TOKEN_H
