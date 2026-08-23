// EnginotechC++ — Arduino Expression Emitter
// Translates EC AST expressions into Arduino C++ expression text.

#pragma once
#include "../ast/nodes.h"
#include "../lexer/token.h"
#include <string>
#include <sstream>

namespace eng {
namespace arduinogen {

class ExprEmitter {
public:
    explicit ExprEmitter(std::ostringstream& out) : out_(out) {}

    std::string emit(const ExprPtr& e);

private:
    std::ostringstream& out_;

    std::string emitLiteral(const LiteralExpr* lit);
    std::string emitIdent(const IdentExpr* id);
    std::string emitBinaryOp(const BinaryOpExpr* op);
    std::string emitUnaryOp(const UnaryOpExpr* op);
    std::string emitFieldAccess(const FieldAccessExpr* fa);
    std::string emitCall(const CallExpr* call);
    std::string stripNamespace(const std::string& dotted);
    static std::string cppOp(TokenType t);
    bool isNamespacePrefix(const IdentExpr* id, const std::string& ns) const;
};

} // namespace arduinogen
} // namespace eng
