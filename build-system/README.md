# EnginotechC++ Build System

Build system untuk EnginotechC++ menggunakan CMake.

## Requirements

- CMake 3.16+
- C++20 compatible compiler (GCC 10+, Clang 14+, MSVC 2019+)
- LLVM/clang (untuk code generation)

## Build

```bash
# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Install

```bash
cmake --install build --prefix /usr/local
```

## Cross-compilation

```bash
# Cross-compile for Linux x86_64 using llvm-mingw
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/linux-x86_64.cmake
cmake --build build
```
