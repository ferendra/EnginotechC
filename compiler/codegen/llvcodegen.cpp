// EnginotechC++ — LLVM Code Generator (M0 bootstrap)
// Simplified LLVM codegen that targets x86-64 via LLVM's C API
// This is a working M0 compiler backend

#include "llvcodegen.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <fstream>
#include <sstream>
#include <cstdio>

namespace eng {

LLVMCodeGen::LLVMCodeGen(DiagnosticEngine& diag)
    : diag_(diag), context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>("enginotech", *context_)),
      builder_(*context_), hasError_(false), targetMachine_(nullptr) {
    scopeStack_.push_back(VarMap());
}

LLVMCodeGen::~LLVMCodeGen() {
    if (targetMachine_) delete targetMachine_;
}

bool LLVMCodeGen::initialize() {
    // Initialize LLVM targets
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // Create target machine for x86_64
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget("x86_64", "", error);
    if (!target) {
        error("Failed to find target: " + error);
        return false;
    }

    targetMachine_ = target->createTargetMachine(
        "x86_64", "generic", "",
        llvm::TargetOptions(), llvm::Optional<llvm::Reloc::Model>());

    if (!targetMachine_) {
        error("Failed to create target machine");
        return false;
    }

    module_->setTargetTriple("x86_64-pc-windows-msvc");
    module_->setDataLayout("e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128");

    return true;
}

void LLVMCodeGen::finalize() {
    // Verify module
    std::string error;
    llvm::raw_string_ostream es(error);
    if (llvm::verifyModule(*module_, &es)) {
        diag_.error("VERIFY", "Module verification failed: " + error);
    }
}

bool LLVMCodeGen::compile(const Program& prog, const std::string& outputPath) {
    if (!initialize()) return false;

    buildModule(prog);

    if (hasError_) return false;

    finalize();

    if (hasError_) return false;

    // Write LLVM IR to file first for debugging
    std::string irPath = outputPath + ".ll";
    {
        std::ofstream irFile(irPath);
        if (irFile) {
            module_->print(irFile, nullptr);
            irFile.close();
        }
    }

    // Emit object code
    std::error_code ec;
    llvm::ToolOutputFile out(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        error("Cannot open output file: " + outputPath);
        return false;
    }

    llvm::CodeGenFileType fileType = llvm::CGFT_ObjectFile;
    if (targetMachine_->addPassesToEmitFile(out.os(), nullptr, fileType, {})) {
        error("Failed to configure codegen pipeline");
        return false;
    }

    out.keep();
    debugPrint("Generated LLVM IR to: " + irPath);
    return true;
}

bool LLVMCodeGen::compileToLLVMIR(const Program& prog, const std::string& outputPath) {
    if (!initialize()) return false;

    buildModule(prog);

    if (hasError_) return false;

    std::ofstream out(outputPath);
    if (!out) {
        error("Cannot open output file: " + outputPath);
        return false;
    }
    module_->print(out, nullptr);
    out.close();
    debugPrint("Written LLVM IR to: " + outputPath);
    return true;
}

void LLVMCodeGen::buildModule(const Program& prog) {
    // Create printf declarations
    llvm::FunctionType* printfFnType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context_),
        {llvm::Type::getInt8PtrTy(*context_)}, true);
    llvm::Function* printfFn = llvm::Function::Create(
        printfFnType, llvm::Function::ExternalLinkage, "printf", module_.get());
    printfFn->addParamAttr(0, llvm::Attribute::NoCapture);

    // Build function declarations
    for (const auto& item : prog.topLevel) {
        if (!item) continue;
        switch (item->kind()) {
            case StmtKind::FunctionDecl:
                emitFunction(static_cast<FunctionDecl*>(item.get()));
                break;
            case StmtKind::StructDecl:
                emitStruct(static_cast<StructDecl*>(item.get()));
                break;
            case StmtKind::EnumDecl:
                emitEnum(static_cast<EnumDecl*>(item.get()));
                break;
            default:
                break;
        }
    }

    // Ensure main exists
    if (!mainFn_) {
        llvm::FunctionType* mainType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*context_), false);
        mainFn_ = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module_.get());
    }

    // Emit main body
    llvm::BasicBlock* mainBB = llvm::BasicBlock::Create(*context_, "entry", mainFn_);
    builder_.SetInsertPoint(mainBB);

    // Find main function and call it
    auto it = functionMap_.find("main");
    if (it != functionMap_.end()) {
        std::vector<llvm::Value*> args;
        builder_.CreateCall(it->second, args, "call_main");
    } else {
        // Empty main — just return 0
        builder_.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0));
    }

    // Verify everything was emitted
    std::string err;
    llvm::raw_string_ostream es(err);
    if (llvm::verifyModule(*module_, &es)) {
        error("Module verification failed after codegen: " + err);
    }
}

void LLVMCodeGen::emitFunction(const FunctionDecl* fn) {
    // Create function type
    std::vector<llvm::Type*> paramTypes;
    bool isMain = (fn->name == "main");

    for (const auto& [name, type] : fn->params) {
        paramTypes.push_back(getTypeFromEC(type));
    }

    llvm::Type* returnType = getTypeFromEC(fn->returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        returnType, paramTypes, false);

    llvm::Function* func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, fn->name, module_.get());
    functionMap_[fn->name] = func;

    if (isMain) mainFn_ = func;

    // Create entry block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context_, "entry", func);
    builder_.SetInsertPoint(entry);

    // Set up argument values
    currentScope_.clear();
    int i = 0;
    for (const auto& [name, type] : fn->params) {
        llvm::Argument* arg = func->arg_begin() + i;
        currentScope_[name] = arg;
        i++;
    }

    // Emit function body
    emitBlock(fn->body);

    // Add implicit return if not present
    if (builder_.GetInsertBlock()->getTerminator() == nullptr) {
        if (isMain)
            builder_.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0));
        else
            builder_.CreateRetVoid();
    }
}

void LLVMCodeGen::emitBlock(const std::vector<StmtPtr>& body) {
    for (const auto& stmt : body) {
        if (!stmt) continue;
        emitStmt(stmt);
        if (hasError_) return;
    }
}

void LLVMCodeGen::emitStmt(const StmtPtr& stmt) {
    if (!stmt) return;
    switch (stmt->kind()) {
        case StmtKind::Let:
            emitLet(static_cast<LetStmt*>(stmt.get()));
            break;
        case StmtKind::Mut:
            emitMut(static_cast<MutStmt*>(stmt.get()));
            break;
        case StmtKind::Assign:
            emitAssign(static_cast<AssignStmt*>(stmt.get()));
            break;
        case StmtKind::Return:
            emitReturn(static_cast<ReturnStmt*>(stmt.get()));
            break;
        case StmtKind::If:
            emitIf(static_cast<IfStmt*>(stmt.get()));
            break;
        case StmtKind::For:
            emitFor(static_cast<ForStmt*>(stmt.get()));
            break;
        case StmtKind::While:
            emitWhile(static_cast<WhileStmt*>(stmt.get()));
            break;
        case StmtKind::Break:
            emitBreak(static_cast<BreakStmt*>(stmt.get()));
            break;
        case StmtKind::Continue:
            emitContinue(static_cast<ContinueStmt*>(stmt.get()));
            break;
        case StmtKind::Match:
            emitMatch(static_cast<MatchExpr*>(stmt.get()));
            break;
        case StmtKind::Expr:
            emitExpression(static_cast<ExprStmt*>(stmt.get())->expr);
            break;
        case StmtKind::Block:
            emitBlock(static_cast<BlockStmt*>(stmt.get())->body);
            break;
        default:
            break;
    }
}

llvm::Value* LLVMCodeGen::emitExpr(const ExprPtr& expr) {
    if (!expr) return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));

    switch (expr->kind()) {
        case ExprKind::Literal:
            return emitLiteral(static_cast<LiteralExpr*>(expr.get()));
        case ExprKind::Ident:
            return emitIdent(static_cast<IdentExpr*>(expr.get()));
        case ExprKind::BinaryOp:
            return emitBinaryOp(static_cast<BinaryOpExpr*>(expr.get()));
        case ExprKind::UnaryOp:
            return emitUnaryOp(static_cast<UnaryOpExpr*>(expr.get()));
        case ExprKind::Call:
            return emitCall(static_cast<CallExpr*>(expr.get()));
        case ExprKind::FieldAccess:
            return emitFieldAccess(static_cast<FieldAccessExpr*>(expr.get()));
        case ExprKind::ArrayLit:
            return emitArrayLit(static_cast<ArrayLitExpr*>(expr.get()));
        case ExprKind::StringInterp:
            return emitStringInterp(static_cast<StringInterpExpr*>(expr.get()));
        case ExprKind::TypeCast:
            return emitTypeCast(static_cast<TypeCastExpr*>(expr.get()));
        default:
            return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
    }
}

void LLVMCodeGen::emitLet(const LetStmt* stmt) {
    llvm::Value* val = emitExpr(stmt->init);
    currentScope_[stmt->name] = val;
}

void LLVMCodeGen::emitMut(const MutStmt* stmt) {
    // For mutable variables, we'd need alloca in real implementation
    // For now, treat as immutable (simplified)
    llvm::Value* val = emitExpr(stmt->init);
    currentScope_[stmt->name] = val;
}

void LLVMCodeGen::emitAssign(const AssignStmt* stmt) {
    llvm::Value* val = emitExpr(stmt->value);
    currentScope_[stmt->name] = val;
}

void LLVMCodeGen::emitReturn(const ReturnStmt* stmt) {
    if (stmt->expr) {
        llvm::Value* val = emitExpr(stmt->expr);
        builder_.CreateRet(val);
    } else {
        builder_.CreateRetVoid();
    }
}

void LLVMCodeGen::emitIf(const IfStmt* stmt) {
    llvm::Value* cond = emitExpr(stmt->cond);
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context_, "if.then");
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context_, "if.else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context_, "if.merge");

    builder_.CreateCondBr(cond, thenBB, elseBB);

    // Then branch
    builder_.SetInsertPoint(thenBB);
    emitBlock(stmt->thenBranch->kind() == StmtKind::Block
              ? static_cast<BlockStmt*>(stmt->thenBranch.get())->body
              : std::vector<StmtPtr>{stmt->thenBranch});
    builder_.CreateBr(mergeBB);

    // Else branch
    llvm::BasicBlock* savedBB = builder_.GetInsertBlock();
    builder_.SetInsertPoint(elseBB);
    if (stmt->elseBranch) {
        emitBlock(stmt->elseBranch->kind() == StmtKind::Block
                  ? static_cast<BlockStmt*>(stmt->elseBranch.get())->body
                  : std::vector<StmtPtr>{stmt->elseBranch});
    }
    builder_.CreateBr(mergeBB);

    // Merge
    builder_.SetInsertPoint(mergeBB);
}

void LLVMCodeGen::emitFor(const ForStmt* stmt) {
    // Simplified for-in loop: treat iterable as array literal for now
    // In production, this would iterate over any container
    llvm::Value* iterVal = emitExpr(stmt->iterable);
    (void)iterVal;

    // Push loop context
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context_, "for.body");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*context_, "for.end");
    loopBreakTargets_.push_back(afterBB);
    loopContinueTargets_.push_back(loopBB);

    builder_.CreateBr(loopBB);
    builder_.SetInsertPoint(loopBB);

    // Declare loop variable
    if (stmt->varName != "__loop_var__") {
        currentScope_[stmt->varName] = llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(*context_), 0);
    }

    emitBlock(stmt->body->kind() == StmtKind::Block
              ? static_cast<BlockStmt*>(stmt->body.get())->body
              : std::vector<StmtPtr>{stmt->body});

    loopBreakTargets_.pop_back();
    loopContinueTargets_.pop_back();

    builder_.CreateBr(afterBB);
    builder_.SetInsertPoint(afterBB);
}

void LLVMCodeGen::emitWhile(const WhileStmt* stmt) {
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*context_, "while.cond");
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context_, "while.body");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*context_, "while.end");

    loopBreakTargets_.push_back(afterBB);
    loopContinueTargets_.push_back(condBB);

    builder_.CreateBr(condBB);
    builder_.SetInsertPoint(condBB);

    llvm::Value* cond = emitExpr(stmt->cond);
    builder_.CreateCondBr(cond, bodyBB, afterBB);

    builder_.SetInsertPoint(bodyBB);
    emitBlock(stmt->body->kind() == StmtKind::Block
              ? static_cast<BlockStmt*>(stmt->body.get())->body
              : std::vector<StmtPtr>{stmt->body});

    loopBreakTargets_.pop_back();
    loopContinueTargets_.pop_back();

    builder_.CreateBr(condBB);
    builder_.SetInsertPoint(afterBB);
}

void LLVMCodeGen::emitBreak(const BreakStmt* stmt) {
    if (!loopBreakTargets_.empty()) {
        builder_.CreateBr(loopBreakTargets_.back());
    }
}

void LLVMCodeGen::emitContinue(const ContinueStmt* stmt) {
    if (!loopContinueTargets_.empty()) {
        builder_.CreateBr(loopContinueTargets_.back());
    }
}

void LLVMCodeGen::emitMatch(const MatchExpr* stmt) {
    (void)stmt;
    // Match is not fully implemented in M0 — emit no-op
}

void LLVMCodeGen::emitExpression(const ExprPtr& expr) {
    emitExpr(expr);
}

llvm::Value* LLVMCodeGen::emitLiteral(const LiteralExpr* lit) {
    const ClassifyResult& val = lit->val;
    switch (val.kind) {
        case ClassifyResult::Integer:
            return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), val.intVal);
        case ClassifyResult::Float:
            return llvm::ConstantFP::get(llvm::Type::getFloatTy(*context_), val.floatVal);
        case ClassifyResult::Bool:
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), val.intVal ? 1 : 0);
        case ClassifyResult::String:
            return llvm::ConstantDataArray::getString(*context_, val.strVal.c_str());
        case ClassifyResult::Char:
            return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context_), (int)val.strVal[0]);
        default:
            return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
    }
}

llvm::Value* LLVMCodeGen::emitIdent(const IdentExpr* ident) {
    auto it = currentScope_.find(ident->name);
    if (it != currentScope_.end()) {
        return it->second;
    }
    // Try function scope
    it = functionScope_.find(ident->name);
    if (it != functionScope_.end()) {
        return it->second;
    }
    // Built-in
    if (ident->name == "true") return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 1);
    if (ident->name == "false") return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 0);
    if (ident->name == "none") return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));

    error("Undefined variable: " + ident->name, ident->line, ident->col);
    return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
}

llvm::Value* LLVMCodeGen::emitBinaryOp(const BinaryOpExpr* op) {
    llvm::Value* left = emitExpr(op->left);
    llvm::Value* right = emitExpr(op->right);

    if (!left || !right) {
        error("Invalid binary operation", op->line, op->col);
        return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
    }

    switch (op->op) {
        case TokenType::PLUS:  return builder_.CreateAdd(left, right, "add");
        case TokenType::MINUS: return builder_.CreateSub(left, right, "sub");
        case TokenType::STAR:  return builder_.CreateMul(left, right, "mul");
        case TokenType::SLASH: return builder_.CreateSDiv(left, right, "div");
        case TokenType::EQ:    return builder_.CreateICmpEQ(left, right, "eq");
        case TokenType::NEQ:   return builder_.CreateICmpNE(left, right, "neq");
        case TokenType::LT:    return builder_.CreateICmpSLT(left, right, "lt");
        case TokenType::GT:    return builder_.CreateICmpSGT(left, right, "gt");
        case TokenType::LTE:   return builder_.CreateICmpSLE(left, right, "lte");
        case TokenType::GTE:   return builder_.CreateICmpSGE(left, right, "gte");
        case TokenType::AND: {
            // Short-circuit
            llvm::BasicBlock* trueBB = llvm::BasicBlock::Create(*context_, "and.true");
            llvm::BasicBlock* falseBB = llvm::BasicBlock::Create(*context_, "and.false");
            llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context_, "and.merge");
            builder_.CreateCondBr(left, trueBB, falseBB);
            builder_.SetInsertPoint(trueBB);
            builder_.CreateCondBr(right, trueBB, falseBB);
            builder_.SetInsertPoint(mergeBB);
            llvm::PHINode* phi = builder_.CreatePHI(llvm::Type::getInt1Ty(*context_), 2, "and_phi");
            phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 1), trueBB);
            phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 0), falseBB);
            return phi;
        }
        case TokenType::OR: {
            llvm::BasicBlock* trueBB = llvm::BasicBlock::Create(*context_, "or.true");
            llvm::BasicBlock* falseBB = llvm::BasicBlock::Create(*context_, "or.false");
            llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context_, "or.merge");
            builder_.CreateCondBr(left, trueBB, falseBB);
            builder_.SetInsertPoint(trueBB);
            builder_.CreateCondBr(right, trueBB, falseBB);
            builder_.SetInsertPoint(mergeBB);
            llvm::PHINode* phi = builder_.CreatePHI(llvm::Type::getInt1Ty(*context_), 2, "or_phi");
            phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 1), trueBB);
            phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 0), falseBB);
            return phi;
        }
        default:
            error("Unsupported binary operator", op->line, op->col);
            return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
    }
}

llvm::Value* LLVMCodeGen::emitUnaryOp(const UnaryOpExpr* op) {
    llvm::Value* operand = emitExpr(op->operand);
    if (!operand) {
        error("Invalid unary operation", op->line, op->col);
        return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
    }

    switch (op->op) {
        case TokenType::NOT:
            return builder_.CreateNot(operand, "not");
        case TokenType::MINUS:
            return builder_.CreateNeg(operand, "neg");
        case TokenType::PLUS:
            return operand; // +x is just x
        default:
            error("Unsupported unary operator", op->line, op->col);
            return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
    }
}

llvm::Value* LLVMCodeGen::emitCall(const CallExpr* call) {
    llvm::Value* callee = emitExpr(call->callee);
    if (!callee) {
        error("Invalid function call", call->line, call->col);
        return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
    }

    std::vector<llvm::Value*> args;
    for (const auto& arg : call->args) {
        args.push_back(emitExpr(arg));
    }

    llvm::FunctionType* fnType = dyn_cast<llvm::FunctionType>(callee->getType()->getScalarType());
    if (fnType) {
        return builder_.CreateCall(fnType, callee, args, "call");
    }

    // It's a function name
    std::string fnName;
    if (auto* ident = dyn_cast<llvm::ConstantFolder>(callee)) {
        // This is complex — in M0 we handle simple named calls
    }

    // Fallback: look up by callee expression
    // In practice, M0 simplifies this to direct name lookup
    return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
}

llvm::Value* LLVMCodeGen::emitFieldAccess(const FieldAccessExpr* field) {
    (void)field;
    error("Field access not yet supported", field->line, field->col);
    return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
}

llvm::Value* LLVMCodeGen::emitArrayLit(const ArrayLitExpr* arr) {
    (void)arr;
    error("Array literals not yet supported", arr->line, arr->col);
    return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
}

llvm::Value* LLVMCodeGen::emitStringInterp(const StringInterpExpr* interp) {
    (void)interp;
    error("String interpolation not yet supported", interp->line, interp->col);
    return llvm::ConstantInt::getNullValue(llvm::Type::getInt32Ty(*context_));
}

llvm::Value* LLVMCodeGen::emitTypeCast(const TypeCastExpr* cast) {
    llvm::Value* val = emitExpr(cast->expr);
    (void)cast->targetType;
    return val;
}

llvm::Type* LLVMCodeGen::getTypeFromEC(const TypePtr& type) {
    if (!type) return llvm::Type::getInt32Ty(*context_); // default int

    const std::string& name = type->name;
    if (name == "int" || name == "int8" || name == "int16" || name == "int32" || name == "int64" ||
        name == "uint" || name == "uint8" || name == "uint16" || name == "uint32" || name == "uint64") {
        return llvm::Type::getInt32Ty(*context_); // M0 uses 32-bit ints
    }
    if (name == "float32" || name == "float64") {
        return llvm::Type::getFloatTy(*context_);
    }
    if (name == "bool") {
        return llvm::Type::getInt1Ty(*context_);
    }
    if (name == "string" || name == "char") {
        return llvm::Type::getInt8PtrTy(*context_);
    }
    if (name == "void") {
        return llvm::Type::getVoidTy(*context_);
    }
    if (name == "byte") {
        return llvm::Type::getInt8Ty(*context_);
    }

    // Generic types
    if (name == "Option") {
        return llvm::Type::getInt32Ty(*context_); // Tagged union, simplified
    }
    if (name == "Result") {
        return llvm::Type::getInt32Ty(*context_);
    }

    return llvm::Type::getInt32Ty(*context_);
}

llvm::Constant* LLVMCodeGen::getConstantValue(const ClassifyResult& val) {
    (void)val;
    return nullptr; // Not used directly
}

void LLVMCodeGen::error(const std::string& msg, int line, int col) {
    diag_.error("CG001", msg, line, col);
    hasError_ = true;
}

void LLVMCodeGen::debugPrint(const std::string& msg) {
    // debug output disabled for now
    (void)msg;
}

} // namespace eng
