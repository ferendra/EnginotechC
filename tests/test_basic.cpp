# Unit tests for EnginotechC++
#include <testing.h>

TEST(HelloTest) {
    ASSERT_EQ(1, 1);
}

TEST(BasicTest) {
    int x = 42;
    int y = 42;
    ASSERT_EQ(x, y);
    ASSERT_TRUE(true);
    ASSERT_FALSE(false);
}

TEST(StringTest) {
    std::string a = "hello";
    std::string b = "hello";
    ASSERT_EQ(a, b);
}

int main() {
    eng::testing::getRunner().runAll();
    
    auto results = eng::testing::getRunner().getResults();
    for (const auto& r : results) {
        std::cout << (r.passed ? "[PASS] " : "[FAIL] ") << r.name 
                  << " (" << r.durationMs << "ms)" 
                  << (r.passed ? "" : " - " + r.message) << "\n";
    }
    
    std::cout << "Total: " << eng::testing::getRunner().getPassedCount() 
              << " passed, " << eng::testing::getRunner().getFailedCount() 
              << " failed\n";
    
    return eng::testing::getRunner().getFailedCount() > 0 ? 1 : 0;
}