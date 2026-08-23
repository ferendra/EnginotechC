#include "js.h"
#include "../codegen/js/emitter.h"
#include <fstream>
#include <iostream>

namespace eng {
namespace target {

JSTarget::JSTarget(DiagnosticEngine& diag) : diag_(diag) {}

std::string JSTarget::compileToJS(const std::vector<StmtPtr>& items) {
    js::Emitter emitter(diag_);
    return emitter.emit(items);
}

bool JSTarget::saveToFile(const std::string& path, const std::string& jsCode) {
    std::ofstream f(path);
    if (!f) return false;
    f << jsCode;
    f.close();
    return true;
}

} // namespace target
} // namespace eng
