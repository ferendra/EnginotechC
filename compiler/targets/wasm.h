#ifndef ENGWASM_TARGET_H
#define ENGWASM_TARGET_H

#include "../ast/nodes.h"
#include "../diagnostics/diagnostic.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

class WASMTarget {
public:
    WASMTarget(DiagnosticEngine& diag);
    
    std::string compileToWASM(const std::vector<StmtPtr>& items);
    bool saveToFile(const std::string& path, const std::string& wasmCode);
    
private:
    DiagnosticEngine& diag_;
};

} // namespace target
} // namespace eng

#endif // ENGWASM_TARGET_H
