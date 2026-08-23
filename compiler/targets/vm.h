#ifndef ENGVM_TARGET_H
#define ENGVM_TARGET_H

#include "../ast/nodes.h"
#include "../diagnostics/diagnostic.h"
#include "../codegen/vm/vm.h"
#include "../codegen/vm/emitter.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

class VMTarget {
public:
    VMTarget(DiagnosticEngine& diag);
    
    // Compile to bytecode
    std::vector<eng::vm::Instruction> compileToBytecode(const std::vector<StmtPtr>& items);
    
    // Execute bytecode
    eng::vm::Value execute(const std::vector<eng::vm::Instruction>& code);
    
    // Save bytecode to file (binary format)
    bool saveBytecode(const std::string& path, const std::vector<eng::vm::Instruction>& code);
    
private:
    DiagnosticEngine& diag_;
};

} // namespace target
} // namespace eng

#endif // ENGVM_TARGET_H
