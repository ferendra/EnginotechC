#include "semantic.h"
#include <algorithm>

namespace eng {

SemanticAnalyzer::SemanticAnalyzer(DiagnosticEngine& diag) : diag_(diag) {}

void SemanticAnalyzer::analyze(const std::vector<StmtPtr>& items) {
    for (const auto& item : items) {
        if (!item) continue;
        switch (item->kind()) {
            case StmtKind::FunctionDecl:
                analyzeStmt(item);
                break;
            case StmtKind::StructDecl:
                analyzeStmt(item);
                break;
            case StmtKind::EnumDecl:
                analyzeStmt(item);
                break;
            case StmtKind::Import:
            case StmtKind::Const:
                analyzeStmt(item);
                break;
            default:
                // top-level expressions — skip for now
                break;
        }
    }
}

void SemanticAnalyzer::analyzeStmt(const StmtPtr& stmt) {
    if (!stmt) return;
    switch (stmt->kind()) {
        case StmtKind::FunctionDecl:
            checkFunction(static_cast<FunctionDecl*>(stmt.get()));
            break;
        case StmtKind::StructDecl:
            checkStruct(static_cast<StructDecl*>(stmt.get()));
            break;
        case StmtKind::EnumDecl:
            checkEnum(static_cast<EnumDecl*>(stmt.get()));
            break;
        case StmtKind::Impl:
            checkImpl(static_cast<ImplDecl*>(stmt.get()));
            break;
        case StmtKind::Let:
            checkLet(static_cast<LetStmt*>(stmt.get()));
            break;
        case StmtKind::Mut:
            checkMut(static_cast<MutStmt*>(stmt.get()));
            break;
        case StmtKind::Return:
            checkReturn(static_cast<ReturnStmt*>(stmt.get()), nullptr);
            break;
        case StmtKind::If:
            checkIf(static_cast<IfStmt*>(stmt.get()));
            break;
        case StmtKind::For:
            checkFor(static_cast<ForStmt*>(stmt.get()));
            break;
        case StmtKind::While:
            checkWhile(static_cast<WhileStmt*>(stmt.get()));
            break;
        case StmtKind::Match:
            checkMatch(static_cast<MatchExpr*>(stmt.get()));
            break;
        default:
            break;
    }
}

void SemanticAnalyzer::checkFunction(FunctionDecl* fn) {
    if (fn->name == "main") {
        mainFunction_.push_back(fn);
    }
    functionDecls_[fn->name] = fn;
    for (const auto& [name, type] : fn->params) {
        symbolTable_[name] = type;
    }
    // Analyze body
    for (const auto& s : fn->body) {
        analyzeStmt(s);
    }
    // Clean up
    for (const auto& [name, type] : fn->params) {
        symbolTable_.erase(name);
    }
}

void SemanticAnalyzer::checkLet(LetStmt* stmt) {
    // Declare variable BEFORE analyzing expression so self-references work
    if (stmt->type) {
        symbolTable_[stmt->name] = stmt->type;
    } else {
        // Declare with no type for inference from init
        symbolTable_[stmt->name] = nullptr;
    }
    if (stmt->init) {
        analyzeExpr(stmt->init);
    }
    symbolTable_.erase(stmt->name);
}

void SemanticAnalyzer::checkMut(MutStmt* stmt) {
    if (stmt->type) {
        symbolTable_[stmt->name] = stmt->type;
    } else {
        symbolTable_[stmt->name] = nullptr;
    }
    if (stmt->init) {
        analyzeExpr(stmt->init);
    }
    symbolTable_.erase(stmt->name);
}

void SemanticAnalyzer::checkIf(IfStmt* stmt) {
    if (stmt->cond) {
        analyzeExpr(stmt->cond, resolveType("bool"));
    }
    if (stmt->thenBranch) analyzeStmt(stmt->thenBranch);
    if (stmt->elseBranch) analyzeStmt(stmt->elseBranch);
}

void SemanticAnalyzer::checkFor(ForStmt* stmt) {
    if (stmt->iterable) {
        analyzeExpr(stmt->iterable);
    }
    if (stmt->body) analyzeStmt(stmt->body);
}

void SemanticAnalyzer::checkWhile(WhileStmt* stmt) {
    if (stmt->cond) {
        analyzeExpr(stmt->cond, resolveType("bool"));
    }
    if (stmt->body) analyzeStmt(stmt->body);
}

void SemanticAnalyzer::checkReturn(ReturnStmt* stmt, TypePtr expected) {
    if (stmt->expr.has_value()) {
        analyzeExpr(stmt->expr.value(), expected);
    }
}

void SemanticAnalyzer::checkStruct(StructDecl* st) {
    if (!st) return;
    // Validate struct fields
    std::unordered_set<std::string> seenFields;
    for (const auto& [fieldName, fieldType] : st->fields) {
        if (seenFields.count(fieldName)) {
            diag_.error("E2007", "Duplicate field '" + fieldName + "' in struct '" + st->name + "'", st->line, st->col);
        }
        seenFields.insert(fieldName);
        if (!fieldType) {
            diag_.error("E2008", "Field '" + fieldName + "' has no type", st->line, st->col);
        }
    }
    // Validate methods
    for (const auto& method : st->methods) {
        analyzeStmt(method);
    }
}

void SemanticAnalyzer::checkEnum(EnumDecl* en) {
    (void)en;
}

void SemanticAnalyzer::checkImpl(ImplDecl* im) {
    for (const auto& method : im->methods) {
        analyzeStmt(method);
    }
}

void SemanticAnalyzer::checkMatch(MatchExpr* m) {
    if (m->scrutinee) {
        analyzeExpr(m->scrutinee);
    }
    for (const auto& arm : m->arms) {
        if (arm.pattern) analyzeExpr(arm.pattern);
        if (arm.body) analyzeStmt(arm.body);
    }
}

void SemanticAnalyzer::analyzeExpr(const ExprPtr& expr, TypePtr expectedType) {
    if (!expr) return;
    // Type inference will be done in codegen for now
    (void)expectedType;
}

TypePtr SemanticAnalyzer::resolveType(const std::string& name) {
    auto it = symbolTable_.find(name);
    if (it != symbolTable_.end()) return it->second;
    return nullptr;
}

TypePtr SemanticAnalyzer::inferReturnTypeOf(const std::vector<StmtPtr>& body) {
    for (auto it = body.rbegin(); it != body.rend(); ++it) {
        if ((*it)->kind() == StmtKind::Return) {
            auto ret = static_cast<ReturnStmt*>(it->get());
            if (ret->expr) {
                // Infer type from expression
                return resolveType("int"); // simplified
            }
        }
    }
    return resolveType("void");
}

TypePtr SemanticAnalyzer::unifyTypes(TypePtr a, TypePtr b) {
    if (!a || !b) return a ? a : b;
    if (a->getName() == b->getName()) return a;
    // Simplified: return int if both are numeric
    auto isInt = [](TypePtr t) {
        auto n = t->getName();
        return n == "int" || n == "int8" || n == "int16" || n == "int32" || n == "int64";
    };
    if (isInt(a) && isInt(b)) {
        return resolveType("int");
    }
    return resolveType("int"); // default fallback
}

bool SemanticAnalyzer::typesCompatible(TypePtr a, TypePtr b) {
    if (!a || !b) return false;
    if (a->getName() == b->getName()) return true;
    return false;
}

void SemanticAnalyzer::error(const std::string& code, const std::string& message, int line, int col) {
    diag_.error(code, message, line, col);
}

void SemanticAnalyzer::warn(const std::string& code, const std::string& message, int line, int col) {
    diag_.warning(code, message, line, col);
}

} // namespace eng
