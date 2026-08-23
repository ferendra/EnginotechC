#ifndef ENGSEM_ANALYZER_H
#define ENGSEM_ANALYZER_H

#include "../ast/nodes.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace eng {

class SemanticAnalyzer {
public:
    struct Diag {
        int line;
        int col;
        std::string message;
        enum class Level { Error, Warning } level;
    };

    SemanticAnalyzer();
    std::vector<DeclPtr> analyze(std::vector<DeclPtr> declarations);
    std::vector<Diag> diagnostics;

private:
    struct Scope {
        std::unordered_map<std::string, TypePtr> variables;
        std::unordered_map<std::string, TypePtr> types;       // struct/enum names
        std::unordered_map<std::string, FunctionDecl*> functions;
        std::vector<Scope*> children;
        Scope* parent = nullptr;
        bool isLoopContext = false;
    };

    Scope* currentScope_ = nullptr;
    Scope rootScope_;
    std::vector<DeclPtr> resolved_;
    std::unordered_set<std::string> knownTypes_;

    Scope* enterScope();
    void leaveScope();
    void declareVar(const std::string& name, TypePtr type, int line, int col);
    TypePtr lookupType(const std::string& name);
    bool hasType(const std::string& name);
    void error(const std::string& msg, int line, int col);
    void warn(const std::string& msg, int line, int col);
    bool isNumericCompatible(TypePtr a, TypePtr b);
    TypePtr commonNumericType(TypePtr a, TypePtr b);
    TypePtr inferType(const ExprPtr& expr);
    void checkExpr(const ExprPtr& expr);

    void analyzeDecl(const DeclPtr& decl);
    void analyzeFunction(const FunctionDecl* fn);
    void analyzeStmt(const StmtPtr& stmt);
    void analyzeExpr(const ExprPtr& expr);
    void analyzeBlock(const StmtList& stmts);
};

} // namespace eng

#endif // ENGSEM_ANALYZER_H
