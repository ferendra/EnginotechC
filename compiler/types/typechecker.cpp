#include "../types/typechecker.h"
#include <set>

namespace eng {

TypeChecker::TypeChecker(DiagnosticEngine& diag) : diag_(diag) {}

std::string TypeChecker::normalizeType(const std::string& name) const {
    std::string n = name;
    for (auto& c : n) c = std::tolower(c);
    if (n == "f32" || n == "f64" || n == "float") n = "float64";
    // Treat vector and array as the same type
    if (n == "vector") n = "array";
    return n;
}

bool TypeChecker::isNumericType(const TypePtr& type) const {
    if (!type) return false;
    auto* basic = dynamic_cast<BasicType*>(type.get());
    if (!basic) return false;
    static const std::set<std::string> numericTypes = {
        "int", "u8", "u16", "u32", "u64", "int8", "int16", "int32", "int64",
        "uint8", "uint16", "uint32", "uint64", "float32", "float64"
    };
    return numericTypes.count(basic->name) > 0;
}

bool TypeChecker::isFloatType(const TypePtr& type) const {
    if (!type) return false;
    auto* basic = dynamic_cast<BasicType*>(type.get());
    return basic && (basic->name == "float" || basic->name == "float32" || basic->name == "float64" || basic->name == "double");
}

TypePtr TypeChecker::inferTypeFromLiteral(const LiteralExpr* lit) const {
    if (!lit) return nullptr;
    switch (lit->val.kind) {
        case ClassifyResult::Integer:  return std::make_shared<BasicType>("int");
        case ClassifyResult::Float:    return std::make_shared<BasicType>("float64");
        case ClassifyResult::String:   return std::make_shared<BasicType>("string");
        case ClassifyResult::Bool:     return std::make_shared<BasicType>("bool");
        case ClassifyResult::Char:     return std::make_shared<BasicType>("char");
        default:                       return nullptr;
    }
}

TypePtr TypeChecker::lookup(const std::string& name) {
    auto it = symbols_.find(name);
    return (it != symbols_.end()) ? it->second : nullptr;
}

TypePtr TypeChecker::getArithmeticResultType(const TokenType& op, const TypePtr& left, const TypePtr& right) const {
    (void)op;
    // String concatenation
    if (left && left->getName() == "string" || right && right->getName() == "string") {
        return std::make_shared<BasicType>("string");
    }
    if (isFloatType(left) || isFloatType(right)) return std::make_shared<BasicType>("float64");
    return std::make_shared<BasicType>("int");
}

void TypeChecker::error(const std::string& code, const std::string& message, int line, int col) {
    diag_.error(code, message, line, col);
}

void TypeChecker::typeCheckLet(const LetStmt* stmt) {
    if (!stmt) return;
    
    if (stmt->init) {
        typeCheckExpr(stmt->init);
    }
    
    auto initType = stmt->init ? stmt->init->type : nullptr;
    if (stmt->type) {
        // TYPE VALIDATION: Check if init type matches declared type
        if (initType) {
            std::string declTypeName = normalizeType(stmt->type->getName());
            std::string initTypeName = normalizeType(initType->getName());
            // Allow same types or numeric promotions (int -> float64)
            bool typesMatch = (declTypeName == initTypeName);
            bool numericPromotion = (declTypeName == "float64" && initTypeName == "int");
            if (!typesMatch && !numericPromotion) {
                error("E2001", "Type mismatch: expected " + stmt->type->getName() + 
                      " but got " + initType->getName(), stmt->line, stmt->col);
                return; // Don't register invalid type
            }
        }
        symbols_[stmt->name] = stmt->type;
    } else if (initType) {
        symbols_[stmt->name] = initType;
    }
    mutableVars_[stmt->name] = false;
}

void TypeChecker::typeCheckMut(const MutStmt* stmt) {
    if (!stmt) return;
    
    if (stmt->init) {
        typeCheckExpr(stmt->init);
    }
    
    auto initType = stmt->init ? stmt->init->type : nullptr;
    if (stmt->type) {
        // TYPE VALIDATION: Check if init type matches declared type
        if (initType) {
            std::string declTypeName = normalizeType(stmt->type->getName());
            std::string initTypeName = normalizeType(initType->getName());
            bool typesMatch = (declTypeName == initTypeName);
            bool numericPromotion = (declTypeName == "float64" && initTypeName == "int");
            if (!typesMatch && !numericPromotion) {
                error("E2001", "Type mismatch: expected " + stmt->type->getName() + 
                      " but got " + initType->getName(), stmt->line, stmt->col);
                return;
            }
        }
        symbols_[stmt->name] = stmt->type;
    } else if (initType) {
        symbols_[stmt->name] = initType;
    }
    mutableVars_[stmt->name] = true;
}

void TypeChecker::typeCheckExpr(const ExprPtr& expr) {
    if (!expr) return;
    if (expr->type) return;

    switch (expr->kind()) {
        case ExprKind::Literal: {
            auto* lit = static_cast<LiteralExpr*>(expr.get());
            expr->type = inferTypeFromLiteral(lit);
            break;
        }
        case ExprKind::Ident: {
            auto* ident = static_cast<IdentExpr*>(expr.get());
            if (symbols_.count(ident->name)) {
                expr->type = symbols_[ident->name];
            } else {
                expr->type = std::make_shared<BasicType>("unknown");
            }
            break;
        }
        case ExprKind::BinaryOp: {
            auto* bo = static_cast<BinaryOpExpr*>(expr.get());
            typeCheckExpr(bo->left);
            typeCheckExpr(bo->right);
            auto* leftType = bo->left->type.get();
            auto* rightType = bo->right->type.get();
            if (leftType && rightType) {
                std::string leftName = leftType->getName();
                std::string rightName = rightType->getName();
                
                // String concatenation
                if ((leftName == "string" && rightName == "string") ||
                    (leftName == "string" && rightName == "int") ||
                    (leftName == "int" && rightName == "string")) {
                    expr->type = std::make_shared<BasicType>("string");
                }
                // Boolean logic operators
                else if ((leftName == "bool" || leftName == "int") && 
                         (rightName == "bool" || rightName == "int") &&
                         (bo->op == TokenType::AND || bo->op == TokenType::OR)) {
                    expr->type = std::make_shared<BasicType>("bool");
                }
                // Numeric operations
                else if ((leftName == "int" || leftName == "float64") &&
                         (rightName == "int" || rightName == "float64")) {
                    TypePtr arithType = getArithmeticResultType(bo->op, bo->left->type, bo->right->type);
                    if (bo->op == TokenType::LT || bo->op == TokenType::GT || 
                        bo->op == TokenType::LTE || bo->op == TokenType::GTE ||
                        bo->op == TokenType::EQ || bo->op == TokenType::NEQ) {
                        expr->type = std::make_shared<BasicType>("bool");
                    } else {
                        expr->type = arithType;
                    }
                } else {
                    error("E2002", "Type mismatch in binary operation: " + leftName + " and " + rightName, bo->line, bo->col);
                    expr->type = std::make_shared<BasicType>("unknown");
                }
            }
            break;
        }
        case ExprKind::UnaryOp: {
            auto* uo = static_cast<UnaryOpExpr*>(expr.get());
            typeCheckExpr(uo->operand);
            expr->type = uo->operand->type;
            break;
        }
        case ExprKind::Call: {
            auto* call = static_cast<CallExpr*>(expr.get());
            if (call->callee) {
                auto* calleeIdent = dynamic_cast<IdentExpr*>(call->callee.get());
                if (calleeIdent) {
                    // Check for built-in functions
                    if (calleeIdent->name == "print" || calleeIdent->name == "println") {
                        for (auto& arg : call->args) typeCheckExpr(arg);
                        expr->type = std::make_shared<BasicType>("void");
                        break;
                    }
                    // Check for user-defined functions
                    if (fnRetTypes_.count(calleeIdent->name)) {
                        expr->type = fnRetTypes_[calleeIdent->name];
                        break;
                    }
                }
            }
            expr->type = std::make_shared<BasicType>("unknown");
            break;
        }
        case ExprKind::TypeCast: {
            auto* cast = static_cast<TypeCastExpr*>(expr.get());
            typeCheckExpr(cast->expr);
            expr->type = cast->targetType;
            break;
        }
        case ExprKind::ArrayLit: {
            auto* al = static_cast<ArrayLitExpr*>(expr.get());
            if (!al->elems.empty()) {
                typeCheckExpr(al->elems[0]);
                expr->type = std::make_shared<GenericType>("array", std::vector{al->elems[0]->type});
            }
            break;
        }
        case ExprKind::StructLit: {
            expr->type = std::make_shared<BasicType>("struct");
            break;
        }
        case ExprKind::FieldAccess: {
            auto* fa = static_cast<FieldAccessExpr*>(expr.get());
            typeCheckExpr(fa->obj);
            // Try to resolve field type from struct declaration
            if (fa->obj->type) {
                std::string objTypeName = fa->obj->type->getName();
                for (const auto* structDecl : structDecls_) {
                    if (structDecl->name == objTypeName) {
                        for (const auto& [fieldName, fieldType] : structDecl->fields) {
                            if (fieldName == fa->field) {
                                expr->type = fieldType;
                                return;
                            }
                        }
                    }
                }
            }
            expr->type = std::make_shared<BasicType>("unknown");
            break;
        }
        case ExprKind::Index: {
            auto* idx = static_cast<IndexExpr*>(expr.get());
            typeCheckExpr(idx->base);
            typeCheckExpr(idx->index);
            // Try to infer element type from array/vector base
            if (idx->base && idx->base->type) {
                std::string baseTypeName = normalizeType(idx->base->type->getName());
                if (baseTypeName == "array" || baseTypeName == "vector") {
                    auto genericType = std::dynamic_pointer_cast<GenericType>(idx->base->type);
                    if (genericType && !genericType->params.empty()) {
                        expr->type = genericType->params[0];
                    } else {
                        // For untyped 'vector' aliases, try to find the element type
                        // from the let/mut statement that declared the array
                        // This handles cases like: let nums: vector = [1, 2, 3]
                        // where the parser stores 'vector' as BasicType without params
                        for (auto& [name, type] : symbols_) {
                            if (name == "temp_array_elem_type") continue;
                            // Skip if this is the current base variable
                            // We need to find the init expression type of the declaration
                        }
                        // Fallback: look up in the scope for the base name
                        // and try to get element type from its init expression
                        // For now, use int as default element type for untyped vector
                        expr->type = std::make_shared<BasicType>("int");
                    }
                } else {
                    expr->type = std::make_shared<BasicType>("unknown");
                }
            } else {
                expr->type = std::make_shared<BasicType>("unknown");
            }
            break;
        }
        case ExprKind::StringInterp: {
            auto* si = static_cast<StringInterpExpr*>(expr.get());
            for (auto& part : si->parts) typeCheckExpr(part);
            expr->type = std::make_shared<BasicType>("string");
            break;
        }
        case ExprKind::Conditional: {
            auto* cond = static_cast<ConditionalExpr*>(expr.get());
            typeCheckExpr(cond->cond);
            typeCheckExpr(cond->thenValue);
            typeCheckExpr(cond->elseValue);
            expr->type = cond->thenValue->type;
            break;
        }
        case ExprKind::Range: {
            auto* range = static_cast<RangeExpr*>(expr.get());
            typeCheckExpr(range->start);
            typeCheckExpr(range->end);
            expr->type = std::make_shared<GenericType>("range", std::vector{range->start->type});
            break;
        }
        default:
            expr->type = std::make_shared<BasicType>("unknown");
            break;
    }
}

void TypeChecker::typeCheckReturn(const ReturnStmt* stmt, const FunctionDecl* fn) {
    (void)fn;
    if (!stmt) return;
    if (stmt->expr.has_value()) {
        typeCheckExpr(stmt->expr.value());
    }
}

void TypeChecker::checkStmt(const StmtPtr& stmt) {
    if (!stmt) return;
    switch (stmt->kind()) {
        case StmtKind::Let:      typeCheckLet(static_cast<LetStmt*>(stmt.get())); break;
        case StmtKind::Mut:      typeCheckMut(static_cast<MutStmt*>(stmt.get())); break;
        case StmtKind::Assign: {
            auto* assign = static_cast<AssignStmt*>(stmt.get());
            typeCheckExpr(assign->value);
            break;
        }
        case StmtKind::If: {
            auto* ifStmt = static_cast<IfStmt*>(stmt.get());
            typeCheckExpr(ifStmt->cond);
            checkStmt(ifStmt->thenBranch);
            if (ifStmt->elseBranch) checkStmt(ifStmt->elseBranch);
            break;
        }
        case StmtKind::While: {
            auto* whileStmt = static_cast<WhileStmt*>(stmt.get());
            typeCheckExpr(whileStmt->cond);
            checkStmt(whileStmt->body);
            break;
        }
        case StmtKind::For: {
            auto* forStmt = static_cast<ForStmt*>(stmt.get());
            if (forStmt->iterable) typeCheckExpr(forStmt->iterable);
            // Extract element type from array/vector for loop variable
            if (forStmt->iterable && forStmt->iterable->type) {
                std::string iterableTypeName = normalizeType(forStmt->iterable->type->getName());
                if (iterableTypeName == "array" || iterableTypeName == "vector") {
                    auto genericType = std::dynamic_pointer_cast<GenericType>(forStmt->iterable->type);
                    if (genericType && !genericType->params.empty()) {
                        symbols_[forStmt->varName] = genericType->params[0];
                        mutableVars_[forStmt->varName] = true;
                    } else {
                        // Fallback: try to infer from ArrayType
                        auto arrType = std::dynamic_pointer_cast<ArrayType>(forStmt->iterable->type);
                        if (arrType) {
                            symbols_[forStmt->varName] = arrType->elem;
                            mutableVars_[forStmt->varName] = true;
                        } else {
                            // Untyped 'vector' or 'array' - default to int
                            symbols_[forStmt->varName] = std::make_shared<BasicType>("int");
                            mutableVars_[forStmt->varName] = true;
                        }
                    }
                }
            }
            checkStmt(forStmt->body);
            break;
        }
        case StmtKind::Return:   typeCheckReturn(static_cast<ReturnStmt*>(stmt.get()), nullptr); break;
        case StmtKind::Expr: {
            auto* exprStmt = static_cast<ExprStmt*>(stmt.get());
            typeCheckExpr(exprStmt->expr);
            break;
        }
        case StmtKind::Block: {
            auto* block = static_cast<BlockStmt*>(stmt.get());
            for (auto& s : block->body) checkStmt(s);
            break;
        }
        case StmtKind::Match: {
            auto* match = static_cast<MatchExpr*>(stmt.get());
            typeCheckExpr(match->scrutinee);
            for (auto& arm : match->arms) {
                if (arm.pattern) typeCheckExpr(arm.pattern);
                if (arm.body) checkStmt(arm.body);
            }
            break;
        }
        default: break;
    }
}

void TypeChecker::typeCheckFn(const FunctionDecl* fn) {
    if (!fn) return;
    for (const auto& [name, type] : fn->params) {
        symbols_[name] = type;
        mutableVars_[name] = false;
    }
    for (auto& stmt : fn->body) checkStmt(stmt);
}

void TypeChecker::typeCheckFnParam(const std::string& name, TypePtr type, int line, int col) {
    (void)line; (void)col;
    symbols_[name] = type;
    mutableVars_[name] = false;
}

void TypeChecker::checkEnumDecls(const std::vector<StmtPtr>& items) {
    for (auto& item : items) {
        if (item->kind() == StmtKind::EnumDecl) enumDecls_.push_back(static_cast<EnumDecl*>(item.get()));
    }
}

void TypeChecker::checkStructDecls(const std::vector<StmtPtr>& items) {
    for (auto& item : items) {
        if (item->kind() == StmtKind::StructDecl) structDecls_.push_back(static_cast<StructDecl*>(item.get()));
    }
}

void TypeChecker::check(const std::vector<StmtPtr>& items) {
    symbols_.clear();
    mutableVars_.clear();
    checkEnumDecls(items);
    checkStructDecls(items);
    for (auto& item : items) {
        if (!item) continue;
        switch (item->kind()) {
            case StmtKind::FunctionDecl: {
                auto* fn = static_cast<FunctionDecl*>(item.get());
                declaredFns_.insert(fn->name);
                fnRetTypes_[fn->name] = fn->returnType;
                typeCheckFn(fn);
                break;
            }
            default: break;
        }
    }
}

} // namespace eng
