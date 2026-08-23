// Package manager for EnginotechC++
#ifndef ENG_PACKAGE_MANAGER_H
#define ENG_PACKAGE_MANAGER_H

#include <string>
#include <vector>
#include <map>

namespace eng {
namespace pkgmgr {

struct Package {
    std::string name;
    std::string version;
    std::string description;
    std::map<std::string, std::string> dependencies;
};

class PackageManager {
public:
    static bool init(const std::string& projectDir);
    static bool install(const std::string& packageName, const std::string& version = "");
    static bool remove(const std::string& packageName);
    static std::vector<Package> listInstalled();
    static bool search(const std::string& query);
    static bool updateAll();
    
    // Additional methods for driver commands
    static bool add(const std::string& packageName);
    static void printList();
    static void showInfo(const std::string& packageName);
    
private:
    static std::string getPackageManagerPath();
    static bool runCommand(const std::string& cmd);
};

} // namespace pkgmgr
} // namespace eng

#endif // ENG_PACKAGE_MANAGER_H
