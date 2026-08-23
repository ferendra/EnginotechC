#ifndef ENGWASM_EMITTER_H
#define ENGWASM_EMITTER_H

#include "../../ast/nodes.h"
#include "../../diagnostics/diagnostic.h"
#include <string>
#include <vector>
#include <sstream>

namespace eng {
namespace wasm {

class Emitter {
public:
    Emitter(DiagnosticEngine& diag);
    std::string emit(const std::vector<StmtPtr>& items);
    
private:
    void emitStmt(const StmtPtr& stmt);
    std::string exprToString(const ExprPtr& expr);
    void emitLet(const LetStmt* stmt);
    void emitMut(const MutStmt* stmt);
    void emitFn(const FunctionDecl* fn);
    void emitReturn(const ReturnStmt* stmt);
    void emitIf(const IfStmt* stmt);
    void emitFor(const ForStmt* stmt);
    void emitWhile(const WhileStmt* stmt);
    std::string getTypeWasm(const TypePtr& type);
    
    DiagnosticEngine& diag_;
    std::string indent_;
    int indentLevel_ = 0;
    std::ostringstream out_;
    int localCount_ = 0;
};

} // namespace wasm
} // namespace eng

#endif // ENGWASM_EMITTER_H
