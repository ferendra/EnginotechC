#include "emitter.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace eng {
namespace python {

Emitter::Emitter(DiagnosticEngine& diag) : diag_(diag) {}

std::string Emitter::emit(const std::vector<StmtPtr>& items) {
    out_.str("");
    out_ << "# Auto-generated from EnginotechC++\n";
    out_ << "# Source: transpiled by engc --target python\n\n";
    
    for (const auto& item : items) {
        emitStmt(item);
    }
    
    return out_.str();
}

void Emitter::emitStmt(const StmtPtr& stmt) {
    if (!stmt) return;
    
    switch (stmt->kind()) {
        case StmtKind::Let:
            emitLet(static_cast<LetStmt*>(stmt.get()));
            break;
        case StmtKind::Mut:
            emitMut(static_cast<MutStmt*>(stmt.get()));
            break;
        case StmtKind::Return:
            emitReturn(static_cast<ReturnStmt*>(stmt.get()));
            break;
        case StmtKind::If:
            emitIf(static_cast<IfStmt*>(stmt.get()));
            break;
        case StmtKind::For:
            emitFor(static_cast<ForStmt*>(stmt.get()));
            break;
        case StmtKind::While:
            emitWhile(static_cast<WhileStmt*>(stmt.get()));
            break;
        case StmtKind::Block:
            emitBlock(static_cast<BlockStmt*>(stmt.get()));
            break;
        case StmtKind::Expr: {
            auto* exprStmt = static_cast<ExprStmt*>(stmt.get());
            if (exprStmt->expr) {
                emitPrint(exprStmt->expr);
            }
            break;
        }
        case StmtKind::FunctionDecl:
            emitFn(static_cast<FunctionDecl*>(stmt.get()));
            break;
        default:
            break;
    }
}

std::string Emitter::exprToString(const ExprPtr& expr) {
    if (!expr) return "None";
    
    switch (expr->kind()) {
        case ExprKind::Literal: {
            auto* lit = static_cast<const LiteralExpr*>(expr.get());
            switch (lit->val.kind) {
                case ClassifyResult::Integer: return std::to_string(lit->val.intVal);
                case ClassifyResult::Float: return std::to_string(lit->val.floatVal);
                case ClassifyResult::Bool: return (lit->val.intVal != 0) ? "True" : "False";
                case ClassifyResult::String: return "\"" + lit->val.strVal + "\"";
                case ClassifyResult::Char: {
                    std::string c(1, static_cast<char>(lit->val.intVal));
                    return "'" + c + "'";
                }
                default: return "None";
            }
        }
        case ExprKind::Ident: {
            return static_cast<const IdentExpr*>(expr.get())->name;
        }
        case ExprKind::BinaryOp: {
            auto* op = static_cast<const BinaryOpExpr*>(expr.get());
            std::string lhs = exprToString(op->left);
            std::string rhs = exprToString(op->right);
            std::string pyOp = "?";
            
            switch (op->op) {
                case TokenType::PLUS: pyOp = "+"; break;
                case TokenType::MINUS: pyOp = "-"; break;
                case TokenType::STAR: pyOp = "*"; break;
                case TokenType::SLASH: pyOp = "/"; break;
                case TokenType::MOD: pyOp = "%"; break;
                case TokenType::EQ: pyOp = "=="; break;
                case TokenType::NEQ: pyOp = "!="; break;
                case TokenType::LT: pyOp = "<"; break;
                case TokenType::GT: pyOp = ">"; break;
                case TokenType::LTE: pyOp = "<="; break;
                case TokenType::GTE: pyOp = ">="; break;
                case TokenType::AND: pyOp = "and"; break;
                case TokenType::OR: pyOp = "or"; break;
                default: break;
            }
            return "(" + lhs + " " + pyOp + " " + rhs + ")";
        }
        case ExprKind::UnaryOp: {
            auto* op = static_cast<const UnaryOpExpr*>(expr.get());
            std::string operand = exprToString(op->operand);
            if (op->op == TokenType::MINUS) return "(-" + operand + ")";
            if (op->op == TokenType::NOT) return "(not " + operand + ")";
            return operand;
        }
        case ExprKind::Call: {
            auto* call = static_cast<const CallExpr*>(expr.get());
            std::string callee = exprToString(call->callee);
            std::string args = "";
            for (size_t i = 0; i < call->args.size(); ++i) {
                if (i > 0) args += ", ";
                args += exprToString(call->args[i]);
            }
            return callee + "(" + args + ")";
        }
        case ExprKind::StringInterp: {
            auto* interp = static_cast<const StringInterpExpr*>(expr.get());
            std::string result = "f\"";
            for (size_t i = 0; i < interp->parts.size(); ++i) {
                result += "{" + exprToString(interp->parts[i]) + "}";
            }
            result += "\"";
            return result;
        }
        case ExprKind::Conditional: {
            auto* cond = static_cast<const ConditionalExpr*>(expr.get());
            std::string test = exprToString(cond->cond);
            std::string thenVal = exprToString(cond->thenValue);
            std::string elseVal = exprToString(cond->elseValue);
            return "(" + thenVal + " if " + test + " else " + elseVal + ")";
        }
        default:
            return "None";
    }
}

void Emitter::emitPrint(const ExprPtr& expr) {
    out_ << indent_ << "print(" << exprToString(expr) << ")" << "\n";
}

void Emitter::emitLet(const LetStmt* stmt) {
    out_ << indent_ << stmt->name << " = " << exprToString(stmt->init) << "\n";
}

void Emitter::emitMut(const MutStmt* stmt) {
    out_ << indent_ << stmt->name << " = " << exprToString(stmt->init) << "\n";
}

void Emitter::emitFn(const FunctionDecl* fn) {
    out_ << indent_ << "def " << fn->name << "(";
    for (size_t i = 0; i < fn->params.size(); ++i) {
        if (i > 0) out_ << ", ";
        out_ << fn->params[i].first;
    }
    out_ << "):\n";
    
    indent_ += "    ";
    for (const auto& s : fn->body) {
        emitStmt(s);
    }
    indent_ = indent_.substr(0, indent_.size() - 4);
    
    out_ << "\n";
}

void Emitter::emitReturn(const ReturnStmt* stmt) {
    out_ << indent_ << "return";
    if (stmt->expr.has_value()) {
        out_ << " " << exprToString(stmt->expr.value());
    }
    out_ << "\n";
}

void Emitter::emitIf(const IfStmt* stmt) {
    out_ << indent_ << "if " << exprToString(stmt->cond) << ":\n";
    
    indent_ += "    ";
    if (stmt->thenBranch) emitStmt(stmt->thenBranch);
    indent_ = indent_.substr(0, indent_.size() - 4);
    
    if (stmt->elseBranch) {
        out_ << indent_ << "else:\n";
        indent_ += "    ";
        emitStmt(stmt->elseBranch);
        indent_ = indent_.substr(0, indent_.size() - 4);
    }
    
    out_ << "\n";
}

void Emitter::emitFor(const ForStmt* stmt) {
    if (stmt->varName.empty() || !stmt->iterable) {
        out_ << indent_ << "# for loop skipped (not supported)\n";
        return;
    }
    
    // Check if it's a range (0..n) or iterable
    std::string iterExpr = exprToString(stmt->iterable);
    
    // Range loop: for i in range(n)
    if (iterExpr.find("Range") != std::string::npos || 
        iterExpr.find("Array.from") != std::string::npos) {
        out_ << indent_ << "for " << stmt->varName << " in range(" << iterExpr << "):\n";
    } else {
        // Iterable loop: for x in items
        out_ << indent_ << "for " << stmt->varName << " in " << iterExpr << ":\n";
    }
    
    indent_ += "    ";
    if (stmt->body) emitStmt(stmt->body);
    indent_ = indent_.substr(0, indent_.size() - 4);
    
    out_ << "\n";
}

void Emitter::emitWhile(const WhileStmt* stmt) {
    out_ << indent_ << "while " << exprToString(stmt->cond) << ":\n";
    
    indent_ += "    ";
    if (stmt->body) emitStmt(stmt->body);
    indent_ = indent_.substr(0, indent_.size() - 4);
    
    out_ << "\n";
}

void Emitter::emitBlock(const BlockStmt* stmt) {
    out_ << indent_ << "# block statement\n";
    indent_ += "    ";
    for (const auto& s : stmt->body) {
        emitStmt(s);
    }
    indent_ = indent_.substr(0, indent_.size() - 4);
}

std::string Emitter::getTypeStr(const TypePtr& type) {
    if (!type) return "Any";
    const auto& name = type->getName();
    if (name == "int" || name == "int8" || name == "int16" || name == "int32" || name == "int64" ||
        name == "uint" || name == "uint8" || name == "uint16" || name == "uint32" || name == "uint64") {
        return "int";
    }
    if (name == "float" || name == "float32" || name == "float64" || name == "double") {
        return "float";
    }
    if (name == "bool") return "bool";
    if (name == "string" || name == "str") return "str";
    if (name == "void") return "None";
    return "Any";
}

void Emitter::error(const std::string& code, const std::string& msg, int line, int col) {
    // Python transpiler doesn't emit errors at this level
    (void)code;
    (void)msg;
    (void)line;
    (void)col;
}

} // namespace python
} // namespace eng
