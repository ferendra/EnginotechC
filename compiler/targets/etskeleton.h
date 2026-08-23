// EnginotechC++ — ETC Target Backend (Skeleton)
// Processes .etc files with simplified syntax for printing/output
// Example .etc syntax:
//   print("Hello World")
//   output 42
//   say "Greeting"

#ifndef ENG_TARGET_ETSKELETON_H
#define ENG_TARGET_ETSKELETON_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

namespace eng {
namespace target {

// ETC Script parser and executor
class EtcSkeleton {
public:
    struct ScriptResult {
        bool success = false;
        std::string output;
        std::string error;
        std::vector<Token> tokens;
        int errorCount = 0;
    };

    // Read file and tokenize
    static ScriptResult tokenizeFile(const std::string& filePath);

    // Parse tokens into simple AST
    static ScriptResult parseTokens(const std::vector<Token>& tokens);

    // Execute script and collect output
    static ScriptResult execute(const std::vector<Token>& tokens);

    // Run complete pipeline: read -> tokenize -> parse -> execute
    static ScriptResult runFile(const std::string& filePath);

    // Print token summary
    static void printTokenSummary(const std::vector<Token>& tokens);

private:
    // Helper to extract string content from token
    static std::string extractStringValue(const Token& token);

    // Helper to extract number from token
    static long long extractNumberValue(const Token& token);

    // Simple expression evaluator
    static std::string evalExpression(const std::vector<Token>& tokens, size_t& pos);
};

} // namespace target
} // namespace eng

#endif // ENG_TARGET_ETSKELETON_H
