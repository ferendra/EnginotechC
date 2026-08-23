// EnginotechC++ — ETC AST Printer
// Pretty-prints the AST for debugging and visualization

#pragma once
#include "ast.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace eng {
namespace ets {

inline std::string printAST(const ScriptProgram& prog) {
    std::stringstream ss;
    
    ss << "=== ETC Script AST ===\n";
    ss << "File: " << prog.filename << "\n";
    ss << "Total tokens: " << prog.totalTokens << "\n";
    ss << "Statements: " << prog.statements.size() << "\n";
    ss << "Comments: " << prog.comments.size() << "\n";
    ss << "Print calls: " << prog.printCount << "\n";
    ss << "Output calls: " << prog.outputCount << "\n";
    ss << "Say calls: " << prog.sayCount << "\n";
    ss << "Errors: " << prog.errorCount << "\n\n";
    
    if (!prog.comments.empty()) {
        ss << "--- Comments ---\n";
        for (const auto& comment : prog.comments) {
            ss << "  " << comment << "\n";
        }
        ss << "\n";
    }
    
    ss << "--- AST Tree ---\n";
    for (size_t i = 0; i < prog.statements.size(); ++i) {
        ss << "[" << i << "] ";
        ss << prog.statements[i]->toString(0) << "\n\n";
    }
    
    return ss.str();
}

inline std::string exprToString(const ExprPtr& expr, int indent = 0) {
    if (!expr) return "null";
    return expr->toString(indent);
}

inline void printASTToFile(const ScriptProgram& prog, const std::string& filepath) {
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << printAST(prog);
        file.close();
        std::cout << "AST printed to: " << filepath << "\n";
    } else {
        std::cerr << "Failed to open file: " << filepath << "\n";
    }
}

} // namespace ets
} // namespace eng
