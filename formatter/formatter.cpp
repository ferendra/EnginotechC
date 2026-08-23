#include "formatter.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace eng {
namespace formatter {

std::string Formatter::getIndentation(int level, const FormatOptions& options) {
    std::string indent = "";
    std::string sep = options.useTabs ? "\t" : std::string(options.indentSize, ' ');
    for (int i = 0; i < level; ++i) {
        indent += sep;
    }
    return indent;
}

std::string Formatter::format(const std::string& source, const FormatOptions& options) {
    // Simplified formatter - M0 placeholder
    // Full implementation will handle tokenization and AST-based formatting
    return source;
}

std::string Formatter::formatFile(const std::string& path, const FormatOptions& options) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Cannot read file: " << path << "\n";
        return "";
    }
    
    std::stringstream ss;
    ss << file.rdbuf();
    return format(ss.str(), options);
}

bool Formatter::writeFormatted(const std::string& input, const std::string& outputPath, const FormatOptions& options) {
    std::string formatted = format(input, options);
    
    std::ofstream file(outputPath);
    if (!file) {
        std::cerr << "Error: Cannot write to: " << outputPath << "\n";
        return false;
    }
    
    file << formatted;
    return true;
}

} // namespace formatter
} // namespace eng
