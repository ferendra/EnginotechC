#include "math.h"
#include <cstdlib>
#include <ctime>

namespace eng {
namespace math {

int abs(int x) { return x < 0 ? -x : x; }
double abs(double x) { return x < 0 ? -x : x; }

int max(int a, int b) { return a > b ? a : b; }
double max(double a, double b) { return a > b ? a : b; }

int min(int a, int b) { return a < b ? a : b; }
double min(double a, double b) { return a < b ? a : b; }

double pow(double base, double exp) { return std::pow(base, exp); }
double sqrt(double x) { return std::sqrt(x); }
double cbrt(double x) { return std::cbrt(x); }

double sin(double radians) { return std::sin(radians); }
double cos(double radians) { return std::cos(radians); }
double tan(double radians) { return std::tan(radians); }
double asin(double x) { return std::asin(x); }
double acos(double x) { return std::acos(x); }
double atan(double x) { return std::atan(x); }

long long round(double x) { return (long long)(x >= 0 ? x + 0.5 : x - 0.5); }
long long floor(double x) { return (long long)std::floor(x); }
long long ceil(double x) { return (long long)std::ceil(x); }

int random(int max) {
    static bool seeded = false;
    if (!seeded) { std::srand((unsigned)std::time(nullptr)); seeded = true; }
    return std::rand() % max;
}

double randomDouble() {
    static bool seeded = false;
    if (!seeded) { std::srand((unsigned)std::time(nullptr)); seeded = true; }
    return (double)std::rand() / RAND_MAX;
}

} // namespace math
} // namespace eng
