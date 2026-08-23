#include "caller.h"

namespace eng {

bool CompilerCaller::compileToNative(const std::vector<Token>& tokens,
                                     const Program& prog,
                                     const std::string& outputPath,
                                     DiagnosticEngine& diag) {
    (void)tokens;
    LLVMIRGenerator irgen(diag);
    return irgen.compile(prog, outputPath, "");
}

std::string CompilerCaller::compileToIR(const std::vector<Token>& tokens,
                                        const Program& prog,
                                        DiagnosticEngine& diag) {
    (void)tokens;
    LLVMIRGenerator irgen(diag);
    return irgen.generate(prog);
}

} // namespace eng
