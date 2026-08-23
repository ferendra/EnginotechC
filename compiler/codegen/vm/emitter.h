#ifndef ENGBYTECODE_EMITTER_H
#define ENGBYTECODE_EMITTER_H

#include "../../ast/nodes.h"
#include "../../diagnostics/diagnostic.h"
#include "vm.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace eng {
namespace bytecode {

class Emitter {
public:
    Emitter(DiagnosticEngine& diag);
    
    // Generate bytecode from AST
    std::vector<eng::vm::Instruction> emit(const std::vector<StmtPtr>& items);
    
private:
    void emitStmt(const StmtPtr& stmt);
    void emitExpr(const ExprPtr& expr);
    void emitLet(const LetStmt* stmt);
    void emitMut(const MutStmt* stmt);
    void emitFn(const FunctionDecl* fn);
    void emitReturn(const ReturnStmt* stmt);
    void emitIf(const IfStmt* stmt);
    void emitFor(const ForStmt* stmt);
    void emitWhile(const WhileStmt* stmt);
    
    // Add instruction
    void emit(eng::vm::Opcode op, int16_t operand = 0, const std::string& strOperand = "");
    
    DiagnosticEngine& diag_;
    std::vector<eng::vm::Instruction> code_;
    std::unordered_map<std::string, int> localMap_;
    int currentLocal_ = 0;
};

} // namespace bytecode
} // namespace eng

#endif // ENGBYTECODE_EMITTER_H
