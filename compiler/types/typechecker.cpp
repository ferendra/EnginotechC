#include "../types/typechecker.h"
#include <set>

namespace eng {

TypeChecker::TypeChecker(DiagnosticEngine& diag, bool dynamicTyping) : diag_(diag), dynamicTyping_(dynamicTyping) {}

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

bool TypeChecker::isEnumType(const TypePtr& type) const {
    if (!type) return false;
    auto* basic = dynamic_cast<BasicType*>(type.get());
    if (!basic) return false;
    for (const auto* enumDecl : enumDecls_) {
        if (enumDecl->name == basic->name) return true;
    }
    return false;
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
            } else if (isEnumType(std::make_shared<BasicType>(ident->name))) {
                // Enum type name used as value (for variant access)
                expr->type = std::make_shared<BasicType>(ident->name);
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
                
                // Allow comparison of same enum types
                if (leftName == rightName && isEnumType(leftType)) {
                    if (bo->op == TokenType::EQ || bo->op == TokenType::NEQ) {
                        expr->type = std::make_shared<BasicType>("bool");
                    } else {
                        error("E2002", "Type mismatch in binary operation: " + leftName + " and " + rightName, bo->line, bo->col);
                        expr->type = std::make_shared<BasicType>("unknown");
                    }
                    break;
                }
                
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
                    // Built-in functions with fixed return types
                    {
                        static const std::unordered_map<std::string, std::string> builtinRet = {
                            {"str", "string"},   {"input", "string"},
                            {"chr", "string"},   {"read_line", "string"},
                            {"len", "int"},      {"abs", "int"},
                            {"min", "int"},      {"max", "int"},
                            {"sqrt", "float64"}, {"pow", "float64"},
                            {"floor", "float64"},{"ceil", "float64"},
                        };
                        auto bit = builtinRet.find(calleeIdent->name);
                        if (bit != builtinRet.end()) {
                            for (auto& arg : call->args) typeCheckExpr(arg);
                            expr->type = std::make_shared<BasicType>(bit->second);
                            break;
                        }
                    }
                    // Check for user-defined functions
                    if (fnRetTypes_.count(calleeIdent->name)) {
                        TypePtr retType = fnRetTypes_[calleeIdent->name];
                        // If return type has type params, try to infer from args
                        if (!typeParamScopes_.empty()) {
                            retType = substituteTypeParams(retType, typeParamScopes_.back());
                        }
                        expr->type = retType;
                        for (auto& arg : call->args) typeCheckExpr(arg);
                        break;
                    }
                }
            }
            for (auto& arg : call->args) typeCheckExpr(arg);
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
            auto* sl = static_cast<StructLitExpr*>(expr.get());
            if (!sl->structName.empty()) {
                for (const auto* structDecl : structDecls_) {
                    if (structDecl->name == sl->structName) {
                        if (!structDecl->typeParams.empty()) {
                            // Infer type args from field types
                            std::unordered_map<std::string, TypePtr> typeArgs;
                            for (const auto& [fieldName, fieldExpr] : sl->fields) {
                                for (const auto& [declFieldName, declFieldType] : structDecl->fields) {
                                    if (declFieldName == fieldName) {
                                        typeCheckExpr(fieldExpr);
                                        if (fieldExpr->type) {
                                            // Infer type params from field types
                                            inferTypeParams(declFieldType, fieldExpr->type, typeArgs);
                                        }
                                    }
                                }
                            }
                            // Build type args vector in order of typeParams
                            std::vector<TypePtr> inferredArgs;
                            for (const auto& tp : structDecl->typeParams) {
                                if (auto* typeParam = dynamic_cast<TypeParam*>(tp.get())) {
                                    auto it = typeArgs.find(typeParam->name);
                                    if (it != typeArgs.end()) {
                                        inferredArgs.push_back(it->second);
                                    } else {
                                        // Default to int if not inferred
                                        inferredArgs.push_back(std::make_shared<BasicType>("int"));
                                    }
                                }
                            }
                            expr->type = std::make_shared<GenericType>(sl->structName, inferredArgs);
                        } else {
                            expr->type = std::make_shared<BasicType>(sl->structName);
                        }
                        break;
                    }
                }
            } else {
                expr->type = std::make_shared<BasicType>("struct");
            }
            break;
        }
        case ExprKind::FieldAccess: {
            auto* fa = static_cast<FieldAccessExpr*>(expr.get());
            typeCheckExpr(fa->obj);
            // Try to resolve field type from struct declaration
            if (fa->obj->type) {
                std::string objTypeName = fa->obj->type->getName();
                // Check if it's a generic type like Vec<T>
                TypePtr objType = fa->obj->type;
                std::unordered_map<std::string, TypePtr> typeArgs;
                
                if (auto* genericType = dynamic_cast<GenericType*>(fa->obj->type.get())) {
                    objTypeName = genericType->name;
                    // Extract type arguments for substitution
                    for (size_t i = 0; i < genericType->params.size(); ++i) {
                        // Find the corresponding type param from struct declaration
                        for (const auto* structDecl : structDecls_) {
                            if (structDecl->name == objTypeName && i < structDecl->typeParams.size()) {
                                if (auto* tp = dynamic_cast<TypeParam*>(structDecl->typeParams[i].get())) {
                                    typeArgs[tp->name] = genericType->params[i];
                                }
                            }
                        }
                    }
                
                // Check if it's an enum type
                for (const auto* enumDecl : enumDecls_) {
                    if (enumDecl->name == objTypeName) {
                        // Look for enum variant
                        for (const auto& [variantName, variantPayload] : enumDecl->variants) {
                            if (variantName == fa->field) {
                                // Enum variant - type is the enum itself
                                expr->type = std::make_shared<BasicType>(enumDecl->name);
                                return;
                            }
                        }
                    }
                
                for (const auto* structDecl : structDecls_) {
                    if (structDecl->name == objTypeName) {
                        for (const auto& [fieldName, fieldType] : structDecl->fields) {
                            if (fieldName == fa->field) {
                                // Substitute type params if needed
                                if (!typeArgs.empty()) {
                                    expr->type = substituteTypeParams(fieldType, typeArgs);
                                } else {
                                    expr->type = fieldType;
                                }
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
                    // String indexing yields a char (compared/used as int in codegen)
                    if (baseTypeName == "string") {
                        expr->type = std::make_shared<BasicType>("int");
                    } else {
                        expr->type = std::make_shared<BasicType>("unknown");
                    }
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
        case ExprKind::Try: {
            auto* te = static_cast<TryExpr*>(expr.get());
            typeCheckExpr(te->inner);
            if (!te->inner->type) {
                expr->type = std::make_shared<BasicType>("unknown");
                break;
            }
            std::string innerName = te->inner->type->getName();
            if (auto* gen = dynamic_cast<GenericType*>(te->inner->type.get())) {
                if (gen->name == "Result" && gen->params.size() == 2) {
                    expr->type = gen->params[0]; // T of Result<T,E>
                    break;
                }
                if (gen->name == "Option" && gen->params.size() == 1) {
                    expr->type = gen->params[0]; // T of Option<T>
                    break;
                }
            }
            if (innerName == "Result" || innerName == "Option") {
                error("E3001", "`?` requires Result<T,E> or Option<T>", te->line, te->col);
            } else {
                error("E3001", "`?` can only be used on Result or Option, got " + innerName, te->line, te->col);
            }
            expr->type = std::make_shared<BasicType>("unknown");
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
                } else {
                    // Range loop (for i in 0..n) — the variable is an integer
                    symbols_[forStmt->varName] = std::make_shared<BasicType>("int");
                    mutableVars_[forStmt->varName] = true;
                }
            } else {
                // No iterable type info — assume integer loop variable
                symbols_[forStmt->varName] = std::make_shared<BasicType>("int");
                mutableVars_[forStmt->varName] = true;
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
                if (arm.guard) typeCheckExpr(arm.guard);
                if (arm.body) checkStmt(arm.body);
            }
            break;
        }
        default: break;
    }
}

void TypeChecker::typeCheckFn(const FunctionDecl* fn, bool dynamicTyping) {
    if (!fn) return;
    
    // Handle generic functions: push type parameters to scope
    if (!fn->typeParams.empty()) {
        pushTypeParams(fn->typeParams);
    }
    
    for (const auto& [name, type] : fn->params) {
        TypePtr substitutedType = substituteTypeParams(type, typeParamScopes_.empty() ? std::unordered_map<std::string, TypePtr>{} : typeParamScopes_.back());
        symbols_[name] = substitutedType;
        mutableVars_[name] = false;
    }
    for (auto& stmt : fn->body) checkStmt(stmt);
    
    if (!fn->typeParams.empty()) {
        popTypeParams();
    }
    
    // In dynamic typing mode, allow nullptr return type (no explicit -> type)
    if (dynamicTyping && fn->returnType == nullptr) {
        // Allow - function can have dynamic return type
    }
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
                typeCheckFn(fn, dynamicTyping_);
                break;
            }
            default: break;
        }
    }
}

// Generics support implementation
TypePtr TypeChecker::substituteTypeParams(const TypePtr& type, const std::unordered_map<std::string, TypePtr>& typeArgs) {
    if (!type) return type;
    
    if (auto* typeParam = dynamic_cast<TypeParam*>(type.get())) {
        auto it = typeArgs.find(typeParam->name);
        if (it != typeArgs.end()) {
            return it->second;
        }
        return type; // Unresolved type param
    }
    
    if (auto* genericType = dynamic_cast<GenericType*>(type.get())) {
        std::vector<TypePtr> newParams;
        for (const auto& param : genericType->params) {
            newParams.push_back(substituteTypeParams(param, typeArgs));
        }
        return std::make_shared<GenericType>(genericType->name, newParams);
    }
    
    if (auto* arrayType = dynamic_cast<ArrayType*>(type.get())) {
        return std::make_shared<ArrayType>(substituteTypeParams(arrayType->elem, typeArgs), arrayType->size);
    }
    
    if (auto* fnType = dynamic_cast<FnType*>(type.get())) {
        TypePtr newRet = substituteTypeParams(fnType->ret, typeArgs);
        std::vector<std::pair<std::string, TypePtr>> newParams;
        for (const auto& [name, paramType] : fnType->params) {
            newParams.emplace_back(name, substituteTypeParams(paramType, typeArgs));
        }
        return std::make_shared<FnType>(newRet, newParams);
    }
    
    return type;
}

void TypeChecker::pushTypeParams(const std::vector<TypePtr>& typeParams) {
    std::unordered_map<std::string, TypePtr> scope;
    for (const auto& tp : typeParams) {
        if (auto* typeParam = dynamic_cast<TypeParam*>(tp.get())) {
            scope[typeParam->name] = tp; // Store the TypeParam itself for now
        }
    }
    typeParamScopes_.push_back(std::move(scope));
}

void TypeChecker::popTypeParams() {
    if (!typeParamScopes_.empty()) {
        typeParamScopes_.pop_back();
    }
}

// Helper: infer type parameters from expected vs actual type
void TypeChecker::inferTypeParams(const TypePtr& expected, const TypePtr& actual, std::unordered_map<std::string, TypePtr>& typeArgs) {
    if (!expected || !actual) return;
    
    if (auto* typeParam = dynamic_cast<TypeParam*>(expected.get())) {
        typeArgs[typeParam->name] = actual;
        return;
    }
    
    if (auto* expGeneric = dynamic_cast<GenericType*>(expected.get())) {
        if (auto* actGeneric = dynamic_cast<GenericType*>(actual.get())) {
            if (expGeneric->name == actGeneric->name && expGeneric->params.size() == actGeneric->params.size()) {
                for (size_t i = 0; i < expGeneric->params.size(); ++i) {
                    inferTypeParams(expGeneric->params[i], actGeneric->params[i], typeArgs);
                }
            }
        }
        return;
    }
    
    if (auto* expArray = dynamic_cast<ArrayType*>(expected.get())) {
        if (auto* actArray = dynamic_cast<ArrayType*>(actual.get())) {
            inferTypeParams(expArray->elem, actArray->elem, typeArgs);
        }
        return;
    }
    
    if (auto* expFn = dynamic_cast<FnType*>(expected.get())) {
        if (auto* actFn = dynamic_cast<FnType*>(actual.get())) {
            inferTypeParams(expFn->ret, actFn->ret, typeArgs);
            for (size_t i = 0; i < expFn->params.size() && i < actFn->params.size(); ++i) {
                inferTypeParams(expFn->params[i].second, actFn->params[i].second, typeArgs);
            }
        }
        return;
    }
}

} // namespace eng
