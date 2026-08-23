// EnginotechC++ — Language Server Protocol (LSP) Implementation
// Provides IDE features: completion, diagnostics, go-to-definition, hover

#include "langserver.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/parser/parser.h"
#include "../compiler/semantic/semantic.h"
#include "../compiler/types/typechecker.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace eng {
namespace langserver {

LanguageServer::LanguageServer() {}
LanguageServer::~LanguageServer() {}

void LanguageServer::sendResponse(int id, const std::string& result, int errorCode, const std::string& errorMsg) {
    std::ostringstream oss;
    
    if (errorCode == 0) {
        oss << "{\"jsonrpc\":\"2.0\",\"id\":" << id << ",\"result\":" << result << "}";
    } else {
        oss << "{\"jsonrpc\":\"2.0\",\"id\":" << id 
            << ",\"error\":{\"code\":" << errorCode 
            << ",\"message\":\"" << errorMsg << "\"}}";
    }
    
    std::string response = oss.str();
    std::cout << "Content-Length: " << response.size() << "\r\n\r\n" << response;
    std::cout.flush();
}

void LanguageServer::initialize() {
    initialized_ = true;
}

void LanguageServer::didOpen(const std::string& uri, const std::string& content, int version) {
    openDocuments_[uri] = content;
}

void LanguageServer::didChange(const std::string& uri, const std::string& content) {
    if (openDocuments_.count(uri)) {
        openDocuments_[uri] = content;
    }
}

void LanguageServer::didSave(const std::string& uri) {
    computeDiagnostics(uri);
}

void LanguageServer::didClose(const std::string& uri) {
    openDocuments_.erase(uri);
    diagnostics_.erase(uri);
}

void LanguageServer::computeDiagnostics(const std::string& uri) {
    auto it = openDocuments_.find(uri);
    if (it == openDocuments_.end()) return;
    
    std::string content = it->second;
    DiagnosticEngine diag;
    
    Lexer lexer(content);
    auto tokens = lexer.tokenize();
    
    Parser parser(tokens, diag);
    auto prog = parser.parse();
    
    SemanticAnalyzer sa(diag);
    sa.analyze(prog.topLevel);
    
    TypeChecker tc(diag);
    tc.check(prog.topLevel);
    
    diagnostics_[uri].clear();
    diag.print();  // Print diagnostics to stderr
}

std::vector<std::string> LanguageServer::getDiagnostics(const std::string& uri) {
    auto it = diagnostics_.find(uri);
    if (it != diagnostics_.end()) return it->second;
    return {};
}

std::vector<std::string> LanguageServer::getCompletions(const std::string& uri, int line, int charPos) {
    std::vector<std::string> completions;
    
    auto docIt = openDocuments_.find(uri);
    if (docIt == openDocuments_.end()) return completions;
    
    std::string content = docIt->second;
    
    // Find current word
    std::string currentWord;
    for (int i = charPos - 1; i >= 0 && (isalnum(content[i]) || content[i] == '_'); --i) {
        currentWord = content[i] + currentWord;
    }
    
    // Keywords
    std::vector<std::string> keywords = {
        "fn", "let", "mut", "if", "else", "for", "while", "in", 
        "return", "break", "continue", "import", "package", "type",
        "int", "float", "bool", "string", "void", "char",
        "true", "false", "null", "nil", "None"
    };
    
    for (const auto& kw : keywords) {
        if (kw.substr(0, currentWord.size()) == currentWord || currentWord.empty()) {
            completions.push_back(kw);
        }
    }
    
    // Standard library functions
    std::vector<std::string> stdFuncs = {
        "print", "println", "str", "len", "upper", "lower", "trim",
        "append", "pop", "push", "clear", "sort", "reverse",
        "min", "max", "abs", "sqrt", "pow", "rand",
        "open", "read", "write", "close",
        "Vec", "Map", "Set", "String"
    };
    
    for (const auto& func : stdFuncs) {
        if (func.substr(0, currentWord.size()) == currentWord || currentWord.empty()) {
            completions.push_back(func);
        }
    }
    
    return completions;
}

std::string LanguageServer::getHover(const std::string& uri, int line, int charPos) {
    auto docIt = openDocuments_.find(uri);
    if (docIt == openDocuments_.end()) return "{}";
    
    std::string content = docIt->second;
    
    // Find word at position
    std::string word;
    for (int i = charPos; i < (int)content.size() && (isalnum(content[i]) || content[i] == '_'); ++i) {
        word += content[i];
    }
    for (int i = charPos - 1; i >= 0 && (isalnum(content[i]) || content[i] == '_'); --i) {
        word = content[i] + word;
    }
    
    if (word.empty()) return "{}";
    
    // Check keywords
    std::vector<std::pair<std::string, std::string>> doc = {
        {"fn", "Function declaration"},
        {"let", "Immutable variable binding"},
        {"mut", "Mutable variable binding"},
        {"if", "Conditional expression"},
        {"else", "Else branch"},
        {"for", "Iteration over range"},
        {"while", "Loop while condition"},
        {"return", "Return from function"},
        {"int", "Integer type"},
        {"float", "Floating point type"},
        {"bool", "Boolean type"},
        {"string", "String type"},
        {"void", "Void type"},
        {"true", "Boolean true literal"},
        {"false", "Boolean false literal"},
    };
    
    for (const auto& [kw, desc] : doc) {
        if (kw == word) {
            return "{\"contents\":{\"language\":\"ec\",\"value\":\"" + desc + "\"}}";
        }
    }
    
    return "{}";
}

std::vector<std::string> LanguageServer::getDefinition(const std::string& uri, int line, int charPos) {
    std::vector<std::string> result;
    
    auto docIt = openDocuments_.find(uri);
    if (docIt == openDocuments_.end()) return result;
    
    std::string content = docIt->second;
    
    // Find word at position
    std::string word;
    for (int i = charPos; i < (int)content.size() && (isalnum(content[i]) || content[i] == '_'); ++i) {
        word += content[i];
    }
    for (int i = charPos - 1; i >= 0 && (isalnum(content[i]) || content[i] == '_'); --i) {
        word = content[i] + word;
    }
    
    if (word.empty()) return result;
    
    // Search for definition in same file
    std::istringstream iss(content);
    std::string searchLine;
    int searchLineNum = 0;
    
    while (std::getline(iss, searchLine)) {
        searchLineNum++;
        if (searchLine.find("fn " + word + "(") != std::string::npos ||
            searchLine.find("fn " + word + " ") != std::string::npos) {
            result.push_back(std::to_string(searchLineNum - 1));
            break;
        }
    }
    
    return result;
}

std::vector<std::string> LanguageServer::getDocumentSymbols(const std::string& uri) {
    std::vector<std::string> symbols;
    
    auto docIt = openDocuments_.find(uri);
    if (docIt == openDocuments_.end()) return symbols;
    
    std::string content = docIt->second;
    std::istringstream iss(content);
    std::string line;
    int lineNum = 0;
    
    while (std::getline(iss, line)) {
        lineNum++;
        // Match function definitions
        size_t fnPos = line.find("fn ");
        if (fnPos != std::string::npos) {
            size_t nameStart = fnPos + 3;
            size_t nameEnd = line.find("(", nameStart);
            if (nameEnd != std::string::npos) {
                std::string name = line.substr(nameStart, nameEnd - nameStart);
                symbols.push_back(name + " (line " + std::to_string(lineNum) + ")");
            }
        }
        // Match type definitions
        size_t typePos = line.find("type ");
        if (typePos != std::string::npos) {
            size_t nameStart = typePos + 5;
            size_t nameEnd = line.find("=", nameStart);
            if (nameEnd == std::string::npos) nameEnd = line.find("{", nameStart);
            if (nameEnd != std::string::npos) {
                std::string name = line.substr(nameStart, nameEnd - nameStart);
                symbols.push_back(name + " (line " + std::to_string(lineNum) + ")");
            }
        }
    }
    
    return symbols;
}

void LanguageServer::handleRequest(int id, const std::string& method, const std::string& params) {
    if (method == "initialize") {
        std::string result = "{\"capabilities\":{\"textDocumentSync\":1,"
            "\"completionProvider\":{\"resolveProvider\":false,\"triggerCharacters\":[\".\",\":\"]},"
            "\"hoverProvider\":true,"
            "\"definitionProvider\":true,"
            "\"documentSymbolProvider\":true,"
            "\"diagnosticProvider\":{\"interFileDependencies\":false,\"workspaceDiagnostics\":false}}}";
        sendResponse(id, result, 0, "");
    }
    else if (method == "initialized") {
        initialized_ = true;
    }
    else if (method == "textDocument/didOpen") {
        // Extract URI from params
        size_t uriPos = params.find("\"uri\"");
        std::string uri = "";
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        didOpen(uri, "", 0);
        sendResponse(id, "{}", 0, "");
    }
    else if (method == "textDocument/didChange") {
        size_t uriPos = params.find("\"uri\"");
        std::string uri = "";
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        didChange(uri, "");
        sendResponse(id, "{}", 0, "");
    }
    else if (method == "textDocument/didSave") {
        size_t uriPos = params.find("\"uri\"");
        std::string uri = "";
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        didSave(uri);
        sendResponse(id, "{}", 0, "");
    }
    else if (method == "textDocument/didClose") {
        size_t uriPos = params.find("\"uri\"");
        std::string uri = "";
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        didClose(uri);
        sendResponse(id, "{}", 0, "");
    }
    else if (method == "textDocument/completion") {
        // Extract position and URI
        size_t linePos = params.find("\"line\"");
        int line = 0;
        if (linePos != std::string::npos) {
            size_t colonPos = params.find(":", linePos);
            line = std::stoi(params.substr(colonPos + 1));
        }
        
        size_t charPos = params.find("\"character\"");
        int charPosVal = 0;
        if (charPos != std::string::npos) {
            size_t colonPos = params.find(":", charPos);
            charPosVal = std::stoi(params.substr(colonPos + 1));
        }
        
        std::string uri = "";
        size_t uriPos = params.find("\"uri\"");
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        
        auto completions = getCompletions(uri, line, charPosVal);
        std::string result = "{\"items\":[";
        for (size_t i = 0; i < completions.size(); ++i) {
            if (i > 0) result += ",";
            result += "{\"label\":\"" + completions[i] + "\",\"kind\":16}";
        }
        result += "]";
        sendResponse(id, result, 0, "");
    }
    else if (method == "textDocument/hover") {
        size_t linePos = params.find("\"line\"");
        int line = 0;
        if (linePos != std::string::npos) {
            size_t colonPos = params.find(":", linePos);
            line = std::stoi(params.substr(colonPos + 1));
        }
        
        size_t charPos = params.find("\"character\"");
        int charPosVal = 0;
        if (charPos != std::string::npos) {
            size_t colonPos = params.find(":", charPos);
            charPosVal = std::stoi(params.substr(colonPos + 1));
        }
        
        std::string uri = "";
        size_t uriPos = params.find("\"uri\"");
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        
        std::string hover = getHover(uri, line, charPosVal);
        sendResponse(id, hover, 0, "");
    }
    else if (method == "textDocument/definition") {
        size_t linePos = params.find("\"line\"");
        int line = 0;
        if (linePos != std::string::npos) {
            size_t colonPos = params.find(":", linePos);
            line = std::stoi(params.substr(colonPos + 1));
        }
        
        size_t charPos = params.find("\"character\"");
        int charPosVal = 0;
        if (charPos != std::string::npos) {
            size_t colonPos = params.find(":", charPos);
            charPosVal = std::stoi(params.substr(colonPos + 1));
        }
        
        std::string uri = "";
        size_t uriPos = params.find("\"uri\"");
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        
        auto defs = getDefinition(uri, line, charPosVal);
        std::string result = "[";
        for (size_t i = 0; i < defs.size(); ++i) {
            if (i > 0) result += ",";
            result += "{\"uri\":\"" + uri + "\",\"range\":{\"start\":{\"line\":" + defs[i] + ",\"character\":0}}}";
        }
        result += "]";
        sendResponse(id, result, 0, "");
    }
    else if (method == "textDocument/documentSymbol") {
        std::string uri = "";
        size_t uriPos = params.find("\"uri\"");
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        
        auto symbols = getDocumentSymbols(uri);
        std::string result = "[";
        for (size_t i = 0; i < symbols.size(); ++i) {
            if (i > 0) result += ",";
            result += "{\"name\":\"" + symbols[i].substr(0, symbols[i].find(" (")) + "\"}";
        }
        result += "]";
        sendResponse(id, result, 0, "");
    }
    else if (method == "textDocument/diagnostic") {
        std::string uri = "";
        size_t uriPos = params.find("\"uri\"");
        if (uriPos != std::string::npos) {
            size_t colonPos = params.find(":", uriPos);
            size_t quoteStart = params.find("\"", colonPos + 1);
            size_t quoteEnd = params.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                uri = params.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
        
        auto diags = getDiagnostics(uri);
        std::string result = "[";
        for (size_t i = 0; i < diags.size(); ++i) {
            if (i > 0) result += ",";
            result += "{" + diags[i] + "}";
        }
        result += "]";
        sendResponse(id, result, 0, "");
    }
    else if (method == "textDocument/signatureHelp") {
        sendResponse(id, "{}", 0, "");
    }
    else if (method == "shutdown") {
        sendResponse(id, "{}", 0, "");
        exit(0);
    }
    else {
        sendResponse(id, "", -32601, "Method not found: " + method);
    }
}

void LanguageServer::run() {
    std::cout << "EnginotechC++ Language Server v0.2.2\n";
    std::cout << "Initializing...\n";
    
    // Read Content-Length header
    std::string header;
    while (std::getline(std::cin, header) && header != "\r") {
        // Skip headers
    }
    
    // Main message loop
    while (std::getline(std::cin, header)) {
        // Parse Content-Length
        size_t pos = header.find("Content-Length: ");
        if (pos == std::string::npos) continue;
        
        int contentLength = std::stoi(header.substr(pos + 16));
        
        // Read JSON body
        std::string body(contentLength, '\0');
        std::cin.read(&body[0], contentLength);
        
        // Extract method and id
        int id = 0;
        std::string method = "";
        
        // Find "id"
        size_t idPos = body.find("\"id\"");
        if (idPos != std::string::npos) {
            size_t colonPos = body.find(":", idPos);
            if (colonPos != std::string::npos) {
                id = std::stoi(body.substr(colonPos + 1));
            }
        }
        
        // Find "method"
        size_t methodPos = body.find("\"method\"");
        if (methodPos != std::string::npos) {
            size_t colonPos = body.find(":", methodPos);
            if (colonPos != std::string::npos) {
                size_t quoteStart = body.find("\"", colonPos + 1);
                size_t quoteEnd = body.find("\"", quoteStart + 1);
                if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                    method = body.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                }
            }
        }
        
        handleRequest(id, method, body);
    }
}

} // namespace langserver
} // namespace eng
