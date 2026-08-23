// EnginotechC++ — Arduino Sketch Builder
// Assembles the top-level structure of an Arduino .ino sketch:
//   #include, shim, forward declarations, user fns, setup(), loop().

#pragma once
#include "../ast/nodes.h"
#include <string>
#include <sstream>
#include <vector>
#include "expr_emitter.h"
#include "stmt_emitter.h"

namespace eng {
namespace arduinogen {

class SketchBuilder {
public:
    explicit SketchBuilder(std::ostringstream& out, const std::string& boardId);

    // Emit the full Arduino sketch from a parsed program
    void build(const Program& prog);

private:
    std::ostringstream& out_;
    std::string         board_;

    ExprEmitter& exprEmitter();
    StmtEmitter& stmtEmitter();

    const FunctionDecl* findFn(const std::string& name, const Program& prog) const;
    std::string emitParams(const FunctionDecl* fn) const;
    std::string emitBody(const std::vector<StmtPtr>& body) const;
};

// Entry point: generate a complete .ino string
std::string generateSketch(const Program& prog, const std::string& boardId = "");

} // namespace arduinogen
} // namespace eng
