#ifndef ENGPY_EMITTER_H
#define ENGPY_EMITTER_H

#include "../../ast/nodes.h"
#include "../../diagnostics/diagnostic.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace eng {
namespace python {

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
    void emitBlock(const BlockStmt* stmt);
    void emitPrint(const ExprPtr& expr);
    std::string getTypeStr(const TypePtr& type);
    void error(const std::string& code, const std::string& msg, int line, int col);
    
    DiagnosticEngine& diag_;
    std::string indent_;
    int indentLevel_ = 0;
    std::ostringstream out_;
};

} // namespace python
} // namespace eng

#endif // ENGPY_EMITTER_H
