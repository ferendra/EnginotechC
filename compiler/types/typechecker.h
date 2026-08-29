#ifndef ENGTYPE_TYPECHECKER_H
#define ENGTYPE_TYPECHECKER_H

#include "../ast/nodes.h"
#include "../diagnostics/diagnostic.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace eng {

class TypeChecker {
public:
    explicit TypeChecker(DiagnosticEngine& diag, bool dynamicTyping = false);

    void check(const std::vector<StmtPtr>& items);
    bool hasErrors() const { return diag_.hasErrors(); }

    const std::unordered_map<std::string, TypePtr>& getSymbols() const { return symbols_; }

private:
    TypePtr lookup(const std::string& name);
    bool isNumericType(const TypePtr& type) const;
    bool isFloatType(const TypePtr& type) const;
    TypePtr inferTypeFromLiteral(const LiteralExpr* lit) const;
    TypePtr getArithmeticResultType(const TokenType& op, const TypePtr& left, const TypePtr& right) const;
    std::string normalizeType(const std::string& name) const;
    void error(const std::string& code, const std::string& message, int line, int col);
    void typeCheckLet(const LetStmt* stmt);
    void typeCheckMut(const MutStmt* stmt);
    void typeCheckFn(const FunctionDecl* stmt, bool dynamicTyping = false);
    void typeCheckExpr(const ExprPtr& expr);
    void typeCheckFnParam(const std::string& name, TypePtr type, int line, int col);
    void typeCheckReturn(const ReturnStmt* stmt, const FunctionDecl* fn);
    void checkStmt(const StmtPtr& stmt);
    void typeCheckIf(const IfStmt* stmt);
    void typeCheckFor(const ForStmt* stmt);
    void typeCheckWhile(const WhileStmt* stmt);
    void typeCheckMatch(const MatchExpr* m);
    void checkEnumDecls(const std::vector<StmtPtr>& items);
    void checkStructDecls(const std::vector<StmtPtr>& items);

    // Generics support
    TypePtr substituteTypeParams(const TypePtr& type, const std::unordered_map<std::string, TypePtr>& typeArgs);
    void pushTypeParams(const std::vector<TypePtr>& typeParams);
    void popTypeParams();
    void inferTypeParams(const TypePtr& expected, const TypePtr& actual, std::unordered_map<std::string, TypePtr>& typeArgs);
    std::vector<std::unordered_map<std::string, TypePtr>> typeParamScopes_;

    DiagnosticEngine& diag_;
    bool dynamicTyping_;
    std::unordered_map<std::string, TypePtr> symbols_;
    std::unordered_map<std::string, bool> mutableVars_;
    std::unordered_set<std::string> declaredFns_;
    std::unordered_map<std::string, TypePtr> fnRetTypes_;
    std::vector<const EnumDecl*> enumDecls_;
    std::vector<const StructDecl*> structDecls_;
    // Warning tracking
    std::unordered_map<std::string, int> varUses_;
    std::unordered_set<std::string> warnedUnused_;
    std::unordered_set<std::string> warnedShadow_;
};

} // namespace eng

#endif // ENGTYPE_TYPECHECKER_H
