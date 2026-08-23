#include "string.h"
#include <algorithm>
#include <sstream>

namespace eng {
namespace str {

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string trimLeft(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string trimRight(const std::string& s) {
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int find(const std::string& s, const std::string& sub, int start) {
    size_t pos = s.find(sub, (size_t)start);
    return pos == std::string::npos ? -1 : (int)pos;
}

int lastFind(const std::string& s, const std::string& sub) {
    size_t pos = s.rfind(sub);
    return pos == std::string::npos ? -1 : (int)pos;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> result;
    std::istringstream stream(s);
    std::string token;
    while (std::getline(stream, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) result += delimiter;
        result += parts[i];
    }
    return result;
}

std::string repeat(const std::string& s, int count) {
    std::string result;
    for (int i = 0; i < count; ++i) {
        result += s;
    }
    return result;
}

std::string replace(const std::string& s, const std::string& from, const std::string& to) {
    std::string result = s;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

int length(const std::string& s) {
    return (int)s.length();
}

} // namespace str
} // namespace eng
