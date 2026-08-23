# Tests for std::math module
#include <testing.h>
#include <math.h>

TEST(MathAbs) {
    ASSERT_EQ(5, eng::math::abs(-5));
    ASSERT_EQ(5, eng::math::abs(5));
}

TEST(MathMaxMin) {
    ASSERT_EQ(10, eng::math::max(10, 5));
    ASSERT_EQ(5, eng::math::min(10, 5));
}

TEST(MathSqrt) {
    eng::testing::TestRunner::assertEquals(4.0, eng::math::sqrt(16.0), 0.0001, "");
    eng::testing::TestRunner::assertEquals(5.0, eng::math::sqrt(25.0), 0.0001, "");
}