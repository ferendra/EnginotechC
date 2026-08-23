#include "python.h"
#include "../codegen/python/emitter.h"
#include <fstream>
#include <iostream>

namespace eng {
namespace target {

PythonTarget::PythonTarget(DiagnosticEngine& diag) : diag_(diag) {}

std::string PythonTarget::compileToPython(const std::vector<StmtPtr>& items) {
    python::Emitter emitter(diag_);
    return emitter.emit(items);
}

bool PythonTarget::saveToFile(const std::string& path, const std::string& pyCode) {
    std::ofstream f(path);
    if (!f) return false;
    f << pyCode;
    f.close();
    return true;
}

} // namespace target
} // namespace eng
