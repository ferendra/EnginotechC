#include "vm.h"
#include <fstream>
#include <iostream>
#include <cstring>

namespace eng {
namespace target {

VMTarget::VMTarget(DiagnosticEngine& diag) : diag_(diag) {}

std::vector<eng::vm::Instruction> VMTarget::compileToBytecode(const std::vector<StmtPtr>& items) {
    bytecode::Emitter emitter(diag_);
    return emitter.emit(items);
}

eng::vm::Value VMTarget::execute(const std::vector<eng::vm::Instruction>& code) {
    eng::vm::VM vm;
    vm.execute(code);
    return vm.getResult();
}

bool VMTarget::saveBytecode(const std::string& path, const std::vector<eng::vm::Instruction>& code) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    
    // Write magic number
    const char magic[] = "ECVM";
    f.write(magic, 4);
    
    // Write version
    uint16_t version = 1;
    f.write(reinterpret_cast<const char*>(&version), 2);
    
    // Write instruction count
    uint32_t count = static_cast<uint32_t>(code.size());
    f.write(reinterpret_cast<const char*>(&count), 4);
    
    // Write instructions
    for (const auto& instr : code) {
        uint8_t op = static_cast<uint8_t>(instr.op);
        f.write(reinterpret_cast<const char*>(&op), 1);
        int16_t operand = instr.operand;
        f.write(reinterpret_cast<const char*>(&operand), 2);
        
        // Write string operand length and data
        uint16_t strLen = static_cast<uint16_t>(instr.strOperand.size());
        f.write(reinterpret_cast<const char*>(&strLen), 2);
        if (strLen > 0) {
            f.write(instr.strOperand.c_str(), strLen);
        }
    }
    
    f.close();
    return true;
}

} // namespace target
} // namespace eng
