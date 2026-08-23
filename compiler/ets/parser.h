// EnginotechC++ — ETC Parser
// Parses .etc files into an AST (Abstract Syntax Tree)
//
// Supported Syntax:
//   print("string")
//   output "string"
//   say "string"
//   # comments
//   // comments
//   if condition { body }
//   for var in iterable { body }
//   while condition { body }

#pragma once
#include "ast.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include <sstream>
#include <algorithm>

namespace eng {
namespace ets {

class Parser {
public:
    struct Error {
        int line;
        int col;
        std::string message;
    };
    
    explicit Parser(const std::vector<Token>& tokens)
        : tokens_(tokens), pos_(0) {}
    
    ScriptProgram parse() {
        ScriptProgram prog;
        
        while (pos_ < tokens_.size() && tokens_[pos_].type != TokenType::TOKEN_EOF) {
            skipWhitespace();
            if (pos_ >= tokens_.size() || tokens_[pos_].type == TokenType::TOKEN_EOF) {
                break;
            }
            
            const auto& tok = tokens_[pos_];
            Span span = {tok.line, tok.col};
            
            if (tok.type == TokenType::PRINT) {
                pos_++;
                skipWhitespace();
                auto expr = parseExpression();
                prog.addStatement(std::make_shared<PrintStmt>(expr, span));
            }
            else if (tok.type == TokenType::OUTPUT) {
                pos_++;
                skipWhitespace();
                auto expr = parseExpression();
                prog.addStatement(std::make_shared<OutputStmt>(expr, span));
            }
            else if (tok.type == TokenType::SAY) {
                pos_++;
                skipWhitespace();
                auto expr = parseExpression();
                prog.addStatement(std::make_shared<SayStmt>(expr, span));
            }
            else if (tok.type == TokenType::IF) {
                pos_++;
                auto cond = parseExpression();
                skipWhitespace();
                auto thenBody = parseBlockOrSingle();
                auto elseBranch = parseElse();
                prog.addStatement(std::make_shared<IfStmt>(cond, thenBody, elseBranch, span));
            }
            else if (tok.type == TokenType::FOR) {
                pos_++;
                std::string varName = parseIdentifier();
                skipWhitespace();
                expect(TokenType::IN, "for loop");
                auto iterable = parseExpression();
                skipWhitespace();
                auto body = parseBlockOrSingle();
                prog.addStatement(std::make_shared<ForStmt>(varName, iterable, body, span));
            }
            else if (tok.type == TokenType::WHILE) {
                pos_++;
                auto cond = parseExpression();
                skipWhitespace();
                auto body = parseBlockOrSingle();
                prog.addStatement(std::make_shared<WhileStmt>(cond, body, span));
            }
            else if (tok.type == TokenType::RETURN) {
                pos_++;
                skipWhitespace();
                std::optional<ExprPtr> expr;
                if (pos_ < tokens_.size() && tokens_[pos_].type != TokenType::TOKEN_EOF) {
                    expr = parseExpression();
                }
                prog.addStatement(std::make_shared<ReturnStmt>(expr, span));
            }
            else if (tok.type == TokenType::BREAK) {
                pos_++;
                prog.addStatement(std::make_shared<BreakStmt>(span));
            }
            else if (tok.type == TokenType::CONTINUE) {
                pos_++;
                prog.addStatement(std::make_shared<ContinueStmt>(span));
            }
            else if (tok.type == TokenType::COMMENT) {
                prog.comments.push_back(tok.lexeme);
                pos_++;
            }
            else {
                // Try parsing as expression statement
                auto expr = parseExpression();
                if (expr) {
                    prog.addStatement(std::make_shared<ExprStmt>(expr, span));
                } else {
                    addError(tok.line, tok.col, "Unexpected token: " + tok.lexeme);
                    pos_++;
                }
            }
        }
        
        prog.totalTokens = static_cast<int>(tokens_.size());
        return prog;
    }
    
    const std::vector<Error>& getErrors() const { return errors_; }

private:
    std::vector<Token> tokens_;
    size_t pos_;
    std::vector<Error> errors_;
    
    void skipWhitespace() {
        while (pos_ < tokens_.size() && 
               (tokens_[pos_].type == TokenType::WHITESPACE ||
                tokens_[pos_].type == TokenType::NEWLINE)) {
            pos_++;
        }
    }
    
    const Token& current() const {
        if (pos_ >= tokens_.size()) {
            static Token eofToken(TokenType::TOKEN_EOF, "", 0, 0, 0);
            return eofToken;
        }
        return tokens_[pos_];
    }
    
    bool match(TokenType type) {
        if (current().type == type) {
            pos_++;
            return true;
        }
        return false;
    }
    
    bool expect(TokenType type, const std::string& context = "") {
        if (current().type == type) {
            pos_++;
            return true;
        }
        addError(current().line, current().col, 
                 "Expected '" + tokenTypeToStr(type) + "'" +
                 (context.empty() ? "" : " in " + context) +
                 ", got: " + current().lexeme);
        return false;
    }
    
    void addError(int line, int col, const std::string& msg) {
        errors_.push_back({line, col, msg});
    }
    
    std::string tokenTypeToStr(TokenType type) {
        switch (type) {
            case TokenType::LPAREN: return "(";
            case TokenType::RPAREN: return ")";
            case TokenType::LBRACE: return "{";
            case TokenType::RBRACE: return "}";
            case TokenType::STRING_LITERAL: return "string";
            case TokenType::INT_LITERAL: return "integer";
            case TokenType::IDENT: return "identifier";
            case TokenType::TOKEN_EOF: return "EOF";
            case TokenType::IN: return "in";
            case TokenType::ELSE: return "else";
            default: return "token";
        }
    }
    
    StmtPtr parseBlockOrSingle() {
        Span span = {current().line, current().col};
        
        if (match(TokenType::LBRACE)) {
            std::vector<StmtPtr> body;
            while (!match(TokenType::RBRACE) && current().type != TokenType::TOKEN_EOF) {
                skipWhitespace();
                if (current().type == TokenType::TOKEN_EOF) break;
                
                const auto& tok = current();
                Span stmtSpan = {tok.line, tok.col};
                
                if (tok.type == TokenType::PRINT) {
                    pos_++;
                    skipWhitespace();
                    body.push_back(std::make_shared<PrintStmt>(parseExpression(), stmtSpan));
                }
                else if (tok.type == TokenType::OUTPUT) {
                    pos_++;
                    skipWhitespace();
                    body.push_back(std::make_shared<OutputStmt>(parseExpression(), stmtSpan));
                }
                else if (tok.type == TokenType::SAY) {
                    pos_++;
                    skipWhitespace();
                    body.push_back(std::make_shared<SayStmt>(parseExpression(), stmtSpan));
                }
                else {
                    auto expr = parseExpression();
                    body.push_back(std::make_shared<ExprStmt>(expr, stmtSpan));
                }
            }
            return std::make_shared<BlockStmt>(std::move(body), span);
        }
        
        return parseStatement();
    }
    
    StmtPtr parseStatement() {
        Span span = {current().line, current().col};
        const auto& tok = current();
        
        if (tok.type == TokenType::PRINT) {
            pos_++;
            skipWhitespace();
            return std::make_shared<PrintStmt>(parseExpression(), span);
        }
        else if (tok.type == TokenType::OUTPUT) {
            pos_++;
            skipWhitespace();
            return std::make_shared<OutputStmt>(parseExpression(), span);
        }
        else if (tok.type == TokenType::SAY) {
            pos_++;
            skipWhitespace();
            return std::make_shared<SayStmt>(parseExpression(), span);
        }
        else {
            auto expr = parseExpression();
            return std::make_shared<ExprStmt>(expr, span);
        }
    }
    
    StmtPtr parseElse() {
        Span span = {current().line, current().col};
        if (!match(TokenType::ELSE)) return nullptr;
        return parseBlockOrSingle();
    }
    
    std::string parseIdentifier() {
        if (current().type == TokenType::IDENT) {
            std::string name = current().lexeme;
            pos_++;
            return name;
        }
        addError(current().line, current().col, "Expected identifier");
        return "";
    }
    
    // ── Expression Parsing (Precedence Climbing) ──
    
    ExprPtr parseExpression() {
        return parseExpression(0);
    }
    
    int getPrecedence(TokenType type) {
        switch (type) {
            case TokenType::OR: return 1;
            case TokenType::AND: return 2;
            case TokenType::EQ: case TokenType::NEQ: return 3;
            case TokenType::LT: case TokenType::GT: case TokenType::LTE: case TokenType::GTE: return 4;
            case TokenType::PLUS: case TokenType::MINUS: return 5;
            case TokenType::STAR: case TokenType::SLASH: case TokenType::MOD: return 6;
            default: return 0;
        }
    }
    
    ExprPtr parseExpression(int minPrec) {
        ExprPtr left = parseUnary();
        
        while (true) {
            int prec = getPrecedence(current().type);
            if (prec < minPrec) break;
            
            TokenType opType = current().type;
            pos_++;
            
            ExprPtr right = parseExpression(prec + 1);
            
            auto binOp = std::make_shared<BinaryOpExpr>(
                tokenTypeToBinOp(opType), left, right,
                makeSpan(left->span.line, left->span.col, right->span.endLine, right->span.endCol));
            
            left = binOp;
        }
        
        return left;
    }
    
    ExprPtr parseUnary() {
        if (current().type == TokenType::MINUS) {
            pos_++;
            auto operand = parseUnary();
            return std::make_shared<UnaryOpExpr>(UnaryOpExpr::UnOp::Negate, operand,
                                                  makeSpan(current().line, current().col));
        }
        if (current().type == TokenType::NOT) {
            pos_++;
            auto operand = parseUnary();
            return std::make_shared<UnaryOpExpr>(UnaryOpExpr::UnOp::Not, operand,
                                                  makeSpan(current().line, current().col));
        }
        return parsePrimary();
    }
    
    ExprPtr parsePrimary() {
        const Token& tok = current();
        Span span = {tok.line, tok.col};
        
        // String literal
        if (tok.type == TokenType::STRING_LITERAL) {
            pos_++;
            span.endLine = tok.line;
            span.endCol = tok.col + tok.lexeme.size();
            return std::make_shared<LiteralExpr>(LiteralExpr::LiteralKind::String, 
                                                  tok.lexeme, tok.lexeme, span);
        }
        
        // Integer literal
        if (tok.type == TokenType::INT_LITERAL) {
            pos_++;
            span.endLine = tok.line;
            span.endCol = tok.col + tok.lexeme.size();
            return std::make_shared<LiteralExpr>(LiteralExpr::LiteralKind::Integer,
                                                  tok.lexeme, tok.lexeme, span);
        }
        
        // Float literal
        if (tok.type == TokenType::FLOAT_LITERAL) {
            pos_++;
            span.endLine = tok.line;
            span.endCol = tok.col + tok.lexeme.size();
            return std::make_shared<LiteralExpr>(LiteralExpr::LiteralKind::Float,
                                                  tok.lexeme, tok.lexeme, span);
        }
        
        // Boolean
        if (tok.type == TokenType::TRUE) {
            pos_++;
            return std::make_shared<LiteralExpr>(LiteralExpr::LiteralKind::Bool,
                                                  "true", "true", span);
        }
        if (tok.type == TokenType::FALSE) {
            pos_++;
            return std::make_shared<LiteralExpr>(LiteralExpr::LiteralKind::Bool,
                                                  "false", "false", span);
        }
        
        // Identifier or function call
        if (tok.type == TokenType::IDENT) {
            pos_++;
            span.endCol = span.col + tok.lexeme.size();
            
            // Check for function call
            if (current().type == TokenType::LPAREN) {
                pos_++;
                std::vector<ExprPtr> args;
                while (current().type != TokenType::RPAREN && 
                       current().type != TokenType::TOKEN_EOF) {
                    args.push_back(parseExpression());
                    if (current().type == TokenType::COMMA) pos_++;
                }
                expect(TokenType::RPAREN, "function call");
                return std::make_shared<CallExpr>(tok.lexeme, std::move(args), span);
            }
            
            return std::make_shared<IdentExpr>(tok.lexeme, span);
        }
        
        // Parenthesized expression
        if (tok.type == TokenType::LPAREN) {
            pos_++;
            auto expr = parseExpression();
            expect(TokenType::RPAREN, "parenthesized expression");
            return expr;
        }
        
        addError(tok.line, tok.col, "Unexpected token: " + tok.lexeme);
        pos_++;
        return std::make_shared<LiteralExpr>(LiteralExpr::LiteralKind::None, "", "", span);
    }
    
    BinaryOpExpr::BinOp tokenTypeToBinOp(TokenType type) {
        switch (type) {
            case TokenType::PLUS: return BinaryOpExpr::BinOp::Add;
            case TokenType::MINUS: return BinaryOpExpr::BinOp::Sub;
            case TokenType::STAR: return BinaryOpExpr::BinOp::Mul;
            case TokenType::SLASH: return BinaryOpExpr::BinOp::Div;
            case TokenType::MOD: return BinaryOpExpr::BinOp::Mod;
            case TokenType::EQ: return BinaryOpExpr::BinOp::Eq;
            case TokenType::NEQ: return BinaryOpExpr::BinOp::Neq;
            case TokenType::LT: return BinaryOpExpr::BinOp::Lt;
            case TokenType::GT: return BinaryOpExpr::BinOp::Gt;
            case TokenType::LTE: return BinaryOpExpr::BinOp::LtEq;
            case TokenType::GTE: return BinaryOpExpr::BinOp::GtEq;
            case TokenType::AND: return BinaryOpExpr::BinOp::And;
            case TokenType::OR: return BinaryOpExpr::BinOp::Or;
            default: return BinaryOpExpr::BinOp::Add;
        }
    }
};

// ── Public API ────────────────────────────────────────────────────
inline ScriptProgram parseETC(const std::vector<Token>& tokens) {
    Parser parser(tokens);
    return parser.parse();
}

} // namespace ets
} // namespace eng
