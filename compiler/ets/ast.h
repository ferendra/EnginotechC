// EnginotechC++ — ETC AST (Abstract Syntax Tree)
// Abstract Syntax Tree nodes for ETC script files (.etc)
//
// ETC Script Syntax:
//   print("Hello")
//   output "World"
//   say "Greeting"
//   # comments with # or //

#ifndef ENG_ETSKELETON_AST_H
#define ENG_ETSKELETON_AST_H

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <optional>
#include <iostream>

namespace eng {
namespace ets {

// ── Token Location ────────────────────────────────────────────────
struct Span {
    int line = 0;
    int col = 0;
    int endLine = 0;
    int endCol = 0;
    
    std::string toString() const {
        if (line == endLine && col == endCol) {
            return std::to_string(line) + ":" + std::to_string(col);
        }
        return std::to_string(line) + ":" + std::to_string(col) +
               "-" + std::to_string(endLine) + ":" + std::to_string(endCol);
    }
};

// ── Expression Kinds ────────────────���─────────────────────────────
enum class ExprKind {
    Literal,
    Ident,
    BinaryOp,
    UnaryOp,
    Call,
    StringInterp
};

// ── Base Node ─────────────────────────────────────────────────────
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual ExprKind getKind() const = 0;
    Span span;
    ASTNode(Span s) : span(s) {}
    ASTNode() : span({0, 0, 0, 0}) {}
    virtual std::string toString(int indent = 0) const = 0;
};

using ExprPtr = std::shared_ptr<ASTNode>;

// ── Expression Nodes ──────────────────────────────────────────────

// Literal: number, string, boolean
struct LiteralExpr : public ASTNode {
    enum class LiteralKind { Integer, Float, String, Bool, None };
    LiteralKind literalKind;
    std::string rawValue;
    long long intVal = 0;
    double floatVal = 0.0;
    std::string strVal;
    bool boolVal = false;
    
    LiteralExpr(LiteralKind k, const std::string& raw, const std::string& val, Span s)
        : ASTNode(s), literalKind(k), rawValue(raw), strVal(val) {
        if (k == LiteralKind::Integer) {
            try { intVal = std::stoll(val); } catch (...) { intVal = 0; }
        } else if (k == LiteralKind::Float) {
            try { floatVal = std::stod(val); } catch (...) { floatVal = 0.0; }
        } else if (k == LiteralKind::Bool) {
            boolVal = (val == "true");
        }
    }
    
    ExprKind getKind() const override { return ExprKind::Literal; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        std::string typeStr;
        switch (literalKind) {
            case LiteralKind::Integer: typeStr = "int"; break;
            case LiteralKind::Float: typeStr = "float"; break;
            case LiteralKind::String: typeStr = "string"; break;
            case LiteralKind::Bool: typeStr = "bool"; break;
            case LiteralKind::None: typeStr = "none"; break;
        }
        return pad + "Literal(" + typeStr + ", \"" + strVal + "\") @" + span.toString();
    }
};

// Identifier: variable reference
struct IdentExpr : public ASTNode {
    std::string name;
    
    IdentExpr(const std::string& n, Span s)
        : ASTNode(s), name(n) {}
    
    ExprKind getKind() const override { return ExprKind::Ident; }
    
    std::string toString(int indent = 0) const override {
        return std::string(indent * 2, ' ') + "Ident(\"" + name + "\") @" + span.toString();
    }
};

// Binary operation: a + b, a == b, etc.
struct BinaryOpExpr : public ASTNode {
    enum class BinOp { Add, Sub, Mul, Div, Mod, Eq, Neq, Lt, Gt, LtEq, GtEq, And, Or };
    BinOp op;
    ExprPtr left, right;
    
    BinaryOpExpr(BinOp o, ExprPtr l, ExprPtr r, Span s)
        : ASTNode(s), op(o), left(std::move(l)), right(std::move(r)) {}
    
    ExprKind getKind() const override { return ExprKind::BinaryOp; }
    
    std::string opName() const {
        switch (op) {
            case BinOp::Add: return "+";
            case BinOp::Sub: return "-";
            case BinOp::Mul: return "*";
            case BinOp::Div: return "/";
            case BinOp::Mod: return "%";
            case BinOp::Eq: return "==";
            case BinOp::Neq: return "!=";
            case BinOp::Lt: return "<";
            case BinOp::Gt: return ">";
            case BinOp::LtEq: return "<=";
            case BinOp::GtEq: return ">=";
            case BinOp::And: return "&&";
            case BinOp::Or: return "||";
        }
        return "?";
    }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        return pad + "BinOp(" + opName() + ")\n" +
               pad + "  left: " + left->toString(indent + 1) + "\n" +
               pad + "  right: " + right->toString(indent + 1);
    }
};

// Unary operation: -x, !x
struct UnaryOpExpr : public ASTNode {
    enum class UnOp { Negate, Not };
    UnOp op;
    ExprPtr operand;
    
    UnaryOpExpr(UnOp o, ExprPtr opnd, Span s)
        : ASTNode(s), op(o), operand(std::move(opnd)) {}
    
    ExprKind getKind() const override { return ExprKind::UnaryOp; }
    
    std::string opName() const {
        return (op == UnOp::Negate) ? "-" : "!";
    }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        return pad + "UnaryOp(" + opName() + ")\n" +
               pad + "  operand: " + operand->toString(indent + 1);
    }
};

// Function call: print("hello"), output(x)
struct CallExpr : public ASTNode {
    std::string functionName;
    std::vector<ExprPtr> args;
    
    CallExpr(const std::string& fn, std::vector<ExprPtr> a, Span s)
        : ASTNode(s), functionName(fn), args(std::move(a)) {}
    
    ExprKind getKind() const override { return ExprKind::Call; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        std::string result = pad + "Call(" + functionName + ")\n";
        for (size_t i = 0; i < args.size(); ++i) {
            result += pad + "  arg[" + std::to_string(i) + "]: " + 
                      args[i]->toString(indent + 1) + "\n";
        }
        return result;
    }
};

// String interpolation: "Hello ${name}"
struct StringInterpExpr : public ASTNode {
    std::vector<std::pair<std::string, ExprPtr>> parts;
    
    StringInterpExpr(std::vector<std::pair<std::string, ExprPtr>> p, Span s)
        : ASTNode(s), parts(std::move(p)) {}
    
    ExprKind getKind() const override { return ExprKind::StringInterp; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        std::string result = pad + "StringInterp\n";
        for (const auto& [text, expr] : parts) {
            result += pad + "  text: \"" + text + "\"\n";
            if (expr) {
                result += pad + "  expr: " + expr->toString(indent + 1) + "\n";
            }
        }
        return result;
    }
};

// ── Statement Nodes ───────────────────────────────────────────────
enum class StmtKind {
    Print,      // print("...")
    Output,     // output "..."
    Say,        // say "..."
    Expr,       // expression statement
    Block,      // { stmts }
    If,         // if expr stmt else stmt
    For,        // for var in expr stmt
    While,      // while expr stmt
    Return,     // return expr
    Break,      // break
    Continue    // continue
};

struct StmtNode {
    virtual ~StmtNode() = default;
    virtual StmtKind getKind() const = 0;
    Span span;
    StmtNode(Span s) : span(s) {}
    StmtNode() : span({0, 0, 0, 0}) {}
    virtual std::string toString(int indent = 0) const = 0;
};

using StmtPtr = std::shared_ptr<StmtNode>;

// Print statement: print("Hello World")
struct PrintStmt : public StmtNode {
    ExprPtr expr;
    
    PrintStmt(ExprPtr e, Span s)
        : StmtNode(s), expr(std::move(e)) {}
    
    StmtKind getKind() const override { return StmtKind::Print; }
    
    std::string toString(int indent = 0) const override {
        return std::string(indent * 2, ' ') + "PrintStmt\n" +
               expr->toString(indent + 1);
    }
};

// Output statement: output "Hello World"
struct OutputStmt : public StmtNode {
    ExprPtr expr;
    
    OutputStmt(ExprPtr e, Span s)
        : StmtNode(s), expr(std::move(e)) {}
    
    StmtKind getKind() const override { return StmtKind::Output; }
    
    std::string toString(int indent = 0) const override {
        return std::string(indent * 2, ' ') + "OutputStmt\n" +
               expr->toString(indent + 1);
    }
};

// Say statement: say "Hello World"
struct SayStmt : public StmtNode {
    ExprPtr expr;
    
    SayStmt(ExprPtr e, Span s)
        : StmtNode(s), expr(std::move(e)) {}
    
    StmtKind getKind() const override { return StmtKind::Say; }
    
    std::string toString(int indent = 0) const override {
        return std::string(indent * 2, ' ') + "SayStmt\n" +
               expr->toString(indent + 1);
    }
};

// Expression statement
struct ExprStmt : public StmtNode {
    ExprPtr expr;
    
    ExprStmt(ExprPtr e, Span s)
        : StmtNode(s), expr(std::move(e)) {}
    
    StmtKind getKind() const override { return StmtKind::Expr; }
    
    std::string toString(int indent = 0) const override {
        return std::string(indent * 2, ' ') + "ExprStmt\n" +
               expr->toString(indent + 1);
    }
};

// Block statement: { stmt1; stmt2; }
struct BlockStmt : public StmtNode {
    std::vector<StmtPtr> body;
    
    BlockStmt(std::vector<StmtPtr> b, Span s)
        : StmtNode(s), body(std::move(b)) {}
    
    StmtKind getKind() const override { return StmtKind::Block; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        std::string result = pad + "BlockStmt\n";
        for (const auto& stmt : body) {
            result += stmt->toString(indent + 1) + "\n";
        }
        return result;
    }
};

// If statement: if expr stmt else stmt
struct IfStmt : public StmtNode {
    ExprPtr cond;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e, Span s)
        : StmtNode(s), cond(std::move(c)), thenBranch(std::move(t)),
          elseBranch(std::move(e)) {}
    
    StmtKind getKind() const override { return StmtKind::If; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        std::string result = pad + "IfStmt\n" +
                           cond->toString(indent + 1) + "\n";
        result += pad + "  then:\n" + thenBranch->toString(indent + 2) + "\n";
        if (elseBranch) {
            result += pad + "  else:\n" + elseBranch->toString(indent + 2);
        }
        return result;
    }
};

// For statement: for var in expr stmt
struct ForStmt : public StmtNode {
    std::string varName;
    ExprPtr iterable;
    StmtPtr body;
    
    ForStmt(const std::string& vn, ExprPtr it, StmtPtr b, Span s)
        : StmtNode(s), varName(vn), iterable(std::move(it)), body(std::move(b)) {}
    
    StmtKind getKind() const override { return StmtKind::For; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        return pad + "ForStmt(" + varName + ")\n" +
               pad + "  iterable: " + iterable->toString(indent + 1) + "\n" +
               pad + "  body: " + body->toString(indent + 1);
    }
};

// While statement: while expr stmt
struct WhileStmt : public StmtNode {
    ExprPtr cond;
    StmtPtr body;
    
    WhileStmt(ExprPtr c, StmtPtr b, Span s)
        : StmtNode(s), cond(std::move(c)), body(std::move(b)) {}
    
    StmtKind getKind() const override { return StmtKind::While; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        return pad + "WhileStmt\n" +
               pad + "  cond: " + cond->toString(indent + 1) + "\n" +
               pad + "  body: " + body->toString(indent + 1);
    }
};

// Return statement: return expr
struct ReturnStmt : public StmtNode {
    std::optional<ExprPtr> expr;
    
    ReturnStmt(std::optional<ExprPtr> e, Span s)
        : StmtNode(s), expr(std::move(e)) {}
    
    StmtKind getKind() const override { return StmtKind::Return; }
    
    std::string toString(int indent = 0) const override {
        std::string pad(indent * 2, ' ');
        if (expr.has_value()) {
            return pad + "ReturnStmt\n" + expr.value()->toString(indent + 1);
        }
        return pad + "ReturnStmt()";
    }
};

// Break statement
struct BreakStmt : public StmtNode {
    BreakStmt(Span s) : StmtNode(s) {}
    StmtKind getKind() const override { return StmtKind::Break; }
    std::string toString(int = 0) const override { return "BreakStmt()"; }
};

// Continue statement
struct ContinueStmt : public StmtNode {
    ContinueStmt(Span s) : StmtNode(s) {}
    StmtKind getKind() const override { return StmtKind::Continue; }
    std::string toString(int = 0) const override { return "ContinueStmt()"; }
};

// ── Script Program ────────────────────────────────────────────────
struct ScriptProgram {
    std::string filename;
    std::vector<StmtPtr> statements;
    std::vector<std::string> comments;
    
    int totalTokens = 0;
    int printCount = 0;
    int outputCount = 0;
    int sayCount = 0;
    int errorCount = 0;
    
    void addStatement(StmtPtr stmt) {
        statements.push_back(stmt);
        if (stmt->getKind() == StmtKind::Print) printCount++;
        else if (stmt->getKind() == StmtKind::Output) outputCount++;
        else if (stmt->getKind() == StmtKind::Say) sayCount++;
    }
    
    std::string toDebugString() const {
        std::string result = "=== ScriptProgram ===\n";
        result += "File: " + filename + "\n";
        result += "Statements: " + std::to_string(statements.size()) + "\n";
        result += "Print calls: " + std::to_string(printCount) + "\n";
        result += "Output calls: " + std::to_string(outputCount) + "\n";
        result += "Say calls: " + std::to_string(sayCount) + "\n";
        result += "Errors: " + std::to_string(errorCount) + "\n\n";
        
        result += "--- AST ---\n";
        for (size_t i = 0; i < statements.size(); ++i) {
            result += "[" + std::to_string(i) + "] ";
            result += statements[i]->toString(0) + "\n\n";
        }
        return result;
    }
};

// ── Visitor Pattern ───────────────────────────────────────────────
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(LiteralExpr*) = 0;
    virtual void visit(IdentExpr*) = 0;
    virtual void visit(BinaryOpExpr*) = 0;
    virtual void visit(UnaryOpExpr*) = 0;
    virtual void visit(CallExpr*) = 0;
    virtual void visit(StringInterpExpr*) = 0;
    
    virtual void visitPrintStmt(PrintStmt*) = 0;
    virtual void visitOutputStmt(OutputStmt*) = 0;
    virtual void visitSayStmt(SayStmt*) = 0;
    virtual void visitExprStmt(ExprStmt*) = 0;
    virtual void visitBlockStmt(BlockStmt*) = 0;
    virtual void visitIfStmt(IfStmt*) = 0;
    virtual void visitForStmt(ForStmt*) = 0;
    virtual void visitWhileStmt(WhileStmt*) = 0;
    virtual void visitReturnStmt(ReturnStmt*) = 0;
    virtual void visitBreakStmt(BreakStmt*) = 0;
    virtual void visitContinueStmt(ContinueStmt*) = 0;
};

// ── Utility Functions ─────────────────────────────────────────────
inline Span makeSpan(int line, int col, int endLine = 0, int endCol = 0) {
    return {line, col, endLine, endCol};
}

} // namespace ets
} // namespace eng

#endif // ENG_ETSKELETON_AST_H
