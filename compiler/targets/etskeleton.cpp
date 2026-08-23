// EnginotechC++ — ETC Target Backend Implementation
// Skeleton implementation for .etc script files

#include "etskeleton.h"
#include <algorithm>
#include <cctype>

namespace eng {
namespace target {

// ── File Reading & Tokenization ───────────────────────────────────

EtcSkeleton::ScriptResult EtcSkeleton::tokenizeFile(const std::string& filePath) {
    ScriptResult result;
    result.success = false;

    // Read file
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.error = "Cannot open file: " + filePath;
        return result;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();
    file.close();

    if (source.empty()) {
        result.error = "Empty file: " + filePath;
        return result;
    }

    // Tokenize
    Lexer lexer(source);
    result.tokens = lexer.tokenize();
    result.errorCount = lexer.diagnostics.size();

    // Check for errors
    if (result.errorCount > 0) {
        result.error = "Lexer errors: " + std::to_string(result.errorCount);
        for (const auto& diag : lexer.diagnostics) {
            result.error += "\n  L" + std::to_string(diag.line) +
                           ":C" + std::to_string(diag.col) +
                           " - " + diag.message;
        }
        return result;
    }

    result.success = true;
    return result;
}

// ── Parsing ───────────────────────────────────────────────────────

EtcSkeleton::ScriptResult EtcSkeleton::parseTokens(const std::vector<Token>& tokens) {
    ScriptResult result;
    result.success = true;
    result.tokens = tokens;

    // Walk through tokens and validate structure
    size_t i = 0;
    while (i < tokens.size()) {
        const auto& t = tokens[i];

        // Skip non-command tokens
        if (t.type == TokenType::COMMENT ||
            t.type == TokenType::NEWLINE ||
            t.type == TokenType::WHITESPACE ||
            t.type == TokenType::TOKEN_EOF) {
            ++i;
            continue;
        }

        // Must be a command: print, output, or say
        if (t.type != TokenType::PRINT &&
            t.type != TokenType::OUTPUT &&
            t.type != TokenType::SAY) {
            result.error = "Expected print/output/say, found: '" + t.lexeme +
                          "' (" + t.tokenName() + ")";
            result.success = false;
            break;
        }

        // Found command, now expect:
        // Option 1: print("string") - LPAREN STRING RPAREN
        // Option 2: output "string" - STRING (no parens)

        ++i; // consume command

        // Skip whitespace
        while (i < tokens.size() && (tokens[i].type == TokenType::NEWLINE ||
               tokens[i].type == TokenType::WHITESPACE)) {
            ++i;
        }

        if (i >= tokens.size() || tokens[i].type == TokenType::TOKEN_EOF) {
            result.error = "Unexpected end of input after '" + t.lexeme + "'";
            result.success = false;
            break;
        }

        if (tokens[i].type == TokenType::LPAREN) {
            // print("...") syntax
            ++i; // consume '('

            // Skip whitespace
            while (i < tokens.size() && (tokens[i].type == TokenType::NEWLINE ||
                   tokens[i].type == TokenType::WHITESPACE)) {
                ++i;
            }

            if (i >= tokens.size() || tokens[i].type != TokenType::STRING_LITERAL) {
                result.error = "Expected string in " + t.lexeme + "()";
                result.success = false;
                break;
            }
            ++i; // consume string

            // Skip whitespace
            while (i < tokens.size() && (tokens[i].type == TokenType::NEWLINE ||
                   tokens[i].type == TokenType::WHITESPACE)) {
                ++i;
            }

            if (i >= tokens.size() || tokens[i].type != TokenType::RPAREN) {
                result.error = "Expected ')' in " + t.lexeme + "()";
                result.success = false;
                break;
            }
            ++i; // consume ')'
        } else if (tokens[i].type == TokenType::STRING_LITERAL) {
            // output "..." syntax (no parens)
            ++i; // consume string
        } else {
            result.error = "Expected '(' or string after '" + t.lexeme + "', got: '" +
                          tokens[i].lexeme + "'";
            result.success = false;
            break;
        }
    }

    return result;
}

// ── Execution ─────────────────────────────────────────────────────

EtcSkeleton::ScriptResult EtcSkeleton::execute(const std::vector<Token>& tokens) {
    ScriptResult result;
    result.success = true;

    size_t i = 0;
    while (i < tokens.size() && tokens[i].type != TokenType::TOKEN_EOF) {
        // Skip whitespace/newlines/comments
        if (tokens[i].type == TokenType::NEWLINE ||
            tokens[i].type == TokenType::WHITESPACE ||
            tokens[i].type == TokenType::COMMENT) {
            ++i;
            continue;
        }

        // Check for print/output/say command
        if (tokens[i].type == TokenType::PRINT ||
            tokens[i].type == TokenType::OUTPUT ||
            tokens[i].type == TokenType::SAY) {

            std::string cmdName = tokens[i].lexeme;
            ++i; // consume command

            // Skip any whitespace
            while (i < tokens.size() && (tokens[i].type == TokenType::NEWLINE ||
                   tokens[i].type == TokenType::WHITESPACE)) {
                ++i;
            }

            if (i >= tokens.size() || tokens[i].type == TokenType::TOKEN_EOF) {
                break;
            }

            if (tokens[i].type == TokenType::LPAREN) {
                // Syntax: print("...")
                ++i; // skip '('

                // Skip whitespace
                while (i < tokens.size() && (tokens[i].type == TokenType::NEWLINE ||
                       tokens[i].type == TokenType::WHITESPACE)) {
                    ++i;
                }

                // Get string value
                if (i < tokens.size() && tokens[i].type == TokenType::STRING_LITERAL) {
                    std::string strVal = extractStringValue(tokens[i]);
                    result.output += strVal;
                    std::cout << strVal;
                    ++i;
                }

                // Skip whitespace
                while (i < tokens.size() && (tokens[i].type == TokenType::NEWLINE ||
                       tokens[i].type == TokenType::WHITESPACE)) {
                    ++i;
                }

                // Skip ')'
                if (i < tokens.size() && tokens[i].type == TokenType::RPAREN) {
                    ++i;
                }
            } else if (tokens[i].type == TokenType::STRING_LITERAL) {
                // Syntax: output "..." or say "..." (without parentheses)
                std::string strVal = extractStringValue(tokens[i]);
                result.output += strVal;
                std::cout << strVal;
                ++i;
            }

            // Add newline after each command
            std::cout << "\n";
            result.output += "\n";
        } else {
            // Unknown token, skip it
            ++i;
        }
    }

    return result;
}

// ── Full Pipeline ─────────────────────────────────────────────────

EtcSkeleton::ScriptResult EtcSkeleton::runFile(const std::string& filePath) {
    // Step 1: Tokenize
    auto tokenizeResult = tokenizeFile(filePath);
    if (!tokenizeResult.success) {
        return tokenizeResult;
    }

    std::cout << "=== Token Summary ===\n";
    printTokenSummary(tokenizeResult.tokens);

    // Step 2: Parse
    auto parseResult = parseTokens(tokenizeResult.tokens);
    if (!parseResult.success) {
        parseResult.tokens = tokenizeResult.tokens;
        return parseResult;
    }

    // Step 3: Execute
    auto execResult = execute(parseResult.tokens);
    execResult.tokens = tokenizeResult.tokens;
    execResult.success = parseResult.success && execResult.success;

    return execResult;
}

// ── Helper Methods ────────────────────────────────────────────────

std::string EtcSkeleton::extractStringValue(const Token& token) {
    // token.lexeme contains the string content (without quotes)
    return token.lexeme;
}

long long EtcSkeleton::extractNumberValue(const Token& token) {
    try {
        return std::stoll(token.lexeme);
    } catch (...) {
        return 0;
    }
}

std::string EtcSkeleton::evalExpression(const std::vector<Token>& tokens, size_t& pos) {
    // Simple expression evaluator for strings and numbers
    if (pos >= tokens.size()) return "";

    const Token& t = tokens[pos];
    if (t.type == TokenType::STRING_LITERAL) {
        std::string val = extractStringValue(t);
        pos++;
        return val;
    } else if (t.type == TokenType::INT_LITERAL) {
        long long val = extractNumberValue(t);
        pos++;
        return std::to_string(val);
    } else if (t.type == TokenType::FLOAT_LITERAL) {
        double val = std::stod(t.lexeme);
        pos++;
        return std::to_string(val);
    }
    return "";
}

void EtcSkeleton::printTokenSummary(const std::vector<Token>& tokens) {
    std::cout << "Total tokens: " << tokens.size() << "\n";
    std::cout << "Tokens by type:\n";

    // Count by type
    std::map<TokenType, int> typeCounts;
    for (const auto& t : tokens) {
        typeCounts[t.type]++;
    }

    // Print summary
    static const std::map<TokenType, std::string> typeNames = {
        {TokenType::PRINT, "PRINT"},
        {TokenType::OUTPUT, "OUTPUT"},
        {TokenType::SAY, "SAY"},
        {TokenType::INPUT, "INPUT"},
        {TokenType::LPAREN, "LPAREN (()"},
        {TokenType::RPAREN, "RPAREN ())"},
        {TokenType::STRING_LITERAL, "STRING"},
        {TokenType::INT_LITERAL, "INT"},
        {TokenType::FLOAT_LITERAL, "FLOAT"},
        {TokenType::NEWLINE, "NEWLINE"},
        {TokenType::WHITESPACE, "WHITESPACE"},
        {TokenType::TOKEN_EOF, "EOF"},
    };

    for (const auto& [type, count] : typeCounts) {
        auto it = typeNames.find(type);
        std::string name = (it != typeNames.end()) ? it->second : "UNKNOWN";
        std::cout << "  " << name << ": " << count << "\n";
    }
}

} // namespace target
} // namespace eng
