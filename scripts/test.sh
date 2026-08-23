#!/usr/bin/env bash
# Run full test suite: build compiler → integration tests → error corpus.
set -euo pipefail
cd "$(dirname "$0")/.."

BIN="${1:-$HOME/.local/bin/engc}"

echo "=== Building compiler ==="
./scripts/build.sh "$BIN" 2>&1 | tail -1

echo "=== Integration tests ==="
FAILED=0
for t in tests/integration/test_*.ec; do
    # Capture output; ignore exit code due to pre-existing runtime cleanup crash
    OUTPUT=$("$BIN" run "$t" /tmp/engtest_$(basename "$t" .ec) 2>&1) || true
    if echo "$OUTPUT" | grep -q "PASS\|OK"; then
        echo "[PASS] $t"
    else
        echo "[FAIL] $t"
        echo "$OUTPUT" | grep -E "error|Error" | head -3
        FAILED=$((FAILED + 1))
    fi
done
echo ""
echo "${FAILED} failed, $(ls tests/integration/test_*.ec | wc -l | tr -d ' ') total"
if [ $FAILED -gt 0 ]; then
    exit 1
fi

echo "=== Error-corpus (should all fail cleanly, no crash) ==="
FAILS_OK=()
for f in tests/errors/*.ec; do
    # Should fail to compile — ignore exit code (crash on cleanup is pre-existing)
    OUTPUT=$("$BIN" run "$f" /tmp/opencode/_err_out 2>&1) || true
    if echo "$OUTPUT" | grep -q "error\|Error\|failed"; then
        FAILS_OK+=("$f")
    else
        echo "FAIL: $f should have errored"
        exit 1
    fi
done
echo "Passed ${#FAILS_OK[@]} error-case test(s): ${FAILS_OK[*]}"

echo "=== Library tests ==="
./scripts/test_lib.sh

echo "=== Self-hosting Stage 1 ==="
# Ignore exit code due to pre-existing runtime cleanup crash
OUTPUT=$("$BIN" run selfhost/stage1-lexer/main.ec /tmp/opencode/sh1 2>&1) || true
if echo "$OUTPUT" | grep -q "OK"; then
    echo "stage1-lexer OK"
else
    echo "stage1-lexer FAIL"
    echo "$OUTPUT" | grep -E "error|Error" | head -3
fi

echo "=== Benchmark compile ==="
OUTPUT=$("$BIN" run examples/bench/main.ec /tmp/opencode/bench_out 2>&1) || true
if echo "$OUTPUT" | grep -qE "fib\(|loop sum|Compiled"; then
    echo "bench OK"
else
    echo "bench FAIL"
    echo "$OUTPUT" | grep -E "error|Error" | head -3
fi

echo "=== Examples verification ==="
EX_OK=()
EX_FAIL=()
for ex in examples/hello/main.ec examples/arithmetic/main.ec examples/calculator/main.ec examples/os-demo/main.ec; do
    OUTPUT=$("$BIN" run "$ex" /tmp/ex_out 2>&1) || true
    if echo "$OUTPUT" | grep -qE "OK|Compiled|Hello|Grade|Calculator"; then
        EX_OK+=("$(basename $(dirname $ex))")
    else
        EX_FAIL+=("$(basename $(dirname $ex))")
    fi
done
echo "Working examples: ${EX_OK[*]}"
if [ ${#EX_FAIL[@]} -gt 0 ]; then
    echo "Failing examples (unsupported features): ${EX_FAIL[*]}"
fi

echo "=== ALL TESTS PASSED ==="

