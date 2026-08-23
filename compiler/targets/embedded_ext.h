// EnginotechC++ — EC Embedded Compiler Extensions
// Defines built-in functions and types available when compiling for embedded targets
// These are injected into the semantic analyzer / type checker

#ifndef ENG_COMPILER_EMBEDDED_EXT_H
#define ENG_COMPILER_EMBEDDED_EXT_H

#include <string>
#include <vector>
#include <map>

namespace eng {
namespace embedded_ext {

// Built-in function signatures available in embedded mode
struct BuiltInFn {
    std::string name;
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> params;  // (name, type)
    std::string target;  // "" = all targets, "arduino", "esp32"
};

// Built-in type definitions
struct BuiltInType {
    std::string name;
    std::string baseType;
    std::vector<std::string> methods;
    std::string target;
};

// Initialize all built-in functions for embedded targets
std::vector<BuiltInFn> getBuiltInFunctions(const std::string& target);

// Initialize all built-in types for embedded targets
std::vector<BuiltInType> getBuiltInTypes(const std::string& target);

// Check if a function name is a built-in for the given target
bool isBuiltInFunction(const std::string& name, const std::string& target);

// Get target-specific defines (preprocessor defines)
std::vector<std::string> getTargetDefines(const std::string& target, const std::string& board);

// Get include paths for the target
std::vector<std::string> getIncludePaths(const std::string& target);

} // namespace embedded_ext
} // namespace eng

#endif // ENG_COMPILER_EMBEDDED_EXT_H
