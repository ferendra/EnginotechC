// EnginotechC++ — Arduino Statement Emitter
// Translates EC AST statements into Arduino C++ statement text.

#pragma once
#include "../ast/nodes.h"
#include "../lexer/token.h"
#include <map>
#include <sstream>
#include <vector>
#include "expr_emitter.h"

namespace eng {
namespace arduinogen {

class StmtEmitter {
public:
    explicit StmtEmitter(std::ostringstream& out, ExprEmitter& expr)
        : out_(out), expr_(expr) {}

    void emit(const StmtPtr& s);
    void emitBlock(const std::vector<StmtPtr>& body);

    // Returns the C++ type string for a type annotation
    std::string cppTypeOf(const TypePtr& t) const;

    // Declare a variable (used by block-level emitter)
    void declareVar(const std::string& name, const TypePtr& type);
    void declareLoopVar(const std::string& name);

private:
    std::ostringstream& out_;
    ExprEmitter&        expr_;
    int indent_ = 0;
    std::map<std::string, std::string> varTypes_; // name -> cxx type

    void line(const std::string& s);
    void emitLet(const LetStmt* st);
    void emitMut(const MutStmt* st);
    void emitAssign(const AssignStmt* st);
    void emitReturn(const ReturnStmt* st);
    void emitIf(const IfStmt* st);
    void emitWhile(const WhileStmt* st);
    void emitFor(const ForStmt* st);
    void emitMatch(const MatchExpr* m);
};

} // namespace arduinogen
} // namespace eng
