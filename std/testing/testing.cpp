#include "testing.h"
#include <chrono>
#include <cmath>
#include <cassert>
#include <sstream>
#include <ctime>

namespace eng {
namespace testing {

TestRunner& getRunner() {
    static TestRunner instance;
    return instance;
}

void TestRunner::addTest(const std::string& name, std::function<void()> testFn) {
    tests_.push_back({name, testFn});
}

void TestRunner::runAll() {
    runFiltered("");
}

void TestRunner::runFiltered(const std::string& filter) {
    for (const auto& test : tests_) {
        if (!filter.empty() && test.name.find(filter) == std::string::npos) {
            continue;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        TestResult result;
        result.name = test.name;
        
        try {
            test.fn();
            result.passed = true;
            result.message = "passed";
        } catch (const std::exception& e) {
            result.passed = false;
            result.message = e.what();
        } catch (...) {
            result.passed = false;
            result.message = "Unknown exception";
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
        results_.push_back(result);
    }
}

int TestRunner::getPassedCount() const {
    int count = 0;
    for (const auto& r : results_) if (r.passed) ++count;
    return count;
}

int TestRunner::getFailedCount() const {
    int count = 0;
    for (const auto& r : results_) if (!r.passed) ++count;
    return count;
}

std::vector<TestResult> TestRunner::getResults() const {
    return results_;
}

void TestRunner::assertEquals(int expected, int actual, const std::string& msg) {
    if (expected != actual) {
        throw std::runtime_error("Assertion failed: expected " + std::to_string(expected) + 
                                 " but got " + std::to_string(actual) + 
                                 (msg.empty() ? "" : ": " + msg));
    }
}

void TestRunner::assertEquals(double expected, double actual, double epsilon, const std::string& msg) {
    if (std::fabs(expected - actual) > epsilon) {
        throw std::runtime_error("Assertion failed: expected " + std::to_string(expected) + 
                                 " but got " + std::to_string(actual) + 
                                 (msg.empty() ? "" : ": " + msg));
    }
}

void TestRunner::assertEquals(const std::string& expected, const std::string& actual, const std::string& msg) {
    if (expected != actual) {
        throw std::runtime_error("Assertion failed: expected \"" + expected + 
                                 "\" but got \"" + actual + "\"" + 
                                 (msg.empty() ? "" : ": " + msg));
    }
}

void TestRunner::assertTrue(bool condition, const std::string& msg) {
    if (!condition) {
        throw std::runtime_error("Assertion failed: expected true" + 
                                 (msg.empty() ? "" : ": " + msg));
    }
}

void TestRunner::assertFalse(bool condition, const std::string& msg) {
    if (condition) {
        throw std::runtime_error("Assertion failed: expected false" + 
                                 (msg.empty() ? "" : ": " + msg));
    }
}

void TestRunner::assertNotEqual(int a, int b, const std::string& msg) {
    if (a == b) {
        throw std::runtime_error("Assertion failed: expected different values" + 
                                 (msg.empty() ? "" : ": " + msg));
    }
}

} // namespace testing
} // namespace eng
