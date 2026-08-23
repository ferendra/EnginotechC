// EnginotechC++ — Arduino Statement Emitter Implementation

#include "stmt_emitter.h"
#include <sstream>

namespace eng {
namespace arduinogen {

// ── public ───────────────────────────────────────────────────────
void StmtEmitter::emit(const StmtPtr& s) {
    if (!s) return;
    switch (s->kind()) {
        case StmtKind::Let:     emitLet(static_cast<LetStmt*>(s.get()));    break;
        case StmtKind::Mut:     emitMut(static_cast<MutStmt*>(s.get()));    break;
        case StmtKind::Assign:  emitAssign(static_cast<AssignStmt*>(s.get())); break;
        case StmtKind::Return:  emitReturn(static_cast<ReturnStmt*>(s.get())); break;
        case StmtKind::If:      emitIf(static_cast<IfStmt*>(s.get()));      break;
        case StmtKind::While:   emitWhile(static_cast<WhileStmt*>(s.get())); break;
        case StmtKind::For:     emitFor(static_cast<ForStmt*>(s.get()));    break;
        case StmtKind::Match:   emitMatch(static_cast<MatchExpr*>(s.get())); break;
        case StmtKind::Break:   line("break;"); break;
        case StmtKind::Continue:line("continue;"); break;
        case StmtKind::Expr:
            line(expr_.emit(static_cast<ExprStmt*>(s.get())->expr) + ";");
            break;
        default: break;
    }
}

void StmtEmitter::emitBlock(const std::vector<StmtPtr>& body) {
    indent_++;
    for (const auto& s : body) emit(s);
    indent_--;
}

std::string StmtEmitter::cppTypeOf(const TypePtr& t) const {
    if (!t) return "int";
    const std::string& n = t->getName();
    if (n == "int" || n == "int8" || n == "int16" || n == "int32" ||
        n == "uint" || n == "uint8" || n == "byte")     return "int";
    if (n == "float64" || n == "float32" || n == "double") return "double";
    if (n == "bool")                                    return "bool";
    if (n == "string" || n == "str")                    return "String";
    if (n == "char")                                    return "char";
    return n; // user types pass through
}

void StmtEmitter::declareVar(const std::string& name, const TypePtr& type) {
    varTypes_[name] = type ? cppTypeOf(type) : std::string("int");
}

void StmtEmitter::declareLoopVar(const std::string& name) {
    varTypes_[name] = "long";
}

// ── let / mut ────────────────────────────────────────────────────
void StmtEmitter::emitLet(const LetStmt* st) {
    const std::string ty = st->type ? cppTypeOf(st->type) : std::string("int");
    std::string init = st->init ? (" = " + expr_.emit(st->init)) : "";
    line(ty + " " + st->name + init + ";");
    varTypes_[st->name] = ty;
}

void StmtEmitter::emitMut(const MutStmt* st) {
    const std::string ty = st->type ? cppTypeOf(st->type) : std::string("int");
    std::string init = st->init ? (" = " + expr_.emit(st->init)) : "";
    line(ty + " " + st->name + init + ";");
    varTypes_[st->name] = ty;
}

// ── assign ───────────────────────────────────────────────────────
void StmtEmitter::emitAssign(const AssignStmt* st) {
    std::string lhs;
    if (st->target)
        lhs = expr_.emit(st->target);
    else
        lhs = st->name;
    line(lhs + " = " + expr_.emit(st->value) + ";");
}

// ── return ───────────────────────────────────────────��───────────
void StmtEmitter::emitReturn(const ReturnStmt* st) {
    if (st->expr.has_value() && st->expr.value()) {
        line("return " + expr_.emit(st->expr.value()) + ";");
    } else {
        line("return;");
    }
}

// ── if ───────────────────────────────────────────────────────────
void StmtEmitter::emitIf(const IfStmt* st) {
    line("if (" + expr_.emit(st->cond) + ") {");
    if (st->thenBranch) {
        if (st->thenBranch->kind() == StmtKind::Block)
            emitBlock(static_cast<BlockStmt*>(st->thenBranch.get())->body);
        else
            emit(st->thenBranch);
    }
    if (st->elseBranch) {
        line("} else {");
        if (st->elseBranch->kind() == StmtKind::Block)
            emitBlock(static_cast<BlockStmt*>(st->elseBranch.get())->body);
        else
            emit(st->elseBranch);
    }
    line("}");
}

// ── while ────────────────────────────────────────────────────────
void StmtEmitter::emitWhile(const WhileStmt* st) {
    line("while (" + expr_.emit(st->cond) + ") {");
    if (st->body && st->body->kind() == StmtKind::Block)
        emitBlock(static_cast<BlockStmt*>(st->body.get())->body);
    else if (st->body)
        emit(st->body);
    line("}");
}

// ── for-in range ─────────────────────────────────────────────────
void StmtEmitter::emitFor(const ForStmt* st) {
    std::string start = "0", end = "0";
    std::string op = "<";
    if (auto* rng = dynamic_cast<RangeExpr*>(st->iterable.get())) {
        start = expr_.emit(rng->start);
        end   = expr_.emit(rng->end);
        if (rng->inclusive) op = "<=";
    }
    line("for (long " + st->varName + " = " + start +
         "; " + st->varName + " " + op + " " + end + "; ++" + st->varName + ") {");
    declareLoopVar(st->varName);
    if (st->body && st->body->kind() == StmtKind::Block)
        emitBlock(static_cast<BlockStmt*>(st->body.get())->body);
    else if (st->body)
        emit(st->body);
    line("}");
}

// ── match ────────────────────────────────────────────────���───────
void StmtEmitter::emitMatch(const MatchExpr* m) {
    // match is compiled to chained if-else on Arduino
    if (!m->scrutinee) { line("{}"); return; }
    if (m->arms.empty()) { line("{}"); return; }

    for (size_t i = 0; i < m->arms.size(); ++i) {
        const MatchArm& arm = m->arms[i];
        const bool wildcard = arm.pattern &&
            arm.pattern->kind() == ExprKind::Ident &&
            static_cast<IdentExpr*>(arm.pattern.get())->name == "_";

        if (!wildcard && arm.pattern) {
            // emit if (scrutinee == pattern) { body }
            std::string patText = expr_.emit(arm.pattern);
            std::string scrutText = expr_.emit(m->scrutinee);
            line("if (" + scrutText + " == " + patText + ") {");
        } else {
            // wildcard → else
            if (i > 0 && !m->arms[i-1].pattern) {
                // avoid double else
            } else {
                line("else {");
            }
        }
        if (arm.body) {
            if (arm.body->kind() == StmtKind::Block)
                emitBlock(static_cast<BlockStmt*>(arm.body.get())->body);
            else
                emit(arm.body);
        }
        line("}");
    }
}

// ── helper ───────────────────────────────────────────────────────
void StmtEmitter::line(const std::string& s) {
    for (int i = 0; i < indent_; ++i) out_ << "    ";
    out_ << s << "\n";
}

} // namespace arduinogen
} // namespace eng
