#include "io.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdarg>

namespace eng {
namespace stdio {

std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

// NEW: input() function - read a line from stdin (Python compatibility)
std::string input() {
    return readLine();
}

std::string readAll() {
    std::stringstream ss;
    ss << std::cin.rdbuf();
    return ss.str();
}

int readInt() {
    int val;
    std::cin >> val;
    return val;
}

double readFloat() {
    double val;
    std::cin >> val;
    return val;
}

bool readBool() {
    std::string s;
    std::cin >> s;
    return s == "true" || s == "1" || s == "yes";
}

void print(const std::string& s) {
    std::cout << s;
}

void println(const std::string& s) {
    std::cout << s << "\n";
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// NEW: range() function - generate sequence 0 to n-1 (Python compatibility)
std::vector<int> range(int n) {
    std::vector<int> result;
    for (int i = 0; i < n; i++) {
        result.push_back(i);
    }
    return result;
}

bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file) return false;
    file << content;
    return true;
}

bool appendFile(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::app);
    if (!file) return false;
    file << content;
    return true;
}

} // namespace stdio
} // namespace eng
