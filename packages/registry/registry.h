// EnginotechC++ — Embedded Package Registry
// Metadata for all available embedded packages in the ecosystem

#ifndef ENG_PACKAGES_REGISTRY_H
#define ENG_PACKAGES_REGISTRY_H

#include <string>
#include <vector>
#include <map>

namespace eng {
namespace packages {

struct PackageInfo {
    std::string name;           // Package identifier (e.g. "dht")
    std::string version;        // Version string
    std::string description;    // Short description
    std::string author;         // Original author
    std::vector<std::string> tags;  // Categorization tags
    std::vector<std::string> dependencies; // Other packages this depends on
    std::string license;        // License type
    bool for_arduino = false;   // Compatible with Arduino backend
    bool for_esp32 = false;     // Compatible with ESP32 backend
    bool for_rpi_pico = false;  // Compatible with RP2040
    std::string include_path;   // Where headers are installed
    std::string source_url;     // Original source repo
    std::string readme;         // Short readme/usage notes
};

// Get all available packages from the registry
std::vector<PackageInfo> getRegistry();

// Find a package by name
PackageInfo* findPackage(const std::string& name);

// Search packages by keyword
std::vector<PackageInfo> search(const std::string& keyword);

// Filter packages by target board
std::vector<PackageInfo> filterByTarget(const std::string& target);

// Get dependency tree for a package
std::vector<std::string> getDependencies(const std::string& packageName);

} // namespace packages
} // namespace eng

#endif // ENG_PACKAGES_REGISTRY_H
