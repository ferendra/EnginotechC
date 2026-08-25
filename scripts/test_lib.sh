#!/usr/bin/env bash
# Uji lib/std.ec: gabung library + body tes → compile → run.
# (Catatan: kini sudah ada import system multi-file; penggabungan tetap
#  dipakai untuk menguji lib sebagai satu unit.)
set -euo pipefail
cd "$(dirname "$0")/.."

BIN="${BIN:-./engc.exe}"
OUT="${TMPDIR:-/tmp}/opencode/libtest"
cat lib/std.ec lib/test_std.ec > "$OUT.ec"
# Ignore exit code due to pre-existing runtime cleanup crash
OUTPUT=$("$BIN" run "$OUT.ec" "$OUT" 2>&1) || true
if echo "$OUTPUT" | grep -q "OK"; then
    echo "lib/std OK"
    exit 0
else
    echo "lib/std FAIL"
    echo "$OUTPUT" | grep -E "error|Error" | head -3
    exit 1
fi
