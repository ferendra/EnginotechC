// Testing Framework for EnginotechC++
#ifndef ENG_STD_TESTING_H
#define ENG_STD_TESTING_H

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <stdexcept>

namespace eng {
namespace testing {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
    double durationMs;
};

class TestRunner {
public:
    void addTest(const std::string& name, std::function<void()> testFn);
    void runAll();
    void runFiltered(const std::string& filter);
    int getPassedCount() const;
    int getFailedCount() const;
    std::vector<TestResult> getResults() const;
    
    // Assertion macros (used in test functions)
    static void assertEquals(int expected, int actual, const std::string& msg = "");
    static void assertEquals(double expected, double actual, double epsilon, const std::string& msg = "");
    static void assertEquals(const std::string& expected, const std::string& actual, const std::string& msg = "");
    static void assertTrue(bool condition, const std::string& msg = "");
    static void assertFalse(bool condition, const std::string& msg = "");
    static void assertNotEqual(int a, int b, const std::string& msg = "");
    
private:
    struct TestCase {
        std::string name;
        std::function<void()> fn;
    };
    std::vector<TestCase> tests_;
    std::vector<TestResult> results_;
};

// Global test runner instance
extern TestRunner& getRunner();

} // namespace testing
} // namespace eng

// Test macro definitions
#define TEST(name) \
    void name(); \
    struct Register_##name { \
        Register_##name() { eng::testing::getRunner().addTest(#name, name); } \
    } register_##name; \
    void name()

#define ASSERT_EQ(expected, actual) eng::testing::TestRunner::assertEquals(expected, actual, "")
#define ASSERT_NE(a, b) eng::testing::TestRunner::assertNotEqual(a, b, "")
#define ASSERT_TRUE(cond) eng::testing::TestRunner::assertTrue(cond, "")
#define ASSERT_FALSE(cond) eng::testing::TestRunner::assertFalse(cond, "")

#endif // ENG_STD_TESTING_H
