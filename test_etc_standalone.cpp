// Standalone ETC Skeleton Test
// Tests tokenization and parsing of .etc files without full compiler

#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token.h"
#include "compiler/targets/etskeleton.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace eng;
using namespace eng::target;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file.etc>\n";
        std::cout << "Example: " << argv[0] << " examples/ets/skeleton_demo.etc\n";
        return 1;
    }

    std::string filePath = argv[1];

    std::cout << "=== ETC Skeleton Target Test ===\n";
    std::cout << "File: " << filePath << "\n\n";

    // Step 1: Tokenize
    std::cout << "--- Step 1: Tokenization ---\n";
    auto tokenizeResult = EtcSkeleton::tokenizeFile(filePath);

    if (!tokenizeResult.success) {
        std::cout << "ERROR: " << tokenizeResult.error << "\n";
        return 1;
    }

    std::cout << "Tokenized successfully: " << tokenizeResult.tokens.size() << " tokens\n\n";

    // Print all tokens
    std::cout << "All Tokens:\n";
    std::cout << "----------------------------------------\n";
    for (const auto& token : tokenizeResult.tokens) {
        std::cout << "  " << token.toString() << "\n";
    }
    std::cout << "----------------------------------------\n\n";

    // Step 2: Parse
    std::cout << "--- Step 2: Parsing ---\n";
    auto parseResult = EtcSkeleton::parseTokens(tokenizeResult.tokens);

    if (!parseResult.success) {
        std::cout << "PARSE ERROR: " << parseResult.error << "\n";
        return 1;
    }

    std::cout << "Parsed successfully!\n\n";

    // Step 3: Execute
    std::cout << "--- Step 3: Execution ---\n";
    auto execResult = EtcSkeleton::execute(parseResult.tokens);

    if (!execResult.success) {
        std::cout << "EXECUTION ERROR: " << execResult.error << "\n";
        return 1;
    }

    std::cout << "Execution output:\n";
    std::cout << execResult.output;
    std::cout << "\n";

    // Step 4: Print token summary
    std::cout << "--- Step 4: Token Summary ---\n";
    EtcSkeleton::printTokenSummary(tokenizeResult.tokens);

    std::cout << "\n=== Test Complete ===\n";
    return 0;
}
