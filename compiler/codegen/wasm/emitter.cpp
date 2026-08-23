#include "emitter.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace eng {
namespace wasm {

Emitter::Emitter(DiagnosticEngine& diag) : diag_(diag) {}

std::string Emitter::emit(const std::vector<StmtPtr>& items) {
    out_.str("");
    out_ << "(module\n";
    indent_ = "  ";
    
    // Emit all functions
    for (const auto& item : items) {
        if (item->kind() == StmtKind::FunctionDecl) {
            emitFn(static_cast<FunctionDecl*>(item.get()));
        }
    }
    
    // Emit main call
    out_ << indent_ << "(call $main)\n";
    out_ << ")\n";
    
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
        case StmtKind::Expr: {
            auto* exprStmt = static_cast<ExprStmt*>(stmt.get());
            if (exprStmt->expr) {
                out_ << indent_ << exprToString(exprStmt->expr) << "\n";
            }
            break;
        }
        default:
            break;
    }
}

std::string Emitter::exprToString(const ExprPtr& expr) {
    if (!expr) return "(i32.const 0)";
    
    switch (expr->kind()) {
        case ExprKind::Literal: {
            auto* lit = static_cast<const LiteralExpr*>(expr.get());
            switch (lit->val.kind) {
                case ClassifyResult::Integer: return "(i32.const " + std::to_string(lit->val.intVal) + ")";
                case ClassifyResult::Float: return "(f32.const " + std::to_string(lit->val.floatVal) + ")";
                case ClassifyResult::Bool: return lit->val.intVal ? "(i32.const 1)" : "(i32.const 0)";
                case ClassifyResult::String: return "(i32.const 0)"; // String support later
                case ClassifyResult::Char: return "(i32.const " + std::to_string(lit->val.intVal) + ")";
                default: return "(i32.const 0)";
            }
        }
        case ExprKind::Ident: {
            return "(local.get $" + static_cast<const IdentExpr*>(expr.get())->name + ")";
        }
        case ExprKind::BinaryOp: {
            auto* op = static_cast<const BinaryOpExpr*>(expr.get());
            std::string lhs = exprToString(op->left);
            std::string rhs = exprToString(op->right);
            std::string opCode = "?";
            
            switch (op->op) {
                case TokenType::PLUS: opCode = "i32.add"; break;
                case TokenType::MINUS: opCode = "i32.sub"; break;
                case TokenType::STAR: opCode = "i32.mul"; break;
                case TokenType::SLASH: opCode = "i32.div_s"; break;
                case TokenType::MOD: opCode = "i32.rem_s"; break;
                case TokenType::EQ: opCode = "i32.eq"; break;
                case TokenType::NEQ: opCode = "i32.ne"; break;
                case TokenType::LT: opCode = "i32.lt_s"; break;
                case TokenType::GT: opCode = "i32.gt_s"; break;
                case TokenType::LTE: opCode = "i32.le_s"; break;
                case TokenType::GTE: opCode = "i32.ge_s"; break;
                default: break;
            }
            return "(" + opCode + " " + lhs + " " + rhs + ")";
        }
        case ExprKind::UnaryOp: {
            auto* op = static_cast<const UnaryOpExpr*>(expr.get());
            std::string operand = exprToString(op->operand);
            if (op->op == TokenType::MINUS) return "(i32.sub (i32.const 0) " + operand + ")";
            return operand;
        }
        case ExprKind::Call: {
            auto* call = static_cast<const CallExpr*>(expr.get());
            std::string callee = exprToString(call->callee);
            std::string args = "";
            for (size_t i = 0; i < call->args.size(); ++i) {
                if (i > 0) args += " ";
                args += exprToString(call->args[i]);
            }
            return "(call " + callee + " " + args + ")";
        }
        default:
            return "(i32.const 0)";
    }
}

void Emitter::emitLet(const LetStmt* stmt) {
    out_ << indent_ << "(local $" << stmt->name << " (i32))\n";
    out_ << indent_ << "(local.set $" << stmt->name << " " << exprToString(stmt->init) << ")\n";
}

void Emitter::emitMut(const MutStmt* stmt) {
    out_ << indent_ << "(local $" << stmt->name << " (i32))\n";
    out_ << indent_ << "(local.set $" << stmt->name << " " << exprToString(stmt->init) << ")\n";
}

void Emitter::emitFn(const FunctionDecl* fn) {
    // Collect param types
    std::string params = "";
    for (size_t i = 0; i < fn->params.size(); ++i) {
        if (i > 0) params += " ";
        params += "(param $" + fn->params[i].first + " i32)";
    }
    
    // Return type
    std::string retType = fn->returnType && fn->returnType->getName() == "void" ? "" : "(result i32)";
    
    out_ << indent_ << "(func $" << fn->name << " " << params << retType << "\n";
    
    indent_ += "  ";
    for (const auto& s : fn->body) {
        emitStmt(s);
    }
    indent_ = indent_.substr(0, indent_.size() - 2);
    
    out_ << indent_ << ")\n\n";
}

void Emitter::emitReturn(const ReturnStmt* stmt) {
    out_ << indent_ << "return";
    if (stmt->expr.has_value()) {
        out_ << " " << exprToString(stmt->expr.value());
    }
    out_ << "\n";
}

void Emitter::emitIf(const IfStmt* stmt) {
    out_ << indent_ << "(if " << exprToString(stmt->cond) << "\n";
    
    indent_ += "  ";
    if (stmt->thenBranch) emitStmt(stmt->thenBranch);
    indent_ = indent_.substr(0, indent_.size() - 2);
    
    if (stmt->elseBranch) {
        out_ << indent_ << "(else\n";
        indent_ += "  ";
        emitStmt(stmt->elseBranch);
        indent_ = indent_.substr(0, indent_.size() - 2);
    }
    
    out_ << indent_ << ")\n";
}

void Emitter::emitFor(const ForStmt* stmt) {
    if (stmt->varName.empty() || !stmt->iterable) {
        out_ << indent_ << ";; for loop not supported\n";
        return;
    }
    
    // Simple range loop: for i in 0..n
    out_ << indent_ << "(local $" << stmt->varName << " (i32))\n";
    out_ << indent_ << "(local.set $" << stmt->varName << " (i32.const 0))\n";
    
    std::string endExpr = exprToString(stmt->iterable);
    
    out_ << indent_ << "(block $break_" << stmt->varName << "\n";
    out_ << indent_ << "  (loop $continue_" << stmt->varName << "\n";
    out_ << indent_ << "    (if (i32.ge_s (local.get $" << stmt->varName << ") " << endExpr << ")\n";
    out_ << indent_ << "      (br $break_" << stmt->varName << ")\n";
    out_ << indent_ << "    )\n";
    
    indent_ += "  ";
    if (stmt->body) emitStmt(stmt->body);
    indent_ = indent_.substr(0, indent_.size() - 2);
    
    out_ << indent_ << "    (local.set $" << stmt->varName << " (i32.add (local.get $" << stmt->varName << ") (i32.const 1)))\n";
    out_ << indent_ << "    (br $continue_" << stmt->varName << ")\n";
    out_ << indent_ << "  )\n";
    out_ << indent_ << ")\n";
}

void Emitter::emitWhile(const WhileStmt* stmt) {
    out_ << indent_ << "(block $break_while\n";
    out_ << indent_ << "  (loop $continue_while\n";
    out_ << indent_ << "    (if (i32.eqz " << exprToString(stmt->cond) << ")\n";
    out_ << indent_ << "      (br $break_while)\n";
    out_ << indent_ << "    )\n";
    
    indent_ += "  ";
    if (stmt->body) emitStmt(stmt->body);
    indent_ = indent_.substr(0, indent_.size() - 2);
    
    out_ << indent_ << "    (br $continue_while)\n";
    out_ << indent_ << "  )\n";
    out_ << indent_ << ")\n";
}

std::string Emitter::getTypeWasm(const TypePtr& type) {
    if (!type) return "i32";
    const auto& name = type->getName();
    if (name == "int" || name == "int8" || name == "int16" || name == "int32" || 
        name == "uint" || name == "uint8" || name == "uint16" || name == "uint32") {
        return "i32";
    }
    if (name == "int64" || name == "uint64") return "i64";
    if (name == "float" || name == "float32" || name == "double" || name == "float64") return "f32";
    if (name == "bool") return "i32";
    return "i32";
}

} // namespace wasm
} // namespace eng
