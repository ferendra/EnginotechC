#!/usr/bin/env bash
# EnginotechC++ — Alpha Smoke Test Suite
#
# Builds the compiler from source (unless ENGC points at a fresh binary),
# then compiles+runs every tests/smoke/*.ec and diffs the program output
# against the matching .expected file.
#
# Conventions:
#   NN_name.ec          → must compile & run; stdout must equal NN_name.expected
#   NN_name.stdin       → optional; fed to the program's stdin
#   neg_NN_name.ec      → must FAIL to compile (non-zero exit + diagnostics)
#
# Environment overrides:
#   ENGC            path to an existing engc binary (skips the build step)
#   ENGC_LLVM_BIN   dir containing clang or llvm-as/llc if not already in PATH
#   ENGC_LD_LIB     extra LD_LIBRARY_PATH needed by those LLVM tools
#
# Usage:  ./tests/alpha_smoke.sh

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SMOKE_DIR="$SCRIPT_DIR/smoke"
WORK="$(mktemp -d /tmp/engc_smoke.XXXXXX)"
trap 'rm -rf "$WORK"' EXIT

PASS=0
FAIL=0
SKIP=0

say()  { printf '%s\n' "$*"; }
good() { printf '\033[32mPASS\033[0m %s\n' "$*"; }
bad()  { printf '\033[31mFAIL\033[0m %s\n' "$*"; }
warn() { printf '\033[33mSKIP\033[0m %s\n' "$*"; }

# ---------------------------------------------------------------- backend ---
if [ -n "${ENGC_LLVM_BIN:-}" ]; then
    export PATH="$ENGC_LLVM_BIN:$PATH"
    export LD_LIBRARY_PATH="${ENGC_LD_LIB:-}:${LD_LIBRARY_PATH:-}"
fi

BACKEND="clang"
if ! command -v clang >/dev/null 2>&1; then
    if command -v llvm-as >/dev/null 2>&1 && command -v llc >/dev/null 2>&1 \
       && { command -v cc >/dev/null 2>&1 || command -v gcc >/dev/null 2>&1; }; then
        BACKEND="llvm-as+llc+cc"
    else
        warn "No backend found (need clang, or llvm-as+llc+cc). Install one or set ENGC_LLVM_BIN."
        exit 77
    fi
fi

# ------------------------------------------------------------------ build ---
ENGC_BIN="${ENGC:-}"
if [ -z "$ENGC_BIN" ]; then
    say "Building engc from source (backend: $BACKEND) ..."
    SOURCES=(
        compiler/lexer/lexer.cpp
        compiler/parser/parser.cpp
        compiler/semantic/semantic.cpp
        compiler/types/typechecker.cpp
        compiler/codegen/irgen.cpp
        compiler/codegen/caller.cpp
        compiler/driver/main.cpp
        compiler/driver/embedded.cpp
        build-system/build.cpp
        runtime/runtime.cpp
        runtime/embedded/embedded_rt.cpp
        std/io/io.cpp std/math/math.cpp std/string/string.cpp
        std/json/json.cpp std/fs/fs.cpp std/testing/testing.cpp
        formatter/formatter.cpp linter/linter.cpp
        compiler/targets/target.cpp compiler/targets/avr.cpp
        compiler/targets/esp32.cpp compiler/targets/x86.cpp
        compiler/targets/ecconfig.cpp compiler/targets/embedded_ext.cpp
        compiler/enginecore/enginecore.cpp
        packages/registry/registry.cpp packages/manager/manager.cpp
        compiler/platform/platform_common.cpp
    )
    for f in std/embedded/*/[a-z]*.cpp std/embedded/packages/*/[a-z]*.cpp; do
        [ -e "$f" ] && SOURCES+=("$f")
    done
    ENGC_BIN="$WORK/engc"
    if ! g++ -std=c++20 -Wall -Wextra -I"$ROOT/compiler" -I"$ROOT" \
         -o "$ENGC_BIN" "${SOURCES[@]}" 2>"$WORK/build.log"; then
        bad "compiler build failed:"
        head -40 "$WORK/build.log"
        exit 1
    fi
    W=$(grep -c "warning:" "$WORK/build.log" || true)
    say "Build OK ($W warnings)."
else
    say "Using provided engc binary: $ENGC_BIN"
fi

# ------------------------------------------------------------------- run ----
say ""
say "== Running smoke tests from $SMOKE_DIR"

for ec in "$SMOKE_DIR"/*.ec; do
    name="$(basename "$ec" .ec)"

    # ---- negative tests: must fail to compile ----
    if [[ "$name" == *_neg_* || "$name" == neg_* ]]; then
        if out=$("$ENGC_BIN" run "$ec" "$WORK/neg_out" 2>&1); then
            bad "$name — expected compile failure, but it succeeded"
            FAIL=$((FAIL+1))
        else
            good "$name"
            PASS=$((PASS+1))
        fi
        continue
    fi

    exp="$SMOKE_DIR/$name.expected"
    if [ ! -f "$exp" ]; then
        warn "$name — no .expected file, skipping"
        SKIP=$((SKIP+1))
        continue
    fi

    # ---- positive: build+run via `engc run`, filter driver lines ----
    stdin_file="$SMOKE_DIR/$name.stdin"
    if [ -f "$stdin_file" ]; then
        out=$("$ENGC_BIN" run "$ec" "$WORK/$name.bin" <"$stdin_file" 2>"$WORK/$name.err")
    else
        out=$("$ENGC_BIN" run "$ec" "$WORK/$name.bin" </dev/null 2>"$WORK/$name.err")
    fi
    rc=$?
    out_filtered="$(printf '%s\n' "$out" | grep -v -E '^(Compiled: |Ran: )')"

    if [ $rc -ne 0 ]; then
        bad "$name — engc exited with code $rc"
        sed 's/^/      /' "$WORK/$name.err" | head -10
        FAIL=$((FAIL+1))
        continue
    fi
    if [ "$out_filtered" != "$(cat "$exp")" ]; then
        bad "$name — output mismatch"
        diff <(printf '%s\n' "$out_filtered") "$exp" | sed 's/^/      /' | head -15
        FAIL=$((FAIL+1))
        continue
    fi
    good "$name"
    PASS=$((PASS+1))
done

say ""
say "Results: $PASS passed, $FAIL failed, $SKIP skipped (backend: $BACKEND)"
[ $FAIL -eq 0 ]
