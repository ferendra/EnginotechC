# Tests for std::io module
#include <testing.h>
#include <io.h>

TEST(FileWriteRead) {
    std::string path = "test_output.txt";
    ASSERT_TRUE(eng::stdio::writeFile(path, "Hello World"));
    std::string content = eng::stdio::readFile(path);
    ASSERT_EQ(std::string("Hello World"), content);
    eng::stdio::fileExists(path);
}

TEST(FileAppend) {
    std::string path = "test_append.txt";
    ASSERT_TRUE(eng::stdio::writeFile(path, "Line 1"));
    ASSERT_TRUE(eng::stdio::appendFile(path, "\nLine 2"));
    std::string content = eng::stdio::readFile(path);
    ASSERT_TRUE(content.find("Line 1") != std::string::npos);
    ASSERT_TRUE(content.find("Line 2") != std::string::npos);
}