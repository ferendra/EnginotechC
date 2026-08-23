// FS Library for EnginotechC++
#ifndef ENG_STD_FS_H
#define ENG_STD_FS_H

#include <string>
#include <vector>

namespace eng {
namespace fs {

bool exists(const std::string& path);
bool isFile(const std::string& path);
bool isDir(const std::string& path);
std::vector<std::string> listDir(const std::string& path);
std::string readText(const std::string& path);
std::vector<char> readBinary(const std::string& path);
bool writeText(const std::string& path, const std::string& content);
bool writeBinary(const std::string& path, const std::vector<char>& data);
bool mkdir(const std::string& path);
bool deleteFile(const std::string& path);
bool deleteDir(const std::string& path);
std::string getParent(const std::string& path);
std::string getExtension(const std::string& path);
std::string getFilename(const std::string& path);
long long getFileSize(const std::string& path);

} // namespace fs
} // namespace eng

#endif // ENG_STD_FS_H
