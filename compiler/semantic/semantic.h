#ifndef ENGSEM_SEMANTIC_H
#define ENGSEM_SEMANTIC_H

#include "../ast/nodes.h"
#include "../diagnostics/diagnostic.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace eng {

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(DiagnosticEngine& diag);

    void analyze(const std::vector<StmtPtr>& items);

    const std::unordered_map<std::string, TypePtr>& getSymbolTable() const { return symbolTable_; }
    bool hasErrors() const { return diag_.hasErrors(); }

private:
    void analyzeStmt(const StmtPtr& stmt);
    void analyzeExpr(const ExprPtr& expr, TypePtr expectedType = nullptr);

    void checkFunction(FunctionDecl* fn);
    void checkLet(LetStmt* stmt);
    void checkMut(MutStmt* stmt);
    void checkIf(IfStmt* stmt);
    void checkFor(ForStmt* stmt);
    void checkWhile(WhileStmt* stmt);
    void checkReturn(ReturnStmt* stmt, TypePtr expected);
    void checkStruct(StructDecl* st);
    void checkEnum(EnumDecl* en);
    void checkImpl(ImplDecl* im);
    void checkMatch(MatchExpr* m);

    void error(const std::string& code, const std::string& message, int line = 0, int col = 0);
    void warn(const std::string& code, const std::string& message, int line = 0, int col = 0);

    // ---- type inference helpers ----
    TypePtr inferReturnTypeOf(const std::vector<StmtPtr>& body);
    TypePtr unifyTypes(TypePtr a, TypePtr b);
    bool typesCompatible(TypePtr a, TypePtr b);
    TypePtr resolveType(const std::string& name);

    DiagnosticEngine& diag_;
    std::unordered_map<std::string, TypePtr> symbolTable_;
    std::unordered_map<std::string, std::vector<StmtPtr>> functionBody_;
    std::unordered_map<std::string, FunctionDecl*> functionDecls_;
    std::vector<FunctionDecl*> mainFunction_;
};

} // namespace eng

#endif // ENGSEM_SEMANTIC_H
