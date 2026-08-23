#include "package_manager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace eng {
namespace pkgmgr {
namespace fs = std::filesystem;

bool PackageManager::init(const std::string& projectDir) {
    std::string configPath = projectDir + "/.engc";
    std::ofstream config(configPath);
    if (!config) {
        std::cerr << "Error: Cannot create project config\n";
        return false;
    }
    config << "# EnginotechC++ Project Configuration\n";
    config << "name = \"" << fs::path(projectDir).filename() << "\"\n";
    config << "version = \"0.1.0\"\n";
    config << "dependencies = []\n";
    config.close();
    return true;
}

bool PackageManager::install(const std::string& packageName, const std::string& version) {
    // M0: Placeholder - will be implemented with registry support
    std::cout << "Installing " << packageName << (version.empty() ? "" : ":" + version) << "...\n";
    std::cout << "[M0] Package installation not yet connected to registry\n";
    return true;
}

bool PackageManager::remove(const std::string& packageName) {
    std::cout << "Removing " << packageName << "...\n";
    return true;
}

std::vector<Package> PackageManager::listInstalled() {
    return {};
}

bool PackageManager::search(const std::string& query) {
    std::cout << "Searching for: " << query << "\n";
    std::cout << "[M0] Package registry not yet available\n";
    return true;
}

bool PackageManager::updateAll() {
    std::cout << "Updating all packages...\n";
    std::cout << "[M0] Package updates not yet available\n";
    return true;
}

bool PackageManager::add(const std::string& packageName) {
    std::cout << "Adding package: " << packageName << "...\n";
    return install(packageName);
}

void PackageManager::printList() {
    auto packages = listInstalled();
    if (packages.empty()) {
        std::cout << "No packages installed.\n";
    } else {
        std::cout << "Installed packages:\n";
        for (const auto& pkg : packages) {
            std::cout << "  - " << pkg.name << " v" << pkg.version << "\n";
        }
    }
}

void PackageManager::showInfo(const std::string& packageName) {
    std::cout << "Package: " << packageName << " (information not yet available)\n";
}

std::string PackageManager::getPackageManagerPath() {
    // Return path to engc package manager
    return "engc";
}

bool PackageManager::runCommand(const std::string& cmd) {
    return std::system(cmd.c_str()) == 0;
}

} // namespace pkgmgr
} // namespace eng
