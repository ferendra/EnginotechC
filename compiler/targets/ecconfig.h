// EnginotechC++ — Embedded Build Configuration (ec.toml parser)
// Parses project configuration from ec.toml

#ifndef ENG_EC_TOML_H
#define ENG_EC_TOML_H

#include <string>
#include <map>
#include <optional>
#include <vector>
#include <cstdint>

namespace eng {
namespace ecconfig {

struct ProjectConfig {
    std::string name;
    std::string version = "0.1.0";

    // Target
    std::string target;           // "arduino", "esp32", "x86"
    std::string board;            // board ID from boards/ directory

    // Build
    std::string optimization = "O2";  // "O0", "O1", "O2", "Os"
    bool debug = false;

    // Embedded profile
    std::string runtimeProfile = "embedded-min";  // "embedded-min" or "embedded-full"

    // Serial / upload
    std::string serialPort;
    uint32_t serialBaud = 115200;
};

// Parse ec.toml from a directory
ProjectConfig parse(const std::string& tomlPath);

// Parse a simple TOML file (subset supporting embedded config)
ProjectConfig parseToml(const std::string& content, const std::string& filePath = "");

// Write a default ec.toml
std::string defaultConfigToml(const std::string& projectName, const std::string& target, const std::string& board);

} // namespace ecconfig
} // namespace eng

#endif // ENG_EC_TOML_H
