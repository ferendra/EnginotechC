// EnginotechC++ — Embedded CLI Commands
// Extends the compiler driver with embedded-specific commands:
//   engc build --target <target> [--board <board>]
//   engc flash --board <board> [--port <port>]
//   engc monitor --baud <baud> [--port <port>]
//   engc doctor
//   engc new <name> --target <target> [--board <board>]

#ifndef ENG_DRIVER_EMBEDDED_H
#define ENG_DRIVER_EMBEDDED_H

#include <string>
#include <vector>
#include <optional>
#include "../diagnostics/diagnostic.h"

namespace eng {
namespace driver {

class EmbeddedCommands {
public:
    // Build with embedded target
    static int cmdBuildEmbedded(const std::vector<std::string>& args);

    // Flash firmware to board
    static int cmdFlash(const std::vector<std::string>& args);

    // Serial monitor
    static int cmdMonitor(const std::vector<std::string>& args);

    // Embedded system check
    static int cmdDoctor();

    // Create new embedded project
    static int cmdNewEmbedded(const std::vector<std::string>& args);

private:
    static std::string findTool(const std::string& name);
    static bool hasTool(const std::string& name);
    static std::string detectBoard();
    static std::vector<std::string> parseArgs(const std::vector<std::string>& args,
                                               const std::string& flag);
    static std::optional<std::string> getArgValue(const std::vector<std::string>& args,
                                                   const std::string& flag);
};

} // namespace driver
} // namespace eng

#endif // ENG_DRIVER_EMBEDDED_H
