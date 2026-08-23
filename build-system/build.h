// Build system helper for EnginotechC++
#ifndef ENG_BUILD_SYSTEM_H
#define ENG_BUILD_SYSTEM_H

#include <string>
#include <vector>
#include <filesystem>

namespace eng {
namespace build {

struct BuildConfig {
    std::string buildType = "Debug";
    std::string compiler = "";
    std::string targetArch = "x86_64";
    std::string outputDir = "build";
};

class BuildSystem {
public:
    static bool configure(const BuildConfig& config);
    static bool build(const BuildConfig& config);
    static bool install(const BuildConfig& config, const std::string& prefix);
    static std::string detectCompiler();
    static std::vector<std::string> getInstalledCompilers();
private:
    static bool runCommand(const std::string& cmd, const std::vector<std::string>& args);
    static std::string findCMake();
    static std::string findLLVMClang();
};

} // namespace build
} // namespace eng

#endif // ENG_BUILD_SYSTEM_H
