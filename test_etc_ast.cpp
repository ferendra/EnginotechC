// EnginotechC++ — ETC Standalone AST Test
// Tests the parser and AST generation

#include "compiler/ets/ast.h"
#include "compiler/ets/parser.h"
#include "compiler/ets/ast_printer.h"
#include "compiler/lexer/lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace eng;
using namespace eng::ets;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file.etc>\n";
        std::cout << "Example: " << argv[0] << " examples/ets/skeleton_demo.etc\n";
        return 1;
    }
    
    std::string filePath = argv[1];
    
    std::cout << "=== ETC AST Parser Test ===\n";
    std::cout << "File: " << filePath << "\n\n";
    
    // Step 1: Read file
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filePath << "\n";
        return 1;
    }
    
    std::stringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();
    file.close();
    
    if (source.empty()) {
        std::cerr << "Error: Empty file\n";
        return 1;
    }
    
    // Step 2: Tokenize
    std::cout << "--- Tokenization ---\n";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    std::cout << "Total tokens: " << tokens.size() << "\n";
    std::cout << "Lexer errors: " << lexer.diagnostics.size() << "\n\n";
    
    // Print tokens
    std::cout << "Tokens:\n";
    std::cout << "----------------------------------------\n";
    for (const auto& tok : tokens) {
        std::cout << "  " << tok.toString() << "\n";
    }
    std::cout << "----------------------------------------\n\n";
    
    // Step 3: Parse
    std::cout << "--- Parsing ---\n";
    Parser parser(tokens);
    auto prog = parser.parse();
    prog.filename = filePath;
    
    // Step 4: Print AST
    std::cout << printAST(prog) << "\n";
    
    // Step 5: Save AST to file
    std::string astFile = filePath + ".ast.txt";
    printASTToFile(prog, astFile);
    
    std::cout << "=== Test Complete ===\n";
    return 0;
}
