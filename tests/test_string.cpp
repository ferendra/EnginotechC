# Tests for std::string module
#include <testing.h>
#include <string.h>

TEST(StringUpper) {
    std::string result = eng::str::toUpper("hello");
    ASSERT_EQ(std::string("HELLO"), result);
}

TEST(StringLower) {
    std::string result = eng::str::toLower("WORLD");
    ASSERT_EQ(std::string("world"), result);
}

TEST(StringTrim) {
    std::string result = eng::str::trim("  hello  ");
    ASSERT_EQ(std::string("hello"), result);
}

TEST(StringStartsEnds) {
    ASSERT_TRUE(eng::str::startsWith("hello world", "hello"));
    ASSERT_TRUE(eng::str::endsWith("hello world", "world"));
    ASSERT_FALSE(eng::str::startsWith("hello", "world"));
}