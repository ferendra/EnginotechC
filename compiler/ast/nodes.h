#ifndef ENGAST_AST_H
#define ENGAST_AST_H

#include "../lexer/token.h"
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <optional>

namespace eng {

// Forward declarations
class TypeNode {
public:
    virtual ~TypeNode() = default;
    virtual std::string getName() const = 0;
};
using TypePtr = std::shared_ptr<TypeNode>;

struct ClassifyResult {
    enum { Integer, Float, String, Bool, Char, None } kind;
    int intVal = 0;
    double floatVal = 0.0;
    std::string strVal;
};

// ---- TYPE NODES ----
struct BasicType : public TypeNode {
    std::string name;
    explicit BasicType(const std::string& n) : name(n) {}
    std::string getName() const override { return name; }
};

struct GenericType : public TypeNode {
    std::string name;
    std::vector<TypePtr> params;
    GenericType(const std::string& n, std::vector<TypePtr> p = {}) : name(n), params(std::move(p)) {}
    std::string getName() const override { return name; }
};

struct ArrayType : public TypeNode {
    TypePtr elem;
    int size;
    ArrayType(TypePtr e, int s) : elem(std::move(e)), size(s) {}
    std::string getName() const override { return "array"; }
};

struct FnType : public TypeNode {
    TypePtr ret;
    std::vector<std::pair<std::string, TypePtr>> params;
    FnType(TypePtr r, std::vector<std::pair<std::string, TypePtr>> p)
        : ret(std::move(r)), params(std::move(p)) {}
    std::string getName() const override { return "fn"; }
};

// ---- EXPRESSION NODES ----
enum class ExprKind {
    Literal, Ident, BinaryOp, UnaryOp, Call, FieldAccess,
    ArrayLit, Block, If, ForLoop, WhileLoop, Match, Return,
    VariableDecl, TypeCast, StringInterp, Tuple, StructLit, Range,
    Index, Conditional,
    Yield, Await,
    PatternMatch, DimensionalType
};

struct Expr {
    virtual ~Expr() = default;
    virtual ExprKind kind() const = 0;
    TypePtr type;  // Inferred/computed type after type checking
    int line;
    int col;
    Expr(int ln, int cl) : line(ln), col(cl) {}
};

using ExprPtr = std::shared_ptr<Expr>;

struct LiteralExpr : public Expr {
    ClassifyResult val;
    std::string raw;
    LiteralExpr(ClassifyResult v, const std::string& r, int ln, int cl)
        : Expr(ln, cl), val(v), raw(r) {}
    ExprKind kind() const override { return ExprKind::Literal; }
};

struct IdentExpr : public Expr {
    std::string name;
    bool isMut = false;
    IdentExpr(const std::string& n, int ln, int cl)
        : Expr(ln, cl), name(n) {}
    ExprKind kind() const override { return ExprKind::Ident; }
};

struct BinaryOpExpr : public Expr {
    TokenType op;
    ExprPtr left, right;
    BinaryOpExpr(TokenType o, ExprPtr l, ExprPtr r, int ln, int cl)
        : Expr(ln, cl), op(o), left(std::move(l)), right(std::move(r)) {}
    ExprKind kind() const override { return ExprKind::BinaryOp; }
};

struct UnaryOpExpr : public Expr {
    TokenType op;
    ExprPtr operand;
    UnaryOpExpr(TokenType o, ExprPtr opnd, int ln, int cl)
        : Expr(ln, cl), op(o), operand(std::move(opnd)) {}
    ExprKind kind() const override { return ExprKind::UnaryOp; }
};

struct CallExpr : public Expr {
    ExprPtr callee;
    std::vector<ExprPtr> args;
    CallExpr(ExprPtr c, std::vector<ExprPtr> a, int ln, int cl)
        : Expr(ln, cl), callee(std::move(c)), args(std::move(a)) {}
    ExprKind kind() const override { return ExprKind::Call; }
};

struct FieldAccessExpr : public Expr {
    ExprPtr obj;
    std::string field;
    FieldAccessExpr(ExprPtr o, const std::string& f, int ln, int cl)
        : Expr(ln, cl), obj(std::move(o)), field(f) {}
    ExprKind kind() const override { return ExprKind::FieldAccess; }
};

struct ArrayLitExpr : public Expr {
    std::vector<ExprPtr> elems;
    ArrayLitExpr(std::vector<ExprPtr> e, int ln, int cl)
        : Expr(ln, cl), elems(std::move(e)) {}
    ExprKind kind() const override { return ExprKind::ArrayLit; }
};

struct TypeCastExpr : public Expr {
    ExprPtr expr;
    TypePtr targetType;
    TypeCastExpr(ExprPtr e, TypePtr t, int ln, int cl)
        : Expr(ln, cl), expr(std::move(e)), targetType(std::move(t)) {}
    ExprKind kind() const override { return ExprKind::TypeCast; }
};

struct StringInterpExpr : public Expr {
    std::vector<ExprPtr> parts;
    StringInterpExpr(std::vector<ExprPtr> p, int ln, int cl)
        : Expr(ln, cl), parts(std::move(p)) {}
    ExprKind kind() const override { return ExprKind::StringInterp; }
};

struct StructLitExpr : public Expr {
    std::string structName;
    std::vector<std::pair<std::string, ExprPtr>> fields;
    StructLitExpr(const std::string& n,
                  std::vector<std::pair<std::string, ExprPtr>> f, int ln, int cl)
        : Expr(ln, cl), structName(n), fields(std::move(f)) {}
    ExprKind kind() const override { return ExprKind::StructLit; }
};

struct RangeExpr : public Expr {
    ExprPtr start;
    ExprPtr end; // exclusive
    bool inclusive = false;
    RangeExpr(ExprPtr s, ExprPtr e, bool incl, int ln, int cl)
        : Expr(ln, cl), start(std::move(s)), end(std::move(e)), inclusive(incl) {}
    ExprKind kind() const override { return ExprKind::Range; }
};

// Python-style conditional expression: value_if_true if cond else value_if_false
struct ConditionalExpr : public Expr {
    ExprPtr cond;
    ExprPtr thenValue;
    ExprPtr elseValue;
    ConditionalExpr(ExprPtr c, ExprPtr t, ExprPtr e, int ln, int cl)
        : Expr(ln, cl), cond(std::move(c)), thenValue(std::move(t)),
          elseValue(std::move(e)) {}
    ExprKind kind() const override { return ExprKind::Conditional; }
};


struct IndexExpr : public Expr {
    ExprPtr base;
    ExprPtr index;
    IndexExpr(ExprPtr b, ExprPtr i, int ln, int cl)
        : Expr(ln, cl), base(std::move(b)), index(std::move(i)) {}
    ExprKind kind() const override { return ExprKind::Index; }
};

// ---- STATEMENT NODES ----
enum class StmtKind {
    Expr, Block, Let, Mut, Const, Return, If, For, While, Match,
    FunctionDecl, StructDecl, EnumDecl, Impl, Interface, Import,
    Expression, Empty, WhileLoop, Break, Continue, Assign,
    // Module management
    ModuleDecl, ExportDecl, LoadStmt,
    // Exception handling
    TryCatch, ThrowStmt, FinallyBlock,
    Test, Assert, Expect,
    Requires, Ensures, Invariant,
    Coroutine, Yield, Await,
    HotReload,
    Macro, MacroExpand,
    DimensionalType,
    // Bytecode
    BytecodeStmt, OpStmt
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual StmtKind kind() const = 0;
    int line;
    int col;
    Stmt(int ln, int cl) : line(ln), col(cl) {}
};

using StmtPtr = std::shared_ptr<Stmt>;

struct ExprStmt : public Stmt {
    ExprPtr expr;
    ExprStmt(ExprPtr e, int ln, int cl) : Stmt(ln, cl), expr(std::move(e)) {}
    StmtKind kind() const override { return StmtKind::Expr; }
};

struct BlockStmt : public Stmt {
    std::vector<StmtPtr> body;
    BlockStmt(std::vector<StmtPtr> b, int ln, int cl) : Stmt(ln, cl), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::Block; }
};

struct LetStmt : public Stmt {
    std::string name;
    TypePtr type;
    ExprPtr init;
    bool immutable = true;
    int initLine = 0;  // line of init expression for better error reporting
    int initCol = 0;   // col of init expression for better error reporting
    LetStmt(const std::string& n, TypePtr t, ExprPtr i, int ln, int cl, int iln = 0, int icl = 0)
        : Stmt(ln, cl), name(n), type(std::move(t)), init(std::move(i)),
          initLine(iln), initCol(icl) {}
    StmtKind kind() const override { return StmtKind::Let; }
};

struct MutStmt : public Stmt {
    std::string name;
    TypePtr type;
    ExprPtr init;
    MutStmt(const std::string& n, TypePtr t, ExprPtr i, int ln, int cl)
        : Stmt(ln, cl), name(n), type(std::move(t)), init(std::move(i)) {}
    StmtKind kind() const override { return StmtKind::Mut; }
};

struct ConstStmt : public Stmt {
    std::string name;
    ExprPtr init;
    ConstStmt(const std::string& n, ExprPtr i, int ln, int cl)
        : Stmt(ln, cl), name(n), init(std::move(i)) {}
    StmtKind kind() const override { return StmtKind::Const; }
};

struct AssignStmt : public Stmt {
    std::string name;          // simple variable target
    ExprPtr target;            // complex target (e.g. field access), if any
    ExprPtr value;
    AssignStmt(const std::string& n, ExprPtr v, int ln, int cl)
        : Stmt(ln, cl), name(n), value(std::move(v)) {}
    AssignStmt(ExprPtr t, ExprPtr v, int ln, int cl)
        : Stmt(ln, cl), target(std::move(t)), value(std::move(v)) {}
    StmtKind kind() const override { return StmtKind::Assign; }
};

struct ReturnStmt : public Stmt {
    std::optional<ExprPtr> expr;
    ReturnStmt(std::optional<ExprPtr> e, int ln, int cl) : Stmt(ln, cl), expr(std::move(e)) {}
    StmtKind kind() const override { return StmtKind::Return; }
};

struct IfStmt : public Stmt {
    ExprPtr cond;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e, int ln, int cl)
        : Stmt(ln, cl), cond(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
    StmtKind kind() const override { return StmtKind::If; }
};

struct ForStmt : public Stmt {
    std::string varName;
    ExprPtr iterable;
    StmtPtr body;
    ForStmt(const std::string& vn, ExprPtr it, StmtPtr b, int ln, int cl)
        : Stmt(ln, cl), varName(vn), iterable(std::move(it)), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::For; }
};

struct WhileStmt : public Stmt {
    ExprPtr cond;
    StmtPtr body;
    WhileStmt(ExprPtr c, StmtPtr b, int ln, int cl)
        : Stmt(ln, cl), cond(std::move(c)), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::While; }
};

struct BreakStmt : public Stmt {
    BreakStmt(int ln, int cl) : Stmt(ln, cl) {}
    StmtKind kind() const override { return StmtKind::Break; }
};

struct ContinueStmt : public Stmt {
    ContinueStmt(int ln, int cl) : Stmt(ln, cl) {}
    StmtKind kind() const override { return StmtKind::Continue; }
};

struct FunctionDecl : public Stmt {
    std::string name;
    bool isMain = false;
    TypePtr returnType;
    std::vector<std::pair<std::string, TypePtr>> params;
    std::vector<StmtPtr> body;
    bool isAsync = false;
    FunctionDecl(const std::string& n, TypePtr ret,
                 std::vector<std::pair<std::string, TypePtr>> p,
                 std::vector<StmtPtr> b, int ln, int cl)
        : Stmt(ln, cl), name(n), returnType(std::move(ret)),
          params(std::move(p)), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::FunctionDecl; }
};

struct StructDecl : public Stmt {
    std::string name;
    std::vector<std::pair<std::string, TypePtr>> fields;
    StructDecl(const std::string& n, std::vector<std::pair<std::string, TypePtr>> f, int ln, int cl)
        : Stmt(ln, cl), name(n), fields(std::move(f)) {}
    StmtKind kind() const override { return StmtKind::StructDecl; }
};

struct EnumDecl : public Stmt {
    std::string name;
    std::vector<std::pair<std::string, std::vector<TypePtr>>> variants;
    EnumDecl(const std::string& n, std::vector<std::pair<std::string, std::vector<TypePtr>>> v, int ln, int cl)
        : Stmt(ln, cl), name(n), variants(std::move(v)) {}
    StmtKind kind() const override { return StmtKind::EnumDecl; }
};

struct ImplDecl : public Stmt {
    std::string structName;
    std::vector<StmtPtr> methods;
    ImplDecl(const std::string& s, std::vector<StmtPtr> m, int ln = 0, int cl = 0)
        : Stmt(ln, cl), structName(s), methods(std::move(m)) {}
    StmtKind kind() const override { return StmtKind::Impl; }
};

struct ImportStmt : public Stmt {
    std::string modulePath;
    ImportStmt(const std::string& p, int ln, int cl)
        : Stmt(ln, cl), modulePath(p) {}
    StmtKind kind() const override { return StmtKind::Import; }
};

// ---- MODULE MANAGEMENT ----
struct ModuleDecl : public Stmt {
    std::string moduleName;
    std::vector<StmtPtr> body;
    ModuleDecl(const std::string& name, std::vector<StmtPtr> b, int ln, int cl)
        : Stmt(ln, cl), moduleName(name), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::ModuleDecl; }
};

struct ExportDecl : public Stmt {
    std::string exportName;
    ExprPtr expr;
    ExportDecl(const std::string& n, ExprPtr e, int ln, int cl)
        : Stmt(ln, cl), exportName(n), expr(std::move(e)) {}
    StmtKind kind() const override { return StmtKind::ExportDecl; }
};

struct LoadStmt : public Stmt {
    std::string moduleName;
    LoadStmt(const std::string& name, int ln, int cl)
        : Stmt(ln, cl), moduleName(name) {}
    StmtKind kind() const override { return StmtKind::LoadStmt; }
};

// ---- EXCEPTION HANDLING ----
struct CatchClause {
    std::string exceptionName;
    TypePtr exceptionType;
    StmtPtr body;
};

struct TryCatchStmt : public Stmt {
    std::vector<StmtPtr> tryBody;
    std::vector<CatchClause> catches;
    std::vector<StmtPtr> finallyBody;
    TryCatchStmt(std::vector<StmtPtr> t, std::vector<CatchClause> c, std::vector<StmtPtr> f, int ln, int cl)
        : Stmt(ln, cl), tryBody(std::move(t)), catches(std::move(c)), finallyBody(std::move(f)) {}
    StmtKind kind() const override { return StmtKind::TryCatch; }
};

struct ThrowStmt : public Stmt {
    ExprPtr expression;
    ThrowStmt(ExprPtr e, int ln, int cl)
        : Stmt(ln, cl), expression(std::move(e)) {}
    StmtKind kind() const override { return StmtKind::ThrowStmt; }
};

// ---- BYTECODE / VM ----
struct BytecodeStmt : public Stmt {
    std::string bytecode;
    BytecodeStmt(const std::string& bc, int ln, int cl)
        : Stmt(ln, cl), bytecode(bc) {}
    StmtKind kind() const override { return StmtKind::BytecodeStmt; }
};

struct OpStmt : public Stmt {
    std::string opcode;
    std::vector<std::string> operands;
    OpStmt(const std::string& op, std::vector<std::string> ops, int ln, int cl)
        : Stmt(ln, cl), opcode(std::move(op)), operands(std::move(ops)) {}
    StmtKind kind() const override { return StmtKind::OpStmt; }
};

// ---- TESTING FRAMEWORK ----
struct TestStmt : public Stmt {
    std::string name;
    std::vector<StmtPtr> body;
    TestStmt(const std::string& n, std::vector<StmtPtr> b, int ln, int cl)
        : Stmt(ln, cl), name(n), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::Test; }
};

struct AssertStmt : public Stmt {
    ExprPtr condition;
    ExprPtr message;
    AssertStmt(ExprPtr c, ExprPtr m, int ln, int cl)
        : Stmt(ln, cl), condition(std::move(c)), message(std::move(m)) {}
    StmtKind kind() const override { return StmtKind::Assert; }
};

struct ExpectStmt : public Stmt {
    ExprPtr actual;
    ExprPtr expected;
    ExpectStmt(ExprPtr a, ExprPtr e, int ln, int cl)
        : Stmt(ln, cl), actual(std::move(a)), expected(std::move(e)) {}
    StmtKind kind() const override { return StmtKind::Expect; }
};

// ---- DESIGN BY CONTRACT ----
struct RequiresStmt : public Stmt {
    ExprPtr condition;
    RequiresStmt(ExprPtr c, int ln, int cl)
        : Stmt(ln, cl), condition(std::move(c)) {}
    StmtKind kind() const override { return StmtKind::Requires; }
};

struct EnsuresStmt : public Stmt {
    ExprPtr condition;
    EnsuresStmt(ExprPtr c, int ln, int cl)
        : Stmt(ln, cl), condition(std::move(c)) {}
    StmtKind kind() const override { return StmtKind::Ensures; }
};

struct InvariantStmt : public Stmt {
    ExprPtr condition;
    InvariantStmt(ExprPtr c, int ln, int cl)
        : Stmt(ln, cl), condition(std::move(c)) {}
    StmtKind kind() const override { return StmtKind::Invariant; }
};

// ---- COROUTINES ----
struct CoroutineDecl : public Stmt {
    std::string name;
    std::vector<std::pair<std::string, TypePtr>> params;
    TypePtr returnType;
    std::vector<StmtPtr> body;
    CoroutineDecl(const std::string& n,
                  std::vector<std::pair<std::string, TypePtr>> p,
                  TypePtr r,
                  std::vector<StmtPtr> b,
                  int ln, int cl)
        : Stmt(ln, cl), name(n), params(std::move(p)),
          returnType(std::move(r)), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::Coroutine; }
};

struct YieldExpr : public Expr {
    ExprPtr value;
    YieldExpr(ExprPtr v, int ln, int cl)
        : Expr(ln, cl), value(std::move(v)) {}
    ExprKind kind() const override { return ExprKind::Yield; }
};

struct AwaitExpr : public Expr {
    ExprPtr operand;
    AwaitExpr(ExprPtr o, int ln, int cl)
        : Expr(ln, cl), operand(std::move(o)) {}
    ExprKind kind() const override { return ExprKind::Await; }
};

// ---- HOT RELOAD ----
struct HotReloadStmt : public Stmt {
    std::string filePath;
    HotReloadStmt(const std::string& f, int ln, int cl)
        : Stmt(ln, cl), filePath(f) {}
    StmtKind kind() const override { return StmtKind::HotReload; }
};

// ---- PATTERNS ----
struct PatternMatchExpr : public Expr {
    ExprPtr scrutinee;
    struct Arm {
        std::string pattern;
        ExprPtr guard;
        ExprPtr body;
        Arm(const std::string& p, ExprPtr g, ExprPtr b)
            : pattern(p), guard(std::move(g)), body(std::move(b)) {}
    };
    std::vector<Arm> arms;
    PatternMatchExpr(ExprPtr s, std::vector<Arm> a, int ln, int cl)
        : Expr(ln, cl), scrutinee(std::move(s)), arms(std::move(a)) {}
    ExprKind kind() const override { return ExprKind::PatternMatch; }
};

// ---- MACROS ----
struct MacroDecl : public Stmt {
    std::string name;
    std::vector<StmtPtr> body;
    MacroDecl(const std::string& n, std::vector<StmtPtr> b, int ln, int cl)
        : Stmt(ln, cl), name(n), body(std::move(b)) {}
    StmtKind kind() const override { return StmtKind::Macro; }
};

struct MacroExpandStmt : public Stmt {
    std::string macroName;
    std::vector<ExprPtr> args;
    MacroExpandStmt(const std::string& n, std::vector<ExprPtr> a, int ln, int cl)
        : Stmt(ln, cl), macroName(n), args(std::move(a)) {}
    StmtKind kind() const override { return StmtKind::MacroExpand; }
};

// ---- DIMENSIONAL TYPES ----
struct DimensionalTypeExpr : public Expr {
    std::string baseType;
    std::vector<std::string> dimensions;
    DimensionalTypeExpr(const std::string& bt, std::vector<std::string> d, int ln, int cl)
        : Expr(ln, cl), baseType(bt), dimensions(std::move(d)) {}
    ExprKind kind() const override { return ExprKind::DimensionalType; }
};

// ---- MODULE / PROGRAM ----
struct Module {
    std::string name;
    std::vector<StmtPtr> items;
    std::vector<ImportStmt*> imports;
};

struct Program {
    std::vector<StmtPtr> topLevel;
    std::vector<Module*> modules;
};

// ---- MATCH EXPRESSION ---- 
struct MatchArm { 
    ExprPtr pattern; 
    StmtPtr body; 
    MatchArm(ExprPtr p, StmtPtr b) : pattern(std::move(p)), body(std::move(b)) {} 
}; 
struct MatchExpr : public Stmt { 
    ExprPtr scrutinee; 
    std::vector<MatchArm> arms; 
    MatchExpr(ExprPtr s, std::vector<MatchArm> a, int ln, int cl) 
        : Stmt(ln, cl), scrutinee(std::move(s)), arms(std::move(a)) {} 
    StmtKind kind() const override { return StmtKind::Match; } 
}; 

} // namespace eng

#endif // ENGAST_AST_H
