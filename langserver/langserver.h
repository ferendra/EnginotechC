// EnginotechC++ — Language Server Protocol (LSP) Implementation
// Provides IDE features: completion, diagnostics, go-to-definition, hover
// Protocol: JSON-RPC 2.0 over stdio

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace eng {
namespace langserver {

class LanguageServer {
public:
    LanguageServer();
    ~LanguageServer();
    
    // Main loop - processes JSON-RPC messages from stdin
    void run();
    
    // Initialize server capabilities
    void initialize();
    
    // Text Document synchronization
    void didOpen(const std::string& uri, const std::string& content, int version);
    void didChange(const std::string& uri, const std::string& content);
    void didSave(const std::string& uri);
    void didClose(const std::string& uri);
    
    // Diagnostics
    std::vector<std::string> getDiagnostics(const std::string& uri);
    
    // Completion
    std::vector<std::string> getCompletions(const std::string& uri, int line, int charPos);
    
    // Hover
    std::string getHover(const std::string& uri, int line, int charPos);
    
    // Go to Definition
    std::vector<std::string> getDefinition(const std::string& uri, int line, int charPos);
    
    // Document Symbols
    std::vector<std::string> getDocumentSymbols(const std::string& uri);
    
private:
    void sendResponse(int id, const std::string& result, int errorCode, const std::string& errorMsg);
    void handleRequest(int id, const std::string& method, const std::string& params);
    void computeDiagnostics(const std::string& uri);
    
    // State
    std::unordered_map<std::string, std::string> openDocuments_;
    std::unordered_map<std::string, std::vector<std::string>> diagnostics_;
    bool initialized_ = false;
};

} // namespace langserver
} // namespace eng
