#ifndef ENGCG_LLVCGEN_H
#define ENGCG_LLVCGEN_H

#include "../ast/nodes.h"
#include "../lexer/lexer.h"
#include "../diagnostics/diagnostic.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JITLinkDebugInfoEmitter.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Support/Signals.h>
#include <llvm/Config/llvm-config.h>

namespace eng {

class LLVMCodeGen {
public:
    LLVMCodeGen(DiagnosticEngine& diag);
    ~LLVMCodeGen();

    bool compile(const Program& prog, const std::string& outputPath);
    bool compileToLLVMIR(const Program& prog, const std::string& outputPath);

private:
    using FunctionMap = std::map<std::string, llvm::Function*>;
    using VarMap = std::map<std::string, llvm::Value*>;

    // Core methods
    bool initialize();
    void finalize();
    llvm::Module* getModule() { return module_.get(); }
    llvm::LLVMContext& getContext() { return context_; }
    llvm::IRBuilder<> builder_;

    // Expression visitor
    llvm::Value* emitExpr(const ExprPtr& expr);
    llvm::Value* emitLiteral(const LiteralExpr* lit);
    llvm::Value* emitIdent(const IdentExpr* ident);
    llvm::Value* emitBinaryOp(const BinaryOpExpr* op);
    llvm::Value* emitUnaryOp(const UnaryOpExpr* op);
    llvm::Value* emitCall(const CallExpr* call);
    llvm::Value* emitFieldAccess(const FieldAccessExpr* field);
    llvm::Value* emitArrayLit(const ArrayLitExpr* arr);
    llvm::Value* emitStringInterp(const StringInterpExpr* interp);
    llvm::Value* emitTypeCast(const TypeCastExpr* cast);

    // Statement visitor
    void emitStmt(const StmtPtr& stmt);
    void emitBlock(const std::vector<StmtPtr>& body);
    void emitLet(const LetStmt* stmt);
    void emitMut(const MutStmt* stmt);
    void emitAssign(const AssignStmt* stmt);
    void emitReturn(const ReturnStmt* stmt);
    void emitIf(const IfStmt* stmt);
    void emitFor(const ForStmt* stmt);
    void emitWhile(const WhileStmt* stmt);
    void emitBreak(const BreakStmt* stmt);
    void emitContinue(const ContinueStmt* stmt);
    void emitMatch(const MatchExpr* stmt);
    void emitFunction(const FunctionDecl* fn);
    void emitStruct(const StructDecl* st);
    void emitEnum(const EnumDecl* en);
    void emitImpl(const ImplDecl* im);
    void emitImport(const ImportStmt* im);
    void emitConst(const ConstStmt* co);
    void emitExpression(const ExprPtr& expr);

    // Helpers
    llvm::Type* getTypeFromEC(const TypePtr& type);
    llvm::Constant* getConstantValue(const ClassifyResult& val);
    llvm::Value* getOrCreateVar(const std::string& name, llvm::Value* init = nullptr);
    llvm::Value* getVar(const std::string& name);
    void setVar(const std::string& name, llvm::Value* val);

    // Build module
    void buildFunctionDecls(const Program& prog);
    void buildModule(const Program& prog);

    // Debug / diagnostic
    void error(const std::string& msg, int line = 0, int col = 0);
    void debugPrint(const std::string& msg);

    // Member data
    DiagnosticEngine& diag_;
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::Function> mainFn_;
    FunctionMap functionMap_;
    VarMap currentScope_;
    VarMap functionScope_;
    std::vector<VarMap> scopeStack_;
    llvm::BasicBlock* currentBlock_;
    std::vector<llvm::BasicBlock*> loopBreakTargets_;
    std::vector<llvm::BasicBlock*> loopContinueTargets_;
    bool hasError_;
    llvm::TargetMachine* targetMachine_;
};

} // namespace eng

#endif // ENGCG_LLVCGEN_H
