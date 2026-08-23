#!/usr/bin/env bash
# Test multi-target compilation
set -euo pipefail

ENGCC="${ENGCC:-$HOME/.local/bin/engc}"
PASS=0
FAIL=0

test_target() {
    local target="$1"
    local src="$2"
    local out="$3"
    local name=$(basename "$src" .ec)
    
    if "$ENGCC" build --target "$target" "$src" "$out" >/dev/null 2>&1; then
        if [ -f "$out" ]; then
            echo "  ✅ $target: $name"
            PASS=$((PASS + 1))
        else
            echo "  ❌ $target: $name (file not created)"
            FAIL=$((FAIL + 1))
        fi
    else
        echo "  ❌ $target: $name (compile failed)"
        FAIL=$((FAIL + 1))
    fi
}

echo "═══════════════════════════════════════════════════════════"
echo "     Multi-Target Compilation Tests"
echo "══════════════════════════════════════════���════════════════"
echo ""

echo "Testing Python target..."
test_target python examples/hello/main.ec /tmp/test_hello.py
test_target python examples/arithmetic/main.ec /tmp/test_arith.py
test_target python examples/calculator/main.ec /tmp/test_calc.py

echo ""
echo "Testing WASM target..."
test_target wasm examples/hello/main.ec /tmp/test_hello.wat
test_target wasm examples/arithmetic/main.ec /tmp/test_arith.wat

echo ""
echo "Testing VM target..."
test_target vm examples/hello/main.ec /tmp/test_hello.ecvm
test_target vm examples/arithmetic/main.ec /tmp/test_arith.ecvm

echo ""
echo "═══════════════════════════════════════════════════════════"
echo "Results: $PASS passed, $FAIL failed"
echo "═══════════════════════════════════════════════════════════"
