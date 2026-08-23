#!/usr/bin/env bash
# Pack ekstensi VSCode EC menjadi .vsix (self-contained: LSP ikut di-bundle).
# Pemakaian: ./scripts/pack_vsix.sh   →  dist/ec-language-<versi>.vsix
set -euo pipefail
cd "$(dirname "$0")/.."

SRC="tools/vscode-ec"
LSP="tools/lsp/ec-lsp.js"
DIST="dist"

command -v node >/dev/null || { echo "node tidak ditemukan" >&2; exit 1; }

# Server LSP disalin ke dalam folder ekstensi agar ikut terpack.
mkdir -p "$SRC/server"
cp "$LSP" "$SRC/server/ec-lsp.js"

cd "$SRC"
VERSION=$(python3 -c "import json;print(json.load(open('package.json'))['version'])")
OUT="../../$DIST/ec-language-$VERSION.vsix"
mkdir -p "../../$DIST"

if npx --yes @vscode/vsce package --out "$OUT" --allow-missing-repository "$@"; then
    echo "VSIX: $OUT"
else
    echo "vsce gagal — fallback pack manual (zip + manifest)" >&2
    cd ../.. && exec python3 tools/pack_vsix_fallback.py
fi
