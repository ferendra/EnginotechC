#ifndef ENGCG_CALLER_H
#define ENGCG_CALLER_H

#include "irgen.h"
#include <memory>

namespace eng {

class CompilerCaller {
public:
    static bool compileToNative(const std::vector<Token>& tokens,
                                const Program& prog,
                                const std::string& outputPath,
                                DiagnosticEngine& diag);
    static std::string compileToIR(const std::vector<Token>& tokens,
                                   const Program& prog,
                                   DiagnosticEngine& diag);
};

} // namespace eng

#endif // ENGCG_CALLER_H
