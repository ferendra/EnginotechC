// I/O Library for EnginotechC++
#ifndef ENG_STD_IO_H
#define ENG_STD_IO_H

#include <string>

namespace eng {
namespace stdio {

// Input
std::string readLine();
std::string readAll();
int readInt();
double readFloat();
bool readBool();

// Output
void print(const std::string& s);
void println(const std::string& s);
void printf(const char* format, ...);

// File operations
bool fileExists(const std::string& path);
std::string readFile(const std::string& path);
bool writeFile(const std::string& path, const std::string& content);
bool appendFile(const std::string& path, const std::string& content);

} // namespace stdio
} // namespace eng

#endif // ENG_STD_IO_H
