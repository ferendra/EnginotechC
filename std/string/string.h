// String Library for EnginotechC++
#ifndef ENG_STD_STRING_H
#define ENG_STD_STRING_H

#include <string>
#include <vector>

namespace eng {
namespace str {

// Conversion
std::string toLower(const std::string& s);
std::string toUpper(const std::string& s);
std::string trim(const std::string& s);
std::string trimLeft(const std::string& s);
std::string trimRight(const std::string& s);

// Search
bool startsWith(const std::string& s, const std::string& prefix);
bool endsWith(const std::string& s, const std::string& suffix);
int find(const std::string& s, const std::string& sub, int start = 0);
int lastFind(const std::string& s, const std::string& sub);

// Split and join
std::vector<std::string> split(const std::string& s, char delimiter);
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

// Repeat
std::string repeat(const std::string& s, int count);

// Replace
std::string replace(const std::string& s, const std::string& from, const std::string& to);

// Length
int length(const std::string& s);

} // namespace str
} // namespace eng

#endif // ENG_STD_STRING_H
