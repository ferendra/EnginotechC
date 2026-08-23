#ifndef ENGPY_TARGET_H
#define ENGPY_TARGET_H

#include "../ast/nodes.h"
#include "../diagnostics/diagnostic.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

class PythonTarget {
public:
    PythonTarget(DiagnosticEngine& diag);
    
    std::string compileToPython(const std::vector<StmtPtr>& items);
    bool saveToFile(const std::string& path, const std::string& pyCode);
    
private:
    DiagnosticEngine& diag_;
};

} // namespace target
} // namespace eng

#endif // ENGPY_TARGET_H
