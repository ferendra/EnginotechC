// Basic linter for EnginotechC++
#ifndef ENG_LINTER_H
#define ENG_LINTER_H

#include <string>
#include <vector>
#include <utility>

namespace eng {
namespace linter {

enum class Severity {
    Info,
    Warning,
    Error
};

struct Diagnostic {
    Severity severity;
    std::string code;
    std::string message;
    int line;
    int column;
};

class Linter {
public:
    std::vector<Diagnostic> lint(const std::string& source);
    std::vector<Diagnostic> lintFile(const std::string& path);
    bool hasErrors(const std::vector<Diagnostic>& diagnostics);
    int countBySeverity(const std::vector<Diagnostic>& diagnostics, Severity severity);
    
private:
    std::vector<Diagnostic> checkIndentation(const std::string& source);
    std::vector<Diagnostic> checkTrailingWhitespace(const std::string& source);
    std::vector<Diagnostic> checkUnusedVariables(const std::string& source);
    std::vector<Diagnostic> checkNamingConventions(const std::string& source);
};

} // namespace linter
} // namespace eng

#endif // ENG_LINTER_H
