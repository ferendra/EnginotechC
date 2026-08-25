#include "../parser/parser.h"
#include <iostream>
#include <string>

namespace eng {

// Simple REPL implementation for engc repl command
// Note: full evaluation pipeline integration is a follow-up; this provides
// the interactive shell skeleton with lex/parse validation per line.
void runRepl() {
    std::string line;

    std::cout << "EnginotechC++ REPL v0.2.0-alpha" << std::endl;
    std::cout << "Type '.exit' to quit" << std::endl;

    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line)) break;

        if (line == ".exit" || line == "exit()" || line == "quit") {
            break;
        }

        if (line.empty()) {
            continue;
        }

        // Validate syntax: tokenize + parse each line
        DiagnosticEngine diag;
        Lexer lx(line);
        auto tokens = lx.tokenize();
        for (const auto& d : lx.diagnostics) {
            std::cout << "lex error: " << d.message << std::endl;
        }
        Parser px(tokens, diag);
        auto prog = px.parse();
        if (diag.hasErrors()) {
            diag.print();
        } else {
            std::cout << "; ok" << std::endl;
        }
    }
}

} // namespace eng