// Math Library for EnginotechC++
#ifndef ENG_STD_MATH_H
#define ENG_STD_MATH_H

#include <cmath>

namespace eng {
namespace math {

// Constants
constexpr double PI = 3.14159265358979323846;
constexpr double E = 2.71828182845904523536;

// Basic operations
int abs(int x);
double abs(double x);
int max(int a, int b);
double max(double a, double b);
int min(int a, int b);
double min(double a, double b);

// Power and roots
double pow(double base, double exp);
double sqrt(double x);
double cbrt(double x);

// Trigonometry
double sin(double radians);
double cos(double radians);
double tan(double radians);
double asin(double x);
double acos(double x);
double atan(double x);

// Rounding
long long round(double x);
long long floor(double x);
long long ceil(double x);

// Random
int random(int max);
double randomDouble();

} // namespace math
} // namespace eng

#endif // ENG_STD_MATH_H
