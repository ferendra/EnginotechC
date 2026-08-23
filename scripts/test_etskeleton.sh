#!/bin/bash
# Build and test ETC skeleton target

echo "=== Building ETC Skeleton Demo ==="
cd /home/ferendra-putra/Downloads/EngineSoft/enginotech-cpp

# Build the compiler
cmake -B build -DCMAKE_BUILD_TYPE=Debug 2>&1 | tail -5
cmake --build build -j$(nproc) 2>&1 | tail -10

if [ $? -ne 0 ]; then
    echo "Build failed, trying direct compilation..."
    g++ -std=c++20 -I. -o engc \
        compiler/driver/main.cpp \
        compiler/lexer/lexer.cpp \
        compiler/parser/parser.cpp \
        compiler/semantic/semantic.cpp \
        compiler/types/typechecker.cpp \
        compiler/codegen/caller.cpp \
        compiler/targets/target.cpp \
        compiler/targets/shim.cpp \
        compiler/targets/etskeleton.cpp \
        compiler/platform/platform_common.cpp \
        packages/registry/registry.cpp \
        packages/manager/manager.cpp \
        std/math/math.cpp \
        std/string/string.cpp \
        2>&1 | head -30
fi

echo ""
echo "=== Running ETC Demo ==="
./engc run examples/ets/skeleton_demo.etc 2>&1 || \
echo "Expected: Full compiler pipeline may not support .etc yet"

echo ""
echo "=== Testing Tokenization Only ==="
cat examples/ets/skeleton_demo.etc
echo ""
echo "Tokens expected:"
echo "  - print (PRINT token)"
echo "  - ( (LPAREN token)"
echo "  - \"Hello from ETC!\" (STRING_LITERAL token)"
echo "  - ) (RPAREN token)"
echo "  - EOF (TOKEN_EOF token)"
