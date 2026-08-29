#include "parser.h"
#include <algorithm>
#include <cassert>
#include <cstdio>

namespace eng {

Parser::Parser(const std::vector<Token>& tokens, DiagnosticEngine& diag)
    : diag_(diag), tokens_(tokens), pos_(0) {
    // Drop comment tokens up front: several parse routines do raw pos_ arithmetic
    // after reading current(), which would otherwise consume a COMMENT instead of
    // the token it inspected (e.g. "let x = 1;\n// c\nprint(x);" mis-parsing print).
    std::cerr << "Parser: constructing with " << tokens_.size() << " tokens" << std::endl; std::cerr.flush();
    tokens_.erase(std::remove_if(tokens_.begin(), tokens_.end(),
                                 [](const Token& t) { return t.type == TokenType::COMMENT; }),
                  tokens_.end());
    pushScope();
    scopeIsFunction_.back() = false;
    scopeIsLoop_.back() = false;
}

Token Parser::current() const {
    size_t p = pos_;
    while (p < tokens_.size() && tokens_[p].type == TokenType::COMMENT) p++;
    if (p < tokens_.size()) {
        auto& t = tokens_[p];
        return t;
    }
    return Token(TokenType::TOKEN_EOF, "", 0, 0, 0);
}

Token Parser::peekNext() const {
    size_t p = pos_ + 1;
    while (p < tokens_.size() && tokens_[p].type == TokenType::COMMENT) p++;
    if (p < tokens_.size()) return tokens_[p];
    return Token(TokenType::TOKEN_EOF, "", 0, 0, 0);
}

void Parser::skipComments() {
    while (pos_ < tokens_.size() && tokens_[pos_].type == TokenType::COMMENT) pos_++;
}

bool Parser::match(TokenType type) {
    size_t p = pos_;
    while (p < tokens_.size() && tokens_[p].type == TokenType::COMMENT) p++;
    if (p < tokens_.size() && tokens_[p].type == type) {
        pos_ = p + 1;
        return true;
    }
    return false;
}

bool Parser::expect(TokenType type, const std::string& context) {
    size_t p = pos_;
    while (p < tokens_.size() && tokens_[p].type == TokenType::COMMENT) p++;
    if (p < tokens_.size() && tokens_[p].type == type) {
        pos_ = p + 1;
        return true;
    }
    size_t errPos = (p < tokens_.size()) ? p : pos_;
    std::string msg = "Expected " + Token(type, "", 0, 0).tokenName();
    if (!context.empty()) msg += " in " + context;
    errorAt(tokens_[errPos], msg);
    return false;
}

void Parser::error(const std::string& code, const std::string& message) {
    diag_.error(code, message, current().line, current().col);
}

void Parser::warn(const std::string& code, const std::string& message) {
    diag_.warning(code, message, current().line, current().col);
}

void Parser::errorAt(const Token& tok, const std::string& message) {
    diag_.error("E0001", message, tok.line, tok.col);
}

void Parser::consumeExpected(const Token& tok, TokenType expected, const std::string& context) {
    if (tok.type != expected) {
        errorAt(tok, "Expected " + std::to_string(static_cast<int>(expected)) +
              (context.empty() ? "" : " in " + context));
    }
}

void Parser::synchronize() {
    while (current().type != TokenType::TOKEN_EOF) {
        if (current().type == TokenType::SEMICOLON) {
            pos_++; // consume ';' — resync point
            return;
        }
        if (current().type == TokenType::RBRACE) {
            return; // let enclosing block handle it; do NOT consume
        }
        if (current().type == TokenType::FN || current().type == TokenType::STRUCT ||
            current().type == TokenType::ENUM || current().type == TokenType::LET ||
            current().type == TokenType::MUT || current().type == TokenType::CONST ||
            current().type == TokenType::IMPORT || current().type == TokenType::IMPL ||
            current().type == TokenType::MODULE || current().type == TokenType::FN ||
            current().type == TokenType::STRUCT || current().type == TokenType::ENUM ||
            current().type == TokenType::IF || current().type == TokenType::WHILE ||
            current().type == TokenType::FOR || current().type == TokenType::RETURN) {
            return; // statement boundary — resume parsing from here
        }
        pos_++;
    }
}

Program Parser::parse() {
    program_.topLevel = parseTopLevel();
    return program_;
}

std::vector<StmtPtr> Parser::parseTopLevel() {
    std::vector<StmtPtr> items;
    while (current().type != TokenType::TOKEN_EOF) {
        size_t before = pos_;
        auto item = parseTopLevelItem();
        if (item) items.push_back(item);
        // Resilience: a failed top-level item must not abort the rest of the
        // file. Sync to the next declaration boundary and keep collecting.
        if (pos_ == before || !item) {
            if (current().type != TokenType::TOKEN_EOF) synchronize();
            if (pos_ == before && current().type != TokenType::TOKEN_EOF) pos_++;
        }
    }
    return items;
}

StmtPtr Parser::parseTopLevelItem() {
    Token t = current();
    // Human-friendly alias: `function` behaves exactly like `fn`
    if (t.type == TokenType::IDENT && t.lexeme == "function")
        return parseFunctionDecl();
    // Human-friendly statements (say/ask/set/repeat/give) at top level too,
    // but keyword followed by '(' is a normal call — leave it to the switch.
    bool parenFollows = (peekNext().type == TokenType::LPAREN);
    if (!parenFollows && (t.type == TokenType::SAY || t.type == TokenType::PRINT ||
        t.type == TokenType::OUTPUT || t.type == TokenType::INPUT))
        return parseStatement();
    if (t.type == TokenType::IDENT) {
        const std::string& w = t.lexeme;
        if (w == "say" || w == "ask" || w == "set" || w == "repeat" || w == "give")
            return parseStatement();
    }
    switch (t.type) {
        case TokenType::FN: return parseFunctionDecl();
        case TokenType::STRUCT: return parseStructDecl();
        case TokenType::ENUM: return parseEnumDecl();
        case TokenType::IMPL: return parseImplBlock();
        case TokenType::INTERFACE: return parseInterfaceDecl();
        case TokenType::IMPORT: return parseImportStmt();
        case TokenType::CONST: return parseConstDecl();
        case TokenType::LET: return parseLetStmt();
        case TokenType::MUT: return parseMutStmt();
        case TokenType::IF: return parseIfStmt();
        case TokenType::FOR: return parseForStmt();
        case TokenType::WHILE: return parseWhileStmt();
        case TokenType::RETURN: return parseReturnStmt();
        case TokenType::BREAK: return parseBreakStmt();
        case TokenType::CONTINUE: return parseContinueStmt();
        case TokenType::MATCH: return parseMatchStmt();
        case TokenType::MODULE: return parseModuleDecl();
        case TokenType::EXPORT: return parseExportDecl();
        case TokenType::LOAD: return parseLoadStmt();
        case TokenType::TRY: return parseTryCatchStmt();
        case TokenType::THROW: return parseThrowStmt();
        case TokenType::BYTECODE: return parseBytecodeStmt();
        case TokenType::OP: return parseOpStmt();
        case TokenType::TEST: return parseTestStmt();
        case TokenType::ASSERT: return parseAssertStmt();
        case TokenType::EXPECT: return parseExpectStmt();
        case TokenType::REQUIRES: return parseRequiresStmt();
        case TokenType::ENSURES: return parseEnsuresStmt();
        case TokenType::INVARIANT: return parseInvariantStmt();
        case TokenType::COROUTINE: return parseCoroutineDecl();
        case TokenType::HOT: return parseHotReloadStmt();
        case TokenType::PUB:
            pos_++;
            if (current().type == TokenType::FN) return parseFunctionDecl();
            if (current().type == TokenType::STRUCT) return parseStructDecl();
            errorAt(current(), "Expected fn or struct after pub");
            pos_++;
            return nullptr;
        default: {
            // assignment at top level (script mode): x = ..., arr[i] = ...
            if (t.type == TokenType::IDENT) {
                if (StmtPtr as = tryParseAssignment()) return as;
            }
            // expression statement at top level
            auto e = parseExpression();
            if (e) {
                if (current().type == TokenType::SEMICOLON) pos_++;
                return std::make_shared<ExprStmt>(e, t.line, t.col);
            }
            errorAt(t, "Unexpected token at top level: " + t.lexeme);
            pos_++;
            return nullptr;
        }
    }
}

StmtPtr Parser::tryParseAssignment() {
    const Token t = current();
    size_t save = pos_;
    pos_++;
    ExprPtr target = std::make_shared<IdentExpr>(t.lexeme, t.line, t.col);
    bool complexTarget = false;
    while (true) {
        if (current().type == TokenType::DOT && peekNext().type == TokenType::IDENT) {
            pos_++;
            std::string field = current().lexeme;
            pos_++;
            target = std::make_shared<FieldAccessExpr>(target, field,
                                                       current().line, current().col);
            complexTarget = true;
        } else if (current().type == TokenType::LBRACKET) {
            pos_++;
            ExprPtr idx = parseExpression();
            if (!expect(TokenType::RBRACKET, "index")) { pos_ = save; return nullptr; }
            target = std::make_shared<IndexExpr>(target, idx, current().line, current().col);
            complexTarget = true;
        } else {
            break;
        }
    }
    if (current().type == TokenType::EQUAL) {
        pos_++; // skip =
        auto val = parseExpression();
        if (current().type == TokenType::SEMICOLON) pos_++;
        if (complexTarget) {
            return std::make_shared<AssignStmt>(target, val, t.line, t.col);
        }
        return std::make_shared<AssignStmt>(t.lexeme, val, t.line, t.col);
    }
    // Compound assignment: x += e  →  x = x + e (same for -= *= /=)
    static const std::map<TokenType, TokenType> compound = {
        {TokenType::PLUS_EQ, TokenType::PLUS},
        {TokenType::MINUS_EQ, TokenType::MINUS},
        {TokenType::STAR_EQ, TokenType::STAR},
        {TokenType::SLASH_EQ, TokenType::SLASH},
    };
    auto cit = compound.find(current().type);
    if (cit != compound.end()) {
        pos_++; // skip op=
        auto rhs = parseExpression();
        ExprPtr bin = std::make_shared<BinaryOpExpr>(
            cit->second, target, rhs, t.line, t.col);
        if (current().type == TokenType::SEMICOLON) pos_++;
        if (complexTarget) {
            return std::make_shared<AssignStmt>(target, bin, t.line, t.col);
        }
        return std::make_shared<AssignStmt>(t.lexeme, bin, t.line, t.col);
    }
    // Not an assignment — restore and let the caller fall through to expression
    pos_ = save;
    return nullptr;
}

std::vector<TypePtr> Parser::parseTypeParams() {
    std::vector<TypePtr> params;
    if (current().type != TokenType::LT) return params;
    pos_++; // skip <
    while (current().type != TokenType::GT && current().type != TokenType::TOKEN_EOF) {
        if (current().type == TokenType::IDENT) {
            std::string name = parseIdentifier();
            params.push_back(std::make_shared<TypeParam>(name));
        } else {
            errorAt(current(), "Expected type parameter name");
            pos_++;
        }
        if (current().type == TokenType::COMMA) pos_++;
    }
    if (current().type == TokenType::GT) pos_++; // skip >
    return params;
}

StmtPtr Parser::parseFunctionDecl() {
    Token fnTok = current();
    // Human-friendly alias: `function` is accepted wherever `fn` is expected
    if (fnTok.type == TokenType::IDENT && fnTok.lexeme == "function") {
        pos_++;
        fnTok = current();
    } else {
        expect(TokenType::FN, "function declaration");
    }
    std::string name = parseIdentifier();
    std::vector<TypePtr> typeParams = parseTypeParams();
    bool isMain = (name == "main");
    if (isMain) currentIsMain_ = true;

    if (!expect(TokenType::LPAREN, "function parameters")) return nullptr;

    std::vector<std::pair<std::string, TypePtr>> params;
    while (current().type != TokenType::RPAREN && current().type != TokenType::TOKEN_EOF) {
        std::string pname = parseIdentifier();
        if (current().type == TokenType::COLON) {
            pos_++;
            TypePtr ptype = parseType();
            params.emplace_back(pname, ptype);
        } else {
            params.emplace_back(pname, nullptr);
        }
        if (current().type == TokenType::COMMA) pos_++;
        else if (current().type == TokenType::RPAREN) break;
        else {
            errorAt(current(), "Expected ',' or ')' in function parameters");
            break;
        }
    }
    expect(TokenType::RPAREN, "function parameters");

    TypePtr retType = nullptr;
    if (current().type == TokenType::ARROW) {
        pos_++;
        retType = parseType();
    }

    auto body = parseBlock();

    if (isMain) currentIsMain_ = false;
    auto fn = std::make_shared<FunctionDecl>(name, retType, params, body, fnTok.line, fnTok.col);
    fn->typeParams = std::move(typeParams);
    return fn;
}

StmtPtr Parser::parseStructDecl() {
    Token stTok = current();
    expect(TokenType::STRUCT, "struct declaration");
    std::string name = parseIdentifier();
    std::vector<TypePtr> typeParams = parseTypeParams();
    if (!expect(TokenType::LBRACE, "struct body")) return nullptr;

    std::vector<std::pair<std::string, TypePtr>> fields;
    std::vector<StmtPtr> methods;

    // Parse mixed body: fields AND/OR methods in any order
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        if (current().type == TokenType::FN) {
            // Method definition inside struct
            auto fnDecl = parseFunctionDecl();
            if (fnDecl) {
                methods.push_back(fnDecl);
            } else {
                // Error recovery: skip token
                pos_++;
            }
        } else {
            // Field definition
            std::string fname = parseIdentifier();
            if (current().type == TokenType::COLON) {
                pos_++;
                TypePtr ftype = parseType();
                fields.emplace_back(fname, ftype);
            } else {
                errorAt(current(), "Expected ': type' after field name");
            }
            if (current().type == TokenType::SEMICOLON) pos_++;
            else if (current().type == TokenType::COMMA) pos_++;
            else if (current().type != TokenType::RBRACE) {
                errorAt(current(), "Expected ';' or ',' between struct members");
            }
        }
    }
    expect(TokenType::RBRACE, "struct body");

    auto st = std::make_shared<StructDecl>(name, fields, methods, stTok.line, stTok.col);
    st->typeParams = std::move(typeParams);
    return st;
}

StmtPtr Parser::parseEnumDecl() {
    Token enTok = current();
    expect(TokenType::ENUM, "enum declaration");
    std::string name = parseIdentifier();
    std::vector<TypePtr> typeParams = parseTypeParams();
    if (!expect(TokenType::LBRACE, "enum body")) return nullptr;

    std::vector<std::pair<std::string, std::vector<TypePtr>>> variants;
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        std::string vname = parseIdentifier();
        std::vector<TypePtr> payload;
        if (current().type == TokenType::LPAREN) {
            pos_++;
            while (current().type != TokenType::RPAREN && current().type != TokenType::TOKEN_EOF) {
                TypePtr t = parseType();
                if (t) payload.push_back(t);
                if (current().type == TokenType::COMMA) pos_++;
                else if (current().type != TokenType::RPAREN) break;
            }
            expect(TokenType::RPAREN, "enum variant payload");
        }
        variants.emplace_back(vname, payload);
        if (current().type == TokenType::COMMA) pos_++;
        else if (current().type != TokenType::RBRACE) {
            errorAt(current(), "Expected ',' or '}' in enum body");
            break;
        }
    }
    expect(TokenType::RBRACE, "enum body");

    auto en = std::make_shared<EnumDecl>(name, variants, enTok.line, enTok.col);
    en->typeParams = std::move(typeParams);
    return en;
}

StmtPtr Parser::parseImplBlock() {
    Token imTok = current();
    expect(TokenType::IMPL, "impl block");
    std::string structName = parseIdentifier();
    std::vector<TypePtr> typeParams = parseTypeParams();
    if (!expect(TokenType::LBRACE, "impl body")) return nullptr;

    std::vector<StmtPtr> methods;
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        if (current().type == TokenType::FN) {
            methods.push_back(parseFunctionDecl());
        } else {
            errorAt(current(), "Only fn declarations allowed in impl");
            pos_++;
        }
    }
    expect(TokenType::RBRACE, "impl body");

    auto im = std::make_shared<ImplDecl>(structName, methods, imTok.line, imTok.col);
    im->typeParams = std::move(typeParams);
    return im;
}

StmtPtr Parser::parseInterfaceDecl() {
    Token inTok = current();
    expect(TokenType::INTERFACE, "interface declaration");
    std::string name = parseIdentifier();
    if (!expect(TokenType::LBRACE, "interface body")) return nullptr;
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        pos_++;
    }
    expect(TokenType::RBRACE, "interface body");
    // Not fully implemented yet — but parse error-free
    return std::make_shared<ImplDecl>(name, std::vector<StmtPtr>{});
}

StmtPtr Parser::parseImportStmt() {
    Token imTok = current();
    expect(TokenType::IMPORT, "import statement");
    std::string path;
    // M3: preserve dots so "std.io" resolves as a module path
    bool lastWasDot = true; // start true so a leading dot would be an error later anyway
    while (current().type == TokenType::IDENT || current().type == TokenType::DOT) {
        if (current().type == TokenType::DOT) {
            if (lastWasDot) { errorAt(current(), "Malformed import path"); }
            path += ".";
            lastWasDot = true;
        } else {
            path += current().lexeme;
            lastWasDot = false;
        }
        pos_++;
    }
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<ImportStmt>(path, imTok.line, imTok.col);
}

StmtPtr Parser::parseConstDecl() {
    Token coTok = current();
    expect(TokenType::CONST, "const declaration");
    std::string name = parseIdentifier();
    if (!expect(TokenType::EQUAL, "const initializer")) return nullptr;
    auto init = parseExpression();
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<ConstStmt>(name, init, coTok.line, coTok.col);
}

StmtPtr Parser::parseStatement() {
    Token t = current();
    // ── Human-friendly syntax (v0.4.0) ──────────────────────────────
    // These read like plain language; each maps onto existing AST.
    if (t.type == TokenType::IDENT || t.type == TokenType::SAY ||
        t.type == TokenType::PRINT || t.type == TokenType::OUTPUT ||
        t.type == TokenType::INPUT) {
        // function ...  →  fn ... (alias, works anywhere)
        if (t.type == TokenType::IDENT && t.lexeme == "function")
            return parseFunctionDecl();
        // give <expr>;  /  give;  →  return
        if (t.type == TokenType::IDENT && t.lexeme == "give") {
            pos_++;
            std::optional<ExprPtr> val;
            if (current().type != TokenType::SEMICOLON &&
                current().type != TokenType::RBRACE &&
                current().type != TokenType::TOKEN_EOF) {
                val = parseExpression();
            }
            if (current().type == TokenType::SEMICOLON) pos_++;
            return std::make_shared<ReturnStmt>(val, t.line, t.col);
        }
        // say <expr>; / print <expr>; / output <expr>;  →  print(<expr>);
        // (only when NOT followed by '(' — that's a normal call expression)
        bool parenNext = (peekNext().type == TokenType::LPAREN);
        if (!parenNext && (t.type == TokenType::SAY || t.type == TokenType::PRINT ||
            t.type == TokenType::OUTPUT ||
            (t.type == TokenType::IDENT && t.lexeme == "say"))) {
            pos_++;
            ExprPtr arg = parseExpression();
            if (current().type == TokenType::SEMICOLON) pos_++;
            auto callee = std::make_shared<IdentExpr>("print", t.line, t.col);
            std::vector<ExprPtr> args{arg};
            auto call = std::make_shared<CallExpr>(callee, args, t.line, t.col);
            return std::make_shared<ExprStmt>(call, t.line, t.col);
        }
        // ask <id>;  →  mut id = input();
        // (input(...) with parens stays a normal call)
        bool askParen = (peekNext().type == TokenType::LPAREN);
        if (!askParen && (t.type == TokenType::INPUT ||
            (t.type == TokenType::IDENT && t.lexeme == "ask"))) {
            pos_++;
            std::string name = parseIdentifier();
            if (current().type == TokenType::SEMICOLON) pos_++;
            auto callee = std::make_shared<IdentExpr>("input", t.line, t.col);
            std::vector<ExprPtr> args;
            auto call = std::make_shared<CallExpr>(callee, args, t.line, t.col);
            return std::make_shared<MutStmt>(name, nullptr, call, t.line, t.col);
        }
        // set <id> = <expr>;   |   set <id> to <expr>;
        // Declares a mutable variable (or re-assigns later via plain `=`).
        if (t.lexeme == "set") {
            pos_++;
            std::string name = parseIdentifier();
            bool toWord = (current().type == TokenType::IDENT && current().lexeme == "to");
            if (toWord) {
                pos_++;                       // accept the English word "to"
            } else if (current().type == TokenType::EQUAL) {
                pos_++;                       // also accept '='
            } else {
                errorAt(current(), "Expected '=' or 'to' after 'set <name>'");
            }
            ExprPtr val = parseExpression();
            if (current().type == TokenType::SEMICOLON) pos_++;
            return std::make_shared<MutStmt>(name, nullptr, val, t.line, t.col);
        }
        // repeat <expr> times <block>;  →  for __i in 0..n { ... }
        if (t.lexeme == "repeat") {
            pos_++;
            ExprPtr count = parseExpression(P_ADD);
            // Accept the plural marker word "times" when present
            if (current().type == TokenType::IDENT && current().lexeme == "times")
                pos_++;
            auto body = parseStatement();     // handles braces itself
            ClassifyResult zeroVal;
            zeroVal.kind = ClassifyResult::Integer;
            ExprPtr zero = std::make_shared<LiteralExpr>(zeroVal, "0", t.line, t.col);
            ExprPtr range = std::make_shared<RangeExpr>(zero, count, false, t.line, t.col);
            return std::make_shared<ForStmt>("__i", range, body, t.line, t.col);
        }
    }
    switch (t.type) {
        case TokenType::FN: return parseFunctionDecl();
        case TokenType::STRUCT: return parseStructDecl();
        case TokenType::ENUM: return parseEnumDecl();
        case TokenType::IMPL: return parseImplBlock();
        case TokenType::LET: return parseLetStmt();
        case TokenType::MUT: return parseMutStmt();
        case TokenType::CONST: return parseConstDecl();
        case TokenType::IF: return parseIfStmt();
        case TokenType::FOR: return parseForStmt();
        case TokenType::WHILE: return parseWhileStmt();
        case TokenType::RETURN: return parseReturnStmt();
        case TokenType::BREAK: return parseBreakStmt();
        case TokenType::CONTINUE: return parseContinueStmt();
        case TokenType::MATCH: return parseMatchStmt();
        case TokenType::MODULE: return parseModuleDecl();
        case TokenType::EXPORT: return parseExportDecl();
        case TokenType::LOAD: return parseLoadStmt();
        case TokenType::TRY: return parseTryCatchStmt();
        case TokenType::THROW: return parseThrowStmt();
        case TokenType::BYTECODE: return parseBytecodeStmt();
        case TokenType::OP: return parseOpStmt();
        case TokenType::TEST: return parseTestStmt();
        case TokenType::ASSERT: return parseAssertStmt();
        case TokenType::EXPECT: return parseExpectStmt();
        case TokenType::REQUIRES: return parseRequiresStmt();
        case TokenType::ENSURES: return parseEnsuresStmt();
        case TokenType::INVARIANT: return parseInvariantStmt();
        case TokenType::HOT: return parseHotReloadStmt();
        case TokenType::LBRACE: {
             auto b = parseBlock();
             return std::make_shared<BlockStmt>(b, current().line, current().col);
         }
        default: {
            // Could be a match expression, function call, or expression statement
            if (t.type == TokenType::IDENT && peekNext().type == TokenType::LPAREN) {
                return parseExprStmt(); // handles fn call
            }
            return parseExprStmt();
        }
    }
}

std::vector<StmtPtr> Parser::parseBlock() {
    Token lb = current();
    expect(TokenType::LBRACE, "block");
    std::vector<StmtPtr> body;
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        size_t before = pos_;
        auto s = parseStatement();
        if (s) body.push_back(s);
        if (current().type == TokenType::SEMICOLON) pos_++;
        // Resilience: a statement that consumed nothing (error path) must not
        // stall the loop or kill the rest of the block. Sync and continue so
        // one bad line reports one error instead of ending the parse.
        if (pos_ == before && current().type != TokenType::RBRACE &&
            current().type != TokenType::TOKEN_EOF) {
            synchronize();
            if (pos_ == before) pos_++; // absolute progress guarantee
        }
    }
    expect(TokenType::RBRACE, "block");
    return body;
}

StmtPtr Parser::parseLetStmt() {
    Token leTok = current();
    expect(TokenType::LET, "let statement");
    // `let mut x = ...` is accepted sugar for `mut x = ...`
    if (current().type == TokenType::MUT) {
        pos_++;
        std::string mname = parseIdentifier();
        TypePtr mtype = nullptr;
        if (current().type == TokenType::COLON) {
            pos_++;
            mtype = parseType();
        }
        ExprPtr minit = nullptr;
        if (current().type == TokenType::EQUAL) {
            pos_++;
            minit = parseExpression();
        }
        if (current().type == TokenType::SEMICOLON) pos_++;
        return std::make_shared<MutStmt>(mname, mtype, minit, leTok.line, leTok.col);
    }
    std::string name = parseIdentifier();
    TypePtr type = nullptr;
    int typeLine = 0, typeCol = 0;
    if (current().type == TokenType::COLON) {
        typeLine = current().line;
        typeCol = current().col;
        pos_++;
        type = parseType();
    }
    ExprPtr init = nullptr;
    int initLine = 0, initCol = 0;
    if (current().type == TokenType::EQUAL) {
        initLine = current().line;
        initCol = current().col;
        pos_++;
        init = parseExpression();
        if (init) {
            initLine = init->line;
            initCol = init->col;
        }
    }
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<LetStmt>(name, type, init, leTok.line, leTok.col, initLine, initCol);
}

StmtPtr Parser::parseMutStmt() {
    Token muTok = current();
    expect(TokenType::MUT, "mut statement");
    std::string name = parseIdentifier();
    TypePtr type = nullptr;
    if (current().type == TokenType::COLON) {
        pos_++;
        type = parseType();
    }
    ExprPtr init = nullptr;
    if (current().type == TokenType::EQUAL) {
        pos_++;
        init = parseExpression();
    }
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<MutStmt>(name, type, init, muTok.line, muTok.col);
}

StmtPtr Parser::parseExprStmt() {
    Token t = current();
    // `loop { ... }` — infinite loop construct (embedded-friendly sugar)
    if (t.type == TokenType::IDENT && t.lexeme == "loop" &&
        peekNext().type == TokenType::LBRACE) {
        pos_++; // consume 'loop'; parseBlock() will consume '{'
        auto body = std::make_shared<BlockStmt>(parseBlock(), t.line, t.col);
        ClassifyResult ctrue;
        ctrue.kind = ClassifyResult::Bool;
        ctrue.intVal = 1;
        auto cond = std::make_shared<LiteralExpr>(ctrue, "true", t.line, t.col);
        return std::make_shared<WhileStmt>(cond, body, t.line, t.col);
    }
    // Check for assignment: IDENT =, field path obj.f = value, or index x[i] = value
    if (t.type == TokenType::IDENT) {
        if (StmtPtr as = tryParseAssignment()) return as;
    }
    auto e = parseExpression();
    if (current().type == TokenType::SEMICOLON) pos_++;
    if (!e) {
        // Unparseable expression (recovery path): emit an error so the
        // statement never becomes a silent null-expr carrier downstream.
        errorAt(t, "Expected expression");
        return nullptr;
    }
    return std::make_shared<ExprStmt>(e, t.line, t.col);
}

StmtPtr Parser::parseIfStmt() {
    Token ifTok = current();
    expect(TokenType::IF, "if statement");
    ExprPtr cond = parseExpression();
    StmtPtr thenBr = parseStatement(); // already handles braces
    StmtPtr elseBr = nullptr;
    if (current().type == TokenType::ELIF) {
        // Python-style elif: desugar to a nested IfStmt in the else branch
        elseBr = parseIfStmtElif();
    } else if (current().type == TokenType::ELSE) {
        pos_++;
        elseBr = parseStatement(); // handles `else if ...` and `else { ... }`
    }
    return std::make_shared<IfStmt>(cond, thenBr, elseBr, ifTok.line, ifTok.col);
}

// Parses the tail of an elif chain (the ELIF token is current).
StmtPtr Parser::parseIfStmtElif() {
    Token ifTok = current();
    pos_++; // consume 'elif'
    ExprPtr cond = parseExpression();
    StmtPtr thenBr = parseStatement();
    StmtPtr elseBr = nullptr;
    if (current().type == TokenType::ELIF) {
        elseBr = parseIfStmtElif();
    } else if (current().type == TokenType::ELSE) {
        pos_++;
        elseBr = parseStatement();
    }
    return std::make_shared<IfStmt>(cond, thenBr, elseBr, ifTok.line, ifTok.col);
}

StmtPtr Parser::parseForStmt() {
    Token fTok = current();
    expect(TokenType::FOR, "for statement");
    std::string varName = parseIdentifier();
    if (!expect(TokenType::IN, "for-in syntax")) return nullptr;
    ExprPtr iter = parseExpression();
    // Range support: `for i in start..end` or `start..=end`
    if (current().type == TokenType::DOTDOT) {
        pos_++;
        bool inclusive = false;
        if (current().type == TokenType::EQUAL) {
            inclusive = true;
            pos_++;
        }
        ExprPtr end = parseExpression(P_ADD);
        iter = std::make_shared<RangeExpr>(iter, end, inclusive, fTok.line, fTok.col);
    }
    auto body = parseStatement(); // Already handles braces
    return std::make_shared<ForStmt>(varName, iter, body, fTok.line, fTok.col);
}

StmtPtr Parser::parseWhileStmt() {
    Token wTok = current();
    expect(TokenType::WHILE, "while statement");
    ExprPtr cond = parseExpression();
    auto body = parseStatement(); // Already handles braces
    return std::make_shared<WhileStmt>(cond, body, wTok.line, wTok.col);
}

StmtPtr Parser::parseReturnStmt() {
    Token rTok = current();
    expect(TokenType::RETURN, "return statement");
    std::optional<ExprPtr> val;
    if (current().type != TokenType::SEMICOLON && current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        val = parseExpression();
    }
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<ReturnStmt>(val, rTok.line, rTok.col);
}

StmtPtr Parser::parseBreakStmt() {
    Token bTok = current();
    expect(TokenType::BREAK, "break statement");
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<BreakStmt>(bTok.line, bTok.col);
}

StmtPtr Parser::parseContinueStmt() {
    Token cTok = current();
    expect(TokenType::CONTINUE, "continue statement");
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<ContinueStmt>(cTok.line, cTok.col);
}

// ---- MODULE MANAGEMENT ----
StmtPtr Parser::parseModuleDecl() {
    Token mTok = current();
    expect(TokenType::MODULE, "module declaration");
    std::string name = parseIdentifier();
    // parseBlock() expects the LBRACE to be current
    std::vector<StmtPtr> body = parseBlock();
    return std::make_shared<ModuleDecl>(name, body, mTok.line, mTok.col);
}

StmtPtr Parser::parseExportDecl() {
    Token eTok = current();
    expect(TokenType::EXPORT, "export declaration");
    std::string name = parseIdentifier();
    // Support both `export name;` and `export name = expr;`
    if (current().type == TokenType::EQUAL) {
        pos_++; // consume =
        auto expr = parseExpression();
        if (current().type == TokenType::SEMICOLON) pos_++;
        return std::make_shared<ExportDecl>(name, expr, eTok.line, eTok.col);
    } else {
        if (current().type == TokenType::SEMICOLON) pos_++;
        return std::make_shared<ExportDecl>(name, nullptr, eTok.line, eTok.col);
    }
}

StmtPtr Parser::parseLoadStmt() {
    Token lTok = current();
    expect(TokenType::LOAD, "load statement");
    std::string name;
    while (current().type == TokenType::IDENT || current().type == TokenType::DOT) {
        name += current().lexeme;
        pos_++;
    }
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<LoadStmt>(name, lTok.line, lTok.col);
}

// ---- EXCEPTION HANDLING ----
StmtPtr Parser::parseTryCatchStmt() {
    Token tTok = current();
    expect(TokenType::TRY, "try statement");
    // parseBlock() expects LBRACE to be current
    std::vector<StmtPtr> tryBody = parseBlock();

    std::vector<CatchClause> catches;
    while (current().type == TokenType::CATCH) {
        CatchClause clause;
        pos_++; // consume catch
        if (current().type == TokenType::LPAREN) {
            pos_++; // consume (
            std::string excName = parseIdentifier();
            // Type annotation is optional: catch (e) or catch (e: string)
            if (current().type == TokenType::COLON) {
                pos_++; // consume :
                std::string excType = parseIdentifier();
                clause.exceptionType = nullptr; // type lookup in semantic
            }
            if (!expect(TokenType::RPAREN, ")")) return nullptr;
            clause.exceptionName = excName;
        }
        clause.body = std::make_shared<BlockStmt>(parseBlock(), current().line, current().col);
        catches.push_back(clause);
    }

    std::vector<StmtPtr> finallyBody;
    if (current().type == TokenType::FINALLY) {
        pos_++;
        finallyBody = parseBlock();
    }

    return std::make_shared<TryCatchStmt>(tryBody, catches, finallyBody, tTok.line, tTok.col);
}

StmtPtr Parser::parseThrowStmt() {
    Token tTok = current();
    expect(TokenType::THROW, "throw statement");
    auto expr = parseExpression();
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<ThrowStmt>(expr, tTok.line, tTok.col);
}

// ---- BYTECODE / VM ----
StmtPtr Parser::parseBytecodeStmt() {
    Token bTok = current();
    expect(TokenType::BYTECODE, "bytecode block");
    if (!expect(TokenType::LBRACE, "bytecode body")) return nullptr;
    std::ostringstream ss;
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        ss << current().lexeme << " ";
        pos_++;
    }
    if (current().type == TokenType::RBRACE) pos_++;
    return std::make_shared<BytecodeStmt>(ss.str(), bTok.line, bTok.col);
}

StmtPtr Parser::parseOpStmt() {
    Token oTok = current();
    expect(TokenType::OP, "opcode statement");
    std::string opcode = parseIdentifier();
    std::vector<std::string> operands;
    while (current().type == TokenType::COMMA || current().type == TokenType::IDENT ||
           current().type == TokenType::INT_LITERAL || current().type == TokenType::FLOAT_LITERAL) {
        if (current().type == TokenType::COMMA) { pos_++; continue; }
        operands.push_back(current().lexeme);
        pos_++;
    }
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<OpStmt>(opcode, operands, oTok.line, oTok.col);
}

StmtPtr Parser::parseMatchStmt() {
    Token mTok = current();
    expect(TokenType::MATCH, "match statement");
    ExprPtr scrut = parseExpression();
    if (!expect(TokenType::LBRACE, "match body")) return nullptr;
    std::vector<MatchArm> arms;
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        ExprPtr pat = parseExpression();
        // optional guard: 'if expr'
        if (current().type == TokenType::EQUAL && peekNext().type == TokenType::GT) {
            pos_ += 2; // skip =>
        } else if (current().type == TokenType::DOUBLE_ARROW) {
            pos_++;
        } else {
            errorAt(current(), "Expected '=>' in match arm");
        }
        StmtPtr body;
        if (current().type == TokenType::LBRACE) {
            body = std::make_shared<BlockStmt>(parseBlock(), current().line, current().col);
        } else {
            body = std::make_shared<ExprStmt>(parseExpression(), current().line, current().col);
        }
        arms.push_back({pat, body});
        if (current().type == TokenType::COMMA) pos_++;
    }
    expect(TokenType::RBRACE, "match body");
    return std::make_shared<MatchExpr>(scrut, arms, mTok.line, mTok.col);
}

ExprPtr Parser::parseExpression() {
    return parseExpression(P_NONE);
}

int Parser::precedenceFor(TokenType t) const {
    switch (t) {
        case TokenType::OR: return P_OR;
        case TokenType::AND: return P_AND;
        case TokenType::EQ: case TokenType::NEQ: return P_EQ;
        case TokenType::LT: case TokenType::GT: case TokenType::LTE: case TokenType::GTE: return P_CMP;
        case TokenType::PLUS: case TokenType::MINUS: return P_ADD;
        case TokenType::STAR: case TokenType::SLASH: case TokenType::MOD: return P_MUL;
        default: return P_NONE;
    }
}

bool Parser::isBinaryOp(TokenType t) const {
    return precedenceFor(t) > P_NONE;
}

bool Parser::isUnaryOp(TokenType t) const {
    return t == TokenType::NOT || t == TokenType::MINUS || t == TokenType::PLUS;
}

ExprPtr Parser::parseExpression(int minPrec) {
    // Human-friendly operator words (v0.4.0): is/and/or/not
    auto humanizeOp = [](Token tk) -> Token {
        if (tk.type == TokenType::IDENT) {
            if (tk.lexeme == "is")       tk.type = TokenType::EQ;
            else if (tk.lexeme == "and") tk.type = TokenType::AND;
            else if (tk.lexeme == "or")  tk.type = TokenType::OR;
            else if (tk.lexeme == "not") tk.type = TokenType::NOT;
        }
        return tk;
    };

    ExprPtr left;
    Token leadTok = current();
    bool leadIsWordNot = (leadTok.type == TokenType::IDENT && leadTok.lexeme == "not");
    if (isUnaryOp(leadTok.type) || leadIsWordNot) {
        Token opTok = current();
        if (leadIsWordNot) opTok.type = TokenType::NOT;
        pos_++;
        auto operand = parseExpression(P_UNARY);
        left = std::make_shared<UnaryOpExpr>(opTok.type, operand, opTok.line, opTok.col);
    } else {
        left = parsePrimary();
    }

    while (true) {
        Token op = current();
        op = humanizeOp(op);
        if (!isBinaryOp(op.type)) break;
        int prec = precedenceFor(op.type);
        if (prec < minPrec) break;
        pos_++;
        auto right = parseExpression(prec + 1);
        left = std::make_shared<BinaryOpExpr>(op.type, left, right, op.line, op.col);
    }

    // Python-style conditional expression: value_if_true if cond else value_if_false.
    // Binds loosest (only at full-expression level); the condition itself may not
    // contain a bare ternary (matches Python's or_test), the else branch may nest.
    // Same-line guard: an `if` starting the NEXT statement must never be captured.
    if (current().type == TokenType::IF && minPrec <= P_NONE && pos_ > 0 &&
        tokens_[pos_ - 1].line == current().line) {
        Token ifTok = current();
        pos_++;
        ExprPtr cond = parseExpression(P_OR);
        if (!expect(TokenType::ELSE, "conditional expression")) {
            return left;
        }
        ExprPtr elseV = parseExpression(P_NONE);
        return std::make_shared<ConditionalExpr>(cond, left, elseV,
                                                 ifTok.line, ifTok.col);
    }
    return left;
}

ExprPtr Parser::parsePrimary() {
    ExprPtr base = parseAtom();
    if (!base) return nullptr;
    return parsePostfix(base);
}

ExprPtr Parser::parsePostfix(ExprPtr base) {
    while (true) {
        Token t = current();
        if (t.type == TokenType::DOT) {
            pos_++;
            if (current().type != TokenType::IDENT) {
                errorAt(current(), "Expected identifier after '.'");
                return base;
            }
            std::string field = current().lexeme;
            pos_++;
            base = std::make_shared<FieldAccessExpr>(base, field, t.line, t.col);
        } else if (t.type == TokenType::LPAREN) {
            pos_++;
            std::vector<ExprPtr> args;
            while (current().type != TokenType::RPAREN && current().type != TokenType::TOKEN_EOF) {
                args.push_back(parseExpression());
                if (current().type == TokenType::COMMA) pos_++;
                else break;
            }
            expect(TokenType::RPAREN, "function call");
            base = std::make_shared<CallExpr>(base, args, t.line, t.col);
        } else if (t.type == TokenType::LBRACKET) {
            // Index access: base[expr]
            pos_++;
            auto idx = parseExpression();
            expect(TokenType::RBRACKET, "index access");
            base = std::make_shared<IndexExpr>(base, idx, t.line, t.col);
        } else {
            break;
        }
    }
    return base;
}

ExprPtr Parser::parseAtom() {
    Token t = current();
    // Conversion calls: int(expr), float(expr) — type keywords used as functions
    if (peekNext().type == TokenType::LPAREN) {
        static const std::map<TokenType, std::string> convNames = {
            {TokenType::INT_T, "int"},
            {TokenType::F32, "float"},
            {TokenType::F64, "float"},
            // ETC script commands used as builtin calls
            {TokenType::PRINT, "print"},
            {TokenType::OUTPUT, "output"},
            {TokenType::SAY, "say"},
            {TokenType::INPUT, "input"},
            // Testing framework calls
            {TokenType::ASSERT, "assert"},
            {TokenType::EXPECT, "expect"},
            {TokenType::TEST, "test"},
        };
        auto cit = convNames.find(t.type);
        if (cit != convNames.end()) {
            pos_++; // consume the keyword; parsePostfix handles the call parens
            return std::make_shared<IdentExpr>(cit->second, t.line, t.col);
        }
    }
    if (t.type == TokenType::INT_LITERAL) {
        pos_++;
        ClassifyResult c;
        c.kind = ClassifyResult::Integer;
        try { c.intVal = std::stoll(t.lexeme); } catch (...) { c.intVal = 0; }
        return std::make_shared<LiteralExpr>(c, t.lexeme, t.line, t.col);
    }
    if (t.type == TokenType::FLOAT_LITERAL) {
        pos_++;
        ClassifyResult c;
        c.kind = ClassifyResult::Float;
        try { c.floatVal = std::stod(t.lexeme); } catch (...) { c.floatVal = 0.0; }
        return std::make_shared<LiteralExpr>(c, t.lexeme, t.line, t.col);
    }
    if (t.type == TokenType::STRING_LITERAL) {
        pos_++;
        const std::string& raw = t.lexeme;
        if (raw.find("${") == std::string::npos) {
            ClassifyResult c;
            c.kind = ClassifyResult::String;
            c.strVal = raw;
            return std::make_shared<LiteralExpr>(c, raw, t.line, t.col);
        }
        // Interpolated string: "text ${expr} text ..."
        std::vector<ExprPtr> parts;
        auto pushText = [&](const std::string& s) {
            if (s.empty()) return;
            ClassifyResult c;
            c.kind = ClassifyResult::String;
            c.strVal = s;
            parts.push_back(std::make_shared<LiteralExpr>(c, s, t.line, t.col));
        };
        size_t last = 0, i = 0;
        while ((i = raw.find("${", last)) != std::string::npos) {
            pushText(raw.substr(last, i - last));
            size_t close = raw.find('}', i + 2);
            if (close == std::string::npos) break;
            std::string exprSrc = raw.substr(i + 2, close - i - 2);
            Lexer subLex(exprSrc);
            auto subToks = subLex.tokenize();
            Parser subParser(subToks, diag_);
            if (ExprPtr e = subParser.parseExpression()) {
                parts.push_back(e);
            }
            last = close + 1;
        }
        pushText(raw.substr(last));
        return std::make_shared<StringInterpExpr>(std::move(parts), t.line, t.col);
    }
    if (t.type == TokenType::TRUE) {
        pos_++;
        ClassifyResult c; c.kind = ClassifyResult::Bool; c.intVal = 1;
        return std::make_shared<LiteralExpr>(c, "true", t.line, t.col);
    }
    if (t.type == TokenType::FALSE) {
        pos_++;
        ClassifyResult c; c.kind = ClassifyResult::Bool; c.intVal = 0;
        return std::make_shared<LiteralExpr>(c, "false", t.line, t.col);
    }
    if (t.type == TokenType::NONE) {
        pos_++;
        ClassifyResult c; c.kind = ClassifyResult::None;
        return std::make_shared<LiteralExpr>(c, "none", t.line, t.col);
    }
    if (t.type == TokenType::CHAR_LITERAL) {
        pos_++;
        ClassifyResult c; c.kind = ClassifyResult::Char; c.strVal = t.lexeme;
        return std::make_shared<LiteralExpr>(c, t.lexeme, t.line, t.col);
    }
    if (t.type == TokenType::LPAREN) {
        pos_++;
        auto e = parseExpression();
        expect(TokenType::RPAREN, "parenthesized expression");
        return e;
    }
    if (t.type == TokenType::IDENT) {
        std::string name = t.lexeme;
        // Struct literal: Ident { field: expr, ... }
        if (peekNext().type == TokenType::LBRACE) {
            size_t save = pos_;
            pos_ += 2; // skip IDENT and LBRACE
            if (current().type == TokenType::IDENT && peekNext().type == TokenType::COLON) {
                std::vector<std::pair<std::string, ExprPtr>> fields;
                while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
                    std::string fname = parseIdentifier();
                    if (!expect(TokenType::COLON, "struct field")) return nullptr;
                    auto fval = parseExpression();
                    fields.emplace_back(fname, fval);
                    if (current().type == TokenType::COMMA) pos_++;
                    else break;
                }
                if (!expect(TokenType::RBRACE, "struct literal")) {
                    pos_ = save;
                    return nullptr;
                }
                return std::make_shared<StructLitExpr>(name, fields, t.line, t.col);
            }
            // Not a struct literal — restore
            pos_ = save;
        }
        pos_++;
        return std::make_shared<IdentExpr>(name, t.line, t.col);
    }
    if (t.type == TokenType::LBRACKET) {
        return parseArrayLiteral();
    }
    if (t.type == TokenType::LBRACE) {
        // block expression
        pos_++;
        std::vector<StmtPtr> body;
        while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
            auto s = parseStatement();
            if (s) body.push_back(s);
            if (current().type == TokenType::SEMICOLON) pos_++;
        }
        expect(TokenType::RBRACE, "block expression");
        // Wrap as BlockStmt — caller handles
        return nullptr; // not used at expr level for now
    }
    if (t.type == TokenType::IF) {
        // Wrap if-stmt in ExprStmt-like call. For M0 just return a literal 0 placeholder.
        parseIfStmt();
        ClassifyResult cr{ClassifyResult::Integer, 0, 0.0, ""};
        return std::make_shared<LiteralExpr>(cr, "0", 0, 0);
    }
    if (t.type == TokenType::MATCH) {
        auto m = parseMatchStmt();
        // Convert to expression (M0 placeholder)
        (void)m;
        ClassifyResult cr{ClassifyResult::Integer, 0, 0.0, ""};
        return std::make_shared<LiteralExpr>(cr, "0", 0, 0);
    }
    if (t.type == TokenType::FN) {
        // lambda
        pos_++;
        if (!expect(TokenType::LPAREN, "lambda")) return nullptr;
        std::vector<std::pair<std::string, TypePtr>> params;
        while (current().type != TokenType::RPAREN && current().type != TokenType::TOKEN_EOF) {
            std::string pname = parseIdentifier();
            if (current().type == TokenType::COLON) {
                pos_++;
                params.emplace_back(pname, parseType());
            } else {
                params.emplace_back(pname, nullptr);
            }
            if (current().type == TokenType::COMMA) pos_++;
            else break;
        }
        expect(TokenType::RPAREN, "lambda");
        TypePtr ret = nullptr;
        if (current().type == TokenType::ARROW) {
            pos_++;
            ret = parseType();
        }
        ExprPtr body;
        if (current().type == TokenType::LBRACE) {
            auto block = parseBlock();
            // wrap block in a fake function decl
            auto fn = std::make_shared<FunctionDecl>("__lambda__", ret, params, block, t.line, t.col);
            ClassifyResult c; c.kind = ClassifyResult::None;
            body = std::make_shared<LiteralExpr>(c, "<lambda>", t.line, t.col);
            (void)fn;
        } else {
            body = parseExpression();
        }
        return body;
    }
    errorAt(t, "Unexpected token: " + t.lexeme);
    pos_++;
    return nullptr;
}

ExprPtr Parser::parseStringInterpolation() {
    Token t = current();
    if (t.type != TokenType::STRING_LITERAL) return parseAtom();
    pos_++;
    ClassifyResult c;
    c.kind = ClassifyResult::String;
    c.strVal = t.lexeme;
    return std::make_shared<LiteralExpr>(c, t.lexeme, t.line, t.col);
}

ExprPtr Parser::parseArrayLiteral() {
    Token t = current();
    expect(TokenType::LBRACKET, "array literal");
    std::vector<ExprPtr> elems;
    while (current().type != TokenType::RBRACKET && current().type != TokenType::TOKEN_EOF) {
        elems.push_back(parseExpression());
        if (current().type == TokenType::COMMA) pos_++;
        else break;
    }
    expect(TokenType::RBRACKET, "array literal");
    return std::make_shared<ArrayLitExpr>(elems, t.line, t.col);
}

TypePtr Parser::parseType() {
    return parseGenericType();
}

TypePtr Parser::parseSimpleType() {
    Token t = current();
    if (t.type == TokenType::IDENT) {
        pos_++;
        return std::make_shared<BasicType>(t.lexeme);
    }
    switch (t.type) {
        case TokenType::INT_T:
        case TokenType::INT8: case TokenType::INT16: case TokenType::INT32: case TokenType::INT64:
        case TokenType::UINT: case TokenType::UINT8: case TokenType::UINT16: case TokenType::UINT32: case TokenType::UINT64:
        case TokenType::F32: case TokenType::F64:
        case TokenType::BOOL_T: case TokenType::CHAR_T: case TokenType::STRING:
        case TokenType::BYTE: case TokenType::VOID: {
            // Canonical lowercase type names so all later phases agree
            static const std::map<TokenType, std::string> canonical = {
                {TokenType::INT_T, "int"},
                {TokenType::INT8, "int8"}, {TokenType::INT16, "int16"},
                {TokenType::INT32, "int32"}, {TokenType::INT64, "int64"},
                {TokenType::UINT, "uint"}, {TokenType::UINT8, "uint8"},
                {TokenType::UINT16, "uint16"}, {TokenType::UINT32, "uint32"},
                {TokenType::UINT64, "uint64"},
                {TokenType::F32, "float32"}, {TokenType::F64, "float64"},
                {TokenType::BOOL_T, "bool"}, {TokenType::CHAR_T, "char"},
                {TokenType::STRING, "string"}, {TokenType::BYTE, "byte"},
                {TokenType::VOID, "void"},
            };
            std::string name = "int";
            auto it = canonical.find(t.type);
            if (it != canonical.end()) name = it->second;
            pos_++;
            return std::make_shared<BasicType>(name);
        }
        case TokenType::LBRACKET: {
            pos_++;
            TypePtr elem = parseType();
            expect(TokenType::SEMICOLON, "array size");
            int n = 0;
            if (current().type == TokenType::INT_LITERAL) {
                n = std::stoi(current().lexeme);
                pos_++;
            }
            expect(TokenType::RBRACKET, "array type");
            return std::make_shared<ArrayType>(elem, n);
        }
        case TokenType::OPTION: {
            pos_++;
            if (!expect(TokenType::LT, "Option<T>")) return nullptr;
            TypePtr inner = parseType();
            expect(TokenType::GT, "Option<T>");
            return std::make_shared<GenericType>("Option", std::vector<TypePtr>{inner});
        }
        case TokenType::RESULT: {
            pos_++;
            if (!expect(TokenType::LT, "Result<T, E>")) return nullptr;
            TypePtr ok = parseType();
            if (current().type == TokenType::COMMA) pos_++;
            TypePtr err = parseType();
            expect(TokenType::GT, "Result<T, E>");
            return std::make_shared<GenericType>("Result", std::vector<TypePtr>{ok, err});
        }
        default:
            errorAt(t, "Expected a type");
            pos_++;
            return nullptr;
    }
}

TypePtr Parser::parseGenericType() {
    auto base = parseSimpleType();
    
    // Support both < > and [ ] syntax for generic parameters
    if (current().type == TokenType::LT) {
        pos_++; // skip <
        std::vector<TypePtr> params;
        while (current().type != TokenType::GT && current().type != TokenType::TOKEN_EOF) {
            params.push_back(parseType());
            if (current().type == TokenType::COMMA) pos_++;
        }
        if (current().type == TokenType::GT) pos_++; // skip >
        return std::make_shared<GenericType>(base->getName(), params);
    }
    
    if (current().type == TokenType::LBRACKET) {
        pos_++; // skip [
        std::vector<TypePtr> params;
        while (current().type != TokenType::RBRACKET && current().type != TokenType::TOKEN_EOF) {
            params.push_back(parseType());
            if (current().type == TokenType::COMMA) pos_++;
        }
        if (current().type == TokenType::RBRACKET) pos_++; // skip ]
        return std::make_shared<GenericType>(base->getName(), params);
    }
    
    return base;
}

std::string Parser::parseIdentifier() {
    Token t = current();
    if (t.type != TokenType::IDENT) {
        errorAt(t, "Expected identifier, got: " + t.lexeme);
        pos_++;
        return "";
    }
    pos_++;
    return t.lexeme;
}

ClassifyResult Parser::classifyLiteral(const Token& tok) {
    ClassifyResult c;
    if (tok.type == TokenType::INT_LITERAL) {
        c.kind = ClassifyResult::Integer;
        try { c.intVal = std::stoll(tok.lexeme); } catch (...) {}
    } else if (tok.type == TokenType::FLOAT_LITERAL) {
        c.kind = ClassifyResult::Float;
        try { c.floatVal = std::stod(tok.lexeme); } catch (...) {}
    } else if (tok.type == TokenType::STRING_LITERAL) {
        c.kind = ClassifyResult::String;
        c.strVal = tok.lexeme;
    }
    return c;
}

bool Parser::isInFunction() const {
    return currentIsMain_;
}

bool Parser::isInLoop() const {
    return !scopeIsLoop_.empty() && scopeIsLoop_.back();
}

void Parser::pushScope() {
    scopes_.emplace_back();
    scopeIsFunction_.push_back(false);
    scopeIsLoop_.push_back(false);
}

void Parser::popScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
        scopeIsFunction_.pop_back();
        scopeIsLoop_.pop_back();
    }
}

void Parser::declareVariable(const std::string& name, TypePtr type, bool isMut) {
    if (!scopes_.empty()) {
        scopes_.back().insert(name + (isMut ? "$mut" : "$imm"));
    }
}


// ---- TESTING FRAMEWORK ----
StmtPtr Parser::parseTestStmt() {
    Token tTok = current();
    expect(TokenType::TEST, "test function");
    std::string name = parseIdentifier();
    if (!expect(TokenType::LPAREN, "(")) return nullptr;
    if (!expect(TokenType::RPAREN, ")")) return nullptr;
    if (!expect(TokenType::LBRACE, "{")) return nullptr;
    std::vector<StmtPtr> body = parseBlock();
    return std::make_shared<TestStmt>(name, body, tTok.line, tTok.col);
}

StmtPtr Parser::parseAssertStmt() {
    Token aTok = current();
    expect(TokenType::ASSERT, "assert statement");
    auto condition = parseExpression();
    ExprPtr message = nullptr;
    if (current().type == TokenType::COMMA) {
        pos_++;
        message = parseExpression();
    }
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<AssertStmt>(condition, message, aTok.line, aTok.col);
}

StmtPtr Parser::parseExpectStmt() {
    Token eTok = current();
    expect(TokenType::EXPECT, "expect statement");
    auto actual = parseExpression();
    if (!expect(TokenType::EQ, "==")) return nullptr;
    auto expected = parseExpression();
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<ExpectStmt>(actual, expected, eTok.line, eTok.col);
}

// ---- DESIGN BY CONTRACT ----
StmtPtr Parser::parseRequiresStmt() {
    Token rTok = current();
    expect(TokenType::REQUIRES, "requires clause");
    auto condition = parseExpression();
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<RequiresStmt>(condition, rTok.line, rTok.col);
}

StmtPtr Parser::parseEnsuresStmt() {
    Token eTok = current();
    expect(TokenType::ENSURES, "ensures clause");
    auto condition = parseExpression();
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<EnsuresStmt>(condition, eTok.line, eTok.col);
}

StmtPtr Parser::parseInvariantStmt() {
    Token iTok = current();
    expect(TokenType::INVARIANT, "invariant clause");
    auto condition = parseExpression();
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<InvariantStmt>(condition, iTok.line, iTok.col);
}

// ---- COROUTINES ----
StmtPtr Parser::parseCoroutineDecl() {
    Token cTok = current();
    expect(TokenType::COROUTINE, "coroutine declaration");
    std::string name = parseIdentifier();
    auto params = parseParamList();
    TypePtr returnType = nullptr;
    if (current().type == TokenType::ARROW) {
        pos_++;
        returnType = parseType();
    }
    auto body = parseBlock();
    return std::make_shared<CoroutineDecl>(name, params, returnType, body, cTok.line, cTok.col);
}

ExprPtr Parser::parseYieldExpr() {
    Token yTok = current();
    pos_++;
    ExprPtr value = nullptr;
    if (current().type != TokenType::SEMICOLON && current().type != TokenType::RPAREN &&
        current().type != TokenType::RBRACE && current().type != TokenType::COMMA) {
        value = parseExpression();
    }
    return std::make_shared<YieldExpr>(value, yTok.line, yTok.col);
}

ExprPtr Parser::parseAwaitExpr() {
    Token aTok = current();
    pos_++;
    auto operand = parseExpression();
    return std::make_shared<AwaitExpr>(operand, aTok.line, aTok.col);
}

// ---- HOT RELOAD ----
StmtPtr Parser::parseHotReloadStmt() {
    Token hTok = current();
    expect(TokenType::HOT, "hot reload statement");
    expect(TokenType::RELOAD, "reload keyword");
    std::string filePath = parseIdentifier();
    if (current().type == TokenType::SEMICOLON) pos_++;
    return std::make_shared<HotReloadStmt>(filePath, hTok.line, hTok.col);
}

// ---- PATTERNS ----
ExprPtr Parser::parsePatternMatchExpr() {
    Token mTok = current();
    expect(TokenType::MATCH, "match expression");
    auto scrutinee = parseExpression();
    if (!expect(TokenType::LBRACE, "match arms")) return nullptr;
    struct Arm {
        std::string pattern;
        ExprPtr guard;
        ExprPtr body;
    };
    std::vector<PatternMatchExpr::Arm> arms;
    while (current().type != TokenType::RBRACE && current().type != TokenType::TOKEN_EOF) {
        std::string pattern = parseIdentifier();
        ExprPtr guard = nullptr;
        if (current().type == TokenType::IF) {
            pos_++;
            guard = parseExpression();
        }
        if (!expect(TokenType::DOUBLE_ARROW, "=>")) return nullptr;
        auto body = parseExpression();
        arms.push_back(PatternMatchExpr::Arm{pattern, guard, body});
        if (current().type == TokenType::COMMA) pos_++;
    }
    expect(TokenType::RBRACE, "match arms");
    return std::make_shared<PatternMatchExpr>(scrutinee, arms, mTok.line, mTok.col);
}

// ---- MACROS ----
StmtPtr Parser::parseMacroDecl() {
    Token mTok = current();
    expect(TokenType::FN, "macro declaration");
    pos_--;
    std::string name = parseIdentifier();
    auto params = parseParamList();
    if (!expect(TokenType::LBRACE, "{")) return nullptr;
    std::vector<StmtPtr> body = parseBlock();
    return std::make_shared<MacroDecl>(name, body, mTok.line, mTok.col);
}

// ---- DIMENSIONAL TYPES ----
ExprPtr Parser::parseDimensionalTypeExpr() {
    Token dTok = current();
    std::string baseType = parseIdentifier();
    std::vector<std::string> dims;
    while (current().type == TokenType::STAR) {
        pos_++;
        std::string dim = parseIdentifier();
        dims.push_back(dim);
    }
    return std::make_shared<DimensionalTypeExpr>(baseType, dims, dTok.line, dTok.col);
}

std::vector<std::pair<std::string, TypePtr>> Parser::parseParamList() {
    std::vector<std::pair<std::string, TypePtr>> params;
    if (!expect(TokenType::LPAREN, "(")) return params;
    while (current().type != TokenType::RPAREN && current().type != TokenType::TOKEN_EOF) {
        std::string name = parseIdentifier();
        if (!expect(TokenType::COLON, ":")) continue;
        auto type = parseType();
        params.push_back({name, type});
        if (current().type == TokenType::COMMA) pos_++;
    }
    expect(TokenType::RPAREN, ")");
    return params;
}

} // namespace eng
