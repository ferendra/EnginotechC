// EnginotechC++ — ec.toml Parser Implementation

#include "ecconfig.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace eng {
namespace ecconfig {
namespace fs = std::filesystem;

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string unquote(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

ProjectConfig parse(const std::string& tomlPath) {
    std::ifstream f(tomlPath);
    if (!f) return ProjectConfig{};
    std::stringstream ss;
    ss << f.rdbuf();
    return parseToml(ss.str(), tomlPath);
}

ProjectConfig parseToml(const std::string& content, const std::string&) {
    ProjectConfig cfg;
    std::istringstream iss(content);
    std::string line;
    std::string currentSection;

    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        // Section header
        if (line[0] == '[') {
            size_t end = line.find(']');
            if (end != std::string::npos) {
                currentSection = trim(line.substr(1, end - 1));
            }
            continue;
        }

        // Key = value
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(unquote(line.substr(eq + 1)));

        if (currentSection == "project") {
            if (key == "name") cfg.name = val;
        } else if (currentSection == "target") {
            if (key == "board") cfg.board = val;
            else if (key == "target") cfg.target = val;
        } else if (currentSection == "build") {
            if (key == "optimization") cfg.optimization = val;
            else if (key == "debug") cfg.debug = (val == "true");
        } else if (currentSection == "embedded") {
            if (key == "runtime") cfg.runtimeProfile = val;
        } else if (currentSection == "serial") {
            if (key == "port") cfg.serialPort = val;
            else if (key == "baud") cfg.serialBaud = std::stoul(val);
        }
    }

    return cfg;
}

std::string defaultConfigToml(const std::string& projectName,
                               const std::string& target,
                               const std::string& board) {
    return "[project]\n"
           "name = \"" + projectName + "\"\n"
           "version = \"0.1.0\"\n\n"
           "[target]\n"
           "target = \"" + target + "\"\n"
           "board = \"" + board + "\"\n\n"
           "[build]\n"
           "optimization = \"O2\"\n"
           "debug = false\n\n"
           "[embedded]\n"
           "runtime = \"embedded-min\"\n\n"
           "[serial]\n"
           "baud = 115200\n";
}

} // namespace ecconfig
} // namespace eng
