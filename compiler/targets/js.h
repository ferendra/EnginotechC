#ifndef ENGJS_TARGET_H
#define ENGJS_TARGET_H

#include "../ast/nodes.h"
#include "../diagnostics/diagnostic.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

class JSTarget {
public:
    JSTarget(DiagnosticEngine& diag);
    
    std::string compileToJS(const std::vector<StmtPtr>& items);
    bool saveToFile(const std::string& path, const std::string& jsCode);
    
private:
    DiagnosticEngine& diag_;
};

} // namespace target
} // namespace eng

#endif // ENGJS_TARGET_H
