// EnginotechC++ — Package Manager (local filesystem-based)
// Manages installed packages in the project's .ec_packages/ directory

#ifndef ENG_PACKAGES_MANAGER_H
#define ENG_PACKAGES_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "../registry/registry.h"

namespace eng {
namespace pkgmgr {

struct InstalledPackage {
    std::string name;
    std::string version;
    std::string install_path;  // Relative to project root
    std::string metadata;      // JSON of package info
};

class PackageManager {
public:
    // Initialize package manager for a project
    static bool init(const std::string& project_dir);

    // Add/install a package (from registry)
    static bool add(const std::string& package_name);

    // Remove a package
    static bool remove(const std::string& package_name);

    // List installed packages
    static std::vector<InstalledPackage> listInstalled();

    // Search registry
    static std::vector<packages::PackageInfo> search(const std::string& query);

    // Get all installed package names
    static std::vector<std::string> getInstalledNames();

    // Check if package is installed
    static bool isInstalled(const std::string& name);

    // Get include paths for all installed packages
    static std::vector<std::string> getIncludePaths();

    // Get sources to compile for all installed packages
    static std::vector<std::string> getSources();

    // Print package info (for engc show <pkg>)
    static void showInfo(const std::string& name);

    // Print all packages (for engc list)
    static void printList();

private:
    static std::string getPackageDir();
    static std::string getManifestPath();
    static bool readManifest(std::map<std::string, InstalledPackage>& out);
    static bool writeManifest(const std::map<std::string, InstalledPackage>& manifest);
    static std::string resolveSource(const std::string& pkg_name);
};

} // namespace pkgmgr
} // namespace eng

#endif // ENG_PACKAGES_MANAGER_H
