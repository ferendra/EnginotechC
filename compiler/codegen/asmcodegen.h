// EnginotechC++ — x86_64 Assembly Code Generator (M0)
// Generates native x86_64 assembly directly without LLVM dependency
// Uses clang/gcc to assemble and link into native binary

#include "../ast/nodes.h"
#include "../lexer/lexer.h"
#include "../diagnostics/diagnostic.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cmath>

namespace eng {

enum class Reg { RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15 };

class X86CodeGen {
public:
    explicit X86CodeGen(DiagnosticEngine& diag);

    bool compile(const Program& prog, const std::string& outputPath);

private:
    // ---- IR Generation ----
    void generateIR(const Program& prog);
    void emitFunction(const FunctionDecl* fn);
    void emitStatement(const StmtPtr& stmt);
    void emitExpression(const ExprPtr& expr);

    // ---- Assembly Helpers ----
    std::string regName(Reg r);
    std::string immStr(int64_t val);
    std::string memStr(const std::string& base, int64_t offset = 0);
    void emitLine(const std::string& line);
    void emitLabel(const std::string& label);
    void emitComment(const std::string& comment);

    // ---- Expression Emission ----
    std::string emitLiteral(const LiteralExpr* lit);
    std::string emitIdent(const IdentExpr* ident);
    std::string emitBinaryOp(const BinaryOpExpr* op);
    std::string emitUnaryOp(const UnaryOpExpr* op);
    std::string emitCall(const CallExpr* call);
    std::string emitFieldAccess(const FieldAccessExpr* field);
    std::string emitArrayLit(const ArrayLitExpr* arr);
    std::string emitStringInterp(const StringInterpExpr* interp);
    std::string emitTypeCast(const TypeCastExpr* cast);

    // ---- Statement Emission ----
    void emitBlock(const std::vector<StmtPtr>& body);
    void emitLet(const LetStmt* stmt);
    void emitMut(const MutStmt* stmt);
    void emitAssign(const AssignStmt* stmt);
    void emitReturn(const ReturnStmt* stmt);
    void emitIf(const IfStmt* stmt);
    void emitFor(const ForStmt* stmt);
    void emitWhile(const WhileStmt* stmt);
    void emitBreak(const BreakStmt* stmt);
    void emitContinue(const ContinueStmt* stmt);
    void emitMatch(const MatchExpr* stmt);
    void emitExpressionStmt(const ExprStmt* stmt);

    // ---- Register Allocation ----
    Reg allocateReg();
    void freeReg(Reg r);
    std::string loadToReg(const std::string& val, Reg target = Reg::RAX);
    void storeFromReg(Reg source, const std::string& target);

    // ---- Type System ----
    std::string getTypeName(const TypePtr& type);
    int getTypeSize(const TypePtr& type);

    // ---- Helpers ----
    void error(const std::string& msg, int line = 0, int col = 0);
    void ensureStackAligned();
    std::string makeTemp() { return "$temp" + std::to_string(++tempCounter_); }

    DiagnosticEngine& diag_;
    std::ofstream out_;
    std::map<std::string, Reg> varRegs_;
    Reg nextReg_;
    int tempCounter_;
    int stackOffset_;
    std::vector<std::string> usedRegs_;
    std::vector<std::string> functionNames_;
};

X86CodeGen::X86CodeGen(DiagnosticEngine& diag)
    : diag_(diag), nextReg_(Reg::RAX), tempCounter_(0), stackOffset_(0) {
    usedRegs_ = {"RAX", "RCX", "RDX", "RBX", "RSI", "RDI", "R8", "R9", "R10", "R11"};
}

std::string X86CodeGen::regName(Reg r) {
    static const char* names[] = {
        "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    };
    return names[static_cast<int>(r)];
}

std::string X86CodeGen::immStr(int64_t val) {
    if (val >= 0 && val <= 255) {
        return "$" + std::to_string(val);
    }
    std::ostringstream oss;
    oss << "$" << val;
    return oss.str();
}

std::string X86CodeGen::memStr(const std::string& base, int64_t offset) {
    if (offset == 0) return "(" + base + ")";
    if (offset > 0) return std::to_string(offset) + "(" + base + ")";
    return "(" + base + ",$" + std::to_string(-offset) + ")";
}

void X86CodeGen::emitLine(const std::string& line) {
    out_ << line << "\n";
}

void X86CodeGen::emitLabel(const std::string& label) {
    out_ << label << ":\n";
}

void X86CodeGen::emitComment(const std::string& comment) {
    out_ << "\t# " << comment << "\n";
}

bool X86CodeGen::compile(const Program& prog, const std::string& outputPath) {
    // Open output file
    out_.open(outputPath);
    if (!out_.is_open()) {
        error("Cannot open output file: " + outputPath);
        return false;
    }

    // Emit file header
    emitLine(".intel_syntax noprefix");
    emitLine(".text");
    emitLine(".global main");
    emitComment("EnginotechC++ M0 Compiler - x86_64 Assembly Output");
    emitComment("Generated from EnginotechC++ source");

    // Generate IR and emit assembly
    generateIR(prog);

    out_.close();
    return !diag_.hasErrors();
}

void X86CodeGen::generateIR(const Program& prog) {
    // First pass: emit all function declarations
    for (const auto& item : prog.topLevel) {
        if (!item) continue;
        switch (item->kind()) {
            case StmtKind::FunctionDecl:
                emitFunction(static_cast<FunctionDecl*>(item.get()));
                break;
            case StmtKind::StructDecl:
            case StmtKind::EnumDecl:
                // Structs and enums are handled via their fields/methods
                break;
            default:
                break;
        }
    }

    // Emit main function wrapper
    emitLabel("main");
    emitLine("push rbp");
    emitLine("mov rbp, rsp");
    ensureStackAligned();

    // Check if main was defined
    bool hasMain = false;
    for (const auto& item : prog.topLevel) {
        if (item && item->kind() == StmtKind::FunctionDecl) {
            auto fn = static_cast<FunctionDecl*>(item.get());
            if (fn->name == "main") {
                hasMain = true;
                break;
            }
        }
    }

    if (!hasMain) {
        // Empty main - just return 0
        emitLine("mov eax, 0");
        emitLine("leave");
        emitLine("ret");
        return;
    }

    // Call user's main function
    emitLine("call _main");
    emitLine("mov eax, 0");
    emitLine("leave");
    emitLine("ret");

    // Emit _main alias if needed
    emitLabel("_main");
    emitLine("jmp main");
}

void X86CodeGen::emitFunction(const FunctionDecl* fn) {
    // Save function name
    functionNames_.push_back(fn->name);

    // Emit function label
    emitLabel(fn->name);
    emitLine("push rbp");
    emitLine("mov rbp, rsp");
    ensureStackAligned();

    // Store parameters on stack
    int paramOffset = 16; // Start after return address and rbp
    for (const auto& [name, type] : fn->params) {
        emitLine("mov [rbp-" + std::to_string(paramOffset) + "], " + regName(Reg::RDI));
        paramOffset += 8;
    }

    // Emit function body
    emitBlock(fn->body);

    // Return
    emitLine("mov rsp, rbp");
    emitLine("pop rbp");
    emitLine("ret");
}

void X86CodeGen::emitStatement(const StmtPtr& stmt) {
    if (!stmt) return;
    switch (stmt->kind()) {
        case StmtKind::Let:
            emitLet(static_cast<LetStmt*>(stmt.get()));
            break;
        case StmtKind::Mut:
            emitMut(static_cast<MutStmt*>(stmt.get()));
            break;
        case StmtKind::Assign:
            emitAssign(static_cast<AssignStmt*>(stmt.get()));
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
        case StmtKind::Break:
            emitBreak(static_cast<BreakStmt*>(stmt.get()));
            break;
        case StmtKind::Continue:
            emitContinue(static_cast<ContinueStmt*>(stmt.get()));
            break;
        case StmtKind::Match:
            emitMatch(static_cast<MatchExpr*>(stmt.get()));
            break;
        case StmtKind::Expr:
            emitExpressionStmt(static_cast<ExprStmt*>(stmt.get()));
            break;
        case StmtKind::Block:
            emitBlock(static_cast<BlockStmt*>(stmt.get())->body);
            break;
        default:
            break;
    }
}

void X86CodeGen::emitBlock(const std::vector<StmtPtr>& body) {
    for (const auto& stmt : body) {
        if (!stmt) continue;
        emitStatement(stmt);
        if (diag_.hasErrors()) return;
    }
}

void X86CodeGen::emitExpression(const ExprPtr& expr) {
    if (!expr) return;
    switch (expr->kind()) {
        case ExprKind::Literal:
            emitLiteral(static_cast<LiteralExpr*>(expr.get()));
            break;
        case ExprKind::Ident:
            emitIdent(static_cast<IdentExpr*>(expr.get()));
            break;
        case ExprKind::BinaryOp:
            emitBinaryOp(static_cast<BinaryOpExpr*>(expr.get()));
            break;
        case ExprKind::UnaryOp:
            emitUnaryOp(static_cast<UnaryOpExpr*>(expr.get()));
            break;
        case ExprKind::Call:
            emitCall(static_cast<CallExpr*>(expr.get()));
            break;
        case ExprKind::FieldAccess:
            emitFieldAccess(static_cast<FieldAccessExpr*>(expr.get()));
            break;
        case ExprKind::ArrayLit:
            emitArrayLit(static_cast<ArrayLitExpr*>(expr.get()));
            break;
        case ExprKind::StringInterp:
            emitStringInterp(static_cast<StringInterpExpr*>(expr.get()));
            break;
        case ExprKind::TypeCast:
            emitTypeCast(static_cast<TypeCastExpr*>(expr.get()));
            break;
        default:
            break;
    }
}

void X86CodeGen::emitLet(const LetStmt* stmt) {
    std::string reg = loadToReg("0", Reg::RAX);
    if (stmt->init) {
        reg = emitExpression(stmt->init);
    }
    emitLine("mov [" + regName(Reg::RBP) + "-8], " + reg);
}

void X86CodeGen::emitMut(const MutStmt* stmt) {
    (void)stmt;
    // Mutable variables - same as let for now
    emitLet(stmt);
}

void X86CodeGen::emitAssign(const AssignStmt* stmt) {
    (void)stmt;
    // Simplified assignment
}

void X86CodeGen::emitReturn(const ReturnStmt* stmt) {
    if (stmt->expr) {
        emitExpression(stmt->expr);
    }
    emitLine("mov rsp, rbp");
    emitLine("pop rbp");
    emitLine("ret");
}

void X86CodeGen::emitIf(const IfStmt* stmt) {
    emitExpression(stmt->cond);
    std::string elseLabel = "$else_" + std::to_string(tempCounter_++);
    std::string endLabel = "$endif_" + std::to_string(tempCounter_++);

    emitLine("test rax, rax");
    emitLine("je " + elseLabel);

    emitBlock(stmt->thenBranch->kind() == StmtKind::Block
              ? static_cast<BlockStmt*>(stmt->thenBranch.get())->body
              : std::vector<StmtPtr>{stmt->thenBranch});
    emitLine("jmp " + endLabel);

    emitLabel(elseLabel);
    if (stmt->elseBranch) {
        emitBlock(stmt->elseBranch->kind() == StmtKind::Block
                  ? static_cast<BlockStmt*>(stmt->elseBranch.get())->body
                  : std::vector<StmtPtr>{stmt->elseBranch});
    }

    emitLabel(endLabel);
}

void X86CodeGen::emitFor(const ForStmt* stmt) {
    (void)stmt;
    // Simplified for loop
}

void X86CodeGen::emitWhile(const WhileStmt* stmt) {
    std::string condLabel = "$while_cond_" + std::to_string(tempCounter_++);
    std::string bodyLabel = "$while_body_" + std::to_string(tempCounter_++);
    std::string endLabel = "$while_end_" + std::to_string(tempCounter_++);

    emitLabel(condLabel);
    emitExpression(stmt->cond);
    emitLine("test rax, rax");
    emitLine("je " + endLabel);

    emitLabel(bodyLabel);
    emitBlock(stmt->body->kind() == StmtKind::Block
              ? static_cast<BlockStmt*>(stmt->body.get())->body
              : std::vector<StmtPtr>{stmt->body});
    emitLine("jmp " + condLabel);

    emitLabel(endLabel);
}

void X86CodeGen::emitBreak(const BreakStmt* stmt) {
    (void)stmt;
    emitLine("jmp $loop_end");
}

void X86CodeGen::emitContinue(const ContinueStmt* stmt) {
    (void)stmt;
    emitLine("jmp $loop_start");
}

void X86CodeGen::emitMatch(const MatchExpr* stmt) {
    (void)stmt;
    // Match not fully implemented in M0
}

void X86CodeGen::emitExpressionStmt(const ExprStmt* stmt) {
    emitExpression(stmt->expr);
}

std::string X86CodeGen::emitLiteral(const LiteralExpr* lit) {
    const ClassifyResult& val = lit->val;
    switch (val.kind) {
        case ClassifyResult::Integer: {
            std::string reg = makeTemp();
            emitLine("mov " + reg + ", " + immStr(val.intVal));
            return reg;
        }
        case ClassifyResult::Float: {
            std::string reg = makeTemp();
            emitLine("movsd " + reg + ", XMM0"); // Simplified
            return reg;
        }
        case ClassifyResult::Bool: {
            std::string reg = makeTemp();
            emitLine("mov " + reg + ", " + immStr(val.intVal ? 1 : 0));
            return reg;
        }
        case ClassifyResult::String:
            // String literals handled separately
            return "0";
        default:
            return "0";
    }
}

std::string X86CodeGen::emitIdent(const IdentExpr* ident) {
    (void)ident;
    return "0"; // Simplified
}

std::string X86CodeGen::emitBinaryOp(const BinaryOpExpr* op) {
    (void)op;
    return "0"; // Simplified
}

std::string X86CodeGen::emitUnaryOp(const UnaryOpExpr* op) {
    (void)op;
    return "0";
}

std::string X86CodeGen::emitCall(const CallExpr* call) {
    (void)call;
    return "0";
}

std::string X86CodeGen::emitFieldAccess(const FieldAccessExpr* field) {
    (void)field;
    return "0";
}

std::string X86CodeGen::emitArrayLit(const ArrayLitExpr* arr) {
    (void)arr;
    return "0";
}

std::string X86CodeGen::emitStringInterp(const StringInterpExpr* interp) {
    (void)interp;
    return "0";
}

std::string X86CodeGen::emitTypeCast(const TypeCastExpr* cast) {
    (void)cast;
    return "0";
}

std::string X86CodeGen::emitExpression(const ExprPtr& expr) {
    if (!expr) return "0";
    // This is a simplified version - real implementation would track registers
    switch (expr->kind()) {
        case ExprKind::Literal:
            return emitLiteral(static_cast<LiteralExpr*>(expr.get()));
        case ExprKind::Ident:
            return emitIdent(static_cast<IdentExpr*>(expr.get()));
        case ExprKind::BinaryOp:
            return emitBinaryOp(static_cast<BinaryOpExpr*>(expr.get()));
        case ExprKind::UnaryOp:
            return emitUnaryOp(static_cast<UnaryOpExpr*>(expr.get()));
        case ExprKind::Call:
            return emitCall(static_cast<CallExpr*>(expr.get()));
        default:
            return "0";
    }
}

Reg X86CodeGen::allocateReg() {
    Reg r = nextReg_;
    nextReg_ = static_cast<Reg>(static_cast<int>(nextReg_) + 1);
    if (nextReg_ > Reg::R11) nextReg_ = Reg::RAX;
    return r;
}

void X86CodeGen::freeReg(Reg r) {
    (void)r;
}

std::string X86CodeGen::loadToReg(const std::string& val, Reg target) {
    std::string reg = regName(target);
    emitLine("mov " + reg + ", " + val);
    return reg;
}

void X86CodeGen::storeFromReg(Reg source, const std::string& target) {
    emitLine("mov " + target + ", " + regName(source));
}

std::string X86CodeGen::getTypeName(const TypePtr& type) {
    if (!type) return "int";
    return type->name;
}

int X86CodeGen::getTypeSize(const TypePtr& type) {
    if (!type) return 8; // int = 8 bytes on x86_64
    const std::string& name = type->name;
    if (name == "bool" || name == "char" || name == "byte" || name == "int8" || name == "uint8") return 1;
    if (name == "int16" || name == "uint16") return 2;
    if (name == "int32" || name == "uint32" || name == "float32") return 4;
    if (name == "int64" || name == "uint64" || name == "float64") return 8;
    return 8; // default to 8 bytes
}

void X86CodeGen::error(const std::string& msg, int line, int col) {
    diag_.error("CG001", msg, line, col);
}

void X86CodeGen::ensureStackAligned() {
    // Ensure 16-byte stack alignment
    int mod = stackOffset_ % 16;
    if (mod != 0) {
        emitLine("sub rsp, " + std::to_string(16 - mod));
        stackOffset_ += (16 - mod);
    }
}

} // namespace eng
