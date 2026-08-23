// EnginotechC++ — Arduino Expression Emitter Implementation
// Translates EC AST expressions into Arduino C++ expression text.

#include "expr_emitter.h"
#include <algorithm>

namespace eng {
namespace arduinogen {

// ── public ───────────────────────────────────────────────────────
std::string ExprEmitter::emit(const ExprPtr& e) {
    if (!e) return "0";
    switch (e->kind()) {
        case ExprKind::Literal:    return emitLiteral(static_cast<LiteralExpr*>(e.get()));
        case ExprKind::Ident:      return emitIdent(static_cast<IdentExpr*>(e.get()));
        case ExprKind::BinaryOp:   return emitBinaryOp(static_cast<BinaryOpExpr*>(e.get()));
        case ExprKind::UnaryOp:    return emitUnaryOp(static_cast<UnaryOpExpr*>(e.get()));
        case ExprKind::FieldAccess:return emitFieldAccess(static_cast<FieldAccessExpr*>(e.get()));
        case ExprKind::Call:       return emitCall(static_cast<CallExpr*>(e.get()));
        default:                   return "0 /* unsupported expr */";
    }
}

// ── literal ──────────────────────────────────────────────────────
std::string ExprEmitter::emitLiteral(const LiteralExpr* lit) {
    switch (lit->val.kind) {
        case ClassifyResult::Integer: return std::to_string(lit->val.intVal);
        case ClassifyResult::Float: {
            std::string r = lit->raw.empty() ? std::to_string(lit->val.floatVal) : lit->raw;
            if (r.find('.') == std::string::npos && r.find('e') == std::string::npos)
                r += ".0";
            return r;
        }
        case ClassifyResult::Bool:  return lit->val.intVal ? "true" : "false";
        case ClassifyResult::String: {
            // Escape backslashes and quotes for valid C++ string literal
            std::string escaped;
            for (size_t i = 0; i < lit->val.strVal.size(); ++i) {
                char c = lit->val.strVal[i];
                if (c == '\\') {
                    // Check for \n, \t, \r sequences in source
                    if (i + 1 < lit->val.strVal.size()) {
                        char next = lit->val.strVal[i + 1];
                        if (next == 'n') { escaped += "\\n"; ++i; continue; }
                        if (next == 't') { escaped += "\\t"; ++i; continue; }
                        if (next == 'r') { escaped += "\\r"; ++i; continue; }
                    }
                    escaped += "\\\\";
                }
                else if (c == '"') escaped += "\\\"";
                else if (c == '\n') escaped += "\\n";
                else if (c == '\r') escaped += "\\r";
                else if (c == '\t') escaped += "\\t";
                else escaped += c;
            }
            return "\"" + escaped + "\"";
        }
        case ClassifyResult::Char:  return "'" + std::string(1, static_cast<char>(lit->val.intVal)) + "'";
        default:                    return "0";
    }
}

// ── ident ────────────────────────────────────────────────────────
std::string ExprEmitter::emitIdent(const IdentExpr* id) {
    return id->name;
}

// ── binary op ────────────────────────────────────────────────────
std::string ExprEmitter::emitBinaryOp(const BinaryOpExpr* op) {
    std::string lhs = emit(op->left);
    std::string rhs = emit(op->right);
    std::string cxxOp = cppOp(op->op);
    // String concat via + works natively on Arduino String
    if (op->op == TokenType::PLUS || op->op == TokenType::MINUS ||
        op->op == TokenType::STAR || op->op == TokenType::SLASH ||
        op->op == TokenType::MOD || op->op == TokenType::EQ ||
        op->op == TokenType::NEQ || op->op == TokenType::LT ||
        op->op == TokenType::GT || op->op == TokenType::LTE ||
        op->op == TokenType::GTE || op->op == TokenType::AND ||
        op->op == TokenType::OR) {
        return "(" + lhs + " " + cxxOp + " " + rhs + ")";
    }
    return "(" + lhs + " " + cxxOp + " " + rhs + ")";
}

// ── unary op ─────────────────────────────────────────────────────
std::string ExprEmitter::emitUnaryOp(const UnaryOpExpr* op) {
    std::string operand = emit(op->operand);
    std::string prefix;
    if (op->op == TokenType::NOT)       prefix = "!";
    else if (op->op == TokenType::MINUS) prefix = "-";
    else                                 prefix = "";
    return "(" + prefix + operand + ")";
}

    // ── Field access: strip namespace, map Arduino constants ──
    std::string ExprEmitter::emitFieldAccess(const FieldAccessExpr* fa) {
        std::string obj = emit(fa->obj);
        std::string dotted = obj + "." + fa->field;
        std::string bare = stripNamespace(dotted);
        // Map common Arduino constants to numeric values
        if (bare == "INPUT_PULLUP")  return "2";   // INPUT_PULLUP = 2
        if (bare == "INPUT")         return "0";   // INPUT = 0
        if (bare == "OUTPUT")        return "1";   // OUTPUT = 1
        if (bare == "LOW")           return "0";   // LOW = 0
        if (bare == "HIGH")          return "1";   // HIGH = 1
        if (bare == "PinState::LOW") return "0";
        if (bare == "PinState::HIGH") return "1";
        if (bare == "Mode::INPUT")   return "0";
        if (bare == "Mode::INPUT_PULLUP") return "2";
        if (bare == "Mode::OUTPUT")  return "1";
        return bare;
    }

// ── call ─────────────────────────────────────────────────────────
std::string ExprEmitter::emitCall(const CallExpr* call) {
    // ── Builtin free functions ──
    if (auto* id = dynamic_cast<IdentExpr*>(call->callee.get())) {
        // delay(ms)
        if (id->name == "delay")
            return "delay(" + emit(call->args[0]) + ")";
        // millis() / micros()
        if (id->name == "millis") return "millis()";
        if (id->name == "micros") return "micros()";
        // print / println
        if (id->name == "print")
            return "(Serial.print(" +
                   (call->args.empty() ? std::string("\"\"") : emit(call->args[0])) +
                   "), 0)";
        if (id->name == "println")
            return "(Serial.println(" +
                   (call->args.empty() ? std::string("\"\"") : emit(call->args[0])) +
                   "), 0)";
        // str(x) → String(x)
        if (id->name == "str")
            return "String(" +
                   (call->args.empty() ? std::string("0") : emit(call->args[0])) + ")";
        // assert → no-op on device
        if (id->name == "assert") return "1";
    }

    // ── Field-access calls: obj.method(args) ──
    if (auto* fa = dynamic_cast<FieldAccessExpr*>(call->callee.get())) {
        const std::string& meth = fa->field;
        std::vector<std::string> argv;
        for (const auto& a : call->args) argv.push_back(emit(a));

        // gpio.output(N) / gpio.input(N) / gpio.pwm(N)
        if (auto* baseId = dynamic_cast<IdentExpr*>(fa->obj.get())) {
            if (baseId->name == "gpio") {
                if (meth == "output")     return "ec_gpio_output(" + argv[0] + ")";
                if (meth == "input")      return "ec_gpio_input(" + argv[0] + ")";
                if (meth == "input_pullup") return "ec_gpio_input_pullup(" + argv[0] + ")";
                if (meth == "pwm")        return "ec_gpio_pwm(" + argv[0] + ")";
                // pinMode(...)
                if (meth == "pinMode")
                    return "pinMode(" + argv[0] + ", " + stripNamespace(argv.size() > 1 ? argv[1] : "OUTPUT") + ")";
                // digitalWrite(...)
                if (meth == "digitalWrite")
                    return "digitalWrite(" + argv[0] + ", " + stripNamespace(argv[1]) + ")";
                // digitalRead(...)
                if (meth == "digitalRead")
                    return "digitalRead(" + argv[0] + ")";
            }

            // uart.begin / uart.write / uart.println / uart.read / uart.available
            if (baseId->name == "uart") {
                if (meth == "begin")     return "Serial.begin(" + argv[0] + ")";
                if (meth == "write")     return "(Serial.print(" + argv[0] + "), 0)";
                if (meth == "println")   return "(Serial.println(" + argv[0] + "), 0)";
                if (meth == "read")      return "(Serial.read())";
                if (meth == "available") return "Serial.available()";
            }

            // system.delay / system.millis
            if (baseId->name == "system") {
                if (meth == "delay")  return "delay(" + argv[0] + ")";
                if (meth == "millis") return "millis()";
                if (meth == "micros") return "micros()";
            }

            // analogRead(pin) / analogWrite(pin, val)
            if (baseId->name == "analog") {
                if (meth == "read")    return "analogRead(" + argv[0] + ")";
                if (meth == "write")   return "analogWrite(" + argv[0] + ", " + argv[1] + ")";
            }
        }

        // Method call on object: obj.method(args)
        std::string args;
        for (size_t i = 0; i < argv.size(); ++i) {
            if (i) args += ", ";
            args += argv[i];
        }
        return emit(fa->obj) + "." + meth + "(" + args + ")";
    }

    // ── Plain call: func(args) ──
    auto* id2 = dynamic_cast<IdentExpr*>(call->callee.get());
    std::string args;
    for (size_t i = 0; i < call->args.size(); ++i) {
        if (i) args += ", ";
        args += emit(call->args[i]);
    }
    return (id2 ? id2->name : "unknown_fn") + "(" + args + ")";
}

// ── helpers ──────────────────────────────────────────────────────
std::string ExprEmitter::stripNamespace(const std::string& dotted) {
    size_t pos = dotted.rfind('.');
    return pos == std::string::npos ? dotted : dotted.substr(pos + 1);
}

std::string ExprEmitter::cppOp(TokenType t) {
    switch (t) {
        case TokenType::PLUS:      return "+";
        case TokenType::MINUS:     return "-";
        case TokenType::STAR:      return "*";
        case TokenType::SLASH:     return "/";
        case TokenType::MOD:       return "%";
        case TokenType::EQ:        return "==";
        case TokenType::NEQ:       return "!=";
        case TokenType::LT:        return "<";
        case TokenType::GT:        return ">";
        case TokenType::LTE:       return "<=";
        case TokenType::GTE:       return ">=";
        case TokenType::AND:       return "&&";
        case TokenType::OR:        return "||";
        default:                   return "?";
    }
}

bool ExprEmitter::isNamespacePrefix(const IdentExpr* id, const std::string& ns) const {
    return id && id->name == ns;
}

} // namespace arduinogen
} // namespace eng
