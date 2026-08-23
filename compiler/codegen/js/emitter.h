#ifndef ENGJS_EMITTER_H
#define ENGJS_EMITTER_H

#include "../../ast/nodes.h"
#include "../../diagnostics/diagnostic.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace eng {
namespace js {

class Emitter {
public:
    Emitter(DiagnosticEngine& diag);
    std::string emit(const std::vector<StmtPtr>& items);
    
private:
    void emitStmt(const StmtPtr& stmt);
    void emitExpr(const ExprPtr& expr);
    std::string exprToString(const ExprPtr& expr);
    void emitLet(const LetStmt* stmt);
    void emitMut(const MutStmt* stmt);
    void emitFn(const FunctionDecl* fn);
    void emitReturn(const ReturnStmt* stmt);
    void emitIf(const IfStmt* stmt);
    void emitFor(const ForStmt* stmt);
    void emitWhile(const WhileStmt* stmt);
    void emitBlock(const BlockStmt* stmt);
    std::string getTypeStr(const TypePtr& type);
    void error(const std::string& code, const std::string& msg, int line, int col);
    
    DiagnosticEngine& diag_;
    std::string indent_;
    int indentLevel_ = 0;
    std::unordered_map<std::string, std::string> variables_;
    std::ostringstream out_;
};

} // namespace js
} // namespace eng

#endif // ENGJS_EMITTER_H
