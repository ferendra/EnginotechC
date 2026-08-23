#ifndef ENGDIAG_DIAGNOSTIC_H
#define ENGDIAG_DIAGNOSTIC_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <format>

namespace eng {

enum class Severity { Error, Warning, Note, Help };

struct Diagnostic {
    Severity severity;
    std::string code;
    std::string message;
    int line;
    int col;
    std::string file;
    std::vector<std::pair<int, std::string>> notes;

    Diagnostic(Severity s, const std::string& code, const std::string& msg,
               int ln = 0, int cl = 0, const std::string& f = "")
        : severity(s), code(code), message(msg), line(ln), col(cl), file(f) {}
};

class DiagnosticEngine {
public:
    // Hard cap so a cascading failure can't spam thousands of messages;
    // beyond this we collect counts, not text.
    static constexpr size_t MAX_DIAGNOSTICS = 100;

    DiagnosticEngine() = default;

    void error(const std::string& code, const std::string& message,
               int line = 0, int col = 0, const std::string& file = "") {
        if (diagnostics_.size() >= MAX_DIAGNOSTICS) { overflow_++; return; }
        diagnostics_.push_back({Severity::Error, code, message, line, col, file});
    }

    void warning(const std::string& code, const std::string& message,
                 int line = 0, int col = 0, const std::string& file = "") {
        if (diagnostics_.size() >= MAX_DIAGNOSTICS) { overflow_++; return; }
        diagnostics_.push_back({Severity::Warning, code, message, line, col, file});
    }

    void note(const std::string& message, int line = 0, int col = 0) {
        diagnostics_.push_back({Severity::Note, "", message, line, col});
    }
    
    std::string severityToString(Severity s) const {
        switch (s) {
            case Severity::Error:   return "error";
            case Severity::Warning: return "warning";
            case Severity::Note:    return "note";
            case Severity::Help:    return "help";
        }
        return "unknown";
    }

    void addNote(int line, const std::string& message) {
        if (!diagnostics_.empty()) {
            diagnostics_.back().notes.push_back({line, message});
        }
    }

    const std::vector<Diagnostic>& get() const { return diagnostics_; }
    bool hasErrors() const {
        return std::any_of(diagnostics_.begin(), diagnostics_.end(),
                           [](const Diagnostic& d) { return d.severity == Severity::Error; });
    }

    int errorCount() const {
        return std::count_if(diagnostics_.begin(), diagnostics_.end(),
                             [](const Diagnostic& d) { return d.severity == Severity::Error; });
    }

    void print(const std::vector<std::string>& sourceLines = {}) const {
        for (const auto& d : diagnostics_) {
            std::string prefix = colorForSeverity(d.severity);
            std::string suffix = "\033[0m";
            std::cerr << prefix << "[" << severityToString(d.severity) << "] ";
            if (!d.code.empty()) {
                std::cerr << "(" << d.code << ") ";
            }
            if (!d.file.empty()) {
                std::cerr << d.file << ":" << d.line << ":" << d.col << " -> ";
            }
            std::cerr << d.message << suffix << "\n";

            // Show source line with caret if source available
            if (!sourceLines.empty() && d.line > 0 && d.line <= (int)sourceLines.size()) {
                const std::string& line = sourceLines[d.line - 1];
                // Clamp col to line length
                int col = d.col;
                if (col < 1) col = 1;
                if (col > (int)line.size()) col = line.size();
                // Print the source line (truncate if too long)
                std::string displayLine = line;
                if ((int)displayLine.size() > 120) {
                    displayLine = displayLine.substr(0, 117) + "...";
                }
                std::cerr << "    | " << displayLine << "\n";
                // Print caret pointing to column
                std::string caret(col - 1, ' ');
                std::cerr << "    | " << caret << "^\n";
            }

            for (const auto& [ln, msg] : d.notes) {
                std::cerr << "    at line " << ln << ": " << msg << "\n";
            }
        }
        if (overflow_ > 0) {
            std::cerr << "\033[31;1m... and " << overflow_
                      << " more diagnostic(s) suppressed (limit "
                      << MAX_DIAGNOSTICS << ")\033[0m\n";
        }
        // Summary line — the compiler never dies silently on the first error.
        int errs = errorCount();
        int warns = warningCount();
        if (errs > 0 || warns > 0) {
            std::cerr << "\n\033[1m=== Compilation diagnostics: " << errs
                      << " error(s), " << warns << " warning(s) ===\033[0m\n";
        }
    }

    int warningCount() const {
        return static_cast<int>(std::count_if(
            diagnostics_.begin(), diagnostics_.end(),
            [](const Diagnostic& d) { return d.severity == Severity::Warning; }));
    }

    size_t totalCollected() const { return diagnostics_.size() + overflow_; }

    std::string formatForLsp() const {
        std::ostringstream oss;
        for (const auto& d : diagnostics_) {
            int code = static_cast<int>(d.severity);
            oss << "{\"severity\":" << code
                << ",\"code\":\"" << d.code << "\""
                << ",\"message\":\"" << d.message << "\""
                << ",\"line\":" << d.line
                << ",\"column\":" << d.col << "}\n";
        }
        return oss.str();
    }

private:
    std::string colorForSeverity(Severity s) const {
        switch (s) {
            case Severity::Error:   return "\033[31;1m"; // red bold
            case Severity::Warning: return "\033[33;1m"; // yellow bold
            case Severity::Note:    return "\033[34m";   // blue
            case Severity::Help:    return "\033[36m";   // cyan
        }
        return "";
    }
    std::string severityStr() const {
        // overridden per-call
        return "";
    }

    std::vector<Diagnostic> diagnostics_;
    size_t overflow_ = 0;
};

} // namespace eng

#endif // ENGDIAG_DIAGNOSTIC_H
