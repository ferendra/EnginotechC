#ifndef ENGPARSER_PARSER_H
#define ENGPARSER_PARSER_H

#include "../ast/nodes.h"
#include "../lexer/lexer.h"
#include "../diagnostics/diagnostic.h"
#include <stack>
#include <unordered_map>
#include <unordered_set>

namespace eng {

// Precedence constants for expression parsing
enum Precedence {
    P_UNARY = 8,
    P_NONE = 0,
    P_ASSIGN = 1,
    P_OR = 2,
    P_AND = 3,
    P_EQ = 4,
    P_CMP = 5,
    P_ADD = 6,
    P_MUL = 7
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens, DiagnosticEngine& diag);

    Program parse();

private:
    // ---- parsing helpers ----
    void skipComments();
    Token current() const;
    Token peekNext() const;
    bool match(TokenType type);
    bool expect(TokenType type, const std::string& context = "");
    void error(const std::string& code, const std::string& message);
    void errorAt(const Token& tok, const std::string& message);
    void warn(const std::string& code, const std::string& message);
    void consume(TokenType type, const std::string& context);
    void consumeExpected(const Token& tok, TokenType expected, const std::string& context);
    void synchronize();

    // ---- top-level ----
    std::vector<StmtPtr> parseTopLevel();
    StmtPtr parseTopLevelItem();

    // ---- declarations ----
    StmtPtr parseFunctionDecl();
    StmtPtr parseStructDecl();
    StmtPtr parseEnumDecl();
    StmtPtr parseImplBlock();
    StmtPtr parseInterfaceDecl();
    StmtPtr parseImportStmt();
    StmtPtr parseConstDecl();

    // ---- statements ----
    StmtPtr parseStatement();
    std::vector<StmtPtr> parseBlock();
    StmtPtr parseLetStmt();
    StmtPtr parseMutStmt();
    StmtPtr parseExprStmt();
    StmtPtr parseIfStmt();
    StmtPtr parseIfStmtElif();
    StmtPtr tryParseAssignment();
    StmtPtr parseForStmt();
    StmtPtr parseWhileStmt();
    StmtPtr parseReturnStmt();
    StmtPtr parseBreakStmt();
    StmtPtr parseContinueStmt();
    StmtPtr parseMatchStmt();

    // ---- module management ----
    StmtPtr parseModuleDecl();
    StmtPtr parseExportDecl();
    StmtPtr parseLoadStmt();

    // ---- exception handling ----
    StmtPtr parseTryCatchStmt();
    StmtPtr parseThrowStmt();

    // ---- bytecode / vm ----
    StmtPtr parseBytecodeStmt();
    StmtPtr parseOpStmt();

    // ---- TESTING FRAMEWORK ----
    StmtPtr parseTestStmt();
    StmtPtr parseAssertStmt();
    StmtPtr parseExpectStmt();

    // ---- DESIGN BY CONTRACT ----
    StmtPtr parseRequiresStmt();
    StmtPtr parseEnsuresStmt();
    StmtPtr parseInvariantStmt();

    // ---- COROUTINES ----
    StmtPtr parseCoroutineDecl();
    ExprPtr parseYieldExpr();
    ExprPtr parseAwaitExpr();

    // ---- HOT RELOAD ----
    StmtPtr parseHotReloadStmt();

    // ---- PATTERNS ----
    ExprPtr parsePatternMatchExpr();

    // ---- MACROS ----
    StmtPtr parseMacroDecl();

    // ---- DIMENSIONAL TYPES ----
    ExprPtr parseDimensionalTypeExpr();

    // ---- expressions (precedence climbing) ----
    ExprPtr parseExpression();
    ExprPtr parseExpression(int minPrecedence);
    ExprPtr parsePrimary();
    ExprPtr parsePostfix(ExprPtr base);
    ExprPtr parseAtom();
    ExprPtr parseStringInterpolation();
    ExprPtr parseArrayLiteral();
    std::string parseIdentifier();
    int precedenceFor(TokenType t) const;

    // ---- types ----
    TypePtr parseType();
    TypePtr parseSimpleType();
    TypePtr parseGenericType();
    std::vector<std::pair<std::string, TypePtr>> parseParamList();

    // ---- scopes ----
    void pushScope();
    void popScope();
    void declareVariable(const std::string& name, TypePtr type, bool isMut);
    bool variableExists(const std::string& name);
    bool isTopLevel() const;

    // ---- classification ----
    ClassifyResult classifyLiteral(const Token& tok);
    bool isIdentifierStart(char c) const;
    bool isIdentifierChar(char c) const;
    bool isDigit(char c) const;
    bool isUnaryOp(TokenType t) const;
    bool isBinaryOp(TokenType t) const;

    // ---- state ----
    bool isInFunction() const;
    bool isInLoop() const;
    
    Program program_;
    bool currentIsMain_;
    std::vector<bool> scopeIsFunction_;
    std::vector<bool> scopeIsLoop_;
    std::vector<std::unordered_set<std::string>> scopes_;
    std::vector<Token> tokens_;
    size_t pos_;
    DiagnosticEngine& diag_;
};

} // namespace eng

#endif // ENGPARSER_PARSER_H
