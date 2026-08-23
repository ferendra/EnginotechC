#include "linter.h"
#include <regex>
#include <set>
#include <sstream>
#include <fstream>

namespace eng {
namespace linter {

std::vector<Diagnostic> Linter::lint(const std::string& source) {
    std::vector<Diagnostic> results;
    
    auto indent = checkIndentation(source);
    auto trailing = checkTrailingWhitespace(source);
    auto unused = checkUnusedVariables(source);
    auto naming = checkNamingConventions(source);
    
    results.insert(results.end(), indent.begin(), indent.end());
    results.insert(results.end(), trailing.begin(), trailing.end());
    results.insert(results.end(), unused.begin(), unused.end());
    results.insert(results.end(), naming.begin(), naming.end());
    
    return results;
}

std::vector<Diagnostic> Linter::checkIndentation(const std::string& source) {
    std::vector<Diagnostic> diagnostics;
    std::istringstream stream(source);
    std::string line;
    int lineNum = 0;
    
    while (std::getline(stream, line)) {
        lineNum++;
        // Check for mixed tabs and spaces (simplified)
        if (line.find('\t') != std::string::npos && line.find(' ') != std::string::npos) {
            if (!line.empty() && line[0] == '\t' || line.find(" \t") != std::string::npos) {
                diagnostics.push_back({Severity::Warning, "LINT001", "Mixed tabs and spaces in indentation", lineNum, 1});
            }
        }
    }
    return diagnostics;
}

std::vector<Diagnostic> Linter::checkTrailingWhitespace(const std::string& source) {
    std::vector<Diagnostic> diagnostics;
    std::istringstream stream(source);
    std::string line;
    int lineNum = 0;
    
    while (std::getline(stream, line)) {
        lineNum++;
        size_t trimmed = line.find_last_not_of(" \t\r\n");
        if (trimmed != std::string::npos && trimmed < line.length() - 1) {
            diagnostics.push_back({Severity::Info, "LINT002", "Trailing whitespace", lineNum, (int)(trimmed + 2)});
        }
    }
    return diagnostics;
}

std::vector<Diagnostic> Linter::checkUnusedVariables(const std::string& source) {
    // M0: Placeholder - full implementation will use AST analysis
    return {};
}

std::vector<Diagnostic> Linter::checkNamingConventions(const std::string& source) {
    // M0: Placeholder - full implementation will use lexer output
    return {};
}

std::vector<Diagnostic> Linter::lintFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return {{Severity::Error, "LINT000", "Cannot read file: " + path, 0, 0}};
    }
    
    std::stringstream ss;
    ss << file.rdbuf();
    return lint(ss.str());
}

bool Linter::hasErrors(const std::vector<Diagnostic>& diagnostics) {
    for (const auto& d : diagnostics) {
        if (d.severity == Severity::Error) return true;
    }
    return false;
}

int Linter::countBySeverity(const std::vector<Diagnostic>& diagnostics, Severity severity) {
    int count = 0;
    for (const auto& d : diagnostics) {
        if (d.severity == severity) count++;
    }
    return count;
}

} // namespace linter
} // namespace eng
