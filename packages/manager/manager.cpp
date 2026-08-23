// EnginotechC++ — Package Manager Implementation
// Local filesystem-based package management for embedded projects

#include "manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cstring>

namespace eng {
namespace pkgmgr {
namespace fs = std::filesystem;

std::string PackageManager::getPackageDir() {
    return ".ec_packages";
}

std::string PackageManager::getManifestPath() {
    return getPackageDir() + "/manifest.json";
}

bool PackageManager::init(const std::string& project_dir) {
    (void)project_dir;
    // Create package directory structure
    fs::create_directories(getPackageDir());
    fs::create_directories(getPackageDir() + "/include");
    fs::create_directories(getPackageDir() + "/src");

    std::ofstream manifest(getManifestPath());
    manifest << "{\n  \"version\": 1,\n  \"packages\": {}\n}\n";
    manifest.close();
    return true;
}

bool PackageManager::readManifest(std::map<std::string, InstalledPackage>& out) {
    out.clear();
    std::ifstream f(getManifestPath());
    if (!f) return false;

    std::stringstream ss;
    ss << f.rdbuf();
    std::string content = ss.str();

    // Simple JSON parsing for flat manifest
    // Format: "pkgname": {"version": "...", "path": "...", ...}
    size_t pos = content.find("\"packages\":");
    if (pos == std::string::npos) return false;
    pos += strlen("\"packages\":");

    // Find matching braces
    int depth = 0;
    size_t start = content.find('{', pos);
    size_t end = content.rfind('}');
    if (start == std::string::npos || end == std::string::npos) return false;

    std::string pkgBlock = content.substr(start, end - start + 1);

    // Parse each package entry
    size_t i = 0;
    while (i < pkgBlock.size()) {
        // Find key
        size_t keyStart = pkgBlock.find('"', i);
        if (keyStart == std::string::npos) break;
        size_t keyEnd = pkgBlock.find('"', keyStart + 1);
        if (keyEnd == std::string::npos) break;
        std::string name = pkgBlock.substr(keyStart + 1, keyEnd - keyStart - 1);

        // Find value object
        size_t valStart = pkgBlock.find('{', keyEnd);
        size_t valEnd = pkgBlock.find('}', valStart);
        if (valStart == std::string::npos || valEnd == std::string::npos) break;

        std::string valStr = pkgBlock.substr(valStart + 1, valEnd - valStart - 1);

        InstalledPackage pkg{};
        pkg.name = name;

        // Extract version
        size_t verPos = valStr.find("\"version\"");
        if (verPos != std::string::npos) {
            size_t colon = valStr.find(':', verPos);
            size_t quote1 = valStr.find('"', colon);
            size_t quote2 = valStr.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                pkg.version = valStr.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        // Extract path
        size_t pathPos = valStr.find("\"path\"");
        if (pathPos != std::string::npos) {
            size_t colon = valStr.find(':', pathPos);
            size_t quote1 = valStr.find('"', colon);
            size_t quote2 = valStr.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                pkg.install_path = valStr.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        out[name] = pkg;
        i = valEnd + 1;
    }

    return !out.empty();
}

bool PackageManager::writeManifest(const std::map<std::string, InstalledPackage>& manifest) {
    std::ofstream f(getManifestPath());
    f << "{\n  \"version\": 1,\n  \"packages\": {\n";
    bool first = true;
    for (const auto& [name, pkg] : manifest) {
        if (!first) f << ",\n";
        f << "    \"" << name << "\": {\n"
          << "      \"version\": \"" << pkg.version << "\",\n"
          << "      \"path\": \"" << pkg.install_path << "\"\n"
          << "    }";
        first = false;
    }
    f << "\n  }\n}\n";
    f.close();
    return true;
}

std::string PackageManager::resolveSource(const std::string& pkg_name) {
    auto* info = packages::findPackage(pkg_name);
    if (!info) return "";
    return info->include_path;
}

bool PackageManager::add(const std::string& package_name) {
    auto* info = packages::findPackage(package_name);
    if (!info) {
        std::cerr << "Error: Package '" << package_name << "' not found in registry.\n";
        return false;
    }

    if (isInstalled(package_name)) {
        std::cout << "Package '" << package_name << "' is already installed.\n";
        return true;
    }

    // Check dependencies
    auto deps = packages::getDependencies(package_name);
    for (const auto& dep : deps) {
        if (!isInstalled(dep)) {
            std::cout << "Installing dependency: " << dep << "\n";
            add(dep);
        }
    }

    // Create package directory
    std::string pkgDir = getPackageDir() + "/" + package_name;
    fs::create_directories(pkgDir);
    fs::create_directories(pkgDir + "/include");
    fs::create_directories(pkgDir + "/src");

    // Copy source files from std/embedded/packages/
    fs::path srcBase = fs::path("std") / "embedded" / "packages" / package_name;
    if (fs::exists(srcBase)) {
        for (const auto& entry : fs::recursive_directory_iterator(srcBase)) {
            if (entry.is_regular_file()) {
                fs::path relPath = fs::relative(entry.path(), srcBase);
                fs::path dest = pkgDir / relPath;
                fs::create_directories(dest.parent_path());
                fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
            }
        }
    }

    // Update manifest
    std::map<std::string, InstalledPackage> manifest;
    readManifest(manifest);

    InstalledPackage pkg{};
    pkg.name = package_name;
    pkg.version = info->version;
    pkg.install_path = pkgDir;
    manifest[package_name] = pkg;
    writeManifest(manifest);

    std::cout << "Installed " << package_name << " v" << info->version << "\n";
    std::cout << "  Source: " << info->source_url << "\n";
    std::cout << "  Path: " << pkgDir << "\n";
    if (!deps.empty()) {
        std::cout << "  Dependencies: ";
        for (size_t i = 0; i < deps.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << deps[i];
        }
        std::cout << "\n";
    }
    return true;
}

bool PackageManager::remove(const std::string& package_name) {
    if (!isInstalled(package_name)) {
        std::cerr << "Error: Package '" << package_name << "' is not installed.\n";
        return false;
    }

    fs::path pkgDir = fs::path(getPackageDir()) / package_name;
    fs::remove_all(pkgDir);

    std::map<std::string, InstalledPackage> manifest;
    readManifest(manifest);
    manifest.erase(package_name);
    writeManifest(manifest);

    std::cout << "Removed " << package_name << "\n";
    return true;
}

std::vector<InstalledPackage> PackageManager::listInstalled() {
    std::map<std::string, InstalledPackage> manifest;
    readManifest(manifest);
    std::vector<InstalledPackage> result;
    for (const auto& [name, pkg] : manifest) {
        result.push_back(pkg);
    }
    return result;
}

std::vector<packages::PackageInfo> PackageManager::search(const std::string& query) {
    return packages::search(query);
}

std::vector<std::string> PackageManager::getInstalledNames() {
    auto pkgs = listInstalled();
    std::vector<std::string> names;
    for (const auto& p : pkgs) names.push_back(p.name);
    return names;
}

bool PackageManager::isInstalled(const std::string& name) {
    auto names = getInstalledNames();
    return std::find(names.begin(), names.end(), name) != names.end();
}

std::vector<std::string> PackageManager::getIncludePaths() {
    std::vector<std::string> paths;
    paths.push_back("std/embedded");
    paths.push_back("std/embedded/packages/dht");
    paths.push_back("std/embedded/packages/bme280");
    paths.push_back("std/embedded/packages/mpu6050");
    paths.push_back("std/embedded/packages/ssd1306");
    paths.push_back("std/embedded/packages/ws2812");
    paths.push_back("std/embedded/packages/servo");
    paths.push_back("std/embedded/packages/relay");
    paths.push_back("std/embedded/packages/ultrasonic");
    paths.push_back("std/embedded/packages/rfid");
    paths.push_back("std/embedded/packages/rtc");
    paths.push_back("std/embedded/packages/gps");
    paths.push_back("std/embedded/packages/mqtt");
    paths.push_back("std/embedded/packages/webserver");
    paths.push_back("std/embedded/packages/storage");
    paths.push_back("std/embedded/packages/power");
    paths.push_back("std/embedded/packages/ble");
    paths.push_back("std/embedded/packages/taskscheduler");
    auto installed = listInstalled();
    for (const auto& p : installed) {
        paths.push_back(p.install_path + "/include");
    }
    return paths;
}

std::vector<std::string> PackageManager::getSources() {
    std::vector<std::string> sources;
    for (const auto& p : listInstalled()) {
        fs::path srcDir = fs::path(p.install_path) / "src";
        if (fs::exists(srcDir)) {
            for (const auto& entry : fs::recursive_directory_iterator(srcDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                    sources.push_back(entry.path().string());
                }
            }
        }
    }
    return sources;
}

void PackageManager::showInfo(const std::string& name) {
    auto* info = packages::findPackage(name);
    if (!info) {
        std::cerr << "Error: Package '" << name << "' not found.\n";
        return;
    }
    std::cout << "Package: " << info->name << " v" << info->version << "\n";
    std::cout << "Description: " << info->description << "\n";
    std::cout << "Author: " << info->author << "\n";
    std::cout << "License: " << info->license << "\n";
    std::cout << "Source: " << info->source_url << "\n";
    std::cout << "Targets: ";
    if (info->for_arduino) std::cout << "Arduino ";
    if (info->for_esp32) std::cout << "ESP32 ";
    if (info->for_rpi_pico) std::cout << "RP2040 ";
    std::cout << "\n";
    std::cout << "Tags: ";
    for (size_t i = 0; i < info->tags.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << info->tags[i];
    }
    std::cout << "\n";
    if (!info->dependencies.empty()) {
        std::cout << "Dependencies: ";
        for (size_t i = 0; i < info->dependencies.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << info->dependencies[i];
        }
        std::cout << "\n";
    }
    std::cout << "Usage: " << info->readme << "\n";
}

void PackageManager::printList() {
    auto pkgs = packages::getRegistry();
    std::cout << "EnginotechC++ Package Registry (" << pkgs.size() << " packages)\n\n";
    for (const auto& p : pkgs) {
        std::string targets = "";
        if (p.for_arduino) targets += "arduino ";
        if (p.for_esp32) targets += "esp32 ";
        if (p.for_rpi_pico) targets += "rpi-pico ";
        std::cout << "  " << p.name << " v" << p.version
                  << " [" << targets << "]" << "\n";
        std::cout << "    " << p.description << "\n";
    }
    std::cout << "\nUse 'engc add <name>' to install a package.\n";
}

} // namespace pkgmgr
} // namespace eng
