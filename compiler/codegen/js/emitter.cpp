#include "emitter.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace eng {
namespace js {

Emitter::Emitter(DiagnosticEngine& diag) : diag_(diag) {}

std::string Emitter::emit(const std::vector<StmtPtr>& items) {
    out_.str("");
    out_ << "'use strict';\n\n";
    
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
        case StmtKind::Expr:
            emitExpr(static_cast<ExprStmt*>(stmt.get())->expr);
            out_ << ";\n";
            break;
        case StmtKind::FunctionDecl:
            emitFn(static_cast<FunctionDecl*>(stmt.get()));
            break;
        default:
            break;
    }
}

std::string Emitter::exprToString(const ExprPtr& expr) {
    if (!expr) return "";
    
    switch (expr->kind()) {
        case ExprKind::Literal: {
            auto* lit = static_cast<const LiteralExpr*>(expr.get());
            switch (lit->val.kind) {
                case ClassifyResult::Integer: return std::to_string(lit->val.intVal);
                case ClassifyResult::Float: return std::to_string(lit->val.floatVal);
                case ClassifyResult::Bool: return "true";  // Simplified
                case ClassifyResult::String: return "\"" + lit->val.strVal + "\"";
                case ClassifyResult::Char: {
                    std::string c(1, static_cast<char>(lit->val.intVal));
                    return "'" + c + "'";
                }
                default: return "undefined";
            }
        }
        case ExprKind::Ident: {
            return static_cast<const IdentExpr*>(expr.get())->name;
        }
        case ExprKind::BinaryOp: {
            auto* op = static_cast<const BinaryOpExpr*>(expr.get());
            std::string lhs = exprToString(op->left);
            std::string rhs = exprToString(op->right);
            std::string jsOp = "?";
            
            switch (op->op) {
                case TokenType::PLUS: jsOp = "+"; break;
                case TokenType::MINUS: jsOp = "-"; break;
                case TokenType::STAR: jsOp = "*"; break;
                case TokenType::SLASH: jsOp = "/"; break;
                case TokenType::MOD: jsOp = "%"; break;
                case TokenType::EQ: jsOp = "=="; break;
                case TokenType::NEQ: jsOp = "!="; break;
                case TokenType::LT: jsOp = "<"; break;
                case TokenType::GT: jsOp = ">"; break;
                case TokenType::LTE: jsOp = "<="; break;
                case TokenType::GTE: jsOp = ">="; break;
                case TokenType::AND: jsOp = "&&"; break;
                case TokenType::OR: jsOp = "||"; break;
                default: break;
            }
            return "(" + lhs + " " + jsOp + " " + rhs + ")";
        }
        case ExprKind::UnaryOp: {
            auto* op = static_cast<const UnaryOpExpr*>(expr.get());
            std::string operand = exprToString(op->operand);
            if (op->op == TokenType::MINUS) return "(-" + operand + ")";
            if (op->op == TokenType::NOT) return "(!" + operand + ")";
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
            std::string result = "`";
            for (size_t i = 0; i < interp->parts.size(); ++i) {
                // Interpolate parts - this is simplified
                result += "${" + exprToString(interp->parts[i]) + "}";
            }
            result += "`";
            return result;
        }
        case ExprKind::Conditional: {
            auto* cond = static_cast<const ConditionalExpr*>(expr.get());
            std::string test = exprToString(cond->cond);
            std::string thenVal = exprToString(cond->thenValue);
            std::string elseVal = exprToString(cond->elseValue);
            return "(" + test + " ? " + thenVal + " : " + elseVal + ")";
        }
        case ExprKind::Range: {
            auto* range = static_cast<const RangeExpr*>(expr.get());
            std::string start = exprToString(range->start);
            std::string end = exprToString(range->end);
            // Generate JS range function
            return "Array.from({length: " + end + " - " + start + "}, (_, i) => " + start + " + i)";
        }
        default:
            return "/* unknown expr */";
    }
}

void Emitter::emitExpr(const ExprPtr& expr) {
    out_ << exprToString(expr);
}

void Emitter::emitLet(const LetStmt* stmt) {
    out_ << indent_ << "const " << stmt->name << " = ";
    emitExpr(stmt->init);
    out_ << ";\n";
}

void Emitter::emitMut(const MutStmt* stmt) {
    out_ << indent_ << "let " << stmt->name << " = ";
    emitExpr(stmt->init);
    out_ << ";\n";
}

void Emitter::emitFn(const FunctionDecl* fn) {
    out_ << indent_ << "function " << fn->name << "(";
    for (size_t i = 0; i < fn->params.size(); ++i) {
        if (i > 0) out_ << ", ";
        out_ << fn->params[i].first;
    }
    out_ << ") {\n";
    
    indent_ += "    ";
    for (const auto& s : fn->body) {
        emitStmt(s);
    }
    indent_ = indent_.substr(0, indent_.size() - 4);
    
    out_ << indent_ << "}\n\n";
}

void Emitter::emitReturn(const ReturnStmt* stmt) {
    out_ << indent_ << "return";
    if (stmt->expr.has_value()) {
        out_ << " " << exprToString(stmt->expr.value());
    }
    out_ << ";\n";
}

void Emitter::emitIf(const IfStmt* stmt) {
    out_ << indent_ << "if (";
    emitExpr(stmt->cond);
    out_ << ") {\n";
    
    indent_ += "    ";
    if (stmt->thenBranch) emitStmt(stmt->thenBranch);
    indent_ = indent_.substr(0, indent_.size() - 4);
    
    if (stmt->elseBranch) {
        out_ << indent_ << "} else {\n";
        indent_ += "    ";
        emitStmt(stmt->elseBranch);
        indent_ = indent_.substr(0, indent_.size() - 4);
    }
    
    out_ << indent_ << "}\n";
}

void Emitter::emitFor(const ForStmt* stmt) {
    // Check if it's a range loop (for i in 0..n) or iterable loop (for x in items)
    if (stmt->iterable && !stmt->varName.empty()) {
        // Range loop: for (let i = 0; i < n; i++)
        std::string rangeExpr = exprToString(stmt->iterable);
        out_ << indent_ << "for (let " << stmt->varName << " = 0; " << stmt->varName << " < " << rangeExpr << "; " << stmt->varName << "++) {\n";
        
        indent_ += "    ";
        if (stmt->body) emitStmt(stmt->body);
        indent_ = indent_.substr(0, indent_.size() - 4);
        
        out_ << indent_ << "}\n";
    } else if (stmt->iterable) {
        // Iterable loop: for (let x of items)
        out_ << indent_ << "for (let " << stmt->varName << " of ";
        emitExpr(stmt->iterable);
        out_ << ") {\n";
        
        indent_ += "    ";
        if (stmt->body) emitStmt(stmt->body);
        indent_ = indent_.substr(0, indent_.size() - 4);
        
        out_ << indent_ << "}\n";
    } else {
        // Fallback
        out_ << indent_ << "/* for loop not supported */\n";
    }
}

void Emitter::emitWhile(const WhileStmt* stmt) {
    out_ << indent_ << "while (";
    emitExpr(stmt->cond);
    out_ << ") {\n";
    
    indent_ += "    ";
    if (stmt->body) emitStmt(stmt->body);
    indent_ = indent_.substr(0, indent_.size() - 4);
    
    out_ << indent_ << "}\n";
}

void Emitter::emitBlock(const BlockStmt* stmt) {
    out_ << indent_ << "{\n";
    indent_ += "    ";
    for (const auto& s : stmt->body) {
        emitStmt(s);
    }
    indent_ = indent_.substr(0, indent_.size() - 4);
    out_ << indent_ << "}\n";
}

std::string Emitter::getTypeStr(const TypePtr& type) {
    if (!type) return "?";
    const auto& name = type->getName();
    if (name == "int" || name == "int8" || name == "int16" || name == "int32" || name == "int64" ||
        name == "uint" || name == "uint8" || name == "uint16" || name == "uint32" || name == "uint64") {
        return "number";
    }
    if (name == "float" || name == "float32" || name == "float64" || name == "double") {
        return "number";
    }
    if (name == "bool") return "boolean";
    if (name == "string" || name == "str") return "string";
    if (name == "void") return "void";
    return "?";
}

} // namespace js
} // namespace eng
