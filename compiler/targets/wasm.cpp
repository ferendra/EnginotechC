#include "wasm.h"
#include "../codegen/wasm/emitter.h"
#include <fstream>
#include <iostream>

namespace eng {
namespace target {

WASMTarget::WASMTarget(DiagnosticEngine& diag) : diag_(diag) {}

std::string WASMTarget::compileToWASM(const std::vector<StmtPtr>& items) {
    wasm::Emitter emitter(diag_);
    return emitter.emit(items);
}

bool WASMTarget::saveToFile(const std::string& path, const std::string& wasmCode) {
    std::ofstream f(path);
    if (!f) return false;
    f << wasmCode;
    f.close();
    return true;
}

} // namespace target
} // namespace eng
