#!/usr/bin/env bash
# EnginotechC++ — build fondasi: satu perintah, auto-glob semua source.
# Pemakaian:
#   ./scripts/build.sh [output]     # default: /tmp/opencode/engc
#   ./scripts/build.sh --rebuild    # tanpa cache object
# Object files di-cache di .build/obj agar build ulang cepat.
set -euo pipefail
cd "$(dirname "$0")/.."

OUT="${1:-/tmp/opencode/engc}"
REBUILD=0
[[ "${1:-}" == "--rebuild" ]] && { OUT="${2:-/tmp/opencode/engc}"; REBUILD=1; }

# CXX: env override → zig di PATH → zig persisten di ~/.local
if [[ -z "${CXX:-}" ]]; then
    if command -v zig >/dev/null 2>&1; then
        CXX="zig c++"
    elif [[ -x "$HOME/.local/bin/zig" ]]; then
        CXX="$HOME/.local/bin/zig c++"
    else
        echo "error: no compiler (install zig to ~/.local/bin/zig)" >&2; exit 1
    fi
fi
STD="-std=c++20"
OBJDIR=".build/obj"
mkdir -p "$OBJDIR"

VERSION="$(awk 'NF && $1 !~ /^#/ {print; exit}' VERSION 2>/dev/null | tr -d '[:space:]')"
[[ -z "$VERSION" ]] && VERSION="dev"
DEFS="-DENG_VERSION=\"${VERSION}\""

# Auto-glob semua modul kecuali folder tes, berkas yang menuntut dependensi
# eksternal yang tidak tersedia, dan implementasi platform alternatif.
EXCLUDE_RE='^compiler/codegen/llvcodegen\.cpp$|^compiler/platform/platform_(windows|posix|common)\.cpp$|^compiler/targets/etskeleton\.cpp$|^compiler/ets/'
mapfile -t SRCS < <(find compiler std runtime packages build-system formatter linter langserver \
                    -name '*.cpp' -not -path '*/tests/*' \
                    | grep -Ev "$EXCLUDE_RE" | sort)
if [[ ${#SRCS[@]} -eq 0 ]]; then
    echo "error: no sources found" >&2; exit 1
fi

# Buang object yatim (source sudah tidak ada) supaya link tak pernah memakai sisa lama.
for old in "$OBJDIR"/*.o; do
    [[ -e "$old" ]] || continue
    stem="$(basename "$old" .o)"
    found=0
    for src in "${SRCS[@]}"; do
        [[ "$(echo "$src" | tr '/' '_')" == "$stem" ]] && { found=1; break; }
    done
    (( found )) || rm -f "$old"
done

changed=0
objs=()
for src in "${SRCS[@]}"; do
    obj="$OBJDIR/$(echo "$src" | tr '/' '_').o"
    if [[ $REBUILD -eq 1 || ! -f "$obj" || "$src" -nt "$obj" ]]; then
        # shellcheck disable=SC2086
        if ! $CXX $STD $DEFS ${CXXFLAGS:-} -c "$src" -o "$obj"; then
            echo "build failed at: $src" >&2
            exit 1
        fi
        changed=$((changed+1))
    fi
    [[ -f "$obj" ]] || { echo "object missing: $obj" >&2; exit 1; }
    objs+=("$obj")
done

echo "sources=${#SRCS[@]} recompiled=$changed version=$VERSION"
if ! $CXX ${CXXFLAGS:-} "${objs[@]}" -o "$OUT"; then
    echo "link failed" >&2
    exit 1
fi
echo "Built: $OUT (v$VERSION)"
