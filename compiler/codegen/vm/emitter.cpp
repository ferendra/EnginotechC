#include "emitter.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace eng {
namespace bytecode {

Emitter::Emitter(DiagnosticEngine& diag) : diag_(diag) {}

void Emitter::emit(eng::vm::Opcode op, int16_t operand, const std::string& strOperand) {
    eng::vm::Instruction instr;
    instr.op = op;
    instr.operand = operand;
    instr.strOperand = strOperand;
    code_.push_back(instr);
}

std::vector<eng::vm::Instruction> Emitter::emit(const std::vector<StmtPtr>& items) {
    code_.clear();
    localMap_.clear();
    currentLocal_ = 0;
    
    for (const auto& item : items) {
        emitStmt(item);
    }
    
    // Add return at end
    emit(eng::vm::Opcode::RETURN);
    
    return code_;
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
                emitExpr(exprStmt->expr);
                emit(eng::vm::Opcode::PRINT);
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

void Emitter::emitExpr(const ExprPtr& expr) {
    if (!expr) return;
    
    switch (expr->kind()) {
        case ExprKind::Literal: {
            auto* lit = static_cast<const LiteralExpr*>(expr.get());
            switch (lit->val.kind) {
                case ClassifyResult::Integer:
                    emit(eng::vm::Opcode::PUSH_INT, static_cast<int16_t>(lit->val.intVal));
                    break;
                case ClassifyResult::Float:
                    emit(eng::vm::Opcode::PUSH_FLOAT, static_cast<int16_t>(lit->val.floatVal));
                    break;
                case ClassifyResult::Bool:
                    emit(eng::vm::Opcode::PUSH_BOOL, lit->val.intVal);
                    break;
                case ClassifyResult::String:
                    emit(eng::vm::Opcode::PUSH_STR, 0, lit->val.strVal);
                    break;
                case ClassifyResult::Char:
                    emit(eng::vm::Opcode::PUSH_INT, static_cast<int16_t>(lit->val.intVal));
                    break;
                default:
                    emit(eng::vm::Opcode::PUSH_INT, 0);
                    break;
            }
            break;
        }
        case ExprKind::Ident: {
            auto* ident = static_cast<const IdentExpr*>(expr.get());
            auto it = localMap_.find(ident->name);
            if (it != localMap_.end()) {
                emit(eng::vm::Opcode::GET_LOCAL, it->second);
            } else {
                // Global
                emit(eng::vm::Opcode::PUSH_INT, 0);
            }
            break;
        }
        case ExprKind::BinaryOp: {
            auto* op = static_cast<const BinaryOpExpr*>(expr.get());
            emitExpr(op->left);
            emitExpr(op->right);
            
            switch (op->op) {
                case TokenType::PLUS: emit(eng::vm::Opcode::ADD); break;
                case TokenType::MINUS: emit(eng::vm::Opcode::SUB); break;
                case TokenType::STAR: emit(eng::vm::Opcode::MUL); break;
                case TokenType::SLASH: emit(eng::vm::Opcode::DIV); break;
                case TokenType::MOD: emit(eng::vm::Opcode::MOD); break;
                case TokenType::EQ: emit(eng::vm::Opcode::EQ); break;
                case TokenType::NEQ: emit(eng::vm::Opcode::NEQ); break;
                case TokenType::LT: emit(eng::vm::Opcode::LT); break;
                case TokenType::GT: emit(eng::vm::Opcode::GT); break;
                case TokenType::LTE: emit(eng::vm::Opcode::LTE); break;
                case TokenType::GTE: emit(eng::vm::Opcode::GTE); break;
                case TokenType::AND: emit(eng::vm::Opcode::AND); break;
                case TokenType::OR: emit(eng::vm::Opcode::OR); break;
                default: emit(eng::vm::Opcode::NOP); break;
            }
            break;
        }
        case ExprKind::UnaryOp: {
            auto* op = static_cast<const UnaryOpExpr*>(expr.get());
            emitExpr(op->operand);
            if (op->op == TokenType::MINUS) {
                emit(eng::vm::Opcode::PUSH_INT, 0);
                emit(eng::vm::Opcode::SUB);
            }
            break;
        }
        case ExprKind::Call: {
            auto* call = static_cast<const CallExpr*>(expr.get());
            // Emit arguments first
            for (const auto& arg : call->args) {
                emitExpr(arg);
            }
            // Then call
            emit(eng::vm::Opcode::CALL);
            break;
        }
        default:
            emit(eng::vm::Opcode::PUSH_INT, 0);
            break;
    }
}

void Emitter::emitLet(const LetStmt* stmt) {
    currentLocal_++;
    localMap_[stmt->name] = currentLocal_;
    emit(eng::vm::Opcode::PUSH_INT, 0);  // Placeholder for init
    emit(eng::vm::Opcode::SET_LOCAL, currentLocal_);
    emitExpr(stmt->init);
    emit(eng::vm::Opcode::SET_LOCAL, currentLocal_);
}

void Emitter::emitMut(const MutStmt* stmt) {
    // Similar to let but mutable - for now just emit as let
    emitLet(static_cast<const LetStmt*>(reinterpret_cast<const void*>(stmt)));
}

void Emitter::emitFn(const FunctionDecl* fn) {
    // Function emission handled by calling convention
    (void)fn;
}

void Emitter::emitReturn(const ReturnStmt* stmt) {
    if (stmt->expr.has_value()) {
        emitExpr(stmt->expr.value());
    }
    emit(eng::vm::Opcode::RETURN);
}

void Emitter::emitIf(const IfStmt* stmt) {
    emitExpr(stmt->cond);
    
    // Save position for jump
    int jumpPos = code_.size();
    emit(eng::vm::Opcode::JUMP_IF, 0);  // Placeholder
    
    emitStmt(stmt->thenBranch);
    
    // Patch jump
    code_[jumpPos].operand = code_.size() + 1;
    
    if (stmt->elseBranch) {
        int elseStart = code_.size();
        emit(eng::vm::Opcode::JUMP, 0);  // Placeholder
        emitStmt(stmt->elseBranch);
        // Patch else jump
        code_[elseStart].operand = code_.size();
    }
}

void Emitter::emitFor(const ForStmt* stmt) {
    if (stmt->varName.empty()) return;
    
    // Initialize counter
    currentLocal_++;
    localMap_[stmt->varName] = currentLocal_;
    emit(eng::vm::Opcode::PUSH_INT, 0);
    emit(eng::vm::Opcode::SET_LOCAL, currentLocal_);
    
    // Loop label
    int loopStart = code_.size();
    
    // Condition check
    emit(eng::vm::Opcode::GET_LOCAL, currentLocal_);
    emit(eng::vm::Opcode::PUSH_INT, 0);  // Placeholder for end
    emit(eng::vm::Opcode::GTE);
    
    int breakPos = code_.size();
    emit(eng::vm::Opcode::JUMP_IF, 0);
    
    // Body
    if (stmt->body) emitStmt(stmt->body);
    
    // Increment
    emit(eng::vm::Opcode::GET_LOCAL, currentLocal_);
    emit(eng::vm::Opcode::PUSH_INT, 1);
    emit(eng::vm::Opcode::ADD);
    emit(eng::vm::Opcode::SET_LOCAL, currentLocal_);
    
    // Jump back
    emit(eng::vm::Opcode::JUMP, loopStart);
    
    // Patch break
    code_[breakPos].operand = code_.size();
}

void Emitter::emitWhile(const WhileStmt* stmt) {
    int loopStart = code_.size();
    
    // Condition
    emitExpr(stmt->cond);
    
    int breakPos = code_.size();
    emit(eng::vm::Opcode::JUMP_IF, 0);
    
    // Body
    if (stmt->body) emitStmt(stmt->body);
    
    // Jump back
    emit(eng::vm::Opcode::JUMP, loopStart);
    
    // Patch break
    code_[breakPos].operand = code_.size();
}

} // namespace bytecode
} // namespace eng
