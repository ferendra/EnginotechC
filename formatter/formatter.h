// Basic formatter utility for EnginotechC++
#ifndef ENG_FORMATTER_H
#define ENG_FORMATTER_H

#include <string>
#include <vector>

namespace eng {
namespace formatter {

struct FormatOptions {
    int indentSize = 4;
    bool useTabs = false;
    int maxWidth = 100;
};

class Formatter {
public:
    static std::string format(const std::string& source, const FormatOptions& options = FormatOptions());
    static std::string formatFile(const std::string& path, const FormatOptions& options = FormatOptions());
    static bool writeFormatted(const std::string& input, const std::string& outputPath, const FormatOptions& options = FormatOptions());
    
private:
    std::string doFormat(const std::string& source, const FormatOptions& options);
    std::string getIndentation(int level, const FormatOptions& options);
};

} // namespace formatter
} // namespace eng

#endif // ENG_FORMATTER_H
