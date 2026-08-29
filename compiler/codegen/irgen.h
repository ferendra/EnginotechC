// EnginotechC++ — LLVM IR Text Generator (M1)
// Generates LLVM IR as text, then uses clang/zig cc to compile to native.
// This avoids requiring the full LLVM C++ API.

#ifndef ENGCG_IRGEN_H
#define ENGCG_IRGEN_H

#include "../ast/nodes.h"
#include "../lexer/lexer.h"
#include "../diagnostics/diagnostic.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <memory>

namespace eng {

// Runtime value produced by expression emission
enum class VT { Int, Float, Bool, Str, Void, StructPtr, ArrayPtr, Unknown };

struct Val {
    std::string ref;      // SSA register ("%3") or literal text ("5")
    VT type = VT::Int;
    std::string typeName; // struct name / array element name when applicable
};

class LLVMIRGenerator {
public:
    explicit LLVMIRGenerator(DiagnosticEngine& diag);

    // Generate LLVM IR text for the program
    std::string generate(const Program& prog);

    // Write IR to file and compile with clang
    bool compile(const Program& prog, const std::string& outputPath, const std::string& clangPath = "");

private:
    // ---- declaration pre-pass ----
    void declareTopLevel(const Program& prog);
    void registerStruct(const StructDecl* st);
    void registerEnum(const EnumDecl* en);
    void registerFunction(const FunctionDecl* fn); // also used for methods

    // ---- emission ----
    void emitStructType(const StructDecl* st);
    void emitEnumConstants(const EnumDecl* en);
    void emitImpl(const ImplDecl* im);
    void emitFunction(const FunctionDecl* fn);
    void emitFunction(const FunctionDecl* fn, const std::string& owner);

    void prescanVars(const std::vector<StmtPtr>& body);
    void prescanStmt(const StmtPtr& stmt);
    void emitEntryAllocas();
    void emitBlock(const std::vector<StmtPtr>& body);
    void emitStmt(const StmtPtr& stmt);

    // forward declaration: per-function variable slot (defined below)
    struct Slot;

    // expressions
    Val emitExpr(const ExprPtr& expr);
    Val emitLiteral(const LiteralExpr* lit);
    Val emitIdent(const IdentExpr* ident);
    Val emitBinaryOp(const BinaryOpExpr* op);
    Val emitUnaryOp(const UnaryOpExpr* op);
    Val emitCall(const CallExpr* call);
    Val emitFieldAccess(const FieldAccessExpr* fa);
    Val emitStructLit(const StructLitExpr* sl);
    Val emitRange(const RangeExpr* r);
    Val emitIndex(const IndexExpr* ix);
    Val emitArrayLit(const ArrayLitExpr* arr);
    Val emitTypeCast(const TypeCastExpr* tc);
    Val emitStringInterp(const StringInterpExpr* si);
    Val emitTry(const TryExpr* te);

    // statements
    void emitLet(const LetStmt* stmt);
    void emitMut(const MutStmt* stmt);
    void emitAssign(const AssignStmt* stmt);
    void emitReturn(const ReturnStmt* stmt);
    void emitIf(const IfStmt* stmt);
    void emitFor(const ForStmt* stmt);
    void emitListFor(const ForStmt* stmt, Slot& arrSlot);
    Val emitListAppend(Slot& arrSlot, const std::string& varName, const CallExpr* call);
    void emitWhile(const WhileStmt* stmt);
    void emitBreak(const BreakStmt* stmt);
    void emitContinue(const ContinueStmt* stmt);
    void emitExpression(const ExprStmt* stmt);
    void emitMatch(const MatchExpr* m);

    // ---- type helpers ----
    std::string getTypeStr(const TypePtr& type);       // Enginotech type -> LLVM type text
    std::string getLLVMType(const std::string& ecType);
    VT vtOfType(const TypePtr& type) const;
    std::string llvmTypeName(const std::string& structName); // %struct.Name

    // ---- variable slots (alloca/load/store) ----
    void declareSlot(const std::string& name, const TypePtr& type);
    bool hasSlot(const std::string& name) const;
    Val loadVar(const std::string& name, int line, int col);
    void storeToSlot(const std::string& slotReg, const std::string& llvmTy, const Val& v);
    void storeByName(const std::string& name, const Val& v, int line, int col);

    // ---- value helpers ----
    std::string newReg();
    Val makeInt(int64_t v);
    Val materialize(const Val& v, const std::string& llvmTy); // ensure operand fits a type
    Val toStringVal(const Val& v);                            // str() builtin lowering

    // ---- strings ----
    Val stringConstant(const std::string& content);           // GEP'd i8* to global
    Val concatStrings(const Val& a, const Val& b);
    std::string escapeLLVMString(const std::string& s);
    void collectStrings(const std::vector<StmtPtr>& topLevel, std::vector<std::string>& out);
    void collectStringsFromStmt(const StmtPtr& stmt, std::vector<std::string>& out);
    void collectStringsFromExpr(const ExprPtr& expr, std::vector<std::string>& out);

    // ---- scopes ----
    void pushScope();
    void popScope();

    // ---- globals (top-level let/mut/const) ----
    struct GlobalVar {
        std::string name;      // source name
        std::string gvName;    // mangled LLVM global symbol ("gv.name")
        std::string llvmTy;
        VT vt = VT::Int;
        std::string typeName;
        ExprPtr init;          // initializer expression (may be null)
    };
    void declareGlobals(const Program& prog);
    void injectGlobalSlots();
    bool emitGlobalInits();    // at top of main(); true if anything was emitted

    // ---- internal helpers ----
    TypePtr inferTypeOf(const Expr* e) const;      // static type inference for untyped lets
    Val emitAddrOf(const ExprPtr& expr);           // lvalue address (ptr) for assignment targets
    bool terminated() const;                       // true if last emitted instr is a terminator

    std::vector<GlobalVar> globals_;

    void error(const std::string& msg, int line = 0, int col = 0);
    std::string label(const std::string& base);

    DiagnosticEngine& diag_;
    std::stringstream irStream_;

    // symbol tables
    std::map<std::string, std::vector<std::pair<std::string, TypePtr>>> structs_;     // structName -> fields
    std::map<std::string, int> fieldIndex_;                                           // "Struct.field" -> index
    std::map<std::string, std::vector<std::string>> enums_;                           // enumName -> variants
    std::map<std::string, const FunctionDecl*> functions_;                            // free function name -> decl
    std::map<std::string, const FunctionDecl*> methods_;                              // "Struct.method" -> decl
    std::map<std::string, std::string> methodOwner_;                                  // "method" -> owner struct (first wins)

    struct FnSig { std::string ret; std::vector<std::string> params; };
    std::map<std::string, FnSig> sigs_;                                               // callable name -> signature

    // ---- Generics / Monomorphization ----
    // Track generic declarations: fn identity<T> -> original decl
    std::map<std::string, const FunctionDecl*> genericFunctions_;    // "identity" -> original
    std::map<std::string, const StructDecl*> genericStructs_;        // "Vec" -> original
    std::map<std::string, const EnumDecl*> genericEnums_;            // "Option" -> original
    std::map<std::string, const ImplDecl*> genericImpls_;            // "Vec" -> original

    // Monomorphized instances: "identity<int>" -> specialized decl
    struct MonoInstance {
        const FunctionDecl* original;
        std::vector<TypePtr> typeArgs;
        std::shared_ptr<FunctionDecl> specialized;
    };
    std::map<std::string, MonoInstance> monoFunctions_;  // mangled name -> instance

    std::string mangleMonoName(const std::string& baseName, const std::vector<TypePtr>& typeArgs);
    const FunctionDecl* getOrCreateMonomorph(const FunctionDecl* original, const std::vector<TypePtr>& typeArgs);
    std::shared_ptr<FunctionDecl> specializeFunction(const FunctionDecl* original, const std::vector<TypePtr>& typeArgs);
    TypePtr substituteTypeInType(const TypePtr& type, const std::vector<TypePtr>& typeParams, const std::vector<TypePtr>& typeArgs);

    // per-function state
    struct Slot { std::string reg; std::string llvmTy; VT vt; std::string typeName; VT elemVT = VT::Int; };
    std::map<std::string, Slot> slots_;
    std::vector<std::string> slotOrder_;
    std::map<std::string, Slot> scopeSnapshot_;
    std::vector<std::map<std::string, Slot>> scopeStack_;
    std::vector<std::pair<std::string, std::string>> loopStack_;                      // (breakBB, continueBB)
    std::string curRetTy_ = "i32";
    VT curRetVT_ = VT::Int;
    bool hasReturn_ = false;
    std::string curSelfType_;                                                          // inside impl methods

    int regCounter_ = 0;
    bool hasError_ = false;
    bool scriptMode_ = false;   // implicit script main: body runs lets in source order

    std::map<std::string, std::pair<std::string, int>> stringConsts_;                  // content -> (gvName, len)
    std::vector<std::string> pendingStrings_;                                          // late constants, flushed at end
    std::map<std::string, Val> constValues_;                                           // module-level const folding
};

} // namespace eng

#endif // ENGCG_IRGEN_H
